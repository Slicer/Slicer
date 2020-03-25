/*=========================================================================

  Program:   Realign Volumes
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "ACPCTransformCLP.h"
#include "vtkPluginFilterWatcher.h"
#include "vtkPrincipalAxesAlign.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLTransformStorageNode.h>

// VTK includes
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

} // end of anonymous namespace

//-----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
  PARSE_ARGS;

  if (ACPC.empty() && Midline.empty())
    {
    std::cerr << "ACPC Line or Midline points must be specified" << std::endl;
    return EXIT_FAILURE;
    }

  // fill in this transform with either the output or input matrix
  vtkNew<vtkTransform> transformToApply;
  transformToApply->PostMultiply();

  if( Midline.size() > 0 )
    {
    if (Midline.size() < 3)
    {
      std::cerr << "Midline expected to contain at least 3 points" << std::endl;
      return EXIT_FAILURE;
    }
    vtkNew<vtkPolyData> midlinePolydata;
    vtkNew<vtkPoints>   midlinePoints;
    midlinePoints->SetDataTypeToDouble();
    size_t x = Midline.size();
    midlinePoints->SetNumberOfPoints(x);
    for( size_t i = 0; i < x; ++i )
      {
      midlinePoints->SetPoint(i, Midline[i][0], Midline[i][1], Midline[i][2]);
      }
    midlinePolydata->SetPoints(midlinePoints.GetPointer());

    vtkNew<vtkPrincipalAxesAlign> pa;
    pa->SetInputData(midlinePolydata.GetPointer());
    pa->Update();

    double *normal = pa->GetZAxis();
    double  nx = normal[0];
    double  ny = normal[1];
    double  nz = normal[2];

    double Max = nx;
    if( ny * ny > Max * Max )
      {
      Max = ny;
      }
    if( nz * nz > Max * Max )
      {
      Max = nz;
      }
    double sign = 1.0;
    if( Max < 0 )
      {
      sign = -1.0;
      }

    // prepare the rotation matrix
    vtkNew<vtkMatrix4x4> mat;
    mat->Identity();
    for( size_t p = 0; p < 4; p++ )
      {
      double point = normal[p];
      mat->SetElement(static_cast<int>(p), 0, (sign * point) );
      }
    double oneAndAlpha = 1.0 + mat->GetElement(0, 0);
    mat->SetElement(0, 1, -1.0 * mat->GetElement(1, 0) );
    mat->SetElement(0, 2, (-1.0 * (mat->GetElement(2, 0) ) ) );
    mat->SetElement(2, 1, (-1.0 * (mat->GetElement(1, 0) * (mat->GetElement(2, 0) / oneAndAlpha) ) ) );
    mat->SetElement(1, 2, (-1.0 * (mat->GetElement(1, 0) * (mat->GetElement(2, 0) / oneAndAlpha) ) ) );
    mat->SetElement(1, 1, (1.0  - (mat->GetElement(1, 0) * (mat->GetElement(1, 0) / oneAndAlpha) ) ) );
    mat->SetElement(2, 2, (1.0  - (mat->GetElement(2, 0) * (mat->GetElement(2, 0) / oneAndAlpha) ) ) );

    vtkNew<vtkMatrix4x4> matInverse;
    matInverse->DeepCopy(mat.GetPointer());
    matInverse->Invert();
    transformToApply->SetMatrix(matInverse.GetPointer());
    }

  // need at least two points
  if( ACPC.size() > 0 )
    {
    if (ACPC.size() != 2)
      {
      std::cerr << "If ACPC line is specified then it must have exactly 2 points" << std::endl;
      return EXIT_FAILURE;
      }
    double top = ACPC[0][2] - ACPC[1][2];
    double bot = ACPC[0][1] - ACPC[1][1];
    double tangent = atan(top / bot) * (180.0 / (4.0 * atan(1.0) ) );
    transformToApply->RotateX(tangent * -1.0);
    }

  // Write result to output transform
  if (OutputTransform.empty())
    {
    std::cerr << "Output transform must be specified" << std::endl;
    return EXIT_FAILURE;
    }
  vtkNew<vtkMRMLLinearTransformNode> outputTransformNode;
  outputTransformNode->SetMatrixTransformToParent(transformToApply->GetMatrix());
  vtkNew<vtkMRMLTransformStorageNode> storageNode;
  storageNode->SetFileName(OutputTransform.c_str());
  if (!storageNode->WriteData(outputTransformNode))
    {
    std::cerr << "Failed to write output transform" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
