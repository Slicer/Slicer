/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSetStructuredGridEdge.cxx,v $
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

#include "vtkMimxSetStructuredGridEdge.h"

#include "vtkCellArray.h"
#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxSetStructuredGridEdge, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxSetStructuredGridEdge);

// Construct object to Set all of the input data.
vtkMimxSetStructuredGridEdge::vtkMimxSetStructuredGridEdge()
{
        this->SetNumberOfInputPorts(2);
        this->EdgeNum = -1;
}

vtkMimxSetStructuredGridEdge::~vtkMimxSetStructuredGridEdge()
{
        
}

int vtkMimxSetStructuredGridEdge::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
        vtkInformation *edgeInfo = inputVector[1]->GetInformationObject(0);

        vtkInformation *outInfo = outputVector->GetInformationObject(0);
        
        vtkStructuredGrid *input = vtkStructuredGrid::SafeDownCast(
                inInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkPolyData *edge = vtkPolyData::SafeDownCast(
                edgeInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));
        
        int numNodes = input->GetNumberOfPoints();
        int numCells = input->GetNumberOfCells();

        if(numNodes <= 0 || numCells <= 0){vtkErrorMacro("Invalid input");
        return 0;}
        if(EdgeNum == -1){      vtkErrorMacro("Edge Number not set");
        return 0;}

        int dim[3];
        input->GetDimensions(dim);

        if(EdgeNum == 0 || EdgeNum == 2 || EdgeNum == 4 || EdgeNum == 6){
                if(edge->GetNumberOfPoints() != dim[0])
                {
                        vtkErrorMacro("Number of Divisions of input curve does not match the structured grid dimensions");
                        return 0;
                }
        } else if(EdgeNum == 1 || EdgeNum == 3 || EdgeNum == 5 || EdgeNum == 7) {       if(edge->GetNumberOfPoints() != dim[1])
        {
                vtkErrorMacro("Number of Divisions of input curve does not match the structured grid dimensions");
                return 0;}
        }
        else{           if(edge->GetNumberOfPoints() != dim[2])
        {
                vtkErrorMacro("Number of Divisions of input curve does not match the structured grid dimensions");
                return 0;
        }
        }

        vtkPoints *points = vtkPoints::New();
        points->DeepCopy(input->GetPoints());
        output->SetDimensions(input->GetDimensions());
        output->SetPoints(points);
        output->GetPointData()->PassData(input->GetPointData());
        this->SetEdge(EdgeNum, edge->GetPoints(), output);
        
        points->Delete();

  return 1;
}

void vtkMimxSetStructuredGridEdge::SetEdge(int EdgeNum, vtkPoints* PointList, vtkStructuredGrid *Input)
{
        switch (EdgeNum) {
                case 0:
                        SetEdge0(PointList, Input);
                        break;
                case 1:
                        SetEdge1(PointList, Input);
                        break;
                case 2:
                        SetEdge2(PointList, Input);
                        break;
                case 3:
                        SetEdge3(PointList, Input);
                        break;
                case 4:
                        SetEdge4(PointList, Input);
                        break;
                case 5:
                        SetEdge5(PointList, Input);
                        break;
                case 6:
                        SetEdge6(PointList, Input);
                        break;
                case 7:
                        SetEdge7(PointList, Input);
                        break;
                case 8:
                        SetEdge8(PointList, Input);
                        break;
                case 9:
                        SetEdge9(PointList, Input);
                        break;
                case 10:
                        SetEdge10(PointList, Input);
                        break;
                case 11:
                        SetEdge11(PointList, Input);
                        break;
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge0(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(i, PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge1(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*(i+1)-1, PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge2(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*(dim[1]-1)+i, PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge3(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(i*dim[0], PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge4(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*dim[1]*(dim[2]-1) + i, PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge5(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(i+1)-1, PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge6(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(dim[1]-1)+i, PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge7(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*dim[1]*(dim[2]-1) + i*dim[0], PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge8(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*dim[1]*i, PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge9(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*dim[1]*i + dim[0]-1, PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge10(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*dim[1]*i + dim[0]*(dim[1]-1), PointList->GetPoint(i));
        }
}

void vtkMimxSetStructuredGridEdge::SetEdge11(vtkPoints* PointList, vtkStructuredGrid* Output)
{
        int dim[3];
        
                Output->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(dim[0]*dim[1]*i + dim[0]*dim[1]-1, PointList->GetPoint(i));
        }
}

int vtkMimxSetStructuredGridEdge::FillInputPortInformation(int port,    vtkInformation *info)
{
        if(port == 0)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkStructuredGrid");
                return 1;
        }
        else if(port == 1)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
                return 1;
        }
        else{ vtkErrorMacro("Invalid input port number");}
        return 0;
}

void vtkMimxSetStructuredGridEdge::SetEdge(vtkPolyData *edge)
{
        this->SetInput(1, edge);
}

void vtkMimxSetStructuredGridEdge::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
