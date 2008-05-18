/*=========================================================================

  Module:    $RCSfile: vtkSlicerRenderWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSlicerRenderWidget - a render widget
// .SECTION Description
// Almost empty subclass of vtkKWRenderWidget that 
// disables the ProcessPendingEvents and Render
// in the Expose callback

#ifndef __vtkSlicerRenderWidget_h
#define __vtkSlicerRenderWidget_h

#include "vtkKWRenderWidget.h"

#include "vtkSlicerBaseGUIWin32Header.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerRenderWidget : public vtkKWRenderWidget
{
public:
  static vtkSlicerRenderWidget* New();
  vtkTypeRevisionMacro(vtkSlicerRenderWidget, vtkKWRenderWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Override 'update' behavior in superclass
  virtual void ExposeCallback();

  // Description:
  // Update the render window interactor size
  // override bug with 10 x 10 window size
  virtual void UpdateRenderWindowInteractorSize(int width, int height);

protected:
  vtkSlicerRenderWidget();
  ~vtkSlicerRenderWidget();

private:
  vtkSlicerRenderWidget(const vtkSlicerRenderWidget&);  // Not implemented
  void operator=(const vtkSlicerRenderWidget&);  // Not implemented
};

#endif

