/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMirrorUnstructuredHexahedronGridCell.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.5 $

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

#include "vtkMimxMirrorUnstructuredHexahedronGridCell.h"

#include "vtkCell.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxMirrorUnstructuredHexahedronGridCell, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxMirrorUnstructuredHexahedronGridCell);

// Description:

vtkMimxMirrorUnstructuredHexahedronGridCell::vtkMimxMirrorUnstructuredHexahedronGridCell()
{
        // default set to x
        this->Axis = 0;
        this->MirrorPoint = 0.0; 
}

vtkMimxMirrorUnstructuredHexahedronGridCell::~vtkMimxMirrorUnstructuredHexahedronGridCell()
{
}

int vtkMimxMirrorUnstructuredHexahedronGridCell::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
 
  vtkIdType numCells = input->GetNumberOfCells();
  vtkIdType numPts = input->GetNumberOfPoints();
  if(numCells < 1 || numPts <8)
  {
          vtkErrorMacro("Invalid input data");
          return 0;
  }
// Initialize and check input
  // if the input is valid
  
  // storage of output
  output->Allocate(2*input->GetNumberOfCells(), 2*input->GetNumberOfCells());

  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(0);
  for (int i=0; i < input->GetNumberOfPoints(); i++)
  {
          points->InsertNextPoint(input->GetPoint(i));
  }
  output->SetPoints(points);
  points->Delete();

  int i;

  for(i=0; i <input->GetNumberOfCells(); i++)
  {
          vtkIdList *locallist = vtkIdList::New();
          locallist->DeepCopy(input->GetCell(i)->GetPointIds());
          output->InsertNextCell(input->GetCellType(i), locallist);
          locallist->Delete();
  }
  // add new cells based on mirroring point and the axis
 
  vtkIdList *idlist = vtkIdList::New();
  double x[3];
  for(i = 0; i < numPts; i++)
  {
          input->GetPoint(i,x);
          x[this->Axis] = 2.0*this->MirrorPoint - x[this->Axis];
          output->GetPoints()->InsertNextPoint(x);
  }
  for(i=0; i <numCells; i++)
  {
          idlist->Initialize();
          idlist->SetNumberOfIds(8);
        /*  for(int j=0; j<8; j++)
          {
                  idlist->SetId(j,input->GetCell(i)->GetPointId(j)+numPts);
          }*/
          // during mirroring the right hand rule for the connectivity changes
          idlist->SetId(0, input->GetCell(i)->GetPointId(3)+numPts);
          idlist->SetId(1, input->GetCell(i)->GetPointId(2)+numPts);
          idlist->SetId(2, input->GetCell(i)->GetPointId(1)+numPts);
          idlist->SetId(3, input->GetCell(i)->GetPointId(0)+numPts);
          idlist->SetId(4, input->GetCell(i)->GetPointId(7)+numPts);
          idlist->SetId(5, input->GetCell(i)->GetPointId(6)+numPts);
          idlist->SetId(6, input->GetCell(i)->GetPointId(5)+numPts);
          idlist->SetId(7, input->GetCell(i)->GetPointId(4)+numPts);
          output->InsertNextCell(12, idlist);
  }
  idlist->Delete();
  output->Squeeze();
 return 1;
  
}

void vtkMimxMirrorUnstructuredHexahedronGridCell::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
