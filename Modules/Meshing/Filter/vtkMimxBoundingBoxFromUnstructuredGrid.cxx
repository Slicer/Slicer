/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundingBoxFromUnstructuredGrid.cxx,v $
Language:  C++
Date:      $Date: 2007/11/12 21:05:35 $
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

#include "vtkMimxBoundingBoxFromUnstructuredGrid.h"

#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkExecutive.h"
#include "vtkGenericCell.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkIdList.h"
//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMimxBoundingBoxFromUnstructuredGrid, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkMimxBoundingBoxFromUnstructuredGrid);
//------------------------------------------------------------------------------
// Construct object to Set all of the input data.
vtkMimxBoundingBoxFromUnstructuredGrid::vtkMimxBoundingBoxFromUnstructuredGrid()
{
        this->SetNumberOfInputPorts(2);
}
//------------------------------------------------------------------------------
vtkMimxBoundingBoxFromUnstructuredGrid::~vtkMimxBoundingBoxFromUnstructuredGrid()
{
        
}
//------------------------------------------------------------------------------
int vtkMimxBoundingBoxFromUnstructuredGrid::RequestData(
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

        vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));
        
        int numNodes = input->GetNumberOfPoints();
        int numCells = input->GetNumberOfCells();

        if(numNodes <= 0 || numCells <= 0){vtkErrorMacro("Invalid input");
        return 0;}
        

        if(!boundingbox->GetCellData()->GetArray("Mesh_Seed"))
        {
                vtkErrorMacro("Vectors containing mesh seed information need to be in the bounding box");
                return 0;
        }

        output->Allocate(boundingbox->GetNumberOfCells(),boundingbox->GetNumberOfCells());
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(0);
        for (int i=0; i < boundingbox->GetNumberOfPoints(); i++)
        {
                points->InsertNextPoint(boundingbox->GetPoint(i));
        }
        output->SetPoints(points);
        points->Delete();

        int i;
        for(i=0; i <boundingbox->GetNumberOfCells(); i++)
        {
                vtkIdList *locallist = vtkIdList::New();
                locallist->DeepCopy(boundingbox->GetCell(i)->GetPointIds());
                output->InsertNextCell(boundingbox->GetCellType(i), locallist);
                locallist->Delete();
        }
        // extract points from the unstructured grid and set them in the output
        int j;
        for (i=0; i<boundingbox->GetNumberOfCells(); i++)
        {
                vtkIdList *ptids = boundingbox->GetCell(i)->GetPointIds();
                int numpts = boundingbox->GetCell(i)->GetPointIds()->GetNumberOfIds();
                for (j=0; j<numpts; j++)
                {
                        double x[3];
                        this->GetCellPoint(i,j,boundingbox, input, x);
                        output->GetPoints()->SetPoint(ptids->GetId(j), x);
                }
        }
        output->Squeeze();
        return 1;
}
//------------------------------------------------------------------------------
void vtkMimxBoundingBoxFromUnstructuredGrid::SetBoundingBox(vtkUnstructuredGrid *UGrid)
{
        this->SetInput(1, UGrid);
}
//------------------------------------------------------------------------------
void vtkMimxBoundingBoxFromUnstructuredGrid::GetCellPoint(vtkIdType CellNum, vtkIdType PointNum, 
                                vtkUnstructuredGrid *BBox, vtkUnstructuredGrid *FEMesh, double x[3])
{
        int startelenum = 0;
        vtkIntArray *meshseedarray = vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"));
        int i, dim[3], dimmod[3];
        for (i=0; i<CellNum; i++)
        {
                meshseedarray->GetTupleValue(i, dim);
                startelenum += (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
        }

        meshseedarray->GetTupleValue(CellNum, dim);
        dimmod[0] = dim[2]-1;   dimmod[1] = dim[0]-1;   dimmod[2] = dim[1]-1;
        vtkIdList *ptids;

        switch (PointNum) {
                case 0:
                        ptids = FEMesh->GetCell(startelenum)->GetPointIds();
                        FEMesh->GetPoint(ptids->GetId(0), x);
                        break;
                case 1:
                        ptids = FEMesh->GetCell(startelenum + dimmod[0]-1)->GetPointIds();
                        FEMesh->GetPoint(ptids->GetId(1), x);
                        break;
                case 2:
                        ptids = FEMesh->GetCell(startelenum + dimmod[0]*dimmod[1]-1)->GetPointIds();
                        FEMesh->GetPoint(ptids->GetId(2), x);
                        break;
                case 3:
                        ptids = FEMesh->GetCell(startelenum + dimmod[0]*(dimmod[1]-1))->GetPointIds();
                        FEMesh->GetPoint(ptids->GetId(3), x);
                        break;
                case 4:
                        ptids = FEMesh->GetCell(startelenum + dimmod[0]*dimmod[1]*(dimmod[2]-1))->GetPointIds();
                        FEMesh->GetPoint(ptids->GetId(4), x);
                        break;
                case 5:
                        ptids = FEMesh->GetCell(startelenum + dimmod[0]*dimmod[1]*(dimmod[2]-1)
                                + dimmod[0]-1)->GetPointIds();
                        FEMesh->GetPoint(ptids->GetId(5), x);
                        break;
                case 6:
                        ptids = FEMesh->GetCell(startelenum + dimmod[0]*dimmod[1]*(dimmod[2]-1)
                                + dimmod[0]*dimmod[1]-1)->GetPointIds();
                        FEMesh->GetPoint(ptids->GetId(6), x);
                        break;
                case 7:
                        ptids = FEMesh->GetCell(startelenum + dimmod[0]*dimmod[1]*(dimmod[2]-1)
                                + dimmod[0]*(dimmod[1]-1))->GetPointIds();
                        FEMesh->GetPoint(ptids->GetId(7), x);
                        break;
        }
}
//------------------------------------------------------------------------------
void vtkMimxBoundingBoxFromUnstructuredGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
