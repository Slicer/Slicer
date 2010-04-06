#include "vtkMeasurementsAngleWidgetClass.h"

#include "vtkProperty.h"

#include "vtkAngleWidget.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkAngleRepresentation3D.h"
#include "vtkPolygonalSurfacePointPlacer.h"

#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSphereHandleRepresentation.h"

#include "vtkCamera.h"
#include "vtkFollower.h"

vtkStandardNewMacro (vtkMeasurementsAngleWidgetClass);
vtkCxxRevisionMacro ( vtkMeasurementsAngleWidgetClass, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkMeasurementsAngleWidgetClass::vtkMeasurementsAngleWidgetClass()
{
  this->Model1PointPlacer = vtkPolygonalSurfacePointPlacer::New();
  this->Model2PointPlacer = vtkPolygonalSurfacePointPlacer::New();
  this->ModelCenterPointPlacer = vtkPolygonalSurfacePointPlacer::New();

  // Angle Widget set up
  this->HandleRepresentation = vtkSphereHandleRepresentation::New();
  this->HandleRepresentation->GetProperty()->SetColor(1, 1, 1);
  this->HandleRepresentation->SetSphereRadius(1.0);
  this->HandleRepresentation->SetHandleSize(5);
  
  //this->Representation = vtkAngleRepresentation3D::New();
  // without calling InstantiateHandleRepresentation, was getting a crash when
  // creating a new angle node and widget, the point reps weren't instantiated
  //this->Representation->InstantiateHandleRepresentation();

  // try a sphere handle representation instead - no way to turn off the
  // scaling as zoom in and out
  //vtkSmartPointer<vtkSphereHandleRepresentation> sphereHandle = vtkSmartPointer<vtkSphereHandleRepresentation>::New();
  // if I don't set the colour here, the handles don't get updated to spheres
  // sphereHandle->GetProperty()->SetColor(0, 1, 0);

  
  this->Representation = vtkAngleRepresentation3D::New();
  this->Representation->SetHandleRepresentation(this->HandleRepresentation);
  //this->Representation->SetHandleRepresentation(sphereHandle);

      
  double textscale[3] = {10.0, 10.0, 10.0};
  this->Representation->SetTextActorScale(textscale);

  // can't get representations of the points for the angle widget
  // representation without using safe down cast
  // unfortunately, the handle representation is cloned, can't have them
  // different colours yet
  //this->Representation->GetPoint1Representation()->GetProperty()->SetColor(1, 0, 0);
  //this->Representation->GetPoint2Representation()->GetProperty()->SetColor(0, 0, 1);

  this->Widget = vtkAngleWidget::New();
  this->Widget->EnabledOff();
  this->Widget->CreateDefaultRepresentation();
  this->Widget->SetRepresentation(this->Representation);

  this->MadeNewHandleProperties = 0;
}

//---------------------------------------------------------------------------
vtkMeasurementsAngleWidgetClass::~vtkMeasurementsAngleWidgetClass()
{
  if (this->HandleRepresentation)
    {
    this->HandleRepresentation->Delete();
    this->HandleRepresentation = NULL;
    }
  if (this->Representation)
    {
    this->Representation->SetHandleRepresentation(NULL);
    this->Representation->Delete();
    this->Representation = NULL;
    }
   
  if (this->Widget)
    {
    // enabled off should remove observers
    this->Widget->EnabledOff();
    this->Widget->SetInteractor(NULL);
    this->Widget->SetRepresentation(NULL);
    this->Widget->Delete();
    this->Widget = NULL;
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
  if (this->ModelCenterPointPlacer)
    {
    this->ModelCenterPointPlacer->RemoveAllProps();
    this->ModelCenterPointPlacer->Delete();
    this->ModelCenterPointPlacer = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsAngleWidgetClass::PrintSelf ( ostream& os, vtkIndent indent )
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
  if (this->ModelCenterPointPlacer)
    {
    os << indent << "ModelCenterPointPlacer:\n";
    this->ModelCenterPointPlacer->PrintSelf(os,indent.GetNextIndent());
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
  os << indent << "Made new handle properties: " << (this->GetMadeNewHandleProperties() ? "yes" : "no") << "\n";
}

//---------------------------------------------------------------------------
void vtkMeasurementsAngleWidgetClass::SetCamera(vtkCamera *cam)
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

  vtkAngleRepresentation3D *ar = vtkAngleRepresentation3D::SafeDownCast(this->Widget->GetRepresentation());
  if (ar)
    {
    // iterate through actors and set their cameras
    //GetPoint1HanldeRepresentation();
    // get the text actor and set the camera on it
    ar->GetTextActor()->SetCamera(cam);
    }
}

//---------------------------------------------------------------------------
void vtkMeasurementsAngleWidgetClass::MakeNewHandleProperties()
{
  // have we already done this?
  if (this->GetMadeNewHandleProperties())
    {
    return;
    }
  else
    {
    vtkDebugMacro("MakeNewHandleProperties: making new handle properties");
    }
  
 // make a new property for the point 1 handle representations
  if (this->Widget == NULL ||
      this->Widget->GetRepresentation() == NULL)
    {
    vtkErrorMacro("MakeNewHandleProperties: no widget or representation");
    return;
    }

  vtkAngleRepresentation3D *angleRep = vtkAngleRepresentation3D::SafeDownCast(this->Widget->GetRepresentation());
  if (angleRep == NULL)
    {
    vtkErrorMacro("MakeNewHandleProperties: no angle representation");
    return;
    }

  if (angleRep->GetPoint1Representation() == NULL)
    {
    vtkErrorMacro("MakeNewHandleProperties: no point 1 handle representation");
    return;
    }
  vtkProperty *prop1 = vtkProperty::New();
  prop1->SetColor(0.2, 0.5, 0.8);
  vtkSphereHandleRepresentation *sphereHandle = vtkSphereHandleRepresentation::SafeDownCast(angleRep->GetPoint1Representation());
  if (sphereHandle)
    {
    sphereHandle->SetProperty(prop1);
    }
  else
    {
     vtkErrorMacro("MakeNewHandleProperties: no point 1 sphere handle representation");
    }
  prop1->Delete();

  // point 2 handle
  vtkProperty *prop2 = vtkProperty::New();
  prop2->SetColor(1.0, 0.8, 0.7);
  sphereHandle = vtkSphereHandleRepresentation::SafeDownCast(angleRep->GetPoint2Representation());
  if (sphereHandle)
    {
    sphereHandle->SetProperty(prop2);
    }
  else
    {
     vtkErrorMacro("MakeNewHandleProperties: no point 2 sphere handle representation");
    }
  prop2->Delete();

  // center handle
  vtkProperty *prop3 = vtkProperty::New();
  prop3->SetColor(1,1,1);
  sphereHandle = vtkSphereHandleRepresentation::SafeDownCast(angleRep->GetCenterRepresentation());
  if (sphereHandle)
    {
    sphereHandle->SetProperty(prop3);
    }
  else
    {
     vtkErrorMacro("MakeNewHandleProperties: no  centre sphere handle representation");
    }
  prop3->Delete();

  this->MadeNewHandlePropertiesOn();
}
