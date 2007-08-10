/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredToStructuredGrid.cxx,v $
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

#include "vtkMimxUnstructuredToStructuredGrid.h"

#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxUnstructuredToStructuredGrid, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkMimxUnstructuredToStructuredGrid);

// Construct object to Set all of the input data.
vtkMimxUnstructuredToStructuredGrid::vtkMimxUnstructuredToStructuredGrid()
{
        this->SetNumberOfInputPorts(2);
        this->StructuredGridNum = -1;
}

vtkMimxUnstructuredToStructuredGrid::~vtkMimxUnstructuredToStructuredGrid()
{
        
}

int vtkMimxUnstructuredToStructuredGrid::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
        vtkInformation *edgeInfo = inputVector[1]->GetInformationObject(0);

        vtkInformation *outInfo = outputVector->GetInformationObject(0);
        
        vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
                inInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkUnstructuredGrid *boundingbox = vtkUnstructuredGrid::SafeDownCast(
                edgeInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));
        
        int numNodes = input->GetNumberOfPoints();
        int numCells = input->GetNumberOfCells();

        if(numNodes <= 0 || numCells <= 0){vtkErrorMacro("Invalid input");
        return 0;}
        if(StructuredGridNum == -1){        vtkErrorMacro("StructuredGrid Number not set");
        return 0;}

        if(!boundingbox->GetCellData()->GetArray("Mesh_Seed"))
        {
                vtkErrorMacro("Vectors containing mesh seed information need to be in the bounding box input");
                return 0;
        }

        if(this->StructuredGridNum >= boundingbox->GetNumberOfCells())
        {
                vtkErrorMacro("Structured grid number specified cannot be greater than number of cells in the bounding box");
                return 0;
        }

        int i, j, k, dim[3], dimtemp[3], startelenum = 0;
        
        for(i=0; i < this->StructuredGridNum; i++)
        {
                vtkIntArray::SafeDownCast(boundingbox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(i, dim);
                startelenum += (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
        }

        vtkIntArray::SafeDownCast(boundingbox->GetCellData()->GetArray("Mesh_Seed"))->
                GetTupleValue(this->StructuredGridNum, dimtemp);

//        IdList->SetNumberOfIds(dim[0]*dim[1]*dim[2]);

        // start adding points and point numbers to the list
        vtkIdType npts, *pts;
        int elenum = startelenum;

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        vtkPoints *Points = vtkPoints::New();

        Points->SetNumberOfPoints(dim[0]*dim[1]*dim[2]);

        for (k=0; k<dim[2]-1; k++)
        {
                for (j=0; j<dim[1]-1; j++)
                {
                        for (i=0; i<dim[0]-1; i++)
                        {
                                input->GetCellPoints(elenum, npts, pts);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i, input->GetPoint(pts[0]));
//                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i, pts[0]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i+1, input->GetPoint(pts[1]));
//                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i+1, pts[1]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1, input->GetPoint(pts[2]));
//                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1, pts[2]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0], input->GetPoint(pts[3]));
//                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0], pts[3]);

                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]*dim[1], input->GetPoint(pts[4]));
//                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]*dim[1], pts[4]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i+1 + dim[0]*dim[1], input->GetPoint(pts[5]));
//                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i+1 + dim[0]*dim[1], pts[5]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1 + dim[0]*dim[1], input->GetPoint(pts[6]));
//                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1 + dim[0]*dim[1], pts[6]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0] + dim[0]*dim[1], input->GetPoint(pts[7]));
//                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0] + dim[0]*dim[1], pts[7]);
                                elenum++;
                        }
                }
        }
        output->SetPoints(Points);
        output->SetDimensions(dim);
        Points->Delete();
        return 1;
}

int vtkMimxUnstructuredToStructuredGrid::FillInputPortInformation(int port,        vtkInformation *info)
{
        if(port == 0 || port == 1)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
                return 1;
        }
        else{ vtkErrorMacro("Invalid input port number");}
        return 0;
}

void vtkMimxUnstructuredToStructuredGrid::SetBoundingBox(vtkUnstructuredGrid *UGrid)
{
        this->SetInput(1, UGrid);
}

void vtkMimxUnstructuredToStructuredGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
