/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSurfacePolyDataActor.cxx,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.7 $

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

vtkCxxRevisionMacro(vtkMimxSurfacePolyDataActor, "$Revision: 1.7 $");

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
  this->DataType = ACTOR_POLYDATA_SURFACE;
}

vtkMimxSurfacePolyDataActor::~vtkMimxSurfacePolyDataActor()
{
  if(this->PolyData)  
    this->PolyData->Delete();
  this->PolyDataMapper->Delete();
  this->Actor->Delete();
 }

vtkPolyData* vtkMimxSurfacePolyDataActor::GetDataSet()
{
  return this->PolyData;
}

//void vtkSurfacePolyDataActor::SetDataType(int){}
void vtkMimxSurfacePolyDataActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

