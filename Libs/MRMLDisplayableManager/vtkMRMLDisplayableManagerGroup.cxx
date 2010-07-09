/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableManagerGroup.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerGroup.h"
#include "vtkMRMLDisplayableManagerFactory.h"
#include "vtkMRMLAbstractDisplayableManager.h"

// MRML includes
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkInstantiator.h>

// STD includes
#include <vector>
//#include <map>
#include <algorithm>
#include <cassert>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLDisplayableManagerGroup, "$Revision: 13859 $");
vtkStandardNewMacro(vtkMRMLDisplayableManagerGroup);

//----------------------------------------------------------------------------
class vtkMRMLDisplayableManagerGroup::vtkInternal
{
public:
  vtkInternal();

  // Collection of Displayable Managers
  std::vector<vtkMRMLAbstractDisplayableManager *> DisplayableManagers;

  // .. and its associated convenient typedef
  typedef std::vector<vtkMRMLAbstractDisplayableManager *>::iterator DisplayableManagersIt;

  // Map DisplayableManagerName -> DisplayableManagers*
  std::map<std::string, vtkMRMLAbstractDisplayableManager*> NameToDisplayableManagerMap;

  // .. and its associated convenient typedef
  typedef std::map<std::string, vtkMRMLAbstractDisplayableManager*>::iterator
      NameToDisplayableManagerMapIt;

  vtkSmartPointer<vtkCallbackCommand> CallBackCommand;
  vtkMRMLDisplayableManagerFactory*   DisplayableManagerFactory;
  vtkMRMLViewNode*                    MRMLViewNode;
  vtkRenderer*                        Renderer;
  bool                                Initialized;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerGroup::vtkInternal::vtkInternal()
{
  this->MRMLViewNode = 0;
  this->Renderer = 0;
  this->Initialized = false;
  this->CallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->DisplayableManagerFactory = 0;
}

//----------------------------------------------------------------------------
// vtkMRMLDisplayableManagerGroup methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerGroup::vtkMRMLDisplayableManagerGroup()
{
  this->Internal = new vtkInternal;
  this->Internal->CallBackCommand->SetCallback(Self::DoCallback);
  this->Internal->CallBackCommand->SetClientData(this);
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerGroup::~vtkMRMLDisplayableManagerGroup()
{
  this->SetAndObserveDisplayableManagerFactory(0);

  for(std::size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    this->Internal->DisplayableManagers[i]->RemoveMRMLObservers();
    }

  this->SetMRMLViewNode(0);

  for(size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    this->Internal->DisplayableManagers[i]->Delete();
    }

  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->UnRegister(this);
    }

  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::SetAndObserveDisplayableManagerFactory(
    vtkMRMLDisplayableManagerFactory * factory)
{
  // Remove observers
  if (this->Internal->DisplayableManagerFactory)
    {
    this->Internal->DisplayableManagerFactory->RemoveObserver(this->Internal->CallBackCommand);
    this->Internal->DisplayableManagerFactory->Delete();
    this->Internal->DisplayableManagerFactory = 0;
    }

  this->Internal->DisplayableManagerFactory = factory;

  // Add observers
  if (this->Internal->DisplayableManagerFactory)
    {

    this->Internal->DisplayableManagerFactory->Register(this);

    // DisplayableManagerFactoryRegisteredEvent
    this->Internal->DisplayableManagerFactory->AddObserver(
        vtkMRMLDisplayableManagerFactory::DisplayableManagerFactoryRegisteredEvent,
        this->Internal->CallBackCommand);

    // DisplayableManagerFactoryUnRegisteredEvent
    this->Internal->DisplayableManagerFactory->AddObserver(
        vtkMRMLDisplayableManagerFactory::DisplayableManagerFactoryUnRegisteredEvent,
        this->Internal->CallBackCommand);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::AddAndInitialize(
    vtkMRMLAbstractDisplayableManager * displayableManager)
{
  // Sanity checks
  if (!displayableManager)
    {
    vtkWarningMacro(<<"AddAndInitialize - displayableManager is NULL");
    return;
    }

  // Make sure the displayableManager has NOT already been added
  vtkInternal::DisplayableManagersIt it = std::find(this->Internal->DisplayableManagers.begin(),
                                                    this->Internal->DisplayableManagers.end(),
                                                    displayableManager);

  if ( it != this->Internal->DisplayableManagers.end())
    {
    vtkWarningMacro(<<"AddAndInitialize - "
                    << displayableManager->GetClassName()
                    << " (" << displayableManager << ") already added");
    return;
    }

  // Initialize DisplayableManager if required
  bool initialized = displayableManager->IsInitialized();

  // Already initialized DisplayableManager are expected to have the same Renderer
  assert(initialized ? displayableManager->GetRenderer() == this->Internal->Renderer : 1);

  if (!initialized)
    {
    displayableManager->Initialize(this, this->Internal->Renderer);
    }

  displayableManager->Register(this);
  this->Internal->DisplayableManagers.push_back(displayableManager);

  // Update Name -> Object map
  const char * displayableManagerClassName = displayableManager->GetClassName();
  this->Internal->NameToDisplayableManagerMap[displayableManagerClassName] = displayableManager;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "registering DisplayableManager: " << displayableManager );
}

//----------------------------------------------------------------------------
int vtkMRMLDisplayableManagerGroup::GetDisplayableManagerCount()
{
  return this->Internal->DisplayableManagers.size();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::Initialize(vtkRenderer* newRenderer)
{
  // Sanity checks
  if (this->Internal->Initialized)
    {
    return;
    }
  if (!newRenderer)
    {
    return;
    }

  this->Internal->Renderer = newRenderer;
  this->Internal->Renderer->Register(this);

  this->Internal->Initialized = true;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "initializing DisplayableManagerGroup using Renderer: " << newRenderer);

  // Loop though DisplayableManager and intialize
  for(size_t i = 0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    vtkMRMLAbstractDisplayableManager * displayableManager = this->Internal->DisplayableManagers[i];
    bool initialized = displayableManager->IsInitialized();

    // Already initalized DisplayableManager are expected to have the same Renderer
    assert(initialized ? displayableManager->GetRenderer() == newRenderer : 1);

    if (!initialized)
      {
      displayableManager->Initialize(this, newRenderer);
      }
    }

  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLDisplayableManagerGroup::IsInitialized()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Initialized of "
                << this->Internal->Initialized);
  return this->Internal->Initialized;
}

//----------------------------------------------------------------------------
vtkRenderWindowInteractor* vtkMRMLDisplayableManagerGroup::GetInteractor()
{
  if (!this->Internal->Renderer || !this->Internal->Renderer->GetRenderWindow())
    {
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning Interactor address 0");
    return 0;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Renderer->GetRenderWindow()->GetInteractor() address "
                << this->Internal->Renderer->GetRenderWindow()->GetInteractor() );
  return this->Internal->Renderer->GetRenderWindow()->GetInteractor();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::RequestRender()
{
  this->InvokeEvent(vtkCommand::UpdateEvent);
}

//----------------------------------------------------------------------------
vtkRenderer* vtkMRMLDisplayableManagerGroup::GetRenderer()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Renderer address " << this->Internal->Renderer );
  return this->Internal->Renderer;
}

//----------------------------------------------------------------------------
vtkMRMLViewNode* vtkMRMLDisplayableManagerGroup::GetMRMLViewNode()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->MRMLViewNode address " << this->Internal->MRMLViewNode );
  return this->Internal->MRMLViewNode;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::SetMRMLViewNode(vtkMRMLViewNode* newMRMLViewNode)
{
  for(std::size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    vtkMRMLAbstractDisplayableManager * displayableManager = this->Internal->DisplayableManagers[i];

    displayableManager->SetAndObserveMRMLViewNode(newMRMLViewNode);

    displayableManager->CreateIfPossible();
    }
  vtkSetObjectBodyMacro(Internal->MRMLViewNode, vtkMRMLViewNode, newMRMLViewNode);
}

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager*
    vtkMRMLDisplayableManagerGroup::GetDisplayableManagerByClassName(const char* className)
{
  if (!className)
    {
    vtkWarningMacro(<< "GetDisplayableManagerByClassName - className is NULL");
    return 0;
    }
  vtkInternal::NameToDisplayableManagerMapIt it =
      this->Internal->NameToDisplayableManagerMap.find(className);

  if (it == this->Internal->NameToDisplayableManagerMap.end())
    {
    vtkWarningMacro(<< "GetDisplayableManagerByClassName - "
                    << "DisplayableManager identified by [" << className << "] does NOT exist");
    return 0;
    }

  return it->second;
}

//-----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::DoCallback(vtkObject* vtk_obj, unsigned long event,
                                                void* client_data, void* call_data)
{
  vtkMRMLDisplayableManagerFactory* factory =
      reinterpret_cast<vtkMRMLDisplayableManagerFactory*>(vtk_obj);
  vtkMRMLDisplayableManagerGroup* self =
      reinterpret_cast<vtkMRMLDisplayableManagerGroup*>(client_data);
  char* displayableManagerName = reinterpret_cast<char*>(call_data);
  assert(factory);
  assert(self);
  assert(displayableManagerName);

  switch(event)
    {
    case vtkMRMLDisplayableManagerFactory::DisplayableManagerFactoryRegisteredEvent:
      self->onDisplayableManagerFactoryRegisteredEvent(displayableManagerName);
      break;
    case vtkMRMLDisplayableManagerFactory::DisplayableManagerFactoryUnRegisteredEvent:
      self->onDisplayableManagerFactoryUnRegisteredEvent(displayableManagerName);
      break;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::onDisplayableManagerFactoryRegisteredEvent(
    const char* displayableManagerName)
{
  assert(displayableManagerName);

  // Object will be unregistered when the SmartPointer will go out-of-scope
  vtkSmartPointer<vtkObject> objectSmartPointer;
  objectSmartPointer.TakeReference(vtkInstantiator::CreateInstance(displayableManagerName));
  vtkMRMLAbstractDisplayableManager* displayableManager =
      vtkMRMLAbstractDisplayableManager::SafeDownCast(objectSmartPointer);
  if (!displayableManager)
    {
    vtkErrorMacro(<<"InstantiateDisplayableManagers - Failed to instantiate "
                  << displayableManagerName);
    return;
    }

  this->AddAndInitialize(displayableManager);

  vtkDebugMacro(<< "group:" << this << ", onDisplayableManagerFactoryRegisteredEvent:"
                << displayableManagerName)
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::onDisplayableManagerFactoryUnRegisteredEvent(
    const char* displayableManagerName)
{
  assert(displayableManagerName);

  // Find the associated object
  vtkInternal::NameToDisplayableManagerMapIt it =
      this->Internal->NameToDisplayableManagerMap.find(displayableManagerName);

  // The DisplayableManager is expected to be in the map
  assert(it != this->Internal->NameToDisplayableManagerMap.end());

  vtkMRMLAbstractDisplayableManager * displayableManager = it->second;
  assert(displayableManager);

  // Find DisplayableManager in the vector
  vtkInternal::DisplayableManagersIt it2 = std::find(this->Internal->DisplayableManagers.begin(),
                                                     this->Internal->DisplayableManagers.end(),
                                                     displayableManager);

  // The DisplayableManager is expected to be in the vector
  assert(it2 != this->Internal->DisplayableManagers.end());

  // Remove it from the vector
  this->Internal->DisplayableManagers.erase(it2);

  // Clean memory
  displayableManager->RemoveMRMLObservers();
  displayableManager->Delete();

  // Remove it from the map
  this->Internal->NameToDisplayableManagerMap.erase(it);

  vtkDebugMacro(<< "group:" << this << ", onDisplayableManagerFactoryUnRegisteredEvent:"
                << displayableManagerName)
}
