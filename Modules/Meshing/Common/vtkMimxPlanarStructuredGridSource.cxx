/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPlanarStructuredGridSource.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.3 $

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

#include "vtkMimxPlanarStructuredGridSource.h"
#include "vtkObjectFactory.h"
#include "vtkSphereSource.h"
#include "vtkCellArray.h"
#include "vtkHexahedron.h"
#include "vtkDataSetMapper.h"
#include "vtkProperty.h"
///////////////////////////////////////////////////////////////
vtkCxxRevisionMacro(vtkMimxPlanarStructuredGridSource, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkMimxPlanarStructuredGridSource);

vtkMimxPlanarStructuredGridSource::vtkMimxPlanarStructuredGridSource()
{
  SGrid = vtkStructuredGrid::New();
  Mapper = vtkDataSetMapper::New();
  Actor = vtkActor::New();
  this->Mapper->SetInput(this->SGrid);
  this->Actor->SetMapper(this->Mapper);
}

vtkMimxPlanarStructuredGridSource::~vtkMimxPlanarStructuredGridSource()
{
  SGrid->Delete();
  Mapper->Delete();
  Actor->Delete();
}

