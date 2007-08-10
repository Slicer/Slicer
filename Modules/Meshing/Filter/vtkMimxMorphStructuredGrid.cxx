/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMorphStructuredGrid.cxx,v $
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

#include "vtkMimxMorphStructuredGrid.h"

#include "vtkCellArray.h"
#include "vtkCellLocator.h"
#include "vtkCellTypes.h"
#include "vtkCollection.h"
#include "vtkExecutive.h"
#include "vtkGenericCell.h"
#include "vtkGeometryFilter.h"
#include "vtkHexahedron.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

#include "vtkStructuredGridWriter.h"

#include "vtkMimxExtractStructuredGridFace.h"
#include "vtkMimxPlanarTransfiniteInterpolation.h"
#include "vtkMimxSolidTransfiniteInterpolation.h"
#include "vtkMimxExtractStructuredGridEdge.h"
#include "vtkMimxSetStructuredGridEdge.h"
#include "vtkMimxSetStructuredGridFace.h"

vtkCxxRevisionMacro(vtkMimxMorphStructuredGrid, "$Revision: 1.6 $");
vtkStandardNewMacro(vtkMimxMorphStructuredGrid);

// Construct object to extract all of the input data.
vtkMimxMorphStructuredGrid::vtkMimxMorphStructuredGrid()
{
        this->SetNumberOfInputPorts(3);
        this->ProjectionType = this->ClosestPoint;
        this->CellNum = -1;
}

vtkMimxMorphStructuredGrid::~vtkMimxMorphStructuredGrid()
{
        
}


int vtkMimxMorphStructuredGrid::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
        vtkInformation *outInfo = outputVector->GetInformationObject(0);
        vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
        vtkInformation *gridInfo = inputVector[2]->GetInformationObject(0);
        
        vtkPolyData *source = NULL;
        
        if(sourceInfo)        source = vtkPolyData::SafeDownCast(
                sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
        else
        {
                vtkErrorMacro("Source needs to be set");
                return 0;
        }

        vtkCellLocator *CellLocator = vtkCellLocator::New();
        CellLocator->SetDataSet(source);
        CellLocator->BuildLocator();

        // get the input and output
        vtkStructuredGrid *input = vtkStructuredGrid::SafeDownCast(
                inInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));
        
        int numNodes = input->GetNumberOfPoints();
        int numCells = input->GetNumberOfCells();

if(numNodes <= 0 || numCells <= 0){vtkErrorMacro("Invalid input");
        return 0;}
        
        vtkPoints *pointsstore = vtkPoints::New();
        pointsstore->DeepCopy(input->GetPoints());

        //vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
        //writer->SetFileName("SGrid.vtk");
        if(gridInfo)   
        {
                vtkUnstructuredGrid *inputgrid = vtkUnstructuredGrid::SafeDownCast(
                        gridInfo->Get(vtkDataObject::DATA_OBJECT()));

                // check input cell type
                vtkCellTypes *celltypes = vtkCellTypes::New();
                inputgrid->GetCellTypes(celltypes);
                for(int i =0; i < celltypes->GetNumberOfTypes(); i++)
                {
                        if(celltypes->GetCellType(i) != 12)
                        {
                                vtkErrorMacro("Unstructuredgrid input must contain only hexahedrons");
                                celltypes->Delete();
                                return 0;
                        }
                }
                celltypes->Delete();

                vtkIdList *idlistedge = vtkIdList::New();
                idlistedge->SetNumberOfIds(12);
                
                vtkIdList *idlistface = vtkIdList::New();
                idlistface->SetNumberOfIds(6);

                vtkGenericCell *gencell = vtkGenericCell::New();

                vtkIdList *idlist  = NULL;

                if(this->CellNum == -1){
                        vtkErrorMacro("Cell number to which the input corresponds to in the unstructured grid not set");
                        return 0;
                }

                inputgrid->GetCell(this->CellNum, gencell);

                for(int k = 0; k<12; k++)        idlistedge->SetId(k,0);
                for(int k=0; k<6; k++)        idlistface->SetId(k,0);

                // start with edges

                vtkGeometryFilter *geofil = vtkGeometryFilter::New();
                geofil->SetInput(inputgrid);
                geofil->Update();
                geofil->GetOutput()->BuildLinks();

                for(int k=0; k<12; k++)
                {
                        idlist = gencell->GetEdge(k)->GetPointIds();
                        if(CheckInteriorEdge(gencell->GetEdge(k)->GetPointIds(),geofil->GetOutput()))
                        {
                                idlistedge->SetId(k,1);
                                vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
                                sgrid->SetPoints(pointsstore);
                                sgrid->SetDimensions(input->GetDimensions());

                                vtkMimxExtractStructuredGridEdge *exsgridedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exsgridedge->SetInput(sgrid);
                                exsgridedge->SetEdgeNum(k);
                                exsgridedge->Update();
                                //writer->SetInput(exsgridedge->GetOutput());
                                //writer->Write();

                                vtkPoints *points1 = vtkPoints::New();
                                points1->DeepCopy(exsgridedge->GetOutput()->GetPoints());
                                this->ClosestPointProjection(CellLocator, points1);

                                vtkPolyData *sgrid1 = vtkPolyData::New();
                                sgrid1->SetPoints(points1);
                                //                                writer->SetInput(sgrid1);
                                //                                writer->Write();
                                vtkMimxSetStructuredGridEdge *setsgridedge = 
                                  vtkMimxSetStructuredGridEdge::New();
                                setsgridedge->SetInput(sgrid);
                                setsgridedge->SetEdge(sgrid1);
                                setsgridedge->SetEdgeNum(k);
                                setsgridedge->Update();

                                ///////
                                //vtkStructuredGridWriter *writer2 = vtkStructuredGridWriter::New();
                                //writer2->SetInput(setsgridface->GetOutput());
                                //writer2->SetFileName("SGrid2.vtk");
                                //writer2->Write();
                                //writer2->Delete();
                                //////////                

                                //sgridstore->GetPoints()->Initialize();
                                pointsstore->Initialize();
                                pointsstore->DeepCopy(setsgridedge->GetOutput()->GetPoints());
                                exsgridedge->Delete();
                                sgrid->Delete();
                                sgrid1->Delete();
                                setsgridedge->Delete();
                                points1->Delete();
                        }
                }
                for(int j=0; j < 6; j++)
                {
                        //        loop through all the face and morph all the boundary faces
                        //                idlist->Initialize();
                        idlist = gencell->GetFace(j)->GetPointIds();
                        // check if the face is a boundary or not
                        if(this->CheckBoundaryFace(idlist, geofil->GetOutput()))
                        {
                                vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
                                sgrid->SetPoints(pointsstore);
                                sgrid->SetDimensions(input->GetDimensions());

                                idlistface->SetId(j,1);
                                vtkMimxExtractStructuredGridFace *exsgridface = 
                                  vtkMimxExtractStructuredGridFace::New();
                                exsgridface->SetInput(sgrid);
                                exsgridface->SetFaceNum(j);
                                exsgridface->Update();
                                //writer->SetInput(exsgridface->GetOutput());
                                //writer->Write();
                                
                                vtkPoints *points1 = vtkPoints::New();
                                points1->DeepCopy(exsgridface->GetOutput()->GetPoints());
                                this->ClosestPointProjection(CellLocator, points1);

                                vtkStructuredGrid *sgrid1 = vtkStructuredGrid::New();
                                sgrid1->SetPoints(points1);
                                sgrid1->SetDimensions(exsgridface->GetOutput()->GetDimensions());
        /*                        writer->SetInput(sgrid1);
                                writer->Write();
        */                        vtkMimxSetStructuredGridFace *setsgridface = 
                                   vtkMimxSetStructuredGridFace::New();
                                setsgridface->SetInput(sgrid);
                                setsgridface->SetFace(sgrid1);
                                setsgridface->SetFaceNum(j);
                                setsgridface->Update();
                        
        /*                        writer->SetInput(setsgridface->GetOutput());
                                writer->Write();
        */                        //sgridstore->GetPoints()->Initialize();
                                pointsstore->Initialize();
                                pointsstore->DeepCopy(setsgridface->GetOutput()->GetPoints());
                                exsgridface->Delete();
                                sgrid->Delete();
                                sgrid1->Delete();
                                setsgridface->Delete();
                                points1->Delete();
                        }
                }
                // for the interior edge start and end points from the morphed mesh need
                // to be chosen and the nodal position has to be recalculated.
                // check if edge of a cell is on the boundary or not

                //vtkIdType pt;
                //for(int k=0; k<12; k++)
                //{
                //        pt = idlistedge->GetId(k);
                //}
                for(int k=0; k<12; k++)
                {
                        if(!idlistedge->GetId(k))
                        {
                                if(this->CheckIfEdgeNodesRecalculated(k, idlistedge))
                                {
                                        // recalculate the interior nodes of a given edge
                                        this->RecalculateEdge(k, idlistedge, pointsstore, input->GetDimensions());
                                        vtkStructuredGrid *sgrid2 = vtkStructuredGrid::New();
                                        sgrid2->SetPoints(pointsstore);
                                        sgrid2->SetDimensions(input->GetDimensions());
                                        //writer->SetInput(sgrid2);
                                        //writer->Write();

                                }
                        }
                }
                //         if a face is an interior face compute interior nodes of the face using planar transfinite interpolation
                for(int j=0; j <6; j++)
                {
                        if(!idlistface->GetId(j))
                        {
                                vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
                                sgrid->SetPoints(pointsstore);
                                sgrid->SetDimensions(input->GetDimensions());

                                vtkMimxExtractStructuredGridFace *exgridface = 
                                  vtkMimxExtractStructuredGridFace::New();
                                exgridface->SetInput(sgrid);
                                exgridface->SetFaceNum(j);
                                exgridface->Update();

                                vtkMimxPlanarTransfiniteInterpolation* interpolation = 
                                        vtkMimxPlanarTransfiniteInterpolation::New();
                                vtkPointSet* pointset = vtkStructuredGrid::New();
                                int dim[3];

                                sgrid->GetDimensions(dim);                                
                                pointset->SetPoints(exgridface->GetOutput()->GetPoints());
                                interpolation->SetInput(pointset);
                                if(j==4 || j==5)
                                {
                                        interpolation->SetIDiv(dim[0]);
                                        interpolation->SetJDiv(dim[1]);
                                }
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
                                // to compute the interior nodes of a face
                                interpolation->Update();

                                vtkStructuredGrid *sgrid1 = vtkStructuredGrid::New();
                                sgrid1->SetPoints(interpolation->GetOutput()->GetPoints());
                                sgrid1->SetDimensions(exgridface->GetOutput()->GetDimensions());

                                vtkMimxSetStructuredGridFace *setgridface = 
                                  vtkMimxSetStructuredGridFace::New();
                                setgridface->SetInput(sgrid);
                                setgridface->SetFace(sgrid1);
                                setgridface->SetFaceNum(j);
                                setgridface->Update();

                /*                sgridstore->SetPoints(setgridface->GetOutput()->GetPoints());
                                writer->Write();*/
                                
                                pointsstore->Initialize();
                                pointsstore->DeepCopy(setgridface->GetOutput()->GetPoints());

                                setgridface->Delete();
                                pointset->Delete();
                                interpolation->Delete();
                                exgridface->Delete();
                                sgrid1->Delete();
                                sgrid->Delete();
                        }
                }
                
                int dim[3];
                input->GetDimensions(dim);
                vtkMimxSolidTransfiniteInterpolation *sol_interp = 
                  vtkMimxSolidTransfiniteInterpolation::New();
                sol_interp->SetIDiv(dim[0]);        sol_interp->SetJDiv(dim[1]); sol_interp->SetKDiv(dim[2]);
                vtkPointSet* pointset = vtkStructuredGrid::New();
                pointset->SetPoints(pointsstore);
                sol_interp->SetInput(pointset);
                // to compute the interior nodes of a structured hexahedron grid
                sol_interp->Update();
                pointsstore->Initialize();
                pointsstore->DeepCopy(sol_interp->GetOutput()->GetPoints());
                pointset->Delete();
                sol_interp->Delete();
                output->SetPoints(pointsstore);
                output->SetDimensions(input->GetDimensions());
                output->GetPointData()->PassData(input->GetPointData());
                pointsstore->Delete();
                geofil->Delete();
}

  return 1;
}

int vtkMimxMorphStructuredGrid::FillInputPortInformation(int port,        vtkInformation *info)
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
        else if(port == 2)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
                return 1;
        }
        else{ vtkErrorMacro("Invalid input port number");}
        return 0;
}

void vtkMimxMorphStructuredGrid::SetSource(vtkPolyData *Polydata)
{
        this->SetInput(1, Polydata);
}

void vtkMimxMorphStructuredGrid::SetGridStructure(vtkUnstructuredGrid *Ugrid)
{
        this->SetInput(2, Ugrid);
}



int vtkMimxMorphStructuredGrid::CheckBoundaryFace(vtkIdList* IdList, vtkPolyData *BoundaryData)
{
        vtkIdList *idlist2;
        //vtkIdType pt1,pt2,pt3,pt4;
        //pt1 = IdList->GetId(0);        pt2 = IdList->GetId(1);
        //pt3 = IdList->GetId(2);        pt4 = IdList->GetId(3);
        for(int i=0; i < BoundaryData->GetNumberOfCells(); i++)
        {
                idlist2 = BoundaryData->GetCell(i)->GetPointIds();
                //vtkIdType p1,p2,p3,p4;
                //p1 = idlist2->GetId(0);        p2 = idlist2->GetId(1);
                //p3 = idlist2->GetId(2);        p4 = idlist2->GetId(3);

                if(idlist2->IsId(IdList->GetId(0)) != -1 && idlist2->IsId(IdList->GetId(1)) != -1 &&
                        idlist2->IsId(IdList->GetId(2)) != -1 && idlist2->IsId(IdList->GetId(3)) != -1)
                {
                        return 1;
                }
        }
        return 0;
}

void vtkMimxMorphStructuredGrid::ClosestPointProjection(vtkCellLocator *Locator, vtkPoints *Points)
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

int vtkMimxMorphStructuredGrid::CheckIfEdgeNodesRecalculated(int EdgeNum, vtkIdList *IdList)
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

void vtkMimxMorphStructuredGrid::RecalculateEdge(
  int EdgeNum, 
  vtkIdList *IdList, 
  vtkPoints* PointsNew, 
  int dim[3])
{
        vtkStructuredGrid *Output = vtkStructuredGrid::New();
        Output->SetPoints(PointsNew);
        Output->SetDimensions(dim);

        vtkPoints *Points = vtkPoints::New();

        double pt1[3], pt2[3];


        switch (EdgeNum) {
                case 0:
                        if(IdList->GetId(3))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(3);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(8);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(1))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(1);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(9);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }

                        Points->SetNumberOfPoints(dim[0]);        
                        break;

                case 1:
                        if(IdList->GetId(0))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(0);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(9);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }

                        if(IdList->GetId(2))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(2);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(11);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[1]);        
                        break;
                case 2:
                        if(IdList->GetId(3))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(3);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(10);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0, pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(1))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(1);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(11);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[0]);        
                        break;
                case 3:
                        if(IdList->GetId(0))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                   vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(0);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(8);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(2))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(2);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(10);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[1]);        
                        break;
                case 4:
                        if(IdList->GetId(8))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(8);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(7);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(9))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(9);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(5);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[0]);        
                        break;
                case 5:
                        if(IdList->GetId(4))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(4);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(9);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(11))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(11);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(6);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[1]);        
                        break;

                case 6:
                        if(IdList->GetId(7))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(7);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(10);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(11))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(11);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(5);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[0]);        
                        break;
                case 7:
                        if(IdList->GetId(4))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(4);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(8);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(10))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(10);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(6);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[1]);        
                        break;
                case 8:
                        if(IdList->GetId(0))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(0);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(3);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(4))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                   vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(4);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(7);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[2]);
                        break;
                case 9:
                        if(IdList->GetId(0))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(0);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(1);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(4))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(4);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(5);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[2]);        
                        break;

                case 10:
                        if(IdList->GetId(3))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(3);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(2);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(7))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(7);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(6);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(0,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[2]);        
                        break;
                case 11:
                        if(IdList->GetId(1))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(1);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(2);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt1);
                                exedge->Delete();
                        }
                        if(IdList->GetId(5))
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(5);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        else
                        {
                                vtkMimxExtractStructuredGridEdge *exedge = 
                                  vtkMimxExtractStructuredGridEdge::New();
                                exedge->SetInput(Output);
                                exedge->SetEdgeNum(6);
                                exedge->Update();
                                exedge->GetOutput()->GetPoint(exedge->GetOutput()->GetNumberOfPoints()-1,pt2);
                                exedge->Delete();
                        }
                        Points->SetNumberOfPoints(dim[2]);
                        break;
        }

        for(int i=0; i <Points->GetNumberOfPoints(); i++)
        {
                Points->SetPoint(i, pt1[0] + ((pt2[0]-pt1[0])/(Points->GetNumberOfPoints()-1))*i, pt1[1] + ((pt2[1]-pt1[1])/(Points->GetNumberOfPoints()-1))*i, pt1[2] + ((pt2[2]-pt1[2])/(Points->GetNumberOfPoints()-1))*i);
        }
        
        vtkPolyData *PolyData = vtkPolyData::New();
        PolyData->SetPoints(Points);
        vtkMimxSetStructuredGridEdge *setedge = vtkMimxSetStructuredGridEdge::New();
        setedge->SetInput(Output);
        setedge->SetEdgeNum(EdgeNum);
        setedge->SetEdge(PolyData);
        setedge->Update();
        PointsNew->Initialize();
        PointsNew->DeepCopy(setedge->GetOutput()->GetPoints());
        setedge->Delete();
        Points->Delete();
        PolyData->Delete();
        Output->Delete();
        
}

int vtkMimxMorphStructuredGrid::CheckInteriorEdge(vtkIdList* IdList, vtkPolyData *BoundaryData)
{
        vtkIdList *idlist2;
        vtkIdType pt1,pt2;
        pt1 = IdList->GetId(0);        pt2 = IdList->GetId(1);
        for(int i=0; i < BoundaryData->GetNumberOfCells(); i++)
        {
                for(int j=0;  j < BoundaryData->GetCell(i)->GetNumberOfEdges(); j++)
                {
                        idlist2 = BoundaryData->GetCell(i)->GetEdge(j)->GetPointIds();

                        vtkIdType p1,p2;
                        p1 = idlist2->GetId(0);        p2 = idlist2->GetId(1);

                        if(idlist2->IsId(IdList->GetId(0)) != -1 && idlist2->IsId(IdList->GetId(1)) != -1)
                        {
                                return 1;
                        }
                }
        }
        return 0;
}


void vtkMimxMorphStructuredGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
