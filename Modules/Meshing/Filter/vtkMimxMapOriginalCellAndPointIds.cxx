/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMapOriginalCellAndPointIds.cxx,v $
Language:  C++
Date:      $Date: 2008/02/01 15:24:52 $
Version:   $Revision: 1.1 $

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

#include "vtkMimxMapOriginalCellAndPointIds.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxMapOriginalCellAndPointIds, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkMimxMapOriginalCellAndPointIds);


vtkMimxMapOriginalCellAndPointIds::vtkMimxMapOriginalCellAndPointIds()
{
  this->SetNumberOfInputPorts(2);
}

vtkMimxMapOriginalCellAndPointIds::~vtkMimxMapOriginalCellAndPointIds()
{
}

int vtkMimxMapOriginalCellAndPointIds::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
 
  // get the info objects
  vtkInformation *completeInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *partialInfo = inputVector[1]->GetInformationObject(0);

  // get the complete and partial mesh
  vtkUnstructuredGrid *completeMesh = vtkUnstructuredGrid::SafeDownCast(
    completeInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkUnstructuredGrid *partialMesh = vtkUnstructuredGrid::SafeDownCast(
    partialInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType partialnumPts,partialnumCells, completenumPts,completenumCells;
  bool completecellstatus = false, completepointstatus = false;

  int i,j;

  // check for the input
  if(!partialMesh)
    {
    vtkErrorMacro("Set PartialMesh File, The input should only be unstructured grid");
    return 0;
    }

  if(!completeMesh)
    {
    vtkErrorMacro("Set CompleteMesh File, The input should only be unstructured grid");
    return 0;
    }

  partialnumPts = partialMesh->GetNumberOfPoints();
  partialnumCells = partialMesh->GetNumberOfCells();

  completenumPts = completeMesh->GetNumberOfPoints();
  completenumCells = completeMesh->GetNumberOfCells();

  if (partialnumPts < 1 || partialnumCells < 1 || 
      completenumPts < 1 || completenumCells < 1)
    {
    vtkErrorMacro(<<"No data to map");
    return 0;
    }

  // check for cell types
  vtkCellTypes *partialcelltypes = vtkCellTypes::New();
  partialMesh->GetCellTypes(partialcelltypes);
  for(i =0; i < partialcelltypes->GetNumberOfTypes(); i++)
    {
    if(partialcelltypes->GetCellType(i) != 12)
      {
      vtkErrorMacro("Partial mesh should contain only hexahedral cells");
      partialcelltypes->Delete();
      return 0;
      }
    }
  partialcelltypes->Delete();

  vtkCellTypes *completecelltypes = vtkCellTypes::New();
  completeMesh->GetCellTypes(completecelltypes);
  for(i =0; i < completecelltypes->GetNumberOfTypes(); i++)
    {
    if(completecelltypes->GetCellType(i) != 12)
      {
      vtkErrorMacro("Complete mesh should contain only hexahedral cells");
      completecelltypes->Delete();
      return 0;
      }
    }
  completecelltypes->Delete();

  // Check if Original_Point_Ids and Original_Cell_Ids are already present in CompleteMesh
  // if not assign new Original_Point_Ids and Original_Cell_Ids
  vtkIntArray *completeOriginalPointIds =  vtkIntArray::SafeDownCast(
    completeMesh->GetPointData()->GetArray("Original_Point_Ids"));
  if (!completeOriginalPointIds)
    {
    completeOriginalPointIds = vtkIntArray::New();
    completeOriginalPointIds->SetNumberOfValues(completenumPts);
    for(i=0; i<completenumPts; i++)
      completeOriginalPointIds->SetValue(i,i);
    completeOriginalPointIds->SetName("Original_Point_Ids");
    completepointstatus = true;
    completeMesh->GetPointData()->AddArray(completeOriginalPointIds);
    }

  vtkIntArray *completeOriginalCellIds =  vtkIntArray::SafeDownCast(
    completeMesh->GetCellData()->GetArray("Original_Cell_Ids"));
  if (!completeOriginalCellIds)
    {
    completeOriginalCellIds = vtkIntArray::New();
    completeOriginalCellIds->SetNumberOfValues(completenumCells);
    for(i=0; i<completenumCells; i++)
      completeOriginalCellIds->SetValue(i,i);
    completeOriginalCellIds->SetName("Original_Cell_Ids");
    completecellstatus = true;
    completeMesh->GetCellData()->AddArray(completeOriginalCellIds);
    }
  
  // start mapping
  vtkPointLocator *locator = vtkPointLocator::New();
  //SetDataSet for the locator does not seem to work should check later
  vtkPoints *newPts = vtkPoints::New();
  locator->InitPointInsertion(newPts, completeMesh->GetBounds());
  for (i=0; i<completeMesh->GetNumberOfPoints(); i++)
    {
    //double x[3], y[3];
    if(locator->IsInsertedPoint(completeMesh->GetPoint(i)) != -1)
      {
      //cout << "Duplicate points "<<endl;
      //completeMesh->GetPoint(i, x);
      //completeMesh->GetPoint(location, y);
      vtkErrorMacro("Duplicate points found in CompleteMesh");
      if (completecellstatus)
        completeOriginalCellIds->Delete();
      if (completepointstatus)
        completeOriginalPointIds->Delete();
      locator->Delete();
      newPts->Delete();
      return 0;
      }
    locator->InsertNextPoint(completeMesh->GetPoint(i));
    }
  //locator->BuildLocator();

  // loop through all the points of partial mesh and find their location in complete mesh
  vtkIntArray *partialOriginalPointIds = vtkIntArray::New();
  partialOriginalPointIds->SetNumberOfValues(partialMesh->GetNumberOfPoints());
  partialOriginalPointIds->SetName("Original_Point_Ids");
  for (i=0; i<partialMesh->GetNumberOfPoints(); i++)
    {
    int location = locator->IsInsertedPoint(partialMesh->GetPoint(i));
    if(location == -1)
      {
      vtkErrorMacro("Points in PartialMesh does not correspond to the points in CompleteMesh");
      if (completecellstatus)
        completeOriginalCellIds->Delete();
      if (completepointstatus)
        completeOriginalPointIds->Delete();
      partialOriginalPointIds->Delete();
      locator->Delete();
      newPts->Delete();
      return 0;
      }
    partialOriginalPointIds->SetValue(i, location);
    }

  //check if the cell connectivities match
  vtkIntArray *partialOriginalCellIds = vtkIntArray::New();
  partialOriginalCellIds->SetName("Original_Cell_Ids");
  partialOriginalCellIds->SetNumberOfValues(partialMesh->GetNumberOfCells());

  for (i=0; i<partialMesh->GetNumberOfCells(); i++)
    {
    vtkIdList *pointids = partialMesh->GetCell(i)->GetPointIds();
    vtkIdList *originalpointids = vtkIdList::New();
    originalpointids->SetNumberOfIds(pointids->GetNumberOfIds());
    for(j=0; j<pointids->GetNumberOfIds(); j++)
      {
      originalpointids->SetId(j, partialOriginalPointIds->GetValue(pointids->GetId(j)));
      }
    //
    int cellnum = this->CheckCellConnectivity(originalpointids, completeMesh);
    if(cellnum == -1)
      {
      vtkErrorMacro("Cell connectivities do not correspond betweeen partial and complete mesh");
      if (completecellstatus)
        completeOriginalCellIds->Delete();
      if (completepointstatus)
        completeOriginalPointIds->Delete();
      partialOriginalPointIds->Delete();
      partialOriginalCellIds->Delete();
      locator->Delete();
      originalpointids->Delete();
      newPts->Delete();
      return 0;
      }
    partialOriginalCellIds->SetValue(i, cellnum);
    originalpointids->Delete();
    }
  // if input should not be modified change this section
  partialMesh->GetPointData()->AddArray(partialOriginalPointIds);
  partialMesh->GetCellData()->AddArray(partialOriginalCellIds);
  //
  if (completecellstatus)
    completeOriginalCellIds->Delete();
  if (completepointstatus)
    completeOriginalPointIds->Delete();
  partialOriginalPointIds->Delete();
  partialOriginalCellIds->Delete();
  newPts->Delete();
  locator->Delete();

  return 1;
}
//-----------------------------------------------------------------------------------------
void vtkMimxMapOriginalCellAndPointIds::SetCompleteMesh(vtkUnstructuredGrid *CompleteMesh)
{
  this->SetInput(0, CompleteMesh);
}
//-----------------------------------------------------------------------------------------
void vtkMimxMapOriginalCellAndPointIds::SetPartialMesh(vtkUnstructuredGrid *PartialMesh)
{
  this->SetInput(1, PartialMesh);
}
//------------------------------------------------------------------------------------------
vtkIdType vtkMimxMapOriginalCellAndPointIds::CheckCellConnectivity(
  vtkIdList *Connectivity, vtkUnstructuredGrid *CompleteMesh)
{
  int i,j;
  for (i=0; i<CompleteMesh->GetNumberOfCells(); i++)
    {
    vtkIdList *completelist = CompleteMesh->GetCell(i)->GetPointIds();
    int connnumpts = Connectivity->GetNumberOfIds();
    int comnumpts = completelist->GetNumberOfIds();
    if(connnumpts == comnumpts)
      {
      bool status = false;
      for (j=0; j<comnumpts; j++)
        {
        if(completelist->IsId(Connectivity->GetId(j)) == -1)
          {
          status = true;
          break;
          }
        }
      if(!status)
        {
        vtkIntArray *intarray = vtkIntArray::SafeDownCast(
          CompleteMesh->GetCellData()->GetArray("Original_Cell_Ids"));
        return intarray->GetValue(i);
        }
      }
    }
  return -1;
}
//------------------------------------------------------------------------------------------
void vtkMimxMapOriginalCellAndPointIds::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//-------------------------------------------------------------------------------------------
