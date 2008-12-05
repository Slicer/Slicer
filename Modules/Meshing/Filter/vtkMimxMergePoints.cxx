/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMergePoints.cxx,v $
Language:  C++
Date:      $Date: 2008/08/08 15:07:41 $
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

#include "vtkMimxMergePoints.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkExecutive.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMergeCells.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPointLocator.h"
#include "vtkUnstructuredGrid.h"
#include "vtkExtractCells.h"

#include "vtkMimxMapOriginalCellAndPointIds.h"

vtkCxxRevisionMacro(vtkMimxMergePoints, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkMimxMergePoints);


vtkMimxMergePoints::vtkMimxMergePoints()
{
        this->Tolerance = 0.0;
}

vtkMimxMergePoints::~vtkMimxMergePoints()
{
}

int vtkMimxMergePoints::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
 
  // get the info objects
  vtkInformation *completeInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

   // get the complete and partial mesh
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
          completeInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkUnstructuredGrid *output =  vtkUnstructuredGrid::SafeDownCast(
          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numPts = input->GetNumberOfPoints();
  int numCells = input->GetNumberOfCells();
  int i,j;
  vtkPointLocator *locator = vtkPointLocator::New();
  locator->SetDataSet(input);
  vtkPoints *locatorpoints = vtkPoints::New();
  locatorpoints->Allocate(numPts);
  locator->InitPointInsertion(locatorpoints, input->GetBounds());
  for (i=0; i<numPts; i++)
  {
          locator->InsertNextPoint(input->GetPoint(i));
  }

  vtkIdList *plist = vtkIdList::New();
  plist->SetNumberOfIds(numPts);
  for (i=0; i<numPts; i++)
  {
          vtkIdList *LocalList = vtkIdList::New();
          double x[3];
          input->GetPoint(i, x);
          locator->FindPointsWithinRadius(this->Tolerance, x, LocalList);
          int numIds = LocalList->GetNumberOfIds();       
          if(numIds)
          {
                int minId = LocalList->GetId(0);
                for (j=1; j<numIds; j++)
                {
                        if(LocalList->GetId(j) < minId) minId = LocalList->GetId(j);
                }
                plist->SetId(i, minId);
          }
          LocalList->Delete();
  }

  vtkUnstructuredGrid *interimGrid = vtkUnstructuredGrid::New();
  interimGrid->Allocate(numCells, numCells);
  interimGrid->SetPoints(input->GetPoints());
  for (i=0; i<numCells; i++)
  {
          vtkIdList *cellIdList = vtkIdList::New();
          input->GetCellPoints(i, cellIdList);
          vtkIdList *cIdList = vtkIdList::New();
          cIdList->SetNumberOfIds(cellIdList->GetNumberOfIds());
          for (j=0; j<cellIdList->GetNumberOfIds(); j++)
          {
                  cIdList->SetId(j, plist->GetId(cellIdList->GetId(j)));
          }
          interimGrid->InsertNextCell(input->GetCellType(i), cIdList);
          cIdList->Delete();
          cellIdList->Delete();
  }
  vtkIdList *extractList = vtkIdList::New();
  for(i=0; i<numCells; i++)     extractList->InsertNextId(i);
  vtkExtractCells *extract = vtkExtractCells::New();
  extract->SetInput(interimGrid);
  extract->SetCellList(extractList);
  extract->Update();

  output->DeepCopy(extract->GetOutput());
  extract->Delete();
  extractList->Delete();
  interimGrid->Delete();
  plist->Delete();
  locator->Delete();
  locatorpoints->Delete();
   return 1;
}
//------------------------------------------------------------------------------------------
void vtkMimxMergePoints::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//-------------------------------------------------------------------------------------------
