/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerGroup.h"
#include "vtkMRMLDisplayableManagerFactory.h"
#include "vtkMRMLAbstractDisplayableManager.h"
#ifdef MRMLDisplayableManager_USE_PYTHON
#include "vtkMRMLScriptedDisplayableManager.h"
#endif

// MRML includes
#include <vtkMRMLNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkInstantiator.h>

// STD includes
#include <vector>
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

  vtkSmartPointer<vtkCallbackCommand>   CallBackCommand;
  vtkMRMLDisplayableManagerFactory*     DisplayableManagerFactory;
  vtkMRMLNode*                          MRMLDisplayableNode;
  vtkRenderer*                          Renderer;
  bool                                  Initialized;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerGroup::vtkInternal::vtkInternal()
{
  this->MRMLDisplayableNode = 0;
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
  this->SetMRMLDisplayableNode(0);

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
    displayableManager->SetAndObserveMRMLDisplayableNode(this->GetMRMLDisplayableNode());
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
  return static_cast<int>(this->Internal->DisplayableManagers.size());
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
vtkMRMLNode* vtkMRMLDisplayableManagerGroup::GetMRMLDisplayableNode()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->MRMLDisplayableNode address "
                << this->Internal->MRMLDisplayableNode );
  return this->Internal->MRMLDisplayableNode;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::SetMRMLDisplayableNode(
    vtkMRMLNode* newMRMLDisplayableNode)
{
  for(std::size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    vtkMRMLAbstractDisplayableManager * displayableManager = this->Internal->DisplayableManagers[i];

    displayableManager->SetAndObserveMRMLDisplayableNode(newMRMLDisplayableNode);
    }
  vtkSetObjectBodyMacro(Internal->MRMLDisplayableNode, vtkMRMLNode, newMRMLDisplayableNode);
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
  vtkMRMLDisplayableManagerGroup* self =
      reinterpret_cast<vtkMRMLDisplayableManagerGroup*>(client_data);
  char* displayableManagerName = reinterpret_cast<char*>(call_data);
  assert(self);
  assert(reinterpret_cast<vtkMRMLDisplayableManagerFactory*>(vtk_obj));
#ifndef _DEBUG
  (void)vtk_obj;
#endif
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

#ifdef MRMLDisplayableManager_USE_PYTHON
  std::string str(displayableManagerName);
  if (str.find(".py") != std::string::npos)
    {
    vtkSmartPointer<vtkMRMLScriptedDisplayableManager> scriptedDisplayableManager =
        vtkSmartPointer<vtkMRMLScriptedDisplayableManager>::New();
    scriptedDisplayableManager->SetPythonSource(displayableManagerName);

    this->AddAndInitialize(scriptedDisplayableManager);
    }
  else
    {
#endif
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
#ifdef MRMLDisplayableManager_USE_PYTHON
    }
#endif

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
  displayableManager->Delete();

  // Remove it from the map
  this->Internal->NameToDisplayableManagerMap.erase(it);

  vtkDebugMacro(<< "group:" << this << ", onDisplayableManagerFactoryUnRegisteredEvent:"
                << displayableManagerName)
}
