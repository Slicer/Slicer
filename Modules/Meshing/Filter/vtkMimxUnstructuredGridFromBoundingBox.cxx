/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridFromBoundingBox.cxx,v $
Language:  C++
Date:      $Date: 2008/08/14 05:01:51 $
Version:   $Revision: 1.14 $

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

#include "vtkMimxUnstructuredGridFromBoundingBox.h"

#include "vtkMimxBoundingBoxToStructuredGrids.h"
#include "vtkCellData.h"
#include "vtkCollection.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMergeCells.h"
#include "vtkMimxMorphStructuredGrid.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPointLocator.h"
#include "vtkIdList.h"
#include "vtkFieldData.h"

#include "vtkStructuredGridWriter.h"

vtkCxxRevisionMacro(vtkMimxUnstructuredGridFromBoundingBox, "$Revision: 1.14 $");
vtkStandardNewMacro(vtkMimxUnstructuredGridFromBoundingBox);

// Description:

vtkMimxUnstructuredGridFromBoundingBox::vtkMimxUnstructuredGridFromBoundingBox()
{
  this->SetNumberOfInputPorts(2);
}

vtkMimxUnstructuredGridFromBoundingBox::~vtkMimxUnstructuredGridFromBoundingBox()
{
}

int vtkMimxUnstructuredGridFromBoundingBox::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        
  // get the info objects
  vtkInformation *bboxInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *surfaceInfo = inputVector[1]->GetInformationObject(0);

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkUnstructuredGrid *bbox = vtkUnstructuredGrid::SafeDownCast(
    bboxInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *surface = vtkPolyData::SafeDownCast(
    surfaceInfo->Get(vtkDataObject::DATA_OBJECT()));

  // check if mesh seed data exists
  if(!bbox->GetCellData()->GetArray("Mesh_Seed"))
    {
    vtkErrorMacro("Mesh seed data needed");
    return 0;
    }

  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
        

  // generate unmorphed structured grids
  vtkMimxBoundingBoxToStructuredGrids *bboxtosgrid = 
    vtkMimxBoundingBoxToStructuredGrids::New();
  bboxtosgrid->SetInput(bbox);
//  bboxtosgrid->SetMeshSeed(vtkIntArray::SafeDownCast(bbox->GetCellData()->GetArray("Mesh_Seed")));
  bboxtosgrid->Update();

  // to store all the morphed structured grids
  vtkCollection *sgridcollection = vtkCollection::New(); 
  // morph all the structured grids and store
  int i;
  for (i=0; i < bbox->GetNumberOfCells(); i++)
    {
    sgridcollection->AddItem((vtkObject*)vtkStructuredGrid::New());

    vtkMimxMorphStructuredGrid *morphsgrid = vtkMimxMorphStructuredGrid::New();
    morphsgrid->SetGridStructure(bbox);
    morphsgrid->SetInput(bboxtosgrid->GetStructuredGrid(i));
    morphsgrid->SetSource(surface);
    morphsgrid->SetCellNum(i);
    morphsgrid->Update();
    //vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
    //writer->SetInput(morphsgrid->GetOutput());
    //char name[20];
    //strcpy(name, "Sgrid");
    //char buffer[10];
    //char fileexten[10];
    //sprintf(buffer, "%d", i);
    //strcpy(fileexten,".vtk");
    //strcat(name, buffer);
    //strcat(name, fileexten);
    //writer->SetFileName(name);
    //writer->Write();
    ((vtkStructuredGrid*)sgridcollection->GetItemAsObject
     (sgridcollection->GetNumberOfItems()-1))->DeepCopy(morphsgrid->GetOutput());
    morphsgrid->Delete();
    }
  //******************************************************************************
      //int numele = 0;
      //int numnodes = 0;
      //int dim[3];
      ////  calculate number of nodes and elements
      //for(int i=0; i <sgridcollection->GetNumberOfItems(); i++)
      //{
      // ((vtkStructuredGrid*)(sgridcollection->
      //  GetItemAsObject(i)))->GetDimensions(dim);
      // numnodes = numnodes + dim[0]*dim[1]*dim[2];
      // numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
      //}

      //vtkMergeCells* mergecells = vtkMergeCells::New();
      //mergecells->SetUnstructuredGrid(output);
      //mergecells->SetPointMergeTolerance(0.001);
      //mergecells->MergeDuplicatePointsOn();
      //mergecells->SetTotalNumberOfDataSets(sgridcollection->GetNumberOfItems());
      //mergecells->SetTotalNumberOfCells(numele);
      //mergecells->SetTotalNumberOfPoints(numnodes);
      //for(int i=0; i <sgridcollection->GetNumberOfItems(); i++)
      //{
      // /* change made here to check for abaqus displacement field*/
      // mergecells->MergeDataSet(((vtkStructuredGrid*)(sgridcollection->
      //  GetItemAsObject(i))));
      // //mergecells->MergeDataSet(bboxtosgrid->GetStructuredGrid(i));
      //}
      //mergecells->Finish();
      //mergecells->Delete();
      //bboxtosgrid->Delete();
      //***************************************************************************************
          int dim[3], j, m, k;
  int numele = 0;
  int numnode = 0;
  for(i=0; i <sgridcollection->GetNumberOfItems(); i++)
    {
    ((vtkStructuredGrid*)(sgridcollection->
                          GetItemAsObject(i)))->GetDimensions(dim);
    numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
    numnode = numnode + dim[0]*dim[1]*dim[2];
    }
  // compute bounds. this is because the interior points of the individual grids
  // might be outside the bounds of complete mesh.
  double Bounds[6];
  ((vtkStructuredGrid*)(sgridcollection->GetItemAsObject(0)))->GetBounds(Bounds);
  for (i=1; i<sgridcollection->GetNumberOfItems(); i++)
    {
    double gridbounds[6];
    ((vtkStructuredGrid*)(sgridcollection->GetItemAsObject(i)))->GetBounds(gridbounds);
    for (j=0; j<6; j=j+2)
      {
      if(gridbounds[j] < Bounds[j])
        Bounds[j] = gridbounds[j];
      }
    for (j=1; j<6; j=j+2)
      {
      if(gridbounds[j] > Bounds[j])
        Bounds[j] = gridbounds[j];
      }
    }
  // start with the other bounding boxes
  // check if a dataset contains coincident points
  vtkPointLocator *globallocator = vtkPointLocator::New();
  vtkPoints *globalpoints = vtkPoints::New();
  globalpoints->Allocate(numnode);
  output->Allocate(numele);
  globallocator->InitPointInsertion(globalpoints, Bounds);
  
  //ofstream FileOutput;
  //FileOutput.open("debug.txt",std::ios::out);

  int startnodenum;
  vtkIntArray *bboxArray = vtkIntArray::New();
  bboxArray->SetName("Bounding_Box_Number");
  for(m=0; m < bbox->GetNumberOfCells(); m++)
    {
    vtkPoints* sgridpoints = ((vtkStructuredGrid*)(sgridcollection->
                                                   GetItemAsObject(m)))->GetPoints();
  
    ((vtkStructuredGrid*)(sgridcollection->
                          GetItemAsObject(m)))->GetDimensions(dim);
  
    // create the global point id list for the individual points in the data set
    // and insert unique points belonging to different data sets
    vtkIdList *poirenum = vtkIdList::New();
    poirenum->Allocate(sgridpoints->GetNumberOfPoints());
    startnodenum = globalpoints->GetNumberOfPoints();
  
    for (i=0; i < sgridpoints->GetNumberOfPoints(); i++)
      {
      // if point does not exist
      //if(globalpoints->GetNumberOfPoints() == 9728)
      //{
      // int x = 0;
      //}
      if(globallocator->IsInsertedPoint(sgridpoints->GetPoint(i)) == -1)
        {
        poirenum->InsertNextId(globalpoints->GetNumberOfPoints());
        globallocator->InsertNextPoint(sgridpoints->GetPoint(i));
        //FileOutput << globalpoints->GetNumberOfPoints()<<std::endl;
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
        //FileOutput << globalpoints->GetNumberOfPoints()<<std::endl;
        }
        }
      }
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
          bboxArray->InsertNextValue(m);
          }
        }
      }
    ptids->Delete();
    poirenum->Delete();
    }
  output->SetPoints(globalpoints);
  output->Squeeze();
  // add mesh seed data for node picking
  // first n elements contain mesh seed data corresponding to n building blocks
  // rest contain -1 as the values.
  vtkIntArray *intarray = vtkIntArray::New();
  intarray->SetNumberOfComponents(3);
  intarray->SetNumberOfTuples(bbox->GetNumberOfCells());
  //intarray->SetNumberOfTuples(output->GetNumberOfCells());
  //dim[0] = -1; dim[1] = -1; dim[2] = -1;
  //// initialize all values
  //for (i=0; i<output->GetNumberOfCells(); i++)
  //{
  //intarray->SetTupleValue(i, dim);
  //}
  for (i=0; i< bbox->GetNumberOfCells(); i++)
    {
    vtkIntArray::SafeDownCast(bbox->GetCellData()->GetArray("Mesh_Seed"))
      ->GetTupleValue(i,dim);
    intarray->SetTupleValue(i, dim);
    }
  intarray->SetName("Mesh_Seed");
  output->GetFieldData()->AddArray(intarray);
  output->GetCellData()->AddArray(bboxArray);
  bboxArray->Delete();
  intarray->Delete();
  globalpoints->Delete();
  globallocator->Delete();

  //***************************************************************************************
      if (sgridcollection)
      {
      int numCol = sgridcollection->GetNumberOfItems();
      sgridcollection->InitTraversal();
      do 
        {
        sgridcollection->GetNextItemAsObject()->Delete();
        numCol--;
        } while(numCol != 0);
      }

  sgridcollection->Delete();
  return 1;
  
}

void vtkMimxUnstructuredGridFromBoundingBox::SetBoundingBox(vtkUnstructuredGrid *UGrid)
{
  this->SetInput(0, UGrid);
}

void vtkMimxUnstructuredGridFromBoundingBox::SetSurface(vtkPolyData *PolyData)
{
  this->SetInput(1, PolyData);
}

int vtkMimxUnstructuredGridFromBoundingBox::FillInputPortInformation(
  int port,
  vtkInformation *info)
{
  if (port == 0 )
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
    return 1;
    }
  if(port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    return 1;
    }
  return 0;
}


void vtkMimxUnstructuredGridFromBoundingBox::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
