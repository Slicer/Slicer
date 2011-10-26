/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractCellWidget.h,v $
Language:  C++

Date:      $Date: 2008/07/06 20:30:13 $
Version:   $Revision: 1.6 $


 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// .NAME vtkMimxExtractCellWidget - a widget to manipulate 3D parallelopipeds

#ifndef __vtkMimxExtractCellWidget_h
#define __vtkMimxExtractCellWidget_h

#include "vtkAbstractWidget.h"
#include "vtkUnstructuredGrid.h"
#include "vtkActor.h"
#include "vtkMimxWidgetsWin32Header.h"


class vtkActor;
class vtkCellPicker;
class vtkCollection;
class vtkPoints;
class vtkUnstructuredGrid;
class vtkDataSetMapper;
class vtkProp;
class vtkProperty;
class vtkActorCollection;
class vtkIdList;

class VTK_MIMXWIDGETS_EXPORT vtkMimxExtractCellWidget : public vtkAbstractWidget
{
 public:
  // Description:
  // Instantiate the object.
  static vtkMimxExtractCellWidget *New();

  vtkTypeRevisionMacro(vtkMimxExtractCellWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Override the superclass method. This is a composite widget, (it internally
  // consists of handle widgets). We will override the superclass method, so
  // that we can pass the enabled state to the internal widgets as well.
  virtual void SetEnabled(int);

  virtual void SetInput(vtkDataSet*);
  void CreateDefaultRepresentation(){}

  vtkSetObjectMacro(InputActor, vtkActor);

  vtkGetObjectMacro(PickedCellList, vtkIdList);
  vtkGetObjectMacro(CompletePickedCellList, vtkIdList);

 protected:
  vtkMimxExtractCellWidget();
  ~vtkMimxExtractCellWidget();

  vtkUnstructuredGrid *UGrid;
 
  int State;
  enum WidgetEventIds 
  {
          Start = 0,
          LeftMouseButtonDown,
          LeftMouseButtonUp,
          Outside,
          ShiftLeftMouseButtonDown,
          ShiftLeftMouseButtonUp,
          RightMouseButtonDown,
          RightMouseButtonUp
  };

  enum WidgetState
  {
          StartWidget = 0,
          PickMultipleFace
  };

  void Initialize();

  static void ShiftLeftButtonDownCallback                       (vtkAbstractWidget* );
  static void ShiftLeftButtonUpCallback            (vtkAbstractWidget* );
  static void LeftButtonDownCallback                    (vtkAbstractWidget* );
  static void LeftButtonUpCallback            (vtkAbstractWidget* );
  static void RightButtonDownCallback                   (vtkAbstractWidget* );
  static void RightButtonUpCallback            (vtkAbstractWidget* );

  // the hexahedron (6 faces)
  vtkActor                      *InputActor;
  vtkCellPicker *CellPicker;

  vtkActorCollection *CellGeometry;
  vtkUnstructuredGrid *CompleteUGrid;
  vtkIdList *PickedCellList;
  vtkIdList *CompletePickedCellList;
  vtkDataSetMapper *InputMapper;
  //void ShowPickedCells();
  //void HidePickedCells();
  void DeletePickedCells();
private:
  vtkMimxExtractCellWidget(const vtkMimxExtractCellWidget&);  //Not implemented
  void operator=(const vtkMimxExtractCellWidget&);  //Not implemented
};

#endif
