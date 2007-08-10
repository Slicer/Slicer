/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxFourPointsToStructuredPlanarMesh.cxx,v $
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

#include "vtkMimxFourPointsToStructuredPlanarMesh.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointLocator.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkMimxPolyDataSingleSourceShortestPath.h"

vtkCxxRevisionMacro(vtkMimxFourPointsToStructuredPlanarMesh, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkMimxFourPointsToStructuredPlanarMesh);

// Construct object to extract all of the input data.
vtkMimxFourPointsToStructuredPlanarMesh::vtkMimxFourPointsToStructuredPlanarMesh()
{
        this->CorrespondingPointList = NULL;

}

vtkMimxFourPointsToStructuredPlanarMesh::~vtkMimxFourPointsToStructuredPlanarMesh()
{
        if(!this->CorrespondingPointList)
        {
                this->CorrespondingPointList->Delete();
        }
}

int vtkMimxFourPointsToStructuredPlanarMesh::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // check for input id list
  if(this->PointList->GetNumberOfIds() != 4)
  {
          vtkErrorMacro("Point list should contain 4 points only");
          return 0;
  } 

  // check for the validity of the input data
  vtkIdType nPts = input->GetNumberOfPoints();
  vtkIdType nCells = input->GetNumberOfCells();
  if(nPts <=0 || nCells <=0)
  {
          vtkErrorMacro("Invalid Input Data");
          return 0;
  }

  // check input cell type
  vtkCellTypes *celltypes = vtkCellTypes::New();
  input->GetCellTypes(celltypes);

  int i;
  for(i =0; i < celltypes->GetNumberOfTypes(); i++)
  {
        if(celltypes->GetCellType(i) != 9)
        {
                vtkErrorMacro("input polydata must contain only quadrilaterals");
                celltypes->Delete();
                return 0;
        }
  }
  celltypes->Delete();

  // calculate the intermediate points connecting corners 0 and 1
  vtkMimxPolyDataSingleSourceShortestPath *shortestpath0 = vtkMimxPolyDataSingleSourceShortestPath::New();
  shortestpath0->SetInput(input);
  shortestpath0->SetEqualWeights(1);
  shortestpath0->SetStartVertex(this->PointList->GetId(0));
  shortestpath0->SetEndVertex(this->PointList->GetId(1));
  shortestpath0->Update();
  
  // calculate the intermediate points connecting corners 1 and 2
  vtkMimxPolyDataSingleSourceShortestPath *shortestpath1 = vtkMimxPolyDataSingleSourceShortestPath::New();
  shortestpath1->SetInput(input);
  shortestpath1->SetEqualWeights(1);
  shortestpath1->SetStartVertex(this->PointList->GetId(1));
  shortestpath1->SetEndVertex(this->PointList->GetId(2));
  shortestpath1->Update();
  
  // calculate the intermediate points connecting corners 3 and 2
  vtkMimxPolyDataSingleSourceShortestPath *shortestpath2 = vtkMimxPolyDataSingleSourceShortestPath::New();
  shortestpath2->SetInput(input);
  shortestpath2->SetEqualWeights(1);
  shortestpath2->SetStartVertex(this->PointList->GetId(3));
  shortestpath2->SetEndVertex(this->PointList->GetId(2));
  shortestpath2->Update();
  if(shortestpath2->GetIdList()->GetNumberOfIds() != shortestpath0->GetIdList()->GetNumberOfIds())
  {
          shortestpath0->Delete();
          shortestpath1->Delete();
          shortestpath2->Delete();
          vtkErrorMacro("Invalid corner ids");
          return 0;
  }
  
  // calculate the intermediate points connecting corners 0 and 3
  vtkMimxPolyDataSingleSourceShortestPath *shortestpath3 = vtkMimxPolyDataSingleSourceShortestPath::New();
  shortestpath3->SetInput(input);
  shortestpath3->SetEqualWeights(1);
  shortestpath3->SetStartVertex(this->PointList->GetId(0));
  shortestpath3->SetEndVertex(this->PointList->GetId(3));
  shortestpath3->Update();

  if(shortestpath3->GetIdList()->GetNumberOfIds() != shortestpath1->GetIdList()->GetNumberOfIds())
  {
          shortestpath0->Delete();
          shortestpath1->Delete();
          shortestpath2->Delete();
          shortestpath3->Delete();
          vtkErrorMacro("Invalid corner ids");
          return 0;
  }

// to store point ids belonging to output structured grid
  CorrespondingPointList = vtkIdList::New();
  
// first row is already calculated
// store first row
        for(i =0; i < shortestpath0->GetIdList()->GetNumberOfIds(); i++)
        {
                CorrespondingPointList->InsertNextId(shortestpath0->GetIdList()->GetId(i));
        }
// store the other rows except the last row
  for(i = 1; i < shortestpath3->GetIdList()->GetNumberOfIds()-1; i++)
  {
          vtkMimxPolyDataSingleSourceShortestPath *shortestpath = vtkMimxPolyDataSingleSourceShortestPath::New();
          shortestpath->SetInput(input);
          shortestpath->SetEqualWeights(1);
          shortestpath->SetStartVertex(shortestpath3->GetIdList()->GetId(i));
          shortestpath->SetEndVertex(shortestpath1->GetIdList()->GetId(i));
          shortestpath->Update();
          if(shortestpath->GetIdList()->GetNumberOfIds() != shortestpath0->GetIdList()->GetNumberOfIds())
          {
                  vtkErrorMacro("Invalid corner ids");
                  shortestpath0->Delete();
                  shortestpath1->Delete();
                  shortestpath2->Delete();
                  shortestpath3->Delete();
                  CorrespondingPointList->Delete();
                  shortestpath->Delete();
                  return 0;
          }
          for (int j =0; j < shortestpath->GetIdList()->GetNumberOfIds(); j++)
          {
                  CorrespondingPointList->InsertNextId(shortestpath->GetIdList()->GetId(j));
          }
          shortestpath->Delete();
  }
  // store last row
  for(i =0; i < shortestpath2->GetIdList()->GetNumberOfIds(); i++)
  {
          CorrespondingPointList->InsertNextId(shortestpath2->GetIdList()->GetId(i));
  }

  //
  for (i = 0; i < CorrespondingPointList->GetNumberOfIds(); i++)
  {
          //cout << CorrespondingPointList->GetId(i)<<endl;
  }
  // create a point co-ordinate list for the output
  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(CorrespondingPointList->GetNumberOfIds());
  for(i = 0; i<CorrespondingPointList->GetNumberOfIds(); i++)
  {
          points->SetPoint(i, input->GetPoint(CorrespondingPointList->GetId(i)));
  }
  output->SetPoints(points);
  points->Delete();
  output->SetDimensions(shortestpath0->GetIdList()->GetNumberOfIds(), shortestpath1->GetIdList()->GetNumberOfIds(), 1);

  shortestpath0->Delete();
  shortestpath1->Delete();
  shortestpath2->Delete();
  shortestpath3->Delete();

   return 1;
}

int vtkMimxFourPointsToStructuredPlanarMesh::FillInputPortInformation(int port,
                                                                                                 vtkInformation *info)
{
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
        return 1;
}


void vtkMimxFourPointsToStructuredPlanarMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}
