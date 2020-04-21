
// Annotations includes
#include "vtkAnnotationBidimensionalRepresentation.h"

// VTK includes
#include <vtkAxisActor2D.h>
#include <vtkCellArray.h>
#include <vtkHandleRepresentation.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>
#include <vtkTextMapper.h>
#include <vtkWindow.h>

// STD includes
#include <sstream>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkAnnotationBidimensionalRepresentation);

//---------------------------------------------------------------------------
void vtkAnnotationBidimensionalRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkAnnotationBidimensionalRepresentation::vtkAnnotationBidimensionalRepresentation()
{

  this->m_Distance1 = 0;
  this->m_Distance2 = 0;

}

//---------------------------------------------------------------------------
vtkAnnotationBidimensionalRepresentation::~vtkAnnotationBidimensionalRepresentation() = default;

//----------------------------------------------------------------------------
void vtkAnnotationBidimensionalRepresentation::BuildRepresentation()
{

  if ( this->GetMTime() > this->BuildTime ||
       this->Point1Representation->GetMTime() > this->BuildTime ||
       this->Point2Representation->GetMTime() > this->BuildTime ||
       this->Point3Representation->GetMTime() > this->BuildTime ||
       this->Point4Representation->GetMTime() > this->BuildTime ||
       (this->Renderer && this->Renderer->GetVTKWindow() &&
        this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime) )
    {
    this->Point1Representation->BuildRepresentation();
    this->Point2Representation->BuildRepresentation();
    this->Point3Representation->BuildRepresentation();
    this->Point4Representation->BuildRepresentation();

    // Now bring the lines up to date
    if ( ! this->Line1Visibility )
      {
      return;
      }

    char distStr1[256], distStr2[256];
    double p1[3], p2[3], p3[3], p4[3];
    this->GetPoint1DisplayPosition(p1);
    this->GetPoint2DisplayPosition(p2);
    this->GetPoint3DisplayPosition(p3);
    this->GetPoint4DisplayPosition(p4);

    double wp1[3], wp2[3], wp3[3], wp4[3];
    this->GetPoint1WorldPosition(wp1);
    this->GetPoint2WorldPosition(wp2);
    this->GetPoint3WorldPosition(wp3);
    this->GetPoint4WorldPosition(wp4);

    this->LinePoints->SetPoint(0,p1);
    this->LinePoints->SetPoint(1,p2);
    this->LinePoints->SetPoint(2,p3);
    this->LinePoints->SetPoint(3,p4);
    this->LinePoints->Modified();

    this->LineCells->Reset();
    this->LineCells->InsertNextCell(2);
    this->LineCells->InsertCellPoint(0);
    this->LineCells->InsertCellPoint(1);

    if ( this->Line2Visibility )
      {
      this->LineCells->InsertNextCell(2);
      this->LineCells->InsertCellPoint(2);
      this->LineCells->InsertCellPoint(3);
      }

    double line1Dist;

    if (this->m_Distance1 > 0)
      {
      line1Dist = this->m_Distance1;
      }
    else
      {
      line1Dist = sqrt(vtkMath::Distance2BetweenPoints(wp1, wp2));
      }

    double line2Dist = 0;
    if (this->Line2Visibility)
      {

      if (this->m_Distance2 > 0)
        {
        line2Dist = this->m_Distance2;
        }
      else
        {
        line2Dist = sqrt(vtkMath::Distance2BetweenPoints(wp3, wp4));
        }

      }
    std::ostringstream label;
    if (this->IDInitialized)
      {
      label << this->ID << ": ";
      }
    sprintf(distStr1,this->LabelFormat, line1Dist);
    sprintf(distStr2,this->LabelFormat, line2Dist);

    if (line1Dist > line2Dist)
      {
      label << distStr1 << " x " << distStr2;
      }
    else
      {
      label << distStr2 << " x " << distStr1;
      }
    this->TextMapper->SetInput(label.str().c_str());

    // Adjust the font size
    int stringSize[2], *winSize = this->Renderer->GetSize();
    vtkTextMapper::SetRelativeFontSize(this->TextMapper, this->Renderer, winSize,
                                       stringSize, 0.015);

    int maxX = VTK_INT_MIN, maxY = VTK_INT_MIN;
    if (p1[1] > maxY)
      {
      maxX = static_cast<int>(p1[0]);
      maxY = static_cast<int>(p1[1]);
      }
    if (p2[1] > maxY)
      {
      maxX = static_cast<int>(p2[0]);
      maxY = static_cast<int>(p2[1]);
      }
    if (p3[1] > maxY)
      {
      maxX = static_cast<int>(p3[0]);
      maxY = static_cast<int>(p3[1]);
      }
    if (p4[1] > maxY)
      {
      maxX = static_cast<int>(p4[0]);
      maxY = static_cast<int>(p4[1]);
      }
    int minX = VTK_INT_MAX, minY = VTK_INT_MAX;
    if (p1[1] < minY)
      {
      minX = static_cast<int>(p1[0]);
      minY = static_cast<int>(p1[1]);
      }
    if (p2[1] < minY)
      {
      minX = static_cast<int>(p2[0]);
      minY = static_cast<int>(p2[1]);
      }
    if (p3[1] < minY)
      {
      minX = static_cast<int>(p3[0]);
      minY = static_cast<int>(p3[1]);
      }
    if (p4[1] < minY)
      {
      minX = static_cast<int>(p4[0]);
      minY = static_cast<int>(p4[1]);
      }
    int textSize[2];
    this->TextMapper->GetSize(this->Renderer, textSize);
    if (this->ShowLabelAboveWidget)
      {
      this->TextActor->SetPosition(maxX - textSize[0]/2, maxY+9);
      }
    else
      {
      this->TextActor->SetPosition(minX - textSize[0]/2, minY-(textSize[1]+9));
      }

    this->BuildTime.Modified();
    }
}

//----------------------------------------------------------------------
void vtkAnnotationBidimensionalRepresentation::SetDistance1(double distance)
{
  this->m_Distance1 = distance;
}

//----------------------------------------------------------------------
void vtkAnnotationBidimensionalRepresentation::SetDistance2(double distance)
{
  this->m_Distance2 = distance;
}
