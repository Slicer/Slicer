/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSolidTransfiniteInterpolation.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.11 $

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

#include "vtkMimxSolidTransfiniteInterpolation.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"
#include "vtkImagePlaneWidget.h"

vtkCxxRevisionMacro(vtkMimxSolidTransfiniteInterpolation, "$Revision: 1.11 $");
vtkStandardNewMacro(vtkMimxSolidTransfiniteInterpolation);

vtkMimxSolidTransfiniteInterpolation::vtkMimxSolidTransfiniteInterpolation()
{
  // minimum of 2 points required for a curve
  this->IDiv = 2;
  this->JDiv = 2;
  this->KDiv = 2;
}

vtkMimxSolidTransfiniteInterpolation::~vtkMimxSolidTransfiniteInterpolation()
{
}


//----------------------------------------------------------------------------
int vtkMimxSolidTransfiniteInterpolation::RequestData(
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

  vtkImagePlaneWidget* imagepl = vtkImagePlaneWidget::New();
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
  double shapeI = 1.0/(this->IDiv - 1.0);
  double shapeJ = 1.0/(this->JDiv - 1.0);
  double shapeK = 1.0/(this->KDiv - 1.0);
  
  //        generate the internal nodes of a solid with the 6 surfaces known
  int count = 0;
  for(int k = 1; k < this->KDiv-1; k++)
  {
          for(int j=1; j < this->JDiv-1; j++)
          {
                  for(int i = 1; i < this->IDiv-1; i++)
                  {
                          interp[0] = 0.0;        interp[1] = 0.0; interp[2] = 0.0;
                          double x[3];
////                          this->mPoints->GetPoint(this->NodeIndex[i][0][0],x);
//                          points->GetPoint(i,x);
//                          ////cout <<i<<"  ";
//                          interp[0] = interp[0] + (1.0 - shapeK*k)*(1.0 - shapeJ*j)*x[0];
//                          interp[1] = interp[1] + (1.0 - shapeK*k)*(1.0 - shapeJ*j)*x[1];
//                          interp[2] = interp[2] + (1.0 - shapeK*k)*(1.0 - shapeJ*j)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[i][0][this->z_div],x);
//                          points->GetPoint(this->JDiv*this->IDiv*(this->KDiv-1)+i,x);
//                          ////cout<<this->JDiv*this->IDiv*(this->KDiv-1)+i<<"  ";
//                          interp[0] = interp[0] + (shapeK*k)*(1.0 - shapeJ*j)*x[0];
//                          interp[1] = interp[1] + (shapeK*k)*(1.0 - shapeJ*j)*x[1];
//                          interp[2] = interp[2] + (shapeK*k)*(1.0 - shapeJ*j)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[i][this->y_div][this->z_div],x);
//                          points->GetPoint(this->JDiv*this->IDiv*this->KDiv -this->IDiv+i,x);
//                          ////cout<<this->JDiv*this->IDiv*this->KDiv -this->IDiv+i<<"  ";
//                          interp[0] = interp[0] + (shapeK*k)*(shapeJ*j)*x[0];
//                          interp[1] = interp[1] + (shapeK*k)*(shapeJ*j)*x[1];
//                          interp[2] = interp[2] + (shapeK*k)*(shapeJ*j)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[i][this->y_div][0],x);
//                          points->GetPoint(this->JDiv*this->IDiv-this->IDiv+i,x);
//                          ////cout<<this->JDiv*this->IDiv-this->IDiv+i<<"  ";
//                          interp[0] = interp[0] + (1.0 - shapeK*k)*(shapeJ*j)*x[0];
//                          interp[1] = interp[1] + (1.0 - shapeK*k)*(shapeJ*j)*x[1];
//                          interp[2] = interp[2] + (1.0 - shapeK*k)*(shapeJ*j)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[0][j][0],x);
//                          points->GetPoint(j*this->IDiv , x);
//                          ////cout<<j*this->IDiv<<"  ";
//                          interp[0] = interp[0] + (1.0 - shapeI*i)*(1.0 - shapeK*k)*x[0];
//                          interp[1] = interp[1] + (1.0 - shapeI*i)*(1.0 - shapeK*k)*x[1];
//                          interp[2] = interp[2] + (1.0 - shapeI*i)*(1.0 - shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[0][j][this->z_div],x);
//                          points->GetPoint(this->IDiv*this->JDiv*(this->KDiv-1) + j*this->IDiv,x);
//                          ////cout<< this->IDiv*this->JDiv*(this->KDiv-1) + j*this->IDiv<<"  ";
//                          interp[0] = interp[0] + (1.0 - shapeI*i)*(shapeK*k)*x[0];
//                          interp[1] = interp[1] + (1.0 - shapeI*i)*(shapeK*k)*x[1];
//                          interp[2] = interp[2] + (1.0 - shapeI*i)*(shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[this->x_div][j][this->z_div],x);
//                          points->GetPoint(this->IDiv*this->JDiv*(this->KDiv-1) + (j+1)*this->IDiv-1,x);
//                          ////cout << this->IDiv*this->JDiv*(this->KDiv-1) + (j+1)*this->IDiv-1<<"  ";
//                          interp[0] = interp[0] + (shapeI*i)*(shapeK*k)*x[0];
//                          interp[1] = interp[1] + (shapeI*i)*(shapeK*k)*x[1];
//                          interp[2] = interp[2] + (shapeI*i)*(shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[this->x_div][j][0],x);
//                          points->GetPoint(this->IDiv*(j+1)-1,x);
//                          ////cout<<this->IDiv*(j+1)-1<<"  ";
//                          interp[0] = interp[0] + (shapeI*i)*(1.0 - shapeK*k)*x[0];
//                          interp[1] = interp[1] + (shapeI*i)*(1.0 - shapeK*k)*x[1];
//                          interp[2] = interp[2] + (shapeI*i)*(1.0 - shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[0][0][k],x);
//                          points->GetPoint(this->IDiv*this->JDiv*k , x);
//                          ////cout<<this->IDiv*this->JDiv*k <<"  ";
//                          interp[0] = interp[0] + (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[0];
//                          interp[1] = interp[1] + (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[1];
//                          interp[2] = interp[2] + (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[0][this->y_div][k],x);
//                          points->GetPoint(this->IDiv*this->JDiv*k + this->IDiv*(this->JDiv-1),x);
//                          ////cout<<this->IDiv*this->JDiv*k + this->IDiv*(this->JDiv-1)<<"  ";
//                          interp[0] = interp[0] + (1.0 - shapeI*i)*(shapeJ*j)*x[0];
//                          interp[1] = interp[1] + (1.0 - shapeI*i)*(shapeJ*j)*x[1];
//                          interp[2] = interp[2] + (1.0 - shapeI*i)*(shapeJ*j)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[this->x_div][this->y_div][k],x);
//                          points->GetPoint(this->IDiv*this->JDiv*(k+1)-1,x);
//                          ////cout<<this->IDiv*this->JDiv*(k+1)-1<<"  ";
//                          interp[0] = interp[0] + (shapeI*i)*(shapeJ*j)*x[0];
//                          interp[1] = interp[1] + (shapeI*i)*(shapeJ*j)*x[1];
//                          interp[2] = interp[2] + (shapeI*i)*(shapeJ*j)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[this->x_div][0][k],x);
//                          points->GetPoint(this->IDiv*this->JDiv*k+ this->IDiv-1,x);
//                          ////cout<<this->IDiv*this->JDiv*k+ this->IDiv-1<<"  ";
//                          interp[0] = interp[0] + (shapeI*i)*(1.0 - shapeJ*j)*x[0];
//                          interp[1] = interp[1] + (shapeI*i)*(1.0 - shapeJ*j)*x[1];
//                          interp[2] = interp[2] + (shapeI*i)*(1.0 - shapeJ*j)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[0][0][0],x);
//                          points->GetPoint(0,x);
//                          ////cout <<"0"<<"  ";
//                          interp[0] = interp[0] - 2.0*(1.0 - shapeI*i)*(1.0 - shapeJ*j)*(1.0 - shapeK*k)*x[0];
//                          interp[1] = interp[1] - 2.0*(1.0 - shapeI*i)*(1.0 - shapeJ*j)*(1.0 - shapeK*k)*x[1];
//                          interp[2] = interp[2] - 2.0*(1.0 - shapeI*i)*(1.0 - shapeJ*j)*(1.0 - shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[0][0][this->z_div],x);
//                          points->GetPoint(this->IDiv*this->JDiv*(this->KDiv-1),x);
//                          ////cout<<this->IDiv*this->JDiv*(this->KDiv-1)<<"  ";
//                          interp[0] = interp[0] - 2.0*(1.0 - shapeI*i)*(1.0 - shapeJ*j)*(shapeK*k)*x[0];
//                          interp[1] = interp[1] - 2.0*(1.0 - shapeI*i)*(1.0 - shapeJ*j)*(shapeK*k)*x[1];
//                          interp[2] = interp[2] - 2.0*(1.0 - shapeI*i)*(1.0 - shapeJ*j)*(shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[0][this->y_div][0],x);
//                          points->GetPoint(this->IDiv*(this->JDiv-1),x);
//                          ////cout <<this->IDiv*(this->JDiv-1)<<"  ";
//                          interp[0] = interp[0] - 2.0*(1.0 - shapeI*i)*(shapeJ*j)*(1.0 - shapeK*k)*x[0];
//                          interp[1] = interp[1] - 2.0*(1.0 - shapeI*i)*(shapeJ*j)*(1.0 - shapeK*k)*x[1];
//                          interp[2] = interp[2] - 2.0*(1.0 - shapeI*i)*(shapeJ*j)*(1.0 - shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[0][this->y_div][this->z_div],x);
//                          points->GetPoint(this->IDiv*this->JDiv*this->KDiv-this->IDiv,x);
//                          ////cout<<this->IDiv*this->JDiv*this->KDiv-this->IDiv<<"  ";
//                          interp[0] = interp[0] - 2.0*(1.0 - shapeI*i)*(shapeJ*j)*(shapeK*k)*x[0];
//                          interp[1] = interp[1] - 2.0*(1.0 - shapeI*i)*(shapeJ*j)*(shapeK*k)*x[1];
//                          interp[2] = interp[2] - 2.0*(1.0 - shapeI*i)*(shapeJ*j)*(shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[this->x_div][0][0],x);
//                          points->GetPoint(this->IDiv-1,x);
//                          ////cout<< this->IDiv-1<<"  ";
//                          interp[0] = interp[0] - 2.0*(shapeI*i)*(1.0 - shapeJ*j)*(1.0 - shapeK*k)*x[0];
//                          interp[1] = interp[1] - 2.0*(shapeI*i)*(1.0 - shapeJ*j)*(1.0 - shapeK*k)*x[1];
//                          interp[2] = interp[2] - 2.0*(shapeI*i)*(1.0 - shapeJ*j)*(1.0 - shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[this->x_div][0][this->z_div],x);
//                          points->GetPoint(this->IDiv*this->JDiv*(this->KDiv-1)+this->IDiv-1,x);
//                          ////cout<<this->IDiv*this->JDiv*(this->KDiv-1)+this->IDiv-1<<"  ";
//                          interp[0] = interp[0] - 2.0*(shapeI*i)*(1.0 - shapeJ*j)*(shapeK*k)*x[0];
//                          interp[1] = interp[1] - 2.0*(shapeI*i)*(1.0 - shapeJ*j)*(shapeK*k)*x[1];
//                          interp[2] = interp[2] - 2.0*(shapeI*i)*(1.0 - shapeJ*j)*(shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[this->x_div][this->y_div][0],x);
//                          points->GetPoint(this->IDiv*this->JDiv-1,x);
//                          ////cout<<this->IDiv*this->JDiv-1<<"  ";
//                          interp[0] = interp[0] - 2.0*(shapeI*i)*(shapeJ*j)*(1.0 - shapeK*k)*x[0];
//                          interp[1] = interp[1] - 2.0*(shapeI*i)*(shapeJ*j)*(1.0 - shapeK*k)*x[1];
//                          interp[2] = interp[2] - 2.0*(shapeI*i)*(shapeJ*j)*(1.0 - shapeK*k)*x[2];
////                          this->mPoints->GetPoint(this->NodeIndex[this->x_div][this->y_div][this->z_div],x);
//                          points->GetPoint(this->IDiv*this->JDiv*this->KDiv-1,x);
//                          ////cout<<this->IDiv*this->JDiv*this->KDiv-1<<endl;
//                          interp[0] = interp[0] - 2.0*(shapeI*i)*(shapeJ*j)*(shapeK*k)*x[0];
//                          interp[1] = interp[1] - 2.0*(shapeI*i)*(shapeJ*j)*(shapeK*k)*x[1];
//                          interp[2] = interp[2] - 2.0*(shapeI*i)*(shapeJ*j)*(shapeK*k)*x[2];
//                          points->SetPoint(k*this->IDiv*this->JDiv+j*this->IDiv+i,interp);
//                    //  ////cout<<k*this->IDiv*this->JDiv+j*this->IDiv+i<<"  "<< interp[0]<<"  "
//                                  //<<interp[1]<<"  "<<interp[2]<<std::endl;

                        int m;

                        points->GetPoint(k*this->JDiv*this->IDiv+i,x);
//                        //cout <<k*this->JDiv*this->IDiv+i<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (1.0-shapeJ*j)*x[m];

                        points->GetPoint(k*this->JDiv*this->IDiv+ this->IDiv*(this->JDiv-1)+i,x);
//                        //cout <<k*this->JDiv*this->IDiv+ this->IDiv*(this->JDiv-1)+i<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + shapeJ*j*x[m];

                        points->GetPoint(k*this->JDiv*this->IDiv+this->IDiv*j,x);
//                        //cout <<k*this->JDiv*this->IDiv+this->IDiv*j<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (1.0-shapeI*i)*x[m];

                        points->GetPoint(k*this->JDiv*this->IDiv+this->IDiv*j+this->IDiv-1,x);
//                        //cout << k*this->JDiv*this->IDiv+this->IDiv*j+this->IDiv-1<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + shapeI*i*x[m];

                        points->GetPoint(j*this->IDiv+i,x);
//                        //cout <<j*this->IDiv+i<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (1.0-shapeK*k)*x[m];

                        points->GetPoint((this->KDiv-1)*this->JDiv*this->IDiv+j*this->IDiv+i,x);
//                        //cout <<(this->KDiv-1)*this->JDiv*this->IDiv+j*this->IDiv+i<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + shapeK*k*x[m];

                        points->GetPoint(i,x);
//                        //cout <<i<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (1.0 - shapeK*k)*(1.0 - shapeJ*j)*x[m];

                        points->GetPoint(this->JDiv*this->IDiv*(this->KDiv-1)+i,x);
//                        //cout<<this->JDiv*this->IDiv*(this->KDiv-1)+i<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (shapeK*k)*(1.0 - shapeJ*j)*x[m];

                        points->GetPoint(this->JDiv*this->IDiv-this->IDiv+i,x);
//                        //cout<<this->JDiv*this->IDiv-this->IDiv+i<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (1.0 - shapeK*k)*(shapeJ*j)*x[m];

                        points->GetPoint(this->JDiv*this->IDiv*this->KDiv -this->IDiv+i,x);
//                        //cout<<this->JDiv*this->IDiv*this->KDiv -this->IDiv+i<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (shapeK*k)*(shapeJ*j)*x[m];

                        points->GetPoint(j*this->IDiv , x);
//                        //cout<<j*this->IDiv<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (1.0 - shapeI*i)*(1.0 - shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*(this->KDiv-1) + j*this->IDiv,x);
//                        //cout<< this->IDiv*this->JDiv*(this->KDiv-1) + j*this->IDiv<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (1.0 - shapeI*i)*(shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*(this->KDiv-1) + (j+1)*this->IDiv-1,x);
//                        //cout << this->IDiv*this->JDiv*(this->KDiv-1) + (j+1)*this->IDiv-1<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (shapeI*i)*(shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*(j+1)-1,x);
//                        //cout<<this->IDiv*(j+1)-1<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (shapeI*i)*(1.0 - shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*k , x);
//                        //cout<<this->IDiv*this->JDiv*k <<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*k + this->IDiv*(this->JDiv-1),x);
//                        //cout<<this->IDiv*this->JDiv*k + this->IDiv*(this->JDiv-1)<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (1.0 - shapeI*i)*(shapeJ*j)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*(k+1)-1,x);
//                        //cout<<this->IDiv*this->JDiv*(k+1)-1<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (shapeI*i)*(shapeJ*j)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*k+ this->IDiv-1,x);
//                        //cout<<this->IDiv*this->JDiv*k+ this->IDiv-1<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] - (shapeI*i)*(1.0 - shapeJ*j)*x[m];

                        points->GetPoint(0,x);
//                        //cout <<"0"<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (1.0 - shapeI*i)*(1.0 - shapeJ*j)*(1.0 - shapeK*k)*x[m];

                        points->GetPoint(this->IDiv-1,x);
//                        //cout<< this->IDiv-1<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (shapeI*i)*(1.0 - shapeJ*j)*(1.0 - shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*(this->KDiv-1)+this->IDiv-1,x);
//                        //cout<<this->IDiv*this->JDiv*(this->KDiv-1)+this->IDiv-1<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (shapeI*i)*(1.0 - shapeJ*j)*(shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv-1,x);
//                        //cout<<this->IDiv*this->JDiv-1<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (shapeI*i)*(shapeJ*j)*(1.0 - shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*this->KDiv-1,x);
//                        //cout<<this->IDiv*this->JDiv*this->KDiv-1<<endl;
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (shapeI*i)*(shapeJ*j)*(shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*(this->KDiv-1),x);
//                        //cout<<this->IDiv*this->JDiv*(this->KDiv-1)<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (1.0 - shapeI*i)*(1.0 - shapeJ*j)*(shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*(this->JDiv-1),x);
//                        //cout <<this->IDiv*(this->JDiv-1)<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (1.0 - shapeI*i)*(shapeJ*j)*(1.0 - shapeK*k)*x[m];

                        points->GetPoint(this->IDiv*this->JDiv*this->KDiv-this->IDiv,x);
//                        //cout<<this->IDiv*this->JDiv*this->KDiv-this->IDiv<<"  ";
                        for (m = 0; m <3; m++)        interp[m] = interp[m] + (1.0 - shapeI*i)*(shapeJ*j)*(shapeK*k)*x[m];

                        points->SetPoint(k*this->IDiv*this->JDiv+j*this->IDiv+i,interp);
                  }
          }
  }

  output->SetPoints(points);
  points->Delete();
 
  return 1;
}


void vtkMimxSolidTransfiniteInterpolation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
