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

#include "vtkMRMLCameraWidget.h"

// VTK includes
#include "vtkCamera.h"
#include "vtkEvent.h"
#include "vtkInteractorStyle.h"
#include "vtkPlane.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkTransform.h"

/// MRML includes
#include "vtkMRMLCrosshairDisplayableManager.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLLayoutNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"

vtkStandardNewMacro(vtkMRMLCameraWidget);

//----------------------------------------------------------------------------------
vtkMRMLCameraWidget::vtkMRMLCameraWidget()
{
  this->MotionFactor = 10.0;
  this->MouseWheelMotionFactor = 1.0;
  this->CameraTiltLocked = false;

  this->StartEventPosition[0] = 0.0;
  this->StartEventPosition[1] = 0.0;

  this->PreviousEventPosition[0] = 0;
  this->PreviousEventPosition[1] = 0;

  this->ModifierKeyPressedSinceLastClickAndDrag = true;

  // Rotate camera to anatomic directions

  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "KP_1", WidgetEventCameraRotateToAnterior);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "End", WidgetEventCameraRotateToAnterior);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "KP_1", WidgetEventCameraRotateToPosterior);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "End", WidgetEventCameraRotateToPosterior);

  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "KP_3", WidgetEventCameraRotateToLeft);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Next", WidgetEventCameraRotateToLeft); // PageDown
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "KP_3", WidgetEventCameraRotateToRight);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "Next", WidgetEventCameraRotateToRight);

  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "KP_7", WidgetEventCameraRotateToSuperior);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Home", WidgetEventCameraRotateToSuperior);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "KP_7", WidgetEventCameraRotateToInferior);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "Home", WidgetEventCameraRotateToInferior);

  // Rotate camera by small increments

  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "KP_2", WidgetEventCameraRotateCwX);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Down", WidgetEventCameraRotateCwX);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "KP_8", WidgetEventCameraRotateCcwX);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Up", WidgetEventCameraRotateCcwX);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "KP_4", WidgetEventCameraRotateCwZ);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Left", WidgetEventCameraRotateCwZ);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "KP_6", WidgetEventCameraRotateCcwZ);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Right", WidgetEventCameraRotateCcwZ);

  // Translate camera by small increments

  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "KP_2", WidgetEventCameraTranslateBackwardY);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "Down", WidgetEventCameraTranslateBackwardY);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "KP_8", WidgetEventCameraTranslateForwardY);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "Up", WidgetEventCameraTranslateForwardY);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "KP_4", WidgetEventCameraTranslateBackwardX);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "Left", WidgetEventCameraTranslateBackwardX);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "KP_6", WidgetEventCameraTranslateForwardX);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "Right", WidgetEventCameraTranslateForwardX);

  // Camera zoom

  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "plus", WidgetEventCameraZoomIn);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "minus", WidgetEventCameraZoomOut);

  // Toggle tiltLock
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ControlModifier, 0, 0, "b", WidgetEventToggleCameraTiltLock);

  // Reset camera

  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "KP_0", WidgetEventCameraReset);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Insert", WidgetEventCameraReset);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "KP_5", WidgetEventCameraResetRotation);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Clear", WidgetEventCameraResetRotation);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "KP_5", WidgetEventCameraResetTranslation);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::ShiftModifier, 0, 0, "Clear", WidgetEventCameraResetTranslation);
  this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "r", WidgetEventCameraResetFieldOfView);

  // Rotate
  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateRotate, WidgetEventRotateStart, WidgetEventRotateEnd);

  // Pan
  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::ShiftModifier,
    WidgetStateTranslate, WidgetEventTranslateStart, WidgetEventTranslateEnd);
  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::MiddleButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateTranslate, WidgetEventTranslateStart, WidgetEventTranslateEnd);
  // Touch translate
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::StartPanEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureStart);
  this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::PanEvent, vtkEvent::AnyModifier, WidgetEventTouchPanTranslate);
  this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::EndPanEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureEnd);

  // Dolly
  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::ShiftModifier + vtkEvent::ControlModifier,
    WidgetStateScale, WidgetEventScaleStart, WidgetEventScaleEnd);
  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateScale, WidgetEventScaleStart, WidgetEventScaleEnd);
  // Mousewheel zooms direction is chosen to be consistent with other applications (Ctrl-MouseWheelForward zooms in in PowerPoint),
  // and with Windows touchpad pinch-to-zoom (pinch to zoom in generates MouseWheelForward event).
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelForwardEvent, vtkEvent::NoModifier, WidgetEventCameraWheelZoomIn);
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelBackwardEvent, vtkEvent::NoModifier, WidgetEventCameraWheelZoomOut);
  // Ctrl-mousewheel (Windows touchpad pinch-to-zoom)
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelForwardEvent, vtkEvent::ControlModifier, WidgetEventCameraWheelZoomIn);
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelBackwardEvent, vtkEvent::ControlModifier, WidgetEventCameraWheelZoomOut);
  // Touch zoom
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::StartPinchEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureStart);
  this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::PinchEvent, vtkEvent::AnyModifier, WidgetEventTouchPinchZoom);
  this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::EndPinchEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureEnd);

  // Spin
  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::ControlModifier,
    WidgetStateSpin, WidgetEventSpinStart, WidgetEventSpinEnd);
  // Touch rotate
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::StartRotateEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureStart);
  this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::RotateEvent, vtkEvent::AnyModifier, WidgetEventTouchSpinCamera);
  this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::EndRotateEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureEnd);

  // Set cursor position
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseMoveEvent, vtkEvent::ShiftModifier, WidgetEventSetCrosshairPositionBackground);

  // Context menu
  this->SetEventTranslation(WidgetStateIdle, vtkMRMLInteractionEventData::RightButtonClickEvent, vtkEvent::NoModifier, WidgetEventMenu);

  // Maximize/restore view
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::NoModifier, WidgetEventMaximizeView);
}

//----------------------------------------------------------------------------------
vtkMRMLCameraWidget::~vtkMRMLCameraWidget()
{
  this->SetMRMLApplicationLogic(nullptr);
}

//----------------------------------------------------------------------
void vtkMRMLCameraWidget::CreateDefaultRepresentation()
{
  // This widget has no visible representation
  return;
}

//-----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2)
{
  if (eventData->GetType() == vtkCommand::LeaveEvent)
    {
    // We cannot capture keypress events until the user clicks in the view
    // so when we are outside then we should assume that modifier
    // is not just "stuck".
    this->ModifierKeyPressedSinceLastClickAndDrag = true;
    }
  if (eventData->GetType() == vtkCommand::KeyPressEvent)
    {
    if (eventData->GetKeySym().find("Shift") != std::string::npos)
      {
      this->ModifierKeyPressedSinceLastClickAndDrag = true;
      }
    }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
    {
    // If this event is not recognized then give a chance to process it as a click event.
    return this->CanProcessButtonClickEvent(eventData, distance2);
    }

  // If we are currently dragging a point then we interact everywhere
  if (this->WidgetState == WidgetStateTranslate
    || this->WidgetState == WidgetStateRotate
    || this->WidgetState == WidgetStateScale
    || this->WidgetState == WidgetStateMoveCrosshair
    || this->WidgetState == WidgetStateSpin)
    {
    distance2 = 0.0;
    return true;
    }

  // By processing the SetCrosshairPosition action at this point, rather than in ProcessInteractionEvent,
  // we allow other widgets to perform actions at the same time.
  // For example, this allows markup preview to remain visible in place mode while adjusting slice position
  // with shift + mouse-move.
  if (widgetEvent == WidgetEventSetCrosshairPositionBackground)
    {
    this->ProcessSetCrosshairBackground(eventData);
    }

  distance2 = 1e10; // we can process this event but we let more specific widgets to claim it (if they are closer)
  return true;
}

//-----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (!this->CameraNode)
    {
    return false;
    }
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = true;

  switch (widgetEvent)
    {
    case WidgetEventCameraRotateToAnterior:
      this->SaveStateForUndo();
      this->CameraNode->RotateTo(vtkMRMLCameraNode::Anterior);
      break;
    case WidgetEventCameraRotateToPosterior:
      this->SaveStateForUndo();
      this->CameraNode->RotateTo(vtkMRMLCameraNode::Posterior);
      break;
    case WidgetEventCameraRotateToRight:
      this->SaveStateForUndo();
      this->CameraNode->RotateTo(vtkMRMLCameraNode::Right);
      break;
    case WidgetEventCameraRotateToLeft:
      this->SaveStateForUndo();
      this->CameraNode->RotateTo(vtkMRMLCameraNode::Left);
      break;
    case WidgetEventCameraRotateToSuperior:
      this->SaveStateForUndo();
      this->CameraNode->RotateTo(vtkMRMLCameraNode::Superior);
      break;
    case WidgetEventCameraRotateToInferior:
      this->SaveStateForUndo();
      this->CameraNode->RotateTo(vtkMRMLCameraNode::Inferior);
      break;

    case WidgetEventCameraTranslateBackwardX:
      this->SaveStateForUndo();
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::X, true); // screen X is towards left
      break;
    case WidgetEventCameraTranslateForwardX:
      this->SaveStateForUndo();
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::X, false); // screen X is towards left
      break;
    case WidgetEventCameraTranslateBackwardY:
      this->SaveStateForUndo();
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::Y, false);
      break;
    case WidgetEventCameraTranslateForwardY:
      this->SaveStateForUndo();
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::Y, true);
      break;
    case WidgetEventCameraTranslateBackwardZ:
      this->SaveStateForUndo();
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::Z, false);
      break;
    case WidgetEventCameraTranslateForwardZ:
      this->SaveStateForUndo();
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::Z, true);
      break;

    case WidgetEventCameraRotateCcwX:
      this->SaveStateForUndo();
      this->CameraNode->RotateAround(vtkMRMLCameraNode::R, false);
      break;
    case WidgetEventCameraRotateCwX:
      this->SaveStateForUndo();
      this->CameraNode->RotateAround(vtkMRMLCameraNode::R, true);
      break;
    case WidgetEventCameraRotateCcwY:
      this->SaveStateForUndo();
      this->CameraNode->RotateAround(vtkMRMLCameraNode::A, false);
      break;
    case WidgetEventCameraRotateCwY:
      this->SaveStateForUndo();
      this->CameraNode->RotateAround(vtkMRMLCameraNode::A, true);
      break;
    case WidgetEventCameraRotateCcwZ:
      this->SaveStateForUndo();
      this->CameraNode->RotateAround(vtkMRMLCameraNode::S, false);
      break;
    case WidgetEventCameraRotateCwZ:
      this->SaveStateForUndo();
      this->CameraNode->RotateAround(vtkMRMLCameraNode::S, true);
      break;

    case WidgetEventToggleCameraTiltLock:
        this->CameraTiltLocked = !this->CameraTiltLocked;
        break;

    case WidgetEventCameraReset:
      this->SaveStateForUndo();
      this->CameraNode->Reset(true, true, true, this->Renderer);
      break;
    case WidgetEventCameraResetRotation:
      this->SaveStateForUndo();
      this->CameraNode->Reset(true, false, false, this->Renderer);
      break;
    case WidgetEventCameraResetTranslation:
      this->SaveStateForUndo();
      this->CameraNode->Reset(false, true, false, this->Renderer);
      break;
    case WidgetEventCameraResetFieldOfView:
      this->SaveStateForUndo();
      if (this->Renderer != nullptr)
        {
        this->Renderer->ResetCamera();
        }
      break;
    case WidgetEventCameraZoomIn:
      this->SaveStateForUndo();
      this->Dolly(1.2);
      break;
    case WidgetEventCameraZoomOut:
      this->SaveStateForUndo();
      this->Dolly(0.8);
      break;

    case WidgetEventCameraWheelZoomIn:
      this->SaveStateForUndo();
      this->Dolly(pow((double)1.1, 0.2 * this->MotionFactor * this->MouseWheelMotionFactor));
      break;
    case WidgetEventCameraWheelZoomOut:
      this->SaveStateForUndo();
      this->Dolly(pow((double)1.1, -0.2 * this->MotionFactor * this->MouseWheelMotionFactor));
      break;

    case WidgetEventMouseMove:
      // click-and-dragging the mouse cursor
      processedEvent = this->ProcessMouseMove(eventData);
      break;

    case WidgetEventTranslateStart:
      this->SetWidgetState(WidgetStateTranslate);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventTranslateEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventRotateStart:
      this->SetWidgetState(WidgetStateRotate);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventRotateEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventScaleStart:
      this->SetWidgetState(WidgetStateScale);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventScaleEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventMoveCrosshairStart:
      this->SetWidgetState(WidgetStateMoveCrosshair);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventMoveCrosshairEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventSpinStart:
      this->SetWidgetState(WidgetStateSpin);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventSpinEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventTouchGestureStart:
      this->ProcessTouchGestureStart(eventData);
      break;
    case WidgetEventTouchGestureEnd:
      this->ProcessTouchGestureEnd(eventData);
      break;
    case WidgetEventTouchSpinCamera:
      this->ProcessTouchCameraSpin(eventData);
      break;
    case WidgetEventTouchPinchZoom:
      this->ProcessTouchCameraZoom(eventData);
      break;
    case WidgetEventTouchPanTranslate:
      this->ProcessTouchCameraTranslate(eventData);
      break;
    case WidgetEventMenu:
      processedEvent = this->ProcessWidgetMenu(eventData);
      break;

    case WidgetEventSetCrosshairPosition:
      this->ProcessSetCrosshair(eventData);
      break;

    case WidgetEventMaximizeView:
      processedEvent = this->ProcessMaximizeView(eventData);
      break;

    default:
      processedEvent = false;
    }

  if (!processedEvent)
    {
    processedEvent = this->ProcessButtonClickEvent(eventData);
    }

  if (processedEvent)
    {
    // invoke interaction event for compatibility with pre-camera-widget
    // behavior of vtk event processing.  This enables events to pass
    // through the qMRMLThreeDView to the cameraNode
    // for broadcast to other cameras
    vtkRenderWindowInteractor* interactor = this->Renderer->GetRenderWindow()->GetInteractor();
    interactor->InvokeEvent(vtkCommand::InteractionEvent);
    }

  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  switch (this->WidgetState)
    {
    case WidgetStateRotate:
      this->ProcessRotate(eventData);
      break;
    case WidgetStateTranslate:
      this->ProcessTranslate(eventData);
      break;
    case WidgetStateScale:
      this->ProcessScale(eventData);
      break;
    case WidgetStateMoveCrosshair:
      this->ProcessSetCrosshair(eventData);
      break;
    case WidgetStateSpin:
      this->ProcessSpin(eventData);
      break;
    default:
      // not processed
      return false;
    }
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessStartMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  this->SaveStateForUndo();

  if (this->Renderer && this->Renderer->GetRenderWindow() && this->Renderer->GetRenderWindow()->GetInteractor())
    {
    vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->Renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle());
    if (interactorStyle)
      {
      // Put the interactor to interactive mode (we always use VTKIS_ROTATE state as it does not matter how exactl we manipulate
      // the camera). This changes the desired frame rate (so that for example volume rendering is performed at lower resolution)
      // and also invokes StartInteractionState (and later EndInteractionState) events, which allow performing operations
      // when interaction is completed.
      if (interactorStyle->GetState() != VTKIS_ROTATE)
        {
        interactorStyle->StartState(VTKIS_ROTATE);
        }
      }
    }

  const int* displayPos = eventData->GetDisplayPosition();

  this->StartEventPosition[0] = displayPos[0];
  this->StartEventPosition[1] = displayPos[1];

  this->PreviousEventPosition[0] = this->StartEventPosition[0];
  this->PreviousEventPosition[1] = this->StartEventPosition[1];

  this->ProcessMouseMove(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  if (this->Renderer && this->Renderer->GetRenderWindow() && this->Renderer->GetRenderWindow()->GetInteractor())
    {
    vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->Renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle());
    if (interactorStyle)
      {
      if (interactorStyle->GetState() != VTKIS_NONE)
        {
        interactorStyle->StopState();
        }
      }
    }

  if (this->WidgetState == WidgetStateIdle)
    {
    return false;
    }
  this->SetWidgetState(WidgetStateIdle);

  // Prevent shift+mousemove events after click-and-drag (until shift is pressed again)
  this->ModifierKeyPressedSinceLastClickAndDrag = false;

  // only claim this as processed if the mouse was moved (this lets the event interpreted as button click)
  bool processedEvent = eventData->GetMouseMovedSinceButtonDown();
  return processedEvent;
}

//----------------------------------------------------------------------------------
void vtkMRMLCameraWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------------
void vtkMRMLCameraWidget::SetCameraNode(vtkMRMLCameraNode* cameraNode)
{
  if (this->CameraNode == cameraNode)
    {
    // no change
    return;
    }
  this->CameraNode = cameraNode;
}

//----------------------------------------------------------------------------------
vtkMRMLCameraNode* vtkMRMLCameraWidget::GetCameraNode()
{
  return this->CameraNode;
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessSetCrosshairBackground(vtkMRMLInteractionEventData* eventData)
{
  if (!this->ModifierKeyPressedSinceLastClickAndDrag)
    {
    // this event was caused by a "stuck" modifier key
    return false;
    }
  return this->ProcessSetCrosshair(eventData);
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessSetCrosshair(vtkMRMLInteractionEventData* eventData)
{
  if (!this->GetCameraNode() || !this->GetCameraNode()->GetScene())
    {
    return false;
    }

  vtkMRMLScene* scene = this->GetCameraNode()->GetScene();
  vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
  if (!crosshairNode)
    {
    return false;
    }

  if (!crosshairNode->GetFastPick3D())
    {
    // Get accurate world position (by default FastPick3D is false
    // because fast picking from Z buffer of the renderer is inaccurate
    // especially when semi-transparent actors are visible)
    eventData->ComputeAccurateWorldPosition();
    }
  if (!eventData->IsWorldPositionValid())
    {
    return false;
    }
  double worldPos[3] = { 0.0 };
  eventData->GetWorldPosition(worldPos);

  crosshairNode->SetCrosshairRAS(worldPos);
  crosshairNode->SetCursorPositionRAS(worldPos);
  if (crosshairNode->GetCrosshairBehavior() != vtkMRMLCrosshairNode::NoAction)
    {
    // Try to get view group of the 3D view and jump only those slices.
    int viewGroup = -1; // jump all by default
    if (this->GetCameraNode() && this->GetCameraNode()->GetLayoutName())
      {
      vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(
        scene->GetSingletonNode(this->GetCameraNode()->GetLayoutName(), "vtkMRMLViewNode"));
      if (viewNode)
        {
        viewGroup = viewNode->GetViewGroup();
        }
      }
    int viewJumpSliceMode = vtkMRMLSliceNode::OffsetJumpSlice;
    if (crosshairNode->GetCrosshairBehavior() == vtkMRMLCrosshairNode::CenteredJumpSlice)
      {
      viewJumpSliceMode = vtkMRMLSliceNode::CenteredJumpSlice;
      }
    vtkMRMLSliceNode::JumpAllSlices(scene, worldPos[0], worldPos[1], worldPos[2], viewJumpSliceMode, viewGroup);
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessRotate(vtkMRMLInteractionEventData* eventData)
{
  if (!this->Renderer || !eventData)
    {
    return false;
    }

  const int* eventPosition = eventData->GetDisplayPosition();
  int dx = eventPosition[0] - this->PreviousEventPosition[0];
  int dy = eventPosition[1] - this->PreviousEventPosition[1];
  if (dx == 0 && dy == 0)
    {
    return true;
    }

  const int *size = this->Renderer->GetRenderWindow()->GetSize();

  double delta_elevation = -20.0 / size[1];
  double delta_azimuth = -20.0 / size[0];

  double rxf = (double)dx * delta_azimuth * this->MotionFactor;
  double ryf = (double)dy * delta_elevation * this->MotionFactor;

  vtkCamera* camera = this->GetCamera();
  if (!camera)
    {
    return false;
    }

  bool wasCameraNodeModified = this->CameraModifyStart();

  if (this->CameraTiltLocked == true)
    {
    camera->Azimuth(rxf);
    }
  else
    {
    camera->Azimuth(rxf);
    camera->Elevation(ryf);
    }
  camera->OrthogonalizeViewUp();

  this->CameraModifyEnd(wasCameraNodeModified, true, true);

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessSpin(vtkMRMLInteractionEventData* eventData)
{
  if (!this->Renderer || !eventData)
    {
    return false;
    }

  const int* eventPosition = eventData->GetDisplayPosition();
  int dx = eventPosition[0] - this->PreviousEventPosition[0];
  int dy = eventPosition[1] - this->PreviousEventPosition[1];
  if (dx == 0 && dy == 0)
    {
    return true;
    }

  double *center = this->Renderer->GetCenter();

  double newAngle =
    atan2((double)eventPosition[1] - (double)center[1],
          (double)eventPosition[0] - (double)center[0]);

  double oldAngle =
    atan2((double)this->PreviousEventPosition[1] - (double)center[1],
          (double)this->PreviousEventPosition[0] - (double)center[0]);

  newAngle = vtkMath::DegreesFromRadians(newAngle);
  oldAngle = vtkMath::DegreesFromRadians(oldAngle);
  vtkCamera *camera = this->GetCamera();
  if (!camera)
    {
    return false;
    }

  bool wasCameraNodeModified = this->CameraModifyStart();

  camera->Roll(newAngle - oldAngle);
  camera->OrthogonalizeViewUp();

  this->CameraModifyEnd(wasCameraNodeModified, true, true);

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessTranslate(vtkMRMLInteractionEventData* eventData)
{
  if (!this->Renderer || !eventData)
    {
    return false;
    }

  const int* eventPosition = eventData->GetDisplayPosition();
  int dx = eventPosition[0] - this->PreviousEventPosition[0];
  int dy = eventPosition[1] - this->PreviousEventPosition[1];
  if (dx == 0 && dy == 0)
    {
    return true;
    }

  // Calculate the focal depth since we'll be using it a lot
  vtkCamera* camera = this->GetCamera();
  if (!camera)
    {
    return false;
    }

  double viewFocus[4] = { 0.0, 0.0, 0.0, 1.0 };
  camera->GetFocalPoint(viewFocus);
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer,
    viewFocus[0], viewFocus[1], viewFocus[2], viewFocus);

  double focalDepth = viewFocus[2];

  double newPickPoint[4] = { 0.0, 0.0, 0.0, 1.0 };
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer,
    (double)eventPosition[0], (double)eventPosition[1], focalDepth, newPickPoint);

  // Has to recalc old mouse point since the viewport has moved,
  // so can't move it outside the loop

  double oldPickPoint[4] = { 0.0, 0.0, 0.0, 1.0 };
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer,
    (double)this->PreviousEventPosition[0], (double)this->PreviousEventPosition[1],
    focalDepth, oldPickPoint);

  // Camera motion is reversed

  double motionVector[4] =
    {
    oldPickPoint[0] - newPickPoint[0],
    oldPickPoint[1] - newPickPoint[1],
    oldPickPoint[2] - newPickPoint[2],
    0.0
    };

  // If model transform is applied to the camera (scale, shear, rotations and translations)
  // then we need to take that into account otherwise we will get
  // under/over estimated translations if camera is scaled for example
  vtkMatrix4x4* cameraModelTransformMatrix = camera->GetModelTransformMatrix();
  if (cameraModelTransformMatrix)
    {
    cameraModelTransformMatrix->MultiplyPoint(motionVector, motionVector);
    }

  camera->GetFocalPoint(viewFocus);

  double viewPoint[3] = { 0.0 };
  camera->GetPosition(viewPoint);

  bool wasCameraNodeModified = this->CameraModifyStart();

  camera->SetFocalPoint(motionVector[0] + viewFocus[0],
    motionVector[1] + viewFocus[1],
    motionVector[2] + viewFocus[2]);

  camera->SetPosition(motionVector[0] + viewPoint[0],
    motionVector[1] + viewPoint[1],
    motionVector[2] + viewPoint[2]);

  this->CameraModifyEnd(wasCameraNodeModified, true, true);

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessScale(vtkMRMLInteractionEventData* eventData)
{
  if (!this->Renderer || !eventData)
    {
    return false;
    }

  const int* eventPosition = eventData->GetDisplayPosition();
  int dx = eventPosition[0] - this->PreviousEventPosition[0];
  int dy = eventPosition[1] - this->PreviousEventPosition[1];
  if (dx == 0 && dy == 0)
    {
    return true;
    }

  double *center = this->Renderer->GetCenter();
  double dyf = this->MotionFactor * (double)(dy) / (double)(center[1]);
  // Slicer: pull mouse towards you to bring models closer (opposite of vtk)
  this->Dolly(pow((double)1.1, -1. * dyf));

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessTouchGestureStart(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  this->SetWidgetState(WidgetStateTouchGesture);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessTouchGestureEnd(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  this->SetWidgetState(WidgetStateIdle);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessTouchCameraSpin(vtkMRMLInteractionEventData* eventData)
{
  vtkCamera* camera = this->GetCamera();
  if (!camera)
    {
    return false;
    }

  bool wasCameraNodeModified = this->CameraModifyStart();

  int pinchPostionDisplay[2] = { 0,0 };
  eventData->GetDisplayPosition(pinchPostionDisplay);

  double oldWorldPosition[4] = { 0,0,0,0 };
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, pinchPostionDisplay[0], pinchPostionDisplay[1], 0, oldWorldPosition);

  camera->Roll(eventData->GetRotation() - eventData->GetLastRotation());
  camera->OrthogonalizeViewUp();

  double newWorldPosition[4] = { 0,0,0,0 };
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, pinchPostionDisplay[0], pinchPostionDisplay[1], 0, newWorldPosition);

  vtkNew<vtkPlane> plane;
  plane->SetOrigin(camera->GetFocalPoint());
  plane->SetNormal(camera->GetViewPlaneNormal());
  plane->ProjectPoint(newWorldPosition, newWorldPosition);
  plane->ProjectPoint(oldWorldPosition, oldWorldPosition);

  double deltaWorld[3] = { 0,0,0 };
  vtkMath::Subtract(oldWorldPosition, newWorldPosition, deltaWorld);

  vtkNew<vtkTransform> deltaWorldTransform;
  deltaWorldTransform->Identity();
  deltaWorldTransform->Translate(deltaWorld);
  camera->ApplyTransform(deltaWorldTransform);

  this->CameraModifyEnd(wasCameraNodeModified, false, true);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessTouchCameraZoom(vtkMRMLInteractionEventData* eventData)
{
  vtkCamera* camera = this->GetCamera();
  if (!camera)
    {
    return false;
    }
  if (!eventData)
    {
    return false;
    }

  bool wasCameraNodeModified = this->CameraModifyStart();

  int pinchPostionDisplay[3] = { 0,0,0 };
  eventData->GetDisplayPosition(pinchPostionDisplay);

  // Remember the position of the center of the pinch in world coordinates
  // This position should stay in the same location on the screen after the dolly has been performed
  double oldWorldPosition[4] = { 0,0,0,0 };
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, pinchPostionDisplay[0], pinchPostionDisplay[1], pinchPostionDisplay[2], oldWorldPosition);

  // Perform the zoom
  this->Dolly(eventData->GetScale() / eventData->GetLastScale());

  // New position at the center of the pinch gesture
  double newWorldPosition[4] = { 0,0,0,0 };
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, pinchPostionDisplay[0], pinchPostionDisplay[1], pinchPostionDisplay[2], newWorldPosition);

  vtkNew<vtkPlane> plane;
  plane->SetOrigin(camera->GetFocalPoint());
  plane->SetNormal(camera->GetViewPlaneNormal());
  plane->ProjectPoint(newWorldPosition, newWorldPosition);
  plane->ProjectPoint(oldWorldPosition, oldWorldPosition);

  // Determine how far the pinch center has been shifted from it's original location
  double deltaWorld[3] = { 0,0,0 };
  vtkMath::Subtract(oldWorldPosition, newWorldPosition, deltaWorld);

  // Translate the camera to compensate for the shift of the pinch center
  vtkNew<vtkTransform> deltaWorldTransform;
  deltaWorldTransform->Identity();
  deltaWorldTransform->Translate(deltaWorld);
  camera->ApplyTransform(deltaWorldTransform);

  this->CameraModifyEnd(wasCameraNodeModified, false, true);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessTouchCameraTranslate(vtkMRMLInteractionEventData* eventData)
{
  vtkCamera* camera = this->GetCamera();
  if (!this->Renderer || !eventData || !camera)
    {
    return false;
    }

  const double* translation = eventData->GetTranslation();
  double deltaView[3] = {-translation[0], -translation[1], 0.0};

  double worldFocus[4];
  camera->GetFocalPoint(worldFocus);
  double viewFocus[4];
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer,
                                               worldFocus[0],
                                               worldFocus[1],
                                               worldFocus[2],
                                               viewFocus);

  double newWorldFocus[4];
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer,
                                               viewFocus[0] + deltaView[0],
                                               viewFocus[1] + deltaView[1],
                                               viewFocus[2] + deltaView[2],
                                               newWorldFocus);

  double deltaWorld[4];
  vtkMath::Subtract(newWorldFocus, worldFocus, deltaWorld);

  vtkNew<vtkTransform> deltaWorldTransform;
  deltaWorldTransform->Identity();
  deltaWorldTransform->Translate(deltaWorld);
  camera->ApplyTransform(deltaWorldTransform);

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::Dolly(double factor)
{
  if (!this->Renderer)
    {
    return false;
    }

  vtkCamera* camera = this->GetCamera();
  if (!camera)
    {
    return false;
    }

  bool wasCameraNodeModified = this->CameraModifyStart();
  bool updateClippingRange = false;
  if (camera->GetParallelProjection())
    {
    camera->SetParallelScale(camera->GetParallelScale() / factor);
    if (this->GetCameraNode() && this->GetCameraNode()->GetScene())
      {
      vtkMRMLScene* scene = this->GetCameraNode()->GetScene();
      vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(
        scene->GetSingletonNode(this->GetCameraNode()->GetLayoutName(), "vtkMRMLViewNode"));
      if (viewNode)
        {
        viewNode->SetFieldOfView(camera->GetParallelScale());
        }
      }
    }
  else
    {
    camera->Dolly(factor);
    updateClippingRange = true;
    }
  this->CameraModifyEnd(wasCameraNodeModified, updateClippingRange, true);

  return true;
}

//----------------------------------------------------------------------------
vtkCamera* vtkMRMLCameraWidget::GetCamera()
{
  if (this->CameraNode)
    {
    return this->CameraNode->GetCamera();
    }
  if (!this->Renderer || !this->Renderer->IsActiveCameraCreated())
    {
    return nullptr;
    }
  return  this->Renderer->GetActiveCamera();
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::CameraModifyStart()
{
  if (this->CameraNode)
    {
    return this->CameraNode->StartModify();
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraWidget::CameraModifyEnd(bool wasModified, bool updateClippingRange, bool updateLights)
{
  if (this->Renderer && this->Renderer->GetRenderWindow())
    {
    vtkRenderWindowInteractor* interactor = this->Renderer->GetRenderWindow()->GetInteractor();
    if (interactor)
      {
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());
      if (updateClippingRange && interactorStyle && interactorStyle->GetAutoAdjustCameraClippingRange())
        {
        this->Renderer->ResetCameraClippingRange();
        }
      if (updateLights && interactor->GetLightFollowCamera())
        {
        this->Renderer->UpdateLightsGeometryToFollowCamera();
        }
      }
    }
  if (this->CameraNode)
    {
    this->CameraNode->EndModify(wasModified);
    }
}


//----------------------------------------------------------------------------
void vtkMRMLCameraWidget::SaveStateForUndo()
{
  if (!this->GetCameraNode() || !this->GetCameraNode()->GetScene())
    {
    return;
    }
  this->GetCameraNode()->GetScene()->SaveStateForUndo();
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != WidgetStateIdle)
    {
    return false;
    }
  if (!this->GetCameraNode() || !this->GetCameraNode()->GetScene())
    {
    return false;
    }
  // This widget has no representation, so we cannot use this->GetInteractionNode().
  vtkMRMLAbstractViewNode* viewNode = eventData->GetViewNode();
  if (!viewNode)
    {
    return false;
    }
  vtkMRMLInteractionNode* interactionNode = viewNode->GetInteractionNode();
  if (!interactionNode)
    {
    return false;
    }

  vtkNew<vtkMRMLInteractionEventData> pickEventData;
  pickEventData->SetType(vtkMRMLInteractionNode::ShowViewContextMenuEvent);
  pickEventData->SetViewNode(viewNode);
  if (pickEventData->IsDisplayPositionValid())
    {
    pickEventData->SetDisplayPosition(eventData->GetDisplayPosition());
    }
  interactionNode->ShowViewContextMenu(pickEventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLCameraWidget::ProcessMaximizeView(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != WidgetStateIdle)
    {
    return false;
    }
  vtkMRMLAbstractViewNode* viewNode = eventData->GetViewNode();
  if (!viewNode)
    {
    return false;
    }

  bool isMaximized = false;
  bool canBeMaximized = false;
  vtkMRMLLayoutNode* layoutNode = viewNode->GetMaximizedState(isMaximized, canBeMaximized);
  if (!layoutNode || !canBeMaximized)
    {
    return false;
    }
  if (isMaximized)
    {
    layoutNode->SetMaximizedViewNode(nullptr);
    }
  else
    {
    layoutNode->SetMaximizedViewNode(viewNode);
    }

  // Maximize/restore takes away the focus without resetting
  // this->ModifierKeyPressedSinceLastMouseButtonRelease
  // therefore we need to reset the flag here.
  this->ModifierKeyPressedSinceLastClickAndDrag = true;

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraWidget::GetTiltLocked()
{
  return this->CameraTiltLocked;
}

//----------------------------------------------------------------------------
void vtkMRMLCameraWidget::SetTiltLocked(bool lockState)
{
  this->CameraTiltLocked = lockState;
}
