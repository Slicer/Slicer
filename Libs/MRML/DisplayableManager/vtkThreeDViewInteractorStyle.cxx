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
#include "vtkMRMLModelDisplayableManager.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkMRMLInteractionNode.h>


vtkStandardNewMacro(vtkThreeDViewInteractorStyle);

//----------------------------------------------------------------------------
vtkThreeDViewInteractorStyle::vtkThreeDViewInteractorStyle()
{
  this->MotionFactor   = 10.0;
  this->CameraNode = 0;
  this->NumberOfPlaces= 0;
  this->NumberOfTransientPlaces = 1;
}

//----------------------------------------------------------------------------
vtkThreeDViewInteractorStyle::~vtkThreeDViewInteractorStyle()
{
  this->SetCameraNode(0);
  this->NumberOfPlaces= 0;
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnChar()
{
  if (!this->Interactor->GetKeySym())
    {
    vtkErrorMacro("OnChar: could not retrieve KeySym");
    return;
    }

  // Ignore KeyPad strokes, they are handled by OnKeyPress instead.
  if (strncmp(this->Interactor->GetKeySym(), "KP_", 3) == 0 ||
      this->Interactor->GetKeyCode() == '3')
    {
    return;
    }
  this->Superclass::OnChar();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnKeyPress()
{
  this->Superclass::OnKeyPress();

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
      this->InvokeEvent(vtkCommand::InteractionEvent, 0);
      break;

    case VTKIS_PAN:
      this->FindPokedRenderer(x, y);
      this->Pan();
      this->InvokeEvent(vtkCommand::InteractionEvent, 0);
      break;

    case VTKIS_DOLLY:
      this->FindPokedRenderer(x, y);
      this->Dolly();
      this->InvokeEvent(vtkCommand::InteractionEvent, 0);
      break;

    case VTKIS_SPIN:
      this->FindPokedRenderer(x, y);
      this->Spin();
      this->InvokeEvent(vtkCommand::InteractionEvent, 0);
      break;
    default:
      this->InvokeEvent(vtkCommand::MouseMoveEvent, 0);
      break;
    }

  if (this->CameraNode)
    {
    this->CameraNode->EndModify(disabledModify);
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnLeftButtonDown()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == 0)
    {
    vtkDebugMacro("OnLeftButtonDown: couldn't find the poked renderer at event position "
                  << this->Interactor->GetEventPosition()[0] << ", "
                  << this->Interactor->GetEventPosition()[1]);
    return;
    }
  this->GrabFocus(this->EventCallbackCommand);

  // get the scene's mouse interaction mode
  int mouseInteractionMode = vtkMRMLInteractionNode::ViewTransform;
  vtkMRMLInteractionNode *interactionNode = 0;

  if ( this->GetCameraNode() != 0 &&
       this->GetCameraNode()->GetScene() != 0 )
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(
        this->GetCameraNode()->GetScene()->GetNthNodeByClass(0,"vtkMRMLInteractionNode"));

    if (interactionNode != 0)
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
        if (mouseInteractionMode == vtkMRMLInteractionNode::Place)
          {
          this->NumberOfPlaces++;
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
  vtkMRMLInteractionNode *interactionNode = 0;

  if ( this->GetCameraNode() != 0 &&
       this->GetCameraNode()->GetScene() != 0 )
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(
        this->GetCameraNode()->GetScene()->GetNthNodeByClass(0,"vtkMRMLInteractionNode"));

    if (interactionNode != 0)
      {
      mouseInteractionMode = interactionNode->GetCurrentInteractionMode();
      placeModePersistence = interactionNode->GetPlaceModePersistence();
      vtkDebugMacro("OnLeftButtonUp: mouse interaction mode = " << mouseInteractionMode);
      }
    else
      {
      vtkErrorMacro("OnLeftButtonUp: no interaction node! Assuming ViewTransform");
      }
    }

  // now throw the events if mouse mode is transient.
  if (mouseInteractionMode == vtkMRMLInteractionNode::Place)
    {
    //--- count the number of picks and
    //--- drop the interaction mode back to
    //--- the default (transform) if mouse mode
    //--- is transient.
    if ( (this->NumberOfPlaces >= this->NumberOfTransientPlaces ) &&
         (placeModePersistence == 0 ) && (interactionNode != 0) )
      {
      interactionNode->NormalizeAllMouseModes();
      interactionNode->SetLastInteractionMode ( mouseInteractionMode );
      interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::ViewTransform );
      // reset the number of place events.
      this->NumberOfPlaces = 0;
      }
    }
  switch (this->State)
    {
    case VTKIS_DOLLY:
      this->EndDolly();
      break;

    case VTKIS_PAN:
      this->EndPan();
      break;

    case VTKIS_SPIN:
      this->EndSpin();
      break;

    case VTKIS_ROTATE:
      this->EndRotate();
      break;
    }

  if ( this->Interactor )
    {
    this->ReleaseFocus();
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnMiddleButtonDown()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == 0)
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
      break;
    }
  if ( this->Interactor )
    {
    this->ReleaseFocus();
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnRightButtonDown()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == 0)
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
      break;
    }
  if ( this->Interactor )
    {
    this->ReleaseFocus();
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::OnMouseWheelForward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == 0)
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
  if (this->CurrentRenderer == 0)
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
  if ( this->GetModelDisplayableManager() != 0 )
    {
    this->GetModelDisplayableManager()->RequestRender();
    }
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::Rotate()
{
  if (this->CurrentRenderer == 0)
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

  vtkCamera *camera = 0;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->IsActiveCameraCreated() ? this->CurrentRenderer->GetActiveCamera() : 0;
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
  camera = 0;
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::Spin()
{
  if (this->CurrentRenderer == 0)
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
  vtkCamera *camera = 0;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->IsActiveCameraCreated() ? this->CurrentRenderer->GetActiveCamera() : 0;
    }

  if (!camera)
    {
    return;
    }

  camera->Roll(newAngle - oldAngle);
  camera->OrthogonalizeViewUp();

  // release the camera
  camera = 0;

  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::Pan()
{
  if (this->CurrentRenderer == 0)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double viewFocus[4], focalDepth, viewPoint[3];
  double newPickPoint[4], oldPickPoint[4], motionVector[3];

  // Calculate the focal depth since we'll be using it a lot
  vtkCamera *camera = 0;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->IsActiveCameraCreated() ? this->CurrentRenderer->GetActiveCamera() : 0;
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
  camera = 0;

  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::Dolly()
{
  if (this->CurrentRenderer == 0)
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
  if (this->CurrentRenderer == 0)
    {
    return;
    }

  vtkCamera *camera = 0;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->IsActiveCameraCreated() ? this->CurrentRenderer->GetActiveCamera() : 0;
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

  camera = 0;
}

//----------------------------------------------------------------------------
void vtkThreeDViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
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
