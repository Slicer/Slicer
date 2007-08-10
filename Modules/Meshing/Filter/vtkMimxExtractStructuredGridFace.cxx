/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractStructuredGridFace.cxx,v $
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

#include "vtkMimxExtractStructuredGridFace.h"

#include "vtkCellArray.h"
#include "vtkCollection.h"
#include "vtkExecutive.h"
#include "vtkHexahedron.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxExtractStructuredGridFace, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxExtractStructuredGridFace);

// Construct object to extract all of the input data.
vtkMimxExtractStructuredGridFace::vtkMimxExtractStructuredGridFace()
{
        this->FaceNum = -1;
}

vtkMimxExtractStructuredGridFace::~vtkMimxExtractStructuredGridFace()
{
        
}

int vtkMimxExtractStructuredGridFace::RequestData(
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
        
        int numNodes = input->GetNumberOfPoints();
        int numCells = input->GetNumberOfCells();

        if(numNodes <= 0 || numCells <= 0){vtkErrorMacro("Invalid input");
        return 0;}
        if(FaceNum == -1){        vtkErrorMacro("Face Number not set");
        return 0;}
        vtkPoints *points = vtkPoints::New();
        this->GetFace(FaceNum, points, input);
        int dimin[3], dimout[3];
        input->GetDimensions(dimin);
        if(FaceNum == 0 || FaceNum == 1){ dimout[0] = dimin[1];
        dimout[1] = dimin[2];}
        else if(FaceNum == 2 || FaceNum == 3){ dimout[0] = dimin[0];
        dimout[1] = dimin[2];}
        else{ dimout[0] = dimin[0]; dimout[1] = dimin[1];}
        dimout[2] = 1;
        output->SetPoints(points);
        output->SetDimensions(dimout);
        points->Delete();

  return 1;
}

void vtkMimxExtractStructuredGridFace::GetFace(int FaceNum, vtkPoints* PointList, vtkStructuredGrid *Input)
{
        switch (FaceNum) {
                case 0:
                        GetFace0(PointList, Input);
                        break;
                case 1:
                        GetFace1(PointList, Input);
                        break;
                case 2:
                        GetFace2(PointList, Input);
                        break;
                case 3:
                        GetFace3(PointList, Input);
                        break;
                case 4:
                        GetFace4(PointList, Input);
                        break;
                case 5:
                        GetFace5(PointList, Input);
                        break;
        }
}

void vtkMimxExtractStructuredGridFace::GetFace0(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
        Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointList->InsertPoint(y*dim[1] + x, Input
                                ->GetPoint(y*dim[0]*dim[1]+x*dim[0]));
//                        //cout << y*dim[1]+x<<"  "<<y*dim[0]*dim[1]+x*dim[0]<<endl;
                }
        }
}

void vtkMimxExtractStructuredGridFace::GetFace1(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointList->InsertPoint(y*dim[1] + x, Input
                                ->GetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1));
//                        //cout <<y*dim[1] + x<<"  "<<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<endl;
                }
        }
}

void vtkMimxExtractStructuredGridFace::GetFace2(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointList->InsertPoint(y*dim[0] + z, Input
                                ->GetPoint(y*dim[0]*dim[1]+z));
//                        //cout << y*dim[0] + z<<"  "<<y*dim[0]*dim[1]+z<<"  "<<endl;
                }
        }
}

void vtkMimxExtractStructuredGridFace::GetFace3(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointList->InsertPoint(y*dim[0] + z,Input
                                ->GetPoint(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z));
//                        //cout << y*dim[0] + z<<"  "<<y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z<<"  "<<endl;
                }
        }
}

void vtkMimxExtractStructuredGridFace::GetFace4(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]*dim[1]);
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointList->InsertPoint(x*dim[0] + z, Input
                                ->GetPoint(x*dim[0]+z));
//                        //cout<<x*dim[0] + z<<"  "<<x*dim[0]+z<<endl;
                }
        }
}

void vtkMimxExtractStructuredGridFace::GetFace5(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]*dim[1]);
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointList->InsertPoint(x*dim[0] + z, Input
                                ->GetPoint((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z));
//                        //cout<<x*dim[0] + z<<"  "<<(dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z<<endl;
                }
        }
}


void vtkMimxExtractStructuredGridFace::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
