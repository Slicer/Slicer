/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxAttachBoundingBoxMesh.cxx,v $
Language:  C++
Date:      $Date: 2007/10/18 21:38:00 $
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


#include "vtkMimxAttachBoundingBoxMesh.h"

#include "vtkMimxMeshSeed.h"

#include "vtkCell.h"
#include "vtkMimxExtractStructuredGridEdge.h"
#include "vtkMimxExtractStructuredGridFace.h"
#include "vtkGeometryFilter.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkMimxPatchSurfaceMesh.h"
#include "vtkPointLocator.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCellLocator.h"
#include "vtkMimxBoundingBoxToStructuredGrids.h"
#include "vtkMimxMorphStructuredGrid.h"
#include "vtkMimxFourPointsToStructuredPlanarMesh.h"
#include "vtkMimxPlanarTransfiniteInterpolation.h"
#include "vtkMimxSetStructuredGridEdge.h"
#include "vtkMimxSetStructuredGridFace.h"
#include "vtkMimxSubdivideCurve.h"
#include "vtkMergeCells.h"
#include "vtkCollection.h"
#include "vtkPointData.h"
#include "vtkIntArray.h"

#include "vtkPolyDataWriter.h"
#include "vtkStructuredGridWriter.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkCellData.h"

vtkCxxRevisionMacro(vtkMimxAttachBoundingBoxMesh, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkMimxAttachBoundingBoxMesh);

// Description:

vtkMimxAttachBoundingBoxMesh::vtkMimxAttachBoundingBoxMesh()
{
        this->SetNumberOfInputPorts(4);
        this->SGridCollection = NULL;
        this->ModifiedInput = vtkUnstructuredGrid::New();
}

vtkMimxAttachBoundingBoxMesh::~vtkMimxAttachBoundingBoxMesh()
{
        if (this->SGridCollection)
        {
                int numCol = this->SGridCollection->GetNumberOfItems();
                this->SGridCollection->InitTraversal();
                do 
                {
                        this->SGridCollection->GetNextItemAsObject()->Delete();
                        numCol--;
                } while(numCol != 0);
                this->SGridCollection->Delete();
        }
        this->ModifiedInput->Delete();

}

int vtkMimxAttachBoundingBoxMesh::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *bboxInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *completeInfo = inputVector[2]->GetInformationObject(0);
  vtkInformation *cutInfo = inputVector[3]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *bbox = vtkUnstructuredGrid::SafeDownCast(
          bboxInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *complete = vtkPolyData::SafeDownCast(completeInfo->Get(
          vtkDataObject::DATA_OBJECT()));
  vtkPolyData *cut = vtkPolyData::SafeDownCast(cutInfo->Get(
          vtkDataObject::DATA_OBJECT()));


  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  // loop through all the cells in the bounding box to check if the
  // any cell has a face attached to the vertebral body
  // to store the planar structured grids that form the attaching points 
  vtkCollection *SGridPlaneCollection = NULL;

  int i,j,k;
  vtkIdList *idlistface = vtkIdList::New();
  idlistface->SetNumberOfIds(bbox->GetNumberOfCells());
  double tolerance = 1e-3;
  vtkPointLocator *locator = vtkPointLocator::New();
//  locator->InitPointInsertion(input->GetPoints(), input->GetPoints()->GetBounds());
  locator->SetDataSet(input);
  locator->BuildLocator();
  // loop through all cells
  cout <<"checking for common posterior and vertebral body elements"<<endl;
  for(i = 0; i< bbox->GetNumberOfCells(); i++)
  {
          idlistface->SetId(i, -1);
          // loop through all the faces
                vtkIdList *pointlistface;       
                bool status;
          for (k =0; k < bbox->GetCell(i)->GetNumberOfFaces(); k++) {
                  status = false;
                pointlistface = bbox->GetCell(i)->GetFace(k)->GetPointIds();
                for (j = 0; j < pointlistface->GetNumberOfIds(); j++) {
                        vtkIdType Id = pointlistface->GetId(j);
//                      if(locator->IsInsertedPoint(bbox->GetPoint(pointlistface->GetId(j))) == -1)
                        if(sqrt(vtkMath::Distance2BetweenPoints(input->GetPoint(
                                locator->FindClosestPoint(bbox->GetPoint(pointlistface->GetId(j)))),
                                bbox->GetPoint(pointlistface->GetId(j)))) > tolerance)
                        {
                                status = true;
                                break;
                        }
                }
                if(!status)
                {
                        idlistface->SetId(i, k);
                        if(!SGridPlaneCollection)
                        {
                                SGridPlaneCollection = vtkCollection::New();
                        }
                        break;
                }
          }               
  }
  // get the contours of the holes
  vtkMimxPatchSurfaceMesh *patch = vtkMimxPatchSurfaceMesh::New();
  patch->SetInput(cut);
  patch->Update();
  // we need to make one more copy of input and bbox
  // since we modify the co-ordinates of both input and bbox
  this->ModifiedInput->DeepCopy(input);

  vtkUnstructuredGrid* bboxcopy = vtkUnstructuredGrid::New();
  vtkPoints *points = vtkPoints::New();
  points->DeepCopy(bbox->GetPoints());
  bboxcopy->Allocate(bbox->GetNumberOfCells(),input->GetNumberOfCells());
  bboxcopy->SetPoints(points);
  for(i=0; i <bbox->GetNumberOfCells(); i++)
  {
          bboxcopy->InsertNextCell(bbox->GetCellType(i), bbox->GetCell(i)->GetPointIds());
  }
  points->Delete();
//  bboxcopy->CopyStructure(bbox);
 // bboxcopy->DeepCopy(bbox);

  double dist,ClosestPoint[3];
  vtkIdType CellId;
  int SubId;
  // a decision need to be made to project different points 
  // onto the surface. 4 categories of nodes exist. 1) exterior nodes
  // not on the surface of the input, 2) interior nodes
  // 3) exterior surface node on the surface of the input
  // and 4) interior surface node on the surface of the input
  // if a bounding box face lies on the vertebral body
  // check for the closest contour and morph the face onto the
  // contour

  vtkCellLocator *celllocatorsurface = vtkCellLocator::New();
  celllocatorsurface->SetDataSet(complete);
  celllocatorsurface->BuildLocator();

  vtkGeometryFilter *bboxfil = vtkGeometryFilter::New();
  bboxfil->SetInput(bbox);
  bboxfil->Update();

  for (i =0; i < bbox->GetNumberOfCells(); i++) {
          if(idlistface->GetId(i) != -1)
          {
                  vtkCellLocator *celllocator = vtkCellLocator::New();
                  celllocator->SetDataSet(patch->GetContour(this->GetClosestContour(
                                patch, bbox, bbox->GetCell(i)->GetFace(idlistface->GetId(i))->GetPointIds())));
                  celllocator->BuildLocator();
                  // get the face formed by 4 points on the vertebral body and morph it on to the contour
                  // also update the co-ordinates of the bounding box
                  vtkIdList *idlistbbox = bboxcopy->GetCell(i)->GetFace(idlistface->GetId(i))->GetPointIds();

                  for(j=0; j< idlistbbox->GetNumberOfIds(); j++)
                  {
                          vtkIdList *bboxpointcells = vtkIdList::New();
                          bboxcopy->GetPointCells(idlistbbox->GetId(j), bboxpointcells);
                          if(bboxpointcells->GetNumberOfIds() != 4)
                          {
                                  celllocator->FindClosestPoint(bboxcopy->GetPoint(idlistbbox->GetId(j)), ClosestPoint,
                                          CellId,SubId,dist);
                                  bboxcopy->GetPoints()->SetPoint(idlistbbox->GetId(j), ClosestPoint);
                          }
                          bboxpointcells->Delete();
                  }
                  celllocator->Delete();
          }
          else{
                 vtkIdList *idlistcell = bboxcopy->GetCell(i)->GetPointIds();

                for (j=0; j < idlistcell->GetNumberOfIds(); j++) {
                        vtkIdList *bboxpointcells = vtkIdList::New();
                        bboxfil->GetOutput()->GetPointCells(idlistcell->GetId(j),bboxpointcells);
                        if(bboxpointcells->GetNumberOfIds() != 0)
                        {
                                celllocatorsurface->FindClosestPoint(bboxcopy->GetPoint(idlistcell->GetId(j)), 
                                        ClosestPoint, CellId,SubId,dist);
                                bboxcopy->GetPoints()->SetPoint(idlistcell->GetId(j), ClosestPoint);
                        }
                        bboxpointcells->Delete();
                }
          }
  }

  celllocatorsurface->Delete();
  bboxfil->Delete();

  cout << "modifying required mesh seeds "<<endl;
  vtkMimxMeshSeed *meshseed = new vtkMimxMeshSeed;
  vtkIntArray *arr;
  bool arrstatus = false;
  meshseed->SetUnstructuredGrid(bboxcopy);
  if (!bbox->GetCellData()->GetVectors()) {
          arr = vtkIntArray::New();
          arr->SetNumberOfValues(bboxcopy->GetNumberOfCells());
          arr->SetNumberOfComponents(3);
          meshseed->SetMeshSeedValues(arr);
          meshseed->MeshSeedFromAverageElementLength(2.0);
          bbox->GetCellData()->SetVectors(arr);
          bboxcopy->GetCellData()->SetVectors(arr);
  }
  else{
                meshseed->SetMeshSeedValues((vtkIntArray*)bbox->GetCellData()->GetVectors());   
                bboxcopy->GetCellData()->SetVectors(bbox->GetCellData()->GetVectors());
                arr = (vtkIntArray*) bbox->GetCellData()->GetVectors();
                arrstatus = true;
  }
   // to store the final structured grids
  this->SGridCollection = vtkCollection::New();
  
  vtkGeometryFilter *fil = vtkGeometryFilter::New();
  fil->SetInput(input);
  fil->Update();
  // generate the planar structuredgrids
  for (i=0; i < bbox->GetNumberOfCells(); i++) {
          if (idlistface->GetId(i) != -1) 
          {
                  vtkCellLocator *celllocator = vtkCellLocator::New();
                  celllocator->SetDataSet(patch->GetContour(this->GetClosestContour(
                          patch, bbox, bbox->GetCell(i)->GetFace(idlistface->GetId(i))->GetPointIds())));
                  celllocator->BuildLocator();
                  // extract the structured grid generated from four points
                  vtkMimxFourPointsToStructuredPlanarMesh *fourpoitostructmesh = 
                    vtkMimxFourPointsToStructuredPlanarMesh::New();
                  fourpoitostructmesh->SetInput(fil->GetOutput());
                  vtkIdList *pointidlist = vtkIdList::New();
                  pointidlist->SetNumberOfIds(4);
                  vtkIdList *pointidlistface = bbox->GetCell(i)->GetFace(idlistface->GetId(i))->GetPointIds();
                  for(j = 0; j<4; j++)
                  {
                          pointidlist->SetId(j, locator->FindClosestPoint(bbox->GetPoint(pointidlistface->GetId(j))));
                  }
                  // ordering of the nodes should be maintained
                  // for faces 0, 3, 4 the ordering should be reversed
                  if(idlistface->GetId(i) == 0 || idlistface->GetId(i) == 3 ||
                          idlistface->GetId(i) == 4){
                                  vtkIdType temp = pointidlist->GetId(1);
                                  pointidlist->SetId(1, pointidlist->GetId(3));
                                  pointidlist->SetId(3, temp);
                  }

                  fourpoitostructmesh->SetPointList(pointidlist);
                  fourpoitostructmesh->Update();
                  vtkStructuredGrid *fourpoitostructmeshstore = vtkStructuredGrid::New();
                  fourpoitostructmeshstore->DeepCopy(fourpoitostructmesh->GetOutput());

                  //vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
                  //writer->SetInput(fourpoitostructmeshstore);
                  //writer->SetFileName("SGrid.vtk");
                  //writer->Write();
                  //writer->Delete();
                  // check if the edges of the faces are interior edges
                  // if yes, linearly interpolate the interior edges
                  // get the point ids of the face to be projected
                  // loop through all the edges
                  for (j =0; j < bboxcopy->GetCell(i)->GetFace
                          (idlistface->GetId(i))->GetNumberOfEdges(); j++) {
                                 vtkIdList *idlistedge = bboxcopy->GetCell(i)->GetFace
                                          (idlistface->GetId(i))->GetEdge(j)->GetPointIds();
                                 vtkIdList *firstpointcells = vtkIdList::New();
                                 vtkIdList *secondpointcells = vtkIdList::New();
                                 vtkIdType pt1 = idlistedge->GetId(0);
                                 vtkIdType pt2 = idlistedge->GetId(1);
                                 bboxcopy->GetPointCells(idlistedge->GetId(0), firstpointcells);
                                 bboxcopy->GetPointCells(idlistedge->GetId(1), secondpointcells);
                                 if(firstpointcells->GetNumberOfIds() == 1 || 
                                         secondpointcells->GetNumberOfIds() == 1)
                                 {
                                         // exterior edge
                                         // access the edge which needs to be projected
                                         vtkMimxExtractStructuredGridEdge *exedge0 = 
                                           vtkMimxExtractStructuredGridEdge::New();
                                         exedge0->SetEdgeNum(3-j);
                                         exedge0->SetInput(fourpoitostructmeshstore);
                                         exedge0->Update();
                                         ////////
                                         //vtkPolyDataWriter *writerp = vtkPolyDataWriter::New();
                                         //writerp->SetInput(exedge0->GetOutput());
                                         //writerp->SetFileName("Polydata.vtk");
                                         //writerp->Write();
                                         //writerp->Delete();
                                         ////////
                                         for(k=0; k < exedge0->GetOutput()->GetNumberOfPoints(); k++)
                                         {
                                                 celllocator->FindClosestPoint(exedge0->GetOutput()->GetPoint(k), ClosestPoint,
                                                         CellId,SubId,dist);
                                                 exedge0->GetOutput()->GetPoints()->SetPoint(k, ClosestPoint);
                                         }
                                         ////////
                                         //vtkPolyDataWriter *writerp1 = vtkPolyDataWriter::New();
                                         //writerp1->SetInput(exedge0->GetOutput());
                                         //writerp1->SetFileName("Polydata.vtk");
                                         //writerp1->Write();
                                         //writerp1->Delete();
                                         ////////

                                         // set the projected edge
                                         vtkMimxSetStructuredGridEdge *setedge0 = 
                                            vtkMimxSetStructuredGridEdge::New();
                                         setedge0->SetEdge(exedge0->GetOutput());
                                         setedge0->SetInput(fourpoitostructmeshstore);
                                         setedge0->SetEdgeNum(3-j);
                                         setedge0->Update();
                                         fourpoitostructmeshstore->Initialize();
//                                       fourpoitostructmesh->GetOutput()->GetPoints()->Initialize();
                                         fourpoitostructmeshstore->DeepCopy(setedge0->GetOutput());
//                                       fourpoitostructmesh->GetOutput()->GetPoints()->
//                                               DeepCopy(setedge0->GetOutput()->GetPoints());
        /*                               vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
                                         writer->SetInput(fourpoitostructmeshstore);
                                         writer->SetFileName("SGrid.vtk");
                                         writer->Write();
                                         writer->Delete();*/

                                         fourpoitostructmeshstore->SetDimensions
                                                 (setedge0->GetOutput()->GetDimensions());
                                         exedge0->Delete();
                                         setedge0->Delete();
                                 }
                                 firstpointcells->Delete();
                                 secondpointcells->Delete();
                  }

                  for (j =0; j < bboxcopy->GetCell(i)->GetFace
                          (idlistface->GetId(i))->GetNumberOfEdges(); j++)
                  {
                                  vtkIdList *idlistedge = bboxcopy->GetCell(i)->GetFace
                                          (idlistface->GetId(i))->GetEdge(j)->GetPointIds();
                                  vtkIdList *firstpointcells = vtkIdList::New();
                                  vtkIdList *secondpointcells = vtkIdList::New();
                                  bboxcopy->GetPointCells(idlistedge->GetId(0), firstpointcells);
                                  bboxcopy->GetPointCells(idlistedge->GetId(1), secondpointcells);
                                  if(firstpointcells->GetNumberOfIds() != 1 && 
                                          secondpointcells->GetNumberOfIds() != 1)
                                  {      
                                          // interior edge
                                          // get the points on the interior edge
                                          vtkMimxExtractStructuredGridEdge *exedge0 = 
                                            vtkMimxExtractStructuredGridEdge::New();
                                          exedge0->SetEdgeNum(3-j);
                                          exedge0->SetInput(fourpoitostructmeshstore);
                                          exedge0->Update();
                                          // first and last 
                                          vtkPoints *twopoints = vtkPoints::New();
                                          twopoints->SetNumberOfPoints(2);
                                          twopoints->SetPoint(0,exedge0->GetOutput()->GetPoint(0));
                                          twopoints->SetPoint(1,exedge0->GetOutput()->GetPoint
                                                  (exedge0->GetOutput()->GetNumberOfPoints()-1));
                                          vtkPolyData *polydata = vtkPolyData::New();
                                          polydata->SetPoints(twopoints);
                                          vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                                          subdivide->SetInput(polydata);
                                          subdivide->SetNumberOfDivisions
                                                  (exedge0->GetOutput()->GetNumberOfPoints()-1);
                                          subdivide->Update();
                                          // set the recalculated edge
                                          vtkMimxSetStructuredGridEdge *setedge0 = 
                                            vtkMimxSetStructuredGridEdge::New();
                                          setedge0->SetEdge(subdivide->GetOutput());
                                          setedge0->SetEdgeNum(3-j);
                                          setedge0->SetInput(fourpoitostructmeshstore);
                                          setedge0->Update();

                                          fourpoitostructmeshstore->Initialize();
//                                        fourpoitostructmesh->GetOutput()->GetPoints()->Initialize();
                                          fourpoitostructmeshstore->DeepCopy(setedge0->GetOutput());
//                                        fourpoitostructmesh->GetOutput()->GetPoints()->
//                                                DeepCopy(setedge0->GetOutput()->GetPoints());
                                          fourpoitostructmeshstore->SetDimensions
                                                  (setedge0->GetOutput()->GetDimensions());

                                          //vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
                                          //writer->SetInput(fourpoitostructmeshstore);
                                          //writer->SetFileName("SGrid.vtk");
                                          //writer->Write();
                                          //writer->Delete();

                                          exedge0->Delete();
                                          setedge0->Delete();
                                          twopoints->Delete();
                                          polydata->Delete();
                                          subdivide->Delete();                                                                    
                                  }
                                  firstpointcells->Delete();
                                  secondpointcells->Delete();
                  }
                  
                  // recalculate the interior nodes of the structured planar mesh
                  vtkMimxPlanarTransfiniteInterpolation *planeinterp = 
                     vtkMimxPlanarTransfiniteInterpolation::New();
                  vtkPointSet *poiset = vtkUnstructuredGrid::New();
                  poiset->SetPoints(fourpoitostructmeshstore->GetPoints());
                  planeinterp->SetInput(poiset);
                  int dim[3];
                  fourpoitostructmeshstore->GetDimensions(dim);
                  planeinterp->SetIDiv(dim[0]);
                  planeinterp->SetJDiv(dim[1]);
                  planeinterp->Update();
                  fourpoitostructmeshstore->GetPoints()->Initialize();
                  fourpoitostructmeshstore->GetPoints()->DeepCopy
                          (planeinterp->GetOutput()->GetPoints());

                  //vtkStructuredGridWriter *writer7 = vtkStructuredGridWriter::New();
                  //writer7->SetInput(fourpoitostructmeshstore);
                  //writer7->SetFileName("SGrid.vtk");
                  //writer7->Write();
                  //writer7->Delete();

                  // move the nodes on the input so that the co-ordinates
                  // correspond to the posterior elements
                  for(j =0; j< fourpoitostructmesh->GetCorrespondingPointList()->
                        GetNumberOfIds(); j++)
                  {
                        this->ModifiedInput->GetPoints()->SetPoint(fourpoitostructmesh->GetCorrespondingPointList()->
                                GetId(j), planeinterp->GetOutput()->GetPoint(j));
                  }

                  SGridPlaneCollection->AddItem((vtkObject*)vtkStructuredGrid::New());
                  ((vtkStructuredGrid*)SGridPlaneCollection->GetItemAsObject
                          (SGridPlaneCollection->GetNumberOfItems()-1))->
                          DeepCopy(planeinterp->GetOutput());
                  // change the mesh seed of the box structure so as to correspond to
                  // that of the structured grids based on face numbers
                  fourpoitostructmesh->GetOutput()->GetDimensions(dim);
                  ((vtkStructuredGrid*)SGridPlaneCollection->GetItemAsObject
                          (SGridPlaneCollection->GetNumberOfItems()-1))->SetDimensions(dim);

                  ((vtkStructuredGrid*)SGridPlaneCollection->GetItemAsObject
                          (SGridPlaneCollection->GetNumberOfItems()-1))->
                          SetPoints(planeinterp->GetOutput()->GetPoints());
                  // designate that the nodes on the particular face to be constrained  
                  vtkIntArray *intarray = vtkIntArray::New();
                  intarray->SetNumberOfValues(dim[0]*dim[1]*dim[2]);
                  intarray->SetNumberOfComponents(1);
                        
                  int status[1]; status[0] = 0;
                  for (int m=0; m<dim[0]*dim[1]*dim[2]; m++)    intarray->SetTupleValue(m,status);
                  ((vtkStructuredGrid*)SGridPlaneCollection->GetItemAsObject
                          (SGridPlaneCollection->GetNumberOfItems()-1))->
                          GetPointData()->SetScalars(intarray);
                  intarray->Delete();
                  if(idlistface->GetId(i) == 0 || idlistface->GetId(i) ==1)
                  {
                        meshseed->ChangeMeshSeed(i, 0, dim[0]);
                        meshseed->ChangeMeshSeed(i, 1, dim[1]);
                  }
                  else{
                          if(idlistface->GetId(i) == 2 || idlistface->GetId(i) ==3)
                          {
                                  meshseed->ChangeMeshSeed(i, 2, dim[0]);
                                  meshseed->ChangeMeshSeed(i, 1, dim[1]);
                          }
                          else
                          {
                                  meshseed->ChangeMeshSeed(i, 2, dim[0]);
                                  meshseed->ChangeMeshSeed(i, 0, dim[1]);
                          }
                  }

                  planeinterp->Delete();
                  poiset->Delete();
                  celllocator->Delete();
                  fourpoitostructmeshstore->Delete();
                  fourpoitostructmesh->Delete();
                  pointidlist->Delete();
          }
  }
////////
//  meshseed->ChangeMeshSeed(0, 2, 9);
//  meshseed->ChangeMeshSeed(4, 2, 9);
  //vtkUnstructuredGridWriter *unwriter = vtkUnstructuredGridWriter::New();
  //unwriter->SetInput(this->ModifiedInput);
  //unwriter->SetFileName("UGrid.vtk");
  //unwriter->Write();
///////
  // generate the rectilinear grids from the modified mesh seeding
  // and the modified unstructuredgrid
  vtkMimxBoundingBoxToStructuredGrids *bboxtosgrid = 
    vtkMimxBoundingBoxToStructuredGrids::New();
  bboxtosgrid->SetInput(bboxcopy);
  //--------------
  //bboxtosgrid->SetMeshSeed(arr);
  //bboxtosgrid->Update();
  //------------------
  int countplanesgrid = -1;
  // set the faces of the grids with the previously computed surface
  // structured meshes.
  vtkGeometryFilter *bboxgeofil = vtkGeometryFilter::New();
  bboxgeofil->SetInput(bboxcopy);
  bboxgeofil->Update();
  ////////
  //vtkUnstructuredGridWriter *unwriter1 = vtkUnstructuredGridWriter::New();
  //unwriter1->SetInput(bboxcopy);
  //unwriter1->SetFileName("UGrid1.vtk");
  //unwriter1->Write();
  //unwriter1->Delete();
  ///////
  cout <<"morphing structured grids"<<endl;
  for (i =0; i < bbox->GetNumberOfCells(); i++)
          {
                  cout <<"Cell Num : " <<i<<endl;
                  if(idlistface->GetId(i) != -1)
                  {
                          countplanesgrid++;
                          ///////
                          //vtkStructuredGridWriter *writer1 = vtkStructuredGridWriter::New();
                          //writer1->SetInput(bboxtosgrid->GetStructuredGrid(i));
                          //writer1->SetFileName("SGrid1.vtk");
                          //writer1->Write();
                          //writer1->Delete();
                          //////////            

                        vtkMimxMorphStructuredGrid *morphsgrid = vtkMimxMorphStructuredGrid::New();
                        morphsgrid->SetGridStructure(bboxcopy);
                        morphsgrid->SetInput(bboxtosgrid->GetStructuredGrid(i));
                        morphsgrid->SetSource(complete);
                        morphsgrid->SetCellNum(i);
                        morphsgrid->Update();
                        
                        ///////
                        //vtkStructuredGridWriter *writer2 = vtkStructuredGridWriter::New();
                        //writer2->SetInput(morphsgrid->GetOutput());
                        //writer2->SetFileName("SGrid2.vtk");
                        //writer2->Write();
                        //writer2->Delete();
                        //////////              
                        ///////
                        //vtkStructuredGridWriter *writer6 = vtkStructuredGridWriter::New();
                        //writer6->SetInput(((vtkStructuredGrid*)SGridPlaneCollection->GetItemAsObject
                        //      (countplanesgrid)));
                        //writer6->SetFileName("SGrid6.vtk");
                        //writer6->Write();
                        //writer6->Delete();
                        //////////              

                        // after morphing using regular technique
                        // we need to replace the previously computed surface mesh
                        // to a face of structured grid
                        vtkMimxSetStructuredGridFace *setstructgridface = 
                          vtkMimxSetStructuredGridFace::New();
                        setstructgridface->SetInput(morphsgrid->GetOutput());
                        setstructgridface->SetFace(((vtkStructuredGrid*)SGridPlaneCollection->GetItemAsObject
                                (countplanesgrid)));
                        setstructgridface->SetFaceNum(idlistface->GetId(i));
                        setstructgridface->Update();
                        ///////
                        //vtkStructuredGridWriter *writer3 = vtkStructuredGridWriter::New();
                        //writer3->SetInput(setstructgridface->GetOutput());
                        //writer3->SetFileName("SGrid3.vtk");
                        //writer3->Write();
                        //writer3->Delete();
                        //////////              

                        vtkStructuredGrid *currsgrid = vtkStructuredGrid::New();
                        currsgrid->DeepCopy(setstructgridface->GetOutput());
                        //recompute the interior edges that are affected
                        for(k=0; k < 12; k++)
                        {
                                if(!this->CheckInteriorEdge(bboxcopy->GetCell(i)->
                                        GetEdge(k)->GetPointIds(),bboxgeofil->GetOutput()))
                                {

                                        vtkMimxExtractStructuredGridEdge *exedge0 = 
                                          vtkMimxExtractStructuredGridEdge::New();
                                        exedge0->SetEdgeNum(k);
                                        exedge0->SetInput(currsgrid);
                                        exedge0->Update();
                                        // first and last 
                                        vtkPoints *twopoints = vtkPoints::New();
                                        twopoints->SetNumberOfPoints(2);
                                        twopoints->SetPoint(0,exedge0->GetOutput()->GetPoint(0));
                                        twopoints->SetPoint(1,exedge0->GetOutput()->GetPoint
                                                (exedge0->GetOutput()->GetNumberOfPoints()-1));
                                        vtkPolyData *polydata = vtkPolyData::New();
                                        polydata->SetPoints(twopoints);
                                        vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                                        subdivide->SetInput(polydata);
                                        subdivide->SetNumberOfDivisions
                                                (exedge0->GetOutput()->GetNumberOfPoints()-1);
                                        subdivide->Update();
                                        // set the recalculated edge
                                        vtkMimxSetStructuredGridEdge *setedge0 = 
                                           vtkMimxSetStructuredGridEdge::New();
                                        setedge0->SetEdge(subdivide->GetOutput());
                                        setedge0->SetEdgeNum(k);
                                        setedge0->SetInput(currsgrid);
                                        setedge0->Update();
                                        currsgrid->Initialize();
                                        currsgrid->DeepCopy(setedge0->GetOutput());

                                        exedge0->Delete();
                                        setedge0->Delete();
                                        twopoints->Delete();
                                        polydata->Delete();
                                        subdivide->Delete();                                                                      
                                }
                        }
                        ///////
                        //vtkStructuredGridWriter *writer5 = vtkStructuredGridWriter::New();
                        //writer5->SetInput(currsgrid);
                        //writer5->SetFileName("SGrid5.vtk");
                        //writer5->Write();
                        //writer5->Delete();
                        //////////              

                        // recompute the interior points of the faces with changed edges
                        for (k=0; k < 6; k++) {
                                if(idlistface->GetId(i) != -1)
                                {
                                        if(!this->CheckBoundaryFace(bboxcopy->GetCell(i)->
                                                GetFace(k)->GetPointIds(),bboxgeofil->GetOutput()))
                                        {
                                                vtkMimxExtractStructuredGridFace *exface0 = 
                                                  vtkMimxExtractStructuredGridFace::New();
                                                exface0->SetInput(currsgrid);
                                                exface0->SetFaceNum(k);
                                                exface0->Update();

                                                // recalculate the interior nodes of the structured planar mesh
                                                vtkMimxPlanarTransfiniteInterpolation *planeinterp =
                                                  vtkMimxPlanarTransfiniteInterpolation::New();
                                                vtkPointSet *poiset = vtkUnstructuredGrid::New();
                                                poiset->SetPoints(exface0->GetOutput()->GetPoints());
                                                planeinterp->SetInput(poiset);
                                                int dim[3];
                                                exface0->GetOutput()->GetDimensions(dim);
                                                planeinterp->SetIDiv(dim[0]);
                                                planeinterp->SetJDiv(dim[1]);
                                                planeinterp->Update();
                                                
                                                vtkStructuredGrid * splanegrid = vtkStructuredGrid::New();
                                                splanegrid->SetPoints(planeinterp->GetOutput()->GetPoints());
                                                splanegrid->SetDimensions(dim);

                                                vtkMimxSetStructuredGridFace *setface0 = 
                                                  vtkMimxSetStructuredGridFace::New();
                                                setface0->SetInput(currsgrid);
                                                setface0->SetFace(splanegrid);
                                                setface0->SetFaceNum(k);
                                                setface0->Update();
                                                
                                                currsgrid->Initialize();
                                                currsgrid->DeepCopy(setface0->GetOutput());

                                                exface0->Delete();
                                                planeinterp->Delete();
                                                poiset->Delete();
                                                setface0->Delete();
                                                splanegrid->Delete();
                                                ///////
                                                //vtkStructuredGridWriter *writer4 = vtkStructuredGridWriter::New();
                                                //writer4->SetInput(currsgrid);
                                                //writer4->SetFileName("SGrid4.vtk");
                                                //writer4->Write();
                                                //writer4->Delete();
                                                //////////              

                                        }
                                }

                        }
                        // store the sgrid in the sgridcollection
                        SGridCollection->AddItem((vtkObject*)vtkStructuredGrid::New());
                        ((vtkStructuredGrid*)SGridCollection->GetItemAsObject
                                (SGridCollection->GetNumberOfItems()-1))->
                                DeepCopy(currsgrid);
                        //vtkStructuredGridWriter *writer4 = vtkStructuredGridWriter::New();
                        //writer4->SetInput(currsgrid);

                        //char name[10];
                        //strcpy(name, "S");
                        //char buffer[10];
                        //char fileexten[10];
                        //sprintf(buffer, "%d", i);
                        //strcpy(fileexten,".vtk");
                        //strcat(name, buffer);
                        //strcat(name, fileexten);
                        //writer4->SetFileName(name);
                        //writer4->Write();
                        //writer4->Delete();
                        ////////////            

                        currsgrid->Delete();
                        morphsgrid->Delete();
                        setstructgridface->Delete();

                }
                  else
                  {
                          vtkMimxMorphStructuredGrid *morphsgrid = vtkMimxMorphStructuredGrid::New();
                          morphsgrid->SetGridStructure(bboxcopy);
                          morphsgrid->SetInput(bboxtosgrid->GetStructuredGrid(i));
                          morphsgrid->SetSource(complete);
                          morphsgrid->SetCellNum(i);
                          morphsgrid->Update();

                          ///////
                          SGridCollection->AddItem((vtkObject*)vtkStructuredGrid::New());
                          ((vtkStructuredGrid*)SGridCollection->GetItemAsObject
                                  (SGridCollection->GetNumberOfItems()-1))->
                                  DeepCopy(morphsgrid->GetOutput());

                          //vtkStructuredGridWriter *writer4 = vtkStructuredGridWriter::New();
                          //writer4->SetInput(morphsgrid->GetOutput());

                          //char name[10];
                          //strcpy(name, "S");
                          //char buffer[10];
                          //char fileexten[10];
                          //sprintf(buffer, "%d", i);
                          //strcpy(fileexten,".vtk");
                          //strcat(name, buffer);
                          //strcat(name, fileexten);
                          //writer4->SetFileName(name);
                          //writer4->Write();
                          //writer4->Delete();
                          //////////            

                          morphsgrid->Delete();                                           
                  }
          }


  if (SGridPlaneCollection)
  {
          int numCol = SGridPlaneCollection->GetNumberOfItems();
          SGridPlaneCollection->InitTraversal();
          do 
          {
                  SGridPlaneCollection->GetNextItemAsObject()->Delete();
                  numCol--;
          } while(numCol != 0);
          SGridPlaneCollection->Delete();
  } 
        int numele = 0;
        int numnodes = 0;
        int dim[3];
        //      calculate number of nodes and elements
        for(int i=0; i <this->SGridCollection->GetNumberOfItems(); i++)
        {
                ((vtkStructuredGrid*)(this->SGridCollection->
                        GetItemAsObject(i)))->GetDimensions(dim);
                numnodes = numnodes + dim[0]*dim[1]*dim[2];
                numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
        }
  
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        vtkMergeCells* mergecells = vtkMergeCells::New();
        mergecells->SetUnstructuredGrid(ugrid);
        mergecells->SetPointMergeTolerance(0.001);
        mergecells->MergeDuplicatePointsOn();
        mergecells->SetTotalNumberOfDataSets(this->SGridCollection->GetNumberOfItems());
        mergecells->SetTotalNumberOfCells(numele);
        mergecells->SetTotalNumberOfPoints(numnodes);
        for(int i=0; i <this->SGridCollection->GetNumberOfItems(); i++)
        {
                mergecells->MergeDataSet(((vtkStructuredGrid*)(this->SGridCollection->
                        GetItemAsObject(i))));
        }
        mergecells->Finish();
        int numeleact = ugrid->GetNumberOfPoints();
        output->DeepCopy(ugrid);
        vtkUnstructuredGridWriter* writerc = vtkUnstructuredGridWriter::New();
        writerc->SetInput(ugrid);
        writerc->SetFileName("Combined.vtk");
        writerc->Write();
        ugrid->Delete();
//  int dim[3], m;
//  int numele = 0;
//  int numnode = 0;
//  for(i=0; i <this->SGridCollection->GetNumberOfItems(); i++)
//  {
//        ((vtkStructuredGrid*)(this->SGridCollection->
//                GetItemAsObject(i)))->GetDimensions(dim);
//        numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
//        numnode = numnode + dim[0]*dim[1]*dim[2];
//  }
//
//  // start with the other bounding boxes
//  // check if a dataset contains coincident points
//  vtkPointLocator *globallocator = vtkPointLocator::New();
//  vtkPoints *globalpoints = vtkPoints::New();
//  globalpoints->Allocate(numnode);
//  output->Allocate(numele);
//  globallocator->InitPointInsertion(globalpoints, input->GetPoints()->GetBounds());
//  
//  vtkIntArray *intarray = vtkIntArray::New();
//  intarray->SetNumberOfComponents(1);
//
//  int startnodenum;
//  for(m=0; m < bboxcopy->GetNumberOfCells(); m++)
//
//  {
//        vtkPoints* sgridpoints = ((vtkStructuredGrid*)(this->SGridCollection->
//                GetItemAsObject(m)))->GetPoints();
//
//        ((vtkStructuredGrid*)(this->SGridCollection->
//                GetItemAsObject(m)))->GetDimensions(dim);
//
//        // create the global point id list for the individual points in the data set
//        // and insert unique points belonging to different data sets
//        vtkIdList *poirenum = vtkIdList::New();
//        startnodenum = globalpoints->GetNumberOfPoints();
//
//        for (i=0; i < sgridpoints->GetNumberOfPoints(); i++)
//        {
//                // if point does not exist
//                if(globallocator->IsInsertedPoint(sgridpoints->GetPoint(i)) == -1)
//                {
//                        poirenum->InsertNextId(globalpoints->GetNumberOfPoints());
//                        globallocator->InsertNextPoint(sgridpoints->GetPoint(i));
//                        double status[1];
//                        ((vtkStructuredGrid*)(this->SGridCollection->
//                                GetItemAsObject(m)))->GetPointData()->GetScalars()->GetTuple(i,status);
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
//                                ((vtkStructuredGrid*)(this->SGridCollection->
//                                        GetItemAsObject(m)))->GetPointData()->GetScalars()->GetTuple(i,status);
//                                intarray->InsertNextTuple1(status[0]);
//                        }
//                }
//        }
//        //for (i=0; i < poirenum->GetNumberOfIds(); i++)
//        //{
//               // //////cout <<i<<"  "<<poirenum->GetId(i)<<endl;
//        //}
//
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
////                              //////cout << ptids->GetId(0)<<"  "<< ptids->GetId(1)<<"  "<< ptids->GetId(2)<<"  "<< ptids->GetId(3)<<"  "<< ptids->GetId(4)<<"  "<< ptids->GetId(5)<<"  "<< ptids->GetId(6)<<"  "<< ptids->GetId(7)<<endl;
//                        }
//                }
//        }
//        ptids->Delete();
//        poirenum->Delete();
//  }
//
//  output->GetPointData()->SetScalars(intarray);
//  intarray->Delete();
  //output->SetPoints(globalpoints);
  //output->Squeeze();
  //globalpoints->Delete();
  //globallocator->Delete();

vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
writer->SetInput(bboxcopy);
writer->SetFileName("ModifiedMeshSeeds.vtk");
writer->Write();
writer->Delete();
  fil->Delete();
  bboxgeofil->Delete();
  locator->Delete();
  patch->Delete();
  idlistface->Delete();
  bboxcopy->Delete();
  bboxtosgrid->Delete();
  if(!arrstatus) arr->Delete();
  delete meshseed;
  return 1;
  
}

int vtkMimxAttachBoundingBoxMesh::FillInputPortInformation(int port, vtkInformation *info)
{
        
        if (port == 0 )
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
                return 1;
        }
        if(port == 1)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
                return 1;
        }
        if(port == 2 )
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
                return 1;
        }
        if(port == 3 )
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
                return 1;
        }
        return 0;
}

void vtkMimxAttachBoundingBoxMesh::SetUnstructuredMesh(vtkUnstructuredGrid * Input)
{
        this->SetInput(0, Input);
}

void vtkMimxAttachBoundingBoxMesh::SetBoundingBox(vtkUnstructuredGrid *BBox)
{
        this->SetInput(1, BBox);
}

void vtkMimxAttachBoundingBoxMesh::SetCompleteSource(vtkPolyData *CompleteSource)
{
        this->SetInput(2, CompleteSource);
}

void vtkMimxAttachBoundingBoxMesh::SetCutSource(vtkPolyData *CutSource)
{
        this->SetInput(3, CutSource);
}

int vtkMimxAttachBoundingBoxMesh::GetClosestContour(
  vtkMimxPatchSurfaceMesh* Patch, 
  vtkUnstructuredGrid* BBox, 
  vtkIdList* PointIdList)
{
        int closestcontour;
        double distance;
        double distancemin = 1e10;
        for (int i=0; i < Patch->GetNumberOfContours(); i++) {
                distance = 0.0;
                for (int j =0; j < PointIdList->GetNumberOfIds(); j++) {
                        distance = distance + sqrt(vtkMath::Distance2BetweenPoints(BBox->GetPoint(
                                PointIdList->GetId(j)), Patch->GetContour(i)->GetCenter()));
                }
                distance = distance/PointIdList->GetNumberOfIds();
                if(distance < distancemin) 
                {
                        distancemin = distance;
                        closestcontour = i;
                }
        }
        return closestcontour;
}
void vtkMimxAttachBoundingBoxMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

int vtkMimxAttachBoundingBoxMesh::CheckBoundaryFace(vtkIdList* IdList, vtkPolyData *BoundaryData)
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

int vtkMimxAttachBoundingBoxMesh::CheckInteriorEdge(vtkIdList* IdList, vtkPolyData *BoundaryData)
{
        vtkIdList *idlist2;
        vtkIdType pt1,pt2;
        pt1 = IdList->GetId(0); pt2 = IdList->GetId(1);
        for(int i=0; i < BoundaryData->GetNumberOfCells(); i++)
        {
                for(int j=0;  j < BoundaryData->GetCell(i)->GetNumberOfEdges(); j++)
                {
                        idlist2 = BoundaryData->GetCell(i)->GetEdge(j)->GetPointIds();

                        vtkIdType p1,p2;
                        p1 = idlist2->GetId(0); p2 = idlist2->GetId(1);

                        if(idlist2->IsId(IdList->GetId(0)) != -1 && idlist2->IsId(IdList->GetId(1)) != -1)
                        {
                                return 1;
                        }
                }
        }
        return 0;
}
