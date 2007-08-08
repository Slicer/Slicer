/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxMergeNodesPointAndCellData.cxx,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.3 $

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

#include "MergeNodesPointAndCellData.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkDataSetCollection.h"
#include "vtkIntArray.h"
#include "vtkMergeCells.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"

MergeNodesPointAndCellData::MergeNodesPointAndCellData()
{
  this->MergedDataSet = NULL;
}

MergeNodesPointAndCellData::~MergeNodesPointAndCellData()
{
  if (this->MergedDataSet) {
     this->MergedDataSet->Delete();
  }
}

void MergeNodesPointAndCellData::SetDataSetCollection(vtkDataSetCollection *Collection)
{
  this->DataSetCollection = Collection;
}

vtkUnstructuredGrid* MergeNodesPointAndCellData::GetMergedDataSet()
{
  return this->MergedDataSet;
}

int MergeNodesPointAndCellData::MergeDataSets()
{
  this->DataSetCollection->InitTraversal();
  int i,j,k,m;
  // determine the maximum numbered element set
  // check if node and element set point data exists
    if (this->CheckNodeAndElementNumbersPresent()) {
  // as of now only extra element sets have been incorporated
    // extra node sets and different data types other than 
  // unstructuredgrid has to be incorporated later.
  int maxelementsetnum = 0;
  for (i=0; i<DataSetCollection->GetNumberOfItems(); i++) {
    vtkDataSet *dataset = DataSetCollection->GetItem(i);
    if(dataset->GetDataObjectType()== VTK_UNSTRUCTURED_GRID)
    {
      vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(dataset);
      int numcelldataarrays = ugrid->GetCellData()->GetNumberOfArrays();
      // start with individual data sets
      // choose an element set and check if that element set exists
      // arrays named Node_Numbers and Element_Numbers are common for all the 
      // datasets.
      for (j=0; j<numcelldataarrays; j++) {
        const char * arrayname = ugrid->GetCellData()->GetArrayName(j);
        // if the string obtained is not Element_Numbers, Bounding_Box 
        // and FE_Mesh.
        // loop through other data sets
        if(strcmp(arrayname,"Element_Numbers"))
        {
          if(strcmp(arrayname,"Bounding_Box"))
          {
          if(strcmp(arrayname,"FE_Mesh"))
          {
          for (k=0; k<DataSetCollection->GetNumberOfItems(); k++) {
            if(k!=i)
            {
              if (!IsArrayPresent(k, arrayname)) {
                vtkDataSet *datasetcomp = DataSetCollection->GetItem(k);
                if(dataset->GetDataObjectType()== VTK_UNSTRUCTURED_GRID)
                {
                  vtkUnstructuredGrid *ugridcomp = 
                    vtkUnstructuredGrid::SafeDownCast(datasetcomp);
      
                vtkIntArray *elementarray = vtkIntArray::New();
                elementarray->SetNumberOfValues(ugridcomp->GetNumberOfCells());
                // by default if a cell data is created it is designated that
                // it does not belong to any other cell data of any other
                // data set
                for (m=0; m<ugridcomp->GetNumberOfCells(); m++) {
                  elementarray->SetValue(m, 0);
                }
                elementarray->SetName(arrayname);
                ugridcomp->GetCellData()->AddArray(elementarray);
                elementarray->Delete();
                }
              }
            }
          }  
        }
      }
        }
      }
    }
  }
  int numele = 0;
  int numnodes = 0;
  //  calculate number of nodes and elements
  for(i=0; i <this->DataSetCollection->GetNumberOfItems(); i++)
  {
    numnodes = numnodes + this->DataSetCollection->GetItem(i)->GetNumberOfPoints();
    numele = numele + this->DataSetCollection->GetItem(i)->GetNumberOfCells();
  }

  vtkMergeCells* mergecells = vtkMergeCells::New();
  this->MergedDataSet = vtkUnstructuredGrid::New();
  mergecells->SetUnstructuredGrid(this->MergedDataSet);
//  mergecells->SetPointMergeTolerance(0.001);
  mergecells->MergeDuplicatePointsOn();
  mergecells->SetTotalNumberOfDataSets(this->DataSetCollection->GetNumberOfItems());
  mergecells->SetTotalNumberOfCells(numele);
  mergecells->SetTotalNumberOfPoints(numnodes);
  for(i=0; i <this->DataSetCollection->GetNumberOfItems(); i++)
  {
    mergecells->MergeDataSet(this->DataSetCollection->GetItem(i));
  }
  mergecells->Finish();
    return 1;

  }
  else  return 0;
}

int MergeNodesPointAndCellData::IsArrayPresent(int DataSetNum, const char *ArrayName)
{
  vtkDataSet *dataset = DataSetCollection->GetItem(DataSetNum);
  if(dataset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
  {
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(dataset);
    int numcelldataarrays = ugrid->GetCellData()->GetNumberOfArrays();
    for (int i=0; i<numcelldataarrays; i++) {
      const char * arrayname = ugrid->GetCellData()->GetArrayName(i);
      if(!strcmp(arrayname,ArrayName))
      {
        return 1;
      }
    }
    return 0;
  }
  return 0;
}

int MergeNodesPointAndCellData::CheckNodeAndElementNumbersPresent()
{
  for (int i=0; i<DataSetCollection->GetNumberOfItems(); i++) {
    if(!DataSetCollection->GetItem(i)->GetCellData()->GetArray("Element_Numbers") ||
      !DataSetCollection->GetItem(i)->GetPointData()->GetArray("Node_Numbers"))
    {
      cout << "Arrays named Node_Numbers and Element_Numbers are not present in all data sets"<<endl;
      return 0;
    }
  }
  return 1;
}
