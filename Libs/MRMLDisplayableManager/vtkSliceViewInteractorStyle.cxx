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

// VTK includes
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkNew.h"

// MRML includes
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSliceCompositeNode.h"

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkSliceViewInteractorStyle, SliceLogic, vtkMRMLSliceLogic);

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSliceViewInteractorStyle, "$Revision$");
vtkStandardNewMacro(vtkSliceViewInteractorStyle);

//----------------------------------------------------------------------------
vtkSliceViewInteractorStyle::vtkSliceViewInteractorStyle()
{
  this->ActionState = vtkSliceViewInteractorStyle::None;

  this->ActionStartSliceToRAS = vtkMatrix4x4::New();
  this->ActionStartXYToRAS = vtkMatrix4x4::New();
  this->ScratchMatrix = vtkMatrix4x4::New();

  this->LastForegroundOpacity = 0;
  this->LastLabelOpacity = 0;

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
      this->SetLastLabelOpacity(opacity);
      sliceCompositeNode->SetLabelOpacity(0.0);
      }
    else
      {
      sliceCompositeNode->SetLabelOpacity(this->GetLastLabelOpacity());
      }
    }
  else if ( !strcmp(key, "t") )
    {
    double opacity = sliceCompositeNode->GetForegroundOpacity();
    if ( opacity != 0.0 )
      {
      this->SetLastForegroundOpacity(opacity);
      sliceCompositeNode->SetForegroundOpacity(0.0);
      }
    else
      {
      sliceCompositeNode->SetForegroundOpacity(this->GetLastForegroundOpacity());
      }
    }
  else if ( !strcmp(key, "s") )
    {
    vtkErrorMacro("TODO: set active lightbox/compare view slice");
    }
  else if ( !strcmp(key, "S") )
    {
    vtkErrorMacro("TODO: set active lightbox/compare view slice");
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
  this->Superclass::OnMiddleButtonDown();
}
//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMiddleButtonUp() 
{
  this->Superclass::OnMiddleButtonUp();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnLeftButtonDown() 
{
  this->Superclass::OnLeftButtonDown();
}
//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnLeftButtonUp() 
{
  this->Superclass::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseMove() 
{
  int windowX, windowY;
  int windowH;

  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  this->GetInteractor()->GetEventPosition(windowX, windowY);
  windowH = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  switch (this->GetActionState())
    {
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
        }
      }
      break;
    default:
      {
      this->Superclass::OnMouseMove();
      }
    }
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseWheelForward() 
{
  this->Superclass::OnMouseWheelForward();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseWheelBackward() 
{
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
  this->Superclass::OnLeave();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::ResizeSliceNode()
{
  // TODO
/*
#
# make sure the size of the slice matches the window size of the widget
#
itcl::body SliceSWidget::resizeSliceNode {} {
  set epsilon 1.0e-6

  if { $_layers(background,node) != "" } {
    set logic [$sliceGUI GetLogic]
    set sliceSpacing [lindex [$logic GetLowestVolumeSliceSpacing] 2]
    if { [catch "expr $sliceSpacing"] } {
      set sliceSpacing 1.0
    }
    $this configure -sliceStep $sliceSpacing
  }

  foreach {windoww windowh} [[$_interactor GetRenderWindow] GetSize] {}
  foreach {windowx windowy} [$_interactor GetEventPosition] {}
  # We should really use the pokedrenderer's size for these calculations.
  # However, viewerports in the LightBox can differ in size by a pixel.  So 
  # set the image size based on the size of renderer zero.
  #
  ###set pokedRenderer [$_interactor FindPokedRenderer $windowx $windowy]
  #set pokedRenderer [$_renderWidget GetRenderer]
  # Get the last renderer since the first corresponds to the overlay
  set nr [$_renderWidget GetNumberOfRenderers]
  set renderedid [expr $nr - 1]
  set pokedRenderer [$_renderWidget GetNthRenderer $renderedid]
  foreach {w h} [$pokedRenderer GetSize] {}


  foreach {nodeW nodeH nodeD} [$_sliceNode GetDimensions] {}
  foreach {nodefovx nodefovy nodefovz} [$_sliceNode GetFieldOfView] {}
  if { [catch "expr $nodefovx"] } {
    set nodefovx 1.0
  }
  if { [catch "expr $nodefovy"] } {
    set nodefovy 1.0
  }
  if { [catch "expr $nodefovz"] } {
    set nodefovz 1.0
  }

  if { $windoww < 1 || $windowh < 1 ||
       $w < 1 || $h < 1 || $nodeW < 1 || $nodeH < 1 ||
       $nodefovx == 0. || $nodefovx == 0.} {
    #puts "ignoring bogus resize"
  } else {
    set scaling0 [expr $w / (1. * $nodeW)]
    set scaling1 [expr $h / (1. * $nodeH)]

    set sMagnitude0 $scaling0
    if { $sMagnitude0 < 1.0 } {
       set sMagnitude0 [expr 1. / $sMagnitude0]
    }

    set sMagnitude1 $scaling1
    if { $sMagnitude1 < 1.0 } {
       set sMagnitude1 [expr 1. / $sMagnitude1]
    }

    if {$sMagnitude0 < $sMagnitude1} {
       # keep x fov the same, adjust y
       set fovx $nodefovx
       set fovy [expr $nodefovy * $scaling1 / $scaling0]
       set fovz [expr $sliceStep * $nodeD]
    } else {
       # keep y fov the same, adjust x
       set fovx [expr $nodefovx * $scaling0 / $scaling1]
       set fovy $nodefovy
       set fovz [expr $sliceStep * $nodeD]
    }

    set windowAspect [expr $h / (1. * $w)]
    set planeAspect [expr $fovy / (1. * $fovx)]
    if { [expr $windowAspect != $planeAspect] } {
      set fovx [expr $fovy / $windowAspect]
    }

    if { $fovx == $nodefovx && $fovy == $nodefovy && $fovz == $nodefovz &&
          $w == $nodeW && $h == $nodeH && [expr abs($sliceStep - ($nodefovz / (1. * $nodeD)))] < $epsilon} {
      return
    }
    set disabled [$_sliceNode GetDisableModifiedEvent]
    $_sliceNode DisableModifiedEventOn
    $_sliceNode SetDimensions $w $h $nodeD
    $_sliceNode SetFieldOfView $fovx $fovy $fovz
    $_sliceNode SetDisableModifiedEvent $disabled
    if { $disabled == 0 } {
        $_sliceNode InvokePendingModifiedEvent
        $_sliceNode DisableModifiedEventOff
    }
  }
}
*/
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

  this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::SliceToRASFlag);
  this->SliceLogic->SetSliceOffset(offset + delta);
  this->SliceLogic->EndSliceNodeInteraction();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::StartTranslate(int x, int y, int windowX, int windowY, double ras[3])
{
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();

  this->SliceLogic->GetMRMLScene()->SaveStateForUndo(sliceNode);
  this->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::SliceToRASFlag);

  this->SetActionState(this->Translate);
  this->GetActionStartSliceToRAS()->DeepCopy(sliceNode->GetSliceToRAS());
  this->GetActionStartXYToRAS()->DeepCopy(sliceNode->GetXYToRAS());
  this->SetActionStartRAS(ras);
  this->SetActionStartWindow(windowX, windowY);
  this->SetActionStartXYZ(x, y, 0);
}
//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::EndTranslate()
{
  this->SliceLogic->EndSliceNodeInteraction();
}
