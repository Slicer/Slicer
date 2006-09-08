/*=========================================================================

  Module:    $RCSfile: vtkKWMatrix4x4.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMatrix4x4 -  multi column list box to display/edit matrix4x4
// .SECTION Description
// vtkKWMatrix4x4 is a widget containing widgets that help view and
// edit a matrix
//

#ifndef __vtkKWMatrix4x4_h
#define __vtkKWMatrix4x4_h

#include "vtkCallbackCommand.h"

#include "vtkKWCompositeWidget.h"

#include "vtkMatrix4x4.h" // Needed for some constants
#include "vtkKWMultiColumnList.h" // Needed for some constants

#include "vtkSlicerBaseGUI.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkKWMatrix4x4 : public vtkKWCompositeWidget
{
public:
  static vtkKWMatrix4x4* New();
  vtkTypeRevisionMacro(vtkKWMatrix4x4,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // the vtk object to manipulate
  vtkGetObjectMacro (Matrix4x4, vtkMatrix4x4);
  vtkSetObjectMacro (Matrix4x4, vtkMatrix4x4);
  void SetAndObserveMatrix4x4(vtkMatrix4x4 *matrix);

  // Description:
  // the listbox widget
  vtkGetObjectMacro (MultiColumnList, vtkKWMultiColumnList);
  vtkSetObjectMacro (MultiColumnList, vtkKWMultiColumnList);

  // Description:
  // Update the widget with the current values of the Matrix
  void UpdateWidget();
  // Description:
  // Update the matrix with the current values of the widget
  void UpdateVTK();

  void UpdateElement (int row, int col, double val);
  
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
  vtkKWMatrix4x4();
  virtual ~vtkKWMatrix4x4();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  char *Command;
  //char *StartCommand;
  //char *EndCommand;

  vtkMatrix4x4 *Matrix4x4;

  vtkKWMultiColumnList *MultiColumnList;

private:
  vtkKWMatrix4x4(const vtkKWMatrix4x4&); // Not implemented
  void operator=(const vtkKWMatrix4x4&); // Not implemented

  vtkCallbackCommand *MRMLCallbackCommand;

};

#endif

