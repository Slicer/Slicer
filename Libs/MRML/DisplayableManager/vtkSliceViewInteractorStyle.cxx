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

  this->ActionStartRAS[0] = 0.;
  this->ActionStartRAS[1] = 0.;
  this->ActionStartRAS[2] = 0.;
  this->ActionStartFOV[0] = 0.;
  this->ActionStartFOV[1] = 0.;
  this->ActionStartFOV[2] = 0.;
  this->ActionStartWindow[0] = 0;
  this->ActionStartWindow[1] = 0;
  this->LastActionWindow[0] = 0;
  this->LastActionWindow[1] = 0;

  this->ActionStartForegroundOpacity = 0;
  this->ActionStartLabelOpacity = 0;
  this->ActionStartVolumeWindow = 0;
  this->ActionStartVolumeLevel = 0;
  this->ActionStartVolumeRangeLow = 0;
  this->ActionStartVolumeRangeHigh = 0;

  this->ActionStartSliceToRAS = vtkMatrix4x4::New();
  this->ActionStartXYToRAS = vtkMatrix4x4::New();
  this->ScratchMatrix = vtkMatrix4x4::New();

  this->LastLabelOpacity = 0.;
  this->LastForegroundOpacity = 0.;

  this->SliceLogic = 0;
}

//----------------------------------------------------------------------------
vtkSliceViewInteractorStyle::~vtkSliceViewInteractorStyle()
{
  this->ActionStartSliceToRAS->Delete();
  this->ActionStartXYToRAS->Delete();
  this->ScratchMatrix ->Delete();

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
    double opacity = sliceCompositeNode->GetLabelOpacity();
    if ( opacity != 0.0 )
      {
      this->SetActionStartLabelOpacity(opacity);
      sliceCompositeNode->SetLabelOpacity(0.0);
      }
    else
      {
      sliceCompositeNode->SetLabelOpacity(this->GetActionStartLabelOpacity());
      }
    }
  else if ( !strcmp(key, "t") )
    {
    double opacity = sliceCompositeNode->GetForegroundOpacity();
    if ( opacity != 0.0 )
      {
      this->SetActionStartForegroundOpacity(opacity);
      sliceCompositeNode->SetForegroundOpacity(0.0);
      }
    else
      {
      sliceCompositeNode->SetForegroundOpacity(this->GetActionStartForegroundOpacity());
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
void vtkSliceViewInteractorStyle::OnRightButtonDown()
{
  this->SliceLogic->GetMRMLScene()->SaveStateForUndo(this->SliceLogic->GetSliceNode());
  this->SetActionState(vtkSliceViewInteractorStyle::Zoom);
  this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::FieldOfViewFlag);
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  this->SetActionStartFOV(sliceNode->GetFieldOfView());
  this->SetActionStartWindow(this->GetInteractor()->GetEventPosition());
  this->SetLastActionWindow(this->GetInteractor()->GetEventPosition());
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
  this->SetActionStartWindow(this->GetInteractor()->GetEventPosition());
  this->SetLastActionWindow(this->GetInteractor()->GetEventPosition());
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
  this->SetActionStartWindow(this->GetInteractor()->GetEventPosition());
  this->SetLastActionWindow(this->GetInteractor()->GetEventPosition());
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
void vtkSliceViewInteractorStyle::OnMouseMove()
{
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  int windowX, windowY;
  this->GetInteractor()->GetEventPosition(windowX, windowY);
  int windowW = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  int windowH = this->GetInteractor()->GetRenderWindow()->GetSize()[1];
  int windowMinSize = std::min(windowW, windowH);

  switch (this->GetActionState())
    {
    case vtkSliceViewInteractorStyle::Translate:
      {
      double xyz[3];
      sliceNode->GetXYZOrigin(xyz);

      // account for zoom using XYToSlice matrix
      this->ScratchMatrix->DeepCopy(sliceNode->GetXYToSlice());
      double deltaX = this->ScratchMatrix->GetElement(0,0)*(this->LastActionWindow[0] - windowX);
      double deltaY = this->ScratchMatrix->GetElement(1,1)*(this->LastActionWindow[1] - windowY);

      sliceNode->SetSliceOrigin(xyz[0] + deltaX, xyz[1] + deltaY, 0);
      }
      break;
    case vtkSliceViewInteractorStyle::Zoom:
      {
      int deltaY = windowY - this->GetActionStartWindow()[1];
      double percent = (windowH + deltaY) / (1.0 * windowH);

      // the factor operation is so 'z' isn't changed and the
      // slider can still move through the full range
      if ( percent > 0. )
        {
        double newFOVx = this->GetActionStartFOV()[0] * percent;
        double newFOVy = this->GetActionStartFOV()[1] * percent;
        double newFOVz = this->GetActionStartFOV()[2];
        sliceNode->SetFieldOfView( newFOVx, newFOVy, newFOVz );
        sliceNode->UpdateMatrices();
        }
      }
      break;
    case vtkSliceViewInteractorStyle::Blend:
      {
      int deltaY = windowY - this->GetActionStartWindow()[1];
      double offsetY =  (2.0 * deltaY) / windowMinSize;
      double newForegroundOpacity =
        this->GetActionStartForegroundOpacity() + offsetY;
      newForegroundOpacity = std::min(std::max(newForegroundOpacity, 0.), 1.);
      if (sliceCompositeNode->GetForegroundVolumeID() != 0)
        {
        sliceCompositeNode->SetForegroundOpacity(newForegroundOpacity);
        }
      int deltaX = windowX - this->GetActionStartWindow()[0];
      double offsetX =  (2.0 * deltaX) / windowMinSize;
      double newLabelOpacity = this->GetActionStartLabelOpacity() + offsetX;
      newLabelOpacity = std::min(std::max(newLabelOpacity, 0.), 1.);
      if (sliceCompositeNode->GetLabelVolumeID() != 0)
        {
        sliceCompositeNode->SetLabelOpacity(newLabelOpacity);
        }
      }
      break;
    case vtkSliceViewInteractorStyle::AdjustWindowLevelBackground:
    case vtkSliceViewInteractorStyle::AdjustWindowLevelForeground:
      {
      int startX = this->GetActionStartWindow()[0];
      int startY = this->GetActionStartWindow()[1];
      int deltaX = windowX - startX;
      int deltaY = windowY - startY;

      double rangeLow = this->GetActionStartVolumeRangeLow();
      double rangeHigh = this->GetActionStartVolumeRangeHigh();
      double gain = (rangeHigh - rangeLow) / 500.0;
      double newWindow = this->GetActionStartVolumeWindow() + (gain * deltaX);
      if (newWindow < 0) newWindow = 0;
      double newLevel = this->GetActionStartVolumeLevel() + (gain * deltaY);
      if (this->GetActionState() == vtkSliceViewInteractorStyle::AdjustWindowLevelBackground)
        {
        this->SliceLogic->SetBackgroundWindowLevel(newWindow, newLevel);
        }
      else
        {
        this->SliceLogic->SetForegroundWindowLevel(newWindow, newLevel);
        }
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
  this->LastActionWindow[0] = windowX;
  this->LastActionWindow[1] = windowY;
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseWheelForward()
{
  this->IncrementSlice();
  this->Superclass::OnMouseWheelForward();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseWheelBackward()
{
  this->DecrementSlice();
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
  this->GetActionStartSliceToRAS()->DeepCopy(sliceNode->GetSliceToRAS());
  this->GetActionStartXYToRAS()->DeepCopy(sliceNode->GetXYToRAS());
  double eventRAS[4];
  this->GetEventRAS(eventRAS);
  this->SetActionStartRAS(eventRAS);
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
  this->SetActionStartForegroundOpacity(sliceCompositeNode->GetForegroundOpacity());
  this->SetActionStartLabelOpacity(sliceCompositeNode->GetLabelOpacity());
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::EndBlend()
{
  this->SetActionState(this->None);
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
  this->SetActionStartForegroundOpacity(sliceCompositeNode->GetForegroundOpacity());
  this->SetActionStartLabelOpacity(sliceCompositeNode->GetLabelOpacity());
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

  double window = 0.0;
  double level = 0.0;
  double rangeLow = 0.0;
  double rangeHigh = 0.0;
  if (adjustForeground)
    {
    this->SetActionState(this->AdjustWindowLevelForeground);
    this->SliceLogic->GetForegroundWindowLevelAndRange(window, level, rangeLow, rangeHigh);
    }
  else
    {
    this->SetActionState(this->AdjustWindowLevelBackground);
    this->SliceLogic->GetBackgroundWindowLevelAndRange(window, level, rangeLow, rangeHigh);
    }
  this->SetActionStartVolumeWindow(window);
  this->SetActionStartVolumeLevel(level);
  this->SetActionStartVolumeRangeLow(rangeLow);
  this->SetActionStartVolumeRangeHigh(rangeHigh);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::EndAdjustWindowLevel()
{
  this->SetActionState(this->None);
  this->SetActionStartVolumeWindow(0);
  this->SetActionStartVolumeLevel(0);
}


//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::GetEventRAS(double ras[4])
{
  int windowX, windowY;

  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  this->GetInteractor()->GetEventPosition(windowX, windowY);
  vtkRenderer *pokedRenderer = this->GetInteractor()->FindPokedRenderer(windowX, windowY);

  double localXY[4];
  localXY[0] = windowX - pokedRenderer->GetOrigin()[0];
  localXY[1] = windowY - pokedRenderer->GetOrigin()[1];
  localXY[2] = 0.;
  localXY[3] = 1.;
  // map the current point from XY to RAS space
  sliceNode->GetXYToRAS()->MultiplyPoint(localXY, ras);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::GetEventRASWithRespectToEventStart(double ras[4])
{
  int windowX, windowY;

  this->GetInteractor()->GetEventPosition(windowX, windowY);
  vtkRenderer *pokedRenderer = this->GetInteractor()->FindPokedRenderer(windowX, windowY);

  double localXY[4];
  localXY[0] = windowX - pokedRenderer->GetOrigin()[0];
  localXY[1] = windowY - pokedRenderer->GetOrigin()[1];
  localXY[2] = 0.;
  localXY[3] = 1.;
  // map the current point from XY to RAS space
  this->GetActionStartXYToRAS()->MultiplyPoint(localXY, ras);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::GetEventXYZ(double xyz[4])
{
  int windowX, windowY;
  int windowW, windowH;
  this->GetInteractor()->GetEventPosition(windowX, windowY);

  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();

  int *size = this->GetInteractor()->GetRenderWindow()->GetSize();
  windowW = size[0];
  windowH = size[1];
  int numRows = sliceNode->GetLayoutGridRows();
  int numCols = sliceNode->GetLayoutGridColumns();

  if ( windowW == 0 || windowH == 0 )
    {
    // degenerate case, return gracefully
    xyz[0] = xyz[1] = xyz[2] = 0.;
    xyz[3] = 1.;
    return;
    }

  vtkRenderer *pokedRenderer = this->GetInteractor()->FindPokedRenderer(windowX, windowY);
  int *origin = pokedRenderer->GetOrigin();
  xyz[0] = windowX - origin[0];
  xyz[1] = windowY - origin[1];

  double tx = windowX / (1. * windowW);
  double ty = (windowH - windowY) / (1. * windowH);
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
