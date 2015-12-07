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

bool TestBSplineTransform(const char *filename);
bool TestGridTransform(const char *filename);
bool TestThinPlateSplineTransform(const char *filename);
bool TestCompositeTransformHardenSplit(const char *filename);
bool TestBSplineLinearCompositeTransformSplit(const char *filename);
bool TestRelativeTransforms(const char *filename);

int vtkMRMLNonlinearTransformNodeTest1(int argc, char * argv[] )
{
  itk::itkFactoryRegistration();

  bool res = true;
  const char *filename = 0;
  if (argc > 1)
    {
    filename = argv[1];
    }
  res = TestBSplineTransform(filename) && res;
  res = TestGridTransform(filename) && res;
  res = TestThinPlateSplineTransform(filename) && res;
  res = TestCompositeTransformHardenSplit(filename) && res;
  res = TestBSplineLinearCompositeTransformSplit(filename) && res;
  res = TestRelativeTransforms(filename) && res;

  if (res)
    {
    std::cout << "Success" << std::endl;
    }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
bool transformPoints(vtkAbstractTransform *transform, vtkPoints* sourcePoints, vtkPoints* transformedPoints)
{
  if (transform == 0)
    {
    std::cout << __LINE__ << ": transformPoints failed: transform is invalid" << std::endl;
    return false;
    }
  transform->TransformPoints(sourcePoints, transformedPoints);
  return true;
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
bool testTransformConsistency(vtkAbstractTransform *xfp, vtkAbstractTransform *xtp, vtkPoints* testPoints, vtkPoints* transformedPoints, vtkPoints* transformedPointsBackToTest)
{
  // Test if transform actually changes point positions
  if (!transformPoints(xfp, testPoints, transformedPoints))
    {
    std::cout << __LINE__ << ": testTransformConsistency failed" << std::endl;
    return false;
    }
  if (isSamePointPositions(testPoints, transformedPoints))
    {
    std::cout << __LINE__ << ": testTransformConsistency failed" << std::endl;
    return false;
    }

  // Test if the inverse transform moves back the point to its original position
  if (!transformPoints(xtp, transformedPoints, transformedPointsBackToTest))
    {
    std::cout << __LINE__ << ": testTransformConsistency failed" << std::endl;
    return false;
    }
  if (!isSamePointPositions(testPoints, transformedPointsBackToTest))
    {
    std::cout << __LINE__ << ": testTransformConsistency failed" << std::endl;
    return false;
    }

  return true;
}

// Checks if the transform accurate enough by comparing point positions to ground truth
//---------------------------------------------------------------------------
bool testTransformAccuracy(vtkAbstractTransform *xfp, vtkAbstractTransform *xtp, vtkPoints* testPoints, vtkPoints* transformedPointsGroundTruth, vtkPoints* transformedPointsBackToTestGroundTruth)
{
  // Test if the copied from parent transform gives the same results as the original
  vtkNew<vtkPoints> transformedPoints;
  if (!transformPoints(xfp, testPoints, transformedPoints.GetPointer()))
    {
    std::cout << __LINE__ << ": testTransformAccuracy failed" << std::endl;
    return false;
    }
  if (!isSamePointPositions(transformedPoints.GetPointer(), transformedPointsGroundTruth))
    {
    std::cout << __LINE__ << ": testTransformAccuracy failed" << std::endl;
    return false;
    }

  // Test if the copied to parent transform gives the same results as the original
  vtkNew<vtkPoints> transformedPointsBackToSource;
  if (!transformPoints(xtp, transformedPoints.GetPointer(), transformedPointsBackToSource.GetPointer()))
    {
    std::cout << __LINE__ << ": testTransformAccuracy failed" << std::endl;
    return false;
    }
  if (!isSamePointPositions(transformedPointsBackToSource.GetPointer(), transformedPointsBackToTestGroundTruth))
    {
    std::cout << __LINE__ << ": testTransformAccuracy failed" << std::endl;
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool testTransformConsistencyAndCopy(const char *filename, const char* nodeId, const char* expectedTransformType)
{
  // Read a BSpline transform from a scene
  vtkNew<vtkMRMLScene> scene;

  scene->SetURL(filename);
  scene->Import();

  vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(scene->GetNodeByID(nodeId));

  if (transformNode == 0)
    {
    std::cout << __LINE__ << ": testTransform failed (" <<expectedTransformType<< ")" << std::endl;
    return false;
    }

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

  if (!testTransformConsistency(xfp, xtp, testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()))
    {
    std::cout << __LINE__ << ": testTransformConsistency failed (" <<expectedTransformType<< ")" << std::endl;
    return false;
    }

  // Test if node copy creates an independent copy
  vtkNew<vtkMRMLTransformNode> transformNodeCopy;
  transformNodeCopy->Copy(transformNode);
  // Reset the original transform to make sure that it was not a shallow copy (transforms from the original transforms are not reused)
  vtkNew<vtkTransform> emptyTransform;
  transformNode->SetAndObserveTransformToParent(emptyTransform.GetPointer());

  vtkAbstractTransform *xfpCopy = transformNodeCopy->GetTransformFromParentAs(expectedTransformType);
  vtkAbstractTransform *xtpCopy = transformNodeCopy->GetTransformToParentAs(expectedTransformType);

  if (!testTransformAccuracy(xfpCopy, xtpCopy, testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()))
    {
    std::cout << __LINE__ << ": testTransformAccuracy failed (" <<expectedTransformType<< ")" << std::endl;
    return false;
    }

  // Cleanup
  scene->Clear(1);
  return true;
}


//---------------------------------------------------------------------------
bool TestBSplineTransform(const char *filename)
{
  if (!testTransformConsistencyAndCopy(filename, "vtkMRMLBSplineTransformNode1", "vtkOrientedBSplineTransform"))
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool TestGridTransform(const char *filename)
{
  if (!testTransformConsistencyAndCopy(filename, "vtkMRMLGridTransformNode1", "vtkOrientedGridTransform"))
    {
    std::cout << __LINE__ << ": TestGridTransform failed" << std::endl;
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool TestThinPlateSplineTransform(const char *filename)
{
  if (!testTransformConsistencyAndCopy(filename, "vtkMRMLTransformNode1", "vtkThinPlateSplineTransform"))
    {
    std::cout << __LINE__ << ": TestThinPlateSplineTransform failed" << std::endl;
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool TestCompositeTransformHardenSplit(const char *filename)
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
  if (!testTransformConsistency(transformFromWorldBeforeHardening.GetPointer(), transformToWorldBeforeHardening.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()))
    {
    std::cout << __LINE__ << ": TestCompositeTransform failed" << std::endl;
    return false;
    }

  // Harden transform
  vtkNew<vtkGeneralTransform> hardeningTransform;
  bsplineTransformNode->GetParentTransformNode()->GetTransformToWorld(hardeningTransform.GetPointer());
  hardeningTransform->Update();
  bsplineTransformNode->ApplyTransform(hardeningTransform.GetPointer());
  bsplineTransformNode->SetAndObserveTransformNodeID(NULL);

  // Test if transform to world is the same after hardening
  vtkNew<vtkGeneralTransform> transformToWorldAfterHardening;
  bsplineTransformNode->GetTransformToWorld(transformToWorldAfterHardening.GetPointer());
  std::cout << "Transform to world after hardening: " << std::endl;
  std::cout << infoPrinter->GetTransformInfo(transformToWorldAfterHardening.GetPointer()) << std::endl;
  vtkNew<vtkGeneralTransform> transformFromWorldAfterHardening;
  bsplineTransformNode->GetTransformFromWorld(transformFromWorldAfterHardening.GetPointer());
  if (!testTransformAccuracy(transformFromWorldAfterHardening.GetPointer(), transformToWorldAfterHardening.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()))
    {
    std::cout << __LINE__ << ": TestCompositeTransform failed" << std::endl;
    return false;
    }

  // Split trasnform
  if (!bsplineTransformNode->Split())
    {
    std::cout << __LINE__ << ": TestCompositeTransform split failed" << std::endl;
    return false;
    }

  // Test if transform to world is the same after splitting
  transformToWorldAfterHardening->Identity();
  bsplineTransformNode->GetTransformToWorld(transformToWorldAfterHardening.GetPointer());
  std::cout << "Transform to world after hardening: " << std::endl;
  std::cout << infoPrinter->GetTransformInfo(transformToWorldAfterHardening.GetPointer()) << std::endl;
  transformFromWorldAfterHardening->Identity();
  bsplineTransformNode->GetTransformFromWorld(transformFromWorldAfterHardening.GetPointer());
  if (!testTransformAccuracy(transformFromWorldAfterHardening.GetPointer(), transformToWorldAfterHardening.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()))
    {
    std::cout << __LINE__ << ": TestCompositeTransform failed" << std::endl;
    return false;
    }

  // Cleanup
  scene->Clear(1);
  return true;
}

//---------------------------------------------------------------------------
bool TestBSplineLinearCompositeTransformSplit(const char *filename)
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
  if (!testTransformConsistency(transformFromWorldBeforeSplit.GetPointer(), transformToWorldBeforeSplit.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()))
    {
    std::cout << __LINE__ << ": TestCompositeSplitTransform failed" << std::endl;
    return false;
    }

  // Split transform
  if (!compositeBsplineTransformNode->Split())
    {
    std::cout << __LINE__ << ": TestCompositeSplitTransform split failed" << std::endl;
    return false;
    }

  // Test if transform to world is the same after splitting
  vtkNew<vtkGeneralTransform> transformToWorldAfterSplit;
  compositeBsplineTransformNode->GetTransformToWorld(transformToWorldAfterSplit.GetPointer());
  vtkNew<vtkGeneralTransform> transformFromWorldAfterSplit;
  compositeBsplineTransformNode->GetTransformFromWorld(transformFromWorldAfterSplit.GetPointer());

  if (!testTransformAccuracy(transformFromWorldAfterSplit.GetPointer(), transformToWorldAfterSplit.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()))
    {
    std::cout << __LINE__ << ": TestCompositeSplitTransform failed" << std::endl;
    return false;
    }

  // Cleanup
  scene->Clear(1);
  return true;
}

//---------------------------------------------------------------------------
bool TestRelativeTransforms(const char *filename)
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

  // Test if relaive transform between two nodes is consistent (transform is not identity and invertible)
  vtkNew<vtkGeneralTransform> compositeToTps;
  compositeTransformNode->GetTransformToNode(tpsTransformNode, compositeToTps.GetPointer());
  std::cout << "Composite transform to tps: " << std::endl;
  std::cout << infoPrinter->GetTransformInfo(compositeToTps.GetPointer()) << std::endl;

  vtkNew<vtkGeneralTransform> tpsToComposite;
  tpsTransformNode->GetTransformToNode(compositeTransformNode, tpsToComposite.GetPointer());
  std::cout << "Tps transform to composite: " << std::endl;
  std::cout << infoPrinter->GetTransformInfo(tpsToComposite.GetPointer()) << std::endl;
  if (!testTransformConsistency(tpsToComposite.GetPointer(), compositeToTps.GetPointer(),
    testPoints, transformedPoints.GetPointer(), transformedPointsBackToTest.GetPointer()))
    {
    std::cout << __LINE__ << ": TestRelativeTransforms failed" << std::endl;
    return false;
    }

  // Check if the relative transform only goes through the common parent
  // (does not contain gridTransformNode)
  vtkNew<vtkCollection> compositeToTpsTransformList;
  vtkMRMLTransformNode::FlattenGeneralTransform(compositeToTpsTransformList.GetPointer(), compositeToTps.GetPointer());

  if (compositeToTpsTransformList->GetNumberOfItems()!=4)
    {
    std::cout << __LINE__ << ": TestRelativeTransforms failed" << std::endl;
    return false;
    }
  if (!vtkAbstractTransform::SafeDownCast(compositeToTpsTransformList->GetItemAsObject(0))->IsA("vtkLinearTransform"))
    {
    std::cout << __LINE__ << ": TestRelativeTransforms failed" << std::endl;
    return false;
    }
  if (!vtkAbstractTransform::SafeDownCast(compositeToTpsTransformList->GetItemAsObject(1))->IsA("vtkBSplineTransform"))
    {
    std::cout << __LINE__ << ": TestRelativeTransforms failed" << std::endl;
    return false;
    }
  if (!vtkAbstractTransform::SafeDownCast(compositeToTpsTransformList->GetItemAsObject(2))->IsA("vtkBSplineTransform"))
    {
    std::cout << __LINE__ << ": TestRelativeTransforms failed" << std::endl;
    return false;
    }
  if (!vtkAbstractTransform::SafeDownCast(compositeToTpsTransformList->GetItemAsObject(3))->IsA("vtkThinPlateSplineTransform"))
    {
    std::cout << __LINE__ << ": TestRelativeTransforms failed" << std::endl;
    return false;
    }

  // Cleanup
  scene->Clear(1);
  return true;
}
