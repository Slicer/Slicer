/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractElementNumbersOfaFaceFilter.cxx,v $
Language:  C++
Date:      $Date: 2007/11/09 21:09:25 $
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

#include "vtkMimxExtractElementNumbersOfaFaceFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkGeometryFilter.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxConstrainedSmoothPolyDataFilter.h"

vtkCxxRevisionMacro(vtkMimxExtractElementNumbersOfaFaceFilter, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxExtractElementNumbersOfaFaceFilter);


vtkMimxExtractElementNumbersOfaFaceFilter::vtkMimxExtractElementNumbersOfaFaceFilter()
{
        this->SetNumberOfInputPorts(2);
        this->CellIdList = vtkIntArray::New();
        this->VertexList = NULL;
}

vtkMimxExtractElementNumbersOfaFaceFilter::~vtkMimxExtractElementNumbersOfaFaceFilter()
{
        this->CellIdList->Delete();
        this->ElementSetName = NULL;
}


int vtkMimxExtractElementNumbersOfaFaceFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
 
  // get the info objects
  vtkInformation *BboxInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *FeMeshInfo = inputVector[1]->GetInformationObject(0);
  vtkUnstructuredGrid *bbox = vtkUnstructuredGrid::SafeDownCast(
           BboxInfo->Get(vtkDataObject::DATA_OBJECT()));
  // get the input and output
  vtkUnstructuredGrid *FeMesh = vtkUnstructuredGrid::SafeDownCast(
         FeMeshInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType numPts,numCells;
  // Check input
  //
  numPts=bbox->GetNumberOfPoints();
  numCells=bbox->GetNumberOfCells();
  if (numPts < 1 || numCells < 1)
  {
          vtkErrorMacro(<<"No data to extract surfaces!");
          return 1;
  }

  if(!this->ElementSetName)
  {
          vtkErrorMacro(<<"Element-Set Name Should be Set");
          return 0;
  }

        int cellno, faceno;

        if (this->VertexList->GetNumberOfComponents()!= 4)
        {
                vtkErrorMacro(<< "Only quad surfaces ");
                return 0;
        }

        // check if mesh seed data exists
        if(!bbox->GetCellData()->GetArray("Mesh_Seed"))
        {
                vtkErrorMacro("Mesh seed data needed");
                return 0;
        }

        // check if element numbers have already been assigned
        if(!FeMesh->GetCellData()->GetArray("Element_Numbers"))
        {
                vtkErrorMacro("Set the Element numbers for the input mesh");
                return 0;
        }
        // create new element set for the output
        vtkIntArray *elementnumbers = vtkIntArray::SafeDownCast(
                FeMesh->GetCellData()->GetArray("Element_Numbers"));

        vtkIntArray *MeshSeedValues = vtkIntArray::SafeDownCast(
                bbox->GetCellData()->GetArray("Mesh_Seed"));
        // check the femesh and building block correspond to each other
        int i;
        int meshseed[3];
        int numele = 0;
        for (i=0; i<MeshSeedValues->GetNumberOfTuples(); i++)
        {
                MeshSeedValues->GetTupleValue(i, meshseed);
                numele += (meshseed[0]-1)*(meshseed[1]-1)*(meshseed[2]-1);
        }

        if(numele != FeMesh->GetNumberOfCells())
        {
                vtkErrorMacro("Check if the FE Mesh and BB structure correspond to each other");
                return 0;
        }
        CellIdList->SetNumberOfValues(FeMesh->GetNumberOfCells());
        CellIdList->SetName(this->ElementSetName);

        for (i=0; i<FeMesh->GetNumberOfCells(); i++)
        {
                CellIdList->SetValue(i,0);
        }

        for (int k = 0; k < this->VertexList->GetNumberOfTuples(); k++)
        {
                int facepointids[4];
                this->VertexList->GetTupleValue(k, facepointids);
                bool status = false; 
                for (i = 0; i< bbox->GetNumberOfCells(); i++)
                {
                        int numberoffaces = bbox->GetCell(i)->GetNumberOfFaces();
                        for (int j = 0; j< numberoffaces; j++)
                        {       
                                vtkIdList *facelist = bbox->GetCell(i)->GetFace(j)->GetPointIds();
                                if (facelist->IsId(facepointids[0])!= -1 && facelist->IsId(facepointids[1])!= -1 &&
                                        facelist->IsId(facepointids[2])!= -1 && facelist->IsId(facepointids[3])!= -1)
                                {
                                        cellno = i;
                                        faceno = j;
                                        int startelementnumber = 0;
                                        int dim[3];
                                        for (int m = 0; m<cellno ; m++)
                                        {
                                                MeshSeedValues->GetTupleValue(m, dim);
                                                startelementnumber = startelementnumber + ((dim[0]-1)*(dim[1]-1)*(dim[2]-1));
                                        }
                                        MeshSeedValues->GetTupleValue(cellno, dim);
                                        int dimmod[3];
                                        dim[0] = dim[0] - 1;    dim[1] = dim[1] - 1;    dim[2] = dim[2] - 1;
                                        dimmod[0] = dim[2];     dimmod[1] = dim[0];     dimmod[2] = dim[1];
                                        this->GetFace(faceno, dimmod, startelementnumber);                      
                                }
                        }
                }
        }

        vtkPoints *points = vtkPoints::New();
        points->DeepCopy(FeMesh->GetPoints());
        output->Allocate(FeMesh->GetNumberOfCells(), FeMesh->GetNumberOfCells());

        for(i=0; i <FeMesh->GetNumberOfCells(); i++)
        {
                vtkIdList *locallist = vtkIdList::New();
                locallist->DeepCopy(FeMesh->GetCell(i)->GetPointIds());
                output->InsertNextCell(FeMesh->GetCellType(i), locallist);
                locallist->Delete();
        }
        output->SetPoints(points);
        points->Delete();
        output->GetCellData()->AddArray(this->CellIdList);
        return 1;
}




int vtkMimxExtractElementNumbersOfaFaceFilter::FillInputPortInformation(int port, vtkInformation *info)
{
        if(port == 0 ||port == 1)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
                return 1;
        }
        
        return 0;
}

void vtkMimxExtractElementNumbersOfaFaceFilter::SetBoundingBox(vtkUnstructuredGrid *source)
{
        this->SetInput(0, source);
}

void vtkMimxExtractElementNumbersOfaFaceFilter::SetFeMesh(vtkUnstructuredGrid *source)
{
        this->SetInput(1, source);
}


void vtkMimxExtractElementNumbersOfaFaceFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//////////////// faces data//////////////

void vtkMimxExtractElementNumbersOfaFaceFilter::GetFace0( 
        int dim[3], int startelementnumber )
{
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        this->CellIdList->SetValue(
                                y*dim[0]*dim[1]+x*dim[0] + startelementnumber, 1); 
                }
        }
}

void vtkMimxExtractElementNumbersOfaFaceFilter::GetFace1(int dim[3], int startelementnumber)
{
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        this->CellIdList->SetValue(
                                (y*dim[0]*dim[1]+x*dim[0]+dim[0]-1)+ startelementnumber, 1); 

                }
        }
}

void vtkMimxExtractElementNumbersOfaFaceFilter::GetFace2(int dim[3], int startelementnumber)
{
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {       
                        this->CellIdList->SetValue(
                                y*dim[0]*dim[1]+z + startelementnumber, 1); 
                }
        }
}

void vtkMimxExtractElementNumbersOfaFaceFilter::GetFace3( int dim[3], int startelementnumber)
{
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        this->CellIdList->SetValue(
                                y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z+ startelementnumber, 1); 
                }
        }
}

void vtkMimxExtractElementNumbersOfaFaceFilter::GetFace4(int dim[3], int startelementnumber)
{
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        this->CellIdList->SetValue(
                                x*dim[0]+z + startelementnumber, 1); 
                }
        }
}

void vtkMimxExtractElementNumbersOfaFaceFilter::GetFace5(int dim[3], int startelementnumber)
{
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        this->CellIdList->SetValue(
                                (dim[2]-1)*dim[1]*dim[0]+x*dim[0]+ z + startelementnumber, 1); 
                }
        }
}
//----------------------------------------------------------------------------------------------
void vtkMimxExtractElementNumbersOfaFaceFilter::GetFace(int FaceNum, int dim[3], int StartEleNum)
{
        switch (FaceNum) {
                case 0:
                        GetFace0(dim, StartEleNum);
                        break;
                case 1:
                        GetFace1(dim, StartEleNum);
                        break;
                case 2:
                        GetFace2(dim, StartEleNum);
                        break;
                case 3:
                        GetFace3(dim, StartEleNum);
                        break;
                case 4:
                        GetFace4(dim, StartEleNum);
                        break;
                case 5:
                        GetFace5(dim, StartEleNum);
                        break;
        }
}
//-------------------------------------------------------------------------------------
