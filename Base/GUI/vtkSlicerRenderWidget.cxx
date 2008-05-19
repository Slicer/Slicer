/*=========================================================================

  Module:    $RCSfile: vtkKWRenderWidget.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSlicerRenderWidget.h"

#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWGenericRenderWindowInteractor.h"


vtkStandardNewMacro(vtkSlicerRenderWidget);
vtkCxxRevisionMacro(vtkSlicerRenderWidget, "$Revision: 1.163 $");

vtkSlicerRenderWidget::vtkSlicerRenderWidget()
{
}

//----------------------------------------------------------------------------
vtkSlicerRenderWidget::~vtkSlicerRenderWidget()
{
}
//----------------------------------------------------------------------------
void vtkSlicerRenderWidget::ExposeCallback()
{
  if (this->InExpose)
    {
    return;
    }
  
  this->InExpose = 1;

  vtkRenderWindowInteractor *interactor = this->GetRenderWindowInteractor();

  vtkGenericRenderWindowInteractor *gen_interactor = 
    vtkGenericRenderWindowInteractor::SafeDownCast(interactor);
  if (gen_interactor)
    {
    gen_interactor->ExposeEvent();
    }

  // If the renderwindow interactor has never been resized, do it now

  if (interactor->GetSize()[0] == 0 && interactor->GetSize()[1] == 0)
    {
    int width = 0, height = 0;
    if (vtkKWTkUtilities::GetWidgetSize(this->VTKWidget, &width, &height) &&
        width && height)
      {
      this->UpdateRenderWindowInteractorSize(width, height);
      }
    }

  this->InExpose = 0;
}

//----------------------------------------------------------------------------
void vtkSlicerRenderWidget::UpdateRenderWindowInteractorSize(int width, int height)
{
  vtkDebugMacro("UpdateRenderWindowInteractorSize: returning as this call was causing slicer 3d and 2d windows to be pixelated on linux and solaris");
  return;
  
  vtkRenderWindowInteractor *interactor = this->GetRenderWindowInteractor();
  if (!interactor)
    {
    return;
    }

  // When calling the superclass's SetWidth or SetHeight, the
  // other field will be set to 1 (i.e. a width/height of 0 for a Tk frame
  // translates to a size 1 in that dimension). Fix that.

  if (width <= 1)
    {
    width = interactor->GetSize()[0];
    }
  if (height <= 1)
    {
    height = interactor->GetSize()[1];
    }

  // Interactor->GetSize() can return 0. in that case set the size to 1.

  if (width == 0) 
    {
    width = 1;
    }
  if (height == 0)
    {
    height = 1;
    }

  // We *need* to propagate the size to the vtkTkRenderWidget
  // if we specified the widget's width/height explicitly

  // SLICER: check for '10x10' error condition - 
  // some condition causes this erroneous size value to 
  // be passed in.  Work around by checking for special case
  // until the underlying problem is understood
  int frame_width = this->GetWidth();
  if (frame_width && frame_width != 10)
    {
    width = frame_width;
    }
  int frame_height = this->GetHeight();
  if (frame_height && frame_height != 10)
    {
    height = frame_height;
    }

  if (frame_width || frame_height)
    {    
    this->VTKWidget->SetConfigurationOptionAsInt("-width", width);
    this->VTKWidget->SetConfigurationOptionAsInt("-height", height);
    }

  // Propagate to the interactor too, for safety

  interactor->UpdateSize(width, height);
}

//----------------------------------------------------------------------------
void vtkSlicerRenderWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
