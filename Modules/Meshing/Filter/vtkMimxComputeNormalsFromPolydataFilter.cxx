/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxComputeNormalsFromPolydataFilter.cxx,v $
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

#include "vtkMimxComputeNormalsFromPolydataFilter.h"

#include "vtkPolyData.h"
#include "vtkTriangle.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"

vtkCxxRevisionMacro(vtkMimxComputeNormalsFromPolydataFilter, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkMimxComputeNormalsFromPolydataFilter);

vtkMimxComputeNormalsFromPolydataFilter::vtkMimxComputeNormalsFromPolydataFilter()
{
}

vtkMimxComputeNormalsFromPolydataFilter::~vtkMimxComputeNormalsFromPolydataFilter()
{
}


//----------------------------------------------------------------------------
int vtkMimxComputeNormalsFromPolydataFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPointSet *output = vtkPointSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));


  vtkPoints *Normals = vtkPoints::New( );
  double initialValue = -2.0;
  vtkIdType numberOfPoints = input->GetNumberOfPoints();
  vtkCellArray *cellArray = input->GetPolys(); // Gives number of surface faces

  Normals->SetNumberOfPoints( numberOfPoints );
  for ( vtkIdType i = 0; i < numberOfPoints; i++ )
  {
    Normals->InsertPoint( i, initialValue, initialValue, initialValue);
  }
  vtkIdType* pts=0;
  vtkIdType t=0;
  int num_neigh;
  vtkTriangle* tria = vtkTriangle::New();
  for( int j = 0; j < numberOfPoints; j++ )
  {
    num_neigh = 0;
    cellArray->InitTraversal();
    vtkPoints* Store_Coor = vtkPoints::New();
    Store_Coor->SetNumberOfPoints(1);

    while(cellArray->GetNextCell(t,pts))
    {
      if(pts[0] == j)
      {
        Store_Coor->InsertPoint(num_neigh,pts[0],pts[1],pts[3]);
        num_neigh++;
      }
      if(pts[1] == j)
      {
        Store_Coor->InsertPoint(num_neigh,pts[1],pts[2],pts[0]);
        num_neigh++;
      }
      if(pts[2] == j)
      {
        Store_Coor->InsertPoint(num_neigh,pts[2],pts[3],pts[1]);
        num_neigh++;
      }
      if(pts[3] == j)
      {
        Store_Coor->InsertPoint(num_neigh,pts[3],pts[0],pts[2]);
        num_neigh++;
      }
    }
    int num_ent = Store_Coor->GetNumberOfPoints();
    double x_comp = 0, y_comp = 0, z_comp = 0;
    double x1[3],x2[3],x3[3],normal[3];
    double x[3];
    if(num_neigh !=0)
    {
      for(int i=0;i<num_ent;i++)
      {
        Store_Coor->GetPoint(i,x);

        input->GetPoint(static_cast<int>(x[0]),x1);
        input->GetPoint(static_cast<int>(x[1]),x2);
        input->GetPoint(static_cast<int>(x[2]),x3);
        tria->ComputeNormal(x1,x2,x3,normal);
        x_comp = x_comp + normal[0];
        y_comp = y_comp + normal[1];
        z_comp = z_comp + normal[2];
      }
      if(num_ent !=0)
      {
        x_comp = x_comp / num_ent;
        y_comp = y_comp / num_ent;
        z_comp = z_comp / num_ent;
        double norm = sqrt(pow(x_comp,2)+
          pow(y_comp,2)+pow(z_comp,2));
        x_comp = x_comp / norm;
        y_comp = y_comp / norm;
        z_comp = z_comp / norm;
        Normals->SetPoint( j, x_comp, y_comp, z_comp);
      }
    }
    
    Store_Coor->Delete();
  }
  output->SetPoints( Normals );
  Normals->Delete();
  return 1;
}


void vtkMimxComputeNormalsFromPolydataFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
