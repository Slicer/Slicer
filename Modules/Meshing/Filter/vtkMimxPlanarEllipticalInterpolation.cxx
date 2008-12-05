/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPlanarEllipticalInterpolation.cxx,v $
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

#include "vtkMimxPlanarEllipticalInterpolation.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointSet.h"
#include "vtkStructuredGrid.h"
#include "vtkMimxPlanarTransfiniteInterpolation.h"

vtkCxxRevisionMacro(vtkMimxPlanarEllipticalInterpolation, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkMimxPlanarEllipticalInterpolation);

// Construct object to extract all of the input data.
vtkMimxPlanarEllipticalInterpolation::vtkMimxPlanarEllipticalInterpolation()
{
        this->ErrorLevel = 1e-3;
        this->NumberOfIterations = 1;
}

vtkMimxPlanarEllipticalInterpolation::~vtkMimxPlanarEllipticalInterpolation()
{
}

int vtkMimxPlanarEllipticalInterpolation::RequestData(
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

        if(dim[2] != 1)
        {
                return 0;
        }
        
        // estimate the initial location of the interior nodes
        vtkMimxPlanarTransfiniteInterpolation *interp = vtkMimxPlanarTransfiniteInterpolation::New();
        vtkPointSet *poiset = vtkStructuredGrid::New();
        vtkPoints *points = vtkPoints::New();
        points->DeepCopy(input->GetPoints());
        poiset->SetPoints(points);
        interp->SetInput(poiset);
        interp->SetIDiv(dim[0]);
        interp->SetJDiv(dim[1]);
        interp->Update();

        // loop through number of iterations
        for (int k =0; k < this->NumberOfIterations; k++) 
        {
                double err = 0.0;
                for (int j = 1; j < dim[1]-1; j++) 
                {
                        for (int i = 1; i < dim[0]-1; i++) 
                        {
//                              double xeta, xpsi, xetaeta, xpsipsi, xpsieta, xetapsi;
//                              double yeta, ypsi, yetaeta, ypsipsi, ypsieta, yetapsi;
                                double g11, g22, g12, errortemp = 0.0;
                                double coorm1m1[3], coorm1p1[3], coorp1m1[3], coorp1p1[3]
                                , coorm11[3], coorp11[3], coor1m1[3], coor1p1[3],X[3],XTemp[3];
                                // get all the surrounding points.
                                interp->GetOutput()->GetPoint((j-1)*dim[0] + i-1, coorm1m1);
                                //cout<<(j-1)*dim[0] + i-1<<endl;
                                interp->GetOutput()->GetPoint((j-1)*dim[0] + i, coor1m1);
                                //cout<<(j-1)*dim[0] + i<<endl;
                                interp->GetOutput()->GetPoint((j-1)*dim[0] + i+1, coorp1m1);
                                //cout<<(j-1)*dim[0] + i+1<<endl;
                                interp->GetOutput()->GetPoint(j*dim[0] + i-1, coorm11);
                                //cout<<j*dim[0] + i-1<<endl;
                                interp->GetOutput()->GetPoint(j*dim[0] + i +1, coorp11);
                                //cout<<j*dim[0] + i +1<<endl;
                                interp->GetOutput()->GetPoint((j+1)*dim[0] + i-1, coorm1p1);
                                //cout<<(j+1)*dim[0] + i-1<<endl;
                                interp->GetOutput()->GetPoint((j+1)*dim[0] + i, coor1p1);
                                //cout<<(j+1)*dim[0] + i<<endl;
                                interp->GetOutput()->GetPoint((j+1)*dim[0] + i+1, coorp1p1);
                                //cout<<(j+1)*dim[0] + i+1<<endl;
                                interp->GetOutput()->GetPoint(j*dim[0] + i, X);
                                //cout<<j*dim[0] + i<<endl;

                                g11 = (pow(coorp11[0] - coorm11[0], 2.0) + pow(coorp11[1] - coorm11[1], 2.0)
                                        + pow(coorp11[2] - coorm11[2], 2.0))/4.0;
                                g22 = (pow(coor1p1[0] - coor1m1[0], 2.0) + pow(coor1p1[1] - coor1m1[1], 2.0)
                                        + pow(coor1p1[2] - coor1m1[2], 2.0))/4.0;
                                g12 = ((coorp11[0] - coorm11[0])*(coor1p1[0] - coor1m1[0]) + 
                                        (coorp11[1] - coorm11[1])*(coor1p1[1] - coor1m1[1])
                                        + (coorp11[2] - coorm11[2])*(coor1p1[2] - coor1m1[2]))/4.0;
                                
//                              XTemp[2] = 0.0;
                                for(int m=0; m <3; m++)
                                {
                                        XTemp[m] = (1.0/(2.0*(g11+g22)))*(g22*coorp11[m] - 0.5*g12*coorp1p1[m] + 
                                                0.5*g12*coorp1m1[m] + g11*coor1p1[m] + g11*coor1m1[m] + g22*coorm11[m] 
                                        - 0.5*g12*coorm1m1[m] + 0.5*g12*coorm1p1[m]);
                                }
                                
                                err = err + vtkMath::Distance2BetweenPoints(X,XTemp);
                                interp->GetOutput()->GetPoints()->SetPoint(j*dim[0] + i, XTemp);
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

void vtkMimxPlanarEllipticalInterpolation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
