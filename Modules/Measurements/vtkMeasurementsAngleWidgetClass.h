#ifndef __vtkMeasurementsAngleWidgetClass_h
#define __vtkMeasurementsAngleWidgetClass_h

#include "vtkMeasurementsWin32Header.h"

#include "vtkObject.h"

class vtkAngleWidget;
class vtkSphereHandleRepresentation;
class vtkAngleRepresentation3D;
class vtkPolygonalSurfacePointPlacer;
class vtkProperty;
class vtkCamera;
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
  vtkGetObjectMacro(Widget, vtkAngleWidget);
  vtkGetObjectMacro(Model1PointPlacer, vtkPolygonalSurfacePointPlacer);
  vtkGetObjectMacro(Model2PointPlacer, vtkPolygonalSurfacePointPlacer);
  vtkGetObjectMacro(ModelCenterPointPlacer, vtkPolygonalSurfacePointPlacer);

  vtkBooleanMacro( MadeNewHandleProperties, int);
  vtkGetMacro ( MadeNewHandleProperties, int );
  vtkSetMacro ( MadeNewHandleProperties, int );

  ///
  /// set the camera on the widget's text actor
  void SetCamera(vtkCamera *cam);

  ///
  /// make new handle properties so can set them to different colours
  void MakeNewHandleProperties();

protected:
  /// 
  /// the representation for the angle end point handles
  vtkSphereHandleRepresentation *HandleRepresentation;
  /// 
  /// the representation for the line
  vtkAngleRepresentation3D *Representation;
  /// 
  /// the top level widget used to bind together the end points and the line
  vtkAngleWidget *Widget;
  ///
  /// point placers to constrain the angle end points to a model's polydata surface
  vtkPolygonalSurfacePointPlacer *Model1PointPlacer;
  vtkPolygonalSurfacePointPlacer *Model2PointPlacer;
  vtkPolygonalSurfacePointPlacer *ModelCenterPointPlacer;
  
private:
  vtkMeasurementsAngleWidgetClass ( const vtkMeasurementsAngleWidgetClass& ); /// Not implemented
  void operator = ( const vtkMeasurementsAngleWidgetClass& ); /// Not
                                                              /// implemented

  /// a flag set to 1 when this class has made new handle properties
  int MadeNewHandleProperties;
};

#endif
