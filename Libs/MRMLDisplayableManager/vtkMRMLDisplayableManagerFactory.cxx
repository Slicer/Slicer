/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableManagerFactory.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerFactory.h"

// MRML includes
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

// STD includes
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLDisplayableManagerFactory, "$Revision: 13859 $");
vtkStandardNewMacro(vtkMRMLDisplayableManagerFactory);

//----------------------------------------------------------------------------
class vtkMRMLDisplayableManagerFactory::vtkInternal
{
public:
  vtkInternal();

  // Collection of Displayable Managers
  std::vector<vtkMRMLAbstractDisplayableManager *> DisplayableManagers;

  // Convenient typedef
  typedef std::vector<vtkMRMLAbstractDisplayableManager *>::iterator DisplayableManagersIt;

  // Map DisplayableManagerName -> DisplayableManagers*
  std::map<std::string, vtkMRMLAbstractDisplayableManager*> NameToDisplayableManagerMap;

  // Convenient typedef
  typedef std::map<std::string, vtkMRMLAbstractDisplayableManager*>::iterator
      NameToDisplayableManagerMapIt;


  bool                        DisplayableManagersCreated;
  vtkMRMLViewNode*            MRMLViewNode;
  vtkRenderer*                Renderer;
  vtkRenderWindowInteractor*  Interactor;
  bool                        Initialized;

};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::vtkInternal::vtkInternal()
{
  this->DisplayableManagersCreated = false;
  this->MRMLViewNode = 0;
  this->Renderer = 0;
  this->Interactor = 0;
  this->Initialized = false;
}

//----------------------------------------------------------------------------
// vtkMRMLDisplayableManagerFactory methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::vtkMRMLDisplayableManagerFactory()
{
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::~vtkMRMLDisplayableManagerFactory()
{
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
void vtkMRMLDisplayableManagerFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerFactory::Initialize(vtkRenderer* newRenderer)
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
                << "initializing DisplayableManager using Renderer: " << newRenderer );

  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLDisplayableManagerFactory::IsInitialized()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Initialized of "
                << this->Internal->Initialized);
  return this->Internal->Initialized;
}

//----------------------------------------------------------------------------
vtkRenderWindowInteractor* vtkMRMLDisplayableManagerFactory::GetInteractor()
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
void vtkMRMLDisplayableManagerFactory::RequestRender()
{
  this->InvokeEvent(vtkCommand::UpdateEvent);
}

//----------------------------------------------------------------------------
vtkRenderer* vtkMRMLDisplayableManagerFactory::GetRenderer()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Renderer address " << this->Internal->Renderer );
  return this->Internal->Renderer;
}

//----------------------------------------------------------------------------
vtkMRMLViewNode* vtkMRMLDisplayableManagerFactory::GetMRMLViewNode()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->MRMLViewNode address " << this->Internal->MRMLViewNode );
  return this->Internal->MRMLViewNode;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerFactory::SetMRMLViewNode(vtkMRMLViewNode* newMRMLViewNode)
{
  if (this->Internal->MRMLViewNode == newMRMLViewNode)
    {
    return;
    }

  for(std::size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    vtkMRMLAbstractDisplayableManager * displayManager = this->Internal->DisplayableManagers[i];

    displayManager->SetAndObserveMRMLViewNode(newMRMLViewNode);

    displayManager->CreateIfPossible();

    }
  vtkSetObjectBodyMacro(Internal->MRMLViewNode, vtkMRMLViewNode, newMRMLViewNode);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerFactory::RegisterDisplayableManager(
  vtkMRMLAbstractDisplayableManager *displayableManager)
{
  // Sanity checks
  if (!displayableManager)
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - displayableManager is NULL");
    return;
    }

  // Make sure the manager has NOT already been registered
  vtkInternal::DisplayableManagersIt it = std::find(this->Internal->DisplayableManagers.begin(),
                                                    this->Internal->DisplayableManagers.end(),
                                                    displayableManager);

  if ( it != this->Internal->DisplayableManagers.end())
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - "
                    << displayableManager->GetClassName()
                    << " (" << displayableManager << ") already registered");
    return;
    }

  // Make sure NO managers having the same className have been registered
  vtkMRMLAbstractDisplayableManager * existingDisplayableManager =
      this->GetDisplayableManagerByClassName(displayableManager->GetClassName());
  if (existingDisplayableManager)
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - There is already an existing "
                    << existingDisplayableManager->GetClassName() << " identified by "
                    << "(" << existingDisplayableManager <<")"
                    << " - Failed to register instance (" << displayableManager << ")");
    return;
    }

  displayableManager->Register(this);

  // Initialize
  assert(!displayableManager->IsInitialized());
  displayableManager->Initialize(this, this->GetRenderer());

  // Add to list
  this->Internal->DisplayableManagers.push_back(displayableManager);

  // Update Name -> Object map
  const char * displayableManagerClassName = displayableManager->GetClassName();
  this->Internal->NameToDisplayableManagerMap[displayableManagerClassName] = displayableManager;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "registering DisplayableManager: " << displayableManager );
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerFactory::UnRegisterDisplayableManager(
    vtkMRMLAbstractDisplayableManager *displayableManager)
{
  // Sanitu checks
  if (!displayableManager)
    {
    return;
    }

  // Make it's a registered displayable manager
  // Make sure the manager has NOT already been registered
  vtkInternal::DisplayableManagersIt it = std::find(this->Internal->DisplayableManagers.begin(),
                                                    this->Internal->DisplayableManagers.end(),
                                                    displayableManager);

  if ( it == this->Internal->DisplayableManagers.end())
    {
    vtkWarningMacro(<< "UnRegisterDisplayableManager - "
                    << "displayableManager (" << displayableManager << ") is NOT registered");
    return;
    }

  // Clean vector
  this->Internal->DisplayableManagers.erase(it);

  // Clean map
  vtkInternal::NameToDisplayableManagerMapIt it2 =
      this->Internal->NameToDisplayableManagerMap.find(displayableManager->GetClassName());
  assert(it2 != this->Internal->NameToDisplayableManagerMap.end());
  this->Internal->NameToDisplayableManagerMap.erase(it2);

  displayableManager->RemoveMRMLObservers();
  displayableManager->SetAndObserveMRMLViewNode(0);
  displayableManager->Delete();
}

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager*
    vtkMRMLDisplayableManagerFactory::GetDisplayableManagerByClassName(const char* className)
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
    //vtkWarningMacro(<< "GetDisplayableManagerByClassName - "
    //                << "DisplayableManager identified by [" << className << "] does NOT exist");
    return 0;
    }

  return it->second;
}

