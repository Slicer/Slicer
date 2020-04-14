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
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLDisplayableManagerGroup.h"
#include "vtkMRMLInteractionEventData.h"

// VTK includes
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLViewInteractorStyle);

//----------------------------------------------------------------------------
vtkMRMLViewInteractorStyle::vtkMRMLViewInteractorStyle()
{
  this->EventCallbackCommand->SetCallback(vtkMRMLViewInteractorStyle::CustomProcessEvents);

  this->FocusedDisplayableManager = nullptr;
  this->MouseMovedSinceButtonDown = false;
  this->NumberOfClicks = 0;
  this->DoubleClickIntervalTimeSec = 0.5;
}

//----------------------------------------------------------------------------
vtkMRMLViewInteractorStyle::~vtkMRMLViewInteractorStyle() = default;

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnKeyPress()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::KeyPressEvent))
    {
    return;
    }
  this->Superclass::OnKeyPress();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnKeyRelease()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::KeyReleaseEvent))
    {
    return;
    }
  this->Superclass::OnKeyRelease();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnChar()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::CharEvent))
    {
    return;
    }
  this->Superclass::OnChar();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMouseMove()
{
  this->MouseMovedSinceButtonDown = true;
  if (!this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MouseMoveEvent))
    {
    return;
    }
  this->Superclass::OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnRightButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::RightButtonPressEvent))
    {
    return;
    }
  this->InvokeEvent(vtkCommand::RightButtonPressEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnRightButtonUp()
{
  if (!this->DelegateInteractionEventToDisplayableManagers(vtkCommand::RightButtonReleaseEvent))
    {
    this->InvokeEvent(vtkCommand::RightButtonReleaseEvent, nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMiddleButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MiddleButtonPressEvent))
    {
    return;
    }
  this->InvokeEvent(vtkCommand::MiddleButtonPressEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMiddleButtonUp()
{
  if (!this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MiddleButtonReleaseEvent))
    {
    this->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnLeftButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::LeftButtonPressEvent))
    {
    return;
    }
  this->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnLeftButtonUp()
{
  if (!this->DelegateInteractionEventToDisplayableManagers(vtkCommand::LeftButtonReleaseEvent))
    {
    this->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnEnter()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::EnterEvent))
    {
    return;
    }
  this->Superclass::OnEnter();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnLeave()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::LeaveEvent))
    {
    return;
    }
  this->Superclass::OnLeave();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMouseWheelForward()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MouseWheelForwardEvent))
    {
    return;
    }
  this->Superclass::OnMouseWheelForward();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMouseWheelBackward()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::MouseWheelBackwardEvent))
    {
    return;
    }
  this->Superclass::OnMouseWheelBackward();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnButton3D(vtkEventData* eventData)
{
  if (this->DelegateInteractionEventToDisplayableManagers(eventData))
    {
    return;
    }
  this->InvokeEvent(eventData->GetType(), eventData);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnMove3D(vtkEventData* eventData)
{
  if (this->DelegateInteractionEventToDisplayableManagers(eventData))
    {
    return;
    }
  this->InvokeEvent(eventData->GetType(), eventData);
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnExpose()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::ExposeEvent))
    {
    return;
    }
  this->Superclass::OnExpose();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnConfigure()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::ConfigureEvent))
    {
    return;
    }
  this->Superclass::OnConfigure();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnPinch()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::PinchEvent))
    {
    return;
    }
  this->Superclass::OnPinch();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnRotate()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::RotateEvent))
    {
    return;
    }
  this->Superclass::OnRotate();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnPan()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::PanEvent))
    {
    return;
    }
  this->Superclass::OnPan();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnTap()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::TapEvent))
    {
    return;
    }
  this->Superclass::OnTap();
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::OnLongTap()
{
  if (this->DelegateInteractionEventToDisplayableManagers(vtkCommand::LongTapEvent))
    {
    return;
    }
  this->Superclass::OnLongTap();
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

  return this->DelegateInteractionEventToDisplayableManagers(ed);
}

//----------------------------------------------------------------------------
bool vtkMRMLViewInteractorStyle::DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData)
{
  int* displayPositionInt = this->GetInteractor()->GetEventPosition();
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(displayPositionInt[0], displayPositionInt[1]);

  vtkNew<vtkMRMLInteractionEventData> ed;
  ed->SetType(inputEventData ? inputEventData->GetType() : vtkCommand::NoEvent);
  int displayPositionCorrected[2] = { displayPositionInt[0] - pokedRenderer->GetOrigin()[0], displayPositionInt[1] - pokedRenderer->GetOrigin()[1] };
  ed->SetDisplayPosition(displayPositionCorrected);
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
bool vtkMRMLViewInteractorStyle::DelegateInteractionEventDataToDisplayableManagers(vtkMRMLInteractionEventData* eventData)
{
  if (!this->DisplayableManagers)
    {
    //this->SetMouseCursor(VTK_CURSOR_DEFAULT);
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
  for (int displayableManagerIndex = 0; displayableManagerIndex < numberOfDisplayableManagers; ++displayableManagerIndex)
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
    // none of the displayable managers can process the event, just ignore it
    //this->SetMouseCursor(VTK_CURSOR_DEFAULT);
    this->DisplayableManagers->GetNthDisplayableManager(0)->SetMouseCursor(VTK_CURSOR_DEFAULT);
    return false;
    }

  // Notify displayable managers about focus change
  vtkMRMLAbstractDisplayableManager* oldFocusedDisplayableManager = this->FocusedDisplayableManager;
  if (oldFocusedDisplayableManager != closestDisplayableManager)
    {
    if (oldFocusedDisplayableManager != nullptr)
      {
      oldFocusedDisplayableManager->SetHasFocus(false);
      }
    this->FocusedDisplayableManager = closestDisplayableManager;
    if (closestDisplayableManager != nullptr)
      {
      closestDisplayableManager->SetHasFocus(true);
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
  this->FocusedDisplayableManager->GetMRMLApplicationLogic()->PauseRender();
  bool processed = this->FocusedDisplayableManager->ProcessInteractionEvent(eventData);
  int cursor = VTK_CURSOR_DEFAULT;
  if (processed)
    {
    cursor = this->FocusedDisplayableManager->GetMouseCursor();
    }
  this->FocusedDisplayableManager->SetMouseCursor(cursor);
  this->FocusedDisplayableManager->GetMRMLApplicationLogic()->ResumeRender();
  return processed;
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::SetMouseCursor(int cursor)
{
  if (this->GetCurrentRenderer() && this->GetCurrentRenderer()->GetRenderWindow())
    {
    this->GetCurrentRenderer()->GetRenderWindow()->SetCurrentCursor(cursor);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::CustomProcessEvents(vtkObject* object,
  unsigned long event, void* clientdata, void* calldata)
{
  vtkMRMLViewInteractorStyle* self
    = reinterpret_cast<vtkMRMLViewInteractorStyle *>(clientdata);

  // Save info for button click detection
  if (event == vtkCommand::LeftButtonPressEvent
    || event == vtkCommand::RightButtonPressEvent
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
  if (self->State != VTKIS_NONE || !self->DelegateInteractionEventToDisplayableManagers(event))
    {
    // Displayable managers did not processed it
    Superclass::ProcessEvents(object, event, clientdata, calldata);
    }

  // VTK does not provide click events, detect them here
  if (!self->MouseMovedSinceButtonDown)
    {
    if (event == vtkCommand::LeftButtonReleaseEvent)
      {
      self->DelegateInteractionEventToDisplayableManagers(vtkMRMLInteractionEventData::LeftButtonClickEvent);
      }
    else if (event == vtkCommand::MiddleButtonReleaseEvent)
      {
      self->DelegateInteractionEventToDisplayableManagers(vtkMRMLInteractionEventData::MiddleButtonClickEvent);
      }
    else if (event == vtkCommand::RightButtonReleaseEvent)
      {
      self->DelegateInteractionEventToDisplayableManagers(vtkMRMLInteractionEventData::RightButtonClickEvent);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewInteractorStyle::SetInteractor(vtkRenderWindowInteractor *interactor)
{
  this->Superclass::SetInteractor(interactor);

  // There is no observer for double-click events in superclass, add them now
  if (this->Interactor)
    {
    this->Interactor->AddObserver(vtkCommand::LeftButtonDoubleClickEvent, this->EventCallbackCommand, this->Priority);
    this->Interactor->AddObserver(vtkCommand::MiddleButtonDoubleClickEvent, this->EventCallbackCommand, this->Priority);
    this->Interactor->AddObserver(vtkCommand::RightButtonDoubleClickEvent, this->EventCallbackCommand, this->Priority);
    this->Interactor->AddObserver(vtkCommand::StartPinchEvent, this->EventCallbackCommand, this->Priority);
    this->Interactor->AddObserver(vtkCommand::EndPinchEvent, this->EventCallbackCommand, this->Priority);
    this->Interactor->AddObserver(vtkCommand::StartRotateEvent, this->EventCallbackCommand, this->Priority);
    this->Interactor->AddObserver(vtkCommand::EndRotateEvent, this->EventCallbackCommand, this->Priority);
    this->Interactor->AddObserver(vtkCommand::StartPanEvent, this->EventCallbackCommand, this->Priority);
    this->Interactor->AddObserver(vtkCommand::EndPanEvent, this->EventCallbackCommand, this->Priority);
    }
}
