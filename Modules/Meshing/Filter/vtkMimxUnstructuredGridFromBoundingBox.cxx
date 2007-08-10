/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridFromBoundingBox.cxx,v $
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

vtkCxxRevisionMacro(vtkMimxUnstructuredGridFromBoundingBox, "$Revision: 1.4 $");
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
  for (int i=0; i < bbox->GetNumberOfCells(); i++) {
          sgridcollection->AddItem((vtkObject*)vtkStructuredGrid::New());

          vtkMimxMorphStructuredGrid *morphsgrid = vtkMimxMorphStructuredGrid::New();
          morphsgrid->SetGridStructure(bbox);
          morphsgrid->SetInput(bboxtosgrid->GetStructuredGrid(i));
          morphsgrid->SetSource(surface);
          morphsgrid->SetCellNum(i);
          morphsgrid->Update();

          ((vtkStructuredGrid*)sgridcollection->GetItemAsObject
                  (sgridcollection->GetNumberOfItems()-1))->DeepCopy(morphsgrid->GetOutput());
          morphsgrid->Delete();
  }
  int numele = 0;
  int numnodes = 0;
  int dim[3];
  //        calculate number of nodes and elements
  for(int i=0; i <sgridcollection->GetNumberOfItems(); i++)
  {
          ((vtkStructuredGrid*)(sgridcollection->
                  GetItemAsObject(i)))->GetDimensions(dim);
          numnodes = numnodes + dim[0]*dim[1]*dim[2];
          numele = numele + (dim[0]-1)*(dim[1]-1)*(dim[2]-1);
  }

  vtkMergeCells* mergecells = vtkMergeCells::New();
  mergecells->SetUnstructuredGrid(output);
  mergecells->MergeDuplicatePointsOn();
  mergecells->SetTotalNumberOfDataSets(sgridcollection->GetNumberOfItems());
  mergecells->SetTotalNumberOfCells(numele);
  mergecells->SetTotalNumberOfPoints(numnodes);
  for(int i=0; i <sgridcollection->GetNumberOfItems(); i++)
  {
          mergecells->MergeDataSet(((vtkStructuredGrid*)(sgridcollection->
                  GetItemAsObject(i))));
  }
  mergecells->Finish();
  mergecells->Delete();
  bboxtosgrid->Delete();

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
