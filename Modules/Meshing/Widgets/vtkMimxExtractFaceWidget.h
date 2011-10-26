/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractFaceWidget.h,v $
Language:  C++

Date:      $Date: 2008/07/06 20:30:13 $
Version:   $Revision: 1.12 $


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
// .NAME vtkMimxExtractFaceWidget - a widget to manipulate 3D parallelopipeds

#ifndef __vtkMimxExtractFaceWidget_h
#define __vtkMimxExtractFaceWidget_h

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
class vtkPolyData;
class vtkPolyDataMapper;
class vtkProp;
class vtkProperty;
class vtkActorCollection;
class vtkIdList;
class vtkGeometryFilter;

class VTK_MIMXWIDGETS_EXPORT vtkMimxExtractFaceWidget : public vtkAbstractWidget
{
 public:
  // Description:
  // Instantiate the object.
  static vtkMimxExtractFaceWidget *New();

  vtkTypeRevisionMacro(vtkMimxExtractFaceWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Override the superclass method. This is a composite widget, (it internally
  // consists of handle widgets). We will override the superclass method, so
  // that we can pass the enabled state to the internal widgets as well.
  virtual void SetEnabled(int);

  virtual void SetInput(vtkDataSet*);
  void CreateDefaultRepresentation(){}

  //  vtkGetMacro(FacePoints, vtkIdList*);
  vtkSetObjectMacro(CompleteUGrid, vtkUnstructuredGrid);
  vtkSetObjectMacro(InputActor, vtkActor);

  vtkGetObjectMacro(PickedCellList, vtkIdList);
  vtkGetObjectMacro(PickedFaceList, vtkIdList);
  vtkGetObjectMacro(CompletePickedCellList, vtkIdList);
  vtkGetObjectMacro(CompletePickedFaceList, vtkIdList);

  void RemoveHighlightedFaces(vtkMimxExtractFaceWidget *Self);
 protected:
  vtkMimxExtractFaceWidget();
  ~vtkMimxExtractFaceWidget();

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

  void ExtractFace();
  void Initialize();

  static void ShiftLeftButtonDownCallback                       (vtkAbstractWidget* );
  static void ShiftLeftButtonUpCallback            (vtkAbstractWidget* );
  static void LeftButtonDownCallback                    (vtkAbstractWidget* );
  static void LeftButtonUpCallback            (vtkAbstractWidget* );
  static void RightButtonDownCallback                   (vtkAbstractWidget* );
  static void RightButtonUpCallback            (vtkAbstractWidget* );

  // the hexahedron (6 faces)
  vtkActor          *FaceActor;
  vtkActor                      *InputActor;
  vtkPolyDataMapper *FaceMapper;
  vtkPolyData       *FacePolyData;
  vtkActor          *SelectedFaceActor;
  vtkPolyDataMapper *SelectedFaceMapper;
  vtkCellPicker *FacePicker;

  vtkActorCollection *FaceGeometry;
  vtkActorCollection *InteriorFaceGeometry;
  vtkUnstructuredGrid *CompleteUGrid;
  vtkIdList *PickedCellList;
  vtkIdList *PickedFaceList;
  vtkIdList *CompletePickedCellList;
  vtkIdList *CompletePickedFaceList;
  vtkIdList *SurfaceCellList;
  vtkGeometryFilter *GeometryFilter;
  int GetInputPickedCellAndFace(int PickedFace, int &CellFace);
  int GetInputPickedCompleteFace(
          int CellNum, int CellFace, int &CompleteCell, int &CompleteFace);
  double LastPickPosition[3];
  void ShowInteriorHighlightedFaces();
  void HideInteriorHighlightedFaces();
  void DeleteInteriorHighlightedFaces();
  void ComputeInteriorHighlightedFaces();
private:
  vtkMimxExtractFaceWidget(const vtkMimxExtractFaceWidget&);  //Not implemented
  void operator=(const vtkMimxExtractFaceWidget&);  //Not implemented
};

#endif
