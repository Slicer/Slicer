/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxCreateIntervertebralDiscElementSets.cxx,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.2 $

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

#include "CreateIntervertebralDiscElementSets.h"

#include "vtkCellData.h"
#include "vtkIntArray.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"

CreateIntervertebralDiscElementSets::CreateIntervertebralDiscElementSets()
{
  this->UGrid = NULL;
  this->BoundingBox = NULL;
  this->ElementSetsName = NULL;
  this->StartingElementSetNumber = 1;
}

CreateIntervertebralDiscElementSets::~CreateIntervertebralDiscElementSets()
{
}

void CreateIntervertebralDiscElementSets::SetUnstructuredGrid(vtkUnstructuredGrid *UnsGrid)
{
  this->UGrid = UnsGrid;
}

void CreateIntervertebralDiscElementSets::SetBoundingBox(vtkUnstructuredGrid *BBox)
{
  this->BoundingBox = BBox;
}

void CreateIntervertebralDiscElementSets::SetStartingElementSetNumber(int StartEleNum)
{
  this->StartingElementSetNumber = StartEleNum;
}

void CreateIntervertebralDiscElementSets::CreateElementSetArrays()
{
  // if an array already present
  //int i;
  //if(UGrid->GetCellData()->GetArray("Element_Numbers"))
  //{
  //  vtkIntArray *elementarray = 
  //    vtkIntArray::SafeDownCast(UGrid->GetCellData()->GetArray("Element_Numbers"));
  //  for (i=0; i<UGrid->GetNumberOfCells(); i++) {
  //    elementarray->SetValue(i, i+this->StartingElementNumber);
  //  }
  //}
  //else
  //{
  //  vtkIntArray *elementarray = vtkIntArray::New();
  //  elementarray->SetNumberOfValues(UGrid->GetNumberOfCells());

  //  for (i=0; i<UGrid->GetNumberOfCells(); i++) {
  //    elementarray->SetValue(i, i+this->StartingElementNumber);
  //  }
  //  elementarray->SetName("Element_Numbers");
  //  UGrid->GetCellData()->AddArray(elementarray);
  //  elementarray->Delete();
  //}
  int i,j;
  // seperate element sets for disc rings
  if(!strcmp(this->ElementSetsName, ""))
  {
    this->ElementSetsName = "Element_Set_";
  }
  double meshseed[3];
  BoundingBox->GetCellData()->GetVectors()->GetTuple(0, meshseed);
  for (i=0; i<meshseed[2]-1; i++) {
    char name[20];
    strcpy(name, this->ElementSetsName);
    char buffer[10];
    sprintf(buffer, "%d", i+this->StartingElementSetNumber);
    strcat(name, buffer);
    vtkIntArray *elementarray = vtkIntArray::New();
    elementarray->SetNumberOfValues(UGrid->GetNumberOfCells());

    for (j=0; j<UGrid->GetNumberOfCells(); j++) {
      elementarray->SetValue(j, 0);
    }
    // loop through the first 4 bounding boxes
    double dimtemp[3];
    for (j=0; j<4; j++) {
      int startelnum=0;
      for (int k=0; k<j; k++) {
        BoundingBox->GetCellData()->GetVectors()->GetTuple(k, dimtemp);
        startelnum = startelnum + (dimtemp[0]-1)*(dimtemp[1]-1)*(dimtemp[2]-1);
      }
      // loop through all the elements of a given face
      double dim[3];
      BoundingBox->GetCellData()->GetVectors()->GetTuple(j, dimtemp);
      dim[0] = dimtemp[2]-1;
      dim[1] = dimtemp[0]-1;
      dim[2] = dimtemp[1]-1;

      for(int y=0; y < dim[2]; y++)
      {
        for(int x=0; x < dim[1]; x++)
        {
          elementarray->SetValue(startelnum + (y*dim[0]*dim[1]+x*dim[0])+i,1);
        }
      }
    }
    elementarray->SetName(name);
    UGrid->GetCellData()->AddArray(elementarray);
    elementarray->Delete();
  }
  //element set for nucleus pulposus
  char name1[20];
  strcpy(name1, this->ElementSetsName);
  char buffer1[10];
  sprintf(buffer1, "%d", 4+this->StartingElementSetNumber);
  strcat(name1, buffer1);

  vtkIntArray *elementarray = vtkIntArray::New();
  elementarray->SetNumberOfValues(UGrid->GetNumberOfCells());

  for (j=0; j<UGrid->GetNumberOfCells(); j++) {
    elementarray->SetValue(j, 0);
  }

  int startelnum=0;
  double dimtemp[3];
  for (int k=0; k<4; k++) {
    BoundingBox->GetCellData()->GetVectors()->GetTuple(k, dimtemp);
    startelnum = startelnum + (dimtemp[0]-1)*(dimtemp[1]-1)*(dimtemp[2]-1);
  }
  // skip the first 4 bounding boxes
  for (j=startelnum; j<UGrid->GetNumberOfCells(); j++) {
    elementarray->SetValue(j, 1);
  }
  elementarray->SetName(name1);
  UGrid->GetCellData()->AddArray(elementarray);
  elementarray->Delete();
}

void CreateIntervertebralDiscElementSets::SetElementSetsName(const char *Name)
{
  this->ElementSetsName = Name;
}
