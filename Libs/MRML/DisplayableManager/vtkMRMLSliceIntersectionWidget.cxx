/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMRMLSliceIntersectionWidget.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMRMLSliceIntersectionWidget.h"

#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLCrosshairDisplayableManager.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLLayoutNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceDisplayNode.h"
#include "vtkMRMLSliceIntersectionInteractionRepresentation.h"
#include "vtkMRMLSliceIntersectionRepresentation2D.h"
#include "vtkMRMLSliceLayerLogic.h"
#include "vtkMRMLVolumeNode.h"

#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkEvent.h"
#include "vtkGeneralTransform.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkTransform.h"
#include "vtkWidgetEvent.h"

#include <deque>

vtkStandardNewMacro(vtkMRMLSliceIntersectionWidget);

//----------------------------------------------------------------------------------
vtkMRMLSliceIntersectionWidget::vtkMRMLSliceIntersectionWidget()
{
  this->StartEventPosition[0] = 0.0;
  this->StartEventPosition[1] = 0.0;

  this->PreviousRotationAngleRad = 0.0;

  this->PreviousEventPosition[0] = 0;
  this->PreviousEventPosition[1] = 0;

  this->StartRotationCenter[0] = 0.0;
  this->StartRotationCenter[1] = 0.0;

  this->StartRotationCenter_RAS[0] = 0.0;
  this->StartRotationCenter_RAS[1] = 0.0;
  this->StartRotationCenter_RAS[2] = 0.0;
  this->StartRotationCenter_RAS[3] = 1.0; // to allow easy homogeneous transformations

  // Interactive slice intersection widget

  this->StartTranslationPoint[0] = 0.0;
  this->StartTranslationPoint[1] = 0.0;

  this->StartTranslationPoint_RAS[0] = 0.0;
  this->StartTranslationPoint_RAS[1] = 0.0;
  this->StartTranslationPoint_RAS[2] = 0.0;

  this->CurrentTranslationPoint_RAS[0] = 0.0;
  this->CurrentTranslationPoint_RAS[1] = 0.0;
  this->CurrentTranslationPoint_RAS[2] = 0.0;

  this->StartActionFOV[0] = 0.;
  this->StartActionFOV[1] = 0.;
  this->StartActionFOV[2] = 0.;
  this->VolumeScalarRange[0] = 0;
  this->VolumeScalarRange[1] = 0;

  this->LastForegroundOpacity = 0.;
  this->LastLabelOpacity = 0.;
  this->StartActionSegmentationDisplayNode = nullptr;

  this->ModifierKeyPressedSinceLastClickAndDrag = true;

  this->TouchRotationThreshold = 10.0;
  this->TouchZoomThreshold = 0.1;
  this->TouchTranslationThreshold = 25.0;

  this->TotalTouchRotation = 0.0;
  this->TouchRotateEnabled = false;
  this->TotalTouchZoom = 0.0;
  this->TouchZoomEnabled = false;
  this->TotalTouchTranslation = 0.0;
  this->TouchTranslationEnabled = false;

  this->SliceModifiedCommand->SetClientData(this);
  this->SliceModifiedCommand->SetCallback(vtkMRMLSliceIntersectionWidget::SliceModifiedCallback);

  this->SliceLogicsModifiedCommand->SetClientData(this);
  this->SliceLogicsModifiedCommand->SetCallback(vtkMRMLSliceIntersectionWidget::SliceLogicsModifiedCallback);

  this->ActionsEnabled = ActionAll;
  this->UpdateInteractionEventMapping();
}

//----------------------------------------------------------------------------------
vtkMRMLSliceIntersectionWidget::~vtkMRMLSliceIntersectionWidget()
{
  this->SetSliceNode(nullptr);
  this->SetMRMLApplicationLogic(nullptr);
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::UpdateInteractionEventMapping()
{
  this->EventTranslators.clear();

  // Interactions without handles

  if (this->GetActionEnabled(ActionRotateSliceIntersection))
    {
    // Touch gesture slice rotate
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::StartRotateEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureStart);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::RotateEvent, vtkEvent::AnyModifier, WidgetEventTouchRotateSliceIntersection);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::EndRotateEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureEnd);
    // Ctrl-alt-left-click-and-drag rotate
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::AltModifier + vtkEvent::ControlModifier,
      WidgetStateRotateIntersectingSlices, WidgetEventRotateIntersectingSlicesStart, WidgetEventRotateIntersectingSlicesEnd);
    }
  if (this->GetActionEnabled(ActionTranslateSliceIntersection))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent,
      vtkEvent::AltModifier + vtkEvent::ShiftModifier, WidgetStateTranslate, WidgetEventTranslateStart, WidgetEventTranslateEnd);
    }
  if (this->GetActionEnabled(ActionSetCrosshairPosition))
    {
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseMoveEvent, vtkEvent::ShiftModifier, WidgetEventSetCrosshairPosition);
    }
  if (this->GetActionEnabled(ActionBlend))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent,
      vtkEvent::ControlModifier, WidgetStateBlend, WidgetEventBlendStart, WidgetEventBlendEnd);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "g", WidgetEventToggleLabelOpacity);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "t", WidgetEventToggleForegroundOpacity);
    }
  if (this->GetActionEnabled(ActionBrowseSlice))
    {
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Right", WidgetEventIncrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Left", WidgetEventDecrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Up", WidgetEventIncrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "Down", WidgetEventDecrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "f", WidgetEventIncrementSlice);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "b", WidgetEventDecrementSlice);
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelForwardEvent, vtkEvent::NoModifier, WidgetEventIncrementSlice);
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelBackwardEvent, vtkEvent::NoModifier, WidgetEventDecrementSlice);
    }
  if (this->GetActionEnabled(ActionShowSlice))
    {
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "v", WidgetEventToggleSliceVisibility);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "V", WidgetEventToggleAllSlicesVisibility);
    }
  if (this->GetActionEnabled(ActionSelectVolume))
    {
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "bracketleft", WidgetEventShowPreviousBackgroundVolume);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "bracketright", WidgetEventShowNextBackgroundVolume);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "braceleft", WidgetEventShowPreviousForegroundVolume);
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::AnyModifier, 0, 0, "braceright", WidgetEventShowNextForegroundVolume);
    }
  if (this->GetActionEnabled(ActionTranslate) && this->GetActionEnabled(ActionZoom))
    {
    this->SetKeyboardEventTranslation(WidgetStateIdle, vtkEvent::NoModifier, 0, 0, "r", WidgetEventResetFieldOfView);
    }
  if (this->GetActionEnabled(ActionZoom))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateZoomSlice, WidgetEventZoomSliceStart, WidgetEventZoomSliceEnd);
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelForwardEvent, vtkEvent::ControlModifier, WidgetEventZoomOutSlice);
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseWheelBackwardEvent, vtkEvent::ControlModifier, WidgetEventZoomInSlice);
    // Touch slice zoom
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::StartPinchEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureStart);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::PinchEvent, vtkEvent::AnyModifier, WidgetEventTouchZoomSlice);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::EndPinchEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureEnd);
    }

  if (this->GetActionEnabled(ActionTranslate))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::ShiftModifier,
      WidgetStateTranslateSlice, WidgetEventTranslateSliceStart, WidgetEventTranslateSliceEnd);
    this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::MiddleButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateTranslateSlice, WidgetEventTranslateSliceStart, WidgetEventTranslateSliceEnd);
    // Touch slice translate
    this->SetEventTranslation(WidgetStateIdle, vtkCommand::StartPanEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureStart);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::PanEvent, vtkEvent::AnyModifier, WidgetEventTouchTranslateSlice);
    this->SetEventTranslation(WidgetStateTouchGesture, vtkCommand::EndPanEvent, vtkEvent::AnyModifier, WidgetEventTouchGestureEnd);
    }

  // Context menu
  this->SetEventTranslation(WidgetStateAny, vtkMRMLInteractionEventData::RightButtonClickEvent, vtkEvent::NoModifier, WidgetEventMenu);

  // Maximize/restore view
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::NoModifier, WidgetEventMaximizeView);

  // Interactions with slice intersection handles

  // Update active component
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  // Update active interaction handle component
  this->SetEventTranslation(WidgetStateOnTranslateIntersectingSlicesHandle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnTranslateIntersectingSlicesHandle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnRotateIntersectingSlicesHandle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnRotateIntersectingSlicesHandle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnTranslateSingleIntersectingSliceHandle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnTranslateSingleIntersectingSliceHandle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);

  if (this->GetActionEnabled(ActionRotateSliceIntersection))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateOnRotateIntersectingSlicesHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateRotateIntersectingSlicesHandle, WidgetEventRotateIntersectingSlicesHandleStart, WidgetEventRotateIntersectingSlicesHandleEnd);
    }
  if (this->GetActionEnabled(ActionTranslateSliceIntersection))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateOnTranslateIntersectingSlicesHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateTranslateIntersectingSlicesHandle, WidgetEventTranslateIntersectingSlicesHandleStart, WidgetEventTranslateIntersectingSlicesHandleEnd);
    }
  if (this->GetActionEnabled(ActionTranslate))
    {
    this->SetEventTranslationClickAndDrag(WidgetStateOnTranslateSingleIntersectingSliceHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
      WidgetStateTranslateSingleIntersectingSliceHandle,
      WidgetEventTranslateSingleIntersectingSliceHandleStart, WidgetEventTranslateSingleIntersectingSliceHandleEnd);
    }
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::CreateDefaultRepresentation()
{
  if (this->WidgetRep)
    {
    // already created
    return;
    }
  vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    vtkWarningMacro("vtkMRMLSliceIntersectionWidget::CreateDefaultRepresentation failed: application logic invalid");
    return;
    }

  if (this->IsSliceIntersectionInteractive())
    {
    vtkNew<vtkMRMLSliceIntersectionInteractionRepresentation> newRepInteraction;
    newRepInteraction->SetMRMLApplicationLogic(mrmlAppLogic);
    newRepInteraction->SetRenderer(this->GetRenderer());
    newRepInteraction->SetSliceNode(this->SliceNode);
    this->WidgetRep = newRepInteraction;
    }
  else
    {
    vtkNew<vtkMRMLSliceIntersectionRepresentation2D> newRep;
    newRep->SetMRMLApplicationLogic(mrmlAppLogic);
    newRep->SetRenderer(this->GetRenderer());
    newRep->SetSliceNode(this->SliceNode);
    this->WidgetRep = newRep;
    }

}

//-----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2)
{
  if (!this->SliceLogic)
    {
    return false;
    }

  if (eventData->GetType() == vtkCommand::LeaveEvent)
    {
    // We cannot capture keypress events until the user clicks in the view
    // so when we are outside then we should assume that modifier
    // is not just "stuck".
    this->ModifierKeyPressedSinceLastClickAndDrag = true;

    if (this->GetSliceDisplayNode() && this->GetSliceDisplayNode()->HasActiveComponent())
      {
      // this widget has active component, therefore leave event is relevant
      distance2 = 0.0;
      return true;
      }
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
    /*TODO: check if needed
    if (eventData->GetType() == vtkCommand::MouseMoveEvent)
      {
      this->SetWidgetState(WidgetStateIdle);
      this->SliceLogic->EndSliceNodeInteraction();
      }
      */
    return false;
    }
  if (!this->GetRepresentation())
    {
    return false;
    }

  // If we are currently dragging a point then we interact everywhere
  if (this->WidgetState == WidgetStateTranslate
    || this->WidgetState == WidgetStateRotateIntersectingSlices
    || this->WidgetState == WidgetStateBlend
    || this->WidgetState == WidgetStateTranslateSlice
    || this->WidgetState == WidgetStateZoomSlice)
    {
    distance2 = 0.0;
    return true;
    }

  // By processing the SetCrosshairPosition action at this point, rather than in ProcessInteractionEvent,
  // we allow other widgets to perform actions at the same time.
  // For example, this allows markup preview to remain visible in place mode while adjusting slice position
  // with shift + mouse-move.
  if (this->GetActionEnabled(ActionSetCrosshairPosition)
    && this->WidgetState == WidgetStateIdle
    && eventData->GetType() == vtkCommand::MouseMoveEvent
    && eventData->GetModifiers() & vtkEvent::ShiftModifier)
    {
    this->ProcessSetCrosshair(eventData);
    }


  // Representation
  vtkMRMLSliceIntersectionInteractionRepresentation* rep = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->GetRepresentation());
  if (rep)
    {
    // Currently interacting
    if (this->WidgetState == WidgetStateTranslate
      || this->WidgetState == WidgetStateRotateIntersectingSlicesHandle
      || this->WidgetState == WidgetStateTranslateSingleIntersectingSliceHandle)
      {
      distance2 = 0.0;
      return true;
      }

    // Interaction
    int foundComponentType = vtkMRMLSliceDisplayNode::ComponentNone;
    int foundComponentIndex = -1;
    double closestDistance2 = 0.0;
    double handleOpacity = 0.0;
    std::string intersectingSliceNodeID = rep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2, handleOpacity);

    if (foundComponentType != vtkMRMLSliceDisplayNode::ComponentNone)
      {
      // Store closest squared distance
      distance2 = closestDistance2;

      // Store last intersecting slice node index
      this->LastIntersectingSliceNodeID = intersectingSliceNodeID;
      return true;
      }
    }

  distance2 = 1e10; // we can process this event but we let more specific widgets to claim it (if they are closer)
  return true;
}

//-----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (!this->SliceLogic)
    {
    return false;
    }

  if (eventData->GetType() == vtkCommand::LeaveEvent)
    {
    // If a widget component was still active when the mouse left the view then this will deactivate it.
    this->Leave(eventData);
    return true;
    }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = true;

  switch (widgetEvent)
    {
    case WidgetEventMouseMove:
      // click-and-dragging the mouse cursor
      processedEvent = this->ProcessMouseMove(eventData);
      break;
    case  WidgetEventTouchGestureStart:
      this->ProcessTouchGestureStart(eventData);
      break;
    case WidgetEventTouchGestureEnd:
      this->ProcessTouchGestureEnd(eventData);
      break;
    case WidgetEventTouchRotateSliceIntersection:
      this->ProcessTouchRotate(eventData);
      break;
    case WidgetEventTouchZoomSlice:
      this->ProcessTouchZoom(eventData);
      break;
    case WidgetEventTouchTranslateSlice:
      this->ProcessTouchTranslate(eventData);
      break;
    case WidgetEventTranslateStart:
      this->SetWidgetState(WidgetStateTranslate);
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventTranslateEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventRotateIntersectingSlicesStart:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      processedEvent = this->ProcessRotateIntersectingSlicesStart(eventData);
      break;
    case WidgetEventRotateIntersectingSlicesEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventTranslateSliceStart:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::XYZOriginFlag);
      this->SetWidgetState(WidgetStateTranslateSlice);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventTranslateSliceEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      this->SliceLogic->EndSliceNodeInteraction();
      break;
    case WidgetEventZoomSliceStart:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->SetWidgetState(WidgetStateZoomSlice);
      this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::FieldOfViewFlag);
      this->SliceLogic->GetSliceNode()->GetFieldOfView(this->StartActionFOV);
      processedEvent = this->ProcessStartMouseDrag(eventData);
      break;
    case WidgetEventZoomSliceEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      this->SliceLogic->EndSliceNodeInteraction();
      break;
    case WidgetEventBlendStart:
      {
      this->SetWidgetState(WidgetStateBlend);
      vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->LastForegroundOpacity = sliceCompositeNode->GetForegroundOpacity();
      this->LastLabelOpacity = this->GetLabelOpacity();
      this->StartActionSegmentationDisplayNode = this->GetVisibleSegmentationDisplayNode();
      processedEvent = this->ProcessStartMouseDrag(eventData);
      }
      break;
    case WidgetEventBlendEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventSetCrosshairPosition:
      // Event is handled in CanProcessInteractionEvent
      break;
    case WidgetEventToggleLabelOpacity:
      {
      this->StartActionSegmentationDisplayNode = nullptr;
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->StartActionSegmentationDisplayNode = nullptr;
      double opacity = this->GetLabelOpacity();
      if (opacity != 0.0)
        {
        this->LastLabelOpacity = opacity;
        this->SetLabelOpacity(0.0);
        }
      else
        {
        this->SetLabelOpacity(this->LastLabelOpacity);
        }
      }
      break;
    case WidgetEventToggleForegroundOpacity:
      {
      vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      double opacity = sliceCompositeNode->GetForegroundOpacity();
      if (opacity != 0.0)
        {
        this->LastForegroundOpacity = opacity;
        sliceCompositeNode->SetForegroundOpacity(0.0);
        }
      else
        {
        sliceCompositeNode->SetForegroundOpacity(this->LastForegroundOpacity);
        }
      }
      break;
    case WidgetEventIncrementSlice:
      this->IncrementSlice();
      break;
    case WidgetEventDecrementSlice:
      this->DecrementSlice();
      break;
    case WidgetEventToggleSliceVisibility:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->GetSliceNode()->SetSliceVisible(!this->GetSliceNode()->GetSliceVisible());
      break;
    case WidgetEventToggleAllSlicesVisibility:
      // TODO: need to set all slices visible
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->GetSliceNode()->SetSliceVisible(!this->GetSliceNode()->GetSliceVisible());
      break;
    case WidgetEventResetFieldOfView:
      {
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::ResetFieldOfViewFlag);
      this->SliceLogic->FitSliceToAll();
      this->GetSliceNode()->UpdateMatrices();
      this->SliceLogic->EndSliceNodeInteraction();
      }
      break;
    case WidgetEventZoomInSlice:
      this->ScaleZoom(1.2, eventData);
      break;
    case WidgetEventZoomOutSlice:
      this->ScaleZoom(0.8, eventData);
      break;
    break;
    case WidgetEventShowPreviousBackgroundVolume:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->CycleVolumeLayer(LayerBackground, -1);
      break;
    case WidgetEventShowNextBackgroundVolume:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->CycleVolumeLayer(LayerBackground, 1);
      break;
    case WidgetEventShowPreviousForegroundVolume:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->CycleVolumeLayer(LayerForeground, -1);
      break;
    case WidgetEventShowNextForegroundVolume:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->CycleVolumeLayer(LayerForeground, 1);
      break;
    case WidgetEventMenu:
      processedEvent = this->ProcessWidgetMenu(eventData);
      break;
    case WidgetEventMaximizeView:
      processedEvent = this->ProcessMaximizeView(eventData);
      break;

    case WidgetEventRotateIntersectingSlicesHandleStart:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      this->SetWidgetState(WidgetStateRotateIntersectingSlicesHandle);
      processedEvent = this->ProcessRotateIntersectingSlicesHandleStart(eventData);
      break;
    case WidgetEventRotateIntersectingSlicesHandleEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventTranslateIntersectingSlicesHandleStart:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      processedEvent = this->ProcessTranslateIntersectingSlicesHandleStart(eventData);
      break;
    case WidgetEventTranslateIntersectingSlicesHandleEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventTranslateSingleIntersectingSliceHandleStart:
      this->SliceLogic->GetMRMLScene()->SaveStateForUndo();
      processedEvent = this->ProcessTranslateSingleIntersectingSliceHandleStart(eventData);
      break;
    case WidgetEventTranslateSingleIntersectingSliceHandleEnd:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      break;

    default:
      processedEvent = false;
    }

  if (!processedEvent)
    {
    processedEvent = this->ProcessButtonClickEvent(eventData);
    }

  return processedEvent;
}

//-------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::Leave(vtkMRMLInteractionEventData* eventData)
{
  /*
  // Ensure that EndInteractionEvent is invoked, even if interrupted by an unexpected event
  if (this->WidgetState == vtkSlicerMarkupsWidget::WidgetStateTranslateControlPoint
    || this->WidgetState == vtkSlicerMarkupsWidget::WidgetStateTranslate
    || this->WidgetState == vtkSlicerMarkupsWidget::WidgetStateScale
    || this->WidgetState == vtkSlicerMarkupsWidget::WidgetStateRotate)
    {
    this->EndWidgetInteraction();
    }
    */

  vtkMRMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
  if (sliceDisplayNode)
    {
    std::string interactionContext;
    if (eventData)
      {
      interactionContext = eventData->GetInteractionContextName();
      }
    sliceDisplayNode->SetActiveComponent(vtkMRMLSliceDisplayNode::ComponentNone, -1, interactionContext);
    }
  this->Superclass::Leave(eventData);
}

//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceIntersectionInteractionRepresentation* interactiveRep = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!this->WidgetRep || !eventData)
    {
    return false;
    }

  switch (this->WidgetState)
    {
    case WidgetStateRotateIntersectingSlices:
      this->ProcessRotateIntersectingSlices(eventData);
      break;
    case WidgetStateTranslate:
      {
      const double* worldPos = eventData->GetWorldPosition();
      this->GetSliceNode()->JumpAllSlices(worldPos[0], worldPos[1], worldPos[2]);
      }
      break;
    case WidgetStateBlend:
      this->ProcessBlend(eventData);
      break;
    case WidgetStateTranslateSlice:
      this->ProcessTranslateSlice(eventData);
      break;
    case WidgetStateZoomSlice:
      this->ProcessZoomSlice(eventData);
      break;
    case WidgetStateIdle: // if moving over an intersection in idle mode then this will change the widget state to WidgetStateOn...
    case WidgetStateOnWidget:
    case WidgetStateOnTranslateIntersectingSlicesHandle:
    case WidgetStateOnRotateIntersectingSlicesHandle:
    case WidgetStateOnTranslateSingleIntersectingSliceHandle:
      if (interactiveRep)
        {
        // Update widget state according to distance to interaction handles
        int foundComponentType = vtkMRMLSliceDisplayNode::ComponentNone;
        int foundComponentIndex = -1;
        double closestDistance2 = 0.0;
        double handleOpacity = 0.0;
        std::string intersectingSliceNodeID = interactiveRep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2, handleOpacity);
        if (foundComponentType == vtkMRMLSliceDisplayNode::ComponentNone)
          {
          this->SetWidgetState(WidgetStateIdle);
          this->SliceLogic->EndSliceNodeInteraction();
          }
        else if (foundComponentType == vtkMRMLSliceDisplayNode::ComponentTranslateIntersectingSlicesHandle)
          {
          this->SetWidgetState(WidgetStateOnTranslateIntersectingSlicesHandle);
          }
        else if (foundComponentType == vtkMRMLSliceDisplayNode::ComponentRotateIntersectingSlicesHandle)
          {
          this->SetWidgetState(WidgetStateOnRotateIntersectingSlicesHandle);
          }
        else if (foundComponentType == vtkMRMLSliceDisplayNode::ComponentTranslateSingleIntersectingSliceHandle)
          {
          this->SetWidgetState(WidgetStateOnTranslateSingleIntersectingSliceHandle);
          }
        else
          {
          this->SetWidgetState(WidgetStateOnWidget);
          }

        // Store last intersecting slice node index
        if (foundComponentType != vtkMRMLSliceDisplayNode::ComponentNone)
          {
          this->LastIntersectingSliceNodeID = intersectingSliceNodeID;
          }

        vtkMRMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
        if (sliceDisplayNode)
          {
          sliceDisplayNode->SetActiveComponent(foundComponentType, foundComponentIndex, eventData->GetInteractionContextName());
          }
        }
        break;
    case WidgetStateTranslateIntersectingSlicesHandle:
    case WidgetStateTranslateSingleIntersectingSliceHandle:
    case WidgetStateRotateIntersectingSlicesHandle:
      {
      // Process the motion
      // Based on the displacement vector (computed in display coordinates) and
      // the cursor state (which corresponds to which part of the widget has been
      // selected), the widget points are modified.
      // First construct a local coordinate system based on the display coordinates
      // of the widget.
      double eventPos[2]
        {
        static_cast<double>(eventData->GetDisplayPosition()[0]),
        static_cast<double>(eventData->GetDisplayPosition()[1]),
        };
      bool inSliceView = interactiveRep->IsMouseCursorInSliceView(eventPos);

      // Make handles disappear during interaction
      vtkMRMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
      if (sliceDisplayNode)
        {
        sliceDisplayNode->SetActiveComponent(vtkMRMLSliceDisplayNode::ComponentSliceIntersection, 0, eventData->GetInteractionContextName());
        }

      if (this->WidgetState == WidgetStateTranslateIntersectingSlicesHandle)
        {
        if (inSliceView) // Stop interaction if mouse cursor is outside of the slice view
          {
          const double* worldPos = eventData->GetWorldPosition();
          this->GetSliceNode()->JumpAllSlices(worldPos[0], worldPos[1], worldPos[2]);
          }
        }
      else if (this->WidgetState == WidgetStateTranslateSingleIntersectingSliceHandle)
        {
        if (inSliceView) // Stop interaction if mouse cursor is outside of the slice view
          {
          this->ProcessTranslateSingleIntersectingSliceHandle(eventData);
          }
        }
      else if (this->WidgetState == WidgetStateRotateIntersectingSlicesHandle)
        {
        this->ProcessRotateIntersectingSlicesHandle(eventData);
        }
      }
    }

  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessStartMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
    {
    return false;
    }

  const int* displayPos = eventData->GetDisplayPosition();

  this->StartEventPosition[0] = displayPos[0];
  this->StartEventPosition[1] = displayPos[1];

  this->PreviousEventPosition[0] = this->StartEventPosition[0];
  this->PreviousEventPosition[1] = this->StartEventPosition[1];

  this->ProcessMouseMove(eventData);

  // Indicate interaction in the slice node to make behavior similar to the 3D reformat widget
  this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::MultiplanarReformatFlag);

  return true;
}

//----------------------------------------------------------------------------
double vtkMRMLSliceIntersectionWidget::GetSliceRotationAngleRad(double eventPos[2])
{
  return atan2(eventPos[1] - this->StartRotationCenter[1],
    eventPos[0] - this->StartRotationCenter[0]);
}


//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
    {
    return false;
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

  this->SliceLogic->EndSliceNodeInteraction();

  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTouchGestureStart(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  this->SetWidgetState(WidgetStateTouchGesture);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTouchGestureEnd(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  this->TotalTouchRotation = 0.0;
  this->TouchRotateEnabled = false;
  this->TotalTouchZoom = 0.0;
  this->TouchZoomEnabled = false;
  this->TotalTouchTranslation = 0.0;
  this->TouchTranslationEnabled = false;
  this->SetWidgetState(WidgetStateIdle);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTouchRotate(vtkMRMLInteractionEventData* eventData)
{
  this->TotalTouchRotation += eventData->GetRotation() - eventData->GetLastRotation();
  if (this->TouchRotateEnabled || std::abs(this->TotalTouchRotation) >= this->TouchRotationThreshold)
    {
    this->Rotate(vtkMath::RadiansFromDegrees(eventData->GetRotation() - eventData->GetLastRotation()));
    this->TouchRotateEnabled = true;
    }
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTouchZoom(vtkMRMLInteractionEventData* eventData)
{
  this->TotalTouchZoom += eventData->GetLastScale() / eventData->GetScale();
  if (this->TouchZoomEnabled || std::abs(this->TotalTouchZoom - 1.0) > this->TouchZoomThreshold)
    {
    this->ScaleZoom(eventData->GetLastScale() / eventData->GetScale(), eventData);
    this->TouchZoomEnabled = true;
    }
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTouchTranslate(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceNode* sliceNode = this->SliceLogic->GetSliceNode();

  vtkMatrix4x4* xyToSlice = sliceNode->GetXYToSlice();
  const double* translate = eventData->GetTranslation();
  double translation[2] = {
    xyToSlice->GetElement(0, 0) * translate[0],
    xyToSlice->GetElement(1, 1) * translate[1]
  };

  this->TotalTouchTranslation += vtkMath::Norm2D(translate);

  if (this->TouchTranslationEnabled || this->TotalTouchTranslation >= this->TouchTranslationThreshold)
    {
    double sliceOrigin[3];
    sliceNode->GetXYZOrigin(sliceOrigin);
    sliceNode->SetSliceOrigin(sliceOrigin[0] - translation[0], sliceOrigin[1] - translation[1], 0);
    this->TouchTranslationEnabled = true;
    }

  return true;
}

//----------------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::SetMRMLApplicationLogic(vtkMRMLApplicationLogic* appLogic)
{
  if (appLogic == this->ApplicationLogic)
    {
    return;
    }
  this->Superclass::SetMRMLApplicationLogic(appLogic);
  vtkCollection* sliceLogics = nullptr;
  this->SliceLogic = nullptr;
  if (appLogic)
    {
    sliceLogics = appLogic->GetSliceLogics();
    }
  if (sliceLogics != this->SliceLogics)
    {
    if (this->SliceLogics)
      {
      this->SliceLogics->RemoveObserver(this->SliceLogicsModifiedCommand);
      }
    if (sliceLogics)
      {
      sliceLogics->AddObserver(vtkCommand::ModifiedEvent, this->SliceLogicsModifiedCommand);
      }
    this->SliceLogics = sliceLogics;
    if (this->GetSliceNode())
      {
      this->SliceLogic = this->GetMRMLApplicationLogic()->GetSliceLogic(this->GetSliceNode());
      }
    }
}

//----------------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::SliceLogicsModifiedCallback(vtkObject* vtkNotUsed(caller),
  unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkMRMLSliceIntersectionWidget* self = vtkMRMLSliceIntersectionWidget::SafeDownCast((vtkObject*)clientData);
  if (!self)
    {
    return;
    }
  vtkMRMLSliceIntersectionRepresentation2D* rep = vtkMRMLSliceIntersectionRepresentation2D::SafeDownCast(self->WidgetRep);
  if (rep)
    {
    rep->UpdateIntersectingSliceNodes();
    }
  vtkMRMLSliceIntersectionInteractionRepresentation* repInteraction = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(self->WidgetRep);
  if (repInteraction)
    {
    repInteraction->UpdateIntersectingSliceNodes();
    }
  self->SliceLogic = nullptr;
  if (self->GetMRMLApplicationLogic())
    {
    self->SliceLogic = self->GetMRMLApplicationLogic()->GetSliceLogic(self->GetSliceNode());
    }
}

//----------------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::SliceModifiedCallback(vtkObject* vtkNotUsed(caller),
  unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkMRMLSliceIntersectionWidget* self = vtkMRMLSliceIntersectionWidget::SafeDownCast((vtkObject*)clientData);
  if (!self)
    {
    return;
    }

  vtkMRMLSliceNode* sliceNode = self->GetSliceNode();
  if (!sliceNode)
    {
    return;
    }

  bool needToUpdateRepresentation = false;


  if (self->IsSliceIntersectionInteractive())
    {
    needToUpdateRepresentation = !vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(self->WidgetRep);
    }
  else
    {
    needToUpdateRepresentation = !vtkMRMLSliceIntersectionRepresentation2D::SafeDownCast(self->WidgetRep);
    }
  if (needToUpdateRepresentation)
    {
    self->WidgetRep = nullptr;
    self->CreateDefaultRepresentation();
    }
}

//----------------------------------------------------------------------------------
vtkMRMLSliceDisplayNode* vtkMRMLSliceIntersectionWidget::GetSliceDisplayNode()
{
  if (!this->SliceLogic)
    {
    return nullptr;
    }
  return this->SliceLogic->GetSliceDisplayNode();
}

//----------------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (this->SliceNode == sliceNode)
    {
    // no change
    return;
    }

  if (this->SliceNode)
    {
    this->SliceNode->RemoveObserver(this->SliceModifiedCommand);
    }
  if (sliceNode)
    {
    sliceNode->AddObserver(vtkCommand::ModifiedEvent, this->SliceModifiedCommand);
    }

  this->SliceNode = sliceNode;

  // Update slice logic
  this->SliceLogic = nullptr;
  if (this->GetMRMLApplicationLogic())
    {
    this->SliceLogic = this->GetMRMLApplicationLogic()->GetSliceLogic(this->SliceNode);
    }

  // Update representation
  vtkMRMLSliceIntersectionRepresentation2D* rep = vtkMRMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  if (rep)
    {
    rep->SetSliceNode(sliceNode);
    }
  vtkMRMLSliceIntersectionInteractionRepresentation* repInteraction = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (repInteraction)
    {
    repInteraction->SetSliceNode(sliceNode);
    }
}

//----------------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceIntersectionWidget::GetSliceNode()
{
  return this->SliceNode;
}

//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessRotateIntersectingSlicesStart(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceIntersectionRepresentation2D* rep = vtkMRMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  this->SetWidgetState(WidgetStateRotateIntersectingSlices);

  // Save rotation center
  double* sliceIntersectionPoint = rep->GetSliceIntersectionPoint();
  this->StartRotationCenter[0] = sliceIntersectionPoint[0];
  this->StartRotationCenter[1] = sliceIntersectionPoint[1];
  double startRotationCenterXY[4] = { sliceIntersectionPoint[0] , sliceIntersectionPoint[1], 0.0, 1.0 };
  this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(startRotationCenterXY, this->StartRotationCenter_RAS);

  // Save initial rotation angle
  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  this->PreviousRotationAngleRad = this->GetSliceRotationAngleRad(displayPosDouble);

  return this->ProcessStartMouseDrag(eventData);
}

//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessRotateIntersectingSlices(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceIntersectionRepresentation2D* rep = vtkMRMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  double sliceRotationAngleRad = this->GetSliceRotationAngleRad(displayPosDouble);

  vtkMatrix4x4* sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
  vtkNew<vtkTransform> rotatedSliceToSliceTransform;

  rotatedSliceToSliceTransform->Translate(this->StartRotationCenter_RAS[0], this->StartRotationCenter_RAS[1], this->StartRotationCenter_RAS[2]);
  double rotationDirection = vtkMath::Determinant3x3(sliceToRAS->Element[0], sliceToRAS->Element[1], sliceToRAS->Element[2]) >= 0 ? 1.0 : -1.0;
  rotatedSliceToSliceTransform->RotateWXYZ(rotationDirection*vtkMath::DegreesFromRadians(sliceRotationAngleRad - this->PreviousRotationAngleRad),
    sliceToRAS->GetElement(0, 2), sliceToRAS->GetElement(1, 2), sliceToRAS->GetElement(2, 2));
  rotatedSliceToSliceTransform->Translate(-this->StartRotationCenter_RAS[0], -this->StartRotationCenter_RAS[1], -this->StartRotationCenter_RAS[2]);

  this->PreviousRotationAngleRad = sliceRotationAngleRad;
  this->PreviousEventPosition[0] = displayPos[0];
  this->PreviousEventPosition[1] = displayPos[1];

  rep->TransformIntersectingSlices(rotatedSliceToSliceTransform->GetMatrix());

  return true;
}

//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::Rotate(double sliceRotationAngleRad)
{
  vtkMRMLSliceIntersectionRepresentation2D* rep = vtkMRMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }
  vtkMatrix4x4* sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
  vtkNew<vtkTransform> rotatedSliceToSliceTransform;

  double* sliceIntersectionPoint = rep->GetSliceIntersectionPoint();
  double startRotationCenterXY[4] = { sliceIntersectionPoint[0] , sliceIntersectionPoint[1], 0.0, 1.0 };
  this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(startRotationCenterXY, this->StartRotationCenter_RAS);

  rotatedSliceToSliceTransform->Translate(this->StartRotationCenter_RAS[0], this->StartRotationCenter_RAS[1], this->StartRotationCenter_RAS[2]);
  double rotationDirection = vtkMath::Determinant3x3(sliceToRAS->Element[0], sliceToRAS->Element[1], sliceToRAS->Element[2]) >= 0 ? 1.0 : -1.0;
  rotatedSliceToSliceTransform->RotateWXYZ(rotationDirection*vtkMath::DegreesFromRadians(sliceRotationAngleRad),
    sliceToRAS->GetElement(0, 2), sliceToRAS->GetElement(1, 2), sliceToRAS->GetElement(2, 2));
  rotatedSliceToSliceTransform->Translate(-this->StartRotationCenter_RAS[0], -this->StartRotationCenter_RAS[1], -this->StartRotationCenter_RAS[2]);

  this->PreviousRotationAngleRad = sliceRotationAngleRad;

  rep->TransformIntersectingSlices(rotatedSliceToSliceTransform->GetMatrix());
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessBlend(vtkMRMLInteractionEventData* eventData)
{
  const int* eventPosition = eventData->GetDisplayPosition();

  const int* windowSize = this->Renderer->GetRenderWindow()->GetSize();
  double windowMinSize = std::min(windowSize[0], windowSize[1]);

  int deltaY = eventPosition[1] - this->PreviousEventPosition[1];
  double offsetY = (2.0 * deltaY) / windowMinSize;
  double newForegroundOpacity =
    this->LastForegroundOpacity + offsetY;
  newForegroundOpacity = std::min(std::max(newForegroundOpacity, 0.), 1.);
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  if (sliceCompositeNode->GetForegroundVolumeID() != nullptr)
    {
    sliceCompositeNode->SetForegroundOpacity(newForegroundOpacity);
    this->LastForegroundOpacity = newForegroundOpacity;
    }
  int deltaX = eventPosition[0] - this->PreviousEventPosition[0];
  double offsetX = (2.0 * deltaX) / windowMinSize;
  double newLabelOpacity = this->LastLabelOpacity + offsetX;
  newLabelOpacity = std::min(std::max(newLabelOpacity, 0.), 1.);
  if (sliceCompositeNode->GetLabelVolumeID() != nullptr || this->StartActionSegmentationDisplayNode != nullptr)
    {
    this->SetLabelOpacity(newLabelOpacity);
    this->LastLabelOpacity = newLabelOpacity;
    }

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];

  return true;
}


//----------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::SetLabelOpacity(double opacity)
{
  // If a labelmap node is selected then adjust opacity of that
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  if (sliceCompositeNode->GetLabelVolumeID())
  {
    sliceCompositeNode->SetLabelOpacity(opacity);
    return;
  }
  // No labelmap node is selected, adjust segmentation node instead
  vtkMRMLSegmentationDisplayNode* displayNode = this->StartActionSegmentationDisplayNode;
  if (!displayNode)
  {
    displayNode = this->GetVisibleSegmentationDisplayNode();
  }
  if (!displayNode)
  {
    return;
  }
  displayNode->SetOpacity(opacity);
}

//----------------------------------------------------------------------------
double vtkMRMLSliceIntersectionWidget::GetLabelOpacity()
{
  // If a labelmap node is selected then get opacity of that
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  if (sliceCompositeNode->GetLabelVolumeID())
  {
    return sliceCompositeNode->GetLabelOpacity();
  }
  // No labelmap node is selected, use segmentation node instead
  vtkMRMLSegmentationDisplayNode* displayNode = this->StartActionSegmentationDisplayNode;
  if (!displayNode)
  {
    displayNode = this->GetVisibleSegmentationDisplayNode();
  }
  if (!displayNode)
  {
    return 0;
  }
  return displayNode->GetOpacity();
}

//----------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode* vtkMRMLSliceIntersectionWidget::GetVisibleSegmentationDisplayNode()
{
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  vtkMRMLScene* scene = this->SliceLogic->GetMRMLScene();
  std::vector<vtkMRMLNode*> displayNodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkMRMLSegmentationDisplayNode", displayNodes) : 0;
  for (int i = 0; i < nnodes; i++)
  {
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(displayNodes[i]);
    if (displayNode
      && displayNode->GetVisibility(sliceNode->GetID())
      && (displayNode->GetVisibility2DOutline() || displayNode->GetVisibility2DFill()))
    {
      return displayNode;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
double vtkMRMLSliceIntersectionWidget::GetSliceSpacing()
{
  double spacing;
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();

  if (sliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode)
    {
    spacing = sliceNode->GetPrescribedSliceSpacing()[2];
    }
  else
    {
    spacing = this->SliceLogic->GetLowestVolumeSliceSpacing()[2];
    }
  return spacing;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::IncrementSlice()
{
  this->MoveSlice(this->GetSliceSpacing());
}
//----------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::DecrementSlice()
{
  this->MoveSlice(-1. * this->GetSliceSpacing());
}
//----------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::MoveSlice(double delta)
{
  double offset = this->SliceLogic->GetSliceOffset();
  double newOffset = offset + delta;

  double sliceBounds[6] = {0, -1, 0, -1, 0, -1};
  this->SliceLogic->GetSliceBounds(sliceBounds);
  if (newOffset >= sliceBounds[4] && newOffset <= sliceBounds[5])
    {
    this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::SliceToRASFlag);
    this->SliceLogic->SetSliceOffset(newOffset);
    this->SliceLogic->EndSliceNodeInteraction();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::CycleVolumeLayer(int layer, int direction)
{
  // first, find the current volume index for the given layer (can be nullptr)
  vtkMRMLScene *scene = this->SliceLogic->GetMRMLScene();
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  const char *volumeID = nullptr;
  switch (layer)
    {
    case 0: { volumeID = sliceCompositeNode->GetBackgroundVolumeID(); } break;
    case 1: { volumeID = sliceCompositeNode->GetForegroundVolumeID(); } break;
    case 2: { volumeID = sliceCompositeNode->GetLabelVolumeID(); } break;
    }
  vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(scene->GetNodeByID(volumeID));

  // now figure out which one it is in the list
  int volumeCount = scene->GetNumberOfNodesByClass("vtkMRMLVolumeNode");
  int volumeIndex;
  for (volumeIndex = 0; volumeIndex < volumeCount; volumeIndex++)
    {
    if (volumeNode == scene->GetNthNodeByClass(volumeIndex, "vtkMRMLVolumeNode"))
      {
      break;
      }
    }

  // now increment by direction, and clamp to number of nodes
  volumeIndex += direction;
  if (volumeIndex >= volumeCount)
    {
    volumeIndex = 0;
    }
  if (volumeIndex < 0)
    {
    volumeIndex = volumeCount - 1;
    }

  // if we found a node, set it in the given layer
  volumeNode = vtkMRMLVolumeNode::SafeDownCast(
                  scene->GetNthNodeByClass(volumeIndex, "vtkMRMLVolumeNode"));
  if (volumeNode)
    {
    switch (layer)
      {
      case 0: { sliceCompositeNode->SetBackgroundVolumeID(volumeNode->GetID()); } break;
      case 1: { sliceCompositeNode->SetForegroundVolumeID(volumeNode->GetID()); } break;
      case 2: { sliceCompositeNode->SetLabelVolumeID(volumeNode->GetID()); } break;
      }
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTranslateSlice(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();

  double xyz[3];
  sliceNode->GetXYZOrigin(xyz);

  const int* eventPosition = eventData->GetDisplayPosition();

  // account for zoom using XYToSlice matrix
  vtkMatrix4x4* xyToSlice = sliceNode->GetXYToSlice();
  double deltaX = xyToSlice->GetElement(0, 0)*(this->PreviousEventPosition[0] - eventPosition[0]);
  double deltaY = xyToSlice->GetElement(1, 1)*(this->PreviousEventPosition[1] - eventPosition[1]);

  sliceNode->SetSliceOrigin(xyz[0] + deltaX, xyz[1] + deltaY, 0);

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessZoomSlice(vtkMRMLInteractionEventData* eventData)
{
  const int* eventPosition = eventData->GetDisplayPosition();

  const int* windowSize = this->GetRenderer()->GetRenderWindow()->GetSize();

  int deltaY = eventPosition[1] - this->StartEventPosition[1];
  double percent = (windowSize[1] + deltaY) / (1.0 * windowSize[1]);

  // the factor operation is so 'z' isn't changed and the
  // slider can still move through the full range
  if (percent > 0.)
    {
    double newFOVx = this->StartActionFOV[0] * percent;
    double newFOVy = this->StartActionFOV[1] * percent;
    double newFOVz = this->StartActionFOV[2];
    vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
    sliceNode->SetFieldOfView(newFOVx, newFOVy, newFOVz);
    sliceNode->UpdateMatrices();
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::ScaleZoom(double zoomScaleFactor, vtkMRMLInteractionEventData* eventData)
{
  // the factor operation is so 'z' isn't changed and the
  // slider can still move through the full range
  if (zoomScaleFactor <= 0)
    {
    vtkWarningMacro("vtkMRMLSliceViewInteractorStyle::ScaleZoom: invalid zoom scale factor (" << zoomScaleFactor);
    return;
    }
  this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::FieldOfViewFlag);
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  int wasModifying = sliceNode->StartModify();

  // Get distance of event position from slice center
  const int* eventPosition = eventData->GetDisplayPosition();
  const int* windowSize = this->GetRenderer()->GetRenderWindow()->GetSize();
  vtkMatrix4x4* xyToSlice = sliceNode->GetXYToSlice();
  double evenPositionDistanceFromOrigin[2] =
    {
    (eventPosition[0] - windowSize[0] / 2) * xyToSlice->GetElement(0, 0),
    (eventPosition[1] - windowSize[1] / 2) * xyToSlice->GetElement(1, 1)
    };

  // Adjust field of view
  double fov[3] = { 1.0 };
  sliceNode->GetFieldOfView(fov);
  fov[0] *= zoomScaleFactor;
  fov[1] *= zoomScaleFactor;
  sliceNode->SetFieldOfView(fov[0], fov[1], fov[2]);

  // Keep the mouse position at the same place on screen
  double sliceOrigin[3] = { 0 };
  sliceNode->GetXYZOrigin(sliceOrigin);
  sliceNode->SetSliceOrigin(
    sliceOrigin[0] + evenPositionDistanceFromOrigin[0] * (1.0 - zoomScaleFactor),
    sliceOrigin[1] + evenPositionDistanceFromOrigin[1] * (1.0 - zoomScaleFactor),
    sliceOrigin[2]);

  sliceNode->UpdateMatrices();
  sliceNode->EndModify(wasModifying);
  this->SliceLogic->EndSliceNodeInteraction();
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessSetCrosshair(vtkMRMLInteractionEventData* eventData)
{
  if (!this->ModifierKeyPressedSinceLastClickAndDrag)
    {
    // this event was caused by a "stuck" modifier key
    return false;
    }
  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(sliceNode->GetScene());
  if (!crosshairNode)
    {
    return false;
    }
  const double* worldPos = eventData->GetWorldPosition();
  crosshairNode->SetCrosshairRAS(const_cast<double*>(worldPos));
  if (crosshairNode->GetCrosshairBehavior() != vtkMRMLCrosshairNode::NoAction)
    {
    int viewJumpSliceMode = vtkMRMLSliceNode::OffsetJumpSlice;
    if (crosshairNode->GetCrosshairBehavior() == vtkMRMLCrosshairNode::CenteredJumpSlice)
      {
      viewJumpSliceMode = vtkMRMLSliceNode::CenteredJumpSlice;
      }
    sliceNode->JumpAllSlices(sliceNode->GetScene(),
      worldPos[0], worldPos[1], worldPos[2],
      viewJumpSliceMode, sliceNode->GetViewGroup(), sliceNode);
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::IsEventInsideVolume(bool background, vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  if (!sliceNode)
  {
    return false;
  }
  vtkMRMLSliceLayerLogic* layerLogic = background ?
    this->SliceLogic->GetBackgroundLayer() : this->SliceLogic->GetForegroundLayer();
  if (!layerLogic)
  {
    return false;
  }
  vtkMRMLVolumeNode* volumeNode = layerLogic->GetVolumeNode();
  if (!volumeNode || !volumeNode->GetImageData())
  {
    return false;
  }
  const int* eventPosition = eventData->GetDisplayPosition();
  double xyz[3] = { 0 };
  vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetRenderer(), sliceNode, eventPosition[0], eventPosition[1], xyz);
  vtkGeneralTransform* xyToBackgroundIJK = layerLogic->GetXYToIJKTransform();
  double mousePositionIJK[3] = { 0 };
  xyToBackgroundIJK->TransformPoint(xyz, mousePositionIJK);
  int volumeExtent[6] = { 0 };
  volumeNode->GetImageData()->GetExtent(volumeExtent);
  for (int i = 0; i < 3; i++)
  {
    if (mousePositionIJK[i]<volumeExtent[i * 2] || mousePositionIJK[i]>volumeExtent[i * 2 + 1])
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::GetActionEnabled(int actionsMask)
{
  return (this->ActionsEnabled & actionsMask) == actionsMask;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::SetActionEnabled(int actionsMask, bool enable /*=true*/)
{
  if (enable)
    {
    this->ActionsEnabled |= actionsMask;
    }
  else
    {
    this->ActionsEnabled &= (~actionsMask);
    }
  this->UpdateInteractionEventMapping();
}

//----------------------------------------------------------------------------
int vtkMRMLSliceIntersectionWidget::GetActionsEnabled()
{
  return this->ActionsEnabled;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceIntersectionWidget::SetActionsEnabled(int actions)
{
  this->ActionsEnabled = actions;
  this->UpdateInteractionEventMapping();
}

//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != WidgetStateIdle)
    {
    return false;
    }
  if (!this->SliceNode)
    {
    return false;
    }
  vtkMRMLInteractionNode* interactionNode = this->SliceNode->GetInteractionNode();
  if (!interactionNode)
    {
    return false;
    }
  vtkNew<vtkMRMLInteractionEventData> pickEventData;
  pickEventData->SetType(vtkMRMLInteractionNode::ShowViewContextMenuEvent);
  pickEventData->SetViewNode(this->SliceNode);
  if (eventData->IsDisplayPositionValid())
    {
    pickEventData->SetDisplayPosition(eventData->GetDisplayPosition());
    }
  if (eventData->IsWorldPositionValid())
    {
    pickEventData->SetWorldPosition(eventData->GetWorldPosition(), eventData->IsWorldPositionAccurate());
    }
  interactionNode->ShowViewContextMenu(pickEventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessMaximizeView(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  if (this->WidgetState != WidgetStateIdle)
    {
    return false;
    }
  if (!this->SliceNode)
    {
    return false;
    }

  bool isMaximized = false;
  bool canBeMaximized = false;
  vtkMRMLLayoutNode* layoutNode = this->SliceNode->GetMaximizedState(isMaximized, canBeMaximized);
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
    layoutNode->SetMaximizedViewNode(this->SliceNode);
    }

  // Maximize/restore takes away the focus without resetting
  // this->ModifierKeyPressedSinceLastMouseButtonRelease
  // therefore we need to reset the flag here.
  this->ModifierKeyPressedSinceLastClickAndDrag = true;

  return true;
}

//-------------------------------------------------------------------------
int vtkMRMLSliceIntersectionWidget::GetMouseCursor()
{
  switch (this->WidgetState)
    {
    case WidgetStateOnTranslateIntersectingSlicesHandle:
    case WidgetStateOnTranslateSingleIntersectingSliceHandle:
      return VTK_CURSOR_SIZEALL;
      break;
    case WidgetStateOnRotateIntersectingSlicesHandle:
      return VTK_CURSOR_HAND;
      break;
    default:
      return VTK_CURSOR_DEFAULT;
      break;
    }
}

//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTranslateIntersectingSlicesHandleStart(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceIntersectionInteractionRepresentation* rep = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  this->SetWidgetState(WidgetStateTranslateIntersectingSlicesHandle);

  // Save start translation point XY
  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  this->StartTranslationPoint[0] = displayPosDouble[0];
  this->StartTranslationPoint[1] = displayPosDouble[1];

  // Save start translation point RAS
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double displayPosDouble_XY[4] = { displayPosDouble[0], displayPosDouble[1], 0, 1 };
  double displayPosDouble_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(displayPosDouble_XY, displayPosDouble_RAS);
  this->StartTranslationPoint_RAS[0] = displayPosDouble_RAS[0];
  this->StartTranslationPoint_RAS[1] = displayPosDouble_RAS[1];
  this->StartTranslationPoint_RAS[2] = displayPosDouble_RAS[2];

  return this->ProcessStartMouseDrag(eventData);
}


//----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTranslateIntersectingSlicesHandle(vtkMRMLInteractionEventData* eventData)
{
  // Get current slice node and scene
  vtkMRMLSliceNode* sliceNode = this->SliceLogic->GetSliceNode();
  vtkMRMLScene* scene = sliceNode->GetScene();

  // Get event position
  const int* eventPosition = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(eventPosition[0]), static_cast<double>(eventPosition[1]) };
  const double* worldPos = eventData->GetWorldPosition();

  // Get event position RAS
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double eventPos_XY[4] = { displayPosDouble[0], displayPosDouble[1], 0, 1 };
  double eventPos_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(eventPos_XY, eventPos_RAS);
  this->CurrentTranslationPoint_RAS[0] = eventPos_RAS[0];
  this->CurrentTranslationPoint_RAS[1] = eventPos_RAS[1];
  this->CurrentTranslationPoint_RAS[2] = eventPos_RAS[2];

  // Get representation
  vtkMRMLSliceIntersectionInteractionRepresentation* rep = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
    {
    return false;
    }

  // Get intersecting slice node
  vtkMRMLSliceNode* intersectingSliceNode = vtkMRMLSliceNode::SafeDownCast(scene->GetNodeByID(this->LastIntersectingSliceNodeID));
  if (!intersectingSliceNode)
    {
    return false;
    }

  // Get translation offset
  double translationOffset[3] = { this->CurrentTranslationPoint_RAS[0] - this->StartTranslationPoint_RAS[0],
                                  this->CurrentTranslationPoint_RAS[1] - this->StartTranslationPoint_RAS[1],
                                  this->CurrentTranslationPoint_RAS[2] - this->StartTranslationPoint_RAS[2] };

  double translationOffsetNorm = vtkMath::Norm(translationOffset);
  if (translationOffsetNorm == 0) // to avoid translation when no mouse dragging was done
    {
    return false;
    }

  // Jump slice in intersecting slice node
  intersectingSliceNode->JumpSlice(worldPos[0], worldPos[1], worldPos[2]);

  // Store previous event position
  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];

  return true;
}


//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTranslateSingleIntersectingSliceHandleStart(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceIntersectionInteractionRepresentation* rep = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  this->SetWidgetState(WidgetStateTranslateSingleIntersectingSliceHandle);

  // Save start translation point XY
  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  this->StartTranslationPoint[0] = displayPosDouble[0];
  this->StartTranslationPoint[1] = displayPosDouble[1];

  // Save start translation point RAS
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double displayPosDouble_XY[4] = { displayPosDouble[0], displayPosDouble[1], 0, 1 };
  double displayPosDouble_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(displayPosDouble_XY, displayPosDouble_RAS);
  this->StartTranslationPoint_RAS[0] = displayPosDouble_RAS[0];
  this->StartTranslationPoint_RAS[1] = displayPosDouble_RAS[1];
  this->StartTranslationPoint_RAS[2] = displayPosDouble_RAS[2];

  return this->ProcessStartMouseDrag(eventData);
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessTranslateSingleIntersectingSliceHandle(vtkMRMLInteractionEventData* eventData)
{
  // Get current slice node and scene
  vtkMRMLSliceNode* sliceNode = this->SliceLogic->GetSliceNode();
  vtkMRMLScene* scene = sliceNode->GetScene();

  // Get event position
  const int* eventPosition = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(eventPosition[0]), static_cast<double>(eventPosition[1]) };
  const double* worldPos = eventData->GetWorldPosition();

  // Get event position RAS
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double eventPos_XY[4] = { displayPosDouble[0], displayPosDouble[1], 0, 1 };
  double eventPos_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(eventPos_XY, eventPos_RAS);
  this->CurrentTranslationPoint_RAS[0] = eventPos_RAS[0];
  this->CurrentTranslationPoint_RAS[1] = eventPos_RAS[1];
  this->CurrentTranslationPoint_RAS[2] = eventPos_RAS[2];

  // Get representation
  vtkMRMLSliceIntersectionInteractionRepresentation* rep = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
    {
    return false;
    }

  // Get intersecting slice node
  std::string intersectingSliceNodeID = this->LastIntersectingSliceNodeID;
  vtkMRMLSliceNode* intersectingSliceNode = vtkMRMLSliceNode::SafeDownCast(scene->GetNodeByID(intersectingSliceNodeID));
  if (!intersectingSliceNode)
    {
    return false;
    }

  this->SetWidgetState(WidgetStateTranslateSingleIntersectingSliceHandle);

  // Get translation offset
  double translationOffset[3] = { this->CurrentTranslationPoint_RAS[0] - this->StartTranslationPoint_RAS[0],
                                  this->CurrentTranslationPoint_RAS[1] - this->StartTranslationPoint_RAS[1],
                                  this->CurrentTranslationPoint_RAS[2] - this->StartTranslationPoint_RAS[2] };

  double translationOffsetNorm = vtkMath::Norm(translationOffset);
  if (translationOffsetNorm == 0) // to avoid translation when no mouse dragging was done
    {
    return false;
    }

  // Jump slice in intersecting slice node
  intersectingSliceNode->JumpSlice(worldPos[0], worldPos[1], worldPos[2]);

  // Store previous event position
  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];

  return true;
}

//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessRotateIntersectingSlicesHandleStart(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceIntersectionInteractionRepresentation* rep = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  // Save rotation center RAS
  double* sliceIntersectionPoint_XY = rep->GetSliceIntersectionPoint(); // in RAS coordinate system
  this->StartRotationCenter[0] = sliceIntersectionPoint_XY[0];
  this->StartRotationCenter[1] = sliceIntersectionPoint_XY[1];

  // Save rotation center XY
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double sliceIntersectionPoint_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(sliceIntersectionPoint_XY, sliceIntersectionPoint_RAS);
  this->StartRotationCenter_RAS[0] = sliceIntersectionPoint_RAS[0];
  this->StartRotationCenter_RAS[1] = sliceIntersectionPoint_RAS[1];
  this->StartRotationCenter_RAS[2] = sliceIntersectionPoint_RAS[2];

  // Save initial rotation angle
  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  this->PreviousRotationAngleRad = this->GetSliceRotationAngleRad(displayPosDouble);

  return this->ProcessStartMouseDrag(eventData);
}

/*
//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessRotate(double sliceRotationAngleRad)
{
  vtkMRMLSliceIntersectionRepresentation2D* rep = vtkMRMLSliceIntersectionRepresentation2D::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }
  vtkMatrix4x4* sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
  vtkNew<vtkTransform> rotatedSliceToSliceTransform;

  // Save rotation center RAS
  double* sliceIntersectionPoint_XY = rep->GetSliceIntersectionPoint(); // in RAS coordinate system
  this->StartRotationCenter[0] = sliceIntersectionPoint_XY[0];
  this->StartRotationCenter[1] = sliceIntersectionPoint_XY[1];

  // Save rotation center XY
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  double sliceIntersectionPoint_RAS[4] = { 0, 0, 0, 1 };
  xyToRAS->MultiplyPoint(sliceIntersectionPoint_XY, sliceIntersectionPoint_RAS);
  this->StartRotationCenter_RAS[0] = sliceIntersectionPoint_RAS[0];
  this->StartRotationCenter_RAS[1] = sliceIntersectionPoint_RAS[1];
  this->StartRotationCenter_RAS[2] = sliceIntersectionPoint_RAS[2];

  rotatedSliceToSliceTransform->Translate(this->StartRotationCenter_RAS[0], this->StartRotationCenter_RAS[1], this->StartRotationCenter_RAS[2]);
  double rotationDirection = vtkMath::Determinant3x3(sliceToRAS->Element[0], sliceToRAS->Element[1], sliceToRAS->Element[2]) >= 0 ? 1.0 : -1.0;
  rotatedSliceToSliceTransform->RotateWXYZ(rotationDirection * vtkMath::DegreesFromRadians(sliceRotationAngleRad),
    sliceToRAS->GetElement(0, 2), sliceToRAS->GetElement(1, 2), sliceToRAS->GetElement(2, 2));
  rotatedSliceToSliceTransform->Translate(-this->StartRotationCenter_RAS[0], -this->StartRotationCenter_RAS[1], -this->StartRotationCenter_RAS[2]);

  this->PreviousRotationAngleRad = sliceRotationAngleRad;

  rep->TransformIntersectingSlices(rotatedSliceToSliceTransform->GetMatrix());
  return true;
}
*/

//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::ProcessRotateIntersectingSlicesHandle(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceIntersectionInteractionRepresentation* rep = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
    {
    return false;
    }

  const int* displayPos = eventData->GetDisplayPosition();
  double displayPosDouble[2] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]) };
  double sliceRotationAngleRad = this->GetSliceRotationAngleRad(displayPosDouble);

  vtkMatrix4x4* sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
  vtkNew<vtkTransform> rotatedSliceToSliceTransform;

  rotatedSliceToSliceTransform->Translate(this->StartRotationCenter_RAS[0], this->StartRotationCenter_RAS[1], this->StartRotationCenter_RAS[2]);
  double rotationDirection = vtkMath::Determinant3x3(sliceToRAS->Element[0], sliceToRAS->Element[1], sliceToRAS->Element[2]) >= 0 ? 1.0 : -1.0;
  rotatedSliceToSliceTransform->RotateWXYZ(rotationDirection * vtkMath::DegreesFromRadians(sliceRotationAngleRad - this->PreviousRotationAngleRad),
    sliceToRAS->GetElement(0, 2), sliceToRAS->GetElement(1, 2), sliceToRAS->GetElement(2, 2));
  rotatedSliceToSliceTransform->Translate(-this->StartRotationCenter_RAS[0], -this->StartRotationCenter_RAS[1], -this->StartRotationCenter_RAS[2]);

  this->PreviousRotationAngleRad = sliceRotationAngleRad;
  this->PreviousEventPosition[0] = displayPos[0];
  this->PreviousEventPosition[1] = displayPos[1];

  rep->TransformIntersectingSlices(rotatedSliceToSliceTransform->GetMatrix());

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionWidget::IsSliceIntersectionInteractive()
{
  vtkMRMLApplicationLogic* mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    return false;
    }
  return mrmlAppLogic->GetSliceIntersectionEnabled(vtkMRMLApplicationLogic::SliceIntersectionInteractive);
}
