#include "vtkMeasurementsDistanceWidgetClass.h"

#include "vtkProperty.h"

#include "vtkLineWidget2.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkLineRepresentation.h"
#include "vtkPolygonalSurfacePointPlacer.h"

#include "vtkObjectFactory.h"
#include "vtkCamera.h"
#include "vtkFollower.h"

vtkStandardNewMacro (vtkMeasurementsDistanceWidgetClass);
vtkCxxRevisionMacro ( vtkMeasurementsDistanceWidgetClass, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkMeasurementsDistanceWidgetClass::vtkMeasurementsDistanceWidgetClass()
{
  this->Model1PointPlacer = vtkPolygonalSurfacePointPlacer::New();
  this->Model2PointPlacer = vtkPolygonalSurfacePointPlacer::New();

  this->HandleRepresentation = vtkPointHandleRepresentation3D::New();
  this->HandleRepresentation->GetProperty()->SetColor(1, 0, 0);

  this->Representation = vtkLineRepresentation::New();
  this->Representation->SetHandleRepresentation(this->HandleRepresentation);
  this->Representation->DistanceAnnotationVisibilityOn();
  this->Representation->SetDistanceAnnotationFormat("%g mm");

   // have to set a scale or else it will scale with the length of the line
  double scale[3];
  scale[0] = 10.0;
  scale[1] = 10.0;
  scale[2] = 10.0;
  this->Representation->SetDistanceAnnotationScale(scale);
  // unfortunately, the handle representation is cloned, can't have them
  // different colours yet
  this->Representation->GetPoint1Representation()->GetProperty()->SetColor(1, 0, 0);
  this->Representation->GetPoint2Representation()->GetProperty()->SetColor(0, 0, 1);

  this->Widget = vtkLineWidget2::New();
  this->Widget->EnabledOff();
  this->Widget->CreateDefaultRepresentation();
  this->Widget->SetRepresentation(this->Representation);
}

//---------------------------------------------------------------------------
vtkMeasurementsDistanceWidgetClass::~vtkMeasurementsDistanceWidgetClass()
{
  if (this->Widget)
    {
      // enabled off should remove observers
      this->Widget->EnabledOff();
      this->Widget->SetInteractor(NULL);
    this->Widget->SetRepresentation(NULL);
    this->Widget->Delete();
    this->Widget = NULL;
    }
  if (this->Representation)
    {
    this->Representation->SetHandleRepresentation(NULL);
    this->Representation->GetPoint1Representation()->SetPointPlacer(NULL);
    this->Representation->GetPoint2Representation()->SetPointPlacer(NULL);
    //this->Representation->SetPointRepresentation(NULL);
    this->Representation->Delete();
    this->Representation = NULL;
    }
  if (this->HandleRepresentation)
    {
    this->HandleRepresentation->Delete();
    this->HandleRepresentation = NULL;
    }
  if (this->Model1PointPlacer)
     {
       this->Model1PointPlacer->RemoveAllProps();
    this->Model1PointPlacer->Delete();
    this->Model1PointPlacer = NULL;
    }
  if (this->Model2PointPlacer)
    {
      this->Model2PointPlacer->RemoveAllProps();
    this->Model2PointPlacer->Delete();
    this->Model2PointPlacer = NULL;
    }
  
  
  
}

//---------------------------------------------------------------------------
void vtkMeasurementsDistanceWidgetClass::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  this->vtkObject::PrintSelf ( os, indent );
  if (this->Model1PointPlacer)
    {
    os << indent << "Model1PointPlacer:\n";
    this->Model1PointPlacer->PrintSelf(os,indent.GetNextIndent());
    }
  if (this->Model2PointPlacer)
    {
    os << indent << "Model2PointPlacer:\n";
    this->Model2PointPlacer->PrintSelf(os,indent.GetNextIndent());
    }
  if (this->HandleRepresentation)
    {
    os << indent << "HandleRepresentation:\n";
    this->HandleRepresentation->PrintSelf(os,indent.GetNextIndent());
    }
  if (this->Representation)
    {
    os << indent << "Representation:\n";
    this->Representation->PrintSelf(os,indent.GetNextIndent());
    }
  if (this->Widget)
    {
    os << indent << "Widget:\n";
    this->Widget->PrintSelf(os,indent.GetNextIndent());
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsDistanceWidgetClass::SetCamera(vtkCamera *cam)
{
  if (!cam)
    {
    return;
    }
  if (this->Widget == NULL ||
      this->Widget->GetRepresentation() == NULL)
    {
    return;
    }

  vtkLineRepresentation *ar = vtkLineRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  if (ar)
    {
    // iterate through actors and set their cameras
    // get the text actor and set the camera on it
    ar->GetTextActor()->SetCamera(cam);
    }
}
