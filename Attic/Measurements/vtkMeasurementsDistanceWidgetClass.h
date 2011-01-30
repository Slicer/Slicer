#ifndef __vtkMeasurementsDistanceWidgetClass_h
#define __vtkMeasurementsDistanceWidgetClass_h

#include "vtkMeasurementsWin32Header.h"

#include "vtkObject.h"

class vtkLineWidget2;
class vtkPointHandleRepresentation3D;
class vtkLineRepresentation;
class vtkPolygonalSurfacePointPlacer;
class vtkProperty;
class vtkCamera;
/// a custom class encapsulating the widget classes needed to display the
/// ruler in 3D
class VTK_MEASUREMENTS_EXPORT vtkMeasurementsDistanceWidgetClass : public vtkObject
{
public:
  static vtkMeasurementsDistanceWidgetClass* New();
  vtkTypeRevisionMacro(vtkMeasurementsDistanceWidgetClass, vtkObject);
  vtkMeasurementsDistanceWidgetClass();
  ~vtkMeasurementsDistanceWidgetClass();
  /// 
  /// print widgets to output stream
  void PrintSelf ( ostream& os, vtkIndent indent );

  /// 
  /// accessor methods
  vtkGetObjectMacro(Widget, vtkLineWidget2);
  vtkGetObjectMacro(Model1PointPlacer, vtkPolygonalSurfacePointPlacer);
  vtkGetObjectMacro(Model2PointPlacer, vtkPolygonalSurfacePointPlacer);

  ///
  /// set the camera on the widget's text actor
  void SetCamera(vtkCamera *cam);
protected:
  /// 
  /// the representation for the ruler end point handles
  vtkPointHandleRepresentation3D *HandleRepresentation;
  /// 
  /// the representation for the line
  vtkLineRepresentation *Representation;
  /// 
  /// the top level widget used to bind together the end points and the line
  vtkLineWidget2 *Widget;
  /// Descriptinon:
  /// point placers to constrain the ruler end points to a model's polydata surface
  vtkPolygonalSurfacePointPlacer *Model1PointPlacer;
  vtkPolygonalSurfacePointPlacer *Model2PointPlacer;
private:
  vtkMeasurementsDistanceWidgetClass ( const vtkMeasurementsDistanceWidgetClass& ); /// Not implemented
  void operator = ( const vtkMeasurementsDistanceWidgetClass& ); /// Not implemented
};

#endif
