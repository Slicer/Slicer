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

==============================================================================*/

#include "vtkMRMLViewInteractorStyle.h"

// MRML includes
#include "vtkMRMLAbstractDisplayableManager.h"
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLDisplayableManagerGroup.h"
#include "vtkMRMLInteractionEventData.h"

// VTK includes
#include "vtkCallbackCommand.h"
#include "vtkInteractorStyle.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLViewInteractorStyle);

//----------------------------------------------------------------------------
vtkMRMLViewInteractorStyle::vtkMRMLViewInteractorStyle()
{
  this->EventCallbackCommand = vtkCallbackCommand::New();
  this->EventCallbackCommand->SetClientData(this);
  this->EventCallbackCommand->SetCallback(vtkMRMLViewInteractorStyle::CustomProcessEvents);

  this->DisplayableManagerCallbackCommand = vtkCallbackCommand::New();
  this->DisplayableManagerCallbackCommand->SetClientData(this);
  this->DisplayableManagerCallbackCommand->SetCallback(vtkMRMLViewInteractorStyle::DisplayableManagerCallback);

  this->FocusedDisplayableManager = nullptr;
  this->MouseMovedSinceButtonDown = false;
}

//----------------------------------------------------------------------------
vtkMRMLViewInteractorStyle::~vtkMRMLViewInteractorStyle()
{
  this->EventCallbackCommand->Delete();

  if (this->DisplayableManagers)
  {
    int numberOfDisplayableManagers = this->DisplayableManagers->GetDisplayableManagerCount();
    for (int displayableManagerIndex = 0; displayableManagerIndex < numberOfDisplayableManagers;
         ++displayableManagerIndex)
    {
      vtkMRMLAbstractDisplayableManager* displayableManager =
        this->DisplayableManagers->GetNthDisplayableManager(displayableManagerIndex);
      displayableManager->RemoveObserver(this->DisplayableManagerCallbackCommand);
    }
  }
  this->DisplayableManagerCallbackCommand->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnKeyPress()
{
  this->GetInteractorStyle()->OnKeyPress();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnKeyRelease()
{
  this->GetInteractorStyle()->OnKeyRelease();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnChar()
{
  // Do not call this->GetInteractorStyle->OnChar(), because char OnChar events perform various
  // low-level operations on the actors (change their rendering style to wireframe, pick them,
  // change rendering mode to stereo, etc.), which would interfere with displayable managers.
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMouseMove()
{
  this->MouseMovedSinceButtonDown = true;
  this->GetInteractorStyle()->OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnRightButtonDoubleClick()
{
  this->GetInteractorStyle()->OnRightButtonDoubleClick();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnRightButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  this->GetInteractorStyle()->OnRightButtonDown();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnRightButtonUp()
{
  this->GetInteractorStyle()->OnRightButtonUp();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMiddleButtonDoubleClick()
{
  this->GetInteractorStyle()->OnMiddleButtonDoubleClick();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMiddleButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  this->GetInteractorStyle()->OnMiddleButtonDown();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMiddleButtonUp()
{
  this->GetInteractorStyle()->OnMiddleButtonUp();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnLeftButtonDoubleClick()
{
  this->GetInteractorStyle()->OnLeftButtonDoubleClick();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnLeftButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  this->GetInteractorStyle()->OnLeftButtonDown();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnLeftButtonUp()
{
  this->GetInteractorStyle()->OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnEnter()
{
  this->GetInteractorStyle()->OnEnter();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnLeave()
{
  this->GetInteractorStyle()->OnLeave();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMouseWheelForward()
{
  this->GetInteractorStyle()->OnMouseWheelForward();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMouseWheelBackward()
{
  this->GetInteractorStyle()->OnMouseWheelBackward();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnButton3D(vtkEventData* eventData)
{
  this->GetInteractorStyle()->OnButton3D(eventData);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMove3D(vtkEventData* eventData)
{
  this->GetInteractorStyle()->OnMove3D(eventData);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnExpose()
{
  this->GetInteractorStyle()->OnExpose();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnConfigure()
{
  this->GetInteractorStyle()->OnConfigure();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnStartPinch()
{
  this->GetInteractorStyle()->OnStartPinch();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnPinch()
{
  this->GetInteractorStyle()->OnPinch();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnEndPinch()
{
  this->GetInteractorStyle()->OnEndPinch();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnStartRotate()
{
  this->GetInteractorStyle()->OnStartRotate();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnRotate()
{
  this->GetInteractorStyle()->OnRotate();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnEndRotate()
{
  this->GetInteractorStyle()->OnEndRotate();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnStartPan()
{
  this->GetInteractorStyle()->OnStartPan();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnPan()
{
  this->GetInteractorStyle()->OnPan();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnEndPan()
{
  this->GetInteractorStyle()->OnEndPan();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnTap()
{
  this->GetInteractorStyle()->OnTap();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnLongTap()
{
  this->GetInteractorStyle()->OnLongTap();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::SetDisplayableManagers(vtkMRMLDisplayableManagerGroup* displayableManagerGroup)
{
  this->DisplayableManagers = displayableManagerGroup;
}

//----------------------------------------------------------------------------
bool vtkMRMLViewInteractorStyle::DelegateInteractionEventToDisplayableManagers(unsigned long event)
{
  vtkNew<vtkMRMLInteractionEventData> ed;
  ed->SetType(event);

  bool delegated = this->DelegateInteractionEventToDisplayableManagers(ed);
  if (delegated)
  {
    this->EventCallbackCommand->SetAbortFlag(1);
  }
  return delegated;
}

//----------------------------------------------------------------------------
bool vtkMRMLViewInteractorStyle::DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData)
{
  if (!inputEventData)
  {
    return false;
  }
  int* displayPositionInt = this->GetInteractor()->GetEventPosition();
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(displayPositionInt[0], displayPositionInt[1]);
  if (!pokedRenderer)
  {
    // can happen during application shutdown
    return false;
  }

  vtkNew<vtkMRMLInteractionEventData> ed;
  ed->SetType(inputEventData ? inputEventData->GetType() : vtkCommand::NoEvent);
  int displayPositionCorrected[2] = { displayPositionInt[0] - pokedRenderer->GetOrigin()[0],
                                      displayPositionInt[1] - pokedRenderer->GetOrigin()[1] };
  ed->SetDisplayPosition(displayPositionCorrected);
  ed->SetMouseMovedSinceButtonDown(this->MouseMovedSinceButtonDown);
  ed->SetAttributesFromInteractor(this->GetInteractor());
  vtkEventDataDevice3D* inputEventDataDevice3D = inputEventData->GetAsEventDataDevice3D();
  if (inputEventDataDevice3D)
  {
    ed->SetDevice(inputEventDataDevice3D->GetDevice());
    ed->SetWorldPosition(inputEventDataDevice3D->GetWorldPosition());
    ed->SetWorldOrientation(inputEventDataDevice3D->GetWorldOrientation());
    ed->SetWorldDirection(inputEventDataDevice3D->GetWorldDirection());
    ed->SetInput(inputEventDataDevice3D->GetInput());
    ed->SetAction(inputEventDataDevice3D->GetAction());
  }

  return this->DelegateInteractionEventDataToDisplayableManagers(ed);
}

//----------------------------------------------------------------------------
bool vtkMRMLViewInteractorStyle::DelegateInteractionEventDataToDisplayableManagers(
  vtkMRMLInteractionEventData* eventData)
{
  if (!this->DisplayableManagers)
  {
    // this->SetMouseCursor(VTK_CURSOR_DEFAULT);
    return false;
  }
  if (eventData->GetType() == vtkCommand::Button3DEvent || eventData->GetType() == vtkCommand::Move3DEvent)
  {
    // Invalidate display position if 3D event
    eventData->SetDisplayPositionInvalid();
  }

  bool canProcessEvent = false;
  double closestDistance2 = VTK_DOUBLE_MAX;
  vtkMRMLAbstractDisplayableManager* closestDisplayableManager = nullptr;
  int numberOfDisplayableManagers = this->DisplayableManagers->GetDisplayableManagerCount();

  // Find the most suitable displayable manager
  for (int displayableManagerIndex = 0; displayableManagerIndex < numberOfDisplayableManagers;
       ++displayableManagerIndex)
  {
    vtkMRMLAbstractDisplayableManager* displayableManager = vtkMRMLAbstractDisplayableManager::SafeDownCast(
      this->DisplayableManagers->GetNthDisplayableManager(displayableManagerIndex));
    if (!displayableManager)
    {
      continue;
    }
    double distance2 = VTK_DOUBLE_MAX;
    if (displayableManager->CanProcessInteractionEvent(eventData, distance2))
    {
      if (!canProcessEvent || (distance2 < closestDistance2))
      {
        canProcessEvent = true;
        closestDisplayableManager = displayableManager;
        closestDistance2 = distance2;
      }
    }
  }

  if (!canProcessEvent)
  {
    // None of the displayable managers can process the event, just ignore it.
    // If click events (non-keyboard events) cannot be processed here then
    // indicate this by setting the mouse cursor to default.
    if (eventData->GetType() != vtkCommand::KeyPressEvent && eventData->GetType() != vtkCommand::KeyReleaseEvent)
    {
      this->DisplayableManagers->GetNthDisplayableManager(0)->SetMouseCursor(VTK_CURSOR_DEFAULT);
    }
    return false;
  }

  // Notify displayable managers about focus change
  vtkMRMLAbstractDisplayableManager* oldFocusedDisplayableManager = this->FocusedDisplayableManager;
  if (oldFocusedDisplayableManager != closestDisplayableManager)
  {
    if (oldFocusedDisplayableManager != nullptr)
    {
      oldFocusedDisplayableManager->SetHasFocus(false, eventData);
    }
    this->FocusedDisplayableManager = closestDisplayableManager;
    if (closestDisplayableManager != nullptr)
    {
      closestDisplayableManager->SetHasFocus(true, eventData);
    }
  }

  // Process event with new displayable manager
  if (!this->FocusedDisplayableManager)
  {
    if (oldFocusedDisplayableManager)
    {
      oldFocusedDisplayableManager->SetMouseCursor(VTK_CURSOR_DEFAULT);
    }
    return false;
  }

  // This prevents desynchronized update of displayable managers during user interaction
  // (ie. slice intersection widget or segmentations lagging behind during slice translation)
  vtkMRMLApplicationLogic* appLogic = this->FocusedDisplayableManager->GetMRMLApplicationLogic();
  if (appLogic)
  {
    this->FocusedDisplayableManager->GetMRMLApplicationLogic()->PauseRender();
  }
  bool processed = this->FocusedDisplayableManager->ProcessInteractionEvent(eventData);
  int cursor = VTK_CURSOR_DEFAULT;
  if (processed)
  {
    cursor = this->FocusedDisplayableManager->GetMouseCursor();
  }
  this->FocusedDisplayableManager->SetMouseCursor(cursor);
  if (appLogic)
  {
    this->FocusedDisplayableManager->GetMRMLApplicationLogic()->ResumeRender();
  }
  return processed;
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::SetMouseCursor(int cursor)
{
  if (this->GetInteractor() && this->GetInteractor()->GetRenderWindow())
  {
    this->GetInteractor()->GetRenderWindow()->SetCurrentCursor(cursor);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::CustomProcessEvents(vtkObject* object,
                                                     unsigned long event,
                                                     void* clientdata,
                                                     void* calldata)
{
  vtkMRMLViewInteractorStyle* self = reinterpret_cast<vtkMRMLViewInteractorStyle*>(clientdata);

  // Save info for button click detection
  if (event == vtkCommand::LeftButtonPressEvent || event == vtkCommand::RightButtonPressEvent
      || event == vtkCommand::MiddleButtonPressEvent)
  {
    self->MouseMovedSinceButtonDown = false;
  }
  if (event == vtkCommand::MouseMoveEvent)
  {
    self->MouseMovedSinceButtonDown = true;
  }

  // Displayable managers add interactor style observers and those observers
  // replace callback method calls. We make sure here that displayable managers
  // get the chance to process the events first (except when we are in an
  // interaction state - such as zooming, panning, etc).

  if (/*self->GetInteractorStyle()->GetState() != VTKIS_NONE || */ !self->DelegateInteractionEventToDisplayableManagers(
        event)
      || self->GetInteractorStyle()->GetState() != VTKIS_NONE)
  {
    // Displayable managers did not processed it
    vtkMRMLViewInteractorStyle::ProcessEvents(object, event, clientdata, calldata);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::ProcessEvents(vtkObject* vtkNotUsed(object),
                                               unsigned long event,
                                               void* clientdata,
                                               void* calldata)
{
  vtkMRMLViewInteractorStyle* self = reinterpret_cast<vtkMRMLViewInteractorStyle*>(clientdata);

  switch (event)
  {
    /// Mouse functions
    case vtkCommand::MouseMoveEvent:
      self->OnMouseMove();
      break;
    case vtkCommand::RightButtonDoubleClickEvent:
      self->OnRightButtonDoubleClick();
      break;
    case vtkCommand::RightButtonPressEvent:
      self->OnRightButtonDown();
      break;
    case vtkCommand::RightButtonReleaseEvent:
      self->OnRightButtonUp();
      break;
    case vtkCommand::MiddleButtonDoubleClickEvent:
      self->OnMiddleButtonDoubleClick();
      break;
    case vtkCommand::MiddleButtonPressEvent:
      self->OnMiddleButtonDown();
      break;
    case vtkCommand::MiddleButtonReleaseEvent:
      self->OnMiddleButtonUp();
      break;
    case vtkCommand::LeftButtonDoubleClickEvent:
      self->OnLeftButtonDoubleClick();
      break;
    case vtkCommand::LeftButtonPressEvent:
      self->OnLeftButtonDown();
      break;
    case vtkCommand::LeftButtonReleaseEvent:
      self->OnLeftButtonUp();
      break;
    case vtkCommand::EnterEvent:
      self->OnEnter();
      break;
    case vtkCommand::LeaveEvent:
      self->OnLeave();
      break;
    case vtkCommand::MouseWheelForwardEvent:
      self->OnMouseWheelForward();
      break;
    case vtkCommand::MouseWheelBackwardEvent:
      self->OnMouseWheelBackward();
      break;

    // Touch gesture interaction events
    case vtkCommand::StartPinchEvent:
      self->OnStartPinch();
      break;
    case vtkCommand::PinchEvent:
      self->OnPinch();
      break;
    case vtkCommand::EndPinchEvent:
      self->OnEndPinch();
      break;
    case vtkCommand::StartRotateEvent:
      self->OnStartRotate();
      break;
    case vtkCommand::RotateEvent:
      self->OnRotate();
      break;
    case vtkCommand::EndRotateEvent:
      self->OnEndRotate();
      break;
    case vtkCommand::StartPanEvent:
      self->OnStartPan();
      break;
    case vtkCommand::PanEvent:
      self->OnPan();
      break;
    case vtkCommand::EndPanEvent:
      self->OnEndPan();
      break;
    case vtkCommand::TapEvent:
      self->OnTap();
      break;
    case vtkCommand::LongTapEvent:
      self->OnLongTap();
      break;

    /// Keyboard functions
    case vtkCommand::KeyPressEvent:
      self->OnConfigure();
      break;
    case vtkCommand::KeyReleaseEvent:
      self->OnKeyRelease();
      break;
    case vtkCommand::CharEvent:
      self->OnChar();
      break;

    /// 3D event bindings
    case vtkCommand::Button3DEvent:
      self->OnButton3D(static_cast<vtkEventData*>(calldata));
      break;
    case vtkCommand::Move3DEvent:
      self->OnMove3D(static_cast<vtkEventData*>(calldata));
      break;

    case vtkCommand::ExposeEvent:
      self->OnExpose();
      break;
    case vtkCommand::ConfigureEvent:
      self->OnConfigure();
      break;

    default:
      break;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::SetInteractor(vtkRenderWindowInteractor* interactor)
{
  if (interactor == this->Interactor)
  {
    return;
  }
  // if we already have an Interactor then stop observing it
  if (this->Interactor)
  {
    this->Interactor->RemoveObserver(this->EventCallbackCommand);
  }
  this->Interactor = interactor;

  if (interactor)
  {
    float priority = 0.0f;

    // Mouse
    interactor->AddObserver(vtkCommand::MouseMoveEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::RightButtonDoubleClickEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::RightButtonPressEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::RightButtonReleaseEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::MiddleButtonDoubleClickEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::MiddleButtonPressEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::MiddleButtonReleaseEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::LeftButtonDoubleClickEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::LeftButtonPressEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::LeftButtonReleaseEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::EnterEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::LeaveEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::MouseWheelForwardEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::MouseWheelBackwardEvent, this->EventCallbackCommand, priority);

    // Touch gesture
    interactor->AddObserver(vtkCommand::StartPinchEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::PinchEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::EndPinchEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::StartRotateEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::RotateEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::EndRotateEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::StartPanEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::PanEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::EndPanEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::TapEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::LongTapEvent, this->EventCallbackCommand, priority);

    // Keyboard
    interactor->AddObserver(vtkCommand::KeyPressEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::KeyReleaseEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::CharEvent, this->EventCallbackCommand, priority);

    // 3D event bindings
    interactor->AddObserver(vtkCommand::Button3DEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::Move3DEvent, this->EventCallbackCommand, priority);

    interactor->AddObserver(vtkCommand::ExposeEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::ConfigureEvent, this->EventCallbackCommand, priority);
  }
}

//----------------------------------------------------------------------------
vtkInteractorStyle* vtkMRMLViewInteractorStyle::GetInteractorStyle()
{
  return vtkInteractorStyle::SafeDownCast(this->GetInteractor()->GetInteractorStyle());
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::DisplayableManagerCallback(vtkObject* object,
                                                            unsigned long event,
                                                            void* clientData,
                                                            void* callData)
{
  vtkMRMLViewInteractorStyle* self = reinterpret_cast<vtkMRMLViewInteractorStyle*>(clientData);
  assert(object->IsA("vtkMRMLAbstractDisplayableManager"));
  self->ProcessDisplayableManagerEvents(vtkMRMLAbstractDisplayableManager::SafeDownCast(object), event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::ProcessDisplayableManagerEvents(
  vtkMRMLAbstractDisplayableManager* vtkNotUsed(displayableManager),
  unsigned long vtkNotUsed(event),
  void* vtkNotUsed(callData))
{
}
