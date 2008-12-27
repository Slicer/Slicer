/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMirrorUnstructuredHexahedronGridCell.cxx,v $
Language:  C++
Date:      $Date: 2008/05/16 22:32:04 $
Version:   $Revision: 1.9 $

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
#include "vtkPlane.h"
#include "vtkMath.h"
#include "vtkIntArray.h"
#include "vtkPointLocator.h"
#include "vtkPointData.h"


vtkCxxRevisionMacro(vtkMimxMirrorUnstructuredHexahedronGridCell, "$Revision: 1.9 $");
vtkStandardNewMacro(vtkMimxMirrorUnstructuredHexahedronGridCell);

// Description:

vtkMimxMirrorUnstructuredHexahedronGridCell::vtkMimxMirrorUnstructuredHexahedronGridCell()
{
  // default set to x
  this->MirrorPlane = NULL;
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
  int i,j;
  for(i=0; i < input->GetNumberOfPoints(); i++)
    {
    points->InsertNextPoint(input->GetPoint(i));
    }
  output->SetPoints(points);
  points->Delete();


  for(i=0; i <input->GetNumberOfCells(); i++)
    {
    vtkIdList *locallist = vtkIdList::New();
    locallist->DeepCopy(input->GetCell(i)->GetPointIds());
    output->InsertNextCell(input->GetCellType(i), locallist);
    locallist->Delete();
    }
  // add new cells based on the plane set
  // order is translation towards the center of the plane about y, about x
  // and reverse translation 
  // check if the mirroring plane is parallel to any of the 3 orthogonal axes
  // if yes use default values to mirror
  double x[3];
  vtkIdList *idlist = vtkIdList::New();

  double *normal = this->MirrorPlane->GetNormal();
  double *center = this->MirrorPlane->GetOrigin();

  for (i=0; i<numPts; i++)
    {
    input->GetPoint(i, x);
    // compute the closest distance of a given point to the plane
    double dist = vtkPlane::DistanceToPlane(x, normal, center);
    // compute the dot product of the normal and the vector defining the center and
    // point of interest.
    double vec[3];
    for(j=0; j<3; j++)
      {
      vec[j] = x[j]-center[j];
      }
    double norm = vtkMath::Norm(vec);
    if(norm)
      vtkMath::Normalize(vec);
    // find the dot product
    double dotproduct = vtkMath::Dot(normal, vec);
    double xtrans[3];
    for(j=0; j<3; j++)
      {
      if(dotproduct >= 0.0)
        {
        xtrans[j] = x[j] - 2.0*normal[j]*dist;
        }
      else
        {
        xtrans[j] = x[j] + 2.0*normal[j]*dist;
        }
      }
    output->GetPoints()->InsertNextPoint(xtrans);
    }
  //if(planenor[0] != 1.0 && planenor[1] != 1.0 && planenor[2] != 1.0)
  //{
  // // calculate the transformation matrices
  // double roty[3][3], rotx[3][3], rotxy[3][3], invrotxy[3][3];
  // for (i=0; i<3; i++)
  // {
  //  for (j=0; j<3; j++)
  //  {
  //        rotx[i][j] = 0.0;
  //        roty[i][j] = 0.0;
  //  }
  // }
  // double costheta = planenor[2]/sqrt(pow(planenor[0],2.0) + pow(planenor[2],2.0));
  // double sintheta = planenor[0]/sqrt(pow(planenor[0],2.0) + pow(planenor[2],2.0));

  // rotx[0][0] = costheta;      rotx[0][2] = sintheta;  rotx[1][1] = 1.0;
  // rotx[2][0] = -sintheta;     rotx[2][2] = costheta;

  // costheta = planenor[2]/sqrt(pow(planenor[1],2.0) + pow(planenor[2],2.0));
  // sintheta = planenor[1]/sqrt(pow(planenor[1],2.0) + pow(planenor[2],2.0));

  // roty[0][0] = 1.0;   roty[1][1] = costheta;  roty[1][2] = sintheta;
  // roty[2][1] = -sintheta;     roty[2][2] = costheta;

  // vtkMath::Multiply3x3(rotx, roty, rotxy);
  // vtkMath::Invert3x3(rotxy, invrotxy);

  //for (i=0; i<numPts; i++)
  //{
  // input->GetPoint(i, x);
  // // translate
  // for (j=0; j<3; j++) x[j] -=center[j];
  // vtkMath::Multiply3x3(rotxy, x, xmod);
  // xmod[2] = -xmod[2];
  // vtkMath::Multiply3x3(invrotxy, xmod,x);
  // for (j=0; j<3; j++) x[j] +=center[j];
  // output->GetPoints()->InsertNextPoint(x);
  //}
  //}
  //else{
  //int Axis;
  //for (i=0; i<3;i++)
  //{
  //      if(planenor[i] == 1.0) Axis = i;
  //}

  //input->GetPoint(i, x);
  //for(i = 0; i < numPts; i++)
  //{
  // input->GetPoint(i,x);
  // x[Axis] = 2.0*center[Axis] - x[Axis];
  // output->GetPoints()->InsertNextPoint(x);
  //}

  //}

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
  this->CopyConstraintValues(input, output);
  return 1;
  
}

//----------------------------------------------------------------------------------
void vtkMimxMirrorUnstructuredHexahedronGridCell::CopyConstraintValues(
  vtkUnstructuredGrid *input, vtkUnstructuredGrid *output)
{
  vtkIntArray *constrain = vtkIntArray::SafeDownCast(
    input->GetPointData()->GetArray("Constrain"));
  if(constrain)
    {
    vtkPoints *inpoints = input->GetPoints();
    vtkPoints *outpoints = output->GetPoints();
    vtkPointLocator *locator = vtkPointLocator::New();

    int i;
    vtkPoints *points = vtkPoints::New();
    locator->InitPointInsertion(points, outpoints->GetBounds());
    for (i=0; i<inpoints->GetNumberOfPoints(); i++)
      {
      locator->InsertNextPoint(inpoints->GetPoint(i));
      }

    vtkIntArray *constrainout = vtkIntArray::New();
    constrainout->SetNumberOfValues(outpoints->GetNumberOfPoints());
    for (i=0; i<outpoints->GetNumberOfPoints(); i++)
      {
      vtkIdType pos;
      pos = locator->IsInsertedPoint(outpoints->GetPoint(i));
      if (pos != -1)
        {
        constrainout->SetValue(i, constrain->GetValue(pos));
        }
      else
        {
        constrainout->SetValue(i, 0);
        }
      }
    constrainout->SetName("Constrain");
    output->GetPointData()->AddArray(constrainout);
    constrainout->Delete();
    locator->Delete();
    points->Delete();
    }
}
//------------------------------------------------------------------------------------
void vtkMimxMirrorUnstructuredHexahedronGridCell::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
