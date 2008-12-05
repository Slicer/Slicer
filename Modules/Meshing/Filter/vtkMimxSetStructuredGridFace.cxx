/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSetStructuredGridFace.cxx,v $
Language:  C++
Date:      $Date: 2008/04/27 03:34:28 $
Version:   $Revision: 1.7 $

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

#include "vtkMimxSetStructuredGridFace.h"

#include "vtkCellArray.h"
#include "vtkCollection.h"
#include "vtkExecutive.h"
#include "vtkHexahedron.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPointSet.h"
#include "vtkMimxSolidTransfiniteInterpolation.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxSetStructuredGridFace, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkMimxSetStructuredGridFace);

// Construct object to extract all of the input data.
vtkMimxSetStructuredGridFace::vtkMimxSetStructuredGridFace()
{
        this->FaceNum = -1;
        this->SetNumberOfInputPorts(1);
}

vtkMimxSetStructuredGridFace::~vtkMimxSetStructuredGridFace()
{
        
}

int vtkMimxSetStructuredGridFace::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
        vtkInformation *outInfo = outputVector->GetInformationObject(0);

        //vtkInformation *inInfoFace = inputVector[1]->GetInformationObject(0);

        
        vtkStructuredGrid *input = vtkStructuredGrid::SafeDownCast(
                inInfo->Get(vtkDataObject::DATA_OBJECT()));

        //vtkStructuredGrid *inputface = vtkStructuredGrid::SafeDownCast(
        //      inInfoFace->Get(vtkDataObject::DATA_OBJECT()));

        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));
        
        int numNodes = input->GetNumberOfPoints();
        int numCells = input->GetNumberOfCells();

        if(numNodes <= 0 || numCells <= 0){vtkErrorMacro("Invalid input");
        return 0;}
        if(FaceNum == -1){      vtkErrorMacro("Face Number not set");
        return 0;}
        vtkPoints *points = vtkPoints::New();
        points->DeepCopy(input->GetPoints());

        output->SetPoints(points);
        output->SetDimensions(input->GetDimensions());
        output->GetPointData()->PassData(input->GetPointData());
        this->SetFaceCase(FaceNum, output, input, faceGrid);

        points->Delete();

  return 1;
}

void vtkMimxSetStructuredGridFace::SetFaceCase(int FaceNum, vtkStructuredGrid *Output, 
                                                                                   vtkStructuredGrid *Input, vtkStructuredGrid *InputFace)
{
        switch (FaceNum) {
                case 0:
                        SetFace0(Output, Input, InputFace);
                        break;
                case 1:
                        SetFace1(Output, Input, InputFace);
                        break;
                case 2:
                        SetFace2(Output, Input, InputFace);
                        break;
                case 3:
                        SetFace3(Output, Input, InputFace);
                        break;
                case 4:
                        SetFace4(Output, Input, InputFace);
                        break;
                case 5:
                        SetFace5(Output, Input, InputFace);
                        break;
        }
}

void vtkMimxSetStructuredGridFace::SetFace0(vtkStructuredGrid *Output, vtkStructuredGrid* Input, vtkStructuredGrid* InputFace)
{
        int dim[3];
        
        Input->GetDimensions(dim);
        int x,y;
        for(y=0; y < dim[2]; y++)
        {
                for(x=0; x < dim[1]; x++)
                {               
                                Output->GetPoints()->SetPoint(y*dim[0]*dim[1]+x*dim[0],
                                InputFace->GetPoints()->GetPoint(y*dim[1] + x));
                                        //cout<<y*dim[0]*dim[1]+x*dim[0]<<"  "<<y*dim[1] + x<<endl;
                }
        }
        if (InputFace->GetPointData()->GetScalars())
        {
                for(y=0; y < dim[2]; y++)
                {
                        for(x=0; x < dim[1]; x++)
                        {               
                                Output->GetPointData()->GetScalars()->SetTuple(y*dim[0]*dim[1]+x*dim[0],
                                        InputFace->GetPointData()->GetScalars()->GetTuple(y*dim[1]));
                        }
                }

        }
}

void vtkMimxSetStructuredGridFace::SetFace1(vtkStructuredGrid *Output, vtkStructuredGrid* Input, vtkStructuredGrid* InputFace)
{
        int dim[3];
        int x,y;
        
        Input->GetDimensions(dim);
        for(y=0; y < dim[2]; y++)
        {
                for(x=0; x < dim[1]; x++)
                {
                        Output->GetPoints()->SetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1,
                        InputFace->GetPoints()->GetPoint(y*dim[1] + x));
                        ////cout <<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<"  "<<y*dim[1] + x<<endl;
                }
        }

        if (InputFace->GetPointData()->GetScalars())
        {
                for(y=0; y < dim[2]; y++)
                {
                        for(x=0; x < dim[1]; x++)
                        {               
                                Output->GetPointData()->GetScalars()->SetTuple(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1,
                                        InputFace->GetPointData()->GetScalars()->GetTuple(y*dim[1]));
                        }
                }
        }

}

void vtkMimxSetStructuredGridFace::SetFace2(vtkStructuredGrid *Output, vtkStructuredGrid* Input, vtkStructuredGrid* InputFace)
{
        int dim[3];
        int y,z;        
        Input->GetDimensions(dim);
        for(y=0; y < dim[2]; y++)
        {
                for(z=0; z < dim[0]; z++)
                {
                        
                                Output->GetPoints()->SetPoint(
                                y*dim[0]*dim[1]+z,InputFace->GetPoints()->GetPoint(y*dim[0] + z));
                }
        }

        if (InputFace->GetPointData()->GetScalars())
        {
                for(y=0; y < dim[2]; y++)
                {
                        for(z=0; z < dim[0]; z++)
                        {               
                                Output->GetPointData()->GetScalars()->SetTuple(y*dim[0]*dim[1]+z,
                                        InputFace->GetPointData()->GetScalars()->GetTuple(y*dim[0] + z));
                        }
                }
        }

}

void vtkMimxSetStructuredGridFace::SetFace3(vtkStructuredGrid *Output, vtkStructuredGrid* Input, vtkStructuredGrid* InputFace)
{
        int dim[3];
        int y,z;
        Input->GetDimensions(dim);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {       
                                Output->GetPoints()->SetPoint(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z,
                                InputFace->GetPoints()->GetPoint(y*dim[0] + z));
                }
        }

        if (InputFace->GetPointData()->GetScalars())
        {
                for(y=0; y < dim[2]; y++)
                {
                        for(z=0; z < dim[0]; z++)
                        {               
                                Output->GetPointData()->GetScalars()->SetTuple(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z,
                                        InputFace->GetPointData()->GetScalars()->GetTuple(y*dim[0] + z));
                        }
                }
        }
}

void vtkMimxSetStructuredGridFace::SetFace4(vtkStructuredGrid *Output, vtkStructuredGrid* Input, vtkStructuredGrid* InputFace)
{
        int dim[3];
        int x,z;
        Input->GetDimensions(dim);

        for(x=0; x < dim[1]; x++)
        {
                for(z=0; z < dim[0]; z++)
                {
                        
                                Output->GetPoints()->SetPoint(x*dim[0]+z,
                                InputFace->GetPoints()->GetPoint(x*dim[0] + z));
                }
        }
        if (InputFace->GetPointData()->GetScalars())
        {
                for(x=0; x < dim[1]; x++)
                {
                        for(z=0; z < dim[0]; z++)
                        {               
                                Output->GetPointData()->GetScalars()->SetTuple(x*dim[0]+z,
                                        InputFace->GetPointData()->GetScalars()->GetTuple(x*dim[0] + z));
                        }
                }
        }
}

void vtkMimxSetStructuredGridFace::SetFace5(vtkStructuredGrid *Output, vtkStructuredGrid* Input, vtkStructuredGrid* InputFace)
{
        int dim[3];
        int x,z;
        Input->GetDimensions(dim);

        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {                       
                        Output->GetPoints()->SetPoint((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z,
                        InputFace->GetPoints()->GetPoint(x*dim[0] + z));
                }
        }
        if (InputFace->GetPointData()->GetScalars())
        {
                for(x=0; x < dim[1]; x++)
                {
                        for(z=0; z < dim[0]; z++)
                        {               
                                Output->GetPointData()->GetScalars()->SetTuple((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z,
                                        InputFace->GetPointData()->GetScalars()->GetTuple(x*dim[0] + z));
                        }
                }
        }
}

void vtkMimxSetStructuredGridFace::SetFace(vtkStructuredGrid *Sgrid)
{
        faceGrid = Sgrid;
        //this->SetInput(1, Sgrid);
}

int vtkMimxSetStructuredGridFace::FillInputPortInformation(int port,    vtkInformation *info)
{
        if(port == 0)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkStructuredGrid");
                return 1;
        }
        else{ vtkErrorMacro("Invalid input port number");}
        return 0;
}

void vtkMimxSetStructuredGridFace::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
