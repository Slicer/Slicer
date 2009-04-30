/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSurfacePolyDataActor.cxx,v $
Language:  C++
Date:      $Date: 2008/08/05 02:32:48 $
Version:   $Revision: 1.10 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkMimxSurfacePolyDataActor.h"

#include "vtkActor.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"

vtkCxxRevisionMacro(vtkMimxSurfacePolyDataActor, "$Revision: 1.10 $");

vtkStandardNewMacro(vtkMimxSurfacePolyDataActor);

vtkMimxSurfacePolyDataActor::vtkMimxSurfacePolyDataActor()
{
  vtkPoints *points = vtkPoints::New();
  this->PolyData = vtkPolyData::New();
  this->PolyData->SetPoints(points);
  points->Delete();
        this->PolyDataMapper = vtkPolyDataMapper::New();
        this->Actor = vtkActor::New();
        // set up the pipe line
        this->PolyDataMapper->SetInput(this->PolyData);
        this->Actor->SetMapper(this->PolyDataMapper);
        this->FillColor[0] = this->FillColor[1] = this->FillColor[2] = 1.0;
        this->OutlineColor[0] = this->OutlineColor[1] = this->OutlineColor[2] = 1.0;
        this->SetDisplayType( DisplaySurface );
}

//----------------------------------------------------------------------------------
vtkMimxSurfacePolyDataActor::~vtkMimxSurfacePolyDataActor()
{
  if(this->PolyData)    
    this->PolyData->Delete();
        this->PolyDataMapper->Delete();
        this->Actor->Delete();
 }

//----------------------------------------------------------------------------------
vtkPolyData* vtkMimxSurfacePolyDataActor::GetDataSet()
{
        return this->PolyData;
}

//----------------------------------------------------------------------------------
void vtkMimxSurfacePolyDataActor::GetOutlineColor(double &red, double &green, double &blue)
{
  red = this->OutlineColor[0];
  green = this->OutlineColor[1];
  blue = this->OutlineColor[2];
}

//----------------------------------------------------------------------------------
void vtkMimxSurfacePolyDataActor::GetOutlineColor(double rgb[3])
{
  rgb[0] = this->OutlineColor[0];
  rgb[1] = this->OutlineColor[1];
  rgb[2] = this->OutlineColor[2];
}

//----------------------------------------------------------------------------------
void vtkMimxSurfacePolyDataActor::SetOutlineColor(double red, double green, double blue)
{
  this->OutlineColor[0] = red;
  this->OutlineColor[1] = green;
  this->OutlineColor[2] = blue;
  if (this->DisplayType == DisplayOutline)
  {
    this->Actor->GetProperty()->SetColor(red, green, blue);
    this->Actor->Modified();
  }
}

//----------------------------------------------------------------------------------
void vtkMimxSurfacePolyDataActor::SetOutlineColor(double rgb[3])
{
  this->SetOutlineColor(rgb[0], rgb[1], rgb[2]);
}
//----------------------------------------------------------------------------------
void vtkMimxSurfacePolyDataActor::SetFillColor(double red, double green, double blue)
{
  this->FillColor[0] = red;
  this->FillColor[1] = green;
  this->FillColor[2] = blue;
  if (this->DisplayType == DisplaySurface)
  {
    this->Actor->GetProperty()->SetColor(red, green, blue);
    this->Actor->Modified();
  }
}

//----------------------------------------------------------------------------------
void vtkMimxSurfacePolyDataActor::SetFillColor(double rgb[3])
{
  this->SetFillColor(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------------
void vtkMimxSurfacePolyDataActor::GetFillColor(double &red, double &green, double &blue)
{
  red = this->FillColor[0];
  green = this->FillColor[1];
  blue = this->FillColor[2];
}

//----------------------------------------------------------------------------------
void vtkMimxSurfacePolyDataActor::GetFillColor(double rgb[3])
{
  rgb[0] = this->FillColor[0];
  rgb[1] = this->FillColor[1];
  rgb[2] = this->FillColor[2];
}

//----------------------------------------------------------------------------------
void vtkMimxSurfacePolyDataActor::SetDisplayType(int mode)
{
  switch ( mode )
  {
    case DisplaySurface:
      this->Actor->GetProperty()->SetRepresentationToSurface();
      this->Actor->GetProperty()->SetColor( this->FillColor );
      break;
    case DisplayOutline:
      this->Actor->GetProperty()->SetRepresentationToWireframe();
      this->Actor->GetProperty()->SetColor( this->OutlineColor );
      break;
  }
  this->DisplayType = mode;
}

//----------------------------------------------------------------------------------
int vtkMimxSurfacePolyDataActor::GetDisplayType( )
{
  return this->DisplayType;
}

//----------------------------------------------------------------------------------
//void vtkSurfacePolyDataActor::SetDataType(int){}
void vtkMimxSurfacePolyDataActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------------

// added to support slicer integration
void vtkMimxSurfacePolyDataActor::SaveVisibility(void) {this->SavedVisibility = (this->Actor->GetVisibility())?true:false;}
void vtkMimxSurfacePolyDataActor::RestoreVisibility(void) {this->Actor->SetVisibility(this->SavedVisibility);}
void vtkMimxSurfacePolyDataActor::Hide() {this->Actor->SetVisibility(0);}
void vtkMimxSurfacePolyDataActor::Show() {this->Actor->SetVisibility(1);}


