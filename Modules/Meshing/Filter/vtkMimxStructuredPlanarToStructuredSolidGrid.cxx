/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxStructuredPlanarToStructuredSolidGrid.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.6 $

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

#include "vtkMimxStructuredPlanarToStructuredSolidGrid.h"
#include "vtkMimxSubdivideCurve.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnsignedCharArray.h"
#include "vtkMimxPlanarTransfiniteInterpolation.h"
#include "vtkMimxSolidTransfiniteInterpolation.h"

vtkCxxRevisionMacro(vtkMimxStructuredPlanarToStructuredSolidGrid, "$Revision: 1.6 $");
vtkStandardNewMacro(vtkMimxStructuredPlanarToStructuredSolidGrid);

// Construct object to extract all of the input data.
vtkMimxStructuredPlanarToStructuredSolidGrid::vtkMimxStructuredPlanarToStructuredSolidGrid()
{
        this->Dimensions[0] = 0; this->Dimensions[1] = 0; this->Dimensions[2] = 0;
        this->SetNumberOfInputPorts(6);
}

vtkMimxStructuredPlanarToStructuredSolidGrid::~vtkMimxStructuredPlanarToStructuredSolidGrid()
{
}

int vtkMimxStructuredPlanarToStructuredSolidGrid::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        // get the info objects
        vtkPoints *points = vtkPoints::New();
        vtkInformation *outInfo = outputVector->GetInformationObject(0);

        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkInformation *inInfo;
    vtkStructuredGrid *input[6];
        inInfo = inputVector[0]->GetInformationObject(0);
        input[0] = vtkStructuredGrid::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
        inInfo = inputVector[2]->GetInformationObject(0);
        input[2] = vtkStructuredGrid::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
        
        int dims[3];
        input[0]->GetDimensions(dims);
        this->Dimensions[1] = dims[0];
        this->Dimensions[2] = dims[1];
        input[2]->GetDimensions(dims);
        this->Dimensions[0] = dims[0];
        if(this->Dimensions[0] <= 0 || this->Dimensions[1] <= 0 || 
                this->Dimensions[2] <= 0)
        {       vtkErrorMacro(<<
        "Structured grid dimensions not set"<< endl);
        points->Delete();
        return 0;
        }
        output->SetDimensions(this->Dimensions);
        points->SetNumberOfPoints(this->Dimensions[0]*
                this->Dimensions[1]*this->Dimensions[2]);

        output->SetPoints(points);

        vtkIntArray *intarray = vtkIntArray::New();
        intarray->SetNumberOfValues(this->Dimensions[0]*
                this->Dimensions[1]*this->Dimensions[2]);
        for (int i=0; i<this->Dimensions[0]*
                this->Dimensions[1]*this->Dimensions[2]; i++) {
                intarray->SetValue(i, 1);
        }
        output->GetPointData()->SetScalars(intarray);
        intarray->Delete();

        for(int i=0; i < 6; i++)
        {
                vtkInformation *inInfo;
                inInfo = NULL;
                inInfo = inputVector[i]->GetInformationObject(0);
                if(inInfo){
                        input[i]  = NULL;
                        input[i] = vtkStructuredGrid::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
                        if(input[i]->GetNumberOfPoints() < 2)
                        {
                                vtkErrorMacro(<<"Number of points defining  Edge  "<<i<<
                                        "  should be 2 or more"<<endl);
                                return 0;
                                points->Delete();
                        }
                        else{
                                this->SetFace(i,input[i]->GetPoints());
                        }
                }
                else
                {
                        vtkErrorMacro(<<"Data not set for Face  "<<i << endl);
                        points->Delete();
                        return 0;
                }
        }
        double x[3];
        for(int i=0; i<output->GetNumberOfPoints(); i++)
        {
                output->GetPoint(i,x);
                //cout <<i<<"  "<<x[0]<<"  "<<x[1]<<"  "<<x[2]<<endl;
        }
        // generate the interior nodes of the solid mesh
        vtkMimxSolidTransfiniteInterpolation *solidinterp = vtkMimxSolidTransfiniteInterpolation::New();
        solidinterp->SetIDiv(this->Dimensions[0]);
        solidinterp->SetJDiv(this->Dimensions[1]);
        solidinterp->SetKDiv(this->Dimensions[2]);
        vtkPointSet *pointset = vtkStructuredGrid::New();
        pointset->SetPoints(points);
        solidinterp->SetInput(pointset);
        solidinterp->Update();
        output->SetPoints(solidinterp->GetOutput()->GetPoints());
        output->Modified();
        solidinterp->Delete();
        points->Delete();
        return 1;
}

int vtkMimxStructuredPlanarToStructuredSolidGrid::SetFace(int FaceNum, vtkPoints *PointsList)
{
        int dim[3];
        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
                this->GetOutputDataObject(0));
        output->GetDimensions(dim);
        double pt[3];
        vtkStructuredGrid *structuredgrid = vtkStructuredGrid::SafeDownCast(
                this->GetInput(FaceNum));

        if(FaceNum == 0)
        {
                for(int y=0; y < dim[2]; y++)
                {
                        for(int x=0; x < dim[1]; x++)
                        {
                                output->GetPoints()
                                        ->SetPoint(y*dim[0]*dim[1]+x*dim[0],
                                        PointsList->GetPoint(y*dim[1] + x));
                                PointsList->GetPoint(y*dim[1] + x,pt);
                                //cout<<y*dim[0]*dim[1]+x*dim[0]<<"  "<<y*dim[1] + x<<endl;
                                if(structuredgrid->GetPointData()->GetScalars())
                                {
                                        double status[1];
                                        structuredgrid->GetPointData()->GetScalars()->GetTuple(y*dim[1],status);
                                        if(!status[0])  output->GetPointData()->GetScalars()->SetTuple(
                                                y*dim[0]*dim[1]+x*dim[0],status);
                                }
                        }
                }
                return 1;
        }

        if(FaceNum == 1)
        {
                for(int y=0; y < dim[2]; y++)
                {
                        for(int x=0; x < dim[1]; x++)
                        {
                                vtkPoints *points1 = this->GetOutput()->GetPoints();
                                output->GetPoints()->SetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1,
                                        PointsList->GetPoint(y*dim[1] + x));
                                PointsList->GetPoint(y*dim[1] + x,pt);
                                if(structuredgrid->GetPointData()->GetScalars())
                                {
                                        double status[1];
                                        structuredgrid->GetPointData()->GetScalars()->GetTuple(y*dim[1],status);
                                        if(!status[0])  output->GetPointData()->GetScalars()->SetTuple(
                                                y*dim[0]*dim[1]+x*dim[0]+dim[0]-1,status);
                                }

                                //cout <<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<"  "<<y*dim[1] + x<<endl;
                        }
                }
                return 1;
        }

        if(FaceNum == 2)
        {
                for(int y=0; y < dim[2]; y++)
                {
                        for(int z=0; z < dim[0]; z++)
                        {
                                output->GetPoints()->SetPoint(
                                        y*dim[0]*dim[1]+z,PointsList->GetPoint(y*dim[0] + z));
                                PointsList->GetPoint(y*dim[0] + z,pt);
                                if(structuredgrid->GetPointData()->GetScalars())
                                {
                                        double status[1];
                                        structuredgrid->GetPointData()->GetScalars()->GetTuple(y*dim[0],status);
                                        if(!status[0])  output->GetPointData()->GetScalars()->SetTuple(
                                                y*dim[0]*dim[1]+z,status);
                                }

                                //cout << y*dim[0]*dim[1]+z<<endl;
                        }
                }
                return 1;
        }

        if(FaceNum == 3)
        {
                for(int y=0; y < dim[2]; y++)
                {
                        for(int z=0; z < dim[0]; z++)
                        {
                                output->GetPoints()->SetPoint
                                        (y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z,
                                        PointsList->GetPoint(y*dim[0] + z));
                                PointsList->GetPoint(y*dim[0] + z,pt);
                                if(structuredgrid->GetPointData()->GetScalars())
                                {
                                        double status[1];
                                        structuredgrid->GetPointData()->GetScalars()->GetTuple(y*dim[0] + z,status);
                                        if(!status[0])  output->GetPointData()->GetScalars()->SetTuple(
                                                (y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z),status);
                                }

                                //cout<<y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z<<endl;
                        }
                }
                return 1;
        }
        if(FaceNum == 4)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        for(int z=0; z < dim[0]; z++)
                        {
                                output->GetPoints()->SetPoint(x*dim[0]+z,
                                        PointsList->GetPoint(x*dim[0] + z));
                                PointsList->GetPoint(x*dim[0] + z,pt);
                                if(structuredgrid->GetPointData()->GetScalars())
                                {
                                        double status[1];
                                        structuredgrid->GetPointData()->GetScalars()->GetTuple(x*dim[0]+z,status);
                                        if(!status[0])  output->GetPointData()->GetScalars()->SetTuple(
                                                x*dim[0] + z,status);
                                }

                                //cout<<x*dim[0]+z<<endl;
                        }
                }
                return 1;
        }

        if(FaceNum == 5)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        for(int z=0; z < dim[0]; z++)
                        {
                                output->GetPoints()->SetPoint
                                        ((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z,
                                        PointsList->GetPoint(x*dim[0] + z));
                                PointsList->GetPoint(x*dim[0] + z,pt);
                                if(structuredgrid->GetPointData()->GetScalars())
                                {
                                        double status[1];
                                        structuredgrid->GetPointData()->GetScalars()->GetTuple(x*dim[0] + z,status);
                                        if(!status[0])  output->GetPointData()->GetScalars()->SetTuple(
                                                (dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z,status);
                                }

                                //cout<<(dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z<<endl;
                        }
                }
                return 1;
        }
}
void vtkMimxStructuredPlanarToStructuredSolidGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
