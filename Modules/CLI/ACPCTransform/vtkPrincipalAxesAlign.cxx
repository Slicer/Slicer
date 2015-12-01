/*=========================================================================

  Program:   Realign Volumes
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#include "vtkPrincipalAxesAlign.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVersion.h>

//----------------------------------------------------------------------------
int vtkPrincipalAxesAlign::RequestData(vtkInformation* vtkNotUsed(request),
      vtkInformationVector** inInfoVec,
      vtkInformationVector* vtkNotUsed(outInfoVec))
{
  vtkPolyData *         input = vtkPolyData::GetData(inInfoVec[0]);

  vtkIdType             nr_points = input->GetNumberOfPoints();
  double* x;

  int i;

  // reset the intermediate arrays
  for( i = 0; i < 3; i++ )
    {
    Center[i] = 0;
    }
  for( i = 0; i < 3; i++ )
    {
    for( int j = 0; j < 3; j++ )
      {
      eigenvalueProblem[i][j] = 0;
      }
    }
  for( i = 0; i < 3; i++ )
    {
    for( int j = 0; j < 3; j++ )
      {
      eigenvalueProblemDiag[i][j] = 0;
      }
    }
  // compute the center
  for( vtkIdType j = 0; j < input->GetNumberOfPoints(); j++ )
    {
    x = input->GetPoint(j);
    Center[0] += x[0];
    Center[1] += x[1];
    Center[2] += x[2];
    }

  Center[0] =   Center[0] / nr_points;
  Center[1] =   Center[1] / nr_points;
  Center[2] =   Center[2] / nr_points;

  // create the eigenvalue-problem
  // using the symmetry of the result matrix
  for( i = 0; i < 3; i++ )
    {
    for( int j = i; j < 3; j++ )
      {
      eigenvalueProblem[i][j] = -Center[i] * nr_points * Center[j];
      }
    }
  for( vtkIdType m = 0; m < nr_points; m++ )
    {
    x = input->GetPoint(m);
    for( i = 0; i < 3; i++ )
      {
      for( int j = i; j < 3; j++ )
        {
        eigenvalueProblemDiag[i][j] +=  x[i] * x[j];
        }
      }
    }
  for( i = 0; i < 3; i++ )
    {
    for( int j = i; j < 3; j++ )
      {
      eigenvalueProblem[i][j] += eigenvalueProblemDiag[i][j];
      }
    }
  for( i = 0; i < 3; i++ )
    {
    for( int j = 0; j < i; j++ )
      {
      eigenvalueProblem[i][j] = eigenvalueProblem[j][i];
      }
    }
  vtkMath::Jacobi(eigenvalueProblem, eigenvalues, eigenvectors);

  // update Axes
  XAxis[0] = eigenvectors[0][0];
  XAxis[1] = eigenvectors[1][0];
  XAxis[2] = eigenvectors[2][0];

  YAxis[0] = eigenvectors[0][1];
  YAxis[1] = eigenvectors[1][1];
  YAxis[2] = eigenvectors[2][1];

  ZAxis[0] = eigenvectors[0][2];
  ZAxis[1] = eigenvectors[1][2];
  ZAxis[2] = eigenvectors[2][2];
  return 1;
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPrincipalAxesAlign);

//----------------------------------------------------------------------------
vtkPrincipalAxesAlign::vtkPrincipalAxesAlign()
{
  Center = (double *) malloc(3 * sizeof(double) );
  Center[0] = 0;
  Center[1] = 0;
  Center[2] = 0;

  XAxis = (double *) malloc(3 * sizeof(double) );
  XAxis[0] = 1;
  XAxis[1] = 0;
  XAxis[2] = 0;

  YAxis = (double *) malloc(3 * sizeof(double) );
  YAxis[0] = 0;
  YAxis[1] = 1;
  YAxis[2] = 0;

  ZAxis = (double *) malloc(3 * sizeof(double) );
  ZAxis[0] = 0;
  ZAxis[1] = 0;
  ZAxis[2] = 1;

  eigenvalueProblem = (double * *)malloc(sizeof(double *) * 3);
  int i;
  for( i = 0; i < 3; i++ )
    {
    eigenvalueProblem[i] = (double *) malloc(3 * sizeof(double) );
    eigenvalueProblem[i][0] = eigenvalueProblem[i][1] = eigenvalueProblem[i][2] = 0.0;
    }

  eigenvalueProblemDiag = (double * *)malloc(sizeof(double *) * 3);
  for( i = 0; i < 3; i++ )
    {
    eigenvalueProblemDiag[i] = (double *) malloc(3 * sizeof(double) );
    eigenvalueProblemDiag[i][0] = eigenvalueProblemDiag[i][1] = eigenvalueProblemDiag[i][2] = 0.0;
    }
  eigenvectors = (double * *)malloc(sizeof(double *) * 3);
  for( i = 0; i < 3; i++ )
    {
    eigenvectors[i] = (double *) malloc(3 * sizeof(double) );
    eigenvectors[i][0] = eigenvectors[i][1] = eigenvectors[i][2] = 0.0;
    }
  eigenvalues = (double *)malloc(sizeof(double) * 3);
  eigenvalues[0] = eigenvalues[1] = eigenvalues[2] = 0.0;
}

//----------------------------------------------------------------------------
vtkPrincipalAxesAlign::~vtkPrincipalAxesAlign()
{
  free(Center);
  free(XAxis);
  free(YAxis);
  free(ZAxis);
  for( int i = 0; i < 3; i++ )
    {
    free(eigenvalueProblem[i]);
    free(eigenvalueProblemDiag[i]);
    free(eigenvectors[i]);
    }
  free(eigenvalueProblem);
  free(eigenvalueProblemDiag);
  free(eigenvectors);
  free(eigenvalues);
}

//----------------------------------------------------------------------------
void vtkPrincipalAxesAlign::PrintSelf(ostream& os, vtkIndent indent)
{
  int i;

  this->Superclass::PrintSelf(os, indent);

  os << indent << "Center: " << endl;
  if( this->Center )
    {
    os << indent << this->Center[0] << " " << this->Center[1] << " " << this->Center[2] << endl;
    }

  os << indent << "XAxis: " << endl;
  if( this->XAxis )
    {
    os << indent << this->XAxis[0] << " " << this->XAxis[1] << " " << this->XAxis[2] << endl;
    }

  os << indent << "YAxis: " << endl;
  if( this->YAxis )
    {
    os << indent << this->YAxis[0] << " " << this->YAxis[1] << " " << this->YAxis[2] << endl;
    }

  os << indent << "ZAxis: " << endl;
  if( this->ZAxis )
    {
    os << indent <<  this->ZAxis[0] << " " << this->ZAxis[1] << " " << this->ZAxis[2] << endl;
    }

  os << indent << "eigenvalueProblem: " << endl;
  if( this->eigenvalueProblem )
    {
    for( i = 0; i < 3; i++ )
      {
      if( this->eigenvalueProblem[i] )
        {
        os << indent << indent << i << ": " << this->eigenvalueProblem[i][0] << " " << this->eigenvalueProblem[i][1]
           << " " << this->eigenvalueProblem[i][2] << endl;
        }
      }
    }

  os << indent << "eigenvalueProblemDiag: " << endl;
  if( this->eigenvalueProblemDiag )
    {
    for( i = 0; i < 3; i++ )
      {
      if( this->eigenvalueProblemDiag[i] )
        {
        os << indent << indent << i << ": " << this->eigenvalueProblemDiag[i][0] << " "
           << this->eigenvalueProblemDiag[i][1] << " " << this->eigenvalueProblemDiag[i][2] << endl;
        }
      }
    }

  os << indent << "eigenvectors: " << endl;
  if( this->eigenvectors )
    {
    for( i = 0; i < 3; i++ )
      {
      if( this->eigenvectors[i] )
        {
        os << indent << indent << i << ": " << this->eigenvectors[i][0] << " " << this->eigenvectors[i][1] << " "
           << this->eigenvectors[i][2] << endl;
        }
      }
    }

  os << indent << "eigenvalues: " << endl;
  if( this->eigenvalues )
    {
    os << indent << this->eigenvalues[0] << " " << this->eigenvalues[1] << " " << this->eigenvalues[2] << endl;
    }
}
