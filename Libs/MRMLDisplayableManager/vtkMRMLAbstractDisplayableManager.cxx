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
#include "vtkMRMLAbstractDisplayableManager.h"
#include "vtkMRMLDisplayableManagerGroup.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkInteractorStyle.h>

// STD includes
#include <cassert>
#include <algorithm>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractDisplayableManager);
vtkCxxRevisionMacro(vtkMRMLAbstractDisplayableManager, "$Revision: 13525 $");

//----------------------------------------------------------------------------
class vtkMRMLAbstractDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLAbstractDisplayableManager* external);
  ~vtkInternal();

  /// Called after vtkCommand::DeleteEvent is called on the DisplayableManager
  static void DoDeleteCallback(vtkObject* vtk_obj, unsigned long event,
                               void* client_data, void* call_data);

  /// Called after vtkCommand::ModifiedEvent is called on the RenderWindowInteractor
  /// Allow to update observer in case the InteractorStyle associated with
  /// RenderWindowInteractor is updated.
  static void DoRenderWindowInteractorCallback(vtkObject* vtk_obj, unsigned long event,
                                               void* client_data, void* call_data);

  /// Set and observe \a newInteractorStyle
  void SetAndObserveInteractorStyle(vtkInteractorObserver* newInteractorStyle);

  /// Called after one of the observable event is invoked
  static void DoInteractorStyleCallback(vtkObject* vtk_obj, unsigned long event,
                                        void* client_data, void* call_data);

  /// Set and observe \a newMRMLInteractionNode
  void SetAndObserveMRMLInteractionNode(vtkMRMLInteractionNode* newMRMLInteractionNode);

  /// Called after vtkMRMLInteractionNode::InteractionModeChangedEvent is called on the
  /// current InteractionNode
  /// Allow to add/remove Interactor style observer in case the InteractionNode MouseMode
  /// is updated. InteractorStyleObserver are enabled if MouseMode
  /// is either: vtkMRMLInteractionNode::Place or vtkMRMLInteractionNode::PickManipulate
  /// \note Since we want to keep the virtual method ProcessMRMLEvent of the base class pure,
  /// the pattern MRMLObserverManager/ProcessMRMLEvent is not used here.
  static void DoMRMLInteractionNodeCallback(vtkObject* vtk_obj, unsigned long event,
                                            void* client_data, void* call_data);

  /// Called after MRML DisplayableNode is set, it will add/remove interactor style observer
  /// according to the state of the current MRML InteractionNode
  /// \sa DoMRMLInteractionNodeCallback
  void UpdateInteractorStyle();

  vtkMRMLAbstractDisplayableManager*        External;
  bool                                      Initialized;
  bool                                      Created;
  bool                                      UpdateFromMRMLRequested;
  vtkRenderer*                              Renderer;
  vtkMRMLNode*                              MRMLDisplayableNode;
  vtkSmartPointer<vtkIntArray>              MRMLDisplayableNodeObservableEvents;
  vtkMRMLSelectionNode*                     MRMLSelectionNode;
  vtkMRMLInteractionNode*                   MRMLInteractionNode;
  vtkSmartPointer<vtkCallbackCommand>       MRMLInteractionNodeCallBackCommand;
  vtkMRMLDisplayableManagerGroup*           DisplayableManagerGroup;
  vtkSmartPointer<vtkCallbackCommand>       DeleteCallBackCommand;
  vtkRenderWindowInteractor*                RenderWindowInteractor;
  vtkSmartPointer<vtkCallbackCommand>       RenderWindowInteractorCallBackCommand;
  vtkInteractorObserver*                    InteractorStyle;
  vtkSmartPointer<vtkCallbackCommand>       InteractorStyleCallBackCommand;
  std::vector<int>                          InteractorStyleObservableEvents;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::vtkInternal::vtkInternal(
    vtkMRMLAbstractDisplayableManager* external):External(external)
{
  this->Created = false;
  this->UpdateFromMRMLRequested = false;
  this->Renderer = 0;
  this->MRMLDisplayableNode = 0;
  this->MRMLDisplayableNodeObservableEvents = vtkSmartPointer<vtkIntArray>::New();
  this->DisplayableManagerGroup = 0;

  this->DeleteCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->DeleteCallBackCommand->SetCallback(
      vtkMRMLAbstractDisplayableManager::vtkInternal::DoDeleteCallback);

  this->RenderWindowInteractor = 0;
  this->RenderWindowInteractorCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->RenderWindowInteractorCallBackCommand->SetCallback(
      vtkMRMLAbstractDisplayableManager::vtkInternal::DoRenderWindowInteractorCallback);

  this->MRMLInteractionNode = 0;
  this->MRMLInteractionNodeCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->MRMLInteractionNodeCallBackCommand->SetCallback(
      vtkMRMLAbstractDisplayableManager::vtkInternal::DoMRMLInteractionNodeCallback);
  this->MRMLInteractionNodeCallBackCommand->SetClientData(this->External);

  this->InteractorStyle = 0;
  this->InteractorStyleCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->InteractorStyleCallBackCommand->SetCallback(
      vtkMRMLAbstractDisplayableManager::vtkInternal::DoInteractorStyleCallback);
  this->InteractorStyleCallBackCommand->SetClientData(this->External);

  // Default Interactor style events to observe
  this->InteractorStyleObservableEvents.push_back(vtkCommand::LeftButtonPressEvent);
  this->InteractorStyleObservableEvents.push_back(vtkCommand::LeftButtonReleaseEvent);
  this->InteractorStyleObservableEvents.push_back(vtkCommand::RightButtonPressEvent);
  this->InteractorStyleObservableEvents.push_back(vtkCommand::RightButtonReleaseEvent);
  this->InteractorStyleObservableEvents.push_back(vtkCommand::MiddleButtonPressEvent);
  this->InteractorStyleObservableEvents.push_back(vtkCommand::MiddleButtonReleaseEvent);
  this->InteractorStyleObservableEvents.push_back(vtkCommand::MouseWheelBackwardEvent);
  this->InteractorStyleObservableEvents.push_back(vtkCommand::MouseWheelForwardEvent);
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetAndObserveInteractorStyle(0);
  this->SetAndObserveMRMLInteractionNode(0);
}

//-----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::vtkInternal::DoDeleteCallback(vtkObject* vtk_obj,
                                                                      unsigned long event,
                                                                      void* vtkNotUsed(client_data),
                                                                      void* vtkNotUsed(call_data))
{
  vtkMRMLAbstractDisplayableManager* self =
      vtkMRMLAbstractDisplayableManager::SafeDownCast(vtk_obj);
  assert(self);
  assert(event == vtkCommand::DeleteEvent);
#ifndef _DEBUG
  (void)event;
#endif

  self->RemoveMRMLObservers();
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::vtkInternal::DoRenderWindowInteractorCallback(
    vtkObject* vtk_obj, unsigned long event, void* client_data, void* vtkNotUsed(call_data))
{
  // ModifiedEvent is expected
  assert(event == vtkCommand::ModifiedEvent);
#ifndef _DEBUG
  (void)event;
#endif

  // vtkRenderWindowInteractor is expected to be source of the ModifiedEvent
  vtkRenderWindowInteractor * rwi = vtkRenderWindowInteractor::SafeDownCast(vtk_obj);
  assert(rwi);

  vtkMRMLAbstractDisplayableManager* self =
      reinterpret_cast<vtkMRMLAbstractDisplayableManager*>(client_data);
  assert(self);

  self->Internal->SetAndObserveInteractorStyle(rwi->GetInteractorStyle());
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::vtkInternal::
    SetAndObserveMRMLInteractionNode(vtkMRMLInteractionNode* newMRMLInteractionNode)
{
  if (this->MRMLInteractionNode == newMRMLInteractionNode)
    {
    return;
    }

  //std::cout << "SetAndObserveMRMLInteractionNode " << newMRMLInteractionNode << std::endl;

  // Remove existing interactionNode observer
  if (this->MRMLInteractionNode)
    {
    this->MRMLInteractionNode->RemoveObserver(this->MRMLInteractionNodeCallBackCommand);
    this->MRMLInteractionNode->UnRegister(this->External);
    }

  // Install observer
  if (newMRMLInteractionNode)
    {
    newMRMLInteractionNode->Register(this->External);
    newMRMLInteractionNode->AddObserver(
        vtkMRMLInteractionNode::InteractionModeChangedEvent,
        this->MRMLInteractionNodeCallBackCommand);
    }

  this->MRMLInteractionNode = newMRMLInteractionNode;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::vtkInternal::DoMRMLInteractionNodeCallback(
    vtkObject* vtk_obj, unsigned long event, void* client_data, void* vtkNotUsed(call_data))
{
  //std::cout << "DoMRMLInteractionNodeCallback " << event << std::endl;

  // ModifiedEvent is expected
  assert(event == vtkMRMLInteractionNode::InteractionModeChangedEvent);
#ifndef _DEBUG
  (void)event;
#endif

  // vtkMRMLInteractionNode is expected to be source of the ModifiedEvent
  assert(vtkMRMLInteractionNode::SafeDownCast(vtk_obj));
#ifndef _DEBUG
  (void)vtk_obj;
#endif

  vtkMRMLAbstractDisplayableManager* self =
      reinterpret_cast<vtkMRMLAbstractDisplayableManager*>(client_data);
  assert(self);

  self->Internal->UpdateInteractorStyle();
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::vtkInternal::SetAndObserveInteractorStyle(
    vtkInteractorObserver* newInteractorStyle)
{
  if (this->InteractorStyle == newInteractorStyle)
    {
    return;
    }

  // Remove existing interactor style observer
  if (this->InteractorStyle)
    {
    this->InteractorStyle->RemoveObserver(this->InteractorStyleCallBackCommand);
    this->InteractorStyle->UnRegister(this->External);
    }

  // Install observers
  if (newInteractorStyle)
    {
    newInteractorStyle->Register(this->External);
    for(size_t i=0; i < this->InteractorStyleObservableEvents.size(); ++i)
      {
      int eid = this->InteractorStyleObservableEvents[i];
      newInteractorStyle->AddObserver(eid, this->InteractorStyleCallBackCommand);
      }
    }

  this->InteractorStyle = newInteractorStyle;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::vtkInternal::DoInteractorStyleCallback(
    vtkObject* vtk_obj, unsigned long event, void* client_data, void* vtkNotUsed(call_data))
{
  // vtkInteractorStyle is expected to be source of the event
  assert(vtkInteractorStyle::SafeDownCast(vtk_obj));
#ifndef _DEBUG
  (void)vtk_obj;
#endif

  vtkMRMLAbstractDisplayableManager* self =
      reinterpret_cast<vtkMRMLAbstractDisplayableManager*>(client_data);
  assert(self);

  self->OnInteractorStyleEvent(event);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::vtkInternal::UpdateInteractorStyle()
{
  int currentInteractionMode = this->MRMLInteractionNode->GetCurrentInteractionMode();
  switch (currentInteractionMode)
    {
    case vtkMRMLInteractionNode::Place:
    case vtkMRMLInteractionNode::PickManipulate:
      this->SetAndObserveInteractorStyle(
          this->Renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle());
      break;
    default:
      this->SetAndObserveInteractorStyle(0);
    }
}

//----------------------------------------------------------------------------
// vtkMRMLAbstractDisplayableManager methods

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::vtkMRMLAbstractDisplayableManager()
{
  this->Internal = new vtkInternal(this);
  this->AddObserver(vtkCommand::DeleteEvent, this->Internal->DeleteCallBackCommand);
  // Default observable event associated with DisplayableNode
  this->AddMRMLDisplayableManagerEvent(vtkCommand::ModifiedEvent);
}

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::~vtkMRMLAbstractDisplayableManager()
{
  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->UnRegister(this);
    }
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerGroup * vtkMRMLAbstractDisplayableManager
::GetMRMLDisplayableManagerGroup()
{
  return this->Internal->DisplayableManagerGroup;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::CreateIfPossible()
{
  if (!this->GetMRMLDisplayableNode())
    {
    return;
    }
  if (!this->IsCreated())
    {
    assert(this->GetMRMLScene());
    assert(this->GetMRMLDisplayableNode());

    // Look for InteractionNode
    this->Internal->MRMLInteractionNode = vtkMRMLInteractionNode::SafeDownCast(
        this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
    if (!this->Internal->MRMLInteractionNode)
      {
      vtkWarningMacro( << "CreateIfPossible - MRMLScene does NOT contain any InteractionNode");
      }

    // Look for SelectionNode
    this->Internal->MRMLSelectionNode = vtkMRMLSelectionNode::SafeDownCast(
        this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
    if (!this->Internal->MRMLSelectionNode)
      {
      vtkWarningMacro( << "CreateIfPossible - MRMLScene does NOT contain any SelectionNode");
      }

    this->Create();
    this->Internal->Created = true;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager
::SetMRMLDisplayableManagerGroup(vtkMRMLDisplayableManagerGroup * group)
{
  // Sanity checks
  if (this->Internal->DisplayableManagerGroup == group)
    {
    return;
    }
  this->Internal->DisplayableManagerGroup = group;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetRenderer(vtkRenderer* newRenderer)
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

  this->AdditionnalInitializeStep();

  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractDisplayableManager::IsCreated()
{
  return this->Internal->Created;
}

//---------------------------------------------------------------------------
vtkRenderer * vtkMRMLAbstractDisplayableManager::GetRenderer()
{
  return this->Internal->Renderer;
}

//---------------------------------------------------------------------------
vtkRenderWindowInteractor * vtkMRMLAbstractDisplayableManager::GetInteractor()
{
  if (!this->Internal->Renderer || !this->Internal->Renderer->GetRenderWindow())
    {
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning Interactor address 0");
    return 0;
    }
  vtkDebugMacro("returning Internal->Renderer->GetRenderWindow()->GetInteractor() address "
                << this->Internal->Renderer->GetRenderWindow()->GetInteractor() );
  return this->Internal->Renderer->GetRenderWindow()->GetInteractor();
}

//---------------------------------------------------------------------------
vtkMRMLInteractionNode* vtkMRMLAbstractDisplayableManager::GetInteractionNode()
{
  return this->Internal->MRMLInteractionNode;
}

//---------------------------------------------------------------------------
vtkMRMLSelectionNode* vtkMRMLAbstractDisplayableManager::GetSelectionNode()
{
  return this->Internal->MRMLSelectionNode;
}

//---------------------------------------------------------------------------
vtkMRMLNode * vtkMRMLAbstractDisplayableManager::GetMRMLDisplayableNode()
{
  return this->Internal->MRMLDisplayableNode;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::ProcessMRMLEvents(
    vtkObject* caller, unsigned long event, void * callData)
{
  if (vtkMRMLScene::SafeDownCast(caller))
    {
    this->Superclass::ProcessMRMLEvents(caller, event, callData);
    }
  else if (vtkMRMLNode::SafeDownCast(caller))
    {
    assert(event == vtkCommand::ModifiedEvent);
    this->OnMRMLDisplayableNodeModifiedEvent(caller);
    }
  else
    {
    vtkErrorMacro(<< "ProcessMRMLEvents - Unknown caller:" << caller->GetClassName()
                  << " - event:" << event);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  VTK_CREATE(vtkIntArray, sceneEvents);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneAboutToBeImportedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneImportedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::AddMRMLDisplayableManagerEvent(int eventId)
{
  for(int i = 0; i < this->Internal->MRMLDisplayableNodeObservableEvents->GetSize(); ++i)
    {
    if (eventId == this->Internal->MRMLDisplayableNodeObservableEvents->GetValue(i))
      {
      vtkErrorMacro(<< "AddMRMLDisplayableManagerEvent - eventId:" << eventId
                    << " already added");
      return;
      }
    }
  this->Internal->MRMLDisplayableNodeObservableEvents->InsertNextValue(eventId);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetAndObserveMRMLDisplayableNode(
    vtkMRMLNode * newMRMLDisplayableNode)
{
  // Observe scene associated with the MRML DisplayableNode
  vtkMRMLScene * sceneToObserve = 0;
  if (newMRMLDisplayableNode)
    {
    sceneToObserve = newMRMLDisplayableNode->GetScene();

    // Observe InteractionNode
    vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast (
        sceneToObserve->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
    if (interactionNode)
      {
      // Observe MRML InteractionNode only if a valid MRML DisplayableNode is set
      this->Internal->SetAndObserveMRMLInteractionNode(newMRMLDisplayableNode ? interactionNode : 0);
      this->Internal->UpdateInteractorStyle();
      }
    else
      {
      vtkWarningMacro(<< "SetAndObserveMRMLDisplayableNode - "
                      "MRMLScene does NOT contain any InteractionNode");
      }
    }
  this->SetMRMLScene(sceneToObserve);
  vtkSetAndObserveMRMLNodeEventsMacro(this->Internal->MRMLDisplayableNode,
                                      newMRMLDisplayableNode,
                                      this->Internal->MRMLDisplayableNodeObservableEvents);
  this->SetUpdateFromMRMLRequested(true);
  this->CreateIfPossible();
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetUpdateFromMRMLRequested(bool requested)
{
  if (this->Internal->UpdateFromMRMLRequested == requested)
    {
    return;
    }

  this->Internal->UpdateFromMRMLRequested = requested;

  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::RequestRender()
{
  // TODO Add a mechanism to check if Rendering is disable

  if (this->Internal->UpdateFromMRMLRequested)
    {
    this->UpdateFromMRML();
    }

  this->InvokeEvent(vtkCommand::UpdateEvent);
  if (this->Internal->DisplayableManagerGroup)
    {
    this->Internal->DisplayableManagerGroup->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::RemoveMRMLObservers()
{
  this->SetAndObserveMRMLDisplayableNode(0);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::AddInteractorStyleObservableEvent(int eventid)
{
  if (this->Internal->InteractorStyle->HasObserver(eventid,
                                                   this->Internal->InteractorStyleCallBackCommand))
    {
    vtkWarningMacro(<< "AddInteractorStyleObservableEvent - eventid:"
                    << eventid << " is already observed !");
    return;
    }

  this->Internal->InteractorStyle->AddObserver(eventid,
                                               this->Internal->InteractorStyleCallBackCommand);

  this->Internal->InteractorStyleObservableEvents.push_back(eventid);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::RemoveInteractorStyleObservableEvent(int eventid)
{
  if (!this->Internal->InteractorStyle->HasObserver(eventid,
                                                   this->Internal->InteractorStyleCallBackCommand))
    {
    vtkWarningMacro(<< "RemoveInteractorStyleObservableEvent - eventid:"
                    << eventid << " is NOT observed !");
    return;
    }

  this->Internal->InteractorStyle->RemoveObservers(eventid,
                                                   this->Internal->InteractorStyleCallBackCommand);

  // Check if the DisplayableManager has already been registered
  std::vector<int>::iterator it = std::find(
      this->Internal->InteractorStyleObservableEvents.begin(),
      this->Internal->InteractorStyleObservableEvents.end(),
      eventid);

  assert(it != this->Internal->InteractorStyleObservableEvents.end());

  this->Internal->InteractorStyleObservableEvents.erase(it);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  std::cout << "OnInteractorStyleEvent:" << eventid <<  std::endl;
}
