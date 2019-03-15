/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkThreeDViewInteractorStyle.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkThreeDViewInteractorStyle.h"

// MRML includes
#include "vtkMRMLCrosshairDisplayableManager.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLModelDisplayableManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkCallbackCommand.h>
#include <vtkEvent.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkWorldPointPicker.h>


vtkStandardNewMacro(vtkThreeDViewInteractorStyle);

//----------------------------------------------------------------------------
vtkThreeDViewInteractorStyle::vtkThreeDViewInteractorStyle()
{
  this->EventCallbackCommand->SetCallback(vtkThreeDViewInteractorStyle::ThreeDViewProcessEvents);
  this->FocusedDisplayableManager = nullptr;
  this->ShiftKeyUsedForPreviousAction = false;
  this->MouseMovedSinceButtonDown = false;

  this->MotionFactor = 10.0;

  this->CameraNode = nullptr;
  this->ModelDisplayableManager = nullptr;
  this->AccuratePicker = vtkSmartPointer<vtkCellPicker>::New();
  this->AccuratePicker->SetTolerance( .005 );
  this->QuickPicker = vtkSmartPointer<vtkWorldPointPicker>::New();
}

//----------------------------------------------------------------------------
vtkThreeDViewInteractorStyle::~vtkThreeDViewInteractorStyle()
{
  this->SetCameraNode(nullptr);
  this->SetModelDisplayableManager(nullptr);
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnKeyPress()
{
  if(!this->CameraNode)
    {
    vtkErrorMacro("OnKeyPress: camera node is null");
    return;
    }

  const char* keySym = this->Interactor->GetKeySym();

  if (!keySym)
    {
    vtkErrorMacro("OnKeyPress: could not retrieve KeySym");
    return;
    }

  bool shift = (this->Interactor->GetShiftKey() != 0);

  if (strcmp(keySym, "KP_1") == 0 ||
      strcmp(keySym, "End") == 0)
    {
    this->CameraNode->RotateTo(
      shift ? vtkMRMLCameraNode::Posterior : vtkMRMLCameraNode::Anterior );
    }
  else if (strcmp(keySym, "KP_3") == 0 ||
           strcmp(keySym, "Next") == 0) // PageDown
    {
    this->CameraNode->RotateTo(
      shift ? vtkMRMLCameraNode::Right : vtkMRMLCameraNode::Left);
    }
  else if (strcmp(keySym, "KP_7") == 0 ||
           strcmp(keySym, "Home") == 0)
    {
    this->CameraNode->RotateTo(
      shift ? vtkMRMLCameraNode::Inferior : vtkMRMLCameraNode::Superior);
    }
  else if (strcmp(keySym, "KP_2") == 0 ||
           strcmp(keySym, "Down") == 0)
    {
    if (shift)
      {
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::Y, false);
      }
    else
      {
      this->CameraNode->RotateAround(vtkMRMLCameraNode::R, true);
      }
    }
  else if (strcmp(keySym, "KP_8") == 0 ||
           strcmp(keySym, "Up") == 0)
    {
    if (shift)
      {
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::Y, true);
      }
    else
      {
      this->CameraNode->RotateAround(vtkMRMLCameraNode::R, false);
      }
    }
  else if (strcmp(keySym, "KP_4") == 0 ||
           strcmp(keySym, "Left") == 0)
    {
    if (shift)
      {
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::X, true);
      }
    else
      {
      this->CameraNode->RotateAround(vtkMRMLCameraNode::S, true);
      }
    }
  else if (strcmp(keySym, "KP_6") == 0 ||
           strcmp(keySym, "Right") == 0)
    {
    if (shift)
      {
      this->CameraNode->TranslateAlong(vtkMRMLCameraNode::X, false);
      }
    else
      {
      this->CameraNode->RotateAround(vtkMRMLCameraNode::S, false);
      }
    }
  else if (strcmp(keySym, "KP_5") == 0 ||
           strcmp(keySym, "Clear") == 0)
    {
    this->CameraNode->Reset(
      !shift,
      shift,
      false,
      this->Interactor->FindPokedRenderer(0,0));
    }
  else if (strcmp(keySym, "KP_0") == 0 ||
           strcmp(keySym, "Insert") == 0)
    {
    this->CameraNode->Reset(
      true, true, true,
      this->Interactor->FindPokedRenderer(0,0));
    }
  else if (strcmp(keySym, "plus") == 0)
    {
    this->Dolly(1.2);
    }
  else if (strcmp(keySym, "minus") == 0)
    {
    this->Dolly(0.8);
    }
}

//----------------------------------------------------------------------------
 void vtkThreeDViewInteractorStyle::OnKeyRelease()
{
  std::string key = this->Interactor->GetKeySym();

  if (((key.find("Shift") != std::string::npos)) && this->ShiftKeyUsedForPreviousAction)
    {
    this->ShiftKeyUsedForPreviousAction = false;
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnChar()
{
  if (!this->Interactor->GetKeySym())
    {
    vtkErrorMacro("OnChar: could not retrieve KeySym");
    return;
    }

  if (strncmp(this->Interactor->GetKeySym(), "KP_", 3) == 0 ||
      this->Interactor->GetKeyCode() == '3')
    {
    // Ignore KeyPad strokes, they are handled by OnKeyPress instead.
    }
  else
    {
    this->Superclass::OnChar();
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnMouseMove()
{
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  int disabledModify = 0;

  if (this->CameraNode)
    {
    disabledModify = this->CameraNode->StartModify();
    }

  switch (this->State)
    {
    case VTKIS_ROTATE:
      this->FindPokedRenderer(x, y);
      this->Rotate();
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;

    case VTKIS_PAN:
      this->FindPokedRenderer(x, y);
      this->Pan();
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;

    case VTKIS_DOLLY:
      this->FindPokedRenderer(x, y);
      this->Dolly();
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;

    case VTKIS_SPIN:
      this->FindPokedRenderer(x, y);
      this->Spin();
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;
    default:
      if ( (!this->ShiftKeyUsedForPreviousAction) && this->Interactor->GetShiftKey() &&
           (this->GetCameraNode() != nullptr) && (this->GetCameraNode()->GetScene() != nullptr) )
        {
        double pickedRAS[3]={0,0,0};
        bool picked = this->AccuratePick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], pickedRAS);
        if (picked)
          {
          vtkMRMLScene* scene = this->GetCameraNode()->GetScene();
          vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
          if (crosshairNode)
            {
            crosshairNode->SetCrosshairRAS(pickedRAS);
            // Computing pick position in 3D is expensive,
            // therefore we only update the cursor position if shift key is pressed
            crosshairNode->SetCursorPositionRAS(pickedRAS);
            if (crosshairNode->GetCrosshairBehavior() != vtkMRMLCrosshairNode::NoAction)
              {
              // Try to get view group of the 3D view and jump only those slices.
              int viewGroup = -1; // jump all by default
              if (this->GetCameraNode() && this->GetCameraNode()->GetActiveTag())
                {
                vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(scene->GetNodeByID(this->GetCameraNode()->GetActiveTag()));
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
              vtkMRMLSliceNode::JumpAllSlices(scene, pickedRAS[0], pickedRAS[1], pickedRAS[2], viewJumpSliceMode, viewGroup);
              }
            }
          }
        }
      break;
    }

  if (this->CameraNode)
    {
    this->CameraNode->EndModify(disabledModify);
  }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnRightButtonDown()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == nullptr)
    {
    return;
    }

  this->GrabFocus(this->EventCallbackCommand);
  this->StartDolly();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnRightButtonUp()
{
  switch (this->State)
    {
    case VTKIS_DOLLY:
      this->EndDolly();
      if ( this->Interactor )
        {
        this->ReleaseFocus();
        }
      break;
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnMiddleButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == nullptr)
    {
    return;
    }
  this->GrabFocus(this->EventCallbackCommand);
  this->StartPan();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnMiddleButtonUp()
{
  switch (this->State)
    {
    case VTKIS_PAN:
      this->EndPan();
      if ( this->Interactor )
        {
        this->ReleaseFocus();
        }
      break;
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnLeftButtonDown()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == nullptr)
    {
    vtkDebugMacro("OnLeftButtonDown: couldn't find the poked renderer at event position "
                  << this->Interactor->GetEventPosition()[0] << ", "
                  << this->Interactor->GetEventPosition()[1]);
    return;
    }
  this->GrabFocus(this->EventCallbackCommand);

  // get the scene's mouse interaction mode
  int mouseInteractionMode = vtkMRMLInteractionNode::ViewTransform;
  vtkMRMLInteractionNode *interactionNode = nullptr;

  if ( this->GetCameraNode() != nullptr &&
       this->GetCameraNode()->GetScene() != nullptr )
    {
    vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(
        this->GetCameraNode()->GetScene()->GetNodeByID(this->GetCameraNode()->GetActiveTag()));
    if (!viewNode)
      {
      vtkErrorMacro("OnLeftButtonDown: failed to lookup view node associated with camera node = "
                    << this->GetCameraNode()->GetID());
      }
    interactionNode = viewNode ? viewNode->GetInteractionNode() : nullptr;

    if (interactionNode != nullptr)
      {
      mouseInteractionMode = interactionNode->GetCurrentInteractionMode();
      vtkDebugMacro("OnLeftButtonDown: mouse interaction mode = " << mouseInteractionMode);
      }
    else
      {
      vtkErrorMacro("OnLeftButtonDown: no interaction node! Assuming ViewTransform");
      }
    }
  else
    {
    vtkErrorMacro("OnLeftButtonDown: camera node " << (this->GetCameraNode() ? "has no scene defined" : "is null"));
    }

  if (this->Interactor->GetShiftKey())
    {
    this->ShiftKeyUsedForPreviousAction = true;
    if (this->Interactor->GetControlKey())
      {
      this->StartDolly();
      }
    else
      {
      this->StartPan();
      }
    }
  else
    {
    if (this->Interactor->GetControlKey())
      {
      this->StartSpin();
      }
    else
      {
      if (mouseInteractionMode == vtkMRMLInteractionNode::ViewTransform)
        {
        this->StartRotate();
        }
      else if (mouseInteractionMode == vtkMRMLInteractionNode::Place)
        {
        // get the current event position, flipping Y
        int x = this->Interactor->GetEventPosition()[0];
        int rawY = this->Interactor->GetEventPosition()[1];
        this->Interactor->SetEventPositionFlipY(x, rawY);
        if (this->GetDebug())
          {
#ifndef NDEBUG
          int y = this->Interactor->GetEventPosition()[1];
#endif
          vtkDebugMacro("MouseMode Place or PickManipulate:: got x = " << x << ", y = " << y << " (raw y = " << rawY << ")\n");
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnLeftButtonUp()
{
  int mouseInteractionMode = vtkMRMLInteractionNode::ViewTransform;
  int placeModePersistence = 0;
  vtkMRMLInteractionNode *interactionNode = nullptr;

  if ( this->GetCameraNode() != nullptr &&
       this->GetCameraNode()->GetScene() != nullptr )
    {
    vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(
        this->GetCameraNode()->GetScene()->GetNodeByID(this->GetCameraNode()->GetActiveTag()));
    if (!viewNode)
      {
      vtkErrorMacro("OnLeftButtonUp: failed to lookup view node associated with camera node = "
                    << this->GetCameraNode()->GetID());
      }
    interactionNode = viewNode ? viewNode->GetInteractionNode() : nullptr;

    if (interactionNode != nullptr)
      {
      mouseInteractionMode = interactionNode->GetCurrentInteractionMode();
      placeModePersistence = interactionNode->GetPlaceModePersistence();
      vtkDebugMacro("OnLeftButtonUp: mouse interaction mode = " << mouseInteractionMode);
      vtkDebugMacro("OnLeftButtonUp: place persistence mode = " << placeModePersistence);
#ifdef NDEBUG
      (void)mouseInteractionMode; // unused
      (void)placeModePersistence; // unused
#endif
      }
    else
      {
      vtkErrorMacro("OnLeftButtonUp: no interaction node! Assuming ViewTransform");
      }
    }


  switch (this->State)
    {
    case VTKIS_DOLLY:
      this->EndDolly();
      if ( this->Interactor )
        {
        this->ReleaseFocus();
        }
      break;

    case VTKIS_PAN:
      this->EndPan();
      if ( this->Interactor )
        {
        this->ReleaseFocus();
        }
      break;

    case VTKIS_SPIN:
      this->EndSpin();
      if ( this->Interactor )
        {
        this->ReleaseFocus();
        }
      break;

    case VTKIS_ROTATE:
      this->EndRotate();
      if ( this->Interactor )
        {
        this->ReleaseFocus();
        }
      break;
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnEnter()
{
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnLeave()
{
  if (this->GetCameraNode() == nullptr || this->GetCameraNode()->GetScene() == nullptr)
    {
    // interactor is not initialized
    return;
    }
  vtkMRMLScene* scene = this->GetCameraNode()->GetScene();
  vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
  if (crosshairNode)
    {
    crosshairNode->SetCursorPositionInvalid();
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnMouseWheelForward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == nullptr)
    {
    return;
    }

  this->GrabFocus(this->EventCallbackCommand);
  this->StartDolly();
  double factor = this->MotionFactor * 0.2 * this->MouseWheelMotionFactor;
  // Slicer; invert direction to match right button drag
  this->Dolly(pow((double)1.1, -1. * factor));
  this->EndDolly();
  this->ReleaseFocus();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnMouseWheelBackward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == nullptr)
    {
    return;
    }
  this->GrabFocus(this->EventCallbackCommand);
  this->StartDolly();
  double factor = this->MotionFactor * -0.2 * this->MouseWheelMotionFactor;
  // Slicer; invert direction to match right button drag
  this->Dolly(pow((double)1.1, -1. * factor));
  this->EndDolly();
  this->ReleaseFocus();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnExpose()
{
  /*
  if ( this->GetModelDisplayableManager() != 0 )
    {
    this->GetModelDisplayableManager()->RequestRender();
    }
    */
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnConfigure()
{
}
//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::SetDisplayableManagers(vtkMRMLDisplayableManagerGroup* displayableManagerGroup)
{
  this->DisplayableManagers = displayableManagerGroup;
}

//----------------------------------------------------------------------------
bool vtkThreeDViewInteractorStyle::ForwardInteractionEventToDisplayableManagers(unsigned long event)
{
  if (!this->DisplayableManagers)
    {
    return false;
    }
  double canProcessEvent = false;
  double closestDistance2 = VTK_DOUBLE_MAX;
  vtkMRMLAbstractDisplayableManager* closestDisplayableManager = nullptr;
  int numberOfDisplayableManagers = this->DisplayableManagers->GetDisplayableManagerCount();

  // Get display and world position
  int* displayPositionInt = this->GetInteractor()->GetEventPosition();
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(displayPositionInt[0], displayPositionInt[1]);

  vtkNew<vtkMRMLInteractionEventData> ed;
  ed->SetType(event);
  int displayPositionCorrected[2] = { displayPositionInt[0] - pokedRenderer->GetOrigin()[0], displayPositionInt[1] - pokedRenderer->GetOrigin()[1] };
  ed->SetDisplayPosition(displayPositionCorrected);
  double worldPosition[4] = { 0.0, 0.0, 0.0, 1.0 };
  if (this->QuickPick(displayPositionInt[0], displayPositionInt[1], worldPosition))
    {
    ed->SetWorldPosition(worldPosition);
    }

  ed->SetAttributesFromInteractor(this->GetInteractor());

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
    if (displayableManager->CanProcessInteractionEvent(ed, distance2))
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
    return false;
    }

  return this->FocusedDisplayableManager->ProcessInteractionEvent(ed);
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::Rotate()
{
  if (this->CurrentRenderer == nullptr)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  int dx = rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

  int *size = this->CurrentRenderer->GetRenderWindow()->GetSize();

  double delta_elevation = -20.0 / size[1];
  double delta_azimuth = -20.0 / size[0];

  double rxf = (double)dx * delta_azimuth * this->MotionFactor;
  double ryf = (double)dy * delta_elevation * this->MotionFactor;

  vtkCamera *camera = nullptr;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->IsActiveCameraCreated() ? this->CurrentRenderer->GetActiveCamera() : nullptr;
    }

  if (!camera)
    {
    return;
    }

  camera->Azimuth(rxf);
  camera->Elevation(ryf);
  camera->OrthogonalizeViewUp();

  if (this->AutoAdjustCameraClippingRange)
    {
    this->CurrentRenderer->ResetCameraClippingRange();
    }

  if (rwi->GetLightFollowCamera())
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

  // release the camera
  camera = nullptr;
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::Spin()
{
  if (this->CurrentRenderer == nullptr)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double *center = this->CurrentRenderer->GetCenter();

  double newAngle =
    atan2((double)rwi->GetEventPosition()[1] - (double)center[1],
          (double)rwi->GetEventPosition()[0] - (double)center[0]);

  double oldAngle =
    atan2((double)rwi->GetLastEventPosition()[1] - (double)center[1],
          (double)rwi->GetLastEventPosition()[0] - (double)center[0]);

  newAngle = vtkMath::DegreesFromRadians(newAngle);
  oldAngle = vtkMath::DegreesFromRadians(oldAngle);
  vtkCamera *camera = nullptr;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->IsActiveCameraCreated() ? this->CurrentRenderer->GetActiveCamera() : nullptr;
    }

  if (!camera)
    {
    return;
    }

  camera->Roll(newAngle - oldAngle);
  camera->OrthogonalizeViewUp();

  // release the camera
  camera = nullptr;

  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::Pan()
{
  if (this->CurrentRenderer == nullptr)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double viewFocus[4], focalDepth, viewPoint[3];
  double newPickPoint[4], oldPickPoint[4], motionVector[3];

  // Calculate the focal depth since we'll be using it a lot
  vtkCamera *camera = nullptr;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->IsActiveCameraCreated() ? this->CurrentRenderer->GetActiveCamera() : nullptr;
    }

  if (!camera)
    {
    return;
    }

  camera->GetFocalPoint(viewFocus);
  this->ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2],
                              viewFocus);
  focalDepth = viewFocus[2];

  this->ComputeDisplayToWorld((double)rwi->GetEventPosition()[0],
                              (double)rwi->GetEventPosition()[1],
                              focalDepth,
                              newPickPoint);

  // Has to recalc old mouse point since the viewport has moved,
  // so can't move it outside the loop

  this->ComputeDisplayToWorld((double)rwi->GetLastEventPosition()[0],
                              (double)rwi->GetLastEventPosition()[1],
                              focalDepth,
                              oldPickPoint);

  // Camera motion is reversed

  motionVector[0] = oldPickPoint[0] - newPickPoint[0];
  motionVector[1] = oldPickPoint[1] - newPickPoint[1];
  motionVector[2] = oldPickPoint[2] - newPickPoint[2];

  camera->GetFocalPoint(viewFocus);
  camera->GetPosition(viewPoint);
  camera->SetFocalPoint(motionVector[0] + viewFocus[0],
                        motionVector[1] + viewFocus[1],
                        motionVector[2] + viewFocus[2]);

  camera->SetPosition(motionVector[0] + viewPoint[0],
                      motionVector[1] + viewPoint[1],
                      motionVector[2] + viewPoint[2]);

  if (rwi->GetLightFollowCamera())
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

  // release the camera
  camera = nullptr;

  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::Dolly()
{
  if (this->CurrentRenderer == nullptr)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;
  double *center = this->CurrentRenderer->GetCenter();
  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];
  double dyf = this->MotionFactor * (double)(dy) / (double)(center[1]);
  // Slicer: pull mouse towards you to bring models closer (opposite of vtk)
  this->Dolly(pow((double)1.1, -1. * dyf));
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::Dolly(double factor)
{
  if (this->CurrentRenderer == nullptr)
    {
    return;
    }

  vtkCamera *camera = nullptr;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->IsActiveCameraCreated() ? this->CurrentRenderer->GetActiveCamera() : nullptr;
    }

  if (!camera)
    {
    return;
    }

  if (camera->GetParallelProjection())
    {
    camera->SetParallelScale(camera->GetParallelScale() / factor);
    }
  else
    {
    camera->Dolly(factor);
    if (this->AutoAdjustCameraClippingRange)
      {
      this->CurrentRenderer->ResetCameraClippingRange();
      }
    }

  if (this->Interactor->GetLightFollowCamera())
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

  this->Interactor->Render();

  camera = nullptr;
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::SetModelDisplayableManager(
    vtkMRMLModelDisplayableManager * modelDisplayableManager)
{
  this->ModelDisplayableManager = modelDisplayableManager;
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::SetInteractor(vtkRenderWindowInteractor *interactor)
{
  if (interactor)
    {
    // A default FPS of 30. seems good enough, but feel free to increase if needed
    // Please note that the VolumeRendering module changes it.
    interactor->SetDesiredUpdateRate( 30.);
    }
  this->Superclass::SetInteractor(interactor);
}

//---------------------------------------------------------------------------
bool vtkThreeDViewInteractorStyle::AccuratePick(int x, int y, double pickPoint[3])
{
  this->FindPokedRenderer(x, y);
  if (this->CurrentRenderer == nullptr)
    {
    vtkDebugMacro("Pick: couldn't find the poked renderer at event position " << x << ", " << y);
    return false;
    }

  if (!this->AccuratePicker->Pick(x, y, 0, this->CurrentRenderer))
    {
    return false;
    }

  vtkPoints* pickPositions = this->AccuratePicker->GetPickedPositions();
  int numberOfPickedPositions = pickPositions->GetNumberOfPoints();
  if (numberOfPickedPositions<1)
    {
    return false;
    }

  // There may be multiple picked positions, choose the one closest to the camera
  double cameraPosition[3]={0,0,0};
  this->CurrentRenderer->GetActiveCamera()->GetPosition(cameraPosition);
  pickPositions->GetPoint(0, pickPoint);
  double minDist2 = vtkMath::Distance2BetweenPoints(pickPoint, cameraPosition);
  for (int i=1; i<numberOfPickedPositions; i++)
  {
    double currentMinDist2 = vtkMath::Distance2BetweenPoints(pickPositions->GetPoint(i), cameraPosition);
    if (currentMinDist2<minDist2)
    {
      pickPositions->GetPoint(i, pickPoint);
      minDist2 = currentMinDist2;
    }
  }
  return true;
}

//---------------------------------------------------------------------------
bool vtkThreeDViewInteractorStyle::QuickPick(int x, int y, double pickPoint[3])
{
  this->FindPokedRenderer(x, y);
  if (this->CurrentRenderer == nullptr)
  {
    vtkDebugMacro("Pick: couldn't find the poked renderer at event position " << x << ", " << y);
    return false;
  }

  this->QuickPicker->Pick(x, y, 0, this->CurrentRenderer);

  this->QuickPicker->GetPickPosition(pickPoint);

  return true;
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::ThreeDViewProcessEvents(vtkObject* object,
  unsigned long event, void* clientdata, void* calldata)
{
  vtkThreeDViewInteractorStyle* self
    = reinterpret_cast<vtkThreeDViewInteractorStyle *>(clientdata);

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
  if (self->State != VTKIS_NONE
    || !self->ForwardInteractionEventToDisplayableManagers(event))
    {
    // Displayable managers did not processed it
    Superclass::ProcessEvents(object, event, clientdata, calldata);
    }

  // Detect click events
  if (!self->MouseMovedSinceButtonDown)
    {
    if (event == vtkCommand::LeftButtonReleaseEvent)
      {
      self->ForwardInteractionEventToDisplayableManagers(vtkMRMLInteractionEventData::LeftButtonClickEvent);
      }
    else if (event == vtkCommand::MiddleButtonReleaseEvent)
      {
      self->ForwardInteractionEventToDisplayableManagers(vtkMRMLInteractionEventData::MiddleButtonClickEvent);
      }
    else if (event == vtkCommand::RightButtonReleaseEvent)
      {
      self->ForwardInteractionEventToDisplayableManagers(vtkMRMLInteractionEventData::RightButtonClickEvent);
      }
    }
}
