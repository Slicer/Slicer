/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkAnnotationROIRepresentation2D.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
///  vtkAnnotationROIRepresentation2D - a class defining the representation for the vtkSlicerBoxWidget2
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


#ifndef __vtkAnnotationROIRepresentation2D_h
#define __vtkAnnotationROIRepresentation2D_h

// Annotations includes
#include "vtkAnnotationROIRepresentation.h"

class vtkLineSource;
class vtkSphereSource;
class vtkPropPicker;
class vtkProperty2D;
class vtkPolyData;
class vtkPoints;
class vtkPolyDataAlgorithm;
class vtkPointHandleRepresentation2D;
class vtkTransform;
class vtkPlanes;
class vtkBox;
class vtkDoubleArray;
class vtkMatrix4x4;

class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPlane;
class vtkCutter;
class vtkTransform;
class vtkTransformPolyDataFilter;


class VTK_SLICER_ANNOTATIONS_MODULE_VTKWIDGETS_EXPORT
vtkAnnotationROIRepresentation2D
  : public vtkAnnotationROIRepresentation
{
public:
  /// 
  /// Instantiate the class.
  static vtkAnnotationROIRepresentation2D *New();

  /// 
  /// Standard methods for the class.
  vtkTypeRevisionMacro(vtkAnnotationROIRepresentation2D,vtkAnnotationROIRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// Get the intersecting plane;
  vtkGetObjectMacro(IntersectionPlane,vtkPlane);

  /// Get the world to display matrix
  vtkGetObjectMacro(WorldToDisplayTransform,vtkTransform);

  ///
  /// \brief GetActors is reimplemented because there is no 3D actors
  /// \param actors the collection where the 3D actors are added to.
  ////
  virtual void GetActors(vtkPropCollection *actors);
  ///
  /// \brief GetActors2D is reimplemented to return all the 2D actors
  /// \param actors the collection where the 2D actors are added to.
  ///
  virtual void GetActors2D(vtkPropCollection *actors);

  ///
  /// \brief GetIntersectionActors is a utility function that returns only
  /// the intersection actors
  /// \param actors
  ///
  void GetIntersectionActors(vtkPropCollection *actors);

  ///
  /// \brief BuildRepresentation is reimplemented to control the handle visibility.
  ///
  virtual void BuildRepresentation();
  virtual int ComputeInteractionState(int X, int Y, int modify=0);
  virtual void WidgetInteraction(double e[2]);
  virtual void SetInteractionState(int state);

  virtual void SizeHandles();

  virtual int HighlightHandle(vtkProp *prop);
  virtual void HighlightFace(int cellId);

  void PrintIntersections(ostream& os);

protected:
  vtkAnnotationROIRepresentation2D();
  ~vtkAnnotationROIRepresentation2D();
  
  /// A face of the hexahedron
  vtkActor2D          *HexFace2D;
  vtkPolyDataMapper2D *HexFaceMapper2D;

  /// glyphs representing hot spots (e.g., handles)
  vtkActor2D          **Handle2D;
  vtkPolyDataMapper2D **HandleMapper2D;
  vtkTransformPolyDataFilter **HandleToPlaneTransformFilters;

  /// Plane/Face intersection pipelines
  vtkPlane *IntersectionPlane;
  vtkTransform *WorldToDisplayTransform;
  vtkCutter *IntersectionCutters[6];
  vtkPolyData *IntersectionFaces[6];
  vtkTransformPolyDataFilter *IntersectionPlaneTransformFilters[6];
  vtkActor2D *IntersectionActors[6];
  vtkPolyDataMapper2D *IntersectionMappers[6];

  vtkProperty2D *HandleProperties2D[NUMBER_HANDLES];
  vtkProperty2D *SelectedHandleProperty2D;
  vtkProperty2D *SelectedFaceProperty2D;
  vtkProperty2D *DefaultFaceProperty2D;

  vtkPropPicker *HandlePicker2D;
  vtkPropPicker *LastPicker2D;
  vtkActor2D *CurrentHandle2D;

  void CreateFaceIntersections();

  double ComputeHandleRadiusInWorldCoordinates(double radInPixels);

  virtual void CreateDefaultProperties();
  virtual void PositionHandles();

private:
  vtkAnnotationROIRepresentation2D(const vtkAnnotationROIRepresentation2D&);  //Not implemented
  void operator=(const vtkAnnotationROIRepresentation2D&);  //Not implemented
};

#endif
