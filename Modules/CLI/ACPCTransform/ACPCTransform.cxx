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

// Markups includes
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsJsonStorageNode.h>
#include <vtkMRMLMarkupsLineNode.h>

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

  vtkNew<vtkMRMLMarkupsLineNode> acpcLineNode;
  if (!ACPC.empty())
    {
    vtkNew<vtkMRMLMarkupsJsonStorageNode> storageNode;
    storageNode->SetFileName(ACPC.c_str());
    if (!storageNode->ReadData(acpcLineNode))
      {
      std::cerr << "Failed to read ACPC line from file " << ACPC << std::endl;
      }
    }

  vtkNew<vtkMRMLMarkupsFiducialNode> midlinePointsNode;
  if (!Midline.empty())
    {
    vtkNew<vtkMRMLMarkupsJsonStorageNode> storageNode;
    storageNode->SetFileName(Midline.c_str());
    if (!storageNode->ReadData(midlinePointsNode))
      {
      std::cerr << "Failed to read midline points from file " << Midline << std::endl;
      }
    }

  if (acpcLineNode->GetNumberOfControlPoints() == 0
    && midlinePointsNode->GetNumberOfControlPoints() == 0)
    {
    std::cerr << "At least ACPC line or midline points must be specified" << std::endl;
    return EXIT_FAILURE;
    }

  // fill in this transform with either the output or input matrix
  vtkNew<vtkTransform> transformToApply;
  transformToApply->Identity();
  transformToApply->PostMultiply();

  if (centerVolume)
    {
    if (acpcLineNode->GetNumberOfControlPoints() < 1)
      {
      std::cerr << "Centering requires specification of ACPC line." << std::endl;
      return EXIT_FAILURE;
      }
    double pointAC[3] = { 0.0 };
    acpcLineNode->GetNthControlPointPosition(0, pointAC);
    transformToApply->Translate(-pointAC[0], -pointAC[1], -pointAC[2]);
    }

  if (midlinePointsNode->GetNumberOfControlPoints() > 0)
    {
    if (midlinePointsNode->GetNumberOfControlPoints() < 3)
      {
      std::cerr << "Midline must contain at least 3 points" << std::endl;
      return EXIT_FAILURE;
      }

    vtkNew<vtkPolyData> midlinePolydata;
    vtkNew<vtkPoints> midlinePoints;
    midlinePointsNode->GetControlPointPositionsWorld(midlinePoints);
    midlinePolydata->SetPoints(midlinePoints.GetPointer());

    vtkNew<vtkPrincipalAxesAlign> pa;
    pa->SetInputData(midlinePolydata);
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
    for( size_t p = 0; p < 3; p++ )
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
    vtkMatrix4x4::Invert(mat, matInverse);

    transformToApply->Concatenate(matInverse);
    }

  // need at least two points
  if (acpcLineNode->GetNumberOfControlPoints() > 0)
    {
    if (acpcLineNode->GetNumberOfControlPoints() != 2)
      {
      std::cerr << "If ACPC line is specified then it must be specified by exactly 2 points" << std::endl;
      return EXIT_FAILURE;
      }
    double pointAC[3] = { 0.0 };
    double pointPC[3] = { 0.0 };
    acpcLineNode->GetNthControlPointPosition(0, pointAC);
    acpcLineNode->GetNthControlPointPosition(1, pointPC);
    double top = pointAC[2] - pointPC[2];
    double bot = pointAC[1] - pointPC[1];
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
