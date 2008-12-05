/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxGenerateHexahedronMesh.cxx,v $
Language:  C++
Date:      $Date: 2007/09/05 18:42:59 $
Version:   $Revision: 1.23 $

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

#include "vtkMimxGenerateHexahedronMesh.h"
#include "vtkCollection.h"
#include "vtkStructuredGrid.h"
#include "vtkCellArray.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkHexahedron.h"
#include "vtkDataSetMapper.h"
#include "vtkActor.h"
#include "vtkStructuredGridWriter.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkProperty.h"
#include "vtkPolyData.h"
#include "vtkMergeCells.h"
#include "vtkMergePoints.h"
#include "vtkCellLocator.h"
#include "vtkGeometryFilter.h"
#include "vtkPolyDataWriter.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkIdList.h"
#include "vtkQuad.h"
#include "vtkMimxSolidTransfiniteInterpolation.h"
#include "vtkMimxPlanarTransfiniteInterpolation.h"
#include "vtkPointSet.h"
#include "vtkMimxSmoothUnstructuredGridFilter.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkGenericCell.h"
#include "vtkStructuredGridWriter.h"
#include "vtkMimxPlanarEllipticalInterpolation.h"
#include "vtkMimxSolidEllipticalInterpolation.h"

#include "mimxLinkedList.h"

vtkCxxRevisionMacro(vtkMimxGenerateHexahedronMesh, "$Revision: 1.23 $");
vtkStandardNewMacro(vtkMimxGenerateHexahedronMesh);


// Constructs with initial  values.
vtkMimxGenerateHexahedronMesh::vtkMimxGenerateHexahedronMesh()
{
        this->StructuredGridCollection = vtkCollection::New();
        this->StructuredGridMapperCollection= vtkCollection::New();
        this->StructuredGridActorCollection = vtkCollection::New();
        this->UGrid = vtkUnstructuredGrid::New();
        this->Mapper = vtkDataSetMapper::New();
        this->Actor =  vtkActor::New();
        this->MeshSeedX = vtkIntArray::New();
        this->MeshSeedY = vtkIntArray::New();
        this->MeshSeedZ = vtkIntArray::New();
        this->Links = 0;
}

// Destroy any allocated memory.
vtkMimxGenerateHexahedronMesh::~vtkMimxGenerateHexahedronMesh()
{
        this->UGrid->Delete();
        this->MeshSeedX->Delete();
        this->MeshSeedY->Delete();
        this->MeshSeedZ->Delete();
}

void vtkMimxGenerateHexahedronMesh::ProjectVertices(vtkPolyData *polydata)
{
        // project all the exterior vertices on the bounding box
        vtkGeometryFilter *fil = vtkGeometryFilter::New();
        fil->SetInput(this->GetInput());
        fil->Update();
        vtkCellLocator *CellLocator = vtkCellLocator::New();
        CellLocator->SetDataSet(polydata);
        CellLocator->BuildLocator();
        vtkIdList *idlist = vtkIdList::New();
        for(int i =0; i < this->GetInput()->GetNumberOfPoints(); i++)
        {
                idlist->Initialize();
                fil->GetOutput()->GetPointCells(i, idlist);
                if(idlist->GetNumberOfIds() > 0)
                {
                        double dist,ClosestPoint[3],x[3];
                        vtkIdType CellId;
                        int SubId;
                        this->GetInput()->GetPoint(i,x);
                        CellLocator->FindClosestPoint(x,ClosestPoint,CellId,SubId,dist);
                        this->GetInput()->GetPoints()->SetPoint(i,ClosestPoint);
                }

        }
        this->GetInput()->Modified();
        fil->Delete();
        CellLocator->Delete();
        idlist->Delete();

}
// function to generate rectilinear mesh, there are duplicate nodes at the common faces
void vtkMimxGenerateHexahedronMesh::GenerateRectilinearHexahedronMesh()
{
        vtkIdType npts;
        vtkIdType *pts;
        // travese through all the cells
        this->GetInput()->GetCells()->InitTraversal();
        // traverse through all the structured hexahedral mesh
        // whose nodes to be computed
        vtkHexahedron* hexahedron = vtkHexahedron::New();
        // to copy the mesh seeds
//      IteratorType iteratorX = this->MeshSeedListX.begin();
//      IteratorType iteratorY = this->MeshSeedListY.begin();
//      IteratorType iteratorZ = this->MeshSeedListZ.begin();

        for(int j=0; j < this->GetInput()->GetNumberOfCells();j++)
        {
                this->GetInput()->GetCells()->GetNextCell(npts, pts);
                this->StructuredGridCollection->AddItem((vtkObject*) vtkStructuredGrid::New());
                for(int i=0; i < npts; i++)
                        hexahedron->GetPoints()->SetPoint(i,this->GetInput()->GetPoint(pts[i]));
                int dim[3];

                dim[0] = this->MeshSeedZ->GetValue(j)+1;
                dim[1] = this->MeshSeedX->GetValue(j)+1;
                dim[2] = this->MeshSeedY->GetValue(j)+1;
                
//              dim[0] =6; dim[1] = 6; dim[2] = 6;
                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(j)))->SetDimensions(dim);
                // to calculate the parametric coordinates;
                double shape[3];
                for(int i=0; i < 3; i++)        shape[i] = 1.0/(dim[i]-1);
                // compute the points for the structured grid corresponding to given cell
                vtkPoints* points = vtkPoints::New();
                points->SetNumberOfPoints(dim[0]*dim[1]*dim[2]);
                for(int y=0; y <dim[2]; y++)
                {
                        for(int x=0; x<dim[1]; x++)
                        {
                                for(int z=0; z < dim[0]; z++)
                                {
                                        double shape_loc[3];
                                        shape_loc[0] = z*shape[0];
                                        shape_loc[1] = x*shape[1];
                                        shape_loc[2] = y*shape[2];
                                        int subid; double interp_loc[3], weights[8];
                                        // use isoparametric shape functions to evaluate
                                        // physical coordinates
                                        hexahedron->EvaluateLocation(subid,shape_loc,interp_loc,weights);
                                        points->SetPoint(y*dim[0]*dim[1]+x*dim[0]+z,interp_loc);
                                        //// cout<<y*dim[0]*dim[1]+x*dim[0]+z<<"  "<<interp_loc[0]<<"  "<<
                                                //interp_loc[1]<<"  "<<interp_loc[2]<<endl;
                                }
                        }
                }
                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(j)))->SetPoints(points);
                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(j)))->Modified();
                //vtkStructuredGridWriter* writer = vtkStructuredGridWriter::New();
                //writer->SetInput(((vtkStructuredGrid*)(this->StructuredGridCollection->
                //      GetItemAsObject(j))));
                //writer->SetFileName("debug.txt");
                //writer->Write();
                points->Delete();
                this->StructuredGridMapperCollection->AddItem
                        ((vtkObject*) vtkDataSetMapper::New());
                ((vtkDataSetMapper*)(this->StructuredGridMapperCollection->
                        GetItemAsObject(j)))->SetInput((vtkStructuredGrid*)
                        (this->StructuredGridCollection->GetItemAsObject(j)));
                this->StructuredGridActorCollection->AddItem
                        ((vtkObject*) vtkActor::New());
                ((vtkActor*)(this->StructuredGridActorCollection->
                        GetItemAsObject(j)))->SetMapper((vtkDataSetMapper*)
                        (this->StructuredGridMapperCollection->GetItemAsObject(j)));
                ((vtkActor*)(this->StructuredGridActorCollection->
                        GetItemAsObject(j)))->GetProperty()->SetRepresentationToWireframe();
        }
}

void vtkMimxGenerateHexahedronMesh::MorphRectilinearHexahedronMesh(vtkPolyData* polydata)
{

//  first extract surface nodes and their connectivity
vtkGeometryFilter *geofil = vtkGeometryFilter::New();
geofil->SetInput(this->GetInput());
geofil->Update();
vtkIdList *idlist ;//= vtkIdList::New();
vtkIdList *idlistedge = vtkIdList::New();
idlistedge->SetNumberOfIds(12);
vtkIdList *idlistface = vtkIdList::New();
idlistface->SetNumberOfIds(6);
vtkCellLocator *CellLocator = vtkCellLocator::New();
CellLocator->SetDataSet(polydata);
CellLocator->BuildLocator();
vtkGenericCell *gencell = vtkGenericCell::New();

//this->GetInput()->BuildLinks();
for(int i=0; i<6; i++)
{
        vtkPoints *temp = vtkPoints::New();
        temp->Initialize();
        this->GetFace(0,i,temp);
}
for(int i=0; i < this->GetInput()->GetNumberOfCells(); i++)
{
        for(int k = 0; k<12; k++)       idlistedge->SetId(k,0);
        for(int k=0; k<6; k++)  idlistface->SetId(k,0);
        // start with edges
        gencell->Initialize();
        this->GetInput()->GetCell(i,gencell);

        for(int k=0; k<12; k++)
        {
                idlist = gencell->GetEdge(k)->GetPointIds();
                if(this->CheckBoundaryEdge(idlist,geofil->GetOutput()))
                {
                        idlistedge->SetId(k,1);
                        vtkPoints * points = vtkPoints::New();
                        this->GetEdge(i,k,points);
                        this->ClosestPointProjection(CellLocator, points);
                        this->SetEdge(i,k,points);
                        points->Delete();
                }
        }
        for(int j=0; j < 6; j++)
        {
                //      loop through all the face and morph all the boundary faces
//              idlist->Initialize();
                idlist = gencell->GetFace(j)->GetPointIds();
                // check if the face is a boundary or not
                if(this->CheckBoundaryFace(idlist, geofil->GetOutput()))
                {
                        idlistface->SetId(j,1);
                        vtkPoints * points = vtkPoints::New();
                        this->GetFace(i,j,points);
                        this->ClosestPointProjection(CellLocator, points);
                        this->SetFace(i,j,points);
                        points->Delete();
                }
        }
        // for the interior edge start and end points from the morphed mesh need
        // to be chosen and the nodal position has to be recalculated.
        // check if edge of a cell is on the boundary or not

        vtkIdType pt;
        for(int k=0; k<12; k++)
        {
                pt = idlistedge->GetId(k);
        }
        for(int k=0; k<12; k++)
        {
                if(!idlistedge->GetId(k))
                {
                        if(this->CheckIfEdgeNodesRecalculated(k, idlistedge))
                        {
                                // recalculate the interior nodes of a given edge
                                this->RecalculateEdge(i, k, idlistedge);
                        }
                }
        }
//       if a face is an interior face computer interior nodes of the face using planar transfinite interpolation
        for(int j=0; j <6; j++)
        {
                if(!idlistface->GetId(j))
                {
                        vtkPoints* points = vtkPoints::New();
                        this->GetFace(i,j,points);
                        vtkMimxPlanarTransfiniteInterpolation* interpolation = 
                                vtkMimxPlanarTransfiniteInterpolation::New();
                        vtkPointSet* pointset = vtkStructuredGrid::New();
                        int dim[3];
                        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(i)))->GetDimensions(dim);
                                pointset->SetPoints(points);
                        interpolation->SetInput(pointset);
                        if(j==2 || j==3)
                        {
                                interpolation->SetIDiv(dim[0]);
                                interpolation->SetJDiv(dim[2]);
                        }
                        if(j==0 || j==1)
                        {
                                interpolation->SetIDiv(dim[1]);
                                interpolation->SetJDiv(dim[2]);
                        }
                        if(j==4 || j==5)
                        {
                                interpolation->SetIDiv(dim[0]);
                                interpolation->SetJDiv(dim[1]);
                        }
                        // to compute the interior nodes of a face
                        interpolation->Update();
                        this->SetFace(i,j,interpolation->GetOutput()->GetPoints());
                        pointset->Delete();
                        points->Delete();
                        interpolation->Delete();
                }
        }
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(i)))->GetDimensions(dim);
//      vtkSolidTransfiniteInterpolation* sol_interp = vtkSolidTransfiniteInterpolation//::New();
        vtkMimxSolidTransfiniteInterpolation *sol_interp = vtkMimxSolidTransfiniteInterpolation::New();
        sol_interp->SetIDiv(dim[0]);    sol_interp->SetJDiv(dim[1]); sol_interp->SetKDiv(dim[2]);
        vtkPointSet* pointset = vtkStructuredGrid::New();
        pointset->SetPoints(((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(i)))->GetPoints());
        sol_interp->SetInput(pointset);
        // to compute the interior nodes of a structured hexahedron grid
        sol_interp->Update();
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(i)))->SetPoints(sol_interp->GetOutput()->GetPoints());
        pointset->Delete();
        sol_interp->Delete();
}
for (int i=0; i<this->GetInput()->GetNumberOfCells(); i++)
{
        ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(i)))->Modified();
        vtkStructuredGridWriter *structgrid = vtkStructuredGridWriter::New();
        structgrid->SetInput(((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(i))));
        structgrid->SetFileName("SGrid.vtk");
        structgrid->Write();
        structgrid->Delete();
}
this->MergeStructuredGridsToUnstructuredGrid(polydata);

idlistedge->Delete();
idlistface->Delete();
idlist->Delete();
//gencell->Delete();
geofil->Delete();
CellLocator->Delete();
}

void vtkMimxGenerateHexahedronMesh::CreateOutput()
{
        //      initial structured grid collection should not be empty
        if(this->StructuredGridCollection->GetNumberOfItems() <=0)
        {
//              return 1;
        }
        int numele = 0;
        int numnodes = 0;
        int dim[3];
        //      calculate number of nodes and elements
        for(int i=0; i <this->StructuredGridCollection->GetNumberOfItems(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(i)))->GetDimensions(dim);
                numnodes = numnodes + dim[0]*dim[1]*dim[2];
                numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
        }
        // copy the nodal co-ordinates from list of structured mesh
        // to the out put which is the unstructured mesh
/*      vtkPoints* points = vtkPoints::New();
        vtkMergePoints* mergepoints = vtkMergePoints::New();
//      points->SetNumberOfPoints(numnodes);
        for(int i=0; i <this->StructuredGridCollection->GetNumberOfItems(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(i)))->GetDimensions(dim);
                for(int n=0; n <dim[2]; n++)
                {
                        for(int m=0; m<dim[1]; m++)
                        {
                                for(int k=0; k < dim[0]; k++)
                                {
                                        points->InsertNextPoint(((vtkStructuredGrid*)(this->StructuredGridCollection->
                                                GetItemAsObject(i)))->GetPoint(n*dim[0]*dim[1]+m*dim[0]+k));
                                }
                        }
                }
        }

        this->UGrid->SetPoints(points);
//      points->Delete();
        //      allocate initial number of cells
        this->UGrid->Allocate(numele,numele);
        // 
        //      copy the cell connectivity in the structured grid 
        numnodes = 0;
        for(int i=0; i <this->StructuredGridCollection->GetNumberOfItems(); i++)
        {
                // store the number of nodes in each cell
                if(i !=0)
                {
                        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(i-1)))->GetDimensions(dim);
                        numnodes = numnodes + dim[0]*dim[1]*dim[2];
                }
                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(i)))->GetDimensions(dim);
                for(int n=0; n <dim[2]-1; n++)
                {
                        for(int m=0; m<dim[1]-1; m++)
                        {
                                for(int k=0; k < dim[0]-1; k++)
                                {
                                        cell = ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                                GetItemAsObject(i)))->GetCell(n*(dim[0]-1)*(dim[1]-1)+m*(dim[0]-1)+k);
                                        if(i == 0)
                                                this->UGrid->InsertNextCell(VTK_HEXAHEDRON, cell->GetPointIds());
                                        else
                                        {
                                                vtkIdList* idlist = cell->GetPointIds();
                                                for(int p=0; p<idlist->GetNumberOfIds(); p++)
                                                {
                                                        idlist->SetId(p,idlist->GetId(p)+numnodes);
                                                }
                                                this->UGrid->InsertNextCell(VTK_HEXAHEDRON, idlist);
                                        }
                                }
                        }
                }
        }*/
        
                
/*      vtkMergeCells* mergecells = vtkMergeCells::New();
        mergecells->SetUnstructuredGrid(this->UGrid);
        mergecells->MergeDuplicatePointsOn();
        mergecells->SetTotalNumberOfDataSets(this->StructuredGridCollection->GetNumberOfItems());
        mergecells->SetTotalNumberOfCells(numele);
        mergecells->SetTotalNumberOfPoints(numnodes);
        for(int i=0; i <this->StructuredGridCollection->GetNumberOfItems(); i++)
        {
                mergecells->MergeDataSet(((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(i))));
        }
        vtkIdType max_val = 0;
        for(int i=0; i < this->UGrid->GetNumberOfCells(); i++)
        {
                cell = this->UGrid->GetCell(i);
                vtkIdList* idlist = cell->GetPointIds();
                
                for(int j=0; j <8; j++)
                {
                        vtkIdType val = idlist->GetId(j);
                        if(val > max_val)       max_val = val;
                }
        }
        this->UGrid->GetPoints()->Squeeze();
        this->UGrid->GetPoints()->SetNumberOfPoints(max_val+1);
        this->Mapper->SetInput(this->UGrid);
        this->Actor->SetMapper(this->Mapper);
        vtkUnstructuredGridWriter* writer = vtkUnstructuredGridWriter::New();
        writer->SetInput(this->UGrid);
        writer->SetFileName("debug.txt");
        writer->Write();*/
}

int vtkMimxGenerateHexahedronMesh::CheckBoundaryFace(vtkIdList* IdList, vtkPolyData *BoundaryData)
{
        vtkIdList *idlist2;
        vtkIdType pt1,pt2,pt3,pt4;
        pt1 = IdList->GetId(0); pt2 = IdList->GetId(1);
        pt3 = IdList->GetId(2); pt4 = IdList->GetId(3);
        for(int i=0; i < BoundaryData->GetNumberOfCells(); i++)
        {
                idlist2 = BoundaryData->GetCell(i)->GetPointIds();
                vtkIdType p1,p2,p3,p4;
                p1 = idlist2->GetId(0); p2 = idlist2->GetId(1);
                p3 = idlist2->GetId(2); p4 = idlist2->GetId(3);

                if(idlist2->IsId(IdList->GetId(0)) != -1 && idlist2->IsId(IdList->GetId(1)) != -1 &&
                        idlist2->IsId(IdList->GetId(2)) != -1 && idlist2->IsId(IdList->GetId(3)) != -1)
                {
                        return 1;
                }
        }
        return 0;
}

int vtkMimxGenerateHexahedronMesh::CheckNumberOfCellsSharingEdge(vtkCell* edge)
{
        int numcells = 0;
        vtkCell* cell;
        vtkIdType edgept0 = edge->GetPointId(0);        vtkIdType edgept1 = edge->GetPointId(1);
        // valency of an interior point in the unstructured hexahedral grid is 8
        // check for the first point
        for(int i = 0; i < this->GetInput()->GetNumberOfCells(); i++)
        {
                cell = this->GetInput()->GetCell(i);
                for(int j=0; j<cell->GetNumberOfEdges(); j++)
                {
                        vtkCell* edgecell = cell->GetEdge(j);
                        vtkIdType pt0 = edgecell->GetPointId(0);
                        vtkIdType pt1 = edgecell->GetPointId(1);
                        if(edgept0 == pt0 || edgept0 == pt1)
                                if(edgept1 == pt0 || edgept1 == pt1)
                                        numcells++;
                }
        }
        return numcells;
}

void vtkMimxGenerateHexahedronMesh::LaplacianSmoothing(vtkPolyData* Polydata)
{
        vtkMimxSmoothUnstructuredGridFilter* smoothugrid = vtkMimxSmoothUnstructuredGridFilter::New();
        smoothugrid->SetNumberOfIterations(10);
        smoothugrid->SetInput(this->UGrid);
        smoothugrid->SetSource(Polydata);
        smoothugrid->Update();
        this->UGrid->SetPoints(smoothugrid->GetOutput()->GetPoints());
        this->UGrid->Modified();
        //vtkUnstructuredGridWriter* writer = vtkUnstructuredGridWriter::New();
        //writer->SetInput(this->UGrid);
        //writer->SetFileName("debug2.txt");
        //writer->Write();
}

// face numbering is accroding to VTK hexahedron convention
void vtkMimxGenerateHexahedronMesh::GetFace(int CellNum, int FaceNum, vtkPoints* PointList)
{
        switch (FaceNum) {
                case 0:
                        GetFace0(CellNum, PointList);
                        break;
                case 1:
                        GetFace1(CellNum, PointList);
                        break;
                case 2:
                        GetFace2(CellNum, PointList);
                        break;
                case 3:
                        GetFace3(CellNum, PointList);
                        break;
                case 4:
                        GetFace4(CellNum, PointList);
                        break;
                case 5:
                        GetFace5(CellNum, PointList);
                        break;
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace0(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointList->InsertPoint(y*dim[1] + x, ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoint(y*dim[0]*dim[1]+x*dim[0]));
                        // cout << y*dim[1]+x<<"  "<<y*dim[0]*dim[1]+x*dim[0]<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace1(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointList->InsertPoint(y*dim[1] + x, ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1));
                        // cout <<y*dim[1] + x<<"  "<<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace2(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointList->InsertPoint(y*dim[0] + z, ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoint(y*dim[0]*dim[1]+z));
                        // cout << y*dim[0] + z<<"  "<<y*dim[0]*dim[1]+z<<"  "<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace3(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointList->InsertPoint(y*dim[0] + z,((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoint(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z));
                        // cout << y*dim[0] + z<<"  "<<y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z<<"  "<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace4(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]*dim[1]);
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointList->InsertPoint(x*dim[0] + z, ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoint(x*dim[0]+z));
                        // cout<<x*dim[0] + z<<"  "<<x*dim[0]+z<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace5(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]*dim[1]);
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointList->InsertPoint(x*dim[0] + z, ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoint((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z));
                        // cout<<x*dim[0] + z<<"  "<<(dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace(int CellNum, int FaceNum, vtkPoints* PointList)
{
        switch (FaceNum) {
                case 0:
                        SetFace0(CellNum, PointList);
                        break;
                case 1:
                        SetFace1(CellNum, PointList);
                        break;
                case 2:
                        SetFace2(CellNum, PointList);
                        break;
                case 3:
                        SetFace3(CellNum, PointList);
                        break;
                case 4:
                        SetFace4(CellNum, PointList);
                        break;
                case 5:
                        SetFace5(CellNum, PointList);
                        break;
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace0(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoints()->SetPoint(y*dim[0]*dim[1]+x*dim[0],
                                PointList->GetPoint(y*dim[1] + x));
        //              // cout<<y*dim[0]*dim[1]+x*dim[0]<<"  "<<y*dim[1] + x<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace1(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoints()->SetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1,
                                PointList->GetPoint(y*dim[1] + x));
                        //// cout <<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<"  "<<y*dim[1] + x<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace2(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoints()->SetPoint(
                                y*dim[0]*dim[1]+z,PointList->GetPoint(y*dim[0] + z));
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace3(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoints()->SetPoint(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z,
                                PointList->GetPoint(y*dim[0] + z));
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace4(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);

        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoints()->SetPoint(x*dim[0]+z,
                                PointList->GetPoint(x*dim[0] + z));
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace5(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);

        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                                GetItemAsObject(CellNum)))->GetPoints()->SetPoint((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z,
                                PointList->GetPoint(x*dim[0] + z));
                }
        }
}

void vtkMimxGenerateHexahedronMesh::MergeStructuredGridsToUnstructuredGrid(vtkPolyData *polydata)
{
//      int numele = 0;
//      int numnodes = 0;
//      int dim[3];
//      //      calculate number of nodes and elements
//      for(int i=0; i <this->StructuredGridCollection->GetNumberOfItems(); i++)
//      {
//              ((vtkStructuredGrid*)(this->StructuredGridCollection->
//                      GetItemAsObject(i)))->GetDimensions(dim);
//              numnodes = numnodes + dim[0]*dim[1]*dim[2];
//              numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
//      }
//
//      vtkMergeCells* mergecells = vtkMergeCells::New();
//      mergecells->SetUnstructuredGrid(this->UGrid);
//      mergecells->MergeDuplicatePointsOn();
//      mergecells->SetTotalNumberOfDataSets(this->StructuredGridCollection->GetNumberOfItems());
//      mergecells->SetTotalNumberOfCells(numele);
//      mergecells->SetTotalNumberOfPoints(numnodes);
//      for(int i=0; i <this->StructuredGridCollection->GetNumberOfItems(); i++)
//      {
//              mergecells->MergeDataSet(((vtkStructuredGrid*)(this->StructuredGridCollection->
//                      GetItemAsObject(i))));
//      }
//      //vtkIdType max_val = 0;
//      //for(int i=0; i < this->UGrid->GetNumberOfCells(); i++)
//      //{
//      //      cell = this->UGrid->GetCell(i);
//      //      vtkIdList* idlist = cell->GetPointIds();
//
//      //      for(int j=0; j <8; j++)
//      //      {
//      //              vtkIdType val = idlist->GetId(j);
//      //              if(val > max_val)       max_val = val;
//      //      }
//      //}
//      //this->UGrid->GetPoints()->Squeeze();
//      //this->UGrid->GetPoints()->SetNumberOfPoints(max_val+1);
//      mergecells->Finish();
//      this->Mapper->SetInput(this->UGrid);
//      this->Actor->SetMapper(this->Mapper);
//      this->UGrid->Modified();
//      vtkUnstructuredGridWriter* writer = vtkUnstructuredGridWriter::New();
//      writer->SetInput(this->UGrid);
//      writer->SetFileName("debug.txt");
//      writer->Write();
///*    vtkUnstructuredGridToPolyDataFilter* ugridtopoly = 
//              vtkAlgorithm::New();
//      ugridtopoly->SetInput(this->UGrid);
////    ugridtopoly->Update();
//      vtkPolyDataWriter* polywriter =  vtkPolyDataWriter::New();
//      polywriter->SetInput(ugridtopoly->GetOutput());
//      polywriter->SetFileName("poly.vtk");
//      polywriter->Write();*/
        // merge points belonging to different data sets and not 
        // belonging to the same data-set
        // insert the first data set
        // estimate the size of the unstructured grid

        int i, j, k, dim[3], m;
        int numele = 0;
        int numnode = 0;
        for(i=0; i <this->StructuredGridCollection->GetNumberOfItems(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(i)))->GetDimensions(dim);
                numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
                numnode = numnode + dim[0]*dim[1]*dim[2];
        }



        // start with the other bounding boxes
        // check if a dataset contains coincident points
        vtkPointLocator *globallocator = vtkPointLocator::New();
        vtkPoints *globalpoints = vtkPoints::New();
        globalpoints->Allocate(numnode);
        globallocator->InitPointInsertion(globalpoints, polydata->GetPoints()->GetBounds());

        int startnodenum;
        for(m=0; m < this->GetInput()->GetNumberOfCells(); m++)

        {
                vtkPoints* sgridpoints = ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(m)))->GetPoints();

                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(m)))->GetDimensions(dim);

                // create the global point id list for the individual points in the data set
                // and insert unique points belonging to different data sets
                vtkIdList *poirenum = vtkIdList::New();
                startnodenum = globalpoints->GetNumberOfPoints();

                for (i=0; i < sgridpoints->GetNumberOfPoints(); i++)
                {
                        // if point does not exist
                        if(globallocator->IsInsertedPoint(sgridpoints->GetPoint(i)) == -1)
                        {
                                poirenum->InsertNextId(globalpoints->GetNumberOfPoints());
                                globallocator->InsertNextPoint(sgridpoints->GetPoint(i));               
                        }
                        else
                        {
                                // if the point belongs to different data set
                                if(globallocator->IsInsertedPoint(sgridpoints->GetPoint(i)) < startnodenum)
                                {
                                        poirenum->InsertNextId(globallocator->IsInsertedPoint(sgridpoints->GetPoint(i)));
                                }
                                else{
                                        poirenum->InsertNextId(globalpoints->GetNumberOfPoints());
                                        globallocator->InsertNextPoint(sgridpoints->GetPoint(i));               
                                }
                        }
                }
                for (i=0; i < poirenum->GetNumberOfIds(); i++)
                {
                        cout <<i<<"  "<<poirenum->GetId(i)<<endl;
                }
                                
                // insert the element connectivity based on global point ids
                vtkIdList *ptids = vtkIdList::New();
                for (k=0; k<dim[2]-1; k++)
                {
                        for (j=0; j<dim[1]-1; j++)
                        {
                                for (i=0; i<dim[0]-1; i++)
                                {
                                        ptids->Initialize();
                                        ptids->SetNumberOfIds(8);
                                        ptids->SetId(0, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i));
                                        ptids->SetId(1, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i+1));
                                        ptids->SetId(2, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1));
                                        ptids->SetId(3, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]));
                                        ptids->SetId(4, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]*dim[1]));
                                        ptids->SetId(5, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i+1 + dim[0]*dim[1]));
                                        ptids->SetId(6, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1 + dim[0]*dim[1]));
                                        ptids->SetId(7, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0] + dim[0]*dim[1]));
                                        this->UGrid->InsertNextCell(12, ptids); 
                                        cout << ptids->GetId(0)<<"  "<< ptids->GetId(1)<<"  "<< ptids->GetId(2)<<"  "<< ptids->GetId(3)<<"  "<< ptids->GetId(4)<<"  "<< ptids->GetId(5)<<"  "<< ptids->GetId(6)<<"  "<< ptids->GetId(7)<<endl;
                                }
                        }
                }
        poirenum->Delete();

        }
        this->UGrid->SetPoints(globalpoints);
        this->UGrid->Squeeze();
        globalpoints->Delete();
        globallocator->Delete();
        this->Mapper->SetInput(this->UGrid);
        this->Actor->SetMapper(this->Mapper);
        this->UGrid->Modified();
        //vtkUnstructuredGridWriter* writer = vtkUnstructuredGridWriter::New();
        //writer->SetInput(this->UGrid);
        //writer->SetFileName("debug.vtk");
        //writer->Write();
}

void vtkMimxGenerateHexahedronMesh::SetMeshSeedsFileName(const char *filename)
{
        ifstream FileInput;
        FileInput.open(filename, std::ios::in);
        int cell_count = 0;
        int meshseed[3];
        this->MeshSeedX->SetNumberOfValues(this->GetInput()->GetNumberOfCells());
        this->MeshSeedY->SetNumberOfValues(this->GetInput()->GetNumberOfCells());
        this->MeshSeedZ->SetNumberOfValues(this->GetInput()->GetNumberOfCells());
        int count = 0;
        do {
                FileInput >> meshseed[0] >> meshseed[1] >> meshseed[2];
                this->MeshSeedX->SetValue(count,meshseed[0]);
                this->MeshSeedY->SetValue(count,meshseed[1]);
                this->MeshSeedZ->SetValue(count,meshseed[2]);
                count++;
        } while(!FileInput.eof());
        
}

void vtkMimxGenerateHexahedronMesh::SetMeshSeed(int cellnum, int X, int Y, int Z)
{
        this->MeshSeedX->SetValue(cellnum,X);
        this->MeshSeedY->SetValue(cellnum,Y);
        this->MeshSeedZ->SetValue(cellnum,Z);
}

void vtkMimxGenerateHexahedronMesh::GetMeshSeed(int cellnum,
                                                                                        int X, int Y, int Z)
{
        X = this->MeshSeedX->GetValue(cellnum);
        Y = this->MeshSeedY->GetValue(cellnum);
        Z = this->MeshSeedZ->GetValue(cellnum);
}

void vtkMimxGenerateHexahedronMesh::BuildMeshSeedLinks()
{
        this->Links = 0;
        this->MeshSeedLinks.SetSize(this->GetInput()->GetNumberOfCells()*3,
                this->GetInput()->GetNumberOfCells());
        this->MeshSeedLinks.Set(0);
        this->MeshSeedCheck.SetSize(this->GetInput()->GetNumberOfCells(), 1);
        vtkIdList *neigh_cell_list = vtkIdList::New();

        CLinkedList<Node*> *meshseedtree =  new CLinkedList<Node*>;

        // starting point for the tree to be constructed
        for (int i=0; i<this->GetInput()->GetNumberOfCells(); i++)
        {
                for (int j=1; j<=3; j++)
                {
                        this->MeshSeedCheck.Set(0);
                        // starting point for the tree to be constructed
                        this->MeshSeedLinks(3*i+j,i+1) = j;
                        meshseedtree->Add(new Node);
                        meshseedtree->GetLastItem()->Parent = NULL;
                        meshseedtree->GetLastItem()->CellNum = i;
                        meshseedtree->GetLastItem()->EdgeNum = j;
                        // get only the neighboring cells that have not already been traversed
                        // first generate the tree structure checking for the edges being shared
                        vtkIdType cellnum = i;
                        this->MeshSeedCheck(i+1,1) = j;
                        neigh_cell_list->Initialize();
                        this->GetCellNeighbors(cellnum, neigh_cell_list);
                        Node *currpar = meshseedtree->GetLastItem();
                        currpar->CellNum = cellnum;
                        currpar->EdgeNum = j;
                        currpar->PeerPrev = NULL;
                        currpar->PeerNext = NULL;
                        currpar->Parent = NULL;
                        currpar->Child = NULL;
                        Node *currchild;
                        Node *currparpar = currpar;
                        Node *currpeerprev;
                        if(neigh_cell_list->GetNumberOfIds() !=0)
                        {
                                while (currparpar != NULL)
                                {
                                        int k;
                                        k=-1;
                                        int count = 0;
                                        do 
                                        {
                                                k++;
                                                if(currpar->EdgeNum == 1 )
                                                {this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeX(currpar->CellNum,neigh_cell_list->GetId(k),i);}
                                                else
                                                {if(currpar->EdgeNum == 2){this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeY(currpar->CellNum,neigh_cell_list->GetId(k),i);}
                                                else{this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeZ(currpar->CellNum,neigh_cell_list->GetId(k),i);}}
                                                if (this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1))
                                                {
                                                        if (!count)
                                                        {
                                                                currpar->Child = new Node;
                                                                meshseedtree->Add(currpar->Child);
                                                                currchild = currpar->Child;
                                                                currchild->CellNum = neigh_cell_list->GetId(k);
                                                                currchild->EdgeNum = this->MeshSeedLinks(3*i+j,neigh_cell_list->GetId(k)+1);
                                                                currchild->PeerPrev = NULL;
                                                                currchild->Parent = currpar;
                                                                count ++;
                                                        }
                                                        else{
                                                                currchild->PeerNext = new Node; 
                                                                meshseedtree->Add(currchild->PeerNext);
                                                                currpeerprev = currchild;
                                                                currchild = currchild->PeerNext;
                                                                currchild->PeerPrev = currpeerprev;
                                                                currchild->Parent = currpar;
                                                                currchild->PeerNext = NULL;
                                                                currchild->CellNum = neigh_cell_list->GetId(k);
                                                                currchild->EdgeNum = this->MeshSeedLinks(3*i+j,neigh_cell_list->GetId(k)+1);
                                                                count++;
                                                        }
                                                }
                                        } while(k < neigh_cell_list->GetNumberOfIds()-1);                                               

        //                              currparpar = currchild->Parent;
                                        if (!count)
                                        {
                                                if(currpar->PeerPrev !=NULL)    currpar = currpar->PeerPrev;
                                                else    
                                                {
                                                        if(currpar->Parent != NULL)currpar = currpar->Parent;
                                                }
                                        }
                                        else
                                        {
                                                currpar = currchild;
                                        }
                                        if (currpar->Parent != NULL)
                                        {
                                                this->GetCellNeighbors(currpar->CellNum, neigh_cell_list);
                                                if(neigh_cell_list->GetNumberOfIds() == 0)
                                                {
                                                        do 
                                                        {
                                                                if(currpar->PeerPrev == NULL)
                                                                {
                                                                        currpar = currpar->Parent;
                                                                }
                                                                else
                                                                {
                                                                        currpar = currpar->PeerPrev;
                                                                        this->GetCellNeighbors(currpar->CellNum, neigh_cell_list);                                              
                                                                }
                                                        } while(neigh_cell_list->GetNumberOfIds() == 0 && currpar->Parent != NULL);
                                                }
                                        }
                                        currparpar = currpar->Parent;
                                }
                        }
                        meshseedtree->Delete();
                        for (int k=0; k < this->GetInput()->GetNumberOfCells(); k++)
                        {
                                // cout << this->MeshSeedLinks(3*i +j, k+1)<<"  ";
                        }
                        // cout<<endl;
                }
        }
        neigh_cell_list->Delete();
        delete meshseedtree;
        //for(int i=0; i < this->GetInput()->GetNumberOfCells(); i++)
        //{
        //      this->GetCellNeighbors(i,neigh_cell_list);
        //      for(int j=0; j < neigh_cell_list->GetNumberOfIds(); j++)
        //      {
        //              status1 = 0;    status2 = 0;
        //              //if(i < neigh_cell_list->GetId(j))
        //              //{
        //                      if(!this->MeshSeedCheck(i+1, neigh_cell_list->GetId(j)+1))
        //                      {
        //                              status1 = this->CheckIfCellsShareEdgeX(i, neigh_cell_list->GetId(j));
        //                              status2 = this->CheckIfCellsShareEdgeY(i, neigh_cell_list->GetId(j));
        //                              if(!status1 || !status2)
        //                              {this->CheckIfCellsShareEdgeZ(i, neigh_cell_list->GetId(j));}
        //                      }
        ////            }
        //              this->MeshSeedCheck(i+1, neigh_cell_list->GetId(j)+1) = 1 ;
        //      }
        //      neigh_cell_list->Initialize();
        //}

        ////    mesh seeds values could propagate even to the elements
        //// not the immediate neighbor
        //// setting the MeshSeedCheck values for those affected to 1
        ////int prevrow, prevcol;
        ////for(int i=0; i <3*this->GetInput()->GetNumberOfCells(); i++)
        ////{
        ////    for(int j=i+1; j <3*this->GetInput()->GetNumberOfCells(); j++)
        ////    {
        ////            prevrow = i+1; prevcol = j+1;
        ////            do {
        ////                                    
        ////            } while();
        ////    }
        ////}

        /*for(int i=0; i <3*this->GetInput()->GetNumberOfCells(); i++)
        {
                for(int j=0; j<3*this->GetInput()->GetNumberOfCells(); j++)
                {
                        if(this->MeshSeedLinks(j+1,i+1) = 0)
                        {
                                this->MeshSeedLinks(j+1,i+1) = this->MeshSeedLinks(i+1,j+1);
                        }
                        else
                        {
                                this->MeshSeedLinks(i+1,j+1) = this->MeshSeedLinks(j+1,i+1);
                        }
                }
        }*/

        //neigh_cell_list->Delete();
        /*for(int i=0; i <3*this->GetInput()->GetNumberOfCells(); i++)
        {
                for(int j=0; j<3*this->GetInput()->GetNumberOfCells(); j++)
                {
                         cout <<this->MeshSeedLinks(i+1,j+1)<<"  ";
                }
                 cout <<endl;
        }*/
}

int vtkMimxGenerateHexahedronMesh::CheckIfCellsShareEdgeZ(int cellnum1, int cellnum2, int parcell)
{
        bool status1 = false;
        bool status2 = false;
        vtkGenericCell *cell = vtkGenericCell::New();
        this->GetInput()->GetCell(cellnum1, cell);
        vtkGenericCell *cell_comp = vtkGenericCell::New();
        this->GetInput()->GetCell(cellnum2, cell_comp);
        vtkIdType pt1, pt2;
        // start with z axis for 0 and 1
        int k;
        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(0))
                {
                        status1 = true;
                        break;
                }
        }
        pt1 = k;

        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(1))
                {
                        status2 = true;
                        break;
                }
        }       
        pt2 = k;

        if(status1 && status2)
        {               
                
                this->MeshSeedLinks(3*parcell+3, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                cell->Delete();
                cell_comp->Delete();
                return 1;
        }
        else
        {
                // 2 and 3
                status1 = false; status2 = false;
                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(3))
                        {
                                status1 = true;
                                break;
                        }
                }
                pt1 = k;

                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(2))
                        {
                                status2 = true;
                                break;
                        }
                }
                pt2 = k;

                if(status1 && status2)
                {
                        
                        this->MeshSeedLinks(3*parcell+3, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                        cell->Delete();
                        cell_comp->Delete();
                        return 1;
                }
                else
                {
                        // 4 and 5
                        status1 = false; status2 = false;
                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(4))
                                {
                                        status1 = true;
                                        break;
                                }
                        }
                        pt1 = k;

                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(5))
                                {
                                        status2 = true;
                                        break;
                                }
                        }
                        pt2 = k;

                        if(status1 && status2)
                        {
                                
                                this->MeshSeedLinks(3*parcell+3, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                cell->Delete();
                                cell_comp->Delete();
                                return 1;
                        }
                        else
                        {
                                // 6 and 7
                                status1 = false; status2 = false;
                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(6))
                                        {
                                                status1 = true;
                                                break;
                                        }
                                }
                                pt1 = k;

                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(7))
                                        {
                                                status2 = true;
                                                break;
                                        }
                                }
                                pt2 = k;

                                if(status1 && status2)
                                {
                                        
                                        this->MeshSeedLinks(3*parcell+3, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                        cell->Delete();
                                        cell_comp->Delete();
                                        return 1;
                                }
                        }
                }
        }
        cell->Delete();
        cell_comp->Delete();
        return 0;
}

int vtkMimxGenerateHexahedronMesh::CheckIfCellsShareEdgeX(int cellnum1, int cellnum2, int parcell)
{
        bool status1 = false;
        bool status2 = false;
        vtkGenericCell *cell = vtkGenericCell::New();
        this->GetInput()->GetCell(cellnum1, cell);
        vtkGenericCell *cell_comp = vtkGenericCell::New();
        this->GetInput()->GetCell(cellnum2, cell_comp);
        vtkIdType pt1, pt2;
        // start with x axis for 0 and 3
        int k;
        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(0))
                {
                        status1 = true;
                        break;
                }
        }
        pt1 = k;

        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(3))
                {
                        status2 = true;
                        break;
                }
        }       
        pt2 = k;

        if(status1 && status2)
        {               
                this->MeshSeedLinks(3*parcell+1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                cell->Delete();
                cell_comp->Delete();

                return 1;
        }
        else
        {
                // 1 and 2
                status1 = false; status2 = false;
                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(1))
                        {
                                status1 = true;
                                break;
                        }
                }
                pt1 = k;

                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(2))
                        {
                                status2 = true;
                                break;
                        }
                }
                pt2 = k;

                if(status1 && status2)
                {
                        
                        this->MeshSeedLinks(3*parcell+1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                        cell->Delete();
                        cell_comp->Delete();
                        return 1;
                }
                else
                {
                        // 4 and 7
                        status1 = false; status2 = false;
                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(4))
                                {
                                        status1 = true;
                                        break;
                                }
                        }
                        pt1 = k;

                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(7))
                                {
                                        status2 = true;
                                        break;
                                }
                        }
                        pt2 = k;

                        if(status1 && status2)
                        {
                                
                                this->MeshSeedLinks(3*parcell+1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                cell->Delete();
                                cell_comp->Delete();
                                return 1;
                        }
                        else
                        {
                                // 5 and 6
                                status1 = false; status2 = false;
                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(6))
                                        {
                                                status1 = true;
                                                break;
                                        }
                                }
                                pt1 = k;

                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(5))
                                        {
                                                status2 = true;
                                                break;
                                        }
                                }
                                pt2 = k;

                                if(status1 && status2)
                                {
                                        
                                        this->MeshSeedLinks(3*parcell+1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                        cell->Delete();
                                        cell_comp->Delete();
                                        return 1;
                                }
                        }
                }
        }
        cell->Delete();
        cell_comp->Delete();
        return 0;
}

int vtkMimxGenerateHexahedronMesh::CheckIfCellsShareEdgeY(int cellnum1, int cellnum2, int parcell)
{
        bool status1 = false;
        bool status2 = false;
        vtkGenericCell *cell = vtkGenericCell::New();
        this->GetInput()->GetCell(cellnum1, cell);
        vtkGenericCell *cell_comp = vtkGenericCell::New();
        this->GetInput()->GetCell(cellnum2, cell_comp);
        vtkIdType pt1, pt2;
        // start with x axis for 0 and 4
        int k;
        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(0))
                {
                        status1 = true;
                        break;
                }
        }
        pt1 = k;

        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(4))
                {
                        status2 = true;
                        break;
                }
        }       
        pt2 = k;

        if(status1 && status2)
        {               
                
                this->MeshSeedLinks(3*parcell+2, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                cell->Delete();
                cell_comp->Delete();
                return 1;
        }
        else
        {
                // 1 and 5
                status1 = false; status2 = false;
                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(1))
                        {
                                status1 = true;
                                break;
                        }
                }
                pt1 = k;

                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(5))
                        {
                                status2 = true;
                                break;
                        }
                }
                pt2 = k;

                if(status1 && status2)
                {
                        
                        this->MeshSeedLinks(3*parcell+2, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                        cell->Delete();
                        cell_comp->Delete();
                        return 1;
                }
                else
                {
                        // 2 and 6
                        status1 = false; status2 = false;
                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(2))
                                {
                                        status1 = true;
                                        break;
                                }
                        }
                        pt1 = k;

                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(6))
                                {
                                        status2 = true;
                                        break;
                                }
                        }
                        pt2 = k;

                        if(status1 && status2)
                        {
                                
                                this->MeshSeedLinks(3*parcell+2, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                cell->Delete();
                                cell_comp->Delete();
                                return 1;
                        }
                        else
                        {
                                // 3 and 7
                                status1 = false; status2 = false;
                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(3))
                                        {
                                                status1 = true;
                                                break;
                                        }
                                }
                                pt1 = k;

                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(7))
                                        {
                                                status2 = true;
                                                break;
                                        }
                                }
                                pt2 = k;

                                if(status1 && status2)
                                {
                                        
                                        this->MeshSeedLinks(3*parcell+2, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                        cell->Delete();
                                        cell_comp->Delete();
                                        return 1;
                                }
                        }
                }
        }
        cell->Delete();
        cell_comp->Delete();

        return 0;
}


void vtkMimxGenerateHexahedronMesh::GetCellNeighbors(vtkIdType cellnum,
                                                                                                 vtkIdList *cellids)
{
        cellids->Initialize();
        vtkIdList *ptids = vtkIdList::New();
        vtkIdList *ptids_temp = vtkIdList::New();
        vtkIdList *cellids_temp = vtkIdList::New();
        this->GetInput()->GetCellPoints(cellnum,ptids);
        for(int i=0; i < ptids->GetNumberOfIds(); i++)
        {
//              ptids_temp->Initialize();
                ptids_temp->SetNumberOfIds(1);
                ptids_temp->SetId(0,ptids->GetId(i));
                cellids_temp->Initialize();
                this->GetInput()->GetCellNeighbors(cellnum,ptids_temp, cellids_temp);
                for(int j=0; j<cellids_temp->GetNumberOfIds(); j++)
                {
                        if(!this->MeshSeedCheck(cellids_temp->GetId(j)+1, 1))
                        {
                                cellids->InsertUniqueId(cellids_temp->GetId(j));
                        }
                }
        }
        
        vtkIdType pt;
        for (int i=0; i<cellids->GetNumberOfIds(); i++)
        {
                pt = cellids->GetId(i);
        }
        ptids->Delete();
        ptids_temp->Delete();
        cellids_temp->Delete();
}

int vtkMimxGenerateHexahedronMesh::WhichEdgeOfCellBeingCompared(vtkIdType vertex1, vtkIdType vertex2)
{
        // check if edge is X (0-3)
        if(vertex1 == 0 || vertex1 == 3)
        {
                if(vertex2 == 0 || vertex2 == 3)        return 1;
        }
        // check if edge is X (1-2)
        if(vertex1 == 1 || vertex1 == 2)
        {
                if(vertex2 == 1 || vertex2 == 2)        return 1;
        }
        // check if edge is X (4-7)
        if(vertex1 == 4 || vertex1 == 7)
        {
                if(vertex2 == 4 || vertex2 == 7)        return 1;
        }
        // check if edge is X (5-6)
        if(vertex1 == 5 || vertex1 == 6)
        {
                if(vertex2 == 5 || vertex2 == 6)        return 1;
        }
        // check if edge is Z (0-1)
        if(vertex1 == 0 || vertex1 == 1)
        {
                if(vertex2 == 0 || vertex2 == 1)        return 3;
        }
        // check if edge is Z (2-3)
        if(vertex1 == 2 || vertex1 == 3)
        {
                if(vertex2 == 2 || vertex2 == 3)        return 3;
        }
        // check if edge is Z (4-5)
        if(vertex1 == 4 || vertex1 == 5)
        {
                if(vertex2 == 4 || vertex2 == 5)        return 3;
        }
        // check if edge is Z (6-7)
        if(vertex1 == 6 || vertex1 == 7)
        {
                if(vertex2 == 6 || vertex2 == 7)        return 3;
        }
        // check if edge is Y (0-4)
        if(vertex1 == 0 || vertex1 == 4)
        {
                if(vertex2 == 0 || vertex2 == 4)        return 2;
        }
        // check if edge is Y (1-5)
        if(vertex1 == 1 || vertex1 == 5)
        {
                if(vertex2 == 1 || vertex2 == 5)        return 2;
        }
        // check if edge is Y (2-6)
        if(vertex1 == 2 || vertex1 == 6)
        {
                if(vertex2 == 2 || vertex2 == 6)        return 2;
        }
        // check if edge is Y (3-7)
        if(vertex1 == 3 || vertex1 == 7)
        {
                if(vertex2 == 3 || vertex2 == 7)        return 2;
        }
        return 0;
}

void vtkMimxGenerateHexahedronMesh::EstimateMeshSeedsBasedOnAverageElementLength(double length)
{
        if(!this->Links)        
        {
                this->BuildMeshSeedLinks();
        }

        if(this->GetInput())
        {
                this->MeshSeedX->SetNumberOfValues(this->GetInput()->GetNumberOfCells());
                this->MeshSeedY->SetNumberOfValues(this->GetInput()->GetNumberOfCells());
                this->MeshSeedZ->SetNumberOfValues(this->GetInput()->GetNumberOfCells());
                for(int i=0; i < this->GetInput()->GetNumberOfCells(); i++)
                {
                        this->MeshSeedX->SetValue(i,0);
                        this->MeshSeedY->SetValue(i,0);
                        this->MeshSeedZ->SetValue(i,0);
                }


                for(int i =0; i < this->GetInput()->GetNumberOfCells(); i++)
                {
                        vtkGenericCell *cell = vtkGenericCell::New();
                        this->GetInput()->GetCell(i, cell);
                        // calculate the average length of X edges,
                        // 0-3, 1-2, 4-7, 5-6
                        if(!this->MeshSeedX->GetValue(i))// if the X meshseed is not precalculated
                        {
                                double lengthX = 0.0;
                                double a[3], b[3];
                                this->GetInput()->GetPoint(cell->GetPointId(0),a);
                                this->GetInput()->GetPoint(cell->GetPointId(3),b);
                                lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->GetInput()->GetPoint(cell->GetPointId(1),a);
                                this->GetInput()->GetPoint(cell->GetPointId(2),b);
                                lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->GetInput()->GetPoint(cell->GetPointId(4),a);
                                this->GetInput()->GetPoint(cell->GetPointId(7),b);
                                lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->GetInput()->GetPoint(cell->GetPointId(5),a);
                                this->GetInput()->GetPoint(cell->GetPointId(6),b);
                                lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                lengthX = lengthX/4.0;
                                this->MeshSeedX->SetValue(i,int(lengthX/length)+1);
                        }
                        this->SetHigherNumberedBBoxMeshSeed(i,1);
                        // calculate the average length of X edges,
                        // 0-4, 1-5, 2-6, 3-7
                        if(!this->MeshSeedY->GetValue(i))// if the X meshseed is not precalculated
                        {
                                double lengthY = 0.0;
                                double a[3], b[3];
                                this->GetInput()->GetPoint(cell->GetPointId(0),a);
                                this->GetInput()->GetPoint(cell->GetPointId(4),b);
                                lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->GetInput()->GetPoint(cell->GetPointId(1),a);
                                this->GetInput()->GetPoint(cell->GetPointId(5),b);
                                lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->GetInput()->GetPoint(cell->GetPointId(2),a);
                                this->GetInput()->GetPoint(cell->GetPointId(6),b);
                                lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->GetInput()->GetPoint(cell->GetPointId(3),a);
                                this->GetInput()->GetPoint(cell->GetPointId(7),b);
                                lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                lengthY = lengthY/4.0;
                                this->MeshSeedY->SetValue(i,int(lengthY/length)+1);
                        }
                        this->SetHigherNumberedBBoxMeshSeed(i,2);
                        // calculate the average length of Z edges,
                        // 0-1, 2-3, 4-5, 6-7
                        if(!this->MeshSeedZ->GetValue(i))// if the X meshseed is not precalculated
                        {
                                double lengthZ = 0.0;
                                double a[3], b[3];
                                this->GetInput()->GetPoint(cell->GetPointId(0),a);
                                this->GetInput()->GetPoint(cell->GetPointId(1),b);
                                lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->GetInput()->GetPoint(cell->GetPointId(2),a);
                                this->GetInput()->GetPoint(cell->GetPointId(3),b);
                                lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->GetInput()->GetPoint(cell->GetPointId(4),a);
                                this->GetInput()->GetPoint(cell->GetPointId(5),b);
                                lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->GetInput()->GetPoint(cell->GetPointId(6),a);
                                this->GetInput()->GetPoint(cell->GetPointId(7),b);
                                lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                lengthZ = lengthZ/4.0;
                                this->MeshSeedZ->SetValue(i,int(lengthZ/length)+1);
                        }
                        this->SetHigherNumberedBBoxMeshSeed(i,3);
                        cell->Delete();
                }
                for(int i=0; i<this->GetInput()->GetNumberOfCells(); i++)
                {
                         cout <<this->MeshSeedX->GetValue(i)<<"  "
                                <<this->MeshSeedY->GetValue(i)<<"  "
                                <<this->MeshSeedZ->GetValue(i)<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetHigherNumberedBBoxMeshSeed(int boxnum, int axis)
{
        for(int i = boxnum+1; i < this->GetInput()->GetNumberOfCells(); i++)
        {
                int j = this->MeshSeedLinks(boxnum*3+axis,i+1);
                if(j)
                {
                        if(axis == 1)
                        {
                                if(j==1)        
                                {this->MeshSeedX->SetValue(i,
                                        this->MeshSeedX->GetValue(boxnum));
                                        break;
                                }
                                if(j==2)        
                                {this->MeshSeedY->SetValue(i,
                                        this->MeshSeedX->GetValue(boxnum));
                                        break;
                                }
                                if(j==3)        
                                {this->MeshSeedZ->SetValue(i,
                                        this->MeshSeedX->GetValue(boxnum));
                                        break;
                                }
                                break;
                        }
                        if(axis == 2)
                        {
                                if(j==1)        
                                {this->MeshSeedX->SetValue(i,
                                        this->MeshSeedY->GetValue(boxnum));
                                        break;
                                }
                                if(j==2)        
                                {this->MeshSeedY->SetValue(i,
                                        this->MeshSeedY->GetValue(boxnum));
                                        break;
                                }
                                if(j==3)        
                                {this->MeshSeedZ->SetValue(i,
                                this->MeshSeedY->GetValue(boxnum));
                                        break;
                                }       
                                break;
                        }
                        if(axis == 3)
                        {
                                if(j==1)        
                                {this->MeshSeedX->SetValue(i,
                                        this->MeshSeedZ->GetValue(boxnum));
                                        break;
                                }
                                if(j==2)        
                                {this->MeshSeedY->SetValue(i,
                                        this->MeshSeedZ->GetValue(boxnum));
                                        break;
                                }
                                if(j==3)        
                                {this->MeshSeedZ->SetValue(i,
                                        this->MeshSeedZ->GetValue(boxnum));
                                        break;
                                }       
                                break;
                        }
                }
        }
}

int vtkMimxGenerateHexahedronMesh::CheckIfEdgeNodesRecalculated(int EdgeNum, vtkIdList *IdList)
{
        if(EdgeNum == 0)
        {
                if(!IdList->GetId(1) && !IdList->GetId(3) && !IdList->GetId(9) && !IdList->GetId(8))    return 0;
                else    return 1;
        }
        if(EdgeNum == 1)
        {
                if(!IdList->GetId(0) && !IdList->GetId(2) && !IdList->GetId(9) && !IdList->GetId(11))   return 0;
                else    return 1;
        }
        if(EdgeNum == 2)
        {
                if(!IdList->GetId(1) && !IdList->GetId(3)&& !IdList->GetId(11) && !IdList->GetId(10))   return 0;
                else    return 1;
        }
        if(EdgeNum == 3)
        {
                if(!IdList->GetId(0) && !IdList->GetId(2) && !IdList->GetId(8) && !IdList->GetId(10))   return 0;
                else    return 1;
        }
        if(EdgeNum == 4)
        {
                if(!IdList->GetId(5) && !IdList->GetId(7) && !IdList->GetId(8) && !IdList->GetId(9))    return 0;
                else    return 1;
        }
        if(EdgeNum == 5)
        {
                if(!IdList->GetId(4) && !IdList->GetId(6) && !IdList->GetId(9) && !IdList->GetId(11))   return 0;
                else    return 1;
        }
        if(EdgeNum == 6)
        {
                if(!IdList->GetId(5) && !IdList->GetId(7) && !IdList->GetId(11) && !IdList->GetId(10))  return 0;
                else    return 1;
        }
        if(EdgeNum == 7)
        {
                if(!IdList->GetId(4) && !IdList->GetId(6) && !IdList->GetId(8) && !IdList->GetId(10))   return 0;
                else    return 1;
        }
        if(EdgeNum == 8)
        {
                if(!IdList->GetId(3) && !IdList->GetId(7) && !IdList->GetId(0) && !IdList->GetId(4))    return 0;
                else    return 1;
        }
        if(EdgeNum == 9)
        {
                if(!IdList->GetId(1) && !IdList->GetId(5) && !IdList->GetId(0) && !IdList->GetId(4))    return 0;
                else    return 1;
        }
        if(EdgeNum == 10)
        {
                if(!IdList->GetId(3) && !IdList->GetId(7) && !IdList->GetId(2) && !IdList->GetId(6))    return 0;
                else    return 1;
        }
        if(EdgeNum == 11)
        {
                if(!IdList->GetId(5) && !IdList->GetId(1) && !IdList->GetId(2) && !IdList->GetId(6))    return 0;
                else    return 1;
        }
        return 0;
}

void vtkMimxGenerateHexahedronMesh::ClosestPointProjection(vtkCellLocator *Locator, vtkPoints *Points)
{
        double dist,ClosestPoint[3],x[3];
        vtkIdType CellId;
        int SubId;
        for(int i=0; i<Points->GetNumberOfPoints(); i++)
        {
                Points->GetPoint(i,x);
                Locator->FindClosestPoint(x,ClosestPoint,CellId,SubId,dist);
                Points->SetPoint(i,ClosestPoint);
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge(int CellNum, int EdgeNum, vtkPoints *PointList)
{
        switch (EdgeNum) {
                case 0:
                        GetEdge0(CellNum, PointList);
                        break;
                case 1:
                        GetEdge1(CellNum, PointList);
                        break;
                case 2:
                        GetEdge2(CellNum, PointList);
                        break;
                case 3:
                        GetEdge3(CellNum, PointList);
                        break;
                case 4:
                        GetEdge4(CellNum, PointList);
                        break;
                case 5:
                        GetEdge5(CellNum, PointList);
                        break;
                case 6:
                        GetEdge6(CellNum, PointList);
                        break;
                case 7:
                        GetEdge7(CellNum, PointList);
                        break;
                case 8:
                        GetEdge8(CellNum, PointList);
                        break;
                case 9:
                        GetEdge9(CellNum, PointList);
                        break;
                case 10:
                        GetEdge10(CellNum, PointList);
                        break;
                case 11:
                        GetEdge11(CellNum, PointList);
                        break;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge0(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i < dim[0]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge1(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*(i+1)-1));
                // cout <<dim[0]*(i+1)-1<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge2(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i< dim[0]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*(dim[1]-1)+i));
                // cout <<dim[0]*(dim[1]-1)+i<<endl;

        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge3(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(i*dim[0]));
                // cout <<i*dim[0]<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge4(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i < dim[0]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*dim[1]*(dim[2]-1) + i));
                // cout<<dim[0]*dim[1]*(dim[2]-1) + i<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge5(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(i+1)-1));
                // cout <<dim[0]*dim[1]*(dim[2]-1) + dim[0]*(i+1)-1<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge6(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[0]);
        for(int i=0; i< dim[0]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(dim[1]-1)+i));
                // cout<<dim[0]*dim[1]*(dim[2]-1) + dim[0]*(dim[1]-1)+i<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge7(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[1]);
        for(int i=0; i< dim[1]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*dim[1]*(dim[2]-1) + i*dim[0]));
                // cout<<dim[0]*dim[1]*(dim[2]-1) + i*dim[0]<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge8(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i < dim[2]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*dim[1]*i));
                // cout<<dim[0]*dim[1]*i<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge9(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i< dim[2]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*dim[1]*i + dim[0]-1));
                // cout<<dim[0]*dim[1]*i + dim[0]-1<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge10(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i< dim[2]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*dim[1]*i + dim[0]*(dim[1]-1)));
                // cout<<dim[0]*dim[1]*i + dim[0]*(dim[1]-1)<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::GetEdge11(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointList->SetNumberOfPoints(dim[2]);
        for(int i=0; i< dim[2]; i++)
        {
                PointList->InsertPoint(i, ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoint(dim[0]*dim[1]*i + dim[0]*dim[1]-1));
                // cout<<dim[0]*dim[1]*i + dim[0]*dim[1]-1<<endl;
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge(int CellNum, int EdgeNum, vtkPoints *PointList)
{
        switch (EdgeNum) {
                case 0:
                        SetEdge0(CellNum, PointList);
                        break;
                case 1:
                        SetEdge1(CellNum, PointList);
                        break;
                case 2:
                        SetEdge2(CellNum, PointList);
                        break;
                case 3:
                        SetEdge3(CellNum, PointList);
                        break;
                case 4:
                        SetEdge4(CellNum, PointList);
                        break;
                case 5:
                        SetEdge5(CellNum, PointList);
                        break;
                case 6:
                        SetEdge6(CellNum, PointList);
                        break;
                case 7:
                        SetEdge7(CellNum, PointList);
                        break;
                case 8:
                        SetEdge8(CellNum, PointList);
                        break;
                case 9:
                        SetEdge9(CellNum, PointList);
                        break;
                case 10:
                        SetEdge10(CellNum, PointList);
                        break;
                case 11:
                        SetEdge11(CellNum, PointList);
                        break;
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge0(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(i, PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge1(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*(i+1)-1, PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge2(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*(dim[1]-1)+i, PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge3(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(i*dim[0], PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge4(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*dim[1]*(dim[2]-1) + i, PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge5(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(i+1)-1, PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge6(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*dim[1]*(dim[2]-1) + dim[0]*(dim[1]-1)+i, PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge7(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*dim[1]*(dim[2]-1) + i*dim[0], PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge8(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*dim[1]*i, PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge9(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*dim[1]*i + dim[0]-1, PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge10(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*dim[1]*i + dim[0]*(dim[1]-1), PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetEdge11(int CellNum, vtkPoints* PointList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int i=0; i < PointList->GetNumberOfPoints(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->GetItemAsObject(CellNum)))->GetPoints()->SetPoint(dim[0]*dim[1]*i + dim[0]*dim[1]-1, PointList->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::RecalculateEdge(int CellNum, int EdgeNum, vtkIdList *IdList)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        vtkPoints *Points1 = vtkPoints::New();
        vtkPoints *Points2 = vtkPoints::New();
        vtkPoints *Points3 = vtkPoints::New();

        double pt1[3], pt2[3];

        switch (EdgeNum) {
                case 0:
                        if(IdList->GetId(3))
                        {
                                this->GetEdge(CellNum, 3, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 8, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(1))
                        {
                                this->GetEdge(CellNum, 1, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 9, Points2);
                                Points2->GetPoint(0,pt2);
                        }

                        Points3->SetNumberOfPoints(dim[0]);     
                        break;

                case 1:
                        if(IdList->GetId(0))
                        {
                                this->GetEdge(CellNum, 0, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 9, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(2))
                        {
                                this->GetEdge(CellNum, 2, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 11, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[1]);     
                        break;
                case 2:
                        if(IdList->GetId(3))
                        {
                                this->GetEdge(CellNum, 3, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 10, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(1))
                        {
                                this->GetEdge(CellNum, 1, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 11, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[0]);     
                        break;
                case 3:
                        if(IdList->GetId(0))
                        {
                                this->GetEdge(CellNum, 0, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 8, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(2))
                        {
                                this->GetEdge(CellNum, 2, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 10, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[1]);     
                        break;
                case 4:
                        if(IdList->GetId(8))
                        {
                                this->GetEdge(CellNum, 8, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 7, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(9))
                        {
                                this->GetEdge(CellNum, 9, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 5, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[0]);     
                        break;
                case 5:
                        if(IdList->GetId(4))
                        {
                                this->GetEdge(CellNum, 4, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 9, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        if(IdList->GetId(11))
                        {
                                this->GetEdge(CellNum, 11, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 6, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[1]);     
                        break;

                case 6:
                        if(IdList->GetId(7))
                        {
                                this->GetEdge(CellNum, 7, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 10, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        if(IdList->GetId(11))
                        {
                                this->GetEdge(CellNum, 11, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 5, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[0]);     
                        break;
                case 7:
                        if(IdList->GetId(4))
                        {
                                this->GetEdge(CellNum, 4, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 8, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        if(IdList->GetId(10))
                        {
                                this->GetEdge(CellNum, 10, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 6, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[1]);     
                        break;
                case 8:
                        if(IdList->GetId(0))
                        {
                                this->GetEdge(CellNum, 0, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 3, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(4))
                        {
                                this->GetEdge(CellNum, 4, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 7, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[2]);
                        break;
                case 9:
                        if(IdList->GetId(0))
                        {
                                this->GetEdge(CellNum, 0, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 1, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(4))
                        {
                                this->GetEdge(CellNum, 4, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 5, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[2]);     
                        break;

                case 10:
                        if(IdList->GetId(3))
                        {
                                this->GetEdge(CellNum, 3, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 2, Points1);
                                Points1->GetPoint(0,pt1);
                        }
                        if(IdList->GetId(7))
                        {
                                this->GetEdge(CellNum, 7, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 6, Points2);
                                Points2->GetPoint(0,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[2]);     
                        break;
                case 11:
                        if(IdList->GetId(1))
                        {
                                this->GetEdge(CellNum, 1, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 2, Points1);
                                Points1->GetPoint(Points1->GetNumberOfPoints()-1,pt1);
                        }
                        if(IdList->GetId(5))
                        {
                                this->GetEdge(CellNum, 5, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        else
                        {
                                this->GetEdge(CellNum, 6, Points2);
                                Points2->GetPoint(Points2->GetNumberOfPoints()-1,pt2);
                        }
                        Points3->SetNumberOfPoints(dim[2]);
                        break;

        }
        
        for(int i=0; i <Points3->GetNumberOfPoints(); i++)
        {
                Points3->SetPoint(i, pt1[0] + ((pt2[0]-pt1[0])/(Points3->GetNumberOfPoints()-1))*i, pt1[1] + ((pt2[1]-pt1[1])/(Points3->GetNumberOfPoints()-1))*i, pt1[2] + ((pt2[2]-pt1[2])/(Points3->GetNumberOfPoints()-1))*i);
        }
        this->SetEdge(CellNum, EdgeNum, Points3);
        Points1->Delete();
        Points2->Delete();
        Points3->Delete();
}

int vtkMimxGenerateHexahedronMesh::CheckBoundaryEdge(vtkIdList* IdList, vtkPolyData *BoundaryData)
{
        vtkIdList *idlist2;
        vtkIdType pt1,pt2;
        pt1 = IdList->GetId(0); pt2 = IdList->GetId(1);
        for(int i=0; i < BoundaryData->GetNumberOfCells(); i++)
        {
                idlist2 = BoundaryData->GetCell(i)->GetPointIds();
                vtkIdType p1,p2,p3,p4;
                p1 = idlist2->GetId(0); p2 = idlist2->GetId(1);
                p3 = idlist2->GetId(2); p4 = idlist2->GetId(3);
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

void vtkMimxGenerateHexahedronMesh::GetCellPoints(int CellNum, vtkPoints* Points, vtkIdList *IdList)
{
        int dim[3],i, j, k, startelenum = 0;
        // calculate starting element number from which points need to be acquired
        for(i=0; i < CellNum; i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(i)))->GetDimensions(dim);
                startelenum += (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
        }
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
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
                                this->UGrid->GetCellPoints(elenum, npts, pts);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i, this->UGrid->GetPoint(pts[0]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i, pts[0]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i+1, this->UGrid->GetPoint(pts[1]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i+1, pts[1]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1, this->UGrid->GetPoint(pts[2]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1, pts[2]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0], this->UGrid->GetPoint(pts[3]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0], pts[3]);

                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]*dim[1], this->UGrid->GetPoint(pts[4]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]*dim[1], pts[4]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i+1 + dim[0]*dim[1], this->UGrid->GetPoint(pts[5]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i+1 + dim[0]*dim[1], pts[5]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1 + dim[0]*dim[1], this->UGrid->GetPoint(pts[6]));
                                IdList->SetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1 + dim[0]*dim[1], pts[6]);
                                Points->SetPoint(k*dim[1]*dim[0]+j*dim[0]+i + dim[0] + dim[0]*dim[1], this->UGrid->GetPoint(pts[7]));
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
        //      Points->GetPoint(i, x);
        //      OFile << i<<"  "<<IdList->GetId(i)<<"  "<<x[0]<<"  "<<x[1]<<"  "<<x[2]<<endl;
        //}
}

void vtkMimxGenerateHexahedronMesh::ReCalculateInteriorNodes()
{
        int dim[3];
        //  first extract surface nodes and their connectivity
        vtkGeometryFilter *geofil = vtkGeometryFilter::New();
        geofil->SetInput(this->GetInput());
        geofil->Update();
        vtkIdList *idlist ;//= vtkIdList::New();

        vtkGenericCell *gencell = vtkGenericCell::New();

        
        //for(int i=0; i < this->GetInput()->GetNumberOfCells(); i++)
        //{
        //      ((vtkStructuredGrid*)(this->StructuredGridCollection->
        //              GetItemAsObject(i)))->GetDimensions(dim);
        //      gencell->Initialize();
        //      this->GetInput()->GetCell(i,gencell);
        //      vtkPoints *pointscell = vtkPoints::New();
        //      vtkIdList *pointidlist = vtkIdList::New();
        //      this->GetCellPoints(i, pointscell,pointidlist);
        //      //       if a face is an interior face computer interior nodes of the face using planar transfinite interpolation
        //      for(int j=0; j <6; j++)
        //      {
        //              idlist = gencell->GetFace(j)->GetPointIds();
        //              if(!this->CheckBoundaryFace(idlist, geofil->GetOutput()))
        //              {
        //                      vtkPoints* points = vtkPoints::New();
        //                      this->GetFaceUGrid(i,j,pointscell, points);
        //                      vtkPlanarTransfiniteInterpolation* interpolation = 
        //                              vtkPlanarTransfiniteInterpolation::New();
        //                      vtkPointSet* pointset = vtkStructuredGrid::New();
        //                      
        //                      pointset->SetPoints(points);
        //                      interpolation->SetInput(pointset);
        //                      if(j==2 || j==3)
        //                      {
        //                              interpolation->SetIDiv(dim[0]);
        //                              interpolation->SetJDiv(dim[2]);
        //                      }
        //                      if(j==0 || j==1)
        //                      {
        //                              interpolation->SetIDiv(dim[1]);
        //                              interpolation->SetJDiv(dim[2]);
        //                      }
        //                      if(j==4 || j==5)
        //                      {
        //                              interpolation->SetIDiv(dim[0]);
        //                              interpolation->SetJDiv(dim[1]);
        //                      }

        //                      // to compute the interior nodes of a face
        //                      interpolation->Update();
        //                      this->SetFaceUGrid(i,j,pointscell, interpolation->GetOutput()->GetPoints());
        //                      pointset->Delete();
        //                      points->Delete();
        //                      interpolation->Delete();
        //              }
        //      }

        //      //      vtkSolidTransfiniteInterpolation* sol_interp = vtkSolidTransfiniteInterpolation//::New();
        //      vtkSolidTransfiniteInterpolation *sol_interp = vtkSolidTransfiniteInterpolation::New();
        //      sol_interp->SetIDiv(dim[0]);    sol_interp->SetJDiv(dim[1]); sol_interp->SetKDiv(dim[2]);
        //      vtkPointSet* pointset = vtkStructuredGrid::New();
        //      pointset->SetPoints(pointscell);
        //      sol_interp->SetInput(pointset);
        //      // to compute the interior nodes of a structured hexahedron grid
        //      sol_interp->Update();
        //      this->SetCellPoints(sol_interp->GetOutput()->GetPoints(), pointidlist);
        //      this->UGrid->Modified();
        //      vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
        //      sgrid->SetPoints(sol_interp->GetOutput()->GetPoints());
        //      sgrid->SetDimensions(dim);
        //      vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
        //      writer->SetInput(sgrid);
        //      writer->SetFileName("sgrid.vtk");
        //      writer->Write();
        //      pointset->Delete();
        //      sol_interp->Delete();
        //      pointscell->Delete();
        //      pointidlist->Delete();
        //}

        for(int i=0; i < this->GetInput()->GetNumberOfCells(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGridCollection->
                        GetItemAsObject(i)))->GetDimensions(dim);
                gencell->Initialize();
                this->GetInput()->GetCell(i,gencell);
                vtkPoints *pointscell = vtkPoints::New();
                vtkIdList *pointidlist = vtkIdList::New();
                this->GetCellPoints(i, pointscell,pointidlist);
                //       if a face is an interior face computer interior nodes of the face using planar transfinite interpolation
                for(int j=0; j <6; j++)
                {
                        idlist = gencell->GetFace(j)->GetPointIds();
                        if(!this->CheckBoundaryFace(idlist, geofil->GetOutput()))
                        {
                                vtkPoints* points = vtkPoints::New();
                                this->GetFaceUGrid(i,j,pointscell, points);
                                vtkMimxPlanarEllipticalInterpolation* interpolation = 
                                        vtkMimxPlanarEllipticalInterpolation::New();
                                interpolation->SetNumberOfIterations(10);
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
                                this->SetFaceUGrid(i,j,pointscell, interpolation->GetOutput()->GetPoints());
                                points->Delete();
                                interpolation->Delete();
                                planegrid->Delete();
                        }
                }

                //      vtkSolidTransfiniteInterpolation* sol_interp = vtkSolidTransfiniteInterpolation//::New();
                vtkMimxSolidEllipticalInterpolation *sol_interp = vtkMimxSolidEllipticalInterpolation::New();
                sol_interp->SetNumberOfIterations(10);
                vtkStructuredGrid *solidgrid = vtkStructuredGrid::New();
                solidgrid->SetPoints(pointscell);
                solidgrid->SetDimensions(dim);
                sol_interp->SetInput(solidgrid);
                // to compute the interior nodes of a structured hexahedron grid
                sol_interp->Update();
                this->SetCellPoints(sol_interp->GetOutput()->GetPoints(), pointidlist);
                this->UGrid->Modified();
                vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
                sgrid->SetPoints(sol_interp->GetOutput()->GetPoints());
                sgrid->SetDimensions(dim);
                //vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
                //writer->SetInput(sgrid);
                //writer->SetFileName("sgrid.vtk");
                //writer->Write();
                solidgrid->Delete();
                sol_interp->Delete();
                pointscell->Delete();
                pointidlist->Delete();
        }
        geofil->Delete();
        //vtkUnstructuredGridWriter* writer1 = vtkUnstructuredGridWriter::New();
        //writer1->SetInput(this->UGrid);
        //writer1->SetFileName("debug.vtk");
        //writer1->Write();
}

void vtkMimxGenerateHexahedronMesh::GetFaceUGrid(int CellNum, int FaceNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        switch (FaceNum) {
        case 0:
                GetFace0UGrid(CellNum, PointListCell, PointListFace);
                break;
        case 1:
                GetFace1UGrid(CellNum, PointListCell, PointListFace);
                break;
        case 2:
                GetFace2UGrid(CellNum, PointListCell, PointListFace);
                break;
        case 3:
                GetFace3UGrid(CellNum, PointListCell, PointListFace);
                break;
        case 4:
                GetFace4UGrid(CellNum, PointListCell, PointListFace);
                break;
        case 5:
                GetFace5UGrid(CellNum, PointListCell, PointListFace);
                break;
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace0UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointListFace->SetNumberOfPoints(dim[1]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointListFace->InsertPoint(y*dim[1] + x, PointListCell->GetPoint(y*dim[0]*dim[1]+x*dim[0]));
                        // cout << y*dim[1]+x<<"  "<<y*dim[0]*dim[1]+x*dim[0]<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace1UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointListFace->SetNumberOfPoints(dim[1]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointListFace->InsertPoint(y*dim[1] + x, PointListCell->GetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1));
                        // cout <<y*dim[1] + x<<"  "<<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace2UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointListFace->SetNumberOfPoints(dim[0]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListFace->InsertPoint(y*dim[0] + z, PointListCell->GetPoint(y*dim[0]*dim[1]+z));
                        // cout << y*dim[0] + z<<"  "<<y*dim[0]*dim[1]+z<<"  "<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace3UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointListFace->SetNumberOfPoints(dim[0]*dim[2]);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListFace->InsertPoint(y*dim[0] + z,PointListCell->GetPoint(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z));
                        // cout << y*dim[0] + z<<"  "<<y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z<<"  "<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace4UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointListFace->SetNumberOfPoints(dim[0]*dim[1]);
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListFace->InsertPoint(x*dim[0] + z, PointListCell->GetPoint(x*dim[0]+z));
                        // cout<<x*dim[0] + z<<"  "<<x*dim[0]+z<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::GetFace5UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        PointListFace->SetNumberOfPoints(dim[0]*dim[1]);
        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListFace->InsertPoint(x*dim[0] + z, PointListCell->GetPoint((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z));
                        // cout<<x*dim[0] + z<<"  "<<(dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetCellPoints(vtkPoints* Points, vtkIdList *IdList)
{
        for(int i=0; i < Points->GetNumberOfPoints(); i++)
        {
                this->UGrid->GetPoints()->SetPoint(IdList->GetId(i), Points->GetPoint(i));
        }
}

void vtkMimxGenerateHexahedronMesh::SetFaceUGrid(int CellNum, int FaceNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        switch (FaceNum) {
                case 0:
                        SetFace0UGrid(CellNum, PointListCell, PointListFace);
                        break;
                case 1:
                        SetFace1UGrid(CellNum, PointListCell, PointListFace);
                        break;
                case 2:
                        SetFace2UGrid(CellNum, PointListCell, PointListFace);
                        break;
                case 3:
                        SetFace3UGrid(CellNum, PointListCell, PointListFace);
                        break;
                case 4:
                        SetFace4UGrid(CellNum, PointListCell, PointListFace);
                        break;
                case 5:
                        SetFace5UGrid(CellNum, PointListCell, PointListFace);
                        break;
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace0UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointListCell->SetPoint(y*dim[0]*dim[1]+x*dim[0],
                                PointListFace->GetPoint(y*dim[1] + x));
                        //              // cout<<y*dim[0]*dim[1]+x*dim[0]<<"  "<<y*dim[1] + x<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace1UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int y=0; y < dim[2]; y++)
        {
                for(int x=0; x < dim[1]; x++)
                {
                        PointListCell->SetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1,
                                PointListFace->GetPoint(y*dim[1] + x));
                        //// cout <<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<"  "<<y*dim[1] + x<<endl;
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace2UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListCell->SetPoint(y*dim[0]*dim[1]+z,PointListFace->GetPoint(y*dim[0] + z));
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace3UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);
        for(int y=0; y < dim[2]; y++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListCell->SetPoint(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z,
                                PointListFace->GetPoint(y*dim[0] + z));
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace4UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);

        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListCell->SetPoint(x*dim[0]+z,PointListFace->GetPoint(x*dim[0] + z));
                }
        }
}

void vtkMimxGenerateHexahedronMesh::SetFace5UGrid(int CellNum, vtkPoints* PointListCell, vtkPoints* PointListFace)
{
        int dim[3];
        ((vtkStructuredGrid*)(this->StructuredGridCollection->
                GetItemAsObject(CellNum)))->GetDimensions(dim);

        for(int x=0; x < dim[1]; x++)
        {
                for(int z=0; z < dim[0]; z++)
                {
                        PointListCell->SetPoint((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z,
                                PointListFace->GetPoint(x*dim[0] + z));
                }
        }
}


void vtkMimxGenerateHexahedronMesh::PrintSelf(ostream& os, vtkIndent indent)
{
}
