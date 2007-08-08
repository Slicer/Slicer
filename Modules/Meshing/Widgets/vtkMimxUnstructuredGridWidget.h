/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridWidget.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.8 $

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

// .NAME vtkMimxUnstructuredGridWidget - orthogonal hexahedron 3D widget
// .SECTION Description
// the 3D widget is used to interactively place the vertices of an unstructured 
// hexahedron grid. the handles are provided by spheres located at the vertices
// of the unstructured grid.

#ifndef __vtkMimxUnstructuredGridWidget_h
#define __vtkMimxUnstructuredGridWidget_h

#include "vtkWidgets.h"

#include "vtk3DWidget.h"

class vtkActor;
class vtkCellPicker;
class vtkCollection;
class vtkPlanes;
class vtkPoints;
class vtkMimxModPointWidget;
class vtkUnstructuredGrid;
class vtkDataSetMapper;
class vtkProp;
class vtkProperty;
class vtkUGPWCallback;
class vtkSphereSource;
class vtkTransform;

class VTK_MIMXWIDGETS_EXPORT vtkMimxUnstructuredGridWidget : public vtk3DWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkMimxUnstructuredGridWidget *New();

  vtkTypeRevisionMacro(vtkMimxUnstructuredGridWidget,vtk3DWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Methods that satisfy the superclass' API.
  virtual void SetEnabled(int);
  virtual void PlaceWidget(double bounds[6]);

  void PlaceWidget()
  {this->Superclass::PlaceWidget();}
  void PlaceWidget(double xmin, double xmax, double ymin, double ymax, 
    double zmin, double zmax)
  {this->Superclass::PlaceWidget(xmin,xmax,ymin,ymax,zmin,zmax);}
  void SetPoint(int i, double x[3]);

  // Description:
  void GetPlanes(vtkPlanes *planes);

  // Description:
  vtkSetMacro(InsideOut,int);
  vtkGetMacro(InsideOut,int);
  vtkBooleanMacro(InsideOut,int);

  void GetTransform(vtkTransform *t);
  
  // Description:
  // Get the handle properties (the little balls are the handles). The 
  // properties of the handles when selected and normal can be 
  // set.
  vtkGetObjectMacro(HandleProperty,vtkProperty);
  vtkGetObjectMacro(SelectedHandleProperty,vtkProperty);

  // Description:
  // Get the face properties (the faces of the box). The 
  // properties of the face when selected and normal can be 
  // set.

  // Description:
  // Get the outline properties (the outline of the box). The 
  // properties of the outline when selected and normal can be 
  // set.
  vtkGetObjectMacro(OutlineProperty,vtkProperty);
  vtkGetObjectMacro(SelectedOutlineProperty,vtkProperty);

  // Description:
  // Control the representation of the outline. This flag enables
  // face wires. By default face wires are off.
  vtkSetMacro(OutlineFaceWires,int);
  vtkGetMacro(OutlineFaceWires,int);
  vtkBooleanMacro(OutlineFaceWires,int);

  // Description:
  // Control the representation of the outline. This flag enables
  // the cursor lines running between the handles. By default cursor
  // wires are on.
  vtkSetMacro(OutlineCursorWires,int);
  vtkGetMacro(OutlineCursorWires,int);
  vtkBooleanMacro(OutlineCursorWires,int);

  // Description:
  // Control the behavior of the widget. Translation, rotation, and
  // scaling can all be enabled and disabled.
  void Execute();

  vtkSetMacro(UGrid,vtkUnstructuredGrid*);
  vtkGetMacro(UGrid,vtkUnstructuredGrid*);
  
  vtkSetMacro(HandleSize, double);
  virtual void SizeHandles();

protected:
  vtkMimxUnstructuredGridWidget();
  ~vtkMimxUnstructuredGridWidget();

  //BTX - manage the state of the widget
  int State;
  enum WidgetState
  {
    Start=0,
    MovingHandle,
    Scaling,
    Outside,
    Translate
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

  // the hexahedron (6 faces)
  vtkActor          *HexActor;
  vtkDataSetMapper *HexMapper;

  
  // glyphs representing hot spots (e.g., handles)
  vtkCollection *Handle;
//  vtkActor          **Handle;
  vtkCollection *HandleMapper;
//  vtkUnstructuredGridMapper **HandleMapper;
  vtkCollection *HandleGeometry;
//  vtkSphereSource   **HandleGeometry;
  void PositionHandles();
  void HandlesOn(double length);
  void HandlesOff();
  int HighlightHandle(vtkProp *prop); //returns cell id
  void HighlightOutline(int highlight);

  // wireframe outline
  vtkActor          *HexOutline;
//  vtkDataSetMapper *OutlineMapper;
//  vtkUnstructuredGrid       *OutlineUnstructuredGrid;
  vtkUnstructuredGrid* UGrid;

  // Do the picking
  vtkCellPicker *HandlePicker;
  vtkCellPicker *HexPicker;
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
  int ForwardEvent(unsigned long event);
  void EnablePointWidget();
  void DisablePointWidget();
  void Initialize();
  double HandleSize;
  void TranslateProp(double p1[3], double p2[3]);
private:
  vtkMimxUnstructuredGridWidget(const vtkMimxUnstructuredGridWidget&);  //Not implemented
  void operator=(const vtkMimxUnstructuredGridWidget&);  //Not implemented
};

#endif
