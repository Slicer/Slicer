/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridFromFourPoints.cxx,v $
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

#include "vtkMimxUnstructuredGridFromFourPoints.h"

#include "vtkCell.h"
#include "vtkHexahedron.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"


vtkCxxRevisionMacro(vtkMimxUnstructuredGridFromFourPoints, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkMimxUnstructuredGridFromFourPoints);

// Description:

vtkMimxUnstructuredGridFromFourPoints::vtkMimxUnstructuredGridFromFourPoints()
{
        this->ExtrusionLength = 1.0;
        this->SetNumberOfInputPorts(0);
}

vtkMimxUnstructuredGridFromFourPoints::~vtkMimxUnstructuredGridFromFourPoints()
{
}

int vtkMimxUnstructuredGridFromFourPoints::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        
  // get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the output

  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
 
  
  if(this->Points->GetNumberOfPoints() !=4)
  {
          vtkErrorMacro("Input should contain 4 points");
          return 0;
  }


// check extrusion length
  if(this->ExtrusionLength <= 0.0)
  {
          vtkErrorMacro("Extrusion Length should always be > 0");
          return 0;
  }
  //        calculate the direction of extrusion
  //    calculate the normals at all the 4 points
  // and average them
  double pt1[3], pt2[3], pt3[3], pt4[3];
  double normal[3], normal1[3], normal2[3], normal3[3], normal4[3];
  this->Points->GetPoint(0,pt1);
  this->Points->GetPoint(1,pt2);
  this->Points->GetPoint(2,pt3);
  this->Points->GetPoint(3,pt4);
 
  vtkTriangle::ComputeNormal(pt1, pt2, pt3, normal1);
  vtkTriangle::ComputeNormal(pt2, pt3, pt4, normal2);
  vtkTriangle::ComputeNormal(pt3, pt4, pt1, normal3);
  vtkTriangle::ComputeNormal(pt4, pt1, pt2, normal4);
  normal[0] = 0.0; normal[1] = 0.0; normal[2] = 0.0;
  normal[0] = normal1[0] + normal2[0] + normal3[0] + normal4[0];
  normal[1] = normal1[1] + normal2[1] + normal3[1] + normal4[1];
  normal[2] = normal1[2] + normal2[2] + normal3[2] + normal4[2];

  for(int i=0; i<3; i++)        normal[i] = normal[i]/4.0;

  vtkPoints* outpoints = vtkPoints::New();
  outpoints->SetNumberOfPoints(8);
  // set all the 8 vertices of the hexahedron
  outpoints->SetPoint(0, pt1);
  outpoints->SetPoint(1, pt1[0] + this->ExtrusionLength*normal[0],
          pt1[1] + this->ExtrusionLength*normal[1], pt1[2] + this->ExtrusionLength*normal[2]);
  outpoints->SetPoint(2, pt2[0] + this->ExtrusionLength*normal[0],
          pt2[1] + this->ExtrusionLength*normal[1], pt2[2] + this->ExtrusionLength*normal[2]);
  outpoints->SetPoint(3, pt2);
  outpoints->SetPoint(4, pt4);
  outpoints->SetPoint(5, pt4[0] + this->ExtrusionLength*normal[0],
          pt4[1] + this->ExtrusionLength*normal[1], pt4[2] + this->ExtrusionLength*normal[2]);
  outpoints->SetPoint(6, pt3[0] + this->ExtrusionLength*normal[0],
          pt3[1] + this->ExtrusionLength*normal[1], pt3[2] + this->ExtrusionLength*normal[2]);
  outpoints->SetPoint(7, pt3);

  vtkHexahedron* aHexahedron = vtkHexahedron::New();
  for(int i=0; i < 8; i++)        aHexahedron->GetPointIds()->SetId(i, i);


  output->Allocate(1, 1);
  output->InsertNextCell(aHexahedron->GetCellType(),
          aHexahedron->GetPointIds());
  output->SetPoints(outpoints);

  outpoints->Delete();
  aHexahedron->Delete();

  return 1;
}

void vtkMimxUnstructuredGridFromFourPoints::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
