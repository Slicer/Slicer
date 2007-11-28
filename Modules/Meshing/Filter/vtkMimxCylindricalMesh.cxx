/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxCylindricalMesh.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
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

#include "vtkMimxCylindricalMesh.h"

#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkDataSetAttributes.h"
#include "vtkDataSetCollection.h"
#include "vtkMimxEdgeToPlanarStructuredGrid.h"
#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkMergeCells.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPolyDataWriter.h"
#include "vtkPlaneWidget.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxSubdivideCurve.h"
#include "vtkStructuredGridWriter.h"
#include "vtkStructuredGridReader.h"
#include "vtkMimxStructuredPlanarToStructuredSolidGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridWriter.h"

vtkCxxRevisionMacro(vtkMimxCylindricalMesh, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkMimxCylindricalMesh);

//----------------------------------------------------------------------------
vtkMimxCylindricalMesh::vtkMimxCylindricalMesh()
{
  this->SetNumberOfInputPorts(6);
  for(int i=0; i < 9; i++)
  {
          this->StructuredSolidGrid[i] = vtkStructuredGrid::New();
  }
  for(int i=0; i < 40; i++)
  {
          this->StructuredPlanarGrid[i] = vtkStructuredGrid::New();
  }
  for(int i=0; i<60; i++)
  {
          this->EdgePolyData[i] = vtkPolyData::New();
  }
  //this->SuperiorDividingPoints = vtkPoints::New();
  //this->InferiorDividingPoints = vtkPoints::New();
}

//----------------------------------------------------------------------------
vtkMimxCylindricalMesh::~vtkMimxCylindricalMesh()
{
        for(int i=0; i < 9; i++)
        {
                this->StructuredSolidGrid[i]->Delete();
        }

        for(int i=0; i < 40; i++)
        {
                this->StructuredPlanarGrid[i]->Delete();
        }

        for(int i=0; i<60; i++)
        {
                this->EdgePolyData[i]->Delete();
        }
        //this->SuperiorDividingPoints->Delete();
        //this->InferiorDividingPoints->Delete();
}

//----------------------------------------------------------------------------
// Append data sets into single unstructured grid
int vtkMimxCylindricalMesh::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the output info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *source = vtkPolyData::SafeDownCast(inputVector[4]->
          GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));
  vtkCellLocator *locator = vtkCellLocator::New();

  vtkUnstructuredGrid *bbox = vtkUnstructuredGrid::SafeDownCast(inputVector[5]->
          GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));

  double dim[3];
  bbox->GetCellData()->GetVectors()->GetTuple(0,dim);
  this->MeshSeed[0] = static_cast<int>(dim[2])-1;
  this->MeshSeed[1] = static_cast<int>(dim[0])-1;
  this->MeshSeed[2] = static_cast<int>(dim[1])-1;

  bbox->GetCellData()->GetVectors()->GetTuple(1,dim);
  this->MeshSeed[3] = static_cast<int>(dim[0])-1;

  bbox->GetCellData()->GetVectors()->GetTuple(4,dim);
  this->MeshSeed[4] = static_cast<int>(dim[2])-1;

  locator->SetDataSet(source);
  locator->BuildLocator();
// temporary input
  vtkInformation *inInfo  = NULL;
  inInfo = inputVector[0]->GetInformationObject(0);
  
//  this->EdgePolyData[0] = vtkPolyData::New();
//  this->EdgePolyData[1] = vtkPolyData::SafeDownCast(
//          inputVector[3]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));
//  this->EdgePolyData[2] = vtkPolyData::New();
//  this->EdgePolyData[3] = vtkPolyData::SafeDownCast(
//          inputVector[2]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));

//  this->EdgePolyData[4] = vtkPolyData::New();
//  this->EdgePolyData[5] = vtkPolyData::SafeDownCast(
//          inputVector[1]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));
//  this->EdgePolyData[6] = vtkPolyData::New();
//  this->EdgePolyData[7] = vtkPolyData::SafeDownCast(
//          inputVector[0]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));
//  for(int i=8; i < 12; i++)
//                 this->EdgePolyData[i] = vtkPolyData::New();

 /* vtkPolyData::SafeDownCast(
          inputVector[0]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()))->GetCenter(SupCenter);
  vtkPolyData::SafeDownCast(
          inputVector[2]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()))->GetCenter(InfCenter);*/

  double bottomcenter[3], topcenter[3];

  this->CalculateCenter(vtkPolyData::SafeDownCast(
          inputVector[0]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT())));
 for (int i=0; i <3; i++) {
         this->SupCenter[i] = this->Center[i];
         topcenter[i] = this->Center[i];
 }

 this->CalculateCenter(vtkPolyData::SafeDownCast(
         inputVector[2]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT())));
 for (int i=0; i <3; i++) {
         bottomcenter[i] = this->Center[i];
         this->InfCenter[i] = this->Center[i];
 }

  for(int i=0; i < 4; i++)
  {
          //if(i==0 || i ==2)
          //{
                 // this->CalculateCenter(vtkPolyData::SafeDownCast(
                        //  inputVector[i]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT())));
          //}
        this->DivideCurve(i,vtkPolyData::SafeDownCast(
                    inputVector[i]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT())), bbox);
  }
  //  block 0
  //  subdivide the known edges if needed project
  //  curve 1
        this->EdgeSubdivideProject(1,this->MeshSeed[1],locator);
        this->SetEdgeScalars(1,0);
  // curve 3
        this->EdgeSubdivideProject(3,this->MeshSeed[1],locator);
        this->SetEdgeScalars(3,0);
  // curve 5
        this->EdgeSubdivideProject(5,this->MeshSeed[1],locator);
        this->SetEdgeScalars(5,0);
  //  curve 7
        this->EdgeSubdivideProject(7,this->MeshSeed[1],locator);
        this->SetEdgeScalars(7,0);
  // generate curves for by connecting the known curves subdivide and 
  // if needed project
  // curve 0
        this->GenerateUnknownEdge(0, 3, 1, 0);
        this->EdgeSubdivideProject(0, this->MeshSeed[0],locator);
        this->SetEdgeScalars(0,1);
  //  curve 2
        this->GenerateUnknownEdge(2, 3, 1, 1);
        this->EdgeSubdivideProject(2, this->MeshSeed[0],locator);
        this->SetEdgeScalars(2,1);
        //  curve 4
        this->GenerateUnknownEdge(4, 7, 5, 0);
        this->EdgeSubdivideProject(4, this->MeshSeed[0],locator);
        this->SetEdgeScalars(4,1);
        //  curve 6
        this->GenerateUnknownEdge(6, 7, 5, 1);
        this->EdgeSubdivideProject(6, this->MeshSeed[0],locator);
        this->SetEdgeScalars(6,1);
        //  curve 8
        this->GenerateUnknownEdge(8, 3, 7, 0);
        this->EdgeSubdivideProject(8, this->MeshSeed[2],NULL);
        this->SetEdgeScalars(8,1);
        //  curve 9
        this->GenerateUnknownEdge(9, 1, 5, 0);
        this->EdgeSubdivideProject(9, this->MeshSeed[2],locator);
        this->SetEdgeScalars(9,1);
        //  curve 10
        this->GenerateUnknownEdge(10, 3, 7, 1);
        this->EdgeSubdivideProject(10, this->MeshSeed[2],NULL);
        this->SetEdgeScalars(10,1);
        //  curve 11
        this->GenerateUnknownEdge(11, 1, 5, 1);
        this->EdgeSubdivideProject(11, this->MeshSeed[2],locator);
        this->SetEdgeScalars(11,1);
        // generate face from 4 curves
        // face 0 through 5
        this->GenerateFaceProject(0,3,10,7,8,NULL);
        this->GenerateFaceProject(1,1,11,5,9,locator);
        this->GenerateFaceProject(2,0,9,4,8,NULL);
        this->GenerateFaceProject(3,2,11,6,10,NULL);
        this->GenerateFaceProject(4,0,1,2,3,locator);
        this->SetFaceScalars(4,0);
        this->GenerateFaceProject(5,4,5,6,7,locator);
        this->SetFaceScalars(5,0);
        // generate the solid mesh for first bounding box
        int FaceArray[6];
        for(int i=0; i<6; i++)        FaceArray[i] = i;
        this->GenerateSolidMesh(0,FaceArray);
        //**************************************************************
        //  block 1
        //  subdivide the known edges if needed project
        //  curve 12
        this->EdgeSubdivideProject(12,this->MeshSeed[3],locator);
        this->SetEdgeScalars(12,0);
        // curve 14
        this->EdgeSubdivideProject(14,this->MeshSeed[3],locator);
        this->SetEdgeScalars(14,0);
        // curve 15
        this->EdgeSubdivideProject(15,this->MeshSeed[3],locator);
        this->SetEdgeScalars(15,0);
        //  curve 17
        this->EdgeSubdivideProject(17,this->MeshSeed[3],locator);
        this->SetEdgeScalars(17,0);
        // generate curves for by connecting the known curves subdivide and 
        // if needed project
        //  curve 13
        this->GenerateUnknownEdge(13, 14, 12, 1);
        this->EdgeSubdivideProject(13, this->MeshSeed[0],locator);
        this->SetEdgeScalars(13,1);
        //  curve 16
        this->GenerateUnknownEdge(16, 17, 15, 1);
        this->EdgeSubdivideProject(16, this->MeshSeed[0],locator);
        this->SetEdgeScalars(16,1);
        //  curve 18
        this->GenerateUnknownEdge(18, 14, 17, 1);
        this->EdgeSubdivideProject(18, this->MeshSeed[2],NULL);
        this->SetEdgeScalars(18,1);
        //  curve 19
        this->GenerateUnknownEdge(19, 12, 15, 1);
        this->EdgeSubdivideProject(19, this->MeshSeed[2],locator);
        this->SetEdgeScalars(19,1);
        // generate face from 4 curves
        // face 0 through 5
        this->GenerateFaceProject(6,14,18,17,10,NULL);
        this->GenerateFaceProject(7,12,19,15,11,locator);
        this->GenerateFaceProject(8,13,19,16,18,NULL);
        this->GenerateFaceProject(9,2,12,13,14,locator);
        this->SetFaceScalars(9,0);
        this->GenerateFaceProject(10,6,15,16,17,locator);
        this->SetFaceScalars(10,0);
        // generate the solid mesh for first bounding box
        FaceArray[0] = 6; FaceArray[1] = 7; FaceArray[2] = 3;
        FaceArray[3] = 8; FaceArray[4] = 9; FaceArray[5] = 10;
        this->GenerateSolidMesh(1,FaceArray);

        //**************************************************************
        //  block 2
        //  subdivide the known edges if needed project
        //  curve 12
        this->EdgeSubdivideProject(20,this->MeshSeed[1],locator);
        this->SetEdgeScalars(20,0);
        // curve 14
        this->EdgeSubdivideProject(22,this->MeshSeed[1],locator);
        this->SetEdgeScalars(22,0);
        // curve 15
        this->EdgeSubdivideProject(23,this->MeshSeed[1],locator);
        this->SetEdgeScalars(23,0);
        //  curve 17
        this->EdgeSubdivideProject(25,this->MeshSeed[1],locator);
        this->SetEdgeScalars(25,0);
        // generate curves for by connecting the known curves subdivide and 
        // if needed project
        //  curve 2
        this->GenerateUnknownEdge(21, 22, 20, 1);
        this->EdgeSubdivideProject(21, this->MeshSeed[0],locator);
        this->SetEdgeScalars(21,1);
        //  curve 6
        this->GenerateUnknownEdge(24, 25, 23, 1);
        this->EdgeSubdivideProject(24, this->MeshSeed[0],locator);
        this->SetEdgeScalars(24,1);
        //  curve 18
        this->GenerateUnknownEdge(26, 22, 25, 1);
        this->EdgeSubdivideProject(26, this->MeshSeed[2],NULL);
        this->SetEdgeScalars(26,1);
        //  curve 19
        this->GenerateUnknownEdge(27, 20, 23, 1);
        this->EdgeSubdivideProject(27, this->MeshSeed[2],locator);
        this->SetEdgeScalars(27,1);
        // generate face from 4 curves
        // face 0 through 5
        this->GenerateFaceProject(11,22,26,25,18,NULL);
        this->GenerateFaceProject(12,20,27,23,19,locator);
        this->GenerateFaceProject(13,21,27,24,26,NULL);
        this->GenerateFaceProject(14,13,20,21,22,locator);
        this->SetFaceScalars(14,0);
        this->GenerateFaceProject(15,16,23,24,25,locator);
        this->SetFaceScalars(15,0);
        // generate the solid mesh for first bounding box
        FaceArray[0] = 11; FaceArray[1] = 12; FaceArray[2] = 8;
        FaceArray[3] = 13; FaceArray[4] = 14; FaceArray[5] = 15;
        this->GenerateSolidMesh(2,FaceArray);
        //**************************************************************
        //  block 3
        //  subdivide the known edges if needed project
        //  curve 12
        this->EdgeSubdivideProject(28,this->MeshSeed[3],locator);
        this->SetEdgeScalars(28,0);
        // curve 14
        this->EdgeSubdivideProject(29,this->MeshSeed[3],locator);
        this->SetEdgeScalars(29,0);
        // curve 15
        this->EdgeSubdivideProject(30,this->MeshSeed[3],locator);
        this->SetEdgeScalars(30,0);
        //  curve 17
        this->EdgeSubdivideProject(31,this->MeshSeed[3],locator);
        this->SetEdgeScalars(31,0);
        // generate face from 4 curves
        // face 0 through 5
        this->GenerateFaceProject(16,29,8,31,26,NULL);
        this->GenerateFaceProject(17,28,9,30,27,locator);
        this->GenerateFaceProject(18,21,28,0,29,NULL);
        this->SetFaceScalars(18,0);
        this->GenerateFaceProject(19,24,30,4,31,locator);
        this->SetFaceScalars(19,0);
        // generate the solid mesh for first bounding box
        FaceArray[0] = 16; FaceArray[1] = 17; FaceArray[2] = 13;
        FaceArray[3] = 2; FaceArray[4] = 18; FaceArray[5] = 19;
        this->GenerateSolidMesh(3,FaceArray);
        ////**************************************************************
        ////  block 4
        //// reverse the edges direction 14,17,22,25
        //this->ReverseEdgeDirection(14);        this->ReverseEdgeDirection(17);
        //this->ReverseEdgeDirection(22);        this->ReverseEdgeDirection(25);
        //// generate 4 new faces required
        //this->GenerateFaceProject(20,22,18,25,26,NULL);
        //this->GenerateFaceProject(21,14,10,17,18,NULL);
        //this->GenerateFaceProject(22,29,3,14,22,locator);
        //this->GenerateFaceProject(23,31,7,17,25,locator);
        //// generate the solid mesh for the center
        //FaceArray[0] = 20; FaceArray[1] = 0; FaceArray[2] = 16;
        //FaceArray[3] = 21; FaceArray[4] = 22; FaceArray[5] = 23;
        //this->GenerateSolidMesh(4,FaceArray);
        //*****************************************************************
        // generating inner blocks
        // calculate the the inner 8 points
        vtkPoints *innerpoints = vtkPoints::New();
        innerpoints->SetNumberOfPoints(8);
        // calculation of the centers of the traces
        //vtkPolyData::SafeDownCast(inputVector[0]->GetInformationObject(0)->
        //        Get(vtkDataObject::DATA_OBJECT()))->GetCenter(topcenter);
        //vtkPolyData::SafeDownCast(inputVector[2]->GetInformationObject(0)->
        //        Get(vtkDataObject::DATA_OBJECT()))->GetCenter(bottomcenter);
        // the four points are placed half way between the center and the
        // point connecting outer mesh
        //double x[3];
        //double ratio = 0.75;
        //int m;
        //this->EdgePolyData[0]->GetPoint(0, outerpoint);
        //for(m =0; m <3; m++)        x[m] = outerpoint[m] + (bottomcenter[m] - outerpoint[m])*ratio;
        //innerpoints->SetPoint(0, x);

        //this->EdgePolyData[2]->GetPoint(0, outerpoint);
        //for(m =0; m <3; m++)        x[m] = outerpoint[m] + (bottomcenter[m] - outerpoint[m])*ratio;
        //innerpoints->SetPoint(1, x);
        //
        //this->EdgePolyData[13]->GetPoint(0, outerpoint);
        //for(m =0; m <3; m++)        x[m] = outerpoint[m] + (bottomcenter[m] - outerpoint[m])*ratio;
        //innerpoints->SetPoint(2, x);
        //
        //this->EdgePolyData[21]->GetPoint(0, outerpoint);
        //for(m =0; m <3; m++)        x[m] = outerpoint[m] + (bottomcenter[m] - outerpoint[m])*ratio;
        //innerpoints->SetPoint(3, x);

        //this->EdgePolyData[4]->GetPoint(0, outerpoint);
        //for(m =0; m <3; m++)        x[m] = outerpoint[m] + (topcenter[m] - outerpoint[m])*ratio;
        //innerpoints->SetPoint(4, x);

        //this->EdgePolyData[6]->GetPoint(0, outerpoint);
        //for(m =0; m <3; m++)        x[m] = outerpoint[m] + (topcenter[m] - outerpoint[m])*ratio;
        //innerpoints->SetPoint(5, x);

        //this->EdgePolyData[16]->GetPoint(0, outerpoint);
        //for(m =0; m <3; m++)        x[m] = outerpoint[m] + (topcenter[m] - outerpoint[m])*ratio;
        //innerpoints->SetPoint(6, x);

        //this->EdgePolyData[24]->GetPoint(0, outerpoint);
        //for(m =0; m <3; m++)        x[m] = outerpoint[m] + (topcenter[m] - outerpoint[m])*ratio;
        //innerpoints->SetPoint(7, x);
        
        int startingnodenum = 8;
        for (int i=startingnodenum; i <startingnodenum+4; i++) {
                innerpoints->SetPoint(i-startingnodenum, bbox->GetPoint(i));
        }

        startingnodenum = 20;
        int startnum = 4;
        for (int i=startingnodenum; i <startingnodenum+4; i++) {
                innerpoints->SetPoint(startnum, bbox->GetPoint(i));
                startnum++;
        }

        // block 4
        // edge 32
        this->GenerateUnknownEdgeFromPoints(32, innerpoints->GetPoint(0), 
                this->EdgePolyData[0]->GetPoint(0));
        this->EdgeSubdivideProject(32, this->MeshSeed[4],locator);
        this->SetEdgeScalars(32, 1);

        // edge 33
        this->GenerateUnknownEdgeFromPoints(33, innerpoints->GetPoint(1), 
                this->EdgePolyData[2]->GetPoint(0));
        this->EdgeSubdivideProject(33, this->MeshSeed[4],locator);
        this->SetEdgeScalars(33, 1);

        // edge 34
        this->GenerateUnknownEdge(34, 32, 33, 0);
        this->EdgeSubdivideProject(34, this->MeshSeed[1],locator);
        this->SetEdgeScalars(34, 1);

        // edge 35
        this->GenerateUnknownEdgeFromPoints(35, innerpoints->GetPoint(4), 
                this->EdgePolyData[4]->GetPoint(0));
        this->EdgeSubdivideProject(35, this->MeshSeed[4],locator);
        this->SetEdgeScalars(35, 1);

        // edge 36
        this->GenerateUnknownEdgeFromPoints(36, innerpoints->GetPoint(5), 
                this->EdgePolyData[6]->GetPoint(0));
        this->EdgeSubdivideProject(36, this->MeshSeed[4],locator);
        this->SetEdgeScalars(36, 1);

        // edge 37
        this->GenerateUnknownEdge(37, 35, 36, 0);
        this->EdgeSubdivideProject(37, this->MeshSeed[1],locator);
        this->SetEdgeScalars(37, 1);

        // edge 38
        this->GenerateUnknownEdge(38, 32, 35, 0);
        this->EdgeSubdivideProject(38, this->MeshSeed[2],NULL);
        this->SetEdgeScalars(38, 1);

        // edge 39
        this->GenerateUnknownEdge(39, 33, 36, 0);
        this->EdgeSubdivideProject(39, this->MeshSeed[2],NULL);
        this->SetEdgeScalars(39, 1);
        //
        this->GenerateFaceProject(20,34,39,37,38,NULL);
        this->GenerateFaceProject(21,32,8,35,38,NULL);
        this->GenerateFaceProject(22,33,10,36,39,NULL);
        this->GenerateFaceProject(23,32,3,33,34,locator);
//        this->SetFaceScalars(23,0);
        this->GenerateFaceProject(24,35,7,36,37,locator);
//        this->SetFaceScalars(24,0);
        // generate the solid mesh for first bounding box
        FaceArray[0] = 20; FaceArray[1] = 0; FaceArray[2] = 21;
        FaceArray[3] = 22; FaceArray[4] = 23; FaceArray[5] = 24;
        this->GenerateSolidMesh(4,FaceArray);
        // block 5
        // edge 40
        this->GenerateUnknownEdgeFromPoints(40, innerpoints->GetPoint(2), 
                this->EdgePolyData[13]->GetPoint(0));
        this->EdgeSubdivideProject(40, this->MeshSeed[4],locator);
        this->SetEdgeScalars(40, 1);

        // edge 41
        this->GenerateUnknownEdge(41, 33, 40, 0);
        this->EdgeSubdivideProject(41, this->MeshSeed[3],locator);
        this->SetEdgeScalars(41, 1);

        // edge 42
        this->GenerateUnknownEdgeFromPoints(42, innerpoints->GetPoint(6), 
                this->EdgePolyData[16]->GetPoint(0));
        this->EdgeSubdivideProject(42, this->MeshSeed[4],locator);
        this->SetEdgeScalars(42, 1);

        // edge 43
        this->GenerateUnknownEdge(43, 36, 42, 0);
        this->EdgeSubdivideProject(43, this->MeshSeed[3],locator);
        this->SetEdgeScalars(43, 1);

        // edge 44
        this->GenerateUnknownEdge(44, 40, 42, 0);
        this->EdgeSubdivideProject(44, this->MeshSeed[2],NULL);
        this->SetEdgeScalars(44, 1);
        // face
        this->GenerateFaceProject(25,41,44,43,39,NULL);
        this->GenerateFaceProject(26,40,18,42,44,NULL);
        this->GenerateFaceProject(27,33,14,40,41,locator);
//        this->SetFaceScalars(27,0);
        this->GenerateFaceProject(28,36,17,42,43,locator);
//        this->SetFaceScalars(28,0);
        // generate the solid mesh for first bounding box
        FaceArray[0] = 25; FaceArray[1] = 6; FaceArray[2] = 22;
        FaceArray[3] = 26; FaceArray[4] = 27; FaceArray[5] = 28;
        this->GenerateSolidMesh(5,FaceArray);
        // block 6
        // edge 45
        this->GenerateUnknownEdgeFromPoints(45, innerpoints->GetPoint(3), 
                this->EdgePolyData[21]->GetPoint(0));
        this->EdgeSubdivideProject(45, this->MeshSeed[4],locator);
        this->SetEdgeScalars(45, 1);

        // edge 46
        this->GenerateUnknownEdge(46, 40, 45, 0);
        this->EdgeSubdivideProject(46, this->MeshSeed[1],locator);
        this->SetEdgeScalars(46, 1);

        // edge 47
        this->GenerateUnknownEdgeFromPoints(47, innerpoints->GetPoint(7), 
                this->EdgePolyData[24]->GetPoint(0));
        this->EdgeSubdivideProject(47, this->MeshSeed[4],locator);
        this->SetEdgeScalars(47, 1);

        // edge 48
        this->GenerateUnknownEdge(48, 42, 47, 0);
        this->EdgeSubdivideProject(48, this->MeshSeed[1],locator);
        this->SetEdgeScalars(48, 1);

        // edge 49
        this->GenerateUnknownEdge(49, 45, 47, 0);
        this->EdgeSubdivideProject(49, this->MeshSeed[2],NULL);
        this->SetEdgeScalars(49, 1);
        // face
        this->GenerateFaceProject(29,46,49,48,44,NULL);
        this->GenerateFaceProject(30,45,26,47,49,NULL);
        this->GenerateFaceProject(31,40,22,45,46,locator);
//        this->SetFaceScalars(31,0);
        this->GenerateFaceProject(32,42,25,47,48,locator);
//        this->SetFaceScalars(32,0);
        // generate the solid mesh for first bounding box
        FaceArray[0] = 29; FaceArray[1] = 11; FaceArray[2] = 26;
        FaceArray[3] = 30; FaceArray[4] = 31; FaceArray[5] = 32;
        this->GenerateSolidMesh(6,FaceArray);

        // block 7
        // edge 50
        this->GenerateUnknownEdge(50, 45, 32, 0);
        this->EdgeSubdivideProject(50, this->MeshSeed[3],locator);
        this->SetEdgeScalars(50, 1);

        // edge 51
        this->GenerateUnknownEdge(51, 47, 35, 0);
        this->EdgeSubdivideProject(51, this->MeshSeed[3],locator);
        this->SetEdgeScalars(51, 1);

        // face
        this->GenerateFaceProject(33,50,38,51,49,NULL);
        this->GenerateFaceProject(34,45,29,32,50,locator);
//        this->SetFaceScalars(34,0);
        this->GenerateFaceProject(35,47,31,35,51,locator);
//        this->SetFaceScalars(35,0);
        // generate the solid mesh for first bounding box
        FaceArray[0] = 33; FaceArray[1] = 16; FaceArray[2] = 30;
        FaceArray[3] = 21; FaceArray[4] = 34; FaceArray[5] = 35;
        this->GenerateSolidMesh(7,FaceArray);
        //  block 8
        // reverse the edges direction 14,17,22,25
        this->ReverseEdgeDirection(41);        this->ReverseEdgeDirection(43);
        this->ReverseEdgeDirection(46);        this->ReverseEdgeDirection(48);
        // generate 4 new faces required
        this->GenerateFaceProject(36,46,44,48,49,NULL);
        this->GenerateFaceProject(37,41,39,43,44,NULL);
        this->GenerateFaceProject(38,50,34,41,46,locator);
//        this->SetFaceScalars(38,0);
        this->GenerateFaceProject(39,51,37,43,48,locator);
//        this->SetFaceScalars(39,0);
        // generate the solid mesh for the center
        FaceArray[0] = 36; FaceArray[1] = 20; FaceArray[2] = 33;
        FaceArray[3] = 37; FaceArray[4] = 38; FaceArray[5] = 39;
        this->GenerateSolidMesh(8,FaceArray);

        // final unstructured grid
        this->MergeSGridToUGrid(source);
        innerpoints->Delete();
   return 1;
}

//----------------------------------------------------------------------------
int vtkMimxCylindricalMesh::FillInputPortInformation(int port, vtkInformation *info)
{
        if(port < 5)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
        }
        else        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), 
                "vtkUnstructuredGrid");
        return 1;
}

//----------------------------------------------------------------------------
//int vtkCylindricalMesh::SetSuperiorDividingPoints(vtkPoints *Points)
//{
//        if(Points->GetNumberOfPoints() != 4)
//        {
//                return 0;
//        }
//        this->SuperiorDividingPoints->SetNumberOfPoints(4);
//        this->SuperiorDividingPoints->DeepCopy(Points);
//        double x[3];
//        this->Center[0] = 0;        this->Center[1] = 0; this->Center[2] = 0;
//        for(int i=0; i <4; i++)
//        {
//                this->SuperiorDividingPoints->GetPoint(i,x);
//                for(int j=0; j<3; j++)
//                {
//                        this->Center[j] = this->Center[j] + x[j];
//                }
//
//        }
//        for(int j=0; j<3; j++)
//        {
//                this->Center[j] = this->Center[j]/4;
//        }
//        return 1;
//}
////----------------------------------------------------------------------------
//int vtkCylindricalMesh::SetInferiorDividingPoints(vtkPoints *Points)
//{
//        if(Points->GetNumberOfPoints() != 4)
//        {
//                return 0;
//        }
//        this->InferiorDividingPoints->SetNumberOfPoints(4);
//        this->InferiorDividingPoints->DeepCopy(Points);
//        double x[3];
//        this->Center[0] = 0;        this->Center[1] = 0; this->Center[2] = 0;
//        for(int i=0; i <4; i++)
//        {
//                this->InferiorDividingPoints->GetPoint(i,x);
//                for(int j=0; j<3; j++)
//                {
//                        this->Center[j] = this->Center[j] + x[j];
//                }
//
//        }
//        for(int j=0; j<3; j++)
//        {
//                this->Center[j] = this->Center[j]/4;
//        }
//        return 1;
//}

//-----------------------------------------------------------------------
//int vtkCylindricalMesh::SetMeshSeed(int MSeed[5])
//{
//        for(int i=0; i<5; i++)
//        {
//                this->MeshSeed[i] = MSeed[i];
//        }
//        return 1;
//}

void vtkMimxCylindricalMesh::SetCenter(double X[3])
{
        for(int i=0; i<3; i++)        this->Center[i] = X[i];
}

//----------------------------------------------------------------------------

void vtkMimxCylindricalMesh::EdgeSubdivideProject(int EdgeNum, int NumDiv, vtkCellLocator *Locator)
{
        vtkCellArray *cellarray = vtkCellArray::New();
        for (int i=0; i < this->EdgePolyData[EdgeNum]->GetNumberOfPoints()-1; i++) {
                cellarray->InsertNextCell(2);
                cellarray->InsertCellPoint(i);
                cellarray->InsertCellPoint(i+1);
        }
        this->EdgePolyData[EdgeNum]->SetLines(cellarray);
        cellarray->Delete();
        //vtkPolyDataWriter *writer1 = vtkPolyDataWriter::New();
        //writer1->SetInput(this->EdgePolyData[EdgeNum]);
        //writer1->SetFileName("Edge1.vtk");
        //writer1->Write();
        //writer1->Delete();

        vtkMimxSubdivideCurve *subdividecurve = vtkMimxSubdivideCurve::New();
        subdividecurve->SetInput(this->EdgePolyData[EdgeNum]);
        subdividecurve->SetNumberOfDivisions(NumDiv);
        subdividecurve->Update();
        this->EdgePolyData[EdgeNum]->GetPoints()->Initialize();
        this->EdgePolyData[EdgeNum]->GetPoints()->DeepCopy(subdividecurve->GetOutput()->GetPoints());
        this->EdgePolyData[EdgeNum]->DeepCopy(subdividecurve->GetOutput());
        subdividecurve->Delete();
        //vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
        //writer->SetInput(this->EdgePolyData[EdgeNum]);
        //writer->SetFileName("Edge.vtk");
        //writer->Write();
        //writer->Delete();

        if(Locator)
        {
                this->ClosestPointProjection(this->EdgePolyData[EdgeNum]->GetPoints(),Locator);
        }
}
//----------------------------------------------------------------------------
void vtkMimxCylindricalMesh::GenerateUnknownEdge(int EdgeNum, int FirstEdge, int SecondEdge, int BeginOrEnd)
{
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(2);
        if(!BeginOrEnd)
        {
                double x[3], y[3];
                this->EdgePolyData[FirstEdge]->GetPoint(0,x);
                this->EdgePolyData[SecondEdge]->GetPoint(0,y);
                points->SetPoint(0,this->EdgePolyData[FirstEdge]->GetPoint(0));
                points->SetPoint(1,this->EdgePolyData[SecondEdge]->GetPoint(0));
        }
        else
        {
                points->SetPoint(0,this->EdgePolyData[FirstEdge]->GetPoint
                        (this->EdgePolyData[FirstEdge]->GetNumberOfPoints()-1));
                points->SetPoint(1,this->EdgePolyData[SecondEdge]->GetPoint
                        (this->EdgePolyData[SecondEdge]->GetNumberOfPoints()-1));
        }
        this->EdgePolyData[EdgeNum]->SetPoints(points);
        points->Delete();
}
//----------------------------------------------------------------------------
void vtkMimxCylindricalMesh::GenerateFaceProject(int FaceNum,int Edge0, 
                                                int Edge1, int Edge2, int Edge3, vtkCellLocator* Locator)
{
        vtkMimxEdgeToPlanarStructuredGrid *edgetostructgrid = vtkMimxEdgeToPlanarStructuredGrid::New();
        int dim[2]; 
        dim[0] = this->EdgePolyData[Edge0]->GetNumberOfPoints();
        dim[1] = this->EdgePolyData[Edge1]->GetNumberOfPoints();
        int dim0 = this->EdgePolyData[Edge2]->GetNumberOfPoints();
        int dim1 = this->EdgePolyData[Edge3]->GetNumberOfPoints();
        edgetostructgrid->SetDimensions(dim);
        edgetostructgrid->SetEdge(0,this->EdgePolyData[Edge0]);
        edgetostructgrid->SetEdge(1,this->EdgePolyData[Edge1]);
        edgetostructgrid->SetEdge(2,this->EdgePolyData[Edge2]);
        edgetostructgrid->SetEdge(3,this->EdgePolyData[Edge3]);
//        edgetostructgrid->SetOutput(this->StructuredPlanarGrid[FaceNum]);
        edgetostructgrid->Update();
        if(Locator)
        {
                this->ClosestPointProjection(edgetostructgrid->GetOutput()->GetPoints(),Locator);
        }
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(edgetostructgrid->GetOutput()->GetNumberOfPoints());
        double x[3];
        for(int i=0; i<edgetostructgrid->GetOutput()->GetNumberOfPoints(); i++)
        {
                edgetostructgrid->GetOutput()->GetPoint(i,x);
                points->SetPoint(i,x);
        }
        this->StructuredPlanarGrid[FaceNum]->SetDimensions
                (edgetostructgrid->GetOutput()->GetDimensions());
        this->StructuredPlanarGrid[FaceNum]->SetPoints(points);
        this->StructuredPlanarGrid[FaceNum]->GetPointData()->SetScalars(
                edgetostructgrid->GetOutput()->GetPointData()->GetScalars());
        //vtkStructuredGridWriter* writer = vtkStructuredGridWriter::New();
        //writer->SetInput(this->StructuredPlanarGrid[FaceNum]);
        //writer->SetFileName("sgrid.vtk");
        //writer->Write();

        points->Delete();
        edgetostructgrid->Delete();
}
//----------------------------------------------------------------------------------
void vtkMimxCylindricalMesh::SetSource(vtkPolyData* Polydata)
{
        this->SetInput(4, Polydata);
}
//-----------------------------------------------------------------------------------
void vtkMimxCylindricalMesh::SetCurve(int CurveNum, vtkPolyData* Polydata)
{
        this->SetInput(CurveNum, Polydata);
}
//-----------------------------------------------------------------------------------
void vtkMimxCylindricalMesh::SetBoundingBox(vtkUnstructuredGrid *BoundingBox)
{
        this->SetInput(5, BoundingBox);
}
//------------------------------------------------------------------------------------
void vtkMimxCylindricalMesh::ClosestPointProjection(vtkPoints *Points, vtkCellLocator *Locator)
{
        double dist,ClosestPoint[3],x[3];
        vtkIdType CellId;
        int SubId;
        for(int i=0; i<Points->GetNumberOfPoints(); i++)
        {
                Points->GetPoint(i,x);
                Locator->FindClosestPoint(x,ClosestPoint,CellId,SubId,dist);
                Points->SetPoint(i,ClosestPoint);
        }}
//----------------------------------------------------------------------------

void vtkMimxCylindricalMesh::GenerateSolidMesh(int BoxNum, int FaceArray[6])
{
        vtkMimxStructuredPlanarToStructuredSolidGrid *sgrid = 
                vtkMimxStructuredPlanarToStructuredSolidGrid::New();
        sgrid->SetOutput(this->StructuredSolidGrid[BoxNum]);
        for(int i=0; i<6; i++)
        {
                sgrid->SetInput(i,this->StructuredPlanarGrid[FaceArray[i]]);
        }
        sgrid->Update();
        vtkPoints *points = vtkPoints::New();
        double x[3];
        points->SetNumberOfPoints(sgrid->GetOutput()->GetNumberOfPoints());
        for(int i=0; i<sgrid->GetOutput()->GetNumberOfPoints(); i++)
        {
                sgrid->GetOutput()->GetPoint(i,x);
                points->SetPoint(i,x);
        }        
        this->StructuredSolidGrid[BoxNum]->SetDimensions(
                sgrid->GetOutput()->GetDimensions());
        this->StructuredSolidGrid[BoxNum]->SetPoints(points);
        points->Delete();
        //vtkStructuredGridWriter* writer = vtkStructuredGridWriter::New();
        //writer->SetInput(this->StructuredSolidGrid[BoxNum]);
        //writer->SetFileName("sgrid.vtk");
        //writer->Write();
        sgrid->Delete();
}
//------------------------------------------------------------------------------

//void vtkCylindricalMesh::DivideCurve(int CurveNum, vtkPolyData *PolyData)
//{
//         find 4 points of intersection of the plane with the input curves
//        vtkPoints *pointsstore = vtkPoints::New();
//        pointsstore->SetNumberOfPoints(5);
//        vtkIntArray *lineid = vtkIntArray::New();
//        lineid->SetNumberOfValues(5);
//         to store the curve numbers for the final mesh
//        vtkIntArray *curveid = vtkIntArray::New();
//        curveid->SetNumberOfValues(4);
//        vtkPoints *pdatapoints[4];
//
//        for(int i=0; i<4; i++)
//        {
//                pdatapoints[i] = vtkPoints::New();
//                pdatapoints[i]->SetNumberOfPoints(0);
//        }
//
//        for(int i=0; i < 4; i++)
//        {
//                 cutting plane definition
//                double x[3], t;
//                this->SuperiorDividingPoints->GetPoint(i,x);
//                double normal[3];
//                normal[1] = 0.0;
//                 calculate normal of the plane
//                double theta;
//                if(x[0] <= this->Center[0] && x[2] <= this->Center[2])
//                {
//                        theta = acos(fabs(this->Center[2] -  x[2])/sqrt(
//                                pow(x[0]-Center[0],2.0)+pow(x[2]-Center[2],2.0)));
//                        theta += 3.1416;
//                }
//                else
//                {
//                        if(x[0] <= this->Center[0] && x[2] >= this->Center[2])
//                        {
//                                theta = acos((x[2] - this->Center[2])/sqrt(
//                                        pow(x[0]-Center[0],2.0)+pow(x[2]-Center[2],2.0)));
//                                theta = 2.0*3.1416 - theta;
//                        }
//                        else
//                        {
//                                theta = acos((x[2] - this->Center[2])/sqrt(
//                                        pow(x[0]-Center[0],2.0)+pow(x[2]-Center[2],2.0)));
//                        }
//                }
//                 determine to which quadrant plane belongs
//                 quadrant 2
///*                if(x[0] >= this->Center[0] && x[2] <=this->Center[2])
//                {
//
//                }*/
//                theta = theta + 3.1416/2.0;
//                normal[2] = cos(theta);
//                normal[0] = sin(theta);
//                 to store two possible intersection points and the line id on which
//                 the point lies
//                vtkIdType lineid1, lineid2;
//                double intpoint1[3], intpoint2[3];
//                 check which line segment intersects
//                 first intersection point
//                for(int j=0; j < PolyData->GetNumberOfPoints(); j++)
//                {
//                        double pt1[3], pt2[3];
//                        PolyData->GetPoint(j,pt1);
//                        PolyData->GetPoint(j+1,pt2);
//                        if(vtkPlane::IntersectWithLine(pt1,pt2,normal,this->Center,t,intpoint1))
//                        {
//                                lineid1 = j;
//                                break;
//                        }
//                }
//                 second intersection point
//                for(int j= lineid1+1; j < PolyData->GetNumberOfPoints(); j++)
//                {
//                        if(j == PolyData->GetNumberOfPoints()-1)
//                        {
//                                double pt1[3], pt2[3];
//                                PolyData->GetPoint(j,pt1);
//                                PolyData->GetPoint(0,pt2);
//                                if(vtkPlane::IntersectWithLine(pt1,pt2,normal,this->Center,t,intpoint2))
//                                {
//                                        lineid2 = j;
//                                        break;
//                                }
//                        }
//                        else
//                        {
//                                double pt1[3], pt2[3];
//                                PolyData->GetPoint(j,pt1);
//                                PolyData->GetPoint(j+1,pt2);
//                                if(vtkPlane::IntersectWithLine(pt1,pt2,normal,this->Center,t,intpoint2))
//                                {
//                                        lineid2 = j;
//                                        break;
//                                }
//                        }
//                }
//                                closest point is the actual point of intersection
//                        double distance1 = sqrt(pow(x[0] - intpoint1[0],2.0) + 
//                                pow(x[2] - intpoint1[2],2.0));
//                        double distance2 = sqrt(pow(x[0] - intpoint2[0],2.0) + 
//                                pow(x[2] - intpoint2[2],2.0));
//
//                        if(distance1 > distance2)
//                        {
//                                pointsstore->SetPoint(i,intpoint2);
//                                lineid->SetValue(i,lineid2);
//                        }
//                        else
//                        {
//                                pointsstore->SetPoint(i,intpoint1);
//                                lineid->SetValue(i,lineid1);
//                        }
//        }
//        
//        pointsstore->InsertPoint(4,pointsstore->GetPoint(0));
//        lineid->SetValue(4,lineid->GetValue(0));
//        if(CurveNum == 0)        
//        {curveid->SetValue(0,7); curveid->SetValue(1,17); 
//        curveid->SetValue(2,25); curveid->SetValue(3,31);}
//        if(CurveNum == 1)
//        {curveid->SetValue(0,5); curveid->SetValue(1,15); 
//        curveid->SetValue(2,23); curveid->SetValue(3,30);}
//        if(CurveNum == 2)
//        {curveid->SetValue(0,3); curveid->SetValue(1,14); 
//        curveid->SetValue(2,22); curveid->SetValue(3,29);}
//        if(CurveNum == 3)
//        {curveid->SetValue(0,1); curveid->SetValue(1,12); 
//        curveid->SetValue(2,20); curveid->SetValue(3,28);}
//
//        for(int i=0; i<4; i++)
//        {
//                // line could contain a part of end of original trace
//                int pt1 = lineid->GetValue(i); int pt2 = lineid->GetValue(i+1); 
//                if(lineid->GetValue(i) > lineid->GetValue(i+1))
//                {
//                        pdatapoints[i] = vtkPoints::New();
//                        pdatapoints[i]->SetNumberOfPoints(1);
//                        pdatapoints[i]->SetPoint(0,pointsstore->GetPoint(i));
//                        for(int j = lineid->GetValue(i)+1; 
//                                j < PolyData->GetNumberOfPoints(); j++)
//                        {
//                                pdatapoints[i]->InsertPoint(
//                                        pdatapoints[i]->GetNumberOfPoints(),PolyData->GetPoint(j));
//                        }
//                        for(int j=0; j <= lineid->GetValue(i+1); j++)
//                        {
//                                pdatapoints[i]->InsertPoint(
//                                        pdatapoints[i]->GetNumberOfPoints(),PolyData->GetPoint(j));
//                        }
//                        pdatapoints[i]->InsertPoint(pdatapoints[i]->GetNumberOfPoints(),
//                                pointsstore->GetPoint(i+1));
//                }
//                else
//                {
//                        pdatapoints[i]->SetPoint(0,pointsstore->GetPoint(i));
//                        for(int j = lineid->GetValue(i)+1; j<= lineid->GetValue(i+1); j++)
//                        {
//                                pdatapoints[i]->InsertPoint(
//                                        pdatapoints[i]->GetNumberOfPoints(),PolyData->GetPoint(j));
//                        }
//                        pdatapoints[i]->InsertPoint(pdatapoints[i]->GetNumberOfPoints(),
//                                pointsstore->GetPoint(i+1));
//                }
//                //// //cout <<" Curve "<<i<<endl;
//                //for(int j=0; j<pdatapoints[i]->GetNumberOfPoints(); j++)
//                //{
//                //        double x[3];
//                //        pdatapoints[i]->GetPoint(j,x);
//                //        // //cout<<j<<"  "<<x[0]<<"  "<<x[1]<<"  "<<x[2]<<endl;
//                //}
//        }
///**********/
//        double length = this->MeasureCurveLength(PolyData, 1);        
//        // divide the length to four parts
//        length = length/4.0;
//        // calculate the dividing points
//        double startingpoint[3], currentlength;
//        int startingpointnum;
//        for (int i=0; i < 4; i++) {
//                if(i =0)        
//                {
//                        PolyData->GetPoint(0, startingpoint);
//                        startingpointnum = 1;
//                }
//                currentlength = 0.0;
//                while (currentlength < length) {
//                        pdatapoints[i]->InsertNextPoint(startingpoint);
//                        if(pdatapoints[i]->GetNumberOfPoints() >1)
//                        {
//                                currentlength = MeasureCurveLength(pdatapoints[i], 0)
//                                        + sqrt(vtkMath::Distance2BetweenPoints(
//                                        pdatapoints->GetPoint(pdatapoints[i]->GetNumberOfPoints()-1),
//                                        PolyData->GetPoint(startingpointnum)));
//                                // if the length of the curve being constructed is less than the
//                                // prescribed length
//                                if(currentlength < length)
//                                {
//                                        pdatapoints[i]->InsertNextPoint(PolyData->GetPoint(startingpointnum));
//                                        startingpointnum ++;
//                                }
//                                else{// interpolate the line and divide
//                                        double nextpoint[3], prevpoint[3];
//                                        PolyData->GetPoint(startingpointnum, nextpoint);
//                                        pdatapoints[i]->GetPoint(
//                                                pdatapoints[i]->GetNumberOfPoints()-1,prevpoint);
//                                        for (int k=0; k <3; k++) {
//                                                startingpoint[k] = prevpoint[k] + 
//                                        }
//                                }
//                        }
//                }
//        }
//        vtkPolyData *polydatastore = vtkPolyData::New();
//        polydatastore->DeepCopy(PolyData);
//        polydatastore->GetPoints()->InsertNextPoint(polydatastore->GetPoint(0));
//        vtkSubdivideCurve *subdivide = vtkSubdivideCurve::New();
//        subdivide->SetInput(polydatastore);
//        subdivide->SetNumberOfDivisions(2*(this->MeshSeed[1] + this->MeshSeed[3]));
//        subdivide->Update();
//        int dim[4];        
//        dim[0] = this->MeshSeed[1];        dim[2] = this->MeshSeed[1];
//        dim[1] = this->MeshSeed[3];        dim[3] = this->MeshSeed[3];
//        // set the divided line to the four curves
//        int pointnum = 1;
//        for (int i=0; i <4; i++) {
//                pointnum --;
//                for (int j=0; j<=dim[i]; j++) {
//                        pdatapoints[i]->InsertPoint(pdatapoints[i]->GetNumberOfPoints(),
//                                subdivide->GetOutput()->GetPoint(pointnum));
//                        pointnum++;
//                }
//        }
//        polydatastore->Delete();
//        subdivide->Delete();
///**********/
//
//        for(int j = 0; j < 4; j++)
//        {
//                this->EdgePolyData[curveid->GetValue(j)]->SetPoints(pdatapoints[j]);
//                pdatapoints[j]->Delete();
//        }
//        pointsstore->Delete();
//        lineid->Delete();
//        curveid->Delete();
//}
void vtkMimxCylindricalMesh::DivideCurve(int CurveNum, vtkPolyData *PolyData, vtkUnstructuredGrid *UGrid)
{
        // find 4 points of intersection of the plane with the input curves
        vtkPoints *pointsstore = vtkPoints::New();
        pointsstore->SetNumberOfPoints(5);
        vtkIntArray *lineid = vtkIntArray::New();
        lineid->SetNumberOfValues(5);
        // to store the curve numbers for the final mesh
        vtkIntArray *curveid = vtkIntArray::New();
        curveid->SetNumberOfValues(4);
        vtkPoints *pdatapoints[4];

        for(int i=0; i<4; i++)
        {
                pdatapoints[i] = vtkPoints::New();
                pdatapoints[i]->SetNumberOfPoints(1);
        }
        vtkCellLocator *locator = vtkCellLocator::New();
        locator->SetDataSet(PolyData);
        locator->BuildLocator();
        int startnodenum;
        if(CurveNum == 0)        startnodenum = 16;
        if(CurveNum == 1)        startnodenum = 12;
        if(CurveNum == 2)        startnodenum = 4;
        if(CurveNum == 3)        startnodenum = 0;
        for(int i=startnodenum; i < startnodenum+4; i++)
        {
                // cutting plane definition
                double x[3], t;
                //if (CurveNum < 2) {
                //        this->SuperiorDividingPoints->GetPoint(i,x);
                //}
                //else
                //{
                //        this->InferiorDividingPoints->GetPoint(i,x);
                //}
        /*        double normal[3];
                switch (i) {
                case 0:
                        this->Plane1->GetCenter(x);
                        this->Plane1->GetNormal(normal);
                        break;
                case 1:
                        this->Plane2->GetCenter(x);
                        this->Plane2->GetNormal(normal);
                        break;
                case 2:
                        this->Plane3->GetCenter(x);
                        this->Plane3->GetNormal(normal);
                        break;
                case 3:
                        this->Plane4->GetCenter(x);
                        this->Plane4->GetNormal(normal);
                        break;
                }*/

        /*        if(CurveNum == 2 || CurveNum == 3)
                {
                        for (int k=0; k<3; k++) {
                                x[k] = x[k] + InfCenter[k] - SupCenter[k];
                        }
                }
        */        //normal[1] = 0.0;
                //// calculate normal of the plane
                //double theta;
                //if(x[0] <= this->Center[0] && x[2] <= this->Center[2])
                //{
                //        theta = acos(fabs(this->Center[2] -  x[2])/sqrt(
                //                pow(x[0]-Center[0],2.0)+pow(x[2]-Center[2],2.0)));
                //        theta += 3.1416;
                //}
                //else
                //{
                //        if(x[0] <= this->Center[0] && x[2] >= this->Center[2])
                //        {
                //                theta = acos((x[2] - this->Center[2])/sqrt(
                //                        pow(x[0]-Center[0],2.0)+pow(x[2]-Center[2],2.0)));
                //                theta = 2.0*3.1416 - theta;
                //        }
                //        else
                //        {
                //                theta = acos((x[2] - this->Center[2])/sqrt(
                //                        pow(x[0]-Center[0],2.0)+pow(x[2]-Center[2],2.0)));
                //        }
                //}
                //// determine to which quadrant plane belongs
                //// quadrant 2
                ///*                if(x[0] >= this->Center[0] && x[2] <=this->Center[2])
                //{

                //}*/
                //theta = theta + 3.1416/2.0;
                //normal[2] = cos(theta);
                //normal[0] = sin(theta);
                // to store two possible intersection points and the line id on which
                // the point lies
                vtkIdType cellid;
                int subid;
                double closestpoint[3], distance;
                locator->FindClosestPoint(UGrid->GetPoint(i),closestpoint, 
                        cellid, subid, distance);

                // check which line segment intersects
                // first intersection point
                //for(int j=0; j < PolyData->GetNumberOfPoints(); j++)
                //{
                //        double pt1[3], pt2[3];
                //        PolyData->GetPoint(j,pt1);
                //        PolyData->GetPoint(j+1,pt2);
                //        if(vtkPlane::IntersectWithLine(pt1,pt2,normal,x,t,intpoint1))
                //        {
                //                lineid1 = j;
                //                break;
                //        }
                //}
                // second intersection point
        /*        for(int j= lineid1+1; j < PolyData->GetNumberOfPoints(); j++)
                {
                        if(j == PolyData->GetNumberOfPoints()-1)
                        {
                                double pt1[3], pt2[3];
                                PolyData->GetPoint(j,pt1);
                                PolyData->GetPoint(0,pt2);
                                if(vtkPlane::IntersectWithLine(pt1,pt2,normal,x,t,intpoint2))
                                {
                                        lineid2 = j;
                                        break;
                                }
                        }
                        else
                        {
                                double pt1[3], pt2[3];
                                PolyData->GetPoint(j,pt1);
                                PolyData->GetPoint(j+1,pt2);
                                if(vtkPlane::IntersectWithLine(pt1,pt2,normal,x,t,intpoint2))
                                {
                                        lineid2 = j;
                                        break;
                                }
                        }
                }
        */        //        closest point is the actual point of intersection
                //double distance1 = sqrt(pow(x[0] - intpoint1[0],2.0) + 
                //        pow(x[2] - intpoint1[2],2.0));
                //double distance2 = sqrt(pow(x[0] - intpoint2[0],2.0) + 
                //        pow(x[2] - intpoint2[2],2.0));

                //if(distance1 > distance2)
                //{
                //        pointsstore->SetPoint(i,intpoint2);
                //        lineid->SetValue(i,lineid2);
                //}
                //else
                //{
                //        pointsstore->SetPoint(i,intpoint1);
                //        lineid->SetValue(i,lineid1);
                //}
                pointsstore->SetPoint(i-startnodenum, closestpoint);
                lineid->SetValue(i-startnodenum, cellid);
        }
        
        locator->Delete();
        pointsstore->InsertPoint(4,pointsstore->GetPoint(0));
        lineid->SetValue(4,lineid->GetValue(0));
        if(CurveNum == 0)        
        {curveid->SetValue(0,7); curveid->SetValue(1,17); 
        curveid->SetValue(2,25); curveid->SetValue(3,31);}
        if(CurveNum == 1)
        {curveid->SetValue(0,5); curveid->SetValue(1,15); 
        curveid->SetValue(2,23); curveid->SetValue(3,30);}
        if(CurveNum == 2)
        {curveid->SetValue(0,3); curveid->SetValue(1,14); 
        curveid->SetValue(2,22); curveid->SetValue(3,29);}
        if(CurveNum == 3)
        {curveid->SetValue(0,1); curveid->SetValue(1,12); 
        curveid->SetValue(2,20); curveid->SetValue(3,28);}

        for(int i=0; i<4; i++)
        {
                vtkPointLocator *pointlocator = vtkPointLocator::New();
                // line could contain a part of end of original trace
                int pt1 = lineid->GetValue(i); int pt2 = lineid->GetValue(i+1);
                vtkIdType pointid;
                pdatapoints[i] = vtkPoints::New();
                pdatapoints[i]->Allocate(100);
                pointlocator->InitPointInsertion(pdatapoints[i],PolyData->GetPoints()->GetBounds());

                if(lineid->GetValue(i) > lineid->GetValue(i+1))
                {
                                pointlocator->InsertUniquePoint(pointsstore->GetPoint(i),pointid);
//                        pdatapoints[i]->SetPoint(0,pointsstore->GetPoint(i));
                        for(int j = lineid->GetValue(i)+1; 
                                j < PolyData->GetNumberOfPoints(); j++)
                        {
                                pointlocator->InsertUniquePoint(PolyData->GetPoint(j),pointid);
        //                        pdatapoints[i]->InsertPoint(
        //                                pdatapoints[i]->GetNumberOfPoints(),PolyData->GetPoint(j));
                        }
                        for(int j=0; j <= lineid->GetValue(i+1); j++)
                        {
                                pointlocator->InsertUniquePoint(PolyData->GetPoint(j),pointid);
//                                pdatapoints[i]->InsertPoint(
//                                        pdatapoints[i]->GetNumberOfPoints(),PolyData->GetPoint(j));
                        }

                        pointlocator->InsertUniquePoint(pointsstore->GetPoint(i+1),pointid);

        //                pdatapoints[i]->InsertPoint(pdatapoints[i]->GetNumberOfPoints(),
        //                        pointsstore->GetPoint(i+1));
                }
                else
                {
                        pointlocator->InsertUniquePoint(pointsstore->GetPoint(i),pointid);
//                        pdatapoints[i]->SetPoint(0,pointsstore->GetPoint(i));
                        for(int j = lineid->GetValue(i)+1; j<= lineid->GetValue(i+1); j++)
                        {
                                pointlocator->InsertUniquePoint(PolyData->GetPoint(j),pointid);

        //                        pdatapoints[i]->InsertPoint(
        //                                pdatapoints[i]->GetNumberOfPoints(),PolyData->GetPoint(j));
                        }
                        pointlocator->InsertUniquePoint(pointsstore->GetPoint(i+1),pointid);

        //                pdatapoints[i]->InsertPoint(pdatapoints[i]->GetNumberOfPoints(),
        //                        pointsstore->GetPoint(i+1));
                }
                cout <<" Curve "<<i<<endl;
                for(int j=0; j<pdatapoints[i]->GetNumberOfPoints(); j++)
                {
                        double x[3];
                        pdatapoints[i]->GetPoint(j,x);
                        cout<<j<<"  "<<x[0]<<"  "<<x[1]<<"  "<<x[2]<<endl;
                }
                pointlocator->Delete();
        }

        for(int j = 0; j < 4; j++)
        {
                this->EdgePolyData[curveid->GetValue(j)]->SetPoints(pdatapoints[j]);
                pdatapoints[j]->Delete();
        }
        pointsstore->Delete();
        lineid->Delete();
        curveid->Delete();
}
//------------------------------------------------------------------------------

void vtkMimxCylindricalMesh::ReverseEdgeDirection(int EdgeNum)
{
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(this->EdgePolyData[EdgeNum]
                        ->GetNumberOfPoints());
        for(int i=1; i<=this->EdgePolyData[EdgeNum]->GetNumberOfPoints(); i++)
        {
                points->SetPoint(i-1, this->EdgePolyData[EdgeNum]->GetPoint(
                        this->EdgePolyData[EdgeNum]->GetNumberOfPoints()-i));
        }
        this->EdgePolyData[EdgeNum]->SetPoints(points);
        points->Delete();
}
//------------------------------------------------------------------------------

void vtkMimxCylindricalMesh::MergeSGridToUGrid(vtkPolyData *source)
{
        vtkUnstructuredGrid *output = 
                vtkUnstructuredGrid::SafeDownCast(this->GetOutputDataObject(0));
        //int numele = 0;
        //int numnodes = 0;
        //int dim[3];
        //vtkCell* cell;
        ////        calculate number of nodes and elements
        //for(int i=0; i < 9; i++)
        //{
        //        this->StructuredSolidGrid[i]->GetDimensions(dim);
        //        numnodes = numnodes + dim[0]*dim[1]*dim[2];
        //        numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
        //}

        //vtkMergeCells* mergecells = vtkMergeCells::New();
        //mergecells->SetUnstructuredGrid(output);
        //mergecells->MergeDuplicatePointsOn();
        //mergecells->SetTotalNumberOfDataSets(9);
        //mergecells->SetTotalNumberOfCells(numele);
        //mergecells->SetTotalNumberOfPoints(numnodes);
        //for(int i = 0; i < 9; i++)
        //{
        //        mergecells->MergeDataSet(this->StructuredSolidGrid[i]);
        //}
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

        int dim[3], m;
        int numele = 0;
        int numnode = 0;
        int i, j, k;
        for(i=0; i <9; i++)
        {
                this->StructuredSolidGrid[i]->GetDimensions(dim);
                numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
                numnode = numnode + dim[0]*dim[1]*dim[2];
        }

        // start with the other bounding boxes
        // check if a dataset contains coincident points
        vtkPointLocator *globallocator = vtkPointLocator::New();
        vtkPoints *globalpoints = vtkPoints::New();
        globalpoints->Allocate(numnode);
        output->Allocate(numele);
        globallocator->InitPointInsertion(globalpoints, source->GetBounds());

        vtkIntArray *intarray = vtkIntArray::New();
        intarray->SetNumberOfComponents(1);

        int startnodenum;
        for(m=0; m < 9; m++)

        {
                vtkPoints* sgridpoints = this->StructuredSolidGrid[m]->GetPoints();

                this->StructuredSolidGrid[m]->GetDimensions(dim);

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
                                double status[1];
                                this->StructuredSolidGrid[m]->GetPointData()->GetScalars()->GetTuple(i,status);
                                intarray->InsertNextTuple1(status[0]);
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
                                        double status[1];
                                        this->StructuredSolidGrid[m]->GetPointData()->GetScalars()->GetTuple(i,status);
                                        intarray->InsertNextTuple1(status[0]);
                                }
                        }
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
                                        output->InsertNextCell(12, ptids);        
                                }
                        }
                }
                ptids->Delete();
                poirenum->Delete();
        }

        output->GetPointData()->SetScalars(intarray);
        intarray->Delete();
        output->SetPoints(globalpoints);
        output->Squeeze();
        globalpoints->Delete();
        globallocator->Delete();
}
//------------------------------------------------------------------------------
double vtkMimxCylindricalMesh::MeasureCurveLength(vtkPolyData *PolyData, int CalculateClosedCurve)
{
        double length = 0.0;
        for (int i=0; i < PolyData->GetNumberOfPoints()-1; i++) {
                length = length + sqrt(vtkMath::Distance2BetweenPoints(PolyData->GetPoint(i),
                        PolyData->GetPoint(i+1)));
        }
        if(CalculateClosedCurve)
        {
                length = length + sqrt(vtkMath::Distance2BetweenPoints(PolyData->GetPoint(0),
                        PolyData->GetPoint(PolyData->GetNumberOfPoints()-1)));
        }
        return length;
}
//------------------------------------------------------------------------------
void vtkMimxCylindricalMesh::SetEdgeScalars(int EdgeNum, int ScalarValue)
{
        vtkIntArray *intarray = vtkIntArray::New();
        intarray->SetNumberOfValues(this->EdgePolyData[EdgeNum]->GetNumberOfPoints());
        for (int i=0; i < this->EdgePolyData[EdgeNum]->GetNumberOfPoints(); i++) {
                intarray->SetValue(i, ScalarValue);
        }
        this->EdgePolyData[EdgeNum]->GetPointData()->SetScalars(intarray);
        intarray->Delete();
}
//------------------------------------------------------------------------------
void vtkMimxCylindricalMesh::GenerateUnknownEdgeFromPoints(int EdgeNum, double *FirstPoint, 
                                                                                         double *SecondPoint)
{
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(2);
        points->SetPoint(0,FirstPoint);
        points->SetPoint(1,SecondPoint);
        this->EdgePolyData[EdgeNum]->SetPoints(points);
        points->Delete();
}
//------------------------------------------------------------------------------
void vtkMimxCylindricalMesh::SetFaceScalars(int FaceNum, int ScalarValue)
{
        //vtkIntArray *intarray = vtkIntArray::New();
        //intarray->SetNumberOfValues(this->EdgePolyData[EdgeNum]->GetNumberOfPoints());
        for (int i=0; i < this->StructuredPlanarGrid[FaceNum]->GetNumberOfPoints(); i++) {
                double status[1];
                this->StructuredPlanarGrid[FaceNum]->GetPointData()->GetScalars()->
                        GetTuple(i,status);
                if(status[0])        
                {
                        status[0] = ScalarValue;
                        this->StructuredPlanarGrid[FaceNum]->GetPointData()->GetScalars()->
                        SetTuple(i,status);
                }
        }
        //this->EdgePolyData[EdgeNum]->GetPointData()->SetScalars(intarray);
        //intarray->Delete();
}
//------------------------------------------------------------------------------
void vtkMimxCylindricalMesh::CalculateCenter(vtkPolyData *curve)
{
        this->Center[0] = 0.0; this->Center[1] = 0.0; this->Center[2] = 0.0;
        for (int i=0; i<curve->GetNumberOfPoints(); i++) {
                double x[3];
                curve->GetPoint(i,x);
                for (int j=0; j<3; j++) {
                        this->Center[j] = this->Center[j] + x[j]; 
                }
        }

        for (int i=0; i<3; i++) {
                this->Center[i] = this->Center[i]/curve->GetNumberOfPoints();
        }
}
//------------------------------------------------------------------------------
void vtkMimxCylindricalMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
