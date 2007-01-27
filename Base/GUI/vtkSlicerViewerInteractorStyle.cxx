/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerViewerInteractorStyle.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSlicerViewerInteractorStyle.h"

#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerFiducialsGUI.h"
#include "vtkMRMLSelectionNode.h"

vtkCxxRevisionMacro(vtkSlicerViewerInteractorStyle, "$Revision: 1.32 $");
vtkStandardNewMacro(vtkSlicerViewerInteractorStyle);

//----------------------------------------------------------------------------
vtkSlicerViewerInteractorStyle::vtkSlicerViewerInteractorStyle() 
{
  this->MotionFactor   = 10.0;
  this->CameraNode = NULL;
  this->Application = NULL;
  this->State = VTKIS_ROTATE;
}

//----------------------------------------------------------------------------
vtkSlicerViewerInteractorStyle::~vtkSlicerViewerInteractorStyle() 
{
  this->SetCameraNode(NULL);
  this->SetApplication ( NULL );
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnMouseMove() 
{ 
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];

  switch (this->State) 
    {
    case VTKIS_ROTATE:
      this->FindPokedRenderer(x, y);
      this->Rotate();
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      break;

    case VTKIS_PAN:
      this->FindPokedRenderer(x, y);
      this->Pan();
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      break;

    case VTKIS_DOLLY:
      this->FindPokedRenderer(x, y);
      this->Dolly();
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      break;

    case VTKIS_SPIN:
      this->FindPokedRenderer(x, y);
      this->Spin();
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      break;
    default:
      this->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
      break;
    }
}


//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnEnter()
{
  if (this->HasObserver(vtkCommand::EnterEvent)) 
    {
    this->InvokeEvent(vtkCommand::EnterEvent,NULL);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnLeave()
{
  if (this->HasObserver(vtkCommand::LeaveEvent)) 
    {
    this->InvokeEvent(vtkCommand::LeaveEvent,NULL);
    }
}



//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnLeftButtonDown() 
{ 
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  // get the scene's mouse interaction mode
  int mouseInteractionMode = vtkMRMLSelectionNode::MouseTransform;
  
  if ( this->GetCameraNode() != NULL )
    {
    vtkMRMLSelectionNode *selnode;
    selnode = vtkMRMLSelectionNode::SafeDownCast (this->GetCameraNode()->GetScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
    if (selnode != NULL)
      {
      mouseInteractionMode = selnode->GetMouseInteractionMode();
      // release the pointer
      selnode = NULL;
      }
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
      if (mouseInteractionMode == vtkMRMLSelectionNode::MouseTransform)
        {
        this->StartRotate();
        }
      else if (mouseInteractionMode == vtkMRMLSelectionNode::MousePut)
        {
        this->AddFiducial();
        }
      else if (mouseInteractionMode == vtkMRMLSelectionNode::MouseSelect)
        {
        // deal with select mode
        std::cout << "Mouse Selection mode not implemented, try something else...\n";
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnLeftButtonUp()
{
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
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnMiddleButtonDown() 
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->StartPan();
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnMiddleButtonUp()
{
  switch (this->State) 
    {
    case VTKIS_PAN:
      this->EndPan();
      break;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnRightButtonDown() 
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->StartDolly();
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnRightButtonUp()
{
  switch (this->State) 
    {
    case VTKIS_DOLLY:
      this->EndDolly();
      break;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnMouseWheelForward() 
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->StartDolly();
  double factor = this->MotionFactor * 0.2 * this->MouseWheelMotionFactor;
  // Slicer; invert direction to match right button drag
  this->Dolly(pow((double)1.1, -1. * factor));
  this->EndDolly();
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::OnMouseWheelBackward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->StartDolly();
  double factor = this->MotionFactor * -0.2 * this->MouseWheelMotionFactor;
  // Slicer; invert direction to match right button drag
  this->Dolly(pow((double)1.1, -1. * factor));
  this->EndDolly();
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::Rotate()
{
  if (this->CurrentRenderer == NULL)
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
  
  vtkCamera *camera = NULL;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->GetActiveCamera();
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
  camera = NULL;
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::Spin()
{
  if (this->CurrentRenderer == NULL)
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
  
  newAngle *= vtkMath::RadiansToDegrees();
  oldAngle *= vtkMath::RadiansToDegrees();

  vtkCamera *camera = NULL;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->GetActiveCamera();
    }

  camera->Roll(newAngle - oldAngle);
  camera->OrthogonalizeViewUp();

  // release the camera
  camera = NULL;
  
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::Pan()
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double viewFocus[4], focalDepth, viewPoint[3];
  double newPickPoint[4], oldPickPoint[4], motionVector[3];
  
  // Calculate the focal depth since we'll be using it a lot
  vtkCamera *camera = NULL;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->GetActiveCamera();
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
  camera = NULL;
  
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::Dolly()
{
  if (this->CurrentRenderer == NULL)
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
void vtkSlicerViewerInteractorStyle::Dolly(double factor)
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  vtkCamera *camera = NULL;
  if (this->CameraNode)
    {
    camera = this->CameraNode->GetCamera();
    }
  else
    {
    camera = this->CurrentRenderer->GetActiveCamera();
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

  camera = NULL;
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::SetApplication ( vtkSlicerApplication *app )
{
  this->Application = app;
}

//----------------------------------------------------------------------------
void vtkSlicerViewerInteractorStyle::AddFiducial()
{
  vtkDebugMacro( "Putting down a fiducial at the origin\n");
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  double ras[3] = {0.0, 0.0, 0.0};
  
  vtkDebugMacro("\tnew pick point = " << ras[0] << ", " << ras[1] << ", " << ras[2]);
  if (this->GetApplication())
    {
    vtkSlicerFiducialsGUI *fidGUI = vtkSlicerFiducialsGUI::SafeDownCast(this->GetApplication()->GetModuleGUIByName("Fiducials"));
    if (fidGUI != NULL)
      {
      if (fidGUI->GetLogic() != NULL)
        {
        int pt = fidGUI->GetLogic()->AddFiducial(ras[0], ras[1], ras[2]);
        if (pt != -1)
          {
          vtkDebugMacro("Added a fiducial at pt = " << pt << endl);
          }
        else
          {
          vtkErrorMacro("Wasn't able to add a fiducial");
          }
        }
      else
        {
        vtkErrorMacro("Cant't get the fiducials logic");
        }
      }
    else
      {
      vtkErrorMacro("Can't get the fiducials gui");
      }
    fidGUI = NULL;
    }
  else
    {
    vtkErrorMacro("Can't get the application");
    }
}
