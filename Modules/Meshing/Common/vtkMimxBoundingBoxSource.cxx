/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundingBoxSource.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.5 $

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
#include "vtkMimxBoundingBoxSource.h"

#include "vtkActor.h"
#include "vtkDataSetMapper.h"
#include "vtkHexahedron.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxBoundingBoxSource, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxBoundingBoxSource);

//----------------------------------------------------------------------------

vtkMimxBoundingBoxSource::vtkMimxBoundingBoxSource()
{
  // default bounds
  this->Bounds[0] = 0; this->Bounds[1] = 1; 
  this->Bounds[2] = 0; this->Bounds[3] = 1; 
  this->Bounds[4] = 0; this->Bounds[5] = 1; 
  this->SetNumberOfInputPorts(0);
  this->Source = NULL;
}

vtkMimxBoundingBoxSource::~vtkMimxBoundingBoxSource()
{
}

//----------------------------------------------------------------------------
int vtkMimxBoundingBoxSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if (this->Source)
  {
    double bounds[6];
    this->Source->GetBounds(bounds);
    this->SetBounds(bounds);
  }
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints* points = vtkPoints::New();
  points->SetNumberOfPoints(8);
  // set all the 8 vertices of the hexahedron
  points->SetPoint(0, this->Bounds[0], this->Bounds[2], this->Bounds[4]);
  points->SetPoint(1, this->Bounds[0], this->Bounds[2], this->Bounds[5]);
  points->SetPoint(2, this->Bounds[1], this->Bounds[2], this->Bounds[5]);
  points->SetPoint(3, this->Bounds[1], this->Bounds[2], this->Bounds[4]);
  points->SetPoint(4, this->Bounds[0], this->Bounds[3], this->Bounds[4]);
  points->SetPoint(5, this->Bounds[0], this->Bounds[3], this->Bounds[5]);
  points->SetPoint(6, this->Bounds[1], this->Bounds[3], this->Bounds[5]);
  points->SetPoint(7, this->Bounds[1], this->Bounds[3], this->Bounds[4]);

  vtkHexahedron* aHexahedron = vtkHexahedron::New();
  for(int i=0; i < 8; i++)  aHexahedron->GetPointIds()->SetId(i, i);

  output->Allocate(1, 1);
  output->InsertNextCell(aHexahedron->GetCellType(),
    aHexahedron->GetPointIds());
  output->SetPoints(points);

  points->Delete();
  aHexahedron->Delete();

  return 1;
}

//----------------------------------------------------------------------------
void vtkMimxBoundingBoxSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMimxBoundingBoxSource::SetBounds(double bounds[6])
{
  for(int i=0; i<6; i++)  { this->Bounds[i] = bounds[i];
  }
}
