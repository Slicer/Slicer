#include "Widgets/vtkAnnotationRulerRepresentation3D.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkRenderer.h>
#include <vtkAxisActor2D.h>
#include <vtkWindow.h>
#include <vtkPoints.h>
#include <vtkHandleRepresentation.h>
#include <vtkVectorText.h>
#include <vtkFollower.h>
#include <vtkDoubleArray.h>
#include <vtkGlyph3D.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkAnnotationRulerRepresentation3D);
vtkCxxRevisionMacro (vtkAnnotationRulerRepresentation3D, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
void vtkAnnotationRulerRepresentation3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkAnnotationRulerRepresentation3D::vtkAnnotationRulerRepresentation3D()
{

  this->m_Distance = 0;

}

//---------------------------------------------------------------------------
vtkAnnotationRulerRepresentation3D::~vtkAnnotationRulerRepresentation3D()
{

}

//----------------------------------------------------------------------
void vtkAnnotationRulerRepresentation3D::BuildRepresentation()
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


    // Line
    this->LinePoints->SetPoint(0,p1);
    this->LinePoints->SetPoint(1,p2);
    this->LinePoints->Modified();

    // Label
    char string[512];
    double pos[3];
    sprintf(string, this->LabelFormat, this->Distance);
    this->LabelText->SetText(string);
    pos[0] = (p1[0]+p2[0])/2.0; pos[1] = (p1[1]+p2[1])/2.0; pos[2] = (p1[2]+p2[2])/2.0;
    this->LabelActor->SetPosition(pos);
    if (this->Renderer) //make the label face the camera
      {
      this->LabelActor->SetCamera( this->Renderer->GetActiveCamera() );
      }

    if (!this->LabelScaleSpecified)
      {
      // If a font size hasn't been specified by the user, scale the text
      // (font size) according to the length of the line widget.
      this->LabelActor->SetScale(
          this->Distance/20.0, this->Distance/20.0, this->Distance/20.0 );
      }

    // Ticks - generate points that are glyphed
    int i, numTicks;
    double v21[3], x[3];
    v21[0] =  p2[0] - p1[0]; v21[1] =  p2[1] - p1[1]; v21[2] =  p2[2] - p1[2];
    vtkMath::Normalize(v21);
    this->GlyphPoints->Reset();
    this->GlyphPoints->Modified();
    this->GlyphVectors->Reset();
    this->Glyph3D->SetScaleFactor(this->Distance/40);
    double distance;
    if ( this->RulerMode ) // specified tick separation
      {
      numTicks = (this->RulerDistance <= 0.0 ? 1 : (this->Distance / this->RulerDistance));
      numTicks = (numTicks > 99 ? 99 : numTicks);
      distance = this->RulerDistance;
      }
    else //evenly spaced
      {
      numTicks = this->NumberOfRulerTicks;
      distance = this->Distance / (numTicks + 1);
      }
    for (i=1; i <= numTicks; ++i)
      {
      x[0] = p1[0] + i*v21[0]*distance;
      x[1] = p1[1] + i*v21[1]*distance;
      x[2] = p1[2] + i*v21[2]*distance;
      this->GlyphPoints->InsertNextPoint(x);
      this->GlyphVectors->InsertNextTuple(v21);
      }

    this->BuildTime.Modified();
    }
}

//----------------------------------------------------------------------
void vtkAnnotationRulerRepresentation3D::SetDistance(double distance)
{
  this->m_Distance = distance;
}
