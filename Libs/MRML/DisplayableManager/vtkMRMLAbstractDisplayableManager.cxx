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
#include <vtkMRMLLightBoxRendererManagerProxy.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkInteractorStyle.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <cassert>
#include <algorithm>
#include <functional>

#if (_MSC_VER >= 1700 && _MSC_VER < 1800)
// Visual Studio 2012 moves bind1st to <functional>
#include <functional>
#endif

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractDisplayableManager);


struct EventEquals
{
  typedef int first_argument_type;
  typedef std::pair<int,float> second_argument_type;
  typedef bool result_type;
  bool operator()(const int& a, std::pair<int,float>& b) const
  { return a == b.first; }
};


//----------------------------------------------------------------------------
class vtkMRMLAbstractDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLAbstractDisplayableManager* external);
  ~vtkInternal();

  /// Called after vtkCommand::DeleteEvent is called on the DisplayableManager
  static void DoDeleteCallback(vtkObject* vtk_obj, unsigned long event,
                               void* client_data, void* call_data);

  /// Set and observe \a newInteractorStyle
  void SetAndObserveInteractor(vtkRenderWindowInteractor* newInteractor);

  /// Set and observe \a newInteractorStyle
  void SetAndObserveInteractorStyle(vtkInteractorObserver* newInteractorStyle);

  /// Called after one of the observable event is invoked
  static void DoInteractorCallback(vtkObject* vtk_obj, unsigned long event,
                                   void* client_data, void* call_data);

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
                             int eventIdToUnObserve = vtkCommand::NoEvent,
                             float priority=0.0);

  /// Called after MRML DisplayableNode is set, it will add/remove
  /// interactor observer
  /// according to the state of the current MRML InteractionNode
  /// \sa DoMRMLInteractionNodeCallback
  void UpdateInteractor(int eventIdToObserve = vtkCommand::NoEvent,
                        int eventIdToUnObserve = vtkCommand::NoEvent,
                        float priority=0.0);

  vtkMRMLAbstractDisplayableManager*        External;
  bool                                      Created;
  vtkObserverManager*                       WidgetsObserverManager;
  bool                                      UpdateFromMRMLRequested;
  vtkRenderer*                              Renderer;
  vtkMRMLNode*                              MRMLDisplayableNode;
  vtkSmartPointer<vtkIntArray>              MRMLDisplayableNodeObservableEvents;
  vtkMRMLInteractionNode*                   MRMLInteractionNode;
  vtkSmartPointer<vtkCallbackCommand>       MRMLInteractionNodeCallBackCommand;
  vtkMRMLDisplayableManagerGroup*           DisplayableManagerGroup;
  vtkSmartPointer<vtkCallbackCommand>       DeleteCallBackCommand;
  vtkRenderWindowInteractor*                Interactor;
  vtkSmartPointer<vtkCallbackCommand>       InteractorCallBackCommand;
  std::vector<std::pair<int,float> >        InteractorObservableEvents;
  vtkInteractorObserver*                    InteractorStyle;
  vtkSmartPointer<vtkCallbackCommand>       InteractorStyleCallBackCommand;
  std::vector<std::pair<int,float> >        InteractorStyleObservableEvents;
  vtkWeakPointer<vtkMRMLLightBoxRendererManagerProxy> LightBoxRendererManagerProxy;

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
  this->Renderer = nullptr;
  this->LightBoxRendererManagerProxy = nullptr;
  this->MRMLDisplayableNode = nullptr;
  this->MRMLDisplayableNodeObservableEvents = vtkSmartPointer<vtkIntArray>::New();
  this->DisplayableManagerGroup = nullptr;

  this->DeleteCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->DeleteCallBackCommand->SetCallback(
      vtkMRMLAbstractDisplayableManager::vtkInternal::DoDeleteCallback);

  this->Interactor = nullptr;
  this->InteractorCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->InteractorCallBackCommand->SetCallback(
      vtkMRMLAbstractDisplayableManager::vtkInternal::DoInteractorCallback);
  this->InteractorCallBackCommand->SetClientData(this->External);

  this->MRMLInteractionNode = nullptr;
  this->MRMLInteractionNodeCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->MRMLInteractionNodeCallBackCommand->SetCallback(
      vtkMRMLAbstractDisplayableManager::vtkInternal::DoMRMLInteractionNodeCallback);
  this->MRMLInteractionNodeCallBackCommand->SetClientData(this->External);

  this->InteractorStyle = nullptr;
  this->InteractorStyleCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->InteractorStyleCallBackCommand->SetCallback(
      vtkMRMLAbstractDisplayableManager::vtkInternal::DoInteractorStyleCallback);
  this->InteractorStyleCallBackCommand->SetClientData(this->External);

  // Default Interactor style events to observe
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::LeftButtonPressEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::LeftButtonReleaseEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::RightButtonPressEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::RightButtonReleaseEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::MiddleButtonPressEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::MiddleButtonReleaseEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::MouseWheelBackwardEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::MouseWheelForwardEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::EnterEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::LeaveEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::Button3DEvent,0.0);
  this->InteractorStyleObservableEvents.emplace_back(vtkCommand::Move3DEvent,0.0);
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetAndObserveInteractor(nullptr);
  this->SetAndObserveInteractorStyle(nullptr);
  this->SetAndObserveMRMLInteractionNode(nullptr);
  this->LightBoxRendererManagerProxy = nullptr;
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
void vtkMRMLAbstractDisplayableManager::vtkInternal::SetAndObserveInteractor(
    vtkRenderWindowInteractor* newInteractor)
{
  if (this->Interactor == newInteractor)
    {
    return;
    }

  // Remove existing interactor observer
  if (this->Interactor)
    {
    this->Interactor->RemoveObserver(this->InteractorCallBackCommand);
    this->Interactor->UnRegister(this->External);
    }

  // Install observers
  if (newInteractor)
    {
    newInteractor->Register(this->External);
    for(size_t i=0; i < this->InteractorObservableEvents.size(); ++i)
      {
      int eid = this->InteractorObservableEvents[i].first;
      float priority = this->InteractorObservableEvents[i].second;
      newInteractor->AddObserver(eid, this->InteractorCallBackCommand, priority);
      }
    }

  this->Interactor = newInteractor;
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

    this->InteractorStyle->UnRegister(this->External);
    }

  // Install observers
  if (newInteractorStyle)
    {
    newInteractorStyle->Register(this->External);
    }

  this->InteractorStyle = newInteractorStyle;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::vtkInternal::DoInteractorCallback(
    vtkObject* vtk_obj, unsigned long event, void* client_data, void* vtkNotUsed(call_data))
{
  // vtkInteractor is expected to be source of the event
  assert(vtkRenderWindowInteractor::SafeDownCast(vtk_obj));
#ifndef _DEBUG
  (void)vtk_obj;
#endif

  vtkMRMLAbstractDisplayableManager* self =
      reinterpret_cast<vtkMRMLAbstractDisplayableManager*>(client_data);
  assert(self);

  self->OnInteractorEvent(event);
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
void vtkMRMLAbstractDisplayableManager::vtkInternal::UpdateInteractorStyle(int eventIdToObserve, int eventIdToUnObserve, float priority)
{
  // TODO The following code could be factorized in shorter and simpler functions
  bool updateObserver = false;
  if (this->MRMLInteractionNode)
    {
    this->SetAndObserveInteractor( this->Renderer->GetRenderWindow()->GetInteractor());
    this->SetAndObserveInteractorStyle(
        this->Renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle());
    updateObserver = (this->InteractorStyle != nullptr);
    }

  // Update observe if it applies
  if (updateObserver)
    {
    if (eventIdToObserve != vtkCommand::NoEvent)
      {
      assert(!this->InteractorStyle->HasObserver(eventIdToObserve, this->InteractorStyleCallBackCommand));
      this->InteractorStyle->AddObserver(eventIdToObserve, this->InteractorStyleCallBackCommand, priority);
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
    std::vector<std::pair<int,float> >::iterator it = std::find_if(
        this->InteractorStyleObservableEvents.begin(),
        this->InteractorStyleObservableEvents.end(),
        std::bind1st(EventEquals(), eventIdToObserve));

    if (it != this->InteractorStyleObservableEvents.end())
      {
      vtkWarningWithObjectMacro(this->External, << "UpdateInteractorStyle - eventid:" << eventIdToObserve
                                << " has already been added to the list of observable events !");
      }
    else
      {
      this->InteractorStyleObservableEvents.emplace_back(eventIdToObserve,priority);
      }
    }

  if (eventIdToUnObserve != vtkCommand::NoEvent)
    {
    // Check if the ObservableEvent has already been registered
    std::vector<std::pair<int,float> >::iterator it = std::find_if(
        this->InteractorStyleObservableEvents.begin(),
        this->InteractorStyleObservableEvents.end(),
        std::bind1st(EventEquals(), eventIdToUnObserve));

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
void vtkMRMLAbstractDisplayableManager::vtkInternal::UpdateInteractor(int eventIdToObserve, int eventIdToUnObserve, float priority)
{
  // TODO The following code could be factorized in shorter and simpler functions
  bool updateObserver = false;
  if (this->MRMLInteractionNode)
    {
    int currentInteractionMode =
      this->MRMLInteractionNode->GetCurrentInteractionMode();
    if ( currentInteractionMode & this->External->ActiveInteractionModes() )
      {
      this->SetAndObserveInteractor(this->Renderer->GetRenderWindow()->GetInteractor());
      updateObserver = (this->Interactor != nullptr);
      }
    else
      {
      this->SetAndObserveInteractor(nullptr);
      }
    }

  // Update observe if it applies
  if (updateObserver)
    {
    if (eventIdToObserve != vtkCommand::NoEvent)
      {
      assert(!this->Interactor->HasObserver(eventIdToObserve, this->InteractorCallBackCommand));
      this->Interactor->AddObserver(eventIdToObserve, this->InteractorCallBackCommand, priority);
      }
    if (eventIdToUnObserve != vtkCommand::NoEvent)
      {
      assert(this->Interactor->HasObserver(eventIdToUnObserve, this->InteractorCallBackCommand));
      this->Interactor->RemoveObservers(eventIdToUnObserve, this->InteractorCallBackCommand);
      }
    }

  // Update InteractorObservableEvents vector
  if (eventIdToObserve != vtkCommand::NoEvent)
    {
    // Check if the ObservableEvent has already been registered
    std::vector<std::pair<int,float> >::iterator it = std::find_if(
        this->InteractorObservableEvents.begin(),
        this->InteractorObservableEvents.end(),
        std::bind1st(EventEquals(), eventIdToObserve));

    if (it != this->InteractorObservableEvents.end())
      {
      vtkWarningWithObjectMacro(this->External, << "UpdateInteractor - eventid:" << eventIdToObserve
                                << " has already been added to the list of observable events !");
      }
    else
      {
      this->InteractorObservableEvents.emplace_back(eventIdToObserve,priority);
      }
    }

  if (eventIdToUnObserve != vtkCommand::NoEvent)
    {
    // Check if the ObservableEvent has already been registered
    std::vector<std::pair<int,float> >::iterator it = std::find_if(
        this->InteractorObservableEvents.begin(),
        this->InteractorObservableEvents.end(),
        std::bind1st(EventEquals(), eventIdToUnObserve));

    if (it == this->InteractorObservableEvents.end())
      {
      vtkWarningWithObjectMacro(this->External, << "UpdateInteractor - eventid:" << eventIdToUnObserve
                                << " has already NOT been added to the list of observable events !");
      }
    else
      {
      this->InteractorObservableEvents.erase(it);
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
  this->AddMRMLDisplayableManagerEvent(vtkMRMLNode::ReferenceAddedEvent);
  this->AddMRMLDisplayableManagerEvent(vtkMRMLNode::ReferenceRemovedEvent);
  this->AddMRMLDisplayableManagerEvent(vtkMRMLNode::ReferenceModifiedEvent);

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
    if (!this->GetInteractionNode())
      {
      vtkWarningMacro( << "CreateIfPossible - MRMLScene does NOT contain any InteractionNode");
      }

    this->Create();
    this->Internal->Created = true;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::Create()
{
  this->ProcessMRMLNodesEvents(this->GetMRMLDisplayableNode(), vtkCommand::ModifiedEvent, nullptr);
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

  if (this->Internal->Renderer)
    {
    // Need to do this AFTER the call to AdditionalInitializeStep(), otherwise
    // the events registered in AfterInitializeStep() are not
    // observed.
    // Indeed, the method "AddInteractorObservableEvent" expects the MRMLInteractionNode
    // to be set to add the observer(s). MRMLInteractionNode being set after "SetRenderer" function
    // is completed, the method "SetAndObserveInteractor" that also setup observers has to be called
    // after the observable events are added.
    this->Internal->SetAndObserveInteractor(this->Internal->Renderer->GetRenderWindow()->GetInteractor());
    }

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

  if (!this->Internal->Interactor)
    {
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning Interactor address 0");
    return nullptr;
    }
  vtkDebugMacro("returning Internal->Interactor address "
                << this->Internal->Interactor );
  return this->Internal->Interactor;
}

//---------------------------------------------------------------------------
vtkMRMLInteractionNode* vtkMRMLAbstractDisplayableManager::GetInteractionNode()
{
  return this->Internal->MRMLInteractionNode;
}

//---------------------------------------------------------------------------
vtkMRMLSelectionNode* vtkMRMLAbstractDisplayableManager::GetSelectionNode()
{
  return vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
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
  if (caller == this->GetMRMLDisplayableNode())
    {
      if(event == vtkCommand::ModifiedEvent)
        {
        this->OnMRMLDisplayableNodeModifiedEvent(caller);
        return;
        }
      else if(event == vtkMRMLNode::ReferenceAddedEvent ||
              event == vtkMRMLNode::ReferenceRemovedEvent ||
              event == vtkMRMLNode::ReferenceModifiedEvent)
        {
        // Update interaction node
        vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(this->GetMRMLDisplayableNode());
        if (viewNode)
          {
          this->Internal->SetAndObserveMRMLInteractionNode(viewNode->GetInteractionNode());
          }
        else
          {
          vtkErrorMacro(<< "ProcessMRMLNodesEvents failed: "
                        << "No viewNode is associated with the displayable manager: " << this->GetClassName());
          }
        }
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
  assert(newScene != this->GetMRMLScene());

  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::StartBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::StartCloseEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::StartImportEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndImportEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::StartRestoreEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndRestoreEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::AddMRMLDisplayableManagerEvent(int eventId)
{
  for(int i = 0; i < this->Internal->MRMLDisplayableNodeObservableEvents->GetNumberOfValues(); ++i)
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
  vtkMRMLScene * sceneToObserve = nullptr;
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(newMRMLDisplayableNode);
  if (viewNode)
    {
    sceneToObserve = viewNode->GetScene();

    // Observe InteractionNode
    vtkMRMLInteractionNode *interactionNode = viewNode->GetInteractionNode();
    this->Internal->SetAndObserveMRMLInteractionNode(interactionNode);
    if (interactionNode)
      {
      this->Internal->UpdateInteractorStyle();
      }
    else
      {
      vtkWarningMacro(<< "SetAndObserveMRMLDisplayableNode - "
                      "MRMLScene does NOT contain any InteractionNode");
      }
    }
  vtkSetAndObserveMRMLNodeEventsMacro(this->Internal->MRMLDisplayableNode,
                                      viewNode,
                                      this->Internal->MRMLDisplayableNodeObservableEvents);
  this->SetMRMLScene(sceneToObserve);
  this->SetUpdateFromMRMLRequested(true);
  this->CreateIfPossible();
  if (viewNode != nullptr)
    {
    this->RequestRender();
    }
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
  this->SetAndObserveMRMLDisplayableNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::AddInteractorStyleObservableEvent(int eventid, float priority)
{
  this->Internal->UpdateInteractorStyle(eventid, vtkCommand::NoEvent, priority);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::RemoveInteractorStyleObservableEvent(int eventid)
{
  this->Internal->UpdateInteractorStyle(vtkCommand::NoEvent, eventid);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::AddInteractorObservableEvent(int eventid, float priority)
{
  this->Internal->UpdateInteractor(eventid, vtkCommand::NoEvent, priority);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::RemoveInteractorObservableEvent(int eventid)
{
  this->Internal->UpdateInteractor(vtkCommand::NoEvent, eventid);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::OnInteractorStyleEvent(int vtkNotUsed(eventid))
{
  // std::cout << "OnInteractorStyleEvent - eventid:" << eventid
  //           << ", eventname:" << vtkCommand::GetStringFromEventId(eventid) << std::endl;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::OnInteractorEvent(int vtkNotUsed(eventid))
{
  // std::cout << "OnInteractorEvent - eventid:" << eventid
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

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetInteractorAbortFlag(int f)
{
  this->Internal->InteractorCallBackCommand->SetAbortFlag(f);
}

//---------------------------------------------------------------------------
int vtkMRMLAbstractDisplayableManager::GetInteractorAbortFlag()
{
  return this->Internal->InteractorCallBackCommand->GetAbortFlag();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::InteractorAbortFlagOn()
{
  this->Internal->InteractorCallBackCommand->AbortFlagOn();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::InteractorAbortFlagOff()
{
  this->Internal->InteractorCallBackCommand->AbortFlagOff();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetLightBoxRendererManagerProxy(vtkMRMLLightBoxRendererManagerProxy* mgr)
{
  this->Internal->LightBoxRendererManagerProxy = mgr;
}

//---------------------------------------------------------------------------
vtkMRMLLightBoxRendererManagerProxy* vtkMRMLAbstractDisplayableManager::GetLightBoxRendererManagerProxy()
{
  return this->Internal->LightBoxRendererManagerProxy;
}

//---------------------------------------------------------------------------
vtkRenderer* vtkMRMLAbstractDisplayableManager::GetRenderer(int idx)
{
  if (this->Internal->LightBoxRendererManagerProxy)
    {
    return this->Internal->LightBoxRendererManagerProxy->GetRenderer(idx);
    }
  else
    {
    return this->GetRenderer();
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLAbstractDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* vtkNotUsed(eventData), double &distance2)
{
  distance2 = VTK_DOUBLE_MAX;
  return false;
}

//---------------------------------------------------------------------------
bool vtkMRMLAbstractDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetHasFocus(bool vtkNotUsed(hasFocus))
{
  return;
}

//---------------------------------------------------------------------------
bool vtkMRMLAbstractDisplayableManager::GetGrabFocus()
{
  return false;
}

//---------------------------------------------------------------------------
bool vtkMRMLAbstractDisplayableManager::GetInteractive()
{
  return false;
}

//---------------------------------------------------------------------------
int vtkMRMLAbstractDisplayableManager::GetMouseCursor()
{
  return VTK_CURSOR_DEFAULT;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetMouseCursor(int cursor)
{
  if (this->GetRenderer() && this->GetRenderer()->GetRenderWindow())
  {
    this->GetRenderer()->GetRenderWindow()->SetCurrentCursor(cursor);
  }
}
