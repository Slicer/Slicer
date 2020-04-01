/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGlyphSource2D.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSlicerGlyphSource2D.h"

// VTK includes
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkMRMLFiducialListNode.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVersion.h>

vtkStandardNewMacro(vtkSlicerGlyphSource2D);

//----------------------------------------------------------------------------
vtkSlicerGlyphSource2D::vtkSlicerGlyphSource2D()
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
int vtkSlicerGlyphSource2D::RequestData(
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
  if ( this->Dash )
    {
    int filled = this->Filled;
    this->Filled = 0;
    this->CreateDash(pts,lines,polys,colors,this->Scale2);
    this->Filled = filled;
    }
  if ( this->Cross )
    {
    int filled = this->Filled;
    this->Filled = 0;
    this->CreateCross(pts,lines,polys,colors,this->Scale2);
    this->Filled = filled;
    }

  //Call the right function
  switch (this->GlyphType)
    {
    case VTK_NO_GLYPH:
      break;
    case VTK_VERTEX_GLYPH:
      this->CreateVertex(pts,verts,colors);
      break;
    case VTK_DASH_GLYPH:
      this->CreateDash(pts,lines,polys,colors,this->Scale);
      break;
    case VTK_CROSS_GLYPH:
      this->CreateCross(pts,lines,polys,colors,this->Scale);
      break;
    case VTK_THICKCROSS_GLYPH:
      this->CreateThickCross(pts,lines,polys,colors);
      break;
    case VTK_TRIANGLE_GLYPH:
      this->CreateTriangle(pts,lines,polys,colors);
      break;
    case VTK_SQUARE_GLYPH:
      this->CreateSquare(pts,lines,polys,colors);
      break;
    case VTK_CIRCLE_GLYPH:
      this->CreateCircle(pts,lines,polys,colors);
      break;
    case VTK_DIAMOND_GLYPH:
      this->CreateDiamond(pts,lines,polys,colors);
      break;
    case VTK_ARROW_GLYPH:
      this->CreateArrow(pts,lines,polys,colors);
      break;
    case VTK_THICKARROW_GLYPH:
      this->CreateThickArrow(pts,lines,polys,colors);
      break;
    case VTK_HOOKEDARROW_GLYPH:
      this->CreateHookedArrow(pts,lines,polys,colors);
      break;
    case VTK_STARBURST_GLYPH:
      this->CreateStarBurst(pts,lines,polys,colors,this->Scale);
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

void vtkSlicerGlyphSource2D::ConvertColor()
{
  this->RGB[0] = (unsigned char) (255.0 * this->Color[0]);
  this->RGB[1] = (unsigned char) (255.0 * this->Color[1]);
  this->RGB[2] = (unsigned char) (255.0 * this->Color[2]);
}

void vtkSlicerGlyphSource2D::TransformGlyph(vtkPoints *pts)
{
  double x[3];
  int i;
  int numPts=pts->GetNumberOfPoints();

  if ( this->RotationAngle == 0.0 )
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

void vtkSlicerGlyphSource2D::CreateVertex(vtkPoints *pts, vtkCellArray *verts,
                                    vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[1];
  ptIds[0] = pts->InsertNextPoint(0.0,0.0,0.0);
  verts->InsertNextCell(1,ptIds);
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);
}

void vtkSlicerGlyphSource2D::CreateCross(vtkPoints *pts, vtkCellArray *lines,
                                   vtkCellArray *polys, vtkUnsignedCharArray *colors,
                                   double scale)
{
  vtkIdType ptIds[4];

  if ( this->Filled )
    {
    this->CreateThickCross(pts,lines,polys,colors);
    }
  else
    {
    ptIds[0] = pts->InsertNextPoint(-0.5*scale, 0.0, 0.0);
    ptIds[1] = pts->InsertNextPoint( 0.5*scale, 0.0, 0.0);
    lines->InsertNextCell(2,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    ptIds[0] = pts->InsertNextPoint(0.0, -0.5*scale, 0.0);
    ptIds[1] = pts->InsertNextPoint(0.0,  0.5*scale, 0.0);
    lines->InsertNextCell(2,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
}

void vtkSlicerGlyphSource2D::CreateThickCross(vtkPoints *pts, vtkCellArray *lines,
                                        vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  if ( this->Filled )
    {
    vtkIdType ptIds[4];
    ptIds[0] = pts->InsertNextPoint(-0.5, -0.1, 0.0);
    ptIds[1] = pts->InsertNextPoint( 0.5, -0.1, 0.0);
    ptIds[2] = pts->InsertNextPoint( 0.5,  0.1, 0.0);
    ptIds[3] = pts->InsertNextPoint(-0.5,  0.1, 0.0);
    polys->InsertNextCell(4,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    ptIds[0] = pts->InsertNextPoint(-0.1, -0.5, 0.0);
    ptIds[1] = pts->InsertNextPoint( 0.1, -0.5, 0.0);
    ptIds[2] = pts->InsertNextPoint( 0.1,  0.5, 0.0);
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
    ptIds[3] = pts->InsertNextPoint( 0.1, -0.5, 0.0);
    ptIds[4] = pts->InsertNextPoint( 0.1, -0.1, 0.0);
    ptIds[5] = pts->InsertNextPoint( 0.5, -0.1, 0.0);
    ptIds[6] = pts->InsertNextPoint( 0.5,  0.1, 0.0);
    ptIds[7] = pts->InsertNextPoint( 0.1,  0.1, 0.0);
    ptIds[8] = pts->InsertNextPoint( 0.1,  0.5, 0.0);
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

void vtkSlicerGlyphSource2D::CreateTriangle(vtkPoints *pts, vtkCellArray *lines,
                                      vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[4];

  ptIds[0] = pts->InsertNextPoint(-0.375, -0.25, 0.0);
  ptIds[1] = pts->InsertNextPoint( 0.0,  0.5, 0.0);
  ptIds[2] = pts->InsertNextPoint( 0.375, -0.25, 0.0);

  if ( this->Filled )
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

void vtkSlicerGlyphSource2D::CreateSquare(vtkPoints *pts, vtkCellArray *lines,
                                    vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[5];

  ptIds[0] = pts->InsertNextPoint(-0.5, -0.5, 0.0);
  ptIds[1] = pts->InsertNextPoint( 0.5, -0.5, 0.0);
  ptIds[2] = pts->InsertNextPoint( 0.5,  0.5, 0.0);
  ptIds[3] = pts->InsertNextPoint(-0.5,  0.5, 0.0);

  if ( this->Filled )
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

void vtkSlicerGlyphSource2D::CreateCircle(vtkPoints *pts, vtkCellArray *lines,
                                    vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[9];
  double x[3], theta;

  // generate eight points in a circle
  x[2] = 0.0;
  theta = 2.0 * vtkMath::Pi() / 8.0;
  for (int i=0; i<8; i++)
    {
    x[0] = 0.5 * cos((double)i*theta);
    x[1] = 0.5 * sin((double)i*theta);
    ptIds[i] = pts->InsertNextPoint(x);
    }

  if ( this->Filled )
    {
    polys->InsertNextCell(8,ptIds);
    }
  else
    {
    ptIds[8] = ptIds[0];
    lines->InsertNextCell(9,ptIds);
    }
  colors->InsertNextValue(this->RGB[0]);
  colors->InsertNextValue(this->RGB[1]);
  colors->InsertNextValue(this->RGB[2]);
}

void vtkSlicerGlyphSource2D::CreateDiamond(vtkPoints *pts, vtkCellArray *lines,
                                     vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[5];

  ptIds[0] = pts->InsertNextPoint( 0.0, -0.5, 0.0);
  ptIds[1] = pts->InsertNextPoint( 0.5,  0.0, 0.0);
  ptIds[2] = pts->InsertNextPoint( 0.0,  0.5, 0.0);
  ptIds[3] = pts->InsertNextPoint(-0.5,  0.0, 0.0);

  if ( this->Filled )
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

void vtkSlicerGlyphSource2D::CreateArrow(vtkPoints *pts, vtkCellArray *lines,
                                   vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  if ( this->Filled ) //create two convex polygons
    {
    this->CreateThickArrow(pts,lines,polys,colors);
    }
  else
    {
    //stem
    vtkIdType ptIds[3];
    ptIds[0] = pts->InsertNextPoint( -0.5, 0.0, 0.0);
    ptIds[1] = pts->InsertNextPoint(  0.5, 0.0, 0.0);
    lines->InsertNextCell(2,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);

    //arrow head
    ptIds[0] = pts->InsertNextPoint( 0.2, -0.1, 0.0);
    ptIds[1] = pts->InsertNextPoint( 0.5,  0.0, 0.0);
    ptIds[2] = pts->InsertNextPoint( 0.2,  0.1, 0.0);
    lines->InsertNextCell(3,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
}

void vtkSlicerGlyphSource2D::CreateThickArrow(vtkPoints *pts, vtkCellArray *lines,
                                         vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  vtkIdType ptIds[8];

  ptIds[0] = pts->InsertNextPoint( -0.5, -0.1, 0.0);
  ptIds[1] = pts->InsertNextPoint(  0.1, -0.1, 0.0);
  ptIds[2] = pts->InsertNextPoint(  0.1, -0.2, 0.0);
  ptIds[3] = pts->InsertNextPoint(  0.5,  0.0, 0.0);
  ptIds[4] = pts->InsertNextPoint(  0.1,  0.2, 0.0);
  ptIds[5] = pts->InsertNextPoint(  0.1,  0.1, 0.0);
  ptIds[6] = pts->InsertNextPoint( -0.5,  0.1, 0.0);

  if ( this->Filled ) //create two convex polygons
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

void vtkSlicerGlyphSource2D::CreateHookedArrow(vtkPoints *pts, vtkCellArray *lines,
                                         vtkCellArray *polys, vtkUnsignedCharArray *colors)
{
  if ( this->Filled )
    {
    //create two convex polygons
    vtkIdType ptIds[4];
    ptIds[0] = pts->InsertNextPoint( -0.5,   -0.1, 0.0);
    ptIds[1] = pts->InsertNextPoint(  0.1,   -0.1, 0.0);
    ptIds[2] = pts->InsertNextPoint(  0.1,  0.075, 0.0);
    ptIds[3] = pts->InsertNextPoint( -0.5,  0.075, 0.0);
    polys->InsertNextCell(4,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);

    ptIds[0] = pts->InsertNextPoint( 0.1, -0.1, 0.0);
    ptIds[1] = pts->InsertNextPoint( 0.5, -0.1, 0.0);
    ptIds[2] = pts->InsertNextPoint( 0.1,  0.2, 0.0);
    polys->InsertNextCell(3,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
  else
    {
    vtkIdType ptIds[3];
    ptIds[0] = pts->InsertNextPoint( -0.5, 0.0, 0.0);
    ptIds[1] = pts->InsertNextPoint(  0.5, 0.0, 0.0);
    ptIds[2] = pts->InsertNextPoint(  0.2, 0.1, 0.0);
    lines->InsertNextCell(3,ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);
    }
}

void vtkSlicerGlyphSource2D::CreateStarBurst(vtkPoints *pts, vtkCellArray *lines,
                                             vtkCellArray *vtkNotUsed(polys),
                                             vtkUnsignedCharArray *colors,
                                             double vtkNotUsed(scale))
{
  // make a star shaped array of lines around the center, from SeedSWidget.tcl
  float PI =  3.1415926;
  float TWOPI = PI * 2.0;
  float PIoverFOUR = PI / 4.0;
  float x, y;
  float _glyphScale = 1.0; // scale;
  float angle = 0.0;
  vtkIdType ptIds[2];

  while (angle <= TWOPI)
    {
    x = _glyphScale * 0.3 * cos(angle);
    y = _glyphScale * 0.3 * sin(angle);
    ptIds[0] = pts->InsertNextPoint(x, y, 0.0);
    x = _glyphScale * cos(angle);
    y = _glyphScale * sin(angle);
    ptIds[1] = pts->InsertNextPoint(x, y, 0.0);
    lines->InsertNextCell(2, ptIds);
    colors->InsertNextValue(this->RGB[0]);
    colors->InsertNextValue(this->RGB[1]);
    colors->InsertNextValue(this->RGB[2]);

    angle += PIoverFOUR;
    }
}

void vtkSlicerGlyphSource2D::CreateDash(vtkPoints *pts, vtkCellArray *lines,
                                  vtkCellArray *polys, vtkUnsignedCharArray *colors,
                                                                  double scale)
{
  vtkIdType ptIds[5];
  ptIds[0] = pts->InsertNextPoint(-0.5, -0.1, 0.0);
  ptIds[1] = pts->InsertNextPoint( 0.5, -0.1, 0.0);
  ptIds[2] = pts->InsertNextPoint( 0.5,  0.1, 0.0);
  ptIds[3] = pts->InsertNextPoint(-0.5,  0.1, 0.0);

  if ( this->Filled )
    {
    polys->InsertNextCell(4,ptIds);
    }
  else
    {
    vtkIdType ptIds2D[2];
    ptIds2D[0] = pts->InsertNextPoint(-0.5*scale, 0.0, 0.0);
    ptIds2D[1] = pts->InsertNextPoint( 0.5*scale, 0.0, 0.0);
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
void vtkSlicerGlyphSource2D::PrintSelf(ostream& os, vtkIndent indent)
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

  os << indent << "Glyph Type";
  switch (this->GlyphType)
    {
    case VTK_NO_GLYPH:
      os << "No Glyph\n";
      break;
    case VTK_VERTEX_GLYPH:
      os << "Vertex\n";
      break;
    case VTK_DASH_GLYPH:
      os << "Dash\n";
      break;
    case VTK_CROSS_GLYPH:
      os << "Cross\n";
      break;
    case VTK_THICKCROSS_GLYPH:
      os << "Cross\n";
      break;
    case VTK_TRIANGLE_GLYPH:
      os << "Triangle\n";
      break;
    case VTK_SQUARE_GLYPH:
      os << "Square\n";
      break;
    case VTK_CIRCLE_GLYPH:
      os << "Circle\n";
      break;
    case VTK_DIAMOND_GLYPH:
      os << "Diamond\n";
      break;
    case VTK_ARROW_GLYPH:
      os << "Arrow\n";
      break;
    case VTK_THICKARROW_GLYPH:
      os << "Arrow\n";
      break;
    case VTK_HOOKEDARROW_GLYPH:
      os << "Hooked Arrow\n";
      break;
    case VTK_STARBURST_GLYPH:
      os << "Star Burst\n";
      break;
    }
}

void vtkSlicerGlyphSource2D::SetGlyphTypeAsString(const char *type)
{
  if (type == nullptr)
    {
    vtkErrorMacro("Cannot set glyph type from a null string.");
    return;
    }

  vtkNew<vtkMRMLFiducialListNode> listNode;

  if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::StarBurst2D) ) )
    {
    this->SetGlyphTypeToStarBurst();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::Vertex2D) ) )
    {
    this->SetGlyphTypeToVertex();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::Dash2D) ) )
    {
    this->SetGlyphTypeToDash();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::Cross2D) ) )
    {
    this->SetGlyphTypeToCross();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::ThickCross2D) ) )
    {
    this->SetGlyphTypeToThickCross();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::Triangle2D) ) )
    {
    this->SetGlyphTypeToTriangle();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::Square2D) ) )
    {
    this->SetGlyphTypeToSquare();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::Circle2D) ) )
    {
    this->SetGlyphTypeToCircle();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::Diamond2D) ) )
    {
    this->SetGlyphTypeToDiamond();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::Arrow2D) ) )
    {
    this->SetGlyphTypeToArrow();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::ThickArrow2D) ) )
    {
    this->SetGlyphTypeToThickArrow();
    }
  else if ( !strcmp( type, listNode->GetGlyphTypeAsString(vtkMRMLFiducialListNode::HookedArrow2D) ) )
    {
    this->SetGlyphTypeToHookedArrow();
    }
}
