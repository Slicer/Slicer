/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MarkupsModule/VTKWidgets includes
#include "vtkMarkupsGlyphSource2D.h"

// VTK includes
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

vtkStandardNewMacro(vtkMarkupsGlyphSource2D);

//----------------------------------------------------------------------------
vtkMarkupsGlyphSource2D::vtkMarkupsGlyphSource2D()
{
  this->Center[0] = 0.0;
  this->Center[1] = 0.0;
  this->Center[2] = 0.0;
  this->Scale = 1.0;
  this->Scale2 = 1.5;
  this->Color[0] = 1.0;
  this->Color[1] = 1.0;
  this->Color[2] = 1.0;
  this->Filled = 1;
  this->Cross = 0;
  this->Dash = 0;
  this->RotationAngle = 0.0;
  this->GlyphType = VTK_VERTEX_GLYPH;

  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
int vtkMarkupsGlyphSource2D::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the output
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  //Allocate storage
  vtkPoints *pts = vtkPoints::New();
  pts->Allocate(6,6);
  vtkCellArray *verts = vtkCellArray::New();
  verts->Allocate(verts->EstimateSize(1,1),1);
  vtkCellArray *lines = vtkCellArray::New();
  lines->Allocate(lines->EstimateSize(4,2),2);
  vtkCellArray *polys = vtkCellArray::New();
  polys->Allocate(polys->EstimateSize(1,4),4);
  vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
  colors->SetNumberOfComponents(3);
  colors->Allocate(2,2);

  this->ConvertColor();

  //Special options
  if (this->Dash)
    {
    int filled = this->Filled;
    this->Filled = 0;
    this->CreateDash(pts,lines,polys,colors,this->Scale2);
    this->Filled = filled;
    }
  if (this->Cross)
    {
    int filled = this->Filled;
    this->Filled = 0;
    this->CreateCross(pts,lines,polys,colors,this->Scale2);
    this->Filled = filled;
    }

  //Call the right function
  switch (this->GlyphType)
    {
    case GlyphNone:
      break;
    case GlyphStarBurst:
      this->CreateStarBurst(pts,lines,polys,colors);
      break;
    case GlyphCross:
      this->CreateCross(pts,lines,polys,colors);
      break;
    case GlyphCrossDot:
      this->CreateCross(pts,lines,polys,colors,1.0,true);
      break;
    case GlyphThickCross:
      this->CreateThickCross(pts,lines,polys,colors);
      break;
    case GlyphVertex:
      this->CreateVertex(pts,verts,colors);
      break;
    case GlyphDash:
      this->CreateDash(pts,lines,polys,colors);
      break;
    case GlyphTriangle:
      this->CreateTriangle(pts,lines,polys,colors);
      break;
    case GlyphSquare:
      this->CreateSquare(pts,lines,polys,colors);
      break;
    case GlyphCircle:
      this->CreateCircle(pts,lines,polys,colors);
      break;
    case GlyphDiamond:
      this->CreateDiamond(pts,lines,polys,colors);
      break;
    case GlyphArrow:
      this->CreateArrow(pts,lines,polys,colors);
      break;
    case GlyphThickArrow:
      this->CreateThickArrow(pts,lines,polys,colors);
      break;
    case GlyphHookedArrow:
      this->CreateHookedArrow(pts,lines,polys,colors);
      break;
    }

  this->TransformGlyph(pts);

  //Clean up
  output->SetPoints(pts);
  pts->Delete();
  pts = nullptr;

  output->SetVerts(verts);
  verts->Delete();
  verts = nullptr;

  output->SetLines(lines);
  lines->Delete();
  lines = nullptr;

  output->SetPolys(polys);
  polys->Delete();
  polys = nullptr;

  output->GetCellData()->SetScalars(colors);
  colors->Delete();
  colors = nullptr;

  return 1;
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::ConvertColor()
{
  this->RGB[0] = (unsigned char) (255.0 * this->Color[0]);
  this->RGB[1] = (unsigned char) (255.0 * this->Color[1]);
  this->RGB[2] = (unsigned char) (255.0 * this->Color[2]);
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::TransformGlyph(vtkPoints *pts)
{
  double x[3];
  int i;
  int numPts=pts->GetNumberOfPoints();

  if (this->RotationAngle == 0.0)
    {
    for (i=0; i<numPts; i++)
      {
      pts->GetPoint(i,x);
      x[0] = this->Center[0] + this->Scale * x[0];
      x[1] = this->Center[1] + this->Scale * x[1];
      pts->SetPoint(i,x);
      }
    }
  else
    {
    double angle = vtkMath::RadiansFromDegrees(this->RotationAngle);
    double xt;
    for (i=0; i<numPts; i++)
      {
      pts->GetPoint(i,x);
      xt = x[0]*cos(angle) - x[1]*sin(angle);
      x[1] = x[0]*sin(angle) + x[1]*cos(angle);
      x[0] = xt;
      x[0] = this->Center[0] + this->Scale * x[0];
      x[1] = this->Center[1] + this->Scale * x[1];
      pts->SetPoint(i,x);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateVertex(vtkPoints *pts, vtkCellArray *verts,
                                           vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[1];
  ptIds[0] = pts->InsertNextPoint(0.0,0.0,0.0);
  verts->InsertNextCell(1,ptIds);
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateCross(vtkPoints *pts, vtkCellArray *lines,
                                          vtkCellArray *polys, vtkUnsignedCharArray *colors,
                                          double scale, bool dot)
{
  vtkIdType ptIds[4];

  double radius = 0.5 * scale;
  double gapRadius = 0.2 * scale;
  if (dot)
    {
    gapRadius *= 1.5;
    }

  ptIds[0] = pts->InsertNextPoint(-radius, 0.0, 0.0);
  ptIds[1] = pts->InsertNextPoint(-gapRadius, 0.0, 0.0);
  lines->InsertNextCell(2,ptIds);
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);

  ptIds[0] = pts->InsertNextPoint(gapRadius, 0.0, 0.0);
  ptIds[1] = pts->InsertNextPoint(radius, 0.0, 0.0);
  lines->InsertNextCell(2, ptIds);
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);

  ptIds[0] = pts->InsertNextPoint(0.0, -radius, 0.0);
  ptIds[1] = pts->InsertNextPoint(0.0, -gapRadius, 0.0);
  lines->InsertNextCell(2,ptIds);
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);

  ptIds[0] = pts->InsertNextPoint(0.0, gapRadius, 0.0);
  ptIds[1] = pts->InsertNextPoint(0.0, radius, 0.0);
  lines->InsertNextCell(2, ptIds);
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);

  if (dot)
    {
    ptIds[0] = pts->InsertNextPoint(-radius * 0.1, 0.0, 0.0);
    ptIds[1] = pts->InsertNextPoint(radius * 0.1, 0.0, 0.0);
    lines->InsertNextCell(2, ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);

    ptIds[0] = pts->InsertNextPoint(0.0, -radius * 0.1, 0.0);
    ptIds[1] = pts->InsertNextPoint(0.0, radius * 0.1, 0.0);
    lines->InsertNextCell(2, ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateThickCross(vtkPoints *pts, vtkCellArray *lines,
                                               vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  if (this->Filled)
    {
    vtkIdType ptIds[4];
    ptIds[0] = pts->InsertNextPoint(-0.5, -0.1, 0.0);
    ptIds[1] = pts->InsertNextPoint(0.5, -0.1, 0.0);
    ptIds[2] = pts->InsertNextPoint(0.5,  0.1, 0.0);
    ptIds[3] = pts->InsertNextPoint(-0.5,  0.1, 0.0);
    polys->InsertNextCell(4,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    ptIds[0] = pts->InsertNextPoint(-0.1, -0.5, 0.0);
    ptIds[1] = pts->InsertNextPoint(0.1, -0.5, 0.0);
    ptIds[2] = pts->InsertNextPoint(0.1,  0.5, 0.0);
    ptIds[3] = pts->InsertNextPoint(-0.1,  0.5, 0.0);
    polys->InsertNextCell(4,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
  else
    {
    vtkIdType ptIds[13];
    ptIds[0] = pts->InsertNextPoint(-0.5, -0.1, 0.0);
    ptIds[1] = pts->InsertNextPoint(-0.1, -0.1, 0.0);
    ptIds[2] = pts->InsertNextPoint(-0.1, -0.5, 0.0);
    ptIds[3] = pts->InsertNextPoint(0.1, -0.5, 0.0);
    ptIds[4] = pts->InsertNextPoint(0.1, -0.1, 0.0);
    ptIds[5] = pts->InsertNextPoint(0.5, -0.1, 0.0);
    ptIds[6] = pts->InsertNextPoint(0.5,  0.1, 0.0);
    ptIds[7] = pts->InsertNextPoint(0.1,  0.1, 0.0);
    ptIds[8] = pts->InsertNextPoint(0.1,  0.5, 0.0);
    ptIds[9] = pts->InsertNextPoint(-0.1,  0.5, 0.0);
    ptIds[10] = pts->InsertNextPoint(-0.1, 0.1, 0.0);
    ptIds[11] = pts->InsertNextPoint(-0.5, 0.1, 0.0);
    ptIds[12] = ptIds[0];
    lines->InsertNextCell(13,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateTriangle(vtkPoints *pts, vtkCellArray *lines,
                                             vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[4];

  ptIds[0] = pts->InsertNextPoint(-0.375, -0.25, 0.0);
  ptIds[1] = pts->InsertNextPoint(0.0,  0.5, 0.0);
  ptIds[2] = pts->InsertNextPoint(0.375, -0.25, 0.0);

  if (this->Filled)
    {
    polys->InsertNextCell(3,ptIds);
    }
  else
    {
    ptIds[3] = ptIds[0];
    lines->InsertNextCell(4,ptIds);
    }
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateSquare(vtkPoints *pts, vtkCellArray *lines,
                                           vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[5];

  ptIds[0] = pts->InsertNextPoint(-0.5, -0.5, 0.0);
  ptIds[1] = pts->InsertNextPoint(0.5, -0.5, 0.0);
  ptIds[2] = pts->InsertNextPoint(0.5,  0.5, 0.0);
  ptIds[3] = pts->InsertNextPoint(-0.5,  0.5, 0.0);

  if (this->Filled)
    {
    polys->InsertNextCell(4,ptIds);
    }
  else
    {
    ptIds[4] = ptIds[0];
    lines->InsertNextCell(5,ptIds);
    }
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateCircle(vtkPoints *pts, vtkCellArray *lines,
                                           vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  const unsigned int numberOfPoints = 16;
  vtkIdType ptIds[numberOfPoints+1];
  double x[3];

  // generate points in a circle
  x[2] = 0.0;
  double theta = 2.0 * vtkMath::Pi() / double(numberOfPoints);
  for (int i=0; i<numberOfPoints; i++)
    {
    x[0] = 0.5 * cos((double)i*theta);
    x[1] = 0.5 * sin((double)i*theta);
    ptIds[i] = pts->InsertNextPoint(x);
    }

  if (this->Filled)
    {
    polys->InsertNextCell(numberOfPoints, ptIds);
    }
  else
    {
    ptIds[numberOfPoints] = ptIds[0];
    lines->InsertNextCell(numberOfPoints+1,ptIds);
    }
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateDiamond(vtkPoints *pts, vtkCellArray *lines,
                                            vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[5];

  ptIds[0] = pts->InsertNextPoint(0.0, -0.5, 0.0);
  ptIds[1] = pts->InsertNextPoint(0.5,  0.0, 0.0);
  ptIds[2] = pts->InsertNextPoint(0.0,  0.5, 0.0);
  ptIds[3] = pts->InsertNextPoint(-0.5,  0.0, 0.0);

  if (this->Filled)
    {
    polys->InsertNextCell(4,ptIds);
    }
  else
    {
    ptIds[4] = ptIds[0];
    lines->InsertNextCell(5,ptIds);
    }
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateArrow(vtkPoints *pts, vtkCellArray *lines,
                                          vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
    //stem
    vtkIdType ptIds[3];
    ptIds[0] = pts->InsertNextPoint(0.0, 0.0, 0.0);
    ptIds[1] = pts->InsertNextPoint(0.5, 0.0, 0.0);
    lines->InsertNextCell(2,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);

    //arrow head
    ptIds[0] = pts->InsertNextPoint(0.2, -0.1, 0.0);
    ptIds[1] = pts->InsertNextPoint(0.0,  0.0, 0.0);
    ptIds[2] = pts->InsertNextPoint(0.2,  0.1, 0.0);
    lines->InsertNextCell(3,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateThickArrow(vtkPoints *pts, vtkCellArray *lines,
                                               vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[8];

  ptIds[0] = pts->InsertNextPoint(0.5, -0.1, 0.0);
  ptIds[1] = pts->InsertNextPoint(0.2, -0.1, 0.0);
  ptIds[2] = pts->InsertNextPoint(0.2, -0.2, 0.0);
  ptIds[3] = pts->InsertNextPoint(0.0,  0.0, 0.0);
  ptIds[4] = pts->InsertNextPoint(0.2,  0.2, 0.0);
  ptIds[5] = pts->InsertNextPoint(0.2,  0.1, 0.0);
  ptIds[6] = pts->InsertNextPoint(0.5,  0.1, 0.0);

  if (this->Filled) //create two convex polygons
    {
    polys->InsertNextCell(4);
    polys->InsertCellPoint(ptIds[0]);
    polys->InsertCellPoint(ptIds[1]);
    polys->InsertCellPoint(ptIds[5]);
    polys->InsertCellPoint(ptIds[6]);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);

    polys->InsertNextCell(5,ptIds+1);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
  else
    {
    ptIds[7] = ptIds[0];
    lines->InsertNextCell(8,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateHookedArrow(vtkPoints *pts, vtkCellArray *lines,
                                                vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  if (this->Filled)
    {
    //create two convex polygons
    vtkIdType ptIds[4];
    ptIds[0] = pts->InsertNextPoint(0.5, 0.0, 0.0);
    ptIds[1] = pts->InsertNextPoint(0.2, 0.0, 0.0);
    ptIds[2] = pts->InsertNextPoint(0.2, 0.125, 0.0);
    ptIds[3] = pts->InsertNextPoint(0.5, 0.125, 0.0);
    polys->InsertNextCell(4,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);

    ptIds[0] = pts->InsertNextPoint(0.2, 0.0, 0.0);
    ptIds[1] = pts->InsertNextPoint(0.0, 0.0, 0.0);
    ptIds[2] = pts->InsertNextPoint(0.2, 0.2, 0.0);
    polys->InsertNextCell(3,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
  else
    {
    vtkIdType ptIds[3];
    ptIds[0] = pts->InsertNextPoint(0.5, 0.0, 0.0);
    ptIds[1] = pts->InsertNextPoint(0.0, 0.0, 0.0);
    ptIds[2] = pts->InsertNextPoint(0.2, 0.175, 0.0);
    lines->InsertNextCell(3,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateStarBurst(vtkPoints *pts, vtkCellArray *lines,
                                              vtkCellArray *vtkNotUsed(polys),
                                              vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[2];
  const int numberOfLines = 8;
  const double angleIncrement = 2.0 * vtkMath::Pi() / numberOfLines;
  const double radius = 0.5;
  const double gapRadius = 0.2;
  double angle = 0;
  for (int lineIndex = 0; lineIndex < numberOfLines; lineIndex++)
    {
    ptIds[0] = pts->InsertNextPoint(gapRadius * cos(angle), gapRadius * sin(angle), 0.0);
    ptIds[1] = pts->InsertNextPoint(radius * cos(angle), radius * sin(angle), 0.0);
    lines->InsertNextCell(2, ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    angle += angleIncrement;
    }
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::CreateDash(vtkPoints *pts, vtkCellArray *lines,
                                         vtkCellArray *polys, vtkUnsignedCharArray *colors,
                                         double scale)
{
  vtkIdType ptIds[5];
  ptIds[0] = pts->InsertNextPoint(-0.5 * scale, -0.1 * scale, 0.0);
  ptIds[1] = pts->InsertNextPoint(0.5 * scale, -0.1 * scale, 0.0);
  ptIds[2] = pts->InsertNextPoint(0.5 * scale,  0.1 * scale, 0.0);
  ptIds[3] = pts->InsertNextPoint(-0.5 * scale,  0.1 * scale, 0.0);

  if (this->Filled)
    {
    polys->InsertNextCell(4,ptIds);
    }
  else
    {
    vtkIdType ptIds2D[2];
    ptIds2D[0] = pts->InsertNextPoint(-0.5*scale, 0.0, 0.0);
    ptIds2D[1] = pts->InsertNextPoint(0.5*scale, 0.0, 0.0);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    lines->InsertNextCell(2,ptIds2D);
    }
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Center: (" << this->Center[0] << ", "
     << this->Center[1] << ", " << this->Center[2] << ")\n";

  os << indent << "Scale: " << this->Scale << "\n";
  os << indent << "Scale2: " << this->Scale2 << "\n";
  os << indent << "Rotation Angle: " << this->RotationAngle << "\n";

  os << indent << "Color: (" << this->Color[0] << ", "
     << this->Color[1] << ", " << this->Color[2] << ")\n";

  os << indent << "Filled: " << (this->Filled ? "On\n" : "Off\n");
  os << indent << "Dash: " << (this->Dash ? "On\n" : "Off\n");
  os << indent << "Cross: " << (this->Cross ? "On\n" : "Off\n");

  os << indent << "Glyph Type: ";
  switch (this->GlyphType)
    {
    case GlyphNone: os << "None\n"; break;
    case GlyphStarBurst: os << "StarBurst\n"; break;
    case GlyphCross: os << "Cross\n"; break;
    case GlyphCrossDot: os << "CrossDot\n"; break;
    case GlyphThickCross: os << "ThickCross\n"; break;
    case GlyphVertex: os << "StarBurst\n"; break;
    case GlyphDash: os << "Dash\n"; break;
    case GlyphTriangle: os << "Triangle\n"; break;
    case GlyphSquare: os << "Square\n"; break;
    case GlyphCircle: os << "Circle\n"; break;
    case GlyphDiamond: os << "Diamond\n"; break;
    case GlyphArrow: os << "Arrow\n"; break;
    case GlyphThickArrow: os << "ThickArrow\n"; break;
    case GlyphHookedArrow: os << "HookedArrow\n"; break;
    default:
      os << "unknown\n";
    }
}

//----------------------------------------------------------------------------
void vtkMarkupsGlyphSource2D::SetNextGlyphType()
{
  this->GlyphType++;
  if (this->GlyphType == GlyphType_Last)
    {
    this->SetGlyphType(GlyphNone+1);
    }
}
