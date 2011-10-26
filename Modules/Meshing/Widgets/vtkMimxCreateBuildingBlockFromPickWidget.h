/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxCreateBuildingBlockFromPickWidget.h,v $
Language:  C++

Date:      $Date: 2008/07/14 23:02:20 $
Version:   $Revision: 1.4 $


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
// .NAME vtkMimxCreateBuildingBlockFromPickWidget - a widget to manipulate 3D parallelopipeds
//
// .SECTION Description
// This widget was designed with the aim of visualizing / probing cuts on
// a skewed image data / structured grid. 
//
// .SECTION Interaction
// The widget allows you to create a parallelopiped (defined by 8 handles).
// The widget is initially placed by using the "PlaceWidget" method in the
// representation class. After the widget has been created, the following
// interactions may be used to manipulate it :
// 1) Click on a handle and drag it around moves the handle in space, while
//    keeping the same axis alignment of the parallelopiped
// 2) Dragging a handle with the shift button pressed resizes the piped
//    along an axis.
// 3) Control-click on a handle creates a chair at that position. (A chair
//    is a depression in the piped that allows you to visualize cuts in the
//    volume). 
// 4) Clicking on a chair and dragging it around moves the chair within the
//    piped.
// 5) Shift-click on the piped enables you to translate it.
//
// .SECTION Caveats
// .SECTION See Also vtkParallelopipedRepresentation

#ifndef __vtkMimxCreateBuildingBlockFromPickWidget_h
#define __vtkMimxCreateBuildingBlockFromPickWidget_h

#include "vtkAbstractWidget.h"
#include "vtkMimxWidgetsWin32Header.h"


class vtkActor;
class vtkDataSetMapper;
class vtkInteractorStyleRubberBandPick;
class vtkPolyData;
class vtkUnstructuredGrid;
class vtkPoints;

class VTK_MIMXWIDGETS_EXPORT vtkMimxCreateBuildingBlockFromPickWidget : public vtkAbstractWidget
{
 public:
  // Description:
  // Instantiate the object.
  static vtkMimxCreateBuildingBlockFromPickWidget *New();

  vtkTypeRevisionMacro(vtkMimxCreateBuildingBlockFromPickWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Override the superclass method. This is a composite widget, (it internally
  // consists of handle widgets). We will override the superclass method, so
  // that we can pass the enabled state to the internal widgets as well.
  virtual void SetEnabled(int);
  virtual void SetInput(vtkPolyData *Input);
 // vtkGetObjectMacro(BuildingBlock, vtkUnstructuredGrid);
  vtkUnstructuredGrid* GetBuildingBlock();
  void SetExtrusionLength(double length);
  // Description:
  // Methods to change the whether the widget responds to interaction.
  // Overridden to pass the state to component widgets.
//  virtual void SetProcessEvents(int);

  void CreateDefaultRepresentation(){}
protected:
  vtkMimxCreateBuildingBlockFromPickWidget();
  ~vtkMimxCreateBuildingBlockFromPickWidget();

  static void CrtlMouseMoveCallback               (vtkAbstractWidget* );
  static void ShiftMouseMoveCallback               (vtkAbstractWidget* );
  static void CrtlLeftButtonDownCallback                        (vtkAbstractWidget* );
  static void CrtlLeftButtonUpCallback            (vtkAbstractWidget* );
  static void LeftButtonUpCallback            (vtkAbstractWidget* );
  static void RightButtonDownCallback                   (vtkAbstractWidget* );
  static void RightButtonUpCallback            (vtkAbstractWidget* );

  // Description:
  // Events invoked by this widget
  int WidgetEvent;
  enum WidgetEventIds 
    {
    Start = 0,
        Outside,
        CrtlLeftMouseButtonDown,
        CrtlLeftMouseButtonUp,
        CrtlLeftMouseButtonMove,
        LeftMouseButtonUp,
        RightMouseButtonDown,
        RightMouseButtonUp
    };

  vtkIdType PickX0;
  vtkIdType PickY0;
  vtkIdType PickX1;
  vtkIdType PickY1;
  
  double normal[3];

  vtkInteractorStyleRubberBandPick *RubberBandStyle;
  vtkUnstructuredGrid *BuildingBlock;
  vtkDataSetMapper *BuildingBlockMapper;
  int PickStatus;
  vtkActor *BuildingBlockActor;
  vtkPolyData *Input;
  double ExtrusionLength;
private:
  vtkMimxCreateBuildingBlockFromPickWidget(const vtkMimxCreateBuildingBlockFromPickWidget&);  //Not implemented
  void operator=(const vtkMimxCreateBuildingBlockFromPickWidget&);  //Not implemented
  
  void OrderBuildingBlockPoints( double *, double *, double *, double *, 
                    double *, double *, double *, double *, vtkPoints *);
    
  double pickPoint1[3];
  double pickPoint2[3];
  double point1[3];
  double point2[3];
};

#endif
