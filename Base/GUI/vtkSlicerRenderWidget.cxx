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
void vtkSlicerRenderWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
