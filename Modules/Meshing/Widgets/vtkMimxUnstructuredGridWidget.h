/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridWidget.h,v $
Language:  C++

Date:      $Date: 2008/07/06 20:30:14 $
Version:   $Revision: 1.21 $


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
// .NAME vtkMimxUnstructuredGridWidget - a widget to manipulate 3D parallelopipeds

#ifndef __vtkMimxUnstructuredGridWidget_h
#define __vtkMimxUnstructuredGridWidget_h

#include "vtkAbstractWidget.h"
#include "vtkUnstructuredGrid.h"
#include "vtkActor.h"
#include "vtkMimxWidgetsWin32Header.h"


class vtkActor;
class vtkCellPicker;
class vtkCollection;
class vtkPlanes;
class vtkPoints;
class vtkMimxModPointWidget;
class vtkUnstructuredGrid;
class vtkDataSetMapper;
class vtkPolyDataMapper;
class vtkProp;
class vtkProperty;
class vtkUGPWCallback;
class vtkSphereSource;
class vtkTransform;
class vtkActorCollection;

class VTK_MIMXWIDGETS_EXPORT vtkMimxUnstructuredGridWidget : public vtkAbstractWidget
{
 public:
  // Description:
  // Instantiate the object.
  static vtkMimxUnstructuredGridWidget *New();

  vtkTypeRevisionMacro(vtkMimxUnstructuredGridWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Override the superclass method. This is a composite widget, (it internally
  // consists of handle widgets). We will override the superclass method, so
  // that we can pass the enabled state to the internal widgets as well.
  virtual void SetEnabled(int);
  void SetEnabled(int, vtkMimxUnstructuredGridWidget *Self);
  vtkGetObjectMacro(HandleProperty,vtkProperty);
  vtkGetObjectMacro(SelectedHandleProperty,vtkProperty);

  void Execute(vtkMimxUnstructuredGridWidget *Self);

  vtkSetMacro(UGrid,vtkUnstructuredGrid*);
  vtkGetMacro(UGrid,vtkUnstructuredGrid*);

  vtkSetMacro(HandleSize, double);
  vtkGetMacro(HandleSize, double);
  virtual void SizeHandles(vtkMimxUnstructuredGridWidget *Self);
  virtual void SizeHandles();
  // set the completemesh - if the input is a partial mesh
  vtkSetObjectMacro(CompleteUGrid, vtkUnstructuredGrid);
  void ComputeSelectedPointIds(vtkMimxUnstructuredGridWidget *Self);
  void CreateDefaultRepresentation(){}
  void SetPoint(int i, double x[3], vtkMimxUnstructuredGridWidget *Self);
  void ComputeAverageHandleSize(vtkMimxUnstructuredGridWidget *Self);
  void ComputeSmallestEdgeLength(vtkMimxUnstructuredGridWidget *Self);
  void ComputeHandleSize(vtkMimxUnstructuredGridWidget *Self);
  
protected:
  vtkMimxUnstructuredGridWidget();
  ~vtkMimxUnstructuredGridWidget();

  //BTX - manage the state of the widget
  int State;
  enum WidgetEventIds 
  {
          Start = 0,
          Outside,
          CrtlLeftMouseButtonDown,
          CrtlLeftMouseButtonUp,
          CrtlLeftMouseButtonMove,
          CrtlRightMouseButtonDown,
          ShiftLeftMouseButtonDown,
          ShiftLeftMouseButtonUp,
          CrtlMiddleMouseButtonDown,
          CrtlMiddleMouseButtonUp
  };

  enum WidgetState
  {
          StartWidget = 0,
          OutsideWidget,
          MovingHandle,
          Scaling,
          Translate,
          MovingFace,
          MovingMultipleFace,
          MovingEdge,
          PickMultipleFace
  };
  //ETX

  static void CrtlMouseMoveCallback               (vtkAbstractWidget* );
  static void CrtlLeftButtonDownCallback                        (vtkAbstractWidget* );
  static void CrtlLeftButtonUpCallback            (vtkAbstractWidget* );
  static void ShiftLeftButtonDownCallback                       (vtkAbstractWidget* );
  static void ShiftLeftButtonUpCallback            (vtkAbstractWidget* );
  static void CrtlMiddleButtonDownCallback                      (vtkAbstractWidget* );
  static void CrtlMiddleButtonUpCallback            (vtkAbstractWidget* );

  // the hexahedron (6 faces)
  vtkActor          *HexActor;
  vtkDataSetMapper *HexMapper;

  // for picking surface
  vtkActor *SurfaceActor;
  vtkPolyDataMapper *SurfaceMapper;
  // to place a quad
  vtkActor *FaceActor;
  vtkPolyDataMapper *FaceMapper;
  // glyphs representing hot spots (e.g., handles)
  vtkCollection *Handle;
  //    vtkActor          **Handle;
  vtkCollection *HandleMapper;
  //    vtkUnstructuredGridMapper **HandleMapper;
  vtkCollection *HandleGeometry;
  //    vtkSphereSource   **HandleGeometry;
  vtkActorCollection *FaceGeometry;
  vtkActor *EdgeGeometry;
  void PositionHandles(vtkMimxUnstructuredGridWidget *Self);
  void HandlesOn(double length);
  void HandlesOff();
  int HighlightHandle(vtkProp *prop, vtkMimxUnstructuredGridWidget *Self); //returns cell id
  void HighlightOutline(int highlight, vtkMimxUnstructuredGridWidget *Self);
  void RemoveHighlightedFaces(vtkMimxUnstructuredGridWidget *Self);
  void RemoveHighlightedEdges(vtkMimxUnstructuredGridWidget *Self);

  void ShowInteriorHighlightedFaces();
  void HideInteriorHighlightedFaces();
  void HideInteriorHighlightedFace(int FaceNum);
  void ShowInteriorHighlightedFace(int FaceNum);
  void DeleteInteriorHighlightedFaces();
  void ComputeInteriorHighlightedFaces();

  int GetInputPickedCellAndFace(int PickedFace, int &CellFace);
  int GetInputPickedCompleteFace(
          int CellNum, int CellFace, int &CompleteCell, int &CompleteFace);
  vtkActorCollection *InteriorFaceGeometry;

  vtkIdList *InteriorFaceList;
  // wireframe outline
  vtkActor          *HexOutline;
  //    vtkDataSetMapper *OutlineMapper;
  //    vtkUnstructuredGrid       *OutlineUnstructuredGrid;
  vtkUnstructuredGrid* UGrid;
  vtkUnstructuredGrid *CompleteUGrid;

  // Do the picking
  vtkCellPicker *HandlePicker;
  vtkCellPicker *HexPicker;
  vtkCellPicker *SurfacePicker;
  vtkCellPicker *EdgePicker;
  vtkActor *CurrentHandle;


  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty *HandleProperty;
  vtkProperty *SelectedHandleProperty;
  vtkProperty *OutlineProperty;
  vtkProperty *SelectedOutlineProperty;
  void CreateDefaultProperties();

  // Control the orientation of the normals
  int InsideOut;
  int OutlineFaceWires;
  int OutlineCursorWires;
  void GenerateOutline();
  // Pointwidget handles
  vtkCollection* PointWidget;
  //vtkMimxModPointWidget *PointWidget1;
  vtkUGPWCallback *PWCallback;
  vtkMimxModPointWidget *CurrentPointWidget;
  void EnablePointWidget(vtkMimxUnstructuredGridWidget *Self);
  void DisablePointWidget(vtkMimxUnstructuredGridWidget *Self);
  void Initialize(vtkMimxUnstructuredGridWidget *Self);
  double HandleSize;
  void TranslateProp(double p1[3], double p2[3], vtkMimxUnstructuredGridWidget *Self);
  void HideHandles(vtkMimxUnstructuredGridWidget *Self);
  void ShowHandles(vtkMimxUnstructuredGridWidget *Self);
  double FaceNormal[3];
  double LastPickPosition[3];
  int ValidPick;
  double InitialLength;
  void TranslatePropFace(double p1[3], double p2[3], vtkMimxUnstructuredGridWidget *Self);
  void TranslatePropFaceParallel(double p1[3], double p2[3], vtkMimxUnstructuredGridWidget *Self);

  vtkIdList *SelectedCellIds;
  vtkIdList *SelectedPointIds;
  vtkActor *EdgeActor;
  void ExtractEdge(vtkMimxUnstructuredGridWidget *Self);
  //    void TranslateEdge(double p1[3], double p2[3]);
  void ComputeSelectedEdgePointIds(vtkMimxUnstructuredGridWidget *Self);
private:
  vtkMimxUnstructuredGridWidget(const vtkMimxUnstructuredGridWidget&);  //Not implemented
  void operator=(const vtkMimxUnstructuredGridWidget&);  //Not implemented
};

#endif
