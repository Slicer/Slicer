#include "Widgets/vtkAnnotationRulerRepresentation.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkRenderer.h>
#include <vtkAxisActor2D.h>
#include <vtkWindow.h>
#include <vtkAxisActor2D.h>
#include <vtkHandleRepresentation.h>

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

    this->Point1Representation->GetWorldPosition(p1);
    this->Point2Representation->GetWorldPosition(p2);

    if (this->m_Distance > 0)
      {
      this->Distance = this->m_Distance;
      }
    else
      {
      this->Distance = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));
      }

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
