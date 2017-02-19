/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSliceViewInteractorStyle.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkSliceViewInteractorStyle.h"

// MRML includes
#include "vtkMRMLCrosshairDisplayableManager.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSliceLayerLogic.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include "vtkGeneralTransform.h"
#include "vtkImageData.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkNew.h"

//STL includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkSliceViewInteractorStyle, SliceLogic, vtkMRMLSliceLogic);

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSliceViewInteractorStyle);

//----------------------------------------------------------------------------
vtkSliceViewInteractorStyle::vtkSliceViewInteractorStyle()
{
  this->ActionState = vtkSliceViewInteractorStyle::None;

  this->StartActionEventPosition[0] = 0;
  this->StartActionEventPosition[1] = 0;
  this->StartActionFOV[0] = 0.;
  this->StartActionFOV[1] = 0.;
  this->StartActionFOV[2] = 0.;
  this->VolumeScalarRange[0] = 0;
  this->VolumeScalarRange[1] = 0;
  this->StartActionSegmentationDisplayNode = 0;

  this->LastForegroundOpacity = 0.;
  this->LastLabelOpacity = 0.;
  this->LastEventPosition[0] = 0;
  this->LastEventPosition[1] = 0;
  this->LastVolumeWindowLevel[0] = 0;
  this->LastVolumeWindowLevel[1] = 0;

  this->SliceLogic = 0;
}

//----------------------------------------------------------------------------
vtkSliceViewInteractorStyle::~vtkSliceViewInteractorStyle()
{
  this->SetSliceLogic(0);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "\nSlice Logic:\n";
  if (this->SliceLogic)
    {
    this->SliceLogic->PrintSelf(os, indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnKeyPress()
{
  char *key = this->Interactor->GetKeySym();

  // Note: up/down/left/right keys don't come via OnChar, so handle
  // them in OnKeyPress instead
  if ( !strcmp(key, "Right") || !strcmp(key, "Up") )
    {
    this->IncrementSlice();
    }
  else if ( !strcmp(key, "Left") || !strcmp(key, "Down") )
    {
    this->DecrementSlice();
    }
  else
    {
    this->Superclass::OnKeyPress();
    }
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnKeyRelease()
{
  this->Superclass::OnKeyRelease();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnChar()
{
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();

  char *key = this->Interactor->GetKeySym();

  if ( !strcmp(key, "f") )
    {
    this->IncrementSlice();
    }
  else if ( !strcmp(key, "b") )
    {
    this->DecrementSlice();
    }
  else if ( !strcmp(key, "v") )
    {
    sliceNode->SetSliceVisible( ! sliceNode->GetSliceVisible() );
    }
  else if ( !strcmp(key, "V") )
    {
    // TODO: need to set all slices visible
    sliceNode->SetSliceVisible( ! sliceNode->GetSliceVisible() );
    }
  else if ( !strcmp(key, "r") )
    {
    this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::ResetFieldOfViewFlag);
    this->SliceLogic->FitSliceToAll();
    sliceNode->UpdateMatrices();
    this->SliceLogic->EndSliceNodeInteraction();
    }
  else if ( !strcmp(key, "g") )
    {
    this->StartActionSegmentationDisplayNode = NULL;
    double opacity = this->GetLabelOpacity();
    if ( opacity != 0.0 )
      {
      this->LastLabelOpacity = opacity;
      this->SetLabelOpacity(0.0);
      }
    else
      {
      this->SetLabelOpacity(this->LastLabelOpacity);
      }
    }
  else if ( !strcmp(key, "t") )
    {
    double opacity = sliceCompositeNode->GetForegroundOpacity();
    if ( opacity != 0.0 )
      {
      this->LastForegroundOpacity = opacity;
      sliceCompositeNode->SetForegroundOpacity(0.0);
      }
    else
      {
      sliceCompositeNode->SetForegroundOpacity(this->LastForegroundOpacity);
      }
    }
  else if ( !strcmp(key, "s") )
    {
    vtkErrorMacro("TODO: set active lightbox/compare view slice");
    /*
     * This is commented out until the active slice highlight and the
     * correct texture is assigned.  See but #1644
    double xyz[4];
    this->GetEventXYZ(xyz);
    int k = static_cast<int>(xyz[2] + 0.5);
    if ( k >= 0 && k < sliceNode->GetDimensions()[2] )
      {
      sliceNode->SetActiveSlice(k);
      }
    */
    }
  else if ( !strcmp(key, "S") )
    {
    vtkErrorMacro("TODO: set active lightbox/compare view slice");
    }
  else if ( !strcmp(key, "bracketleft") )
    {
    this->CycleVolumeLayer(0,-1);
    }
  else if ( !strcmp(key, "bracketright") )
    {
    this->CycleVolumeLayer(0,1);
    }
  else if ( !strcmp(key, "braceleft") )
    {
    this->CycleVolumeLayer(1,-1);
    }
  else if ( !strcmp(key, "braceright") )
    {
    this->CycleVolumeLayer(1,1);
    }
  else
    {
    this->Superclass::OnChar();
    }
}

//----------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode* vtkSliceViewInteractorStyle::GetVisibleSegmentationDisplayNode()
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
  return NULL;
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::SetLabelOpacity(double opacity)
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
double vtkSliceViewInteractorStyle::GetLabelOpacity()
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
void vtkSliceViewInteractorStyle::OnRightButtonDown()
{
  this->SliceLogic->GetMRMLScene()->SaveStateForUndo(this->SliceLogic->GetSliceNode());
  this->SetActionState(vtkSliceViewInteractorStyle::Zoom);
  this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::FieldOfViewFlag);
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  sliceNode->GetFieldOfView(this->StartActionFOV);
  this->GetInteractor()->GetEventPosition(this->StartActionEventPosition);
  this->GetInteractor()->GetEventPosition(this->LastEventPosition);
}
//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnRightButtonUp()
{
  this->SetActionState(vtkSliceViewInteractorStyle::None);
  this->SliceLogic->EndSliceNodeInteraction();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMiddleButtonDown()
{
  this->StartTranslate();
  this->GetInteractor()->GetEventPosition(this->StartActionEventPosition);
  this->GetInteractor()->GetEventPosition(this->LastEventPosition);
}
//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMiddleButtonUp()
{
  this->EndTranslate();
}

//----------------------------------------------------------------------------
int vtkSliceViewInteractorStyle::GetMouseInteractionMode()
{
  if ( this->SliceLogic == 0 ||
       this->SliceLogic->GetMRMLScene() == 0 )
    {
    vtkErrorMacro("vtkSliceViewInteractorStyle::GetMouseInteractionMode: failed to get scene");
    return vtkMRMLInteractionNode::ViewTransform;
    }
  vtkMRMLScene* scene = this->SliceLogic->GetMRMLScene();

  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast(scene->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
  if (interactionNode == 0)
    {
    vtkErrorMacro("vtkSliceViewInteractorStyle::GetMouseInteractionMode: failed to get interaction node");
    return vtkMRMLInteractionNode::ViewTransform;
    }

  return interactionNode->GetCurrentInteractionMode();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnLeftButtonDown()
{
  if (this->Interactor->GetShiftKey())
    {
    this->StartTranslate();
    }
  else if (this->Interactor->GetControlKey())
    {
    this->StartBlend();
    }
  else
    {
    // Only adjust window/level in the default mouse mode.
    // Without this window/level could be changed accidentally while in place mode
    // and accidentally dragging the mouse while placing a new markup.
    // (in the future it may make sense to add a special mouse mode for window/level)
    if (this->GetMouseInteractionMode() == vtkMRMLInteractionNode::ViewTransform)
      {
      this->StartAdjustWindowLevel();
      }
    }
  this->GetInteractor()->GetEventPosition(this->StartActionEventPosition);
  this->GetInteractor()->GetEventPosition(this->LastEventPosition);
  this->Superclass::OnLeftButtonDown();
}
//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnLeftButtonUp()
{
  if (this->ActionState == this->Translate)
    {
    this->EndTranslate();
    }
  else if (this->ActionState == this->Blend)
    {
    this->EndBlend();
    }
  else
    {
    this->EndAdjustWindowLevel();
    }
  this->Superclass::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::ScaleZoom(double zoomScaleFactor)
{
  // the factor operation is so 'z' isn't changed and the
  // slider can still move through the full range
  if (zoomScaleFactor <= 0)
    {
    vtkWarningMacro("vtkSliceViewInteractorStyle::ScaleZoom: invalid zoom scale factor (" << zoomScaleFactor);
    return;
    }
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();

  // Get distance of event position from slice center
  int eventPosition[2] = { 0 };
  this->GetInteractor()->GetEventPosition(eventPosition[0], eventPosition[1]);
  int* windowSize = this->GetInteractor()->GetRenderWindow()->GetSize();
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
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseMove()
{
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  int eventPosition[2] = { 0 };
  this->GetInteractor()->GetEventPosition(eventPosition[0], eventPosition[1]);
  int* windowSize = this->GetInteractor()->GetRenderWindow()->GetSize();
  double windowMinSize = std::min(windowSize[0], windowSize[1]);

  switch (this->GetActionState())
    {
    case vtkSliceViewInteractorStyle::Translate:
      {
      double xyz[3];
      sliceNode->GetXYZOrigin(xyz);

      // account for zoom using XYToSlice matrix
      vtkMatrix4x4* xyToSlice = sliceNode->GetXYToSlice();
      double deltaX = xyToSlice->GetElement(0, 0)*(this->LastEventPosition[0] - eventPosition[0]);
      double deltaY = xyToSlice->GetElement(1, 1)*(this->LastEventPosition[1] - eventPosition[1]);

      sliceNode->SetSliceOrigin(xyz[0] + deltaX, xyz[1] + deltaY, 0);
      }
      break;
    case vtkSliceViewInteractorStyle::Zoom:
      {
      int deltaY = eventPosition[1] - this->StartActionEventPosition[1];
      double percent = (windowSize[1] + deltaY) / (1.0 * windowSize[1]);

      // the factor operation is so 'z' isn't changed and the
      // slider can still move through the full range
      if ( percent > 0. )
        {
        double newFOVx = this->StartActionFOV[0] * percent;
        double newFOVy = this->StartActionFOV[1] * percent;
        double newFOVz = this->StartActionFOV[2];
        sliceNode->SetFieldOfView( newFOVx, newFOVy, newFOVz );
        sliceNode->UpdateMatrices();
        }
      }
      break;
    case vtkSliceViewInteractorStyle::Blend:
      {
      int deltaY = eventPosition[1] - this->LastEventPosition[1];
      double offsetY =  (2.0 * deltaY) / windowMinSize;
      double newForegroundOpacity =
        this->LastForegroundOpacity + offsetY;
      newForegroundOpacity = std::min(std::max(newForegroundOpacity, 0.), 1.);
      if (sliceCompositeNode->GetForegroundVolumeID() != 0)
        {
        sliceCompositeNode->SetForegroundOpacity(newForegroundOpacity);
        this->LastForegroundOpacity = newForegroundOpacity;
        }
      int deltaX = eventPosition[0] - this->LastEventPosition[0];
      double offsetX =  (2.0 * deltaX) / windowMinSize;
      double newLabelOpacity = this->LastLabelOpacity + offsetX;
      newLabelOpacity = std::min(std::max(newLabelOpacity, 0.), 1.);
      if (sliceCompositeNode->GetLabelVolumeID() != 0 || this->StartActionSegmentationDisplayNode != 0)
        {
        this->SetLabelOpacity(newLabelOpacity);
        this->LastLabelOpacity = newLabelOpacity;
        }
      }
      break;
    case vtkSliceViewInteractorStyle::AdjustWindowLevelBackground:
    case vtkSliceViewInteractorStyle::AdjustWindowLevelForeground:
      {
      int deltaX = eventPosition[0] - this->LastEventPosition[0];
      int deltaY = eventPosition[1] - this->LastEventPosition[1];

      double rangeLow = this->VolumeScalarRange[0];
      double rangeHigh = this->VolumeScalarRange[1];

      double gain = (rangeHigh - rangeLow) / windowMinSize;
      double newWindow = this->LastVolumeWindowLevel[0] + (gain * deltaX);
      if (newWindow < 0)
        {
        newWindow = 0;
        }
      double newLevel = this->LastVolumeWindowLevel[1] + (gain * deltaY);
      if (newLevel < rangeLow - newWindow / 2)
        {
        newLevel = rangeLow - newWindow / 2;
        }
      if (newLevel > rangeHigh + newWindow / 2)
        {
        newLevel = rangeHigh + newWindow / 2;
        }
      if (this->GetActionState() == vtkSliceViewInteractorStyle::AdjustWindowLevelBackground)
        {
        this->SliceLogic->SetBackgroundWindowLevel(newWindow, newLevel);
        }
      else
        {
        this->SliceLogic->SetForegroundWindowLevel(newWindow, newLevel);
        }
      this->LastVolumeWindowLevel[0] = newWindow;
      this->LastVolumeWindowLevel[1] = newLevel;
      }
      break;
    default:
      {
      bool performDefaultAction = true;
      vtkMRMLScene *scene = this->SliceLogic->GetMRMLScene();
      vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
      if (crosshairNode)
        {
        int *pos = this->GetInteractor()->GetEventPosition();
        double xyz[3];
        vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetInteractor(), sliceNode, pos[0], pos[1], xyz);
        crosshairNode->SetCursorPositionXYZ(xyz, sliceNode);
        if (this->Interactor->GetShiftKey())
          {
          performDefaultAction = false;
          double cursorPositionRAS[3];
          if (crosshairNode->GetCursorPositionRAS(cursorPositionRAS))
            {
            crosshairNode->SetCrosshairRAS(cursorPositionRAS);
            if (crosshairNode->GetCrosshairBehavior() == vtkMRMLCrosshairNode::JumpSlice)
              {
              sliceNode->JumpAllSlices(cursorPositionRAS[0], cursorPositionRAS[1], cursorPositionRAS[2]);
              }
            }
          }
        }
      if (performDefaultAction)
        {
        this->Superclass::OnMouseMove();
        }
      }
    }
  this->LastEventPosition[0] = eventPosition[0];
  this->LastEventPosition[1] = eventPosition[1];
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseWheelForward()
{
  if (this->Interactor->GetControlKey())
    {
    this->ScaleZoom(0.8);
    }
  else// if (!this->Interactor->GetShiftKey())
    {
    this->IncrementSlice();
    }
  this->Superclass::OnMouseWheelForward();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseWheelBackward()
{
  if (this->Interactor->GetControlKey())
    {
    this->ScaleZoom(1.2);
    }
  else// if (!this->Interactor->GetShiftKey())
    {
    this->DecrementSlice();
    }
  this->Superclass::OnMouseWheelBackward();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnExpose()
{
  this->Superclass::OnExpose();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnConfigure()
{
  this->Superclass::OnConfigure();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnEnter()
{
  this->Superclass::OnEnter();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnLeave()
{
  vtkMRMLScene *scene = this->SliceLogic->GetMRMLScene();
  vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
  if (crosshairNode)
    {
    crosshairNode->SetCursorPositionInvalid();
    }
  this->Superclass::OnLeave();
}

//----------------------------------------------------------------------------
double vtkSliceViewInteractorStyle::GetSliceSpacing()
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
void vtkSliceViewInteractorStyle::IncrementSlice()
{
  this->MoveSlice(this->GetSliceSpacing());
}
//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::DecrementSlice()
{
  this->MoveSlice(-1. * this->GetSliceSpacing());
}
//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::MoveSlice(double delta)
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
void vtkSliceViewInteractorStyle::StartTranslate()
{
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  this->SliceLogic->GetMRMLScene()->SaveStateForUndo(sliceNode);
  this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::XYZOriginFlag);

  this->SetActionState(this->Translate);
}
//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::EndTranslate()
{
  this->SetActionState(this->None);
  this->SliceLogic->EndSliceNodeInteraction();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::StartBlend()
{
  this->SetActionState(this->Blend);
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  this->LastForegroundOpacity = sliceCompositeNode->GetForegroundOpacity();
  this->LastLabelOpacity = this->GetLabelOpacity();
  this->StartActionSegmentationDisplayNode = this->GetVisibleSegmentationDisplayNode();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::EndBlend()
{
  this->SetActionState(this->None);
}

//----------------------------------------------------------------------------
bool vtkSliceViewInteractorStyle::IsMouseInsideVolume(bool background)
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
  int *pos = this->GetInteractor()->GetEventPosition();
  double xyz[3] = { 0 };
  vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetInteractor(), sliceNode, pos[0], pos[1], xyz);
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
void vtkSliceViewInteractorStyle::StartAdjustWindowLevel()
{
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  if (!sliceNode)
    {
    return;
    }
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  if (!sliceCompositeNode)
    {
    return;
    }

  // By default adjust background volume, if available
  bool adjustForeground = (sliceCompositeNode->GetBackgroundVolumeID() == NULL);
  // If both foreground and background volumes are visible then choose adjustment of
  // foreground volume, if foreground volume is visible in current mouse position
  if (sliceCompositeNode->GetBackgroundVolumeID() && sliceCompositeNode->GetForegroundVolumeID())
    {
    adjustForeground = (sliceCompositeNode->GetForegroundOpacity() > 0.0)
      && this->IsMouseInsideVolume(true)   // inside background (used as mask for displaying foreground)
      && this->IsMouseInsideVolume(false); // inside foreground
    }

  if (adjustForeground)
    {
    this->SetActionState(this->AdjustWindowLevelForeground);
    this->SliceLogic->GetForegroundWindowLevelAndRange(
      this->LastVolumeWindowLevel[0], this->LastVolumeWindowLevel[1],
      this->VolumeScalarRange[0], this->VolumeScalarRange[1]);
    }
  else
    {
    this->SetActionState(this->AdjustWindowLevelBackground);
    this->SliceLogic->GetBackgroundWindowLevelAndRange(
      this->LastVolumeWindowLevel[0], this->LastVolumeWindowLevel[1],
      this->VolumeScalarRange[0], this->VolumeScalarRange[1]);
    }
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::EndAdjustWindowLevel()
{
  this->SetActionState(this->None);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::GetEventXYZ(double xyz[4])
{
  int eventPosition[2] = { 0 };
  this->GetInteractor()->GetEventPosition(eventPosition);
  int *windowSize = this->GetInteractor()->GetRenderWindow()->GetSize();

  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  int numRows = sliceNode->GetLayoutGridRows();
  int numCols = sliceNode->GetLayoutGridColumns();

  if ( windowSize[0] == 0 || windowSize[1] == 0 )
    {
    // degenerate case, return gracefully
    xyz[0] = xyz[1] = xyz[2] = 0.;
    xyz[3] = 1.;
    return;
    }

  vtkRenderer *pokedRenderer = this->GetInteractor()->FindPokedRenderer(eventPosition[0], eventPosition[1]);
  int *origin = pokedRenderer->GetOrigin();
  xyz[0] = eventPosition[0] - origin[0];
  xyz[1] = eventPosition[1] - origin[1];

  double tx = eventPosition[0] / (1. * windowSize[0]);
  double ty = (windowSize[1] - eventPosition[1]) / (1. * windowSize[1]);
  xyz[2] = (floor(ty*numRows)*numCols + floor(tx*numCols));

  xyz[3] = 1.;
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::CycleVolumeLayer(int layer, int direction)
{
  // first, find the current volume index for the given layer (can be NULL)
  vtkMRMLScene *scene = this->SliceLogic->GetMRMLScene();
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  char *volumeID = NULL;
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
