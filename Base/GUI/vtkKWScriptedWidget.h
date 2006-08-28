/*=========================================================================

  Module:    $RCSfile: vtkKWScriptedWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWScriptedWidget -  multi column list box to display/edit matrix4x4
// .SECTION Description
// vtkKWScriptedWidget uses a tcl script to create and manage widgets
// 
//

#ifndef __vtkKWScriptedWidget_h
#define __vtkKWScriptedWidget_h

#include "vtkKWCompositeWidget.h"

#include "vtkSlicerBaseGUI.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkKWScriptedWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWScriptedWidget* New();
  vtkTypeRevisionMacro(vtkKWScriptedWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetStringMacro(InitializeCommand);
  vtkGetStringMacro(InitializeCommand);

  // Description:
  // Update the widget with the current values of the Matrix
  void UpdateWidget();
  // Description:
  // Update the matrix with the current values of the widget
  void UpdateVTK();
  
  // Description:
  // Command to call when the User manipulates the widget
  virtual void SetCommand(vtkObject *object, const char *method);

  // TODO: have special commands for start/end events
  //virtual void SetStartCommand(vtkObject *object, const char *method);
  //virtual void SetEndCommand(vtkObject *object, const char *method);

  // Description:
  // TODO: access internal widgets
  //vtkKWRange* GetXRange() { return this->Range[0]; };

protected:
  vtkKWScriptedWidget();
  virtual ~vtkKWScriptedWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  char *Command;

  char *InitializeCommand;

private:
  vtkKWScriptedWidget(const vtkKWScriptedWidget&); // Not implemented
  void operator=(const vtkKWScriptedWidget&); // Not implemented
};

#endif

