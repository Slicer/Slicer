/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxRecalculateInteriorNodes.cxx,v $
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

#include "vtkMimxRecalculateInteriorNodes.h"

#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkExecutive.h"
#include "vtkGenericCell.h"
#include "vtkGeometryFilter.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkMimxPlanarEllipticalInterpolation.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkMimxSolidEllipticalInterpolation.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

#include "vtkStructuredGridWriter.h"
#include "vtkUnstructuredGridWriter.h"

vtkCxxRevisionMacro(vtkMimxRecalculateInteriorNodes, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkMimxRecalculateInteriorNodes);

// Construct object to Set all of the input data.
vtkMimxRecalculateInteriorNodes::vtkMimxRecalculateInteriorNodes()
{
        this->SetNumberOfInputPorts(2);
        this->NumberOfIterations = 10;
}

vtkMimxRecalculateInteriorNodes::~vtkMimxRecalculateInteriorNodes()
{
        
}

int vtkMimxRecalculateInteriorNodes::RequestData(
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
                vtkErrorMacro("Vectors containing mesh seed information need to be in the bounding box input");
                return 0;
        }

        output->DeepCopy(input);

        //  first extract surface nodes and their connectivity
        vtkGeometryFilter *geofil = vtkGeometryFilter::New();
        geofil->SetInput(boundingbox);
        geofil->Update();
        vtkIdList *idlist ;//= vtkIdList::New();

        vtkGenericCell *gencell = vtkGenericCell::New();

        for(int i=0; i < boundingbox->GetNumberOfCells(); i++)
        {
                cout << "Cell "<<i<<endl;
                int dim[3], dimtemp[3];
                vtkIntArray::SafeDownCast(boundingbox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(i, dimtemp);

                vtkIdList *idlistedge = vtkIdList::New();
                idlistedge->SetNumberOfIds(12);

                dim[0] = dimtemp[2];
                dim[1] = dimtemp[0];
                dim[2] = dimtemp[1];
                int k;
                for(k = 0; k<12; k++)        idlistedge->SetId(k,0);

                // access all the points contained in a structured grid corresponding
                // to the cell.
                vtkPoints *pointscell = vtkPoints::New();
                vtkIdList *pointidlist = vtkIdList::New();
                this->GetCellPoints(i, boundingbox, input, pointscell,pointidlist);

                gencell->Initialize();
                boundingbox->GetCell(i,gencell);

                for(k=0; k<12; k++)
                {
                        idlist = gencell->GetEdge(k)->GetPointIds();
                        if(this->CheckBoundaryEdge(idlist,geofil->GetOutput()))
                        {
                                idlistedge->SetId(k,1);
                        }
                }

                // recalculate the nodal positions of the interior edges
                for(k=0; k<12; k++)
                {
                        if(!idlistedge->GetId(k))
                        {
                                if(this->CheckIfEdgeNodesRecalculated(k, idlistedge))
                                {
                                        // recalculate the interior nodes of a given edge
                                        this->RecalculateEdge(i, k, idlistedge, boundingbox, pointscell);
                                }
                        }
                }

                gencell->Initialize();
                boundingbox->GetCell(i,gencell);
                //         if a face is an interior face computer interior nodes of the face using planar transfinite interpolation
                for(int j=0; j <6; j++)
                {
                        idlist = gencell->GetFace(j)->GetPointIds();
                        if(!this->CheckBoundaryFace(idlist, geofil->GetOutput()))
                        {
                                vtkPoints* points = vtkPoints::New();
                                this->GetFaceUGrid(i,j,boundingbox, pointscell, points);
                                vtkMimxPlanarEllipticalInterpolation* interpolation = 
                                        vtkMimxPlanarEllipticalInterpolation::New();
                                interpolation->SetNumberOfIterations(this->NumberOfIterations);
                                vtkStructuredGrid *planegrid = vtkStructuredGrid::New();
                                planegrid->SetPoints(points);
                                if(j==2 || j==3)
                                {
                                        planegrid->SetDimensions(dim[0],dim[2],1);
                                }
                                if(j==0 || j==1)
                                {
                                        planegrid->SetDimensions(dim[1],dim[2],1);
                                }
                                if(j==4 || j==5)
                                {
                                        planegrid->SetDimensions(dim[0],dim[1],1);
                                }
                                interpolation->SetInput(planegrid);

                                // to compute the interior nodes of a face
                                interpolation->Update();
                                //vtkStructuredGridWriter *writer1 = vtkStructuredGridWriter::New();
                                //writer1->SetInput(interpolation->GetOutput());
                                //writer1->SetFileName("sgridface.vtk");
                                //writer1->Write();

                                this->SetFaceUGrid(i,j, boundingbox, pointscell, interpolation->GetOutput()->GetPoints());
                                points->Delete();
                                interpolation->Delete();
                                planegrid->Delete();
                        }
                }

                //        vtkSolidTransfiniteInterpolation* sol_interp = vtkSolidTransfiniteInterpolation//::New();
                vtkMimxSolidEllipticalInterpolation *sol_interp = 
                  vtkMimxSolidEllipticalInterpolation::New();
                sol_interp->SetNumberOfIterations(this->NumberOfIterations);
                vtkStructuredGrid *solidgrid = vtkStructuredGrid::New();
                solidgrid->SetPoints(pointscell);
                solidgrid->SetDimensions(dim);
                sol_interp->SetInput(solidgrid);
                // to compute the interior nodes of a structured hexahedron grid
                sol_interp->Update();
                this->SetCellPoints(sol_interp->GetOutput()->GetPoints(), pointidlist, output);
                //vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
                //writer->SetInput(sol_interp->GetOutput());
                //writer->SetFileName("sgrid.vtk");
                //writer->Write();

                solidgrid->Delete();
                sol_interp->Delete();
                pointscell->Delete();
                pointidlist->Delete();
                idlistedge->Delete();
        }
        //geofil->Delete();
        //vtkUnstructuredGridWriter* writer1 = vtkUnstructuredGridWriter::New();
        //writer1->SetInput(Input);
        //writer1->SetFileName("debug.vtk");
        //writer1->Write();

        return 1;
}

void vtkMimxRecalculateInteriorNodes::GetFaceUGrid(
  int CellNum, 
  int FaceNum, 
  vtkUnstructuredGrid* BBox, 
  vtkPoints* PointListCell, 
  vtkPoints* PointListFace)
{
        switch (FaceNum) {
        case 0:
                GetFace0UGrid(CellNum, BBox, PointListCell, PointListFace);
                break;
        case 1:
                GetFace1UGrid(CellNum, BBox, PointListCell, PointListFace);
                break;
        case 2:
                GetFace2UGrid(CellNum, BBox, PointListCell, PointListFace);
                break;
        case 3:
                GetFace3UGrid(CellNum, BBox, PointListCell, PointListFace);
                break;
        case 4:
                GetFace4UGrid(CellNum, BBox, PointListCell, PointListFace);
                break;
        case 5:
                GetFace5UGrid(CellNum, BBox, PointListCell, PointListFace);
                break;
        }
}

void vtkMimxRecalculateInteriorNodes::GetFace0UGrid(
  int CellNum, 
  vtkUnstructuredGrid* BBox,  
  vtkPoints* PointListCell, 
  vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointListFace->SetNumberOfPoints(dim[1]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointListFace->InsertPoint(y*dim[1] + x, PointListCell->GetPoint(y*dim[0]*dim[1]+x*dim[0]));
                        // //cout << y*dim[1]+x<<"  "<<y*dim[0]*dim[1]+x*dim[0]<<endl;
                }
        }
}

void vtkMimxRecalculateInteriorNodes::GetFace1UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointListFace->SetNumberOfPoints(dim[1]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointListFace->InsertPoint(y*dim[1] + x, PointListCell->GetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1));
                        // //cout <<y*dim[1] + x<<"  "<<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<endl;
                }
        }
}

void vtkMimxRecalculateInteriorNodes::GetFace2UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointListFace->SetNumberOfPoints(dim[0]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListFace->InsertPoint(y*dim[0] + z, PointListCell->GetPoint(y*dim[0]*dim[1]+z));
                        // //cout << y*dim[0] + z<<"  "<<y*dim[0]*dim[1]+z<<"  "<<endl;
                }
        }
}

void vtkMimxRecalculateInteriorNodes::GetFace3UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointListFace->SetNumberOfPoints(dim[0]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListFace->InsertPoint(y*dim[0] + z,PointListCell->GetPoint(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z));
                        // //cout << y*dim[0] + z<<"  "<<y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z<<"  "<<endl;
                }
        }
}

void vtkMimxRecalculateInteriorNodes::GetFace4UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointListFace->SetNumberOfPoints(dim[0]*dim[1]);
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListFace->InsertPoint(x*dim[0] + z, PointListCell->GetPoint(x*dim[0]+z));
                        // //cout<<x*dim[0] + z<<"  "<<x*dim[0]+z<<endl;
                }
        }
}

void vtkMimxRecalculateInteriorNodes::GetFace5UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointListFace->SetNumberOfPoints(dim[0]*dim[1]);
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListFace->InsertPoint(x*dim[0] + z, PointListCell->GetPoint((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z));
                        // //cout<<x*dim[0] + z<<"  "<<(dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z<<endl;
                }
        }
}

void vtkMimxRecalculateInteriorNodes::GetCellPoints(int CellNum, vtkUnstructuredGrid* BBox,  
                                                                                          vtkUnstructuredGrid *Input, vtkPoints* Points, vtkIdList *IdList)
{
        int dim[3], dimtemp[3], i, j, k, startelenum = 0;
        // calculate starting element number from which points need to be acquired
        for(i=0; i < CellNum; i++)
        {
                vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(i, dim);
                startelenum += (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
        }

        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        Points->SetNumberOfPoints(dim[0]*dim[1]*dim[2]);
        IdList->SetNumberOfIds(dim[0]*dim[1]*dim[2]);

        // start adding points and point numbers to the list
        vtkIdType npts, *pts;
        int elenum = startelenum;
        for (k=0; k<dim[2]-1; k++)
        {
                for (j=0; j<dim[1]-1; j++)
                {
                        for (i=0; i<dim[0]-1; i++)
                        {
                                Input->GetCellPoints(elenum, npts, pts);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i, Input->GetPoint(pts[0]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i, pts[0]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i+1, Input->GetPoint(pts[1]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i+1, pts[1]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1, Input->GetPoint(pts[2]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1, pts[2]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0], Input->GetPoint(pts[3]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0], pts[3]);

                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]*dim[1], Input->GetPoint(pts[4]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]*dim[1], pts[4]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i+1 + dim[0]*dim[1], Input->GetPoint(pts[5]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i+1 + dim[0]*dim[1], pts[5]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1 + dim[0]*dim[1], Input->GetPoint(pts[6]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1 + dim[0]*dim[1], pts[6]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0] + dim[0]*dim[1], Input->GetPoint(pts[7]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0] + dim[0]*dim[1], pts[7]);
                                elenum++;
                        }
                }
        }
        //double x[3];
        //ofstream OFile;
        //OFile.open("debug.txt", std::ios::out);

        //for (i=0; i < Points->GetNumberOfPoints(); i++)
        //{
        //        Points->GetPoint(i, x);
        //        OFile << i<<"  "<<IdList->GetId(i)<<"  "<<x[0]<<"  "<<x[1]<<"  "<<x[2]<<endl;
        //}
}


void vtkMimxRecalculateInteriorNodes::SetCellPoints(vtkPoints* Points, vtkIdList *IdList, vtkUnstructuredGrid *Output)
{
        for(int i=0; i < Points->GetNumberOfPoints(); i++)
        {
                Output->GetPoints()->SetPoint(IdList->GetId(i), Points->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetFaceUGrid(int CellNum, int FaceNum, vtkUnstructuredGrid* BBox,  
                                                                                           vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        switch (FaceNum) {
                case 0:
                        SetFace0UGrid(CellNum, BBox, PointListCell, PointListFace);
                        break;
                case 1:
                        SetFace1UGrid(CellNum, BBox, PointListCell, PointListFace);
                        break;
                case 2:
                        SetFace2UGrid(CellNum, BBox, PointListCell, PointListFace);
                        break;
                case 3:
                        SetFace3UGrid(CellNum, BBox, PointListCell, PointListFace);
                        break;
                case 4:
                        SetFace4UGrid(CellNum, BBox, PointListCell, PointListFace);
                        break;
                case 5:
                        SetFace5UGrid(CellNum, BBox, PointListCell, PointListFace);
                        break;
        }
}

void vtkMimxRecalculateInteriorNodes::SetFace0UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointListCell->SetPoint(y*dim[0]*dim[1]+x*dim[0],
                                PointListFace->GetPoint(y*dim[1] + x));
                        //                // //cout<<y*dim[0]*dim[1]+x*dim[0]<<"  "<<y*dim[1] + x<<endl;
                }
        }
}

void vtkMimxRecalculateInteriorNodes::SetFace1UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointListCell->SetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1,
                                PointListFace->GetPoint(y*dim[1] + x));
                        //// //cout <<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<"  "<<y*dim[1] + x<<endl;
                }
        }
}

void vtkMimxRecalculateInteriorNodes::SetFace2UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListCell->SetPoint(y*dim[0]*dim[1]+z,PointListFace->GetPoint(y*dim[0] + z));
                }
        }
}

void vtkMimxRecalculateInteriorNodes::SetFace3UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListCell->SetPoint(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z,
                                PointListFace->GetPoint(y*dim[0] + z));
                }
        }
}

void vtkMimxRecalculateInteriorNodes::SetFace4UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListCell->SetPoint(x*dim[0]+z,PointListFace->GetPoint(x*dim[0] + z));
                }
        }
}

void vtkMimxRecalculateInteriorNodes::SetFace5UGrid(int CellNum, vtkUnstructuredGrid* BBox,  vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListCell->SetPoint((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z,
                                PointListFace->GetPoint(x*dim[0] + z));
                }
        }
}

int vtkMimxRecalculateInteriorNodes::CheckBoundaryFace(vtkIdList* IdList, vtkPolyData *BoundaryData)
{
        vtkIdList *idlist2;
        vtkIdType pt1,pt2,pt3,pt4;
        pt1 = IdList->GetId(0);        pt2 = IdList->GetId(1);
        pt3 = IdList->GetId(2);        pt4 = IdList->GetId(3);
        for(int i=0; i < BoundaryData->GetNumberOfCells(); i++)
        {
                idlist2 = BoundaryData->GetCell(i)->GetPointIds();
                vtkIdType p1,p2,p3,p4;
                p1 = idlist2->GetId(0);        p2 = idlist2->GetId(1);
                p3 = idlist2->GetId(2);        p4 = idlist2->GetId(3);

                if(idlist2->IsId(IdList->GetId(0)) != -1 && idlist2->IsId(IdList->GetId(1)) != -1 &&
                        idlist2->IsId(IdList->GetId(2)) != -1 && idlist2->IsId(IdList->GetId(3)) != -1)
                {
                        return 1;
                }
        }
        return 0;
}

int vtkMimxRecalculateInteriorNodes::CheckBoundaryEdge(vtkIdList* IdList, vtkPolyData *BoundaryData)
{
        vtkIdList *idlist2;
        vtkIdType pt1,pt2;
        pt1 = IdList->GetId(0);        pt2 = IdList->GetId(1);
        for(int i=0; i < BoundaryData->GetNumberOfCells(); i++)
        {
                idlist2 = BoundaryData->GetCell(i)->GetPointIds();
                vtkIdType p1,p2,p3,p4;
                p1 = idlist2->GetId(0);        p2 = idlist2->GetId(1);
                p3 = idlist2->GetId(2);        p4 = idlist2->GetId(3);
                // we always have 4 sided polygon
                // check for all the four sides
                //side 1
                if(idlist2->IsId(IdList->GetId(0)) != -1 && idlist2->IsId(IdList->GetId(1)) != -1 )
                {
                        return 1;
                }
        }
        return 0;
}

void vtkMimxRecalculateInteriorNodes::RecalculateEdge(int CellNum, int EdgeNum, vtkIdList *IdList, 
                                                                                                  vtkUnstructuredGrid *BBox, vtkPoints *CellPoints)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        vtkPoints *Points1 = vtkPoints::New();
        vtkPoints *Points2 = vtkPoints::New();
        vtkPoints *Points3 = vtkPoints::New();

        double pt1[3], pt2[3];

        switch (EdgeNum) {
                case 0:
                        if(IdList->GetId(3))
                        {
                                this->GetEdge(CellNum, 3, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 8, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(1))
                        {
                                this->GetEdge(CellNum, 1, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 9, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }

                        Points3->SetNumberOfPoints(dim[0]);        
                        break;

                case 1:
                        if(IdList->GetId(0))
                        {
                                this->GetEdge(CellNum, 0, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 9, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(2))
                        {
                                this->GetEdge(CellNum, 2, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 11, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[1]);        
                        break;
                case 2:
                        if(IdList->GetId(3))
                        {
                                this->GetEdge(CellNum, 3, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 10, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(1))
                        {
                                this->GetEdge(CellNum, 1, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 11, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[0]);        
                        break;
                case 3:
                        if(IdList->GetId(0))
                        {
                                this->GetEdge(CellNum, 0, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 8, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(2))
                        {
                                this->GetEdge(CellNum, 2, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 10, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[1]);        
                        break;
                case 4:
                        if(IdList->GetId(8))
                        {
                                this->GetEdge(CellNum, 8, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 7, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(9))
                        {
                                this->GetEdge(CellNum, 9, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 5, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[0]);        
                        break;
                case 5:
                        if(IdList->GetId(4))
                        {
                                this->GetEdge(CellNum, 4, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 9, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        if(IdList->GetId(11))
                        {
                                this->GetEdge(CellNum, 11, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 6, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[1]);        
                        break;

                case 6:
                        if(IdList->GetId(7))
                        {
                                this->GetEdge(CellNum, 7, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 10, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        if(IdList->GetId(11))
                        {
                                this->GetEdge(CellNum, 11, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 5, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[0]);        
                        break;
                case 7:
                        if(IdList->GetId(4))
                        {
                                this->GetEdge(CellNum, 4, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 8, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        if(IdList->GetId(10))
                        {
                                this->GetEdge(CellNum, 10, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 6, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[1]);        
                        break;
                case 8:
                        if(IdList->GetId(0))
                        {
                                this->GetEdge(CellNum, 0, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 3, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(4))
                        {
                                this->GetEdge(CellNum, 4, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 7, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[2]);
                        break;
                case 9:
                        if(IdList->GetId(0))
                        {
                                this->GetEdge(CellNum, 0, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 1, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(4))
                        {
                                this->GetEdge(CellNum, 4, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 5, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[2]);        
                        break;

                case 10:
                        if(IdList->GetId(3))
                        {
                                this->GetEdge(CellNum, 3, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 2, BBox, CellPoints, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(7))
                        {
                                this->GetEdge(CellNum, 7, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 6, BBox, CellPoints, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[2]);        
                        break;
                case 11:
                        if(IdList->GetId(1))
                        {
                                this->GetEdge(CellNum, 1, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 2, BBox, CellPoints, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        if(IdList->GetId(5))
                        {
                                this->GetEdge(CellNum, 5, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 6, BBox, CellPoints, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[2]);
                        break;

        }

        for(int i=0; i <Points3->GetNumberOfPoints(); i++)
        {
                Points3->SetPoint(i, pt1[0] + ((pt2[0]-pt1[0])/(Points3->GetNumberOfPoints()-1))*i, pt1[1] + ((pt2[1]-pt1[1])/(Points3->GetNumberOfPoints()-1))*i, pt1[2] + ((pt2[2]-pt1[2])/(Points3->GetNumberOfPoints()-1))*i);
        }
        this->SetEdge(CellNum, EdgeNum, BBox, CellPoints, Points3);
        Points1->Delete();
        Points2->Delete();
        Points3->Delete();
}
void vtkMimxRecalculateInteriorNodes::GetEdge(int CellNum, int EdgeNum, vtkUnstructuredGrid* BBox,  
                                                                                  vtkPoints* PointListCell, vtkPoints* PointListEdge)
{
        switch (EdgeNum) {
                case 0:
                        GetEdge0(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 1:
                        GetEdge1(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 2:
                        GetEdge2(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 3:
                        GetEdge3(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 4:
                        GetEdge4(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 5:
                        GetEdge5(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 6:
                        GetEdge6(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 7:
                        GetEdge7(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 8:
                        GetEdge8(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 9:
                        GetEdge9(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 10:
                        GetEdge10(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 11:
                        GetEdge11(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge0(int CellNum, vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, 
                                                                                   vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i < dim[0]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge1(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*(i+1)-1));
                // //cout <<dim[0]*(i+1)-1<<endl;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge2(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i< dim[0]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*(dim[1]-1)+i));
                // //cout <<dim[0]*(dim[1]-1)+i<<endl;

        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge3(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(i*dim[0]));
                // //cout <<i*dim[0]<<endl;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge4(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i < dim[0]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*dim[1]*(dim[2]-1) + i));
                // //cout<<dim[0]*dim[1]*(dim[2]-1) + i<<endl;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge5(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(i+1)-1));
                // //cout <<dim[0]*dim[1]*(dim[2]-1) + dim[0]*(i+1)-1<<endl;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge6(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i< dim[0]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(dim[1]-1)+i));
                // //cout<<dim[0]*dim[1]*(dim[2]-1) + dim[0]*(dim[1]-1)+i<<endl;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge7(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*dim[1]*(dim[2]-1) + i*dim[0]));
                // //cout<<dim[0]*dim[1]*(dim[2]-1) + i*dim[0]<<endl;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge8(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i < dim[2]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*dim[1]*i));
                // //cout<<dim[0]*dim[1]*i<<endl;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge9(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i< dim[2]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*dim[1]*i + dim[0]-1));
                // //cout<<dim[0]*dim[1]*i + dim[0]-1<<endl;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge10(int CellNum, vtkPoints *PointListCell,
                                                                                        vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i< dim[2]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*dim[1]*i + dim[0]*(dim[1]-1)));
                // //cout<<dim[0]*dim[1]*i + dim[0]*(dim[1]-1)<<endl;
        }
}

void vtkMimxRecalculateInteriorNodes::GetEdge11(int CellNum, vtkPoints *PointListCell,
                                                                                        vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i< dim[2]; i++)
        {
                PointList->InsertPoint(i, PointListCell->GetPoint(dim[0]*dim[1]*i + dim[0]*dim[1]-1));
                // //cout<<dim[0]*dim[1]*i + dim[0]*dim[1]-1<<endl;
        }
}
void vtkMimxRecalculateInteriorNodes::SetEdge(int CellNum, int EdgeNum, vtkUnstructuredGrid* BBox,  
                                                                                  vtkPoints* PointListCell, vtkPoints* PointListEdge)
{
        switch (EdgeNum) {
                case 0:
                        SetEdge0(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 1:
                        SetEdge1(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 2:
                        SetEdge2(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 3:
                        SetEdge3(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 4:
                        SetEdge4(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 5:
                        SetEdge5(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 6:
                        SetEdge6(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 7:
                        SetEdge7(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 8:
                        SetEdge8(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 9:
                        SetEdge9(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 10:
                        SetEdge10(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
                case 11:
                        SetEdge11(CellNum,  PointListCell, BBox, PointListEdge);
                        break;
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge0(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(i, PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge1(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*(i+1)-1, PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge2(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*(dim[1]-1)+i, PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge3(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(i*dim[0], PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge4(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*dim[1]*(dim[2]-1) + i, PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge5(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(i+1)-1, PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge6(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(dim[1]-1)+i, PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge7(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*dim[1]*(dim[2]-1) + i*dim[0], PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge8(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*dim[1]*i, PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge9(int CellNum, vtkPoints *PointListCell,
                                                                                   vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*dim[1]*i + dim[0]-1, PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge10(int CellNum, vtkPoints *PointListCell,
                                                                                        vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*dim[1]*i + dim[0]*(dim[1]-1), PointList->GetPoint(i));
        }
}

void vtkMimxRecalculateInteriorNodes::SetEdge11(int CellNum, vtkPoints *PointListCell,
                                                                                        vtkUnstructuredGrid *BBox, vtkPoints* PointList)
{
        int dim[3], dimtemp[3];
        vtkIntArray::SafeDownCast(BBox->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(CellNum, dimtemp);

        dim[0] = dimtemp[2];
        dim[1] = dimtemp[0];
        dim[2] = dimtemp[1];

        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                PointListCell->SetPoint(dim[0]*dim[1]*i + dim[0]*dim[1]-1, PointList->GetPoint(i));
        }
}

int vtkMimxRecalculateInteriorNodes::CheckIfEdgeNodesRecalculated(int EdgeNum, vtkIdList *IdList)
{
        if(EdgeNum == 0)
        {
                if(!IdList->GetId(1) && !IdList->GetId(3) && !IdList->GetId(9) && !IdList->GetId(8))        return 0;
                else        return 1;
        }
        if(EdgeNum == 1)
        {
                if(!IdList->GetId(0) && !IdList->GetId(2) && !IdList->GetId(9) && !IdList->GetId(11))        return 0;
                else        return 1;
        }
        if(EdgeNum == 2)
        {
                if(!IdList->GetId(1) && !IdList->GetId(3)&& !IdList->GetId(11) && !IdList->GetId(10))        return 0;
                else        return 1;
        }
        if(EdgeNum == 3)
        {
                if(!IdList->GetId(0) && !IdList->GetId(2) && !IdList->GetId(8) && !IdList->GetId(10))        return 0;
                else        return 1;
        }
        if(EdgeNum == 4)
        {
                if(!IdList->GetId(5) && !IdList->GetId(7) && !IdList->GetId(8) && !IdList->GetId(9))        return 0;
                else        return 1;
        }
        if(EdgeNum == 5)
        {
                if(!IdList->GetId(4) && !IdList->GetId(6) && !IdList->GetId(9) && !IdList->GetId(11))        return 0;
                else        return 1;
        }
        if(EdgeNum == 6)
        {
                if(!IdList->GetId(5) && !IdList->GetId(7) && !IdList->GetId(11) && !IdList->GetId(10))        return 0;
                else        return 1;
        }
        if(EdgeNum == 7)
        {
                if(!IdList->GetId(4) && !IdList->GetId(6) && !IdList->GetId(8) && !IdList->GetId(10))        return 0;
                else        return 1;
        }
        if(EdgeNum == 8)
        {
                if(!IdList->GetId(3) && !IdList->GetId(7) && !IdList->GetId(0) && !IdList->GetId(4))        return 0;
                else        return 1;
        }
        if(EdgeNum == 9)
        {
                if(!IdList->GetId(1) && !IdList->GetId(5) && !IdList->GetId(0) && !IdList->GetId(4))        return 0;
                else        return 1;
        }
        if(EdgeNum == 10)
        {
                if(!IdList->GetId(3) && !IdList->GetId(7) && !IdList->GetId(2) && !IdList->GetId(6))        return 0;
                else        return 1;
        }
        if(EdgeNum == 11)
        {
                if(!IdList->GetId(5) && !IdList->GetId(1) && !IdList->GetId(2) && !IdList->GetId(6))        return 0;
                else        return 1;
        }
        return 0;
}

void vtkMimxRecalculateInteriorNodes::SetBoundingBox(vtkUnstructuredGrid *UGrid)
{
        this->SetInput(1, UGrid);
}

void vtkMimxRecalculateInteriorNodes::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
