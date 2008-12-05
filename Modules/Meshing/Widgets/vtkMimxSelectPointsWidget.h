/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSelectPointsWidget.h,v $
Language:  C++

Date:      $Date: 2008/08/14 05:01:52 $
Version:   $Revision: 1.11 $


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
// .NAME vtkMimxSelectPointsWidget - a widget to manipulate 3D parallelopipeds
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

#ifndef __vtkMimxSelectPointsWidget_h
#define __vtkMimxSelectPointsWidget_h

#include "vtkAbstractWidget.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxWidgetsWin32Header.h"


class vtkActor;
class vtkDataSet;
class vtkDataSetMapper;
class vtkCleanPolyData;
class vtkGeometryFilter;
class vtkGlyph3D;
class vtkIdList;
class vtkInteractorStyleRubberBandPick;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkRenderedAreaPicker;
class vtkSphereSource;
class vtkStructuredGrid;
class vtkUnstructuredGrid;
class vtkPointLocator;
class vtkPoints;

class VTK_MIMXWIDGETS_EXPORT vtkMimxSelectPointsWidget : public vtkAbstractWidget
{
 public:
  // Description:
  // Instantiate the object.
  static vtkMimxSelectPointsWidget *New();

  vtkTypeRevisionMacro(vtkMimxSelectPointsWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Override the superclass method. This is a composite widget, (it internally
  // consists of handle widgets). We will override the superclass method, so
  // that we can pass the enabled state to the internal widgets as well.
  virtual void SetEnabled(int);
  
  // Description:
  // Methods to change the whether the widget responds to interaction.
  // Overridden to pass the state to component widgets.
//  virtual void SetProcessEvents(int);

  void SetPointSelectionState(int PointState);
  vtkSetMacro(BooleanState, int);
  vtkGetObjectMacro(SelectedPointIds, vtkIdList);
  void CreateDefaultRepresentation(){}
  void ComputeSelectedPointIds(vtkDataSet *ExtractedUGrid, vtkMimxSelectPointsWidget *Self);
  void ComputeExtractedPointIds(vtkDataSet *ExtractedUGrid, vtkMimxSelectPointsWidget *Self);
  void ComputeSelectedPointIds(vtkMimxSelectPointsWidget *Self);
  int CheckGivenPointBelongsToStructuredGrid(
          vtkStructuredGrid *SGrid, double x[3], double *bounds);
  int DoAllPointsLieOnSurface(vtkStructuredGrid *SGrid, vtkPolyData *Surface, double *bounds);
  double GetSphereRadius();
  void SetSphereRadius(double Radius);
  void SetInput(vtkUnstructuredGrid *UGrid);
  void SetInputOpacity(double Val);
  void ClearSelections();
  enum BooleanType
  {
          Add = 0,
          Subtract
  };
 protected:
  vtkMimxSelectPointsWidget();
  ~vtkMimxSelectPointsWidget();

  static void CrtlMouseMoveCallback               (vtkAbstractWidget* );
  static void ShiftMouseMoveCallback               (vtkAbstractWidget* );
  static void CrtlLeftButtonDownCallback                        (vtkAbstractWidget* );
  static void CrtlLeftButtonUpCallback            (vtkAbstractWidget* );
  static void LeftButtonUpCallback            (vtkAbstractWidget* );
  static void RightButtonUpCallback                     (vtkAbstractWidget* );
  static void RightButtonDownCallback                   (vtkAbstractWidget* );

  static void SelectPointsOnSurfaceFunction(vtkMimxSelectPointsWidget *Self);
  static void SelectVisiblePointsOnSurfaceFunction(vtkMimxSelectPointsWidget *Self);
  static double ComputeSphereRadius(vtkDataSet *DataSet);
  static void SelectPointsThroughFunction(vtkMimxSelectPointsWidget *Self);
  static void SelectSinglePointFunction(vtkMimxSelectPointsWidget *Self);
  static void SelectSinglePointBelongingToAFaceFunction(vtkMimxSelectPointsWidget *Self);
  static void SelectPointsOnAFaceFunction(vtkMimxSelectPointsWidget *Self);
  static void ShiftLeftButtonUpCallback            (vtkAbstractWidget* );
  static void ShiftLeftButtonDownCallback                       (vtkAbstractWidget* );
  static void UpdateExtractedGlyphDisplay       (vtkDataSet * UGrid, 
          vtkMimxSelectPointsWidget *Self);
  static void UpdateSelectedGlyphDisplay        (vtkMimxSelectPointsWidget *Self);
  //void ComputeExtratedPointSet(vtkPoints *points);
  //BTX
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
        VTKLeftButtonDown,
        VTKMouseMove,
        ShiftLeftMouseButtonDown,
        ShiftLeftMouseButtonUp,
        ShiftLeftMouseButtonMove,
        LeftMouseButtonUp,
        RightMouseButtonDown,
        RightMouseButtonUp
    };
  //ETX
  int PointSelectionState;
  enum PointSelectionType
  {
          SelectPointsThrough = 0,
          SelectPointsOnSurface,
          SelectVisiblePointsOnSurface,
          SelectSinglePoint,
          SelectPointsBelongingToAFace
  };

  vtkInteractorStyleRubberBandPick *RubberBandStyle;
  vtkRenderedAreaPicker *AreaPicker;

  vtkUnstructuredGrid *Input;
  vtkDataSetMapper *InputMapper;
  vtkActor *InputActor;

  vtkPointSet *SelectedPointSet;
  vtkSphereSource *SelectedSphere;
  vtkGlyph3D *SelectedGlyph;
  vtkPolyDataMapper *SelectedGlyphMapper;
  vtkActor *SelectedGlyphActor;

  vtkPointSet *ExtractedPointSet;
  vtkSphereSource *ExtractedSphere;
  vtkGlyph3D *ExtractedGlyph;
  vtkPolyDataMapper *ExtractedGlyphMapper;
  vtkActor *ExtractedGlyphActor;

  vtkSphereSource *SingleSphere;
  vtkPolyDataMapper *SingleSphereMapper;
  vtkActor *SingleSphereActor;

  vtkIdType PickX0;
  vtkIdType PickY0;
  vtkIdType PickX1;
  vtkIdType PickY1;

  int PickStatus;

  vtkIdList *SelectedPointIds;
  vtkIdList *ExtractedPointIds;

  int BooleanState;
  int EditMeshState;
  enum EditMeshType
  {
          Full = 0,
          Partial
  };
  vtkPolyDataMapper *SurfaceMapper;
  vtkActor *SurfaceActor;
  vtkGeometryFilter *InputSurfaceFilter;
  vtkCleanPolyData *InputCleanPolyData;
  vtkPointLocator *PointLocator;
  vtkPoints *LocatorPoints;
private:
  vtkMimxSelectPointsWidget(const vtkMimxSelectPointsWidget&);  //Not implemented
  void operator=(const vtkMimxSelectPointsWidget&);  //Not implemented
};

#endif
