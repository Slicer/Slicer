/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMergeNodesPointAndCellData.cxx,v $
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

#include "vtkMimxMergeNodesPointAndCellData.h"

#include "vtkAppendFilter.h"
#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkDataArray.h"
#include "vtkDataSetCollection.h"
#include "vtkIntArray.h"
#include "vtkMergeCells.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkLinkedList.h"
#include "vtkStringArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkFieldData.h"

vtkMimxMergeNodesPointAndCellData::vtkMimxMergeNodesPointAndCellData()
{
  this->MergedDataSet = NULL;
  this->Tol = 0.0;
  this->State = 0;
}

vtkMimxMergeNodesPointAndCellData::~vtkMimxMergeNodesPointAndCellData()
{
  if (this->MergedDataSet)
    {
    this->MergedDataSet->Delete();
    }
}

void vtkMimxMergeNodesPointAndCellData::SetDataSetCollection(vtkDataSetCollection *Collection)
{
  this->DataSetCollection = Collection;
}

vtkUnstructuredGrid* vtkMimxMergeNodesPointAndCellData::GetMergedDataSet()
{
  return this->MergedDataSet;
}

int vtkMimxMergeNodesPointAndCellData::MergeDataSets()
{

  int i,j,k,m;

  // determine the maximum numbered element set
  // check if node and element set point data exists
  if (this->CheckNodeAndElementNumbersPresent())
    {

    // as of now only extra element sets have been incorporated
    // extra node sets and different data types other than 
    // unstructuredgrid has to be incorporated later.

    int IsYoungsModulusArray = 0;
    vtkStringArray *refnodepar = NULL;
    vtkDataSetCollection *LocalDataSetCollection = vtkDataSetCollection::New();
    for (i=0; i<this->DataSetCollection->GetNumberOfItems(); i++)
      {
      LocalDataSetCollection->AddItem(vtkUnstructuredGrid::New());
      LocalDataSetCollection->GetItem(i)->DeepCopy(
        this->DataSetCollection->GetItem(i));
      if(this->DataSetCollection->GetItem(i)->GetCellData()->GetArray("Youngs_Modulus"))
        IsYoungsModulusArray = 1;
      if(this->DataSetCollection->GetItem(i)->GetFieldData()->GetAbstractArray("Reference_Node_Parameters"))
        {
        refnodepar = vtkStringArray::SafeDownCast(
          this->DataSetCollection->GetItem(i)->GetFieldData()->GetAbstractArray("Reference_Node_Parameters"));
        }
      }
    LocalDataSetCollection->InitTraversal();
    vtkStringArray *ElementSetNames = vtkStringArray::New();
    vtkStringArray *NodeSetNames = vtkStringArray::New();
    for (i=0; i<this->DataSetCollection->GetNumberOfItems(); i++)
      {
      vtkDataSet *dataset = LocalDataSetCollection->GetItem(i);
      if(dataset->GetDataObjectType()== VTK_UNSTRUCTURED_GRID)
        {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(dataset);
        vtkStringArray *strarray = vtkStringArray::SafeDownCast(
          ugrid->GetFieldData()->GetAbstractArray("Element_Set_Names"));
        //
        for (m=0; m<strarray->GetNumberOfTuples(); m++)
          {
          ElementSetNames->InsertNextValue(strarray->GetValue(m));
          }
        vtkCellData *celldata = ugrid->GetCellData();
        vtkDataArray *datasetarray;
        if(!strarray)   return 0;
        int numelementsets = strarray->GetNumberOfTuples();
        // start with individual data sets
        // choose an element set and check if that element set exists
        // arrays named Node_Numbers and Element_Numbers are common for all the 
        // datasets.
        // remove all the arrays that are not required during merging
        // start with youngsmodulus storage.
        if(IsYoungsModulusArray)
          {
          //if(ugrid->GetCellData()->GetArray("Youngs_Modulus"))
          //{
          vtkDoubleArray *doublearray = vtkDoubleArray::SafeDownCast(
            ugrid->GetCellData()->GetArray("Youngs_Modulus"));
          if(!doublearray)
            {
            vtkDoubleArray *youngsmodulus = vtkDoubleArray::New();
            int numcells = ugrid->GetNumberOfCells();
            youngsmodulus->SetNumberOfTuples(numcells);
            for (m=0; m<numcells; m++)
              {
              youngsmodulus->SetValue(m, -9999.0);
              }
            youngsmodulus->SetName("Youngs_Modulus");
            ugrid->GetCellData()->AddArray(youngsmodulus);
            youngsmodulus->Delete();
            }
          //}
          }
        for (j=0; j<numelementsets; j++) 
          {
          const char * arrayname = strarray->GetValue(j);
          // if the string obtained is not Element_Numbers, Bounding_Box 
          // and FE_Mesh.
          // loop through other data sets
          for (k=0; k<this->DataSetCollection->GetNumberOfItems(); k++)
            {
            if(k!=i)
              {
              vtkDataSet *datasetcomp = LocalDataSetCollection->GetItem(k);
              if(dataset->GetDataObjectType()== VTK_UNSTRUCTURED_GRID)
                {
                vtkUnstructuredGrid *ugridcomp = 
                  vtkUnstructuredGrid::SafeDownCast(datasetcomp);

                vtkIntArray *elementarray = vtkIntArray::New();
                elementarray->SetNumberOfValues(ugridcomp->GetNumberOfCells());
                // by default if a cell data is created it is designated that
                // it does not belong to any other cell data of any other
                // data set
                for (m=0; m<ugridcomp->GetNumberOfCells(); m++)
                  {
                  elementarray->SetValue(m, 0);
                  }
                elementarray->SetName(arrayname);
                ugridcomp->GetCellData()->AddArray(elementarray);
                elementarray->Delete();
                // create other data sets associated with the given element sets
        
                char ImageBased[256];
                strcpy(ImageBased, arrayname);
                strcat(ImageBased, "_Image_Based_Material_Property");
                datasetarray = celldata->GetArray(ImageBased);
                if(datasetarray)
                  {
                  vtkDoubleArray *imagebasedarray = vtkDoubleArray::New();
                  int numcells = ugridcomp->GetNumberOfCells();
                  imagebasedarray->SetNumberOfTuples(numcells);
                  for (m=0; m<numcells; m++)
                    {
                    imagebasedarray->SetValue(m, -1.0);
                    }
                  imagebasedarray->SetName(ImageBased);
                  ugridcomp->GetCellData()->AddArray(imagebasedarray);
                  imagebasedarray->Delete();
                  }

                strcat(ImageBased, "_ReBin");
                datasetarray = celldata->GetArray(ImageBased);
                if(datasetarray)
                  {
                  vtkDoubleArray *rebinarray = vtkDoubleArray::New();
                  int numcells = ugridcomp->GetNumberOfCells();
                  rebinarray->SetNumberOfTuples(numcells);
                  for (m=0; m<numcells; m++)
                    {
                    rebinarray->SetValue(m, -1.0);
                    }
                  rebinarray->SetName(ImageBased);
                  ugridcomp->GetCellData()->AddArray(rebinarray);
                  rebinarray->Delete();
                  }
                }
              }
            }       
          }
        // for node sets

        strarray = vtkStringArray::SafeDownCast(
          ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));

        for (m=0; m<strarray->GetNumberOfTuples(); m++)
          {
          NodeSetNames->InsertNextValue(strarray->GetValue(m));
          }

        int numpointdataarrays = strarray->GetNumberOfTuples();
        // start with individual data sets
        // choose an element set and check if that element set exists
        // arrays named Node_Numbers and Element_Numbers are common for all the 
        // datasets.
        for (j=0; j<numpointdataarrays; j++)
          {
          const char * arrayname = strarray->GetValue(j);
          // if the string obtained is not Element_Numbers, Bounding_Box 
          // and FE_Mesh.
          // loop through other data sets
          for (k=0; k<this->DataSetCollection->GetNumberOfItems(); k++)
            {
            if(k!=i)
              {
              vtkDataSet *datasetcomp = LocalDataSetCollection->GetItem(k);
              if(dataset->GetDataObjectType()== VTK_UNSTRUCTURED_GRID)
                {
                vtkUnstructuredGrid *ugridcomp = 
                  vtkUnstructuredGrid::SafeDownCast(datasetcomp);

                vtkIntArray *nodearray = vtkIntArray::New();
                nodearray->SetNumberOfValues(ugridcomp->GetNumberOfPoints());
                // by default if a cell data is created it is designated that
                // it does not belong to any other cell data of any other
                // data set
                for (m=0; m<ugridcomp->GetNumberOfPoints(); m++)
                  {
                  nodearray->SetValue(m, 0);
                  }
                nodearray->SetName(arrayname);
                ugridcomp->GetPointData()->AddArray(nodearray);
                nodearray->Delete();
                }
              }
            }       
          }
        }
      }
    // vtkMergeCells throws a warning if all the cell and node data is not consistent in all
    // data sets. hence we use appendfilter and use its output to merge cells.
    vtkAppendFilter *appendfilter = vtkAppendFilter::New();
    for(i=0; i <this->DataSetCollection->GetNumberOfItems(); i++)
      {
      appendfilter->AddInput(LocalDataSetCollection->GetItem(i));
      }
    appendfilter->Update();
    //
    int numele = 0;
    int numnodes = 0;
    //      calculate number of nodes and elements
    for(i=0; i <this->DataSetCollection->GetNumberOfItems(); i++)
      {
      numnodes = numnodes + this->DataSetCollection->GetItem(i)->GetNumberOfPoints();
      numele = numele + this->DataSetCollection->GetItem(i)->GetNumberOfCells();
      }

    vtkMergeCells* mergecells = vtkMergeCells::New();
    this->MergedDataSet = vtkUnstructuredGrid::New();
    mergecells->SetUnstructuredGrid(this->MergedDataSet);
    mergecells->SetPointMergeTolerance(this->Tol);
    mergecells->SetMergeDuplicatePoints(this->State);
//      mergecells->MergeDuplicatePointsOn();
    mergecells->SetTotalNumberOfDataSets(1);
    mergecells->SetTotalNumberOfCells(numele);
    mergecells->SetTotalNumberOfPoints(numnodes);
    //for(i=0; i <this->DataSetCollection->GetNumberOfItems(); i++)
    //{
    mergecells->MergeDataSet(appendfilter->GetOutput());
    //}
    mergecells->Finish();
    //
    NodeSetNames->SetName("Node_Set_Names");
    this->MergedDataSet->GetFieldData()->AddArray(NodeSetNames);
    NodeSetNames->Delete();
    ElementSetNames->SetName("Element_Set_Names");
    this->MergedDataSet->GetFieldData()->AddArray(ElementSetNames);
    ElementSetNames->Delete();
        
    // reference node parameter
    if(refnodepar)
      {
      vtkStringArray *refnodearray = vtkStringArray::New();
      refnodearray->SetName("Reference_Node_Parameters");
      refnodearray->DeepCopy(refnodepar);
      this->MergedDataSet->GetFieldData()->AddArray(refnodearray);
      refnodearray->Delete();
      }
    // copy all the field data belonging to all the data sets in the collection.
    int numitems = this->DataSetCollection->GetNumberOfItems();
        
    for (i=0; i<numitems; i++)
      {
      vtkDataSet *dataset = this->DataSetCollection->GetItem(i);
      if(dataset->GetDataObjectType()== VTK_UNSTRUCTURED_GRID)
        {
        vtkFieldData *fielddata = dataset->GetFieldData();
        vtkStringArray *strarray = vtkStringArray::SafeDownCast(
          fielddata->GetAbstractArray("Element_Set_Names"));
        vtkDataArray *datasetarray;
        // element related data
        for (j=0; j< strarray->GetNumberOfTuples(); j++)
          {
          const char *Name = strarray->GetValue(j);
          char Young[256];
          strcpy(Young, Name);
          strcat(Young, "_Constant_Youngs_Modulus");
          datasetarray = fielddata->GetArray(Young);
          if(datasetarray)
            {
            vtkDoubleArray *floatarray = vtkDoubleArray::New();
            floatarray->DeepCopy(vtkDoubleArray::SafeDownCast(
                                   datasetarray));
            floatarray->SetName(Young);
            this->MergedDataSet->GetFieldData()->AddArray(floatarray);
            floatarray->Delete();
            }
          char Poisson[256];
          strcpy(Poisson, Name);
          strcat(Poisson, "_Constant_Poissons_Ratio");
          datasetarray = fielddata->GetArray(Poisson);
          if(datasetarray)
            {
            vtkFloatArray *floatarray = vtkFloatArray::New();
            floatarray->DeepCopy(vtkFloatArray::SafeDownCast(
                                   datasetarray));
            floatarray->SetName(Poisson);
            this->MergedDataSet->GetFieldData()->AddArray(floatarray);
            floatarray->Delete();
            }
          }
        }
      }
    for (i=0; i<this->DataSetCollection->GetNumberOfItems(); i++)
      LocalDataSetCollection->GetItem(i)->Delete();
    LocalDataSetCollection->Delete();
    appendfilter->Delete();
    return 1;
    }
  return 0;
}

int vtkMimxMergeNodesPointAndCellData::IsArrayPresent(int DataSetNum, const char *ArrayName)
{
  vtkDataSet *dataset = DataSetCollection->GetItem(DataSetNum);
  if(dataset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(dataset);
    int numcelldataarrays = ugrid->GetCellData()->GetNumberOfArrays();
    for (int i=0; i<numcelldataarrays; i++)
      {
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

int vtkMimxMergeNodesPointAndCellData::IsNodeArrayPresent(int DataSetNum, const char *ArrayName)
{
  vtkDataSet *dataset = DataSetCollection->GetItem(DataSetNum);
  if(dataset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(dataset);
    int numpointdataarrays = ugrid->GetPointData()->GetNumberOfArrays();
    for (int i=0; i<numpointdataarrays; i++)
      {
      const char * arrayname = ugrid->GetPointData()->GetArrayName(i);
      if(!strcmp(arrayname,ArrayName))
        {
        return 1;
        }
      }
    return 0;
    }
  return 0;
}

int vtkMimxMergeNodesPointAndCellData::CheckNodeAndElementNumbersPresent()
{
  for (int i=0; i<DataSetCollection->GetNumberOfItems(); i++)
    {
    if(!DataSetCollection->GetItem(i)->GetCellData()->GetArray("Element_Numbers") ||
       !DataSetCollection->GetItem(i)->GetPointData()->GetArray("Node_Numbers"))
      {
      cout << "Arrays named Node_Numbers and Element_Numbers are not present in all data sets"<<endl;
      return 0;
      }
    }
  return 1;
}
