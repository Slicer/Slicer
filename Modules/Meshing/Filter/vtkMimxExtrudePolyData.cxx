/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtrudePolyData.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
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

#include "vtkMimxExtrudePolyData.h"

#include "vtkCell.h"
#include "vtkDataArray.h"
#include "vtkHexahedron.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"




vtkCxxRevisionMacro(vtkMimxExtrudePolyData, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkMimxExtrudePolyData);

// Description:

vtkMimxExtrudePolyData::vtkMimxExtrudePolyData()
{
        this->ExtrusionLength = 1.0;
        this->ReverseExtrusionDirection = 0;
}

vtkMimxExtrudePolyData::~vtkMimxExtrudePolyData()
{
}

int vtkMimxExtrudePolyData::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        
  // get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the output as Unstructured grid

  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
  outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  // get the input as polydata 
  vtkPolyData *input = vtkPolyData::SafeDownCast(
  inInfo->Get(vtkDataObject::DATA_OBJECT()));


  
  if(input->GetNumberOfCells()<=0||input->GetNumberOfPoints()<=0)
  {
          vtkErrorMacro("Invalid Input");
          return 0;
  }


// check extrusion length
  if(this->ExtrusionLength <= 0.0)
  {
          vtkErrorMacro("Extrusion Length should always be > 0");
          return 0;
  }
 


  vtkPolyDataNormals *normals = vtkPolyDataNormals::New();
  normals->SetInput(input);
  normals->AutoOrientNormalsOn();
  normals->SetComputePointNormals(1);        
  normals->Update();


  vtkDataArray* dataarray = normals->GetOutput()->GetPointData()->GetNormals();
  vtkPoints* point1 = vtkPoints::New();
  point1->DeepCopy(input->GetPoints());


  int i,j; vtkIdType point=0; unsigned short  noofcells; vtkIdType* cells;
  int count = input->GetNumberOfPoints();
  double x[3],xnew[3],normalx[3];
  vtkIdList* idlist1;
  vtkIdList* idlist2;
  vtkIdList* idlist3 = vtkIdList::New();
  idlist1 = vtkIdList::New();
  idlist2 = vtkIdList::New(); 
  
  double actualextrusionlength ;

  if (ReverseExtrusionDirection)
  {
        actualextrusionlength = this->ExtrusionLength*(-1.0);
  }

  else

        actualextrusionlength = this->ExtrusionLength;

  input->BuildLinks();
  for (i=0;i<input->GetNumberOfPoints();i++)

  {
          input->GetPointCells(i,noofcells,cells);
          if (noofcells>0)
          {
                  idlist1->InsertNextId(i);
                  idlist2->InsertNextId(count);
                  input->GetPoint(i,x);
                  dataarray->GetTuple(i,normalx);
                  for (j=0;j<3; j++)
                  {
                          xnew[j] = x[j] + normalx[j]*(actualextrusionlength);

                  }

                  point1->InsertPoint(count,xnew);        
//                  cout<<count<<" " <<xnew[0] <<" " <<xnew[1] <<" " <<xnew[2] <<endl;
                  count++;

          }
  }

  vtkIdList *cellidlist = vtkIdList::New();

  output->Allocate(input->GetNumberOfCells(), input->GetNumberOfCells());

  for (i=0;i<input->GetNumberOfCells();i++)
  {
          idlist3->Initialize();
          input->GetCellPoints(i,idlist3);
          cellidlist->Initialize();
          cellidlist->SetNumberOfIds(2*idlist3->GetNumberOfIds());
          for (j=0;j<idlist3->GetNumberOfIds();j++)
          {
                  cellidlist->InsertId(j, idlist3->GetId(j));
                  cellidlist->InsertId(j+4,idlist2->GetId(idlist1->IsId(idlist3->GetId(j))));
          }
          output->InsertNextCell(12, cellidlist);
  }

  output->SetPoints(point1);

  normals->Delete();
  point1->Delete();
  idlist1->Delete();
  idlist2->Delete();
  idlist3->Delete();
  cellidlist->Delete();

  return 1;
}

void vtkMimxExtrudePolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

int vtkMimxExtrudePolyData::FillInputPortInformation(int, vtkInformation *info)
{
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
        return 1;
}
