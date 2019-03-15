/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>

#include "vtkOrientedBSplineTransform.h"
#include "vtkOrientedGridTransform.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLScene.h"

#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkPointSource.h>
#include <vtkTransform.h>

#include "vtkMRMLCoreTestingMacros.h"

int TestBSplineTransform(const char *filename);
int TestGridTransform(const char *filename);
int TestThinPlateSplineTransform(const char *filename);
int TestCompositeTransformHardenSplit(const char *filename);
int TestBSplineLinearCompositeTransformSplit(const char *filename);
int TestRelativeTransforms(const char *filename);
int TestGetTransform();

int vtkMRMLNonlinearTransformNodeTest1(int argc, char * argv[] )
{
  itk::itkFactoryRegistration();

  const char *filename = nullptr;
  if (argc > 1)
    {
    filename = argv[1];
    }

  CHECK_EXIT_SUCCESS(TestBSplineTransform(filename));
  CHECK_EXIT_SUCCESS(TestGridTransform(filename));
  CHECK_EXIT_SUCCESS(TestThinPlateSplineTransform(filename));
  CHECK_EXIT_SUCCESS(TestCompositeTransformHardenSplit(filename));
  CHECK_EXIT_SUCCESS(TestBSplineLinearCompositeTransformSplit(filename));
  CHECK_EXIT_SUCCESS(TestRelativeTransforms(filename));
  CHECK_EXIT_SUCCESS(TestGetTransform());

  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int transformPoints(vtkAbstractTransform *transform, vtkPoints* sourcePoints, vtkPoints* transformedPoints)
{
  CHECK_NOT_NULL(transform);
  transform->TransformPoints(sourcePoints, transformedPoints);
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
bool isSamePointPositions(vtkPoints* sourcePoints, vtkPoints* transformedPoints)
{
  for (int i=0; i<sourcePoints->GetNumberOfPoints(); i++)
    {
    if (vtkMath::Distance2BetweenPoints(sourcePoints->GetPoint(i), transformedPoints->GetPoint(i))>0.1*0.1)
      {
      return false;
      }
    }
  return true;
}

// Checks if the transform changes point positions and inverse transform moves points back to original position
//---------------------------------------------------------------------------
int testTransformConsistency(vtkAbstractTransform *xfp, vtkAbstractTransform *xtp, vtkPoints* testPoints,
  vtkPoints* transformedPoints, vtkPoints* transformedPointsBackToTest)
{
  // Test if transform actually changes point positions
  CHECK_EXIT_SUCCESS(transformPoints(xfp, testPoints, transformedPoints));
  CHECK_BOOL(isSamePointPositions(testPoints, transformedPoints), false);

  // Test if the inverse transform moves back the point to its original position
  CHECK_EXIT_SUCCESS(transformPoints(xtp, transformedPoints, transformedPointsBackToTest));
  CHECK_BOOL(isSamePointPositions(testPoints, transformedPointsBackToTest), true);

  return EXIT_SUCCESS;
}

// Checks if the transform accurate enough by comparing point positions to ground truth
//---------------------------------------------------------------------------
int testTransformAccuracy(vtkAbstractTransform *xfp, vtkAbstractTransform *xtp, vtkPoints* testPoints,
  vtkPoints* transformedPointsGroundTruth, vtkPoints* transformedPointsBackToTestGroundTruth)
{
  // Test if the copied from parent transform gives the same results as the original
  vtkNew<vtkPoints> transformedPoints;
  CHECK_EXIT_SUCCESS(transformPoints(xfp, testPoints, transformedPoints.GetPointer()));
  CHECK_BOOL(isSamePointPositions(transformedPoints.GetPointer(), transformedPointsGroundTruth), true);

  // Test if the copied to parent transform gives the same results as the original
  vtkNew<vtkPoints> transformedPointsBackToSource;
  CHECK_EXIT_SUCCESS(transformPoints(xtp, transformedPoints.GetPointer(), transformedPointsBackToSource.GetPointer()));
  CHECK_BOOL(isSamePointPositions(transformedPointsBackToSource.GetPointer(), transformedPointsBackToTestGroundTruth), true);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int testTransformConsistencyAndCopy(const char *filename, const char* nodeId, const char* expectedTransformType)
{
  // Read a BSpline transform from a scene
  vtkNew<vtkMRMLScene> scene;

  scene->SetURL(filename);
  scene->Import();

  vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(scene->GetNodeByID(nodeId));
  CHECK_NOT_NULL(transformNode);

  // Create test point set where the transform will be tested at
  vtkNew<vtkPointSource> pointSource;
  pointSource->SetCenter(0,0,0);
  pointSource->SetNumberOfPoints(100);
  pointSource->SetRadius(25.0);
  pointSource->Update();
  vtkPoints* testPoints = pointSource->GetOutput()->GetPoints();

  vtkNew<vtkPoints> transformedPoints;
  vtkNew<vtkPoints> transformedPointsBackToTest;

  // Test if both the transform and its inverse are available
  vtkAbstractTransform *xfp = transformNode->GetTransformFromParentAs(expectedTransformType);
  vtkAbstractTransform *xtp = transformNode->GetTransformToParentAs(expectedTransformType);

  CHECK_EXIT_SUCCESS(testTransformConsistency(xfp, xtp, testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()));

  // Test if node copy creates an independent copy
  vtkNew<vtkMRMLTransformNode> transformNodeCopy;
  transformNodeCopy->Copy(transformNode);
  // Reset the original transform to make sure that it was not a shallow copy (transforms from the original transforms are not reused)
  vtkNew<vtkTransform> emptyTransform;
  transformNode->SetAndObserveTransformToParent(emptyTransform.GetPointer());

  vtkAbstractTransform *xfpCopy = transformNodeCopy->GetTransformFromParentAs(expectedTransformType);
  vtkAbstractTransform *xtpCopy = transformNodeCopy->GetTransformToParentAs(expectedTransformType);

  CHECK_EXIT_SUCCESS(testTransformAccuracy(xfpCopy, xtpCopy, testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()));

  // Cleanup
  scene->Clear(1);
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestBSplineTransform(const char *filename)
{
  CHECK_EXIT_SUCCESS(testTransformConsistencyAndCopy(filename, "vtkMRMLBSplineTransformNode1", "vtkOrientedBSplineTransform"));
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGridTransform(const char *filename)
{
  CHECK_EXIT_SUCCESS(testTransformConsistencyAndCopy(filename, "vtkMRMLGridTransformNode1", "vtkOrientedGridTransform"));
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestThinPlateSplineTransform(const char *filename)
{
  CHECK_EXIT_SUCCESS(testTransformConsistencyAndCopy(filename, "vtkMRMLTransformNode1", "vtkThinPlateSplineTransform"));
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestCompositeTransformHardenSplit(const char *filename)
{
  // Read a BSpline transform from a scene
  vtkNew<vtkMRMLScene> scene;

  scene->SetURL(filename);
  scene->Import();

  vtkNew<vtkMRMLTransformNode> infoPrinter;

  // Create a composite transform from bspline and a grid transform
  // WORLD
  //  |-- gridTransformNode
  //       |-- bsplineTransformNode
  vtkMRMLBSplineTransformNode *bsplineTransformNode = vtkMRMLBSplineTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLBSplineTransformNode1"));
  vtkMRMLGridTransformNode *gridTransformNode = vtkMRMLGridTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLGridTransformNode1"));
  bsplineTransformNode->SetAndObserveTransformNodeID(gridTransformNode->GetID());

  // Create test point set where the transform will be tested at
  vtkNew<vtkPointSource> pointSource;
  pointSource->SetCenter(0,0,0);
  pointSource->SetNumberOfPoints(100);
  pointSource->SetRadius(25.0);
  pointSource->Update();
  vtkPoints* testPoints = pointSource->GetOutput()->GetPoints();

  vtkNew<vtkPoints> transformedPoints;
  vtkNew<vtkPoints> transformedPointsBackToTest;

  // Test if transform is consistent (transform is not identity and invertible)
  vtkNew<vtkGeneralTransform> transformToWorldBeforeHardening;
  bsplineTransformNode->GetTransformToWorld(transformToWorldBeforeHardening.GetPointer());
  std::cout << "Transform to world before hardening: " << std::endl;
  std::cout << infoPrinter->GetTransformInfo(transformToWorldBeforeHardening.GetPointer()) << std::endl;

  vtkNew<vtkGeneralTransform> transformFromWorldBeforeHardening;
  bsplineTransformNode->GetTransformFromWorld(transformFromWorldBeforeHardening.GetPointer());
  CHECK_EXIT_SUCCESS(testTransformConsistency(transformFromWorldBeforeHardening.GetPointer(), transformToWorldBeforeHardening.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()));

  // Harden transform
  vtkNew<vtkGeneralTransform> hardeningTransform;
  bsplineTransformNode->GetParentTransformNode()->GetTransformToWorld(hardeningTransform.GetPointer());
  hardeningTransform->Update();
  bsplineTransformNode->ApplyTransform(hardeningTransform.GetPointer());
  bsplineTransformNode->SetAndObserveTransformNodeID(nullptr);

  // Test if transform to world is the same after hardening
  vtkNew<vtkGeneralTransform> transformToWorldAfterHardening;
  bsplineTransformNode->GetTransformToWorld(transformToWorldAfterHardening.GetPointer());
  std::cout << "Transform to world after hardening: " << std::endl;
  std::cout << infoPrinter->GetTransformInfo(transformToWorldAfterHardening.GetPointer()) << std::endl;
  vtkNew<vtkGeneralTransform> transformFromWorldAfterHardening;
  bsplineTransformNode->GetTransformFromWorld(transformFromWorldAfterHardening.GetPointer());
  CHECK_EXIT_SUCCESS(testTransformAccuracy(transformFromWorldAfterHardening.GetPointer(), transformToWorldAfterHardening.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()));

  // Split trasnform
  CHECK_BOOL(bsplineTransformNode->Split(), true);

  // Test if transform to world is the same after splitting
  transformToWorldAfterHardening->Identity();
  bsplineTransformNode->GetTransformToWorld(transformToWorldAfterHardening.GetPointer());
  std::cout << "Transform to world after hardening: " << std::endl;
  std::cout << infoPrinter->GetTransformInfo(transformToWorldAfterHardening.GetPointer()) << std::endl;
  transformFromWorldAfterHardening->Identity();
  bsplineTransformNode->GetTransformFromWorld(transformFromWorldAfterHardening.GetPointer());
  CHECK_EXIT_SUCCESS(testTransformAccuracy(transformFromWorldAfterHardening.GetPointer(), transformToWorldAfterHardening.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()));

  // Cleanup
  scene->Clear(1);
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestBSplineLinearCompositeTransformSplit(const char *filename)
{
  // Read a BSpline transform from a scene
  vtkNew<vtkMRMLScene> scene;

  scene->SetURL(filename);
  scene->Import();

  // Get composite transform

  vtkMRMLTransformNode *compositeBsplineTransformNode = vtkMRMLTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLTransformNode2"));

  // Create test point set where the transform will be tested at
  vtkNew<vtkPointSource> pointSource;
  pointSource->SetCenter(0,0,0);
  pointSource->SetNumberOfPoints(100);
  pointSource->SetRadius(25.0);
  pointSource->Update();
  vtkPoints* testPoints = pointSource->GetOutput()->GetPoints();

  vtkNew<vtkPoints> transformedPoints;
  vtkNew<vtkPoints> transformedPointsBackToTest;

  // Test if transform is consistent (transform is not identity and invertible)
  vtkNew<vtkGeneralTransform> transformToWorldBeforeSplit;
  compositeBsplineTransformNode->GetTransformToWorld(transformToWorldBeforeSplit.GetPointer());
  vtkNew<vtkGeneralTransform> transformFromWorldBeforeSplit;
  compositeBsplineTransformNode->GetTransformFromWorld(transformFromWorldBeforeSplit.GetPointer());
  CHECK_EXIT_SUCCESS(testTransformConsistency(transformFromWorldBeforeSplit.GetPointer(), transformToWorldBeforeSplit.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()));

  // Split transform
  CHECK_BOOL(compositeBsplineTransformNode->Split(), true);

  // Test if transform to world is the same after splitting
  vtkNew<vtkGeneralTransform> transformToWorldAfterSplit;
  compositeBsplineTransformNode->GetTransformToWorld(transformToWorldAfterSplit.GetPointer());
  vtkNew<vtkGeneralTransform> transformFromWorldAfterSplit;
  compositeBsplineTransformNode->GetTransformFromWorld(transformFromWorldAfterSplit.GetPointer());

  CHECK_EXIT_SUCCESS(testTransformAccuracy(transformFromWorldAfterSplit.GetPointer(), transformToWorldAfterSplit.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()));

  // Cleanup
  scene->Clear(1);
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestRelativeTransforms(const char *filename)
{
  // Read a BSpline transform from a scene
  vtkNew<vtkMRMLScene> scene;

  scene->SetURL(filename);
  scene->Import();

  vtkNew<vtkMRMLTransformNode> infoPrinter;

  // Create transform tree

  // WORLD
  //  |-- gridTransformNode
  //       |-- bsplineTransformNode
  //       |      |-- compositeTransformNode
  //       |-- tpsTransformNode
  //              |-- linearTransformNode1
  //              |-- linearTransformNode2

  vtkMRMLTransformNode *gridTransformNode = vtkMRMLTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLGridTransformNode1"));
  vtkMRMLTransformNode *bsplineTransformNode = vtkMRMLTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLBSplineTransformNode1"));
  vtkMRMLTransformNode *compositeTransformNode = vtkMRMLTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLTransformNode2"));
  vtkMRMLTransformNode *tpsTransformNode = vtkMRMLTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLTransformNode1"));

  bsplineTransformNode->SetAndObserveTransformNodeID(gridTransformNode->GetID());
  compositeTransformNode->SetAndObserveTransformNodeID(bsplineTransformNode->GetID());
  tpsTransformNode->SetAndObserveTransformNodeID(gridTransformNode->GetID());

  // Create test point set where the transform will be tested at
  vtkNew<vtkPointSource> pointSource;
  pointSource->SetCenter(0,0,0);
  pointSource->SetNumberOfPoints(100);
  pointSource->SetRadius(25.0);
  pointSource->Update();
  vtkPoints* testPoints = pointSource->GetOutput()->GetPoints();

  vtkNew<vtkPoints> transformedPoints;
  vtkNew<vtkPoints> transformedPointsBackToTest;

  // Test if relative transform between two nodes is consistent (transform is not identity and invertible)
  vtkNew<vtkGeneralTransform> compositeToTps;
  compositeTransformNode->GetTransformToNode(tpsTransformNode, compositeToTps.GetPointer());
  std::cout << "Composite transform to tps: " << std::endl;
  std::cout << infoPrinter->GetTransformInfo(compositeToTps.GetPointer()) << std::endl;

  vtkNew<vtkGeneralTransform> tpsToComposite;
  tpsTransformNode->GetTransformToNode(compositeTransformNode, tpsToComposite.GetPointer());
  std::cout << "Tps transform to composite: " << std::endl;
  std::cout << infoPrinter->GetTransformInfo(tpsToComposite.GetPointer()) << std::endl;
  CHECK_EXIT_SUCCESS(testTransformConsistency(tpsToComposite.GetPointer(), compositeToTps.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()));

  // Check if the relative transform only goes through the common parent
  // (does not contain gridTransformNode)
  vtkNew<vtkCollection> compositeToTpsTransformList;
  vtkMRMLTransformNode::FlattenGeneralTransform(compositeToTpsTransformList.GetPointer(), compositeToTps.GetPointer());

  CHECK_INT(compositeToTpsTransformList->GetNumberOfItems(), 4);
  CHECK_BOOL(vtkAbstractTransform::SafeDownCast(compositeToTpsTransformList->GetItemAsObject(0))->IsA("vtkLinearTransform"), true);
  CHECK_BOOL(vtkAbstractTransform::SafeDownCast(compositeToTpsTransformList->GetItemAsObject(1))->IsA("vtkBSplineTransform"), true);
  CHECK_BOOL(vtkAbstractTransform::SafeDownCast(compositeToTpsTransformList->GetItemAsObject(2))->IsA("vtkBSplineTransform"), true);
  CHECK_BOOL(vtkAbstractTransform::SafeDownCast(compositeToTpsTransformList->GetItemAsObject(3))->IsA("vtkThinPlateSplineTransform"), true);

  // Cleanup
  scene->Clear(1);
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGetTransform()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLTransformNode> transformWithToParentSet;
  vtkNew<vtkOrientedBSplineTransform> transform1;
  transformWithToParentSet->SetAndObserveTransformToParent(transform1.GetPointer());
  scene->AddNode(transformWithToParentSet.GetPointer());

  // Get transform for reading
  CHECK_NOT_NULL(transformWithToParentSet->GetTransformToParentAs("vtkOrientedBSplineTransform"));
  CHECK_NOT_NULL(transformWithToParentSet->GetTransformFromParentAs("vtkOrientedBSplineTransform"));
  // Get transform for writing
  CHECK_NOT_NULL(transformWithToParentSet->GetTransformToParentAs("vtkOrientedBSplineTransform", false, true));
  CHECK_NULL(transformWithToParentSet->GetTransformFromParentAs("vtkOrientedBSplineTransform", false, true));

  vtkNew<vtkMRMLTransformNode> transformWithFromParentSet;
  vtkNew<vtkOrientedGridTransform> transform2;
  transformWithFromParentSet->SetAndObserveTransformFromParent(transform2.GetPointer());
  scene->AddNode(transformWithFromParentSet.GetPointer());

  // Get transform for reading
  CHECK_NOT_NULL(transformWithFromParentSet->GetTransformToParentAs("vtkOrientedGridTransform"));
  CHECK_NOT_NULL(transformWithFromParentSet->GetTransformFromParentAs("vtkOrientedGridTransform"));
  // Get transform for writing
  CHECK_NULL(transformWithFromParentSet->GetTransformToParentAs("vtkOrientedGridTransform", false, true));
  CHECK_NOT_NULL(transformWithFromParentSet->GetTransformFromParentAs("vtkOrientedGridTransform", false, true));

  return EXIT_SUCCESS;
}
