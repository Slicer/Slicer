/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include "vtkMRMLAbstractDisplayableManager.h"
#include "vtkMRMLDisplayableManagerGroup.h"
#include "vtkMRMLDisplayableManagerFactory.h"
#include <vtkMRMLLightBoxRendererManagerProxy.h>

#ifdef MRMLDisplayableManager_USE_PYTHON
#include "vtkMRMLScriptedDisplayableManager.h"
#endif

// MRML includes
#include <vtkMRMLNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkDebugLeaks.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <vector>

//----------------------------------------------------------------------------
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
  vtkWeakPointer<vtkMRMLLightBoxRendererManagerProxy> LightBoxRendererManagerProxy;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerGroup::vtkInternal::vtkInternal()
{
  this->MRMLDisplayableNode = nullptr;
  this->Renderer = nullptr;
  this->CallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->DisplayableManagerFactory = nullptr;
  this->LightBoxRendererManagerProxy = nullptr;
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
  this->SetAndObserveDisplayableManagerFactory(nullptr);
  this->SetMRMLDisplayableNode(nullptr);

  for(size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    this->Internal->DisplayableManagers[i]->Delete();
    }

  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->UnRegister(this);
    }

  if (this->Internal->LightBoxRendererManagerProxy)
    {
    this->Internal->LightBoxRendererManagerProxy = nullptr;
    }

  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLDisplayableManagerGroup
::IsADisplayableManager(const char* displayableManagerName)
{
  // Check if displayableManagerName is a valid displayable manager
  vtkSmartPointer<vtkObject> objectSmartPointer;
  objectSmartPointer.TakeReference(vtkObjectFactory::CreateInstance(displayableManagerName));
  if (objectSmartPointer.GetPointer() &&
      objectSmartPointer->IsA("vtkMRMLAbstractDisplayableManager"))
    {
    return true;
    }
#ifdef VTK_DEBUG_LEAKS
  vtkDebugLeaks::DestructClass(displayableManagerName);
#endif
#ifdef MRMLDisplayableManager_USE_PYTHON
  // Check if vtkClassOrScriptName is a python script
  if (std::string(displayableManagerName).find(".py") != std::string::npos)
    {
    // TODO Make sure the file exists ...
    return true;
    }
#endif
  return false;
}

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager* vtkMRMLDisplayableManagerGroup
::InstantiateDisplayableManager(const char* displayableManagerName)
{
  vtkMRMLAbstractDisplayableManager* displayableManager = nullptr;
#ifdef MRMLDisplayableManager_USE_PYTHON
  // Are we dealing with a python scripted displayable manager
  if (std::string(displayableManagerName).find(".py") != std::string::npos)
    {
    // TODO Make sure the file exists ...
    vtkMRMLScriptedDisplayableManager* scriptedDisplayableManager =
      vtkMRMLScriptedDisplayableManager::New();
    scriptedDisplayableManager->SetPythonSource(displayableManagerName);
    displayableManager = scriptedDisplayableManager;
    }
  else
    {
#endif
    // Object will be unregistered when the SmartPointer will go out-of-scope
    displayableManager = vtkMRMLAbstractDisplayableManager::SafeDownCast(
      vtkObjectFactory::CreateInstance(displayableManagerName));
#ifdef MRMLDisplayableManager_USE_PYTHON
    }
#endif
  return displayableManager;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::Initialize(vtkMRMLDisplayableManagerFactory * factory,
                                                vtkRenderer * renderer)
{
  // Sanity checks
  if (!factory)
    {
    vtkWarningMacro(<<"Initialize - factory is NULL");
    return;
    }
  if (!renderer)
    {
    vtkWarningMacro(<<"Initialize - renderer is NULL");
    return;
    }

  // A Group observes the factory and eventually instantiates new DisplayableManager
  // when they are registered in the factory
  this->SetAndObserveDisplayableManagerFactory(factory);
  this->SetRenderer(renderer);

  for(int i=0; i < factory->GetRegisteredDisplayableManagerCount(); ++i)
    {
    std::string classOrScriptName = factory->GetRegisteredDisplayableManagerName(i);
    vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
    displayableManager.TakeReference(
      vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(classOrScriptName.c_str()));
    // Note that DisplayableManagerGroup will take ownership of the object
    this->AddDisplayableManager(displayableManager);
    }
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
    this->Internal->DisplayableManagerFactory = nullptr;
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
void vtkMRMLDisplayableManagerGroup::AddDisplayableManager(
    vtkMRMLAbstractDisplayableManager * displayableManager)
{
  // Sanity checks
  if (!displayableManager)
    {
    vtkWarningMacro(<<"AddDisplayableManager - displayableManager is NULL");
    return;
    }

  // Make sure the displayableManager has NOT already been added
  const char * displayableManagerClassName = displayableManager->GetClassName();
  if (this->GetDisplayableManagerByClassName(displayableManagerClassName) != nullptr)
    {
    vtkWarningMacro(<<"AddDisplayableManager - "
                    << displayableManager->GetClassName()
                    << " (" << displayableManager << ") already added");
    return;
    }

  displayableManager->SetMRMLDisplayableManagerGroup(this);
  if (this->Internal->DisplayableManagerFactory)
    {
    displayableManager->SetMRMLApplicationLogic(
      this->Internal->DisplayableManagerFactory->GetMRMLApplicationLogic());
    }
  displayableManager->SetRenderer(this->Internal->Renderer);

  // pass the lightbox manager proxy to the new displayable manager
  displayableManager->SetLightBoxRendererManagerProxy(this->Internal->LightBoxRendererManagerProxy);

  displayableManager->SetAndObserveMRMLDisplayableNode(this->GetMRMLDisplayableNode());

  displayableManager->Register(this);
  this->Internal->DisplayableManagers.push_back(displayableManager);
  this->Internal->NameToDisplayableManagerMap[displayableManagerClassName] = displayableManager;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "registering DisplayableManager: " << displayableManager << "("
                << displayableManager->GetClassName() << ")");
}

//----------------------------------------------------------------------------
int vtkMRMLDisplayableManagerGroup::GetDisplayableManagerCount()
{
  return static_cast<int>(this->Internal->DisplayableManagers.size());
}

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager * vtkMRMLDisplayableManagerGroup::GetNthDisplayableManager(int n)
{
  int numManagers = this->GetDisplayableManagerCount();
  if (n < 0 || n >= numManagers)
    {
    return nullptr;
    }
  return this->Internal->DisplayableManagers[n];
}
//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::SetRenderer(vtkRenderer* newRenderer)
{
  // Sanity checks
  if (this->Internal->Renderer == newRenderer)
    {
    return;
    }

  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->Delete();
    }

  this->Internal->Renderer = newRenderer;

  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->Register(this);
    }

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "initializing DisplayableManagerGroup using Renderer: " << newRenderer);

  // Loop though DisplayableManager and initialize
  for(size_t i = 0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    vtkMRMLAbstractDisplayableManager * displayableManager = this->Internal->DisplayableManagers[i];
    displayableManager->SetRenderer(newRenderer);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
vtkRenderWindowInteractor* vtkMRMLDisplayableManagerGroup::GetInteractor()
{
  if (!this->Internal->Renderer || !this->Internal->Renderer->GetRenderWindow())
    {
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning Interactor address 0");
    return nullptr;
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
    return nullptr;
    }
  vtkInternal::NameToDisplayableManagerMapIt it =
      this->Internal->NameToDisplayableManagerMap.find(className);

  if (it == this->Internal->NameToDisplayableManagerMap.end())
    {
    return nullptr;
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

  vtkSmartPointer<vtkMRMLAbstractDisplayableManager> newDisplayableManager;
  newDisplayableManager.TakeReference(
    vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName));
  this->AddDisplayableManager(newDisplayableManager);
  vtkDebugMacro(<< "group:" << this << ", onDisplayableManagerFactoryRegisteredEvent:"
                << displayableManagerName);
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
                << displayableManagerName);
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayableManagerGroup::SetLightBoxRendererManagerProxy(vtkMRMLLightBoxRendererManagerProxy* mgr)
{
  this->Internal->LightBoxRendererManagerProxy = mgr;

  for(size_t i=0; i < this->Internal->DisplayableManagers.size(); ++i)
    {
    this->Internal->DisplayableManagers[i]->SetLightBoxRendererManagerProxy(this->Internal->LightBoxRendererManagerProxy);
    }
}

//---------------------------------------------------------------------------
vtkMRMLLightBoxRendererManagerProxy* vtkMRMLDisplayableManagerGroup::GetLightBoxRendererManagerProxy()
{
  return this->Internal->LightBoxRendererManagerProxy;
}
