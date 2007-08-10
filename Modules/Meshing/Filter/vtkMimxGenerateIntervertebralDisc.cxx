/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxGenerateIntervertebralDisc.cxx,v $
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

#include "vtkMimxGenerateIntervertebralDisc.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkMergeCells.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPolyData.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"

#include "vtkMimxButterflyMeshFromBounds.h"
#include "vtkMimxEdgeToPlanarStructuredGrid.h"
#include "vtkMimxExtractStructuredGridEdge.h"
#include "vtkMimxExtractStructuredGridFace.h"
#include "vtkMimxStructuredPlanarToStructuredSolidGrid.h"
#include "vtkMimxSubdivideCurve.h"
#include "vtkMimxUnstructuredToStructuredGrid.h"

#include "vtkPolyDataWriter.h"
#include "vtkStructuredGridWriter.h"

vtkCxxRevisionMacro(vtkMimxGenerateIntervertebralDisc, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkMimxGenerateIntervertebralDisc);

// Description:

vtkMimxGenerateIntervertebralDisc::vtkMimxGenerateIntervertebralDisc()
{
        this->DivisionsAlongAxis = 1;
        this->InnerBulgeOffset = 0.0;
        this->OuterBulgeOffset = 0.0;
        this->SetNumberOfInputPorts(3);
        this->DiscBoundingBox = vtkUnstructuredGrid::New();
        for (int i=0; i<9; i++) {
                this->UpSGrid[i] = vtkStructuredGrid::New();
                this->LowSGrid[i] = vtkStructuredGrid::New();
                this->DiscSGrid[i] = vtkStructuredGrid::New();
        }
}

vtkMimxGenerateIntervertebralDisc::~vtkMimxGenerateIntervertebralDisc()
{
        this->DiscBoundingBox->Delete();
        for (int i=0; i<9; i++) {
                this->UpSGrid[i]->Delete();
                this->LowSGrid[i]->Delete();
                this->DiscSGrid[i]->Delete();
        }
}

int vtkMimxGenerateIntervertebralDisc::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        
  // get the info objects
  vtkInformation *lowerVertBodyInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *upperVertBodyInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *bboxInfo = inputVector[2]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkUnstructuredGrid *lowervertbody = vtkUnstructuredGrid::SafeDownCast(
    lowerVertBodyInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *uppervertbody = vtkUnstructuredGrid::SafeDownCast(
          upperVertBodyInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *bbox = vtkUnstructuredGrid::SafeDownCast(
          bboxInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
 
        // calculate the center of the intervertebral disc
    // right now the calculation is based on the average of the centers of the 
  // 8th boxes in the upper and lower vertebral bodies
  int i;
  vtkMimxUnstructuredToStructuredGrid *upugridtosgrid ;
  vtkMimxUnstructuredToStructuredGrid *lowugridtosgrid ;

  for (i=0; i<9; i++) {
          upugridtosgrid = vtkMimxUnstructuredToStructuredGrid::New();
          upugridtosgrid->SetInput(uppervertbody);
          upugridtosgrid->SetBoundingBox(bbox);
          upugridtosgrid->SetStructuredGridNum(i);
          upugridtosgrid->Update();
          this->UpSGrid[i]->DeepCopy(upugridtosgrid->GetOutput());

          lowugridtosgrid = vtkMimxUnstructuredToStructuredGrid::New();
          lowugridtosgrid->SetInput(lowervertbody);
          lowugridtosgrid->SetBoundingBox(bbox);
          lowugridtosgrid->SetStructuredGridNum(i);
          lowugridtosgrid->Update();
          this->LowSGrid[i]->DeepCopy(lowugridtosgrid->GetOutput());

          upugridtosgrid->Delete();
          lowugridtosgrid->Delete();
  }

        
  double lowcenter[3], upcenter[3];
  this->LowSGrid[8]->GetCenter(lowcenter);
  this->UpSGrid[8]->GetCenter(upcenter);

  for(i=0; i<3; i++)        this->Center[i] = (lowcenter[i]+upcenter[i])/2.0;
        
  for (i=0; i<9; i++) {
          this->GenerateStructuredGrid(i);
  }
        MergeSGridToUGrid();
        // generate disc bounding box structure
        vtkMimxButterflyMeshFromBounds *butterfly = vtkMimxButterflyMeshFromBounds::New();
        butterfly->SetOutput(this->DiscBoundingBox);
        butterfly->SetNumberOfLayers(2);
        butterfly->SetBounds(output->GetBounds());
        butterfly->Update();
//        this->DiscBoundingBox->DeepCopy(butterfly->GetOutput());
        vtkIntArray *intarray = vtkIntArray::New();
        intarray->DeepCopy((vtkIntArray*) bbox->GetCellData()->GetVectors());
        double meshseed[3];
        // change the mesh seed
        for (i=0; i<9; i++) {
                intarray->GetTuple(i, meshseed);
                meshseed[1] = this->DivisionsAlongAxis+1;
                intarray->SetTuple(i, meshseed);
        }
        this->DiscBoundingBox->GetCellData()->SetVectors(intarray);
        intarray->Delete();
        butterfly->Delete();

   return 1;
}

void vtkMimxGenerateIntervertebralDisc::EdgeFromPoints(int CellNum, int EdgeNum,  vtkPolyData *Edge1, 
                                                                                                   vtkPolyData *Edge2, vtkPolyData *OutputEdge, int BeginOrEnd)
{
        double BulgeFactor;
        if(CellNum < 4)
        {
                if(EdgeNum == 8 || EdgeNum == 10)
                {
                        BulgeFactor = this->InnerBulgeOffset;
                }
                else        BulgeFactor = this->OuterBulgeOffset;
        }
        else
        {
                if(CellNum >= 4 && CellNum <8)
                {
                        if(EdgeNum == 8 || EdgeNum == 10)
                        {
                                BulgeFactor = 0.0;
                        }
                        else        BulgeFactor = this->InnerBulgeOffset;
                }
                else{
                        BulgeFactor = 0.0;
                }
        }
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(2);
        // set first and last points
        if (BeginOrEnd) {        points->SetPoint(0, Edge1->GetPoint(Edge1->GetNumberOfPoints()-1));
                points->SetPoint(1, Edge2->GetPoint(Edge2->GetNumberOfPoints()-1));
        }
        else{        points->SetPoint(0, Edge1->GetPoint(0));
        points->SetPoint(1, Edge2->GetPoint(0));
        }
        // insert points on the straight line connecting the two points
        vtkPolyData *polydata = vtkPolyData::New();
        polydata->SetPoints(points);
        vtkCellArray *cellarray = vtkCellArray::New();
        cellarray->InsertNextCell(2);
        cellarray->InsertCellPoint(0);        cellarray->InsertCellPoint(1);
        cellarray->Delete();
        vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
        subdivide->SetInput(polydata);
        subdivide->SetNumberOfDivisions(this->DivisionsAlongAxis);
        subdivide->Update();
        OutputEdge->DeepCopy(subdivide->GetOutput());
        subdivide->Delete();

        if (BulgeFactor != 0.0) {
                // normalized vector connecting the start and the end point
                double normal1[3], normal2[3], normal3[3];

                double x[3], y[3];
                int i, j;
                points->GetPoint(0, x);        points->GetPoint(this->DivisionsAlongAxis-1, y);

                for (i=0; i<3; i++) {
                        normal1[i] = (y[i]-x[i])/sqrt(vtkMath::Distance2BetweenPoints(x,y));
                }
                // normal of the plane formed by the start and end point and the center
                vtkTriangle::ComputeNormal(this->Center, y, x, normal2);

                // vector on which the imaginary circle's center lies
                vtkMath::Cross(normal2, normal1, normal3);
                // center of the line connecting points on the upper and lower vertebra
                double linecenter[3];
                for (i=0; i <3; i++) {
                        linecenter[i] = (x[i]+y[i])/2.0;
                }
                // calculate the center of the circle

                double z[3];
                double circlecenter[3];
                for (i=0; i<3; i++) {
                        z[i] = linecenter[i] - BulgeFactor*normal3[i];
                }
                // calculate the normals of two line segments defining the circle
                double normal4[3], normal5[3], normal6[3], normal7[3];
                for (i=0; i<3; i++) {
                        normal4[i] = (z[i]-x[i])/sqrt(vtkMath::Distance2BetweenPoints(z,x));
                }
                vtkMath::Cross(normal2, normal4, normal5);

                for (i=0; i<3; i++) {
                        normal6[i] = (y[i]-z[i])/sqrt(vtkMath::Distance2BetweenPoints(y,z));
                }
                vtkMath::Cross(normal2, normal6, normal7);

                double x1[3], y1[3];
                for (i=0; i<3; i++) {
                        x1[i] = x[i] + 1e7*normal5[i];
                        y1[i] = y[i] + 1e7*normal7[i];
                }
                vtkLine *line = vtkLine::New();
                line->GetPoints()->SetPoint(0,x);        line->GetPoints()->SetPoint(1,x1);
                double t, pcoords[3];
                int subid;
                line->IntersectWithLine(y,y1, 0.1, t, circlecenter, pcoords, subid);
                double radius = sqrt(vtkMath::Distance2BetweenPoints(x,circlecenter));
                line->Delete();
                points->Delete();
                // move the nodes lying on a straight line so that they conform to the shape of a circle
                for (i=1; i<OutputEdge->GetNumberOfPoints()-1; i++) {
                        // check the actual distance from center
                        double actdist = sqrt(vtkMath::Distance2BetweenPoints(OutputEdge->GetPoint(i),circlecenter));
                        OutputEdge->GetPoint(i,z);
                        // calculate the vector connecting two points
                        double normaltemp[3];
                        for (j=0; j<3; j++) {
                                normaltemp[j] = (z[j] - circlecenter[j])/sqrt(
                                        vtkMath::Distance2BetweenPoints(z,circlecenter));
                        }
                        for (j=0; j<3; j++) {
                                z[j] = z[j] - normaltemp[j]*(radius - actdist);
                        }
                        OutputEdge->GetPoints()->SetPoint(i,z);

                
                }
        }
        /*vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
        writer->SetInput(OutputEdge);
        writer->SetFileName("Edge.vtk");
        writer->Write();*/

}

void vtkMimxGenerateIntervertebralDisc::SetLowerVertebralBody(vtkUnstructuredGrid *LowUGrid)
{
        this->SetInput(0, LowUGrid);
}

void vtkMimxGenerateIntervertebralDisc::SetUpperVertebralBody(vtkUnstructuredGrid *UpUGrid)
{
        this->SetInput(1, UpUGrid);
}

void vtkMimxGenerateIntervertebralDisc::SetBoundingBox(vtkUnstructuredGrid *BBox)
{
        this->SetInput(2, BBox);
}

void vtkMimxGenerateIntervertebralDisc::GenerateStructuredGrid(int SGridNum)
{
        vtkMimxExtractStructuredGridEdge *exedge[8];
        vtkMimxEdgeToPlanarStructuredGrid *edgetoplane[4];
        vtkMimxExtractStructuredGridFace *exface4, *exface5;
        int i, dimplane[2];

        //edges from lower body mesh
        for (i=0; i<4; i++) {
                exedge[i] = vtkMimxExtractStructuredGridEdge::New();
                exedge[i]->SetInput(this->LowSGrid[SGridNum]);
                exedge[i]->SetEdgeNum(i+4);
                exedge[i]->Update();
        }

        //edges from upper body mesh
        for (i=4; i<8; i++) {
                exedge[i] = vtkMimxExtractStructuredGridEdge::New();
                exedge[i]->SetInput(this->UpSGrid[SGridNum]);
                exedge[i]->SetEdgeNum(i-4);
                exedge[i]->Update();
        }

        vtkPolyData *outpolydata8 , *outpolydata9, *outpolydata10, *outpolydata11;

        outpolydata8 = vtkPolyData::New();
        EdgeFromPoints(SGridNum, 8, exedge[3]->GetOutput(), exedge[7]->GetOutput(),  outpolydata8, 0);

        outpolydata9 = vtkPolyData::New();
        EdgeFromPoints(SGridNum, 9, exedge[1]->GetOutput(), exedge[5]->GetOutput(),  outpolydata9, 0);

        outpolydata10 = vtkPolyData::New();
        EdgeFromPoints(SGridNum, 10, exedge[3]->GetOutput(), exedge[7]->GetOutput(),  outpolydata10, 1);

        outpolydata11 = vtkPolyData::New();
        EdgeFromPoints(SGridNum, 11, exedge[1]->GetOutput(), exedge[5]->GetOutput(),  outpolydata11, 1);

        for (i=0; i <4; i++) {        edgetoplane[i] = vtkMimxEdgeToPlanarStructuredGrid::New();
        }
        // face 0
        i = 0;
        dimplane[0] = exedge[3]->GetOutput()->GetNumberOfPoints();
        dimplane[1] = outpolydata10->GetNumberOfPoints();
        edgetoplane[i]->SetDimensions(dimplane);
        edgetoplane[i]->SetInput(0, exedge[3]->GetOutput());
        edgetoplane[i]->SetInput(1, outpolydata10);
        edgetoplane[i]->SetInput(2, exedge[7]->GetOutput());
        edgetoplane[i]->SetInput(3, outpolydata8);
        edgetoplane[i]->Update();

        // face 1
        i = 1;
        dimplane[0] = exedge[1]->GetOutput()->GetNumberOfPoints();
        dimplane[1] = outpolydata11->GetNumberOfPoints();
        edgetoplane[i]->SetDimensions(dimplane);
        edgetoplane[i]->SetInput(0, exedge[1]->GetOutput());
        edgetoplane[i]->SetInput(1, outpolydata11);
        edgetoplane[i]->SetInput(2, exedge[5]->GetOutput());
        edgetoplane[i]->SetInput(3, outpolydata9);
        edgetoplane[i]->Update();

        // face 2
        i = 2;
        dimplane[0] = exedge[0]->GetOutput()->GetNumberOfPoints();
        dimplane[1] = outpolydata9->GetNumberOfPoints();
        edgetoplane[i]->SetDimensions(dimplane);
        edgetoplane[i]->SetInput(0, exedge[0]->GetOutput());
        edgetoplane[i]->SetInput(1, outpolydata9);
        edgetoplane[i]->SetInput(2, exedge[4]->GetOutput());
        edgetoplane[i]->SetInput(3, outpolydata8);
        edgetoplane[i]->Update();

        // face 3
        i = 3;
        dimplane[0] = exedge[2]->GetOutput()->GetNumberOfPoints();
        dimplane[1] = outpolydata11->GetNumberOfPoints();
        edgetoplane[i]->SetDimensions(dimplane);
        edgetoplane[i]->SetInput(0, exedge[2]->GetOutput());
        edgetoplane[i]->SetInput(1, outpolydata11);
        edgetoplane[i]->SetInput(2, exedge[6]->GetOutput());
        edgetoplane[i]->SetInput(3, outpolydata10);
        edgetoplane[i]->Update();

        //face 4
        exface4 = vtkMimxExtractStructuredGridFace::New();
        exface4->SetInput(this->LowSGrid[SGridNum]);
        exface4->SetFaceNum(5);
        exface4->Update();

        //face 5
        exface5 = vtkMimxExtractStructuredGridFace::New();
        exface5->SetInput(this->UpSGrid[SGridNum]);
        exface5->SetFaceNum(4);
        exface5->Update();

        // structured solid mesh
        vtkMimxStructuredPlanarToStructuredSolidGrid *planetosolid = 
                vtkMimxStructuredPlanarToStructuredSolidGrid::New();
        for(i=0; i<4; i++)
        {
                planetosolid->SetInput(i, edgetoplane[i]->GetOutput());
        }
        planetosolid->SetInput(4, exface4->GetOutput());
        planetosolid->SetInput(5, exface5->GetOutput());
        planetosolid->Update();
        this->DiscSGrid[SGridNum]->DeepCopy(planetosolid->GetOutput());
        //vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
        //writer->SetInput(this->DiscSGrid[SGridNum]);
        //writer->SetFileName("sgrid.vtk");
        //writer->Write();
        //writer->Delete();
        for (i=0; i<8; i++)                exedge[i]->Delete();
        for (i=0; i<4; i++)                edgetoplane[i]->Delete();
        outpolydata8->Delete();        outpolydata9->Delete(); outpolydata10->Delete();
        outpolydata11->Delete();        planetosolid->Delete();
        exface4->Delete();        exface5->Delete();
}

void vtkMimxGenerateIntervertebralDisc::MergeSGridToUGrid()
{
        vtkUnstructuredGrid *output = 
                vtkUnstructuredGrid::SafeDownCast(this->GetOutputDataObject(0));
        int numele = 0;
        int numnodes = 0;
        int dim[3];
//        vtkCell* cell;
        //        calculate number of nodes and elements
        for(int i=0; i < 9; i++)
        {
                this->DiscSGrid[i]->GetDimensions(dim);
                numnodes = numnodes + dim[0]*dim[1]*dim[2];
                numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
        }

        vtkMergeCells* mergecells = vtkMergeCells::New();
        mergecells->SetUnstructuredGrid(output);
        mergecells->MergeDuplicatePointsOn();
        mergecells->SetTotalNumberOfDataSets(9);
        mergecells->SetTotalNumberOfCells(numele);
        mergecells->SetTotalNumberOfPoints(numnodes);
        for(int i = 0; i < 9; i++)
        {
                mergecells->MergeDataSet(this->DiscSGrid[i]);
        }
        mergecells->Finish();
        //vtkIdType max_val = 0;
        //for(int i=0; i < output->GetNumberOfCells(); i++)
        //{
        //        cell = output->GetCell(i);
        //        vtkIdList* idlist = cell->GetPointIds();

        //        for(int j=0; j <8; j++)
        //        {
        //                vtkIdType val = idlist->GetId(j);
        //                if(val > max_val)        max_val = val;
        //        }
        //}
        //output->GetPoints()->Squeeze();
        //output->GetPoints()->SetNumberOfPoints(max_val+1);

        //int dim[3], m;
        //int numele = 0;
        //int numnode = 0;
        //int i, j, k;
        //for(i=0; i <9; i++)
        //{
        //        this->DiscSGrid[i]->GetDimensions(dim);
        //        numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
        //        numnode = numnode + dim[0]*dim[1]*dim[2];
        //}

        //// start with the other bounding boxes
        //// check if a dataset contains coincident points
        //vtkPointLocator *globallocator = vtkPointLocator::New();
        //vtkPoints *globalpoints = vtkPoints::New();
        //globalpoints->Allocate(numnode);
        //output->Allocate(numele);
        //globallocator->InitPointInsertion(globalpoints, source->GetBounds());

        //vtkIntArray *intarray = vtkIntArray::New();
        //intarray->SetNumberOfComponents(1);

        //int startnodenum;
        //for(m=0; m < 9; m++)

        //{
        //        vtkPoints* sgridpoints = this->DiscSGrid[m]->GetPoints();

        //        this->DiscSGrid[m]->GetDimensions(dim);

        //        // create the global point id list for the individual points in the data set
        //        // and insert unique points belonging to different data sets
        //        vtkIdList *poirenum = vtkIdList::New();
        //        startnodenum = globalpoints->GetNumberOfPoints();

        //        for (i=0; i < sgridpoints->GetNumberOfPoints(); i++)
        //        {
        //                // if point does not exist
        //                if(globallocator->IsInsertedPoint(sgridpoints->GetPoint(i)) == -1)
        //                {
        //                        poirenum->InsertNextId(globalpoints->GetNumberOfPoints());
        //                        globallocator->InsertNextPoint(sgridpoints->GetPoint(i));
        //                        double status[1];
        //                        this->DiscSGrid[m]->GetPointData()->GetScalars()->GetTuple(i,status);
        //                        intarray->InsertNextTuple1(status[0]);
        //                }
        //                else
        //                {
        //                        // if the point belongs to different data set
        //                        if(globallocator->IsInsertedPoint(sgridpoints->GetPoint(i)) < startnodenum)
        //                        {
        //                                poirenum->InsertNextId(globallocator->IsInsertedPoint(sgridpoints->GetPoint(i)));
        //                        }
        //                        else{
        //                                poirenum->InsertNextId(globalpoints->GetNumberOfPoints());
        //                                globallocator->InsertNextPoint(sgridpoints->GetPoint(i));        
        //                                double status[1];
        //                                this->DiscSGrid[m]->GetPointData()->GetScalars()->GetTuple(i,status);
        //                                intarray->InsertNextTuple1(status[0]);
        //                        }
        //                }
        //        }
        //        // insert the element connectivity based on global point ids
        //        vtkIdList *ptids = vtkIdList::New();
        //        for (k=0; k<dim[2]-1; k++)
        //        {
        //                for (j=0; j<dim[1]-1; j++)
        //                {
        //                        for (i=0; i<dim[0]-1; i++)
        //                        {
        //                                ptids->Initialize();
        //                                ptids->SetNumberOfIds(8);
        //                                ptids->SetId(0, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i));
        //                                ptids->SetId(1, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i+1));
        //                                ptids->SetId(2, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1));
        //                                ptids->SetId(3, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]));
        //                                ptids->SetId(4, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]*dim[1]));
        //                                ptids->SetId(5, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i+1 + dim[0]*dim[1]));
        //                                ptids->SetId(6, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0]+1 + dim[0]*dim[1]));
        //                                ptids->SetId(7, poirenum->GetId(k*dim[1]*dim[0]+j*dim[0]+i + dim[0] + dim[0]*dim[1]));
        //                                output->InsertNextCell(12, ptids);        
        //                        }
        //                }
        //        }
        //        ptids->Delete();
        //        poirenum->Delete();
        //}

        //output->GetPointData()->SetScalars(intarray);
        //intarray->Delete();
        //output->SetPoints(globalpoints);
        //output->Squeeze();
        //globalpoints->Delete();
        //globallocator->Delete();
}


void vtkMimxGenerateIntervertebralDisc::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

