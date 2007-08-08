/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridExtractWidget.h,v $
Language:  C++

Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.14 $


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

// .NAME vtkMimxUnstructuredGridExtractWidget - orthogonal hexahedron 3D widget
// .SECTION Description
// vtkMimxUnstructuredGridExtractWidget takes in an unstructured hexahedron grid 
// and extracts lower geometric entities such as vertex, edge etc.
// the picked entity is colored in blue and the other entities in red.

#ifndef __vtkMimxUnstructuredGridExtractWidget_h
#define __vtkMimxUnstructuredGridExtractWidget_h

#include "vtkWidgets.h"

#include "vtk3DWidget.h"

class vtkActor;
class vtkCellPicker;
class vtkCollection;
class vtkDataSetMapper;
class vtkPoints;
class vtkUnstructuredGrid;
class vtkPolyDataMapper;
class vtkProp;
class vtkProperty;
class vtkUGEWCallback;
class vtkPolyData;
class vtkIdList;
class vtkPointPicker;

//#define MIMX_EXPORT __declspec( dllexport )


class VTK_MIMXWIDGETS_EXPORT vtkMimxUnstructuredGridExtractWidget : public vtk3DWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkMimxUnstructuredGridExtractWidget *New();

  vtkTypeRevisionMacro(vtkMimxUnstructuredGridExtractWidget,vtk3DWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Methods that satisfy the superclass' API.
  virtual void SetEnabled(int);  
  void GetEdge();

  vtkSetMacro(UGrid,vtkUnstructuredGrid*);
  vtkGetMacro(UGrid,vtkUnstructuredGrid*);

  vtkSetMacro(UGridActor,vtkActor*);
  vtkGetMacro(UGridActor,vtkActor*);

  vtkIdType* PointsList;
  int State;
  vtkIdType ReturnPickedCell();
  void DeleteHandles();
  virtual void SizeHandles();
  vtkGetMacro(PointIdList, vtkIdList*);
  // unstructured grid
protected:
  vtkMimxUnstructuredGridExtractWidget();
  ~vtkMimxUnstructuredGridExtractWidget();

  //BTX - manage the state of the widget
  enum WidgetState
  {
    Start=0,
    PickPoint = 1,
    PickEdge = 2,
    PickFace = 3,
    PickCell = 4,
    DeleteCell = 5,
    MergeNodes = 6,
    PickMultipleFaces = 7
  };
  //ETX

  // Handles the events
  static void ProcessEvents(vtkObject* object, 
    unsigned long event,
    void* clientdata, 
    void* calldata);

  // ProcessEvents() dispatches to these methods.
  void OnMouseMove();
  void OnLeftButtonDown();
  void OnLeftButtonUp();
  void OnMiddleButtonDown();
  void OnMiddleButtonUp();
  void OnRightButtonDown();
  void OnRightButtonUp();

  // for polydata
  vtkActor          *PolyDataActor;
  vtkPolyDataMapper *PolyDataMapper;
  vtkPolyData      *PolyData;

  // for points picked on the polydata
  // glyphs representing hot spots (e.g., handles)
  vtkCollection *Handle;
  vtkCollection *HandleMapper;
  vtkCollection *HandleGeometry;

  int HighlightCell(vtkProp *prop); //returns cell id

  // for 3D solid unstructured grid
  vtkUnstructuredGrid* UGrid;
  vtkUnstructuredGrid *UGridSet;
  vtkDataSetMapper *UGridMapperSet;
  vtkActor *UGridActorSet;
  vtkActor *UGridActor;
  // Do the picking
  vtkCellPicker *CellPicker;
  vtkPointPicker *PointPicker;
  
//  void CreateDefaultProperties();

  //vtkModPointWidget *PointWidget1;
  vtkUGEWCallback *EWCallback;
  int ForwardEvent(unsigned long event);
  void Initialize();
  void CreateDefaultProperties(){};
  void PlaceWidget(double bounds[6]){};
  void ExtractEdge();
  void ExtractFace();
  vtkIdList *CellIdList;
  vtkIdList *PointIdList;

  // definitions for highlighting any given cell
  // polydata
  vtkPolyData *PolyDataHighLight;
  vtkPolyDataMapper *PolyDataHighLightMapper;
  vtkActor *PolyDataHighLightActor;
  // unstructured grid
  vtkUnstructuredGrid *UGridHighLight;
  vtkDataSetMapper *UGridHighLightMapper;
  vtkActor *UGridHighLightActor;

private:
  vtkMimxUnstructuredGridExtractWidget(const vtkMimxUnstructuredGridExtractWidget&);  //Not implemented
  void operator=(const vtkMimxUnstructuredGridExtractWidget&);  //Not implemented
};

#endif
