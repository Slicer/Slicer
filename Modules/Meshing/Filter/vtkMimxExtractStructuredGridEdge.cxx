/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractStructuredGridEdge.cxx,v $
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

#include "vtkMimxExtractStructuredGridEdge.h"

#include "vtkCellArray.h"
#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxExtractStructuredGridEdge, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxExtractStructuredGridEdge);

// Construct object to extract all of the input data.
vtkMimxExtractStructuredGridEdge::vtkMimxExtractStructuredGridEdge()
{
        this->EdgeNum = -1;
}

vtkMimxExtractStructuredGridEdge::~vtkMimxExtractStructuredGridEdge()
{
        
}

int vtkMimxExtractStructuredGridEdge::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
        vtkInformation *outInfo = outputVector->GetInformationObject(0);
        
        vtkStructuredGrid *input = vtkStructuredGrid::SafeDownCast(
                inInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkPolyData *output = vtkPolyData::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));
        
        int numNodes = input->GetNumberOfPoints();
        int numCells = input->GetNumberOfCells();

        if(numNodes <= 0 || numCells <= 0){vtkErrorMacro("Invalid input");
        return 0;}
        if(EdgeNum == -1){        vtkErrorMacro("Edge Number not set");
        return 0;}
        vtkPoints *points = vtkPoints::New();
        this->GetEdge(EdgeNum, points, input);
        vtkCellArray *cellarray = vtkCellArray::New();
        
        for(int i=0; i < points->GetNumberOfPoints()-1; i++)
        {
                cellarray->InsertNextCell(2);
                cellarray->InsertCellPoint(i);
                cellarray->InsertCellPoint(i+1);
        }
        output->SetLines(cellarray);
        output->SetPoints(points);
        points->Delete();
        cellarray->Delete();

  return 1;
}

void vtkMimxExtractStructuredGridEdge::GetEdge(int EdgeNum, vtkPoints* PointList, vtkStructuredGrid *Input)
{
        switch (EdgeNum) {
                case 0:
                        GetEdge0(PointList, Input);
                        break;
                case 1:
                        GetEdge1(PointList, Input);
                        break;
                case 2:
                        GetEdge2(PointList, Input);
                        break;
                case 3:
                        GetEdge3(PointList, Input);
                        break;
                case 4:
                        GetEdge4(PointList, Input);
                        break;
                case 5:
                        GetEdge5(PointList, Input);
                        break;
                case 6:
                        GetEdge6(PointList, Input);
                        break;
                case 7:
                        GetEdge7(PointList, Input);
                        break;
                case 8:
                        GetEdge8(PointList, Input);
                        break;
                case 9:
                        GetEdge9(PointList, Input);
                        break;
                case 10:
                        GetEdge10(PointList, Input);
                        break;
                case 11:
                        GetEdge11(PointList, Input);
                        break;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge0(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
        Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i < dim[0]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(i));
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge1(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*(i+1)-1));
//                //cout <<dim[0]*(i+1)-1<<endl;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge2(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i< dim[0]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*(dim[1]-1)+i));
//                //cout <<dim[0]*(dim[1]-1)+i<<endl;

        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge3(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(i*dim[0]));
//                //cout <<i*dim[0]<<endl;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge4(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i < dim[0]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*dim[1]*(dim[2]-1) + i));
//                //cout<<dim[0]*dim[1]*(dim[2]-1) + i<<endl;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge5(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(i+1)-1));
//                //cout <<dim[0]*dim[1]*(dim[2]-1) + dim[0]*(i+1)-1<<endl;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge6(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i< dim[0]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(dim[1]-1)+i));
//                //cout<<dim[0]*dim[1]*(dim[2]-1) + dim[0]*(dim[1]-1)+i<<endl;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge7(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*dim[1]*(dim[2]-1) + i*dim[0]));
//                //cout<<dim[0]*dim[1]*(dim[2]-1) + i*dim[0]<<endl;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge8(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i < dim[2]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*dim[1]*i));
//                //cout<<dim[0]*dim[1]*i<<endl;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge9(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i< dim[2]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*dim[1]*i + dim[0]-1));
//                //cout<<dim[0]*dim[1]*i + dim[0]-1<<endl;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge10(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
                Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i< dim[2]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*dim[1]*i + dim[0]*(dim[1]-1)));
//                //cout<<dim[0]*dim[1]*i + dim[0]*(dim[1]-1)<<endl;
        }
}

void vtkMimxExtractStructuredGridEdge::GetEdge11(vtkPoints* PointList, vtkStructuredGrid *Input)
{
        int dim[3];
        
        Input->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i< dim[2]; i++)
        {
                PointList->InsertPoint(i, Input->GetPoint(dim[0]*dim[1]*i + dim[0]*dim[1]-1));
//                //cout<<dim[0]*dim[1]*i + dim[0]*dim[1]-1<<endl;
        }
}

int vtkMimxExtractStructuredGridEdge::FillInputPortInformation(int port,        vtkInformation *info)
{
        if(port == 0)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkStructuredGrid");
                return 1;
        }
        else{ vtkErrorMacro("Invalid input port number");}
        return 0;
}

void vtkMimxExtractStructuredGridEdge::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
