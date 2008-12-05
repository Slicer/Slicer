/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSolidEllipticalInterpolation.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.4 $

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

#include "vtkMimxSolidEllipticalInterpolation.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointSet.h"
#include "vtkStructuredGrid.h"
#include "vtkMimxSolidTransfiniteInterpolation.h"

vtkCxxRevisionMacro(vtkMimxSolidEllipticalInterpolation, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkMimxSolidEllipticalInterpolation);

// Construct object to extract all of the input data.
vtkMimxSolidEllipticalInterpolation::vtkMimxSolidEllipticalInterpolation()
{
        this->ErrorLevel = 1e-3;
        this->NumberOfIterations = 10;
}

vtkMimxSolidEllipticalInterpolation::~vtkMimxSolidEllipticalInterpolation()
{
}

int vtkMimxSolidEllipticalInterpolation::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
        vtkInformation *outInfo = outputVector->GetInformationObject(0);

        vtkStructuredGrid *input = vtkStructuredGrid::SafeDownCast(
                inInfo->Get(vtkDataObject::DATA_OBJECT()));
        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));

        int dim[3];
        input->GetDimensions(dim);
        
        // estimate the initial location of the interior nodes
        vtkMimxSolidTransfiniteInterpolation *interp = vtkMimxSolidTransfiniteInterpolation::New();
        vtkPointSet *poiset = vtkStructuredGrid::New();
        vtkPoints *points = vtkPoints::New();
        points->DeepCopy(input->GetPoints());
        poiset->SetPoints(points);
        interp->SetInput(poiset);
        interp->SetIDiv(dim[0]);
        interp->SetJDiv(dim[1]);
        interp->SetJDiv(dim[2]);
        interp->Update();
//      interp->GetOutput()->GetPoints()->SetPoint(13, 4.0,5.0,6.0);

        // loop through number of iterations
        for (int iter = 0; iter < this->NumberOfIterations; iter++) 
        {
                for (int k =1; k < dim[2]-1; k++) 
                {
                        for (int j = 1; j < dim[1]-1; j++) 
                        {
                                for (int i = 1; i < dim[0]-1; i++) 
                                {
                                        double eta[3], psi[3], rho[3], psieta[3], psirho[3], etarho[3], coortemp[3];
                                        double alpha11, alpha22, alpha33, alpha12, alpha23, alpha13;
                                        double a11, a22, a33, a12, a23, a13;
                                        double coor1m1p1[3], coorm1m11[3], coor1m11[3], coorp1m11[3], coor1m1m1[3];
                                        double coor1p1p1[3], coorm1p11[3], coor1p11[3], coorp1p11[3], coor1p1m1[3];
                                        double coorm11p1[3], coor11p1[3], coorp11p1[3], 
                                                coorm111[3], coor111[3], coorp111[3],
                                                coorm11m1[3], coor11m1[3], coorp11m1[3];
                                        // get all the surrounding points.
                                        //1
                                        interp->GetOutput()->GetPoint((k-1)*dim[0]*dim[1]+j*dim[0]+i +1, coor1m1p1);
                                        //cout<<(k-1)*dim[0]*dim[1]+j*dim[0]+i +1<<endl;
                                        //2
                                        interp->GetOutput()->GetPoint((k-1)*dim[0]*dim[1]+(j-1)*dim[0]+i , coorm1m11);
                                        //cout<<(k-1)*dim[0]*dim[1]+(j-1)*dim[0]+i<<endl;
                                        //3
                                        interp->GetOutput()->GetPoint((k-1)*dim[0]*dim[1]+j*dim[0]+i, coor1m11);
                                        //cout<<(k-1)*dim[0]*dim[1]+j*dim[0]+i<<endl;
                                        //4
                                        interp->GetOutput()->GetPoint((k-1)*dim[0]*dim[1]+(j+1)*dim[0]+i, coorp1m11);
                                        //cout<<(k-1)*dim[0]*dim[1]+(j+1)*dim[0]+i<<endl;
                                        //5
                                        interp->GetOutput()->GetPoint((k-1)*dim[0]*dim[1]+j*dim[0]+i-1,  coor1m1m1);
                                        //cout<<(k-1)*dim[0]*dim[1]+j*dim[0]+i-1<<endl;
                                        //6
                                        interp->GetOutput()->GetPoint(k*dim[0]*dim[1]+(j-1)*dim[0]+i+1, coorm11p1);
                                        //cout<< k*dim[0]*dim[1]+(j-1)*dim[0]+i+1 <<endl;
                                        //7
                                        interp->GetOutput()->GetPoint(k*dim[0]*dim[1]+j*dim[0]+i +1, coor11p1);
                                        //cout<<k*dim[0]*dim[1]+j*dim[0]+i +1<<endl;
                                        //8
                                        interp->GetOutput()->GetPoint(k*dim[0]*dim[1]+(j+1)*dim[0]+i +1, coorp11p1);
                                        //cout<< k*dim[0]*dim[1]+(j+1)*dim[0]+i +1 <<endl;
                                        //9
                                        interp->GetOutput()->GetPoint(k*dim[0]*dim[1]+(j-1)*dim[0]+i , coorm111);
                                        //cout<< k*dim[0]*dim[1]+(j-1)*dim[0]+i <<endl;
                                        //10
                                        interp->GetOutput()->GetPoint(k*dim[0]*dim[1]+j*dim[0]+i , coor111);
                                        //cout<< k*dim[0]*dim[1]+j*dim[0]+i  <<endl;
                                        //11
                                        interp->GetOutput()->GetPoint(k*dim[0]*dim[1]+(j+1)*dim[0]+i , coorp111);
                                        //cout<<k*dim[0]*dim[1]+(j+1)*dim[0]+i <<endl;
                                        //12
                                        interp->GetOutput()->GetPoint(k*dim[0]*dim[1]+(j-1)*dim[0]+i-1, coorm11m1);
                                        //cout<< k*dim[0]*dim[1]+(j-1)*dim[0]+i-1 <<endl;
                                        //13
                                        interp->GetOutput()->GetPoint(k*dim[0]*dim[1]+j*dim[0]+i-1, coor11m1);
                                        //cout<<k*dim[0]*dim[1]+j*dim[0]+i-1<<endl;
                                        //14
                                        interp->GetOutput()->GetPoint(k*dim[0]*dim[1]+(j+1)*dim[0]+i-1, coorp11m1);
                                        //cout<<k*dim[0]*dim[1]+(j+1)*dim[0]+i-1<<endl;
                                        //15
                                        interp->GetOutput()->GetPoint((k+1)*dim[0]*dim[1]+j*dim[0]+i +1, coor1p1p1);
                                        //cout<<(k+1)*dim[0]*dim[1]+j*dim[0]+i +1<<endl;
                                        //16
                                        interp->GetOutput()->GetPoint((k+1)*dim[0]*dim[1]+(j-1)*dim[0]+i, coorm1p11);
                                        //cout<<(k+1)*dim[0]*dim[1]+(j-1)*dim[0]+i<<endl;
                                        //17
                                        interp->GetOutput()->GetPoint((k+1)*dim[0]*dim[1]+j*dim[0]+i, coor1p11);
                                        //cout<< (k+1)*dim[0]*dim[1]+j*dim[0]+i <<endl;
                                        //18
                                        interp->GetOutput()->GetPoint((k+1)*dim[0]*dim[1]+(j+1)*dim[0]+i, coorp1p11);
                                        //cout<<(k+1)*dim[0]*dim[1]+(j+1)*dim[0]+i<<endl;
                                        //19
                                        interp->GetOutput()->GetPoint((k+1)*dim[0]*dim[1]+j*dim[0]+i-1, coor1p1m1);
                                        //cout<<(k+1)*dim[0]*dim[1]+j*dim[0]+i-1<<endl;
                                        
                                        //first derivatives
                                        int m;
                                        for (m = 0; m < 3; m++)         psi[m] = (coorp111[m]-coorm111[m])/2.0;
                                        for (m = 0; m < 3; m++)         eta[m] = (coor1p11[m]-coor1m11[m])/2.0;
                                        for (m = 0; m < 3; m++)         rho[m] = (coor11p1[m]-coor11m1[m])/2.0;
                                        //other variables
                                        a11 = vtkMath::Dot(psi,psi);    a22 = vtkMath::Dot(eta,eta);
                                        a33 = vtkMath::Dot(rho,rho);    a12 = vtkMath::Dot(psi,eta);
                                        a13 = vtkMath::Dot(psi,rho);    a23 = vtkMath::Dot(eta,rho);

                                        alpha11 = a22*a33 - a23*a23;    alpha22 = a11*a33 - a13*a13;
                                        alpha33 = a11*a22 - a12*a12;    alpha12 = a13*a23 - a12*a33;
                                        alpha13 = a12*a23 - a13*a22;    alpha23 = a13*a12 - a11*a23;

                                        // mixed derivatives
                                        for (m=0; m<3; m++) psieta[m] = ((coorp1p11[m] - coorp1m11[m]) - 
                                                (coorm1p11[m] - coorm1m11[m]))/4.0;
                                        for (m=0; m<3; m++) psirho[m] = ((coorp11p1[m] - coorp11m1[m]) - 
                                                (coorm11p1[m] - coorm11m1[m]))/4.0;
                                        for (m=0; m<3; m++) etarho[m] = ((coor1p1p1[m] - coor1p1m1[m]) - 
                                                (coor1m1p1[m] - coor1m1m1[m]))/4.0;
                                        
                                        // final calculation
                                        for (m=0; m<3;m++) {
                                                coortemp[m] = (1.0/(2.0*(alpha11 + alpha22 + alpha33)))*
                                                        (2.0*alpha12*psieta[m] + 2.0*alpha13*psirho[m] + 2.0*alpha23*etarho[m]
                                                + alpha11*(coorp111[m] + coorm111[m]) + alpha22*(coor1p11[m] + coor1m11[m])
                                                        + alpha33*(coor11p1[m] + coor11m1[m]));
                                        }
                                        interp->GetOutput()->GetPoints()->SetPoint(k*dim[0]*dim[1]+j*dim[0]+i, coortemp);
                                }
                        }

                }

        }

        output->SetDimensions(input->GetDimensions());
        output->SetPoints(interp->GetOutput()->GetPoints());
        interp->Delete();
        poiset->Delete();
        points->Delete();

        return 1;
}

void vtkMimxSolidEllipticalInterpolation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
