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

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkInteractorStyle.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>
#include <algorithm>

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
  /// is one of the ones matching ActiveInteractionModes
  /// \note Since we want to keep the virtual method ProcessMRMLEvent of the base class pure,
  /// the pattern MRMLObserverManager/ProcessMRMLEvent is not used here.
  static void DoMRMLInteractionNodeCallback(vtkObject* vtk_obj, unsigned long event,
                                            void* client_data, void* call_data);

  /// Called after MRML DisplayableNode is set, it will add/remove interactor style observer
  /// according to the state of the current MRML InteractionNode
  /// \sa DoMRMLInteractionNodeCallback
  void UpdateInteractorStyle(int eventIdToObserve = vtkCommand::NoEvent,
                             int eventIdToUnObserve = vtkCommand::NoEvent);

  vtkMRMLAbstractDisplayableManager*        External;
  bool                                      Created;
  vtkObserverManager*                       WidgetsObserverManager;
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
  this->WidgetsObserverManager = vtkObserverManager::New();
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
  this->InteractorStyleObservableEvents.push_back(vtkCommand::EnterEvent);
  this->InteractorStyleObservableEvents.push_back(vtkCommand::LeaveEvent);
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetAndObserveInteractorStyle(0);
  this->SetAndObserveMRMLInteractionNode(0);

  this->WidgetsObserverManager->AssignOwner(0);
  this->WidgetsObserverManager->Delete();
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

  // InteractionModeChangedEvent is expected
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
void vtkMRMLAbstractDisplayableManager::vtkInternal::UpdateInteractorStyle(int eventIdToObserve, int eventIdToUnObserve)
{
  // TODO The following code could be factorized in shorter and simpler functions
  bool updateObserver = false;
  if (this->MRMLInteractionNode)
    {
    int currentInteractionMode =
      this->MRMLInteractionNode->GetCurrentInteractionMode();
    if ( currentInteractionMode & this->External->ActiveInteractionModes() )
      {
      this->SetAndObserveInteractorStyle(
          this->Renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle());
      updateObserver = (this->InteractorStyle != 0);
      }
    else
      {
      this->SetAndObserveInteractorStyle(0);
      }
    }

  // Update observe if it applies
  if (updateObserver)
    {
    if (eventIdToObserve != vtkCommand::NoEvent)
      {
      assert(!this->InteractorStyle->HasObserver(eventIdToObserve, this->InteractorStyleCallBackCommand));
      this->InteractorStyle->AddObserver(eventIdToObserve, this->InteractorStyleCallBackCommand);
      }
    if (eventIdToUnObserve != vtkCommand::NoEvent)
      {
      assert(this->InteractorStyle->HasObserver(eventIdToUnObserve, this->InteractorStyleCallBackCommand));
      this->InteractorStyle->RemoveObservers(eventIdToUnObserve, this->InteractorStyleCallBackCommand);
      }
    }

  // Update InteractorStyleObservableEvents vector
  if (eventIdToObserve != vtkCommand::NoEvent)
    {
    // Check if the ObservableEvent has already been registered
    std::vector<int>::iterator it = std::find(
        this->InteractorStyleObservableEvents.begin(),
        this->InteractorStyleObservableEvents.end(),
        eventIdToUnObserve);

    if (it != this->InteractorStyleObservableEvents.end())
      {
      vtkWarningWithObjectMacro(this->External, << "UpdateInteractorStyle - eventid:" << eventIdToUnObserve
                                << " has already been added to the list of observable events !");
      }
    else
      {
      this->InteractorStyleObservableEvents.push_back(eventIdToObserve);
      }
    }

  if (eventIdToUnObserve != vtkCommand::NoEvent)
    {
    // Check if the ObservableEvent has already been registered
    std::vector<int>::iterator it = std::find(
        this->InteractorStyleObservableEvents.begin(),
        this->InteractorStyleObservableEvents.end(),
        eventIdToUnObserve);

    if (it == this->InteractorStyleObservableEvents.end())
      {
      vtkWarningWithObjectMacro(this->External, << "UpdateInteractorStyle - eventid:" << eventIdToUnObserve
                                << " has already NOT been added to the list of observable events !");
      }
    else
      {
      this->InteractorStyleObservableEvents.erase(it);
      }
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

  // Setup widgets callback
  vtkObserverManager * widgetsObserver = this->Internal->WidgetsObserverManager;
  widgetsObserver->AssignOwner(this);
  widgetsObserver->GetCallbackCommand()->SetClientData(this);
  widgetsObserver->GetCallbackCommand()->SetCallback(
    vtkMRMLAbstractDisplayableManager::WidgetsCallback);
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
void vtkMRMLAbstractDisplayableManager::Create()
{
  this->ProcessMRMLNodesEvents(this->GetMRMLDisplayableNode(), vtkCommand::ModifiedEvent, 0);
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

  this->AdditionalInitializeStep();

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
int vtkMRMLAbstractDisplayableManager::ActiveInteractionModes() {
  return vtkMRMLInteractionNode::Place;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::ProcessMRMLNodesEvents(
  vtkObject* caller, unsigned long event, void * callData)
{
  if (caller == this->GetMRMLDisplayableNode() &&
      event == vtkCommand::ModifiedEvent)
    {
    this->OnMRMLDisplayableNodeModifiedEvent(caller);
    return;
    }
  this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager
::OnMRMLDisplayableNodeModifiedEvent(vtkObject* vtkNotUsed(caller))
{
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager
::ProcessWidgetsEvents(vtkObject *vtkNotUsed(caller),
                       unsigned long vtkNotUsed(event),
                       void *vtkNotUsed(callData))
{
}

//----------------------------------------------------------------------------
// Description:
// the WidgetCallback is a static function to relay modified events from the
// observed vtk widgets back into the mrml node for further processing
void vtkMRMLAbstractDisplayableManager::WidgetsCallback(vtkObject *caller,
                                                        unsigned long eid,
                                                        void *clientData,
                                                        void *callData)
{
  vtkMRMLAbstractDisplayableManager* self =
    reinterpret_cast<vtkMRMLAbstractDisplayableManager *>(clientData);
  assert(!caller->IsA("vtkMRMLNode"));
  self->ProcessWidgetsEvents(caller, eid, callData);
}

//----------------------------------------------------------------------------
vtkCallbackCommand* vtkMRMLAbstractDisplayableManager::GetWidgetsCallbackCommand()
{
  return this->GetWidgetsObserverManager()->GetCallbackCommand();
}

//----------------------------------------------------------------------------
vtkObserverManager* vtkMRMLAbstractDisplayableManager::GetWidgetsObserverManager()const
{
  return this->Internal->WidgetsObserverManager;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneAboutToBeImportedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneImportedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
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
  this->Internal->UpdateInteractorStyle(eventid, vtkCommand::NoEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::RemoveInteractorStyleObservableEvent(int eventid)
{
  this->Internal->UpdateInteractorStyle(vtkCommand::NoEvent, eventid);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::OnInteractorStyleEvent(int vtkNotUsed(eventid))
{
  // std::cout << "OnInteractorStyleEvent - eventid:" << eventid
  //           << ", eventname:" << vtkCommand::GetStringFromEventId(eventid) << std::endl;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetInteractorStyleAbortFlag(int f)
{
  this->Internal->InteractorStyleCallBackCommand->SetAbortFlag(f);
}

//---------------------------------------------------------------------------
int vtkMRMLAbstractDisplayableManager::GetInteractorStyleAbortFlag()
{
  return this->Internal->InteractorStyleCallBackCommand->GetAbortFlag();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::InteractorStyleAbortFlagOn()
{
  this->Internal->InteractorStyleCallBackCommand->AbortFlagOn();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::InteractorStyleAbortFlagOff()
{
  this->Internal->InteractorStyleCallBackCommand->AbortFlagOff();
}
