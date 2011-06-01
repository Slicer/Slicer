
// AnnotationModule/VTKWidgets includes
#include "VTKWidgets/vtkAnnotationRulerRepresentation.h"

// VTK includes
#include <vtkAxisActor2D.h>
#include <vtkHandleRepresentation.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkWindow.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkAnnotationRulerRepresentation);
vtkCxxRevisionMacro (vtkAnnotationRulerRepresentation, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
void vtkAnnotationRulerRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkAnnotationRulerRepresentation::vtkAnnotationRulerRepresentation()
{

  this->m_Distance = 0;

}

//---------------------------------------------------------------------------
vtkAnnotationRulerRepresentation::~vtkAnnotationRulerRepresentation()
{

}

//----------------------------------------------------------------------
void vtkAnnotationRulerRepresentation::BuildRepresentation()
{
  if ( this->GetMTime() > this->BuildTime ||
       this->Point1Representation->GetMTime() > this->BuildTime ||
       this->Point2Representation->GetMTime() > this->BuildTime ||
       (this->Renderer && this->Renderer->GetVTKWindow() &&
        this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime) )
    {
    this->Superclass::BuildRepresentation();

    // Okay, compute the distance and set the label
    double p1[3], p2[3];
    double displayP1[3],displayP2[3];

    this->Point1Representation->GetWorldPosition(p1);
    this->Point2Representation->GetWorldPosition(p2);
    this->Point1Representation->GetDisplayPosition(displayP1);
    this->Point2Representation->GetDisplayPosition(displayP2);

    if (this->m_Distance > 0)
      {
      this->Distance = this->m_Distance;
      }
    else
      {
      this->Distance = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));
      }

    this->AxisActor->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
    this->AxisActor->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();

    this->AxisActor->GetPoint1Coordinate()->SetValue(displayP1);
    this->AxisActor->GetPoint2Coordinate()->SetValue(displayP2);
    this->AxisActor->SetRulerMode(this->RulerMode);
    this->AxisActor->SetRulerDistance(this->RulerDistance);
    this->AxisActor->SetNumberOfLabels(this->NumberOfRulerTicks);

    char string[512];
    sprintf(string, this->LabelFormat, this->Distance);
    this->AxisActor->SetTitle(string);

    this->BuildTime.Modified();
    }
}

//----------------------------------------------------------------------
void vtkAnnotationRulerRepresentation::SetDistance(double distance)
{
  this->m_Distance = distance;
}
