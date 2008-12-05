/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEquivalancePoints.cxx,v $
Language:  C++
Date:      $Date: 2008/02/05 18:26:00 $
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

#include "vtkMimxEquivalancePoints.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxEquivalancePoints, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkMimxEquivalancePoints);


vtkMimxEquivalancePoints::vtkMimxEquivalancePoints()
{
}

vtkMimxEquivalancePoints::~vtkMimxEquivalancePoints()
{
}

int vtkMimxEquivalancePoints::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
 
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

   // get the complete and partial mesh
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
          inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType numPts,numCells;

  int i,j;

  // check for the input
  if(!input)
  {
          vtkErrorMacro("Set CompleteMesh File, The input should only be unstructured grid");
          return 0;
  }

  numPts = input->GetNumberOfPoints();
  numCells = input->GetNumberOfCells();

  if (numPts < 1 || numCells < 1)
  {
          vtkErrorMacro(<<"No data to Equivalence");
          return 0;
  }

  // start equivalancing
  vtkPointLocator *locator = vtkPointLocator::New();
  //SetDataSet for the locator does not seem to work should check later
  vtkPoints *newPts = vtkPoints::New();
  locator->InitPointInsertion(newPts, input->GetBounds());
  
  output->Allocate(input->GetNumberOfCells(), input->GetNumberOfCells());

  // equivalence points
  for (i=0; i<input->GetNumberOfCells(); i++)
  {
          vtkIdList *idlist = input->GetCell(i)->GetPointIds();
          vtkIdType celltype = input->GetCellType(i);
          vtkIdList *localidlist = vtkIdList::New();
          localidlist->SetNumberOfIds(idlist->GetNumberOfIds());
          for (j=0; j<idlist->GetNumberOfIds(); j++)
          {
                  int location = locator->IsInsertedPoint(input->GetPoint(idlist->GetId(j)));
                  if(location == -1)
                  {
                          locator->InsertNextPoint(input->GetPoint(idlist->GetId(j)));
                          localidlist->SetId(j, newPts->GetNumberOfPoints()-1);
                  }
                  else
                  {
                          localidlist->SetId(j, location);
                  }
          }
          output->InsertNextCell(celltype, localidlist);
          localidlist->Delete();
  }
  output->SetPoints(newPts);
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());
  newPts->Delete();
  locator->Delete();
  return 1;
}
//------------------------------------------------------------------------------------------
void vtkMimxEquivalancePoints::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//-------------------------------------------------------------------------------------------
