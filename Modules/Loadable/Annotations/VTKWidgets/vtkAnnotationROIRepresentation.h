/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkAnnotationROIRepresentation.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
///  vtkAnnotationROIRepresentation - a class defining the representation for the vtkSlicerBoxWidget2
///
/// This class is a concrete representation for the vtkSlicerBoxWidget2. It
/// represents a box with seven handles: one on each of the six faces, plus a
/// center handle. Through interaction with the widget, the box
/// representation can be arbitrarily positioned in the 3D space.
//
/// To use this representation, you normally use the PlaceWidget() method
/// to position the widget at a specified region in space.
//
/// .SECTION Caveats
/// This class, and vtkSlicerBoxWidget2, are second generation VTK
/// widgets. An earlier version of this functionality was defined in the
/// class vtkSlicerBoxWidget.

/// .SECTION See Also
/// vtkSlicerBoxWidget2 vtkSlicerBoxWidget


#ifndef __vtkAnnotationROIRepresentation_h
#define __vtkAnnotationROIRepresentation_h

// Annotations includes
#include "vtkSlicerAnnotationsModuleVTKWidgetsExport.h"

// VTK includes
#include "vtkWidgetRepresentation.h"

class vtkActor;
class vtkBox;
class vtkCellPicker;
class vtkDoubleArray;
class vtkLineSource;
class vtkMatrix4x4;
class vtkPlanes;
class vtkPointHandleRepresentation3D;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataAlgorithm;
class vtkPolyDataMapper;
class vtkProperty;
class vtkSphereSource;
class vtkTransform;

class VTK_SLICER_ANNOTATIONS_MODULE_VTKWIDGETS_EXPORT
vtkAnnotationROIRepresentation
  : public vtkWidgetRepresentation
{
public:
  ///
  /// Instantiate the class.
  static vtkAnnotationROIRepresentation *New();

  ///
  /// Standard methods for the class.
  vtkTypeMacro(vtkAnnotationROIRepresentation,vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static const int NUMBER_HANDLES = 7;

  ///
  /// Get the planes describing the implicit function defined by the box
  /// widget. The user must provide the instance of the class vtkPlanes. Note
  /// that vtkPlanes is a subclass of vtkImplicitFunction, meaning that it can
  /// be used by a variety of filters to perform clipping, cutting, and
  /// selection of data.  (The direction of the normals of the planes can be
  /// reversed enabling the InsideOut flag.)
  void GetPlanes(vtkPlanes *planes);

  ///
  /// Set/Get the InsideOut flag. This data member is used in conjunction
  /// with the GetPlanes() method. When off, the normals point out of the
  /// box. When on, the normals point into the hexahedron.  InsideOut is off
  /// by default.
  vtkSetMacro(InsideOut,int);
  vtkGetMacro(InsideOut,int);
  vtkBooleanMacro(InsideOut,int);

  /// Get/Set optional world to local coordinate system transformation
  void SetWorldToLocalMatrix(vtkMatrix4x4 *worldToLocalMatrix);
  vtkGetObjectMacro(WorldToLocalMatrix,vtkMatrix4x4);

  ///
  /// Retrieve a linear transform characterizing the transformation of the
  /// box. Note that the transformation is relative to where PlaceWidget()
  /// was initially called. This method modifies the transform provided. The
  /// transform can be used to control the position of vtkProp3D's, as well as
  /// other transformation operations (e.g., vtkTranformPolyData).
  virtual void GetTransform(vtkTransform *t);

  ///
  /// Set the position, scale and orientation of the box widget using the
  /// transform specified. Note that the transformation is relative to
  /// where PlaceWidget() was initially called (i.e., the original bounding
  /// box).
  virtual void SetTransform(vtkTransform* t);

  ///
  /// Grab the polydata (including points) that define the box widget. The
  /// polydata consists of 6 quadrilateral faces and 15 points. The first
  /// eight points define the eight corner vertices; the next six define the
  /// -x,+x, -y,+y, -z,+z face points; and the final point (the 15th out of 15
  /// points) defines the center of the box. These point values are guaranteed
  /// to be up-to-date when either the widget's corresponding InteractionEvent
  /// or EndInteractionEvent events are invoked. The user provides the
  /// vtkPolyData and the points and cells are added to it.
  void GetPolyData(vtkPolyData *pd);

  ///
  /// Get the handle properties (the little balls are the handles). The
  /// properties of the handles, when selected or normal, can be
  /// specified.
  //vtkGetObjectMacro(HandleProperty,vtkProperty);
  vtkGetObjectMacro(SelectedHandleProperty,vtkProperty);

  ///
  /// Get the face properties (the faces of the box). The
  /// properties of the face when selected and normal can be
  /// set.
  vtkGetObjectMacro(FaceProperty,vtkProperty);
  vtkGetObjectMacro(SelectedFaceProperty,vtkProperty);

  ///
  /// Get the outline properties (the outline of the box). The
  /// properties of the outline when selected and normal can be
  /// set.
  vtkGetObjectMacro(OutlineProperty,vtkProperty);
  vtkGetObjectMacro(SelectedOutlineProperty,vtkProperty);

  ///
  /// Control the representation of the outline. This flag enables
  /// face wires. By default face wires are off.
  void SetOutlineFaceWires(int);
  vtkGetMacro(OutlineFaceWires,int);
  void OutlineFaceWiresOn() {this->SetOutlineFaceWires(1);}
  void OutlineFaceWiresOff() {this->SetOutlineFaceWires(0);}

  ///
  /// Control the representation of the outline. This flag enables
  /// the cursor lines running between the handles. By default cursor
  /// wires are on.
  void SetOutlineCursorWires(int);
  vtkGetMacro(OutlineCursorWires,int);
  void OutlineCursorWiresOn() {this->SetOutlineCursorWires(1);}
  void OutlineCursorWiresOff() {this->SetOutlineCursorWires(0);}

  ///
  /// Switches handles (the spheres) on or off by manipulating the underlying
  /// actor visibility.
  void HandlesOn();
  void HandlesOff();

  ///
  /// These are methods that satisfy vtkWidgetRepresentation's API.
  void PlaceWidget(double bounds[6]) override;
  void BuildRepresentation() override;
  int ComputeInteractionState(int X, int Y, int modify=0) override;
  void StartWidgetInteraction(double e[2]) override;
  void WidgetInteraction(double e[2]) override;
  double *GetBounds() override;

  ///
  /// Methods supporting, and required by, the rendering process.
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOpaqueGeometry(vtkViewport*) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport*) override;
  int HasTranslucentPolygonalGeometry() override;

  enum {Outside=0,MoveF0,MoveF1,MoveF2,MoveF3,MoveF4,MoveF5,Translating,Rotating,Scaling};

  ///
  /// The interaction state may be set from a widget (e.g., vtkSlicerBoxWidget2) or
  /// other object. This controls how the interaction with the widget
  /// proceeds. Normally this method is used as part of a handshaking
  /// process with the widget: First ComputeInteractionState() is invoked that
  /// returns a state based on geometric considerations (i.e., cursor near a
  /// widget feature), then based on events, the widget may modify this
  /// further.
  virtual void SetInteractionState(int state);

  ///
  /// get the center of the box
  void GetCenter(double center[]);

  ///
  /// get 3 extents along sides of the box
  void GetExtents(double bounds[]);

  void GetActors(vtkPropCollection *actors) override;


protected:
  vtkAnnotationROIRepresentation();
  ~vtkAnnotationROIRepresentation() override;

  /// Manage how the representation appears
  double LastEventPosition[4];

  /// the hexahedron (6 faces)
  vtkActor          *HexActor;
  vtkPolyDataMapper *HexMapper;
  vtkPolyData       *HexPolyData;
  vtkPoints         *Points;  //used by others as well
  double             N[6][3]; //the normals of the faces

  /// A face of the hexahedron
  vtkActor          *HexFace;
  vtkPolyDataMapper *HexFaceMapper;
  vtkPolyData       *HexFacePolyData;

  /// glyphs representing hot spots (e.g., handles)
  vtkActor          **Handle;
  vtkPolyDataMapper **HandleMapper;
  vtkSphereSource   **HandleGeometry;
  virtual void PositionHandles();
  virtual int HighlightHandle(vtkProp *prop); //returns cell id
  virtual void HighlightFace(int cellId);
  void HighlightOutline(int highlight);
  void ComputeNormals();
  virtual void SizeHandles();

  /// wireframe outline
  vtkActor          *HexOutline;
  vtkPolyDataMapper *OutlineMapper;
  vtkPolyData       *OutlinePolyData;

  /// Do the picking
  vtkCellPicker *HandlePicker;
  //vtkCellPicker *HexPicker;
  vtkActor *CurrentHandle;
  int      CurrentHexFace;
  vtkCellPicker *LastPicker;

  /// Transform the hexahedral points (used for rotations)
  vtkTransform *Transform;

  /// Support GetBounds() method
  vtkBox *BoundingBox;

  /// Properties used to control the appearance of selected objects and
  /// the manipulator in general.
  //vtkProperty *HandleProperty;
  vtkProperty *HandleProperties[NUMBER_HANDLES];
  vtkProperty *SelectedHandleProperty;
  vtkProperty *FaceProperty;
  vtkProperty *SelectedFaceProperty;
  vtkProperty *OutlineProperty;
  vtkProperty *SelectedOutlineProperty;
  virtual void CreateDefaultProperties();

  /// Control the orientation of the normals
  int InsideOut;
  int OutlineFaceWires;
  int OutlineCursorWires;
  void GenerateOutline();

  /// Helper methods
  virtual void Translate(double *p1, double *p2);
  virtual void Scale(double *p1, double *p2, int X, int Y);
  virtual void Rotate(int X, int Y, double *p1, double *p2, double *vpn);
  void MovePlusXFace(double *p1, double *p2);
  void MoveMinusXFace(double *p1, double *p2);
  void MovePlusYFace(double *p1, double *p2);
  void MoveMinusYFace(double *p1, double *p2);
  void MovePlusZFace(double *p1, double *p2);
  void MoveMinusZFace(double *p1, double *p2);

  /// Internal ivars for performance
  vtkPoints      *PlanePoints;
  vtkDoubleArray *PlaneNormals;
  vtkMatrix4x4   *Matrix;

  vtkMatrix4x4   *WorldToLocalMatrix;

  //"dir" is the direction in which the face can be moved i.e. the axis passing
  //through the center
  void MoveFace(double *p1, double *p2, double *dir,
                double *x1, double *x2, double *x3, double *x4,
                double *x5);
  //Helper method to obtain the direction in which the face is to be moved.
  //Handles special cases where some of the scale factors are 0.
  void GetDirection(const double Nx[3],const double Ny[3],
                    const double Nz[3], double dir[3]);


private:
  vtkAnnotationROIRepresentation(const vtkAnnotationROIRepresentation&) = delete;
  void operator=(const vtkAnnotationROIRepresentation&) = delete;
};

#endif
