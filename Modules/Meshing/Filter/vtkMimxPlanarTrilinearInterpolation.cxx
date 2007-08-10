/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPlanarTrilinearInterpolation.cxx,v $
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

#include "vtkMimxPlanarTrilinearInterpolation.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"
#include "vtkImagePlaneWidget.h"

vtkCxxRevisionMacro(vtkMimxPlanarTrilinearInterpolation, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkMimxPlanarTrilinearInterpolation);

vtkMimxPlanarTrilinearInterpolation::vtkMimxPlanarTrilinearInterpolation()
{
  // Minimum of 2 points required 
  this->IDiv = 2;
  this->JDiv = 2;
}

vtkMimxPlanarTrilinearInterpolation::~vtkMimxPlanarTrilinearInterpolation()
{
}


//----------------------------------------------------------------------------
int vtkMimxPlanarTrilinearInterpolation::RequestData(
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

  
  // Generate the internal nodes of a solid from the known nodes on each face
  int count = 0;
        for(int j=1; j < this->JDiv-1; j++)
          {
          for(int i = 1; i < this->IDiv-1; i++)
                {        
                double pt[3];        
        // Get the Current Point
        points->GetPoint(j*this->IDiv+i,pt);
                 cout << j*this->IDiv+i<<endl;    
        // Get the I Bounds
        double x0[3], x1[3];
        points->GetPoint(j*this->IDiv,x0);
        points->GetPoint((j+1)*this->IDiv+ -1,x1);
                 cout <<  j*this->IDiv<<endl;
                 cout <<  (j+1)*this->IDiv+ -1<<endl;
        // Get the J Bounds
        double y0[3], y1[3];
        points->GetPoint(i,y0);
        points->GetPoint(this->IDiv*(this->JDiv-1)+i,y1);
                 cout <<  i<<endl;
                 cout <<  this->IDiv*(this->JDiv-1)+i<<endl;
 
                double x[3], y[3];
                int m;
                for (m=0; m <3; m++)
                {
                        x[m] = x0[m] + (x1[m] - x0[m])*static_cast<double>(i)/static_cast<double>(this->IDiv-1);
                }
                for (m=0; m <3; m++)
                {
                        y[m] = y0[m] + (y1[m] - y0[m])*static_cast<double>(j)/static_cast<double>(this->JDiv-1);
                }
                                
        double interp[3];
                for (m =0; m<3; m++)
                {
                        interp[m] = (x[m]+y[m])/2.0;
                }
        /* Debug Printing
         * std::// cout << "Division " << this->IDiv << " " << this->JDiv << " " << this->KDiv << std::endl;
         * std::// cout << "Index " << k*this->IDiv*this->JDiv+j*this->IDiv+i << std::endl;
         * std::// cout << "Loc " << i << " " << j << " " << k << std::endl;
         * std::// cout << "X0 " << k*this->JDiv*this->IDiv+j*this->IDiv << " " << x0[0] << " " << x0[1] << " " << x0[2] << std::endl;
         * std::// cout << "X1 " << k*this->JDiv*this->IDiv+j*this->IDiv+this->IDiv-1 << " " << x1[0] << " " << x1[1] << " " << x1[2] << std::endl;
         * std::// cout << "Y0 " << k*this->JDiv*this->IDiv+i << " "  << y0[0] << " " << y0[1] << " " << y0[2] << std::endl;
         * std::// cout << "Y1 " << k*this->JDiv*this->IDiv+(this->JDiv-1)*this->IDiv+i << " " << y1[0] << " " << y1[1] << " " << y1[2] << std::endl;
         * std::// cout << "Z0 " << j*this->IDiv+i << " "  << z0[0] << " " << z0[1] << " " << z0[2] << std::endl;
         * std::// cout << "Z1 " << (this->KDiv-1)*this->JDiv*this->IDiv+j*this->IDiv+i << " " << z1[0] << " " << z1[1] << " " << z1[2] << std::endl;
         * std::// cout << "New Point  " << interp[0] << " " << interp[1] << " " << interp[2] << std::endl;
         * std::// cout << "Distance " << distanceI << " " << distanceJ << " " << distanceK << std::endl;
         */
                points->SetPoint(j*this->IDiv+i,interp);
                }
          }

  output->SetPoints(points);
  points->Delete();
 
  return 1;
}


void vtkMimxPlanarTrilinearInterpolation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

