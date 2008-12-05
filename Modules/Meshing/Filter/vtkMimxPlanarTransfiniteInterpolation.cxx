/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPlanarTransfiniteInterpolation.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.9 $

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

#include "vtkMimxPlanarTransfiniteInterpolation.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"

vtkCxxRevisionMacro(vtkMimxPlanarTransfiniteInterpolation, "$Revision: 1.9 $");
vtkStandardNewMacro(vtkMimxPlanarTransfiniteInterpolation);

vtkMimxPlanarTransfiniteInterpolation::vtkMimxPlanarTransfiniteInterpolation()
{
  // minimum of 2 points required for a curve
  this->IDiv = 2;
  this->JDiv = 2;
}

vtkMimxPlanarTransfiniteInterpolation::~vtkMimxPlanarTransfiniteInterpolation()
{
}


//----------------------------------------------------------------------------
int vtkMimxPlanarTransfiniteInterpolation::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPointSet *input = vtkPointSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPointSet *output = vtkPointSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints *points;
  vtkIdType numPts;

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  if (input == NULL || input->GetPoints() == NULL)
        {
        return 1;
    }
  numPts = input->GetPoints()->GetNumberOfPoints();


  if ( !numPts)
    {
    vtkDebugMacro(<<"No input data");
    return 1;
    }

  // SETUP AND ALLOCATE THE OUTPUT
  numPts = input->GetNumberOfPoints();
  points = input->GetPoints()->NewInstance();
  points->SetDataType(input->GetPoints()->GetDataType());
  points->Allocate(numPts);
  points->SetNumberOfPoints(numPts);
  points->DeepCopy(input->GetPoints());
  // apply interpolation to calculate the interior nodes
  double interp[3];
  double x[3];
  double shapeI = 1.0/(this->IDiv - 1.0);
  double shapeJ = 1.0/(this->JDiv - 1.0);

  for(int j = 1; j < this->JDiv - 1; j++)
  {
          for(int i = 1; i < this->IDiv -1; i++)
          {             
                  interp[0] = 0.0;      interp[1] = 0.0; interp[2] = 0.0;
                  ////cout <<i<<"  ";
                  points->GetPoint(i,x);
                  interp[0] = interp[0] + (1.0 - shapeJ*j)*x[0];
                  interp[1] = interp[1] + (1.0 - shapeJ*j)*x[1];
                  interp[2] = interp[2] + (1.0 - shapeJ*j)*x[2];
                  points->GetPoint(this->IDiv*(this->JDiv-1)+i,x);
                  ////cout <<this->IDiv*(this->JDiv-1)+i<<"  ";
                  interp[0] = interp[0] + (shapeJ*j)*x[0];
                  interp[1] = interp[1] + (shapeJ*j)*x[1];
                  interp[2] = interp[2] + (shapeJ*j)*x[2];
                  points->GetPoint(this->IDiv*j,x);
                  ////cout <<this->IDiv*j<<"  ";
                  interp[0] = interp[0] + (1.0 - shapeI*i)*x[0];
                  interp[1] = interp[1] + (1.0 - shapeI*i)*x[1];
                  interp[2] = interp[2] + (1.0 - shapeI*i)*x[2];
                  points->GetPoint(this->IDiv*(j+1)-1,x);
                  ////cout <<this->IDiv*(j+1)-1<<"  ";
                  interp[0] = interp[0] + (shapeI*i)*x[0];
                  interp[1] = interp[1] + (shapeI*i)*x[1];
                  interp[2] = interp[2] + (shapeI*i)*x[2];
                        ////cout <<"0"<<"  ";
                  points->GetPoint(0,x);
                  interp[0] = interp[0] - (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[0];
                  interp[1] = interp[1] - (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[1];
                  interp[2] = interp[2] - (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[2];
                  ////cout <<this->IDiv-1<<"  ";
                  points->GetPoint(this->IDiv-1,x);
                  interp[0] = interp[0] - (shapeI*i)*(1.0 - shapeJ*j)*x[0];
                  interp[1] = interp[1] - (shapeI*i)*(1.0 - shapeJ*j)*x[1];
                  interp[2] = interp[2] - (shapeI*i)*(1.0 - shapeJ*j)*x[2];
                  points->GetPoint(this->IDiv*this->JDiv-1,x);
                  ////cout <<this->IDiv*this->JDiv-1<<"  ";
                  interp[0] = interp[0] - (shapeI*i)*(shapeJ*j)*x[0];
                  interp[1] = interp[1] - (shapeI*i)*(shapeJ*j)*x[1];
                  interp[2] = interp[2] - (shapeI*i)*(shapeJ*j)*x[2];
                  points->GetPoint(this->IDiv*(this->JDiv-1),x);
                  ////cout <<this->IDiv*(this->JDiv-1)<<"  ";
                  interp[0] = interp[0] - (1.0 - shapeI*i)*(shapeJ*j)*x[0];
                  interp[1] = interp[1] - (1.0 - shapeI*i)*(shapeJ*j)*x[1];
                  interp[2] = interp[2] - (1.0 - shapeI*i)*(shapeJ*j)*x[2];
                  points->SetPoint(j*this->IDiv + i,interp);
                  ////cout <<j*this->IDiv + i<<"  "<< interp[0]<<"  "<<interp[1]<<"  "<<interp[2]<<std::endl;
          }
  }

  output->SetPoints(points);
  points->Delete();
 
  return 1;
}


void vtkMimxPlanarTransfiniteInterpolation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << "I Divisions: " << this->IDiv << std::endl;
  os << "J Divisions: " << this->JDiv << std::endl;

}
