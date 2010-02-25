#ifndef __vtkMeasurementsAngleWidgetClass_h
#define __vtkMeasurementsAngleWidgetClass_h

#include "vtkMeasurementsWin32Header.h"

#include "vtkObject.h"

class vtkAngleWidget;
class vtkPointHandleRepresentation3D;
class vtkAngleRepresentation3D;
class vtkPolygonalSurfacePointPlacer;
class vtkProperty;
/// a custom class encapsulating the widget classes needed to display the
/// angle in 3D
class VTK_MEASUREMENTS_EXPORT vtkMeasurementsAngleWidgetClass : public vtkObject
{
public:
  static vtkMeasurementsAngleWidgetClass* New();
  vtkTypeRevisionMacro(vtkMeasurementsAngleWidgetClass, vtkObject);
  vtkMeasurementsAngleWidgetClass();
  ~vtkMeasurementsAngleWidgetClass();
  /// 
  /// print widgets to output stream
  void PrintSelf ( ostream& os, vtkIndent indent );

  /// 
  /// accessor methods
  vtkGetObjectMacro(HandleRepresentation, vtkPointHandleRepresentation3D);
  vtkGetObjectMacro(Representation, vtkAngleRepresentation3D);
  vtkGetObjectMacro(Widget, vtkAngleWidget);
  vtkGetObjectMacro(Model1PointPlacer, vtkPolygonalSurfacePointPlacer);
  vtkGetObjectMacro(Model2PointPlacer, vtkPolygonalSurfacePointPlacer);
  vtkGetObjectMacro(ModelCenterPointPlacer, vtkPolygonalSurfacePointPlacer);

protected:
  /// 
  /// the representation for the angle end point handles
  vtkPointHandleRepresentation3D *HandleRepresentation;
  /// 
  /// the representation for the line
  vtkAngleRepresentation3D *Representation;
  /// 
  /// the top level widget used to bind together the end points and the line
  vtkAngleWidget *Widget;
  /// Descriptinon:
  /// point placers to constrain the angle end points to a model's polydata surface
  vtkPolygonalSurfacePointPlacer *Model1PointPlacer;
  vtkPolygonalSurfacePointPlacer *Model2PointPlacer;
  vtkPolygonalSurfacePointPlacer *ModelCenterPointPlacer;
  
private:
  vtkMeasurementsAngleWidgetClass ( const vtkMeasurementsAngleWidgetClass& ); /// Not implemented
  void operator = ( const vtkMeasurementsAngleWidgetClass& ); /// Not implemented
};

#endif
