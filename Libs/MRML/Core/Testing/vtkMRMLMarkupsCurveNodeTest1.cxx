/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointLocator.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

// STD includes
#include <iostream>

//----------------------------------------------------------------------------
int TestGetClosestPointPositionAlongCurveWorld();
int TestGetClosestPointPositionAlongCurveWithLocator();

//----------------------------------------------------------------------------
int vtkMRMLMarkupsCurveNodeTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestGetClosestPointPositionAlongCurveWorld());
  CHECK_EXIT_SUCCESS(TestGetClosestPointPositionAlongCurveWithLocator());
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestGetClosestPointPositionAlongCurveWorld()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLMarkupsCurveNode* curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsCurveNode"));
  CHECK_NOT_NULL(curveNode);
  curveNode->AddControlPoint(0.0, 0.0, 0.0);
  curveNode->AddControlPoint(10.0, 5.0, 0.0);
  curveNode->AddControlPoint(20.0, 0.0, 3.0);
  curveNode->AddControlPoint(30.0, 8.0, 0.0);

  vtkPoints* curvePoints = curveNode->GetCurvePointsWorld();
  CHECK_NOT_NULL(curvePoints);
  CHECK_BOOL(curvePoints->GetNumberOfPoints() > 2, true);

  // When the input position is exactly on an inner curve point, the returned
  // line index must be the index of that curve point, regardless of the
  // initial content of the output buffer (see Slicer issue #9558).
  for (vtkIdType curvePointIndex = 1; curvePointIndex < curvePoints->GetNumberOfPoints() - 1; ++curvePointIndex)
  {
    double curvePoint[3] = { 0.0, 0.0, 0.0 };
    curvePoints->GetPoint(curvePointIndex, curvePoint);

    double closestPosZeroInitialized[3] = { 0.0, 0.0, 0.0 };
    CHECK_INT(curveNode->GetClosestPointPositionAlongCurveWorld(curvePoint, closestPosZeroInitialized), curvePointIndex);

    double closestPosPrefilled[3] = { curvePoint[0], curvePoint[1], curvePoint[2] };
    CHECK_INT(curveNode->GetClosestPointPositionAlongCurveWorld(curvePoint, closestPosPrefilled), curvePointIndex);

    // Result must match the code path that does not use a point locator
    double closestPosNoLocator[3] = { 0.0, 0.0, 0.0 };
    CHECK_INT(vtkMRMLMarkupsCurveNode::GetClosestPointPositionAlongCurve(curvePoints, curvePoint, closestPosNoLocator), curvePointIndex);

    for (int i = 0; i < 3; ++i)
    {
      CHECK_DOUBLE_TOLERANCE(closestPosZeroInitialized[i], curvePoint[i], 1e-6);
      CHECK_DOUBLE_TOLERANCE(closestPosPrefilled[i], curvePoint[i], 1e-6);
      CHECK_DOUBLE_TOLERANCE(closestPosNoLocator[i], curvePoint[i], 1e-6);
    }
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestGetClosestPointPositionAlongCurveWithLocator()
{
  // Polyline passing near the origin, with the closest curve point far from the input position.
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(-10.0, 1.0, 0.0);
  points->InsertNextPoint(10.0, 1.0, 0.0);
  points->InsertNextPoint(30.0, 1.0, 0.0);
  vtkNew<vtkPolyData> polyData;
  polyData->SetPoints(points);
  vtkNew<vtkPointLocator> pointLocator;
  pointLocator->SetDataSet(polyData);
  pointLocator->BuildLocator();

  // The closest position is on the first line segment, between curve points 0 and 1.
  // The output buffer is zero-initialized, as in vtkSlicerCurveWidget. Before the fix,
  // the distance to the closest curve point was computed from this buffer (0.25 instead
  // of about 91.25), so the closer position on the line segment was rejected and
  // curve point 1 (9.55 away) was returned instead.
  const double pos[3] = { 0.5, 0.0, 0.0 };
  const double expectedClosestPos[3] = { 0.5, 1.0, 0.0 };

  double closestPosWithLocator[3] = { 0.0, 0.0, 0.0 };
  CHECK_INT(vtkMRMLMarkupsCurveNode::GetClosestPointPositionAlongCurve(points, pos, closestPosWithLocator, pointLocator), 0);

  double closestPosWithoutLocator[3] = { 0.0, 0.0, 0.0 };
  CHECK_INT(vtkMRMLMarkupsCurveNode::GetClosestPointPositionAlongCurve(points, pos, closestPosWithoutLocator), 0);

  for (int i = 0; i < 3; ++i)
  {
    CHECK_DOUBLE_TOLERANCE(closestPosWithLocator[i], expectedClosestPos[i], 1e-6);
    CHECK_DOUBLE_TOLERANCE(closestPosWithoutLocator[i], expectedClosestPos[i], 1e-6);
  }

  return EXIT_SUCCESS;
}
