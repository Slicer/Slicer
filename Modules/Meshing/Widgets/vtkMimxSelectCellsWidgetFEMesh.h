/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSelectCellsWidgetFEMesh.h,v $
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
// .NAME vtkMimxSelectCellsWidgetFEMesh - a widget to manipulate 3D parallelopipeds
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

#ifndef __vtkMimxSelectCellsWidgetFEMesh_h
#define __vtkMimxSelectCellsWidgetFEMesh_h

#include "vtkAbstractWidget.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxWidgetsWin32Header.h"

class vtkActor;
class vtkDataSet;
class vtkGlyph3D;
class vtkIdList;
class vtkInteractorStyleRubberBandPick;
class vtkPolyDataMapper;
class vtkRenderedAreaPicker;
class vtkUnstructuredGrid;

class VTK_MIMXWIDGETS_EXPORT vtkMimxSelectCellsWidgetFEMesh : public vtkAbstractWidget
{
 public:
  // Description:
  // Instantiate the object.
  static vtkMimxSelectCellsWidgetFEMesh *New();

  vtkTypeRevisionMacro(vtkMimxSelectCellsWidgetFEMesh,vtkAbstractWidget);
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

  vtkSetMacro(CellSelectionState, int);
  vtkGetObjectMacro(SelectedCellIds, vtkIdList);
  vtkGetObjectMacro(SelectedPointIds, vtkIdList);
  vtkGetObjectMacro(ExtractedSurfaceActor, vtkActor);
  void SetInput(vtkUnstructuredGrid *Ugrid);
  void CreateDefaultRepresentation(){}
  void ComputeSelectedPointIds(vtkDataSet *ExtractedUGrid, vtkMimxSelectCellsWidgetFEMesh *Self);

protected:
  vtkMimxSelectCellsWidgetFEMesh();
  ~vtkMimxSelectCellsWidgetFEMesh();

  static void CrtlMouseMoveCallback               (vtkAbstractWidget* );
  static void CrtlLeftButtonDownCallback                        (vtkAbstractWidget* );
  static void CrtlLeftButtonUpCallback            (vtkAbstractWidget* );
  static void LeftButtonUpCallback            (vtkAbstractWidget* );
  static void SelectCellsOnSurfaceFunction(vtkMimxSelectCellsWidgetFEMesh *Self);
  static void SelectVisibleCellsOnSurfaceFunction(vtkMimxSelectCellsWidgetFEMesh *Self);
  static void SelectCellsThroughFunction(vtkMimxSelectCellsWidgetFEMesh *Self);
  static void SelectSingleCellFunction(vtkMimxSelectCellsWidgetFEMesh *Self);
  static void ExtractSurfaceFunction(vtkMimxSelectCellsWidgetFEMesh *Self);
  static void ExtractVisibleSurfaceFunction(vtkMimxSelectCellsWidgetFEMesh *Self);
  void ComputeSelectedCellIds(
        vtkMimxSelectCellsWidgetFEMesh *self, vtkDataSet *DataSet);
  int ComputeOriginalCellNumber(
          vtkMimxSelectCellsWidgetFEMesh *self, vtkIdList *PtIds);
  vtkIdList *SelectedCellIds;
  vtkIdList *SelectedPointIds;
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
        LeftMouseButtonUp
    };
  enum CellSelectionType
  {
          SelectCellsThrough = 0,
          SelectCellsOnSurface,
          SelectVisibleCellsOnSurface,
          SelectSingleCell,
          ExtractSurface,
          ExtractVisibleSurface
  };
  
  enum MeshSelectionType
  {
          MeshHexahedron = 0,
          MeshTetrahedron,
          MeshQuadrangle,
          MeshTriangle
  };
  
   int CellSelectionState;
  vtkInteractorStyleRubberBandPick *RubberBandStyle;
  vtkRenderedAreaPicker *AreaPicker;

  vtkUnstructuredGrid *Input;
  vtkPolyDataMapper *Mapper;

  vtkIdType PickX0;
  vtkIdType PickY0;
  vtkIdType PickX1;
  vtkIdType PickY1;
  vtkIdType PickStatus;
  vtkActor *InputActor;
  vtkActor *ExtractedVolumeActor;
  vtkActor *ExtractedSurfaceActor;
  vtkActor *SingleCellActor;
private:
  vtkMimxSelectCellsWidgetFEMesh(const vtkMimxSelectCellsWidgetFEMesh&);  //Not implemented
  void operator=(const vtkMimxSelectCellsWidgetFEMesh&);  //Not implemented
  
  int MeshType;
};

#endif
