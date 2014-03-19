/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// ITK includes
#include <itkConfigure.h>
#if ITK_VERSION_MAJOR > 3
#include <itkFactoryRegistration.h>
#endif

#include "vtkITKBSplineTransform.h"
#include "vtkGridTransform.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLScene.h"

#include <vtkGeneralTransform.h>

#include "vtkMRMLCoreTestingMacros.h"

bool TestBSplineTransform(const char *filename);
bool TestGridTransform(const char *filename);
bool TestCompositeTransform(const char *filename);

int vtkMRMLNonlinearTransformNodeTest1(int argc, char * argv[] )
{
#if ITK_VERSION_MAJOR > 3
  itk::itkFactoryRegistration();
#endif

  bool res = true;
  const char *filename = 0;
  if (argc > 1)
    {
    filename = argv[1];
    }
  res = TestBSplineTransform(filename) && res;
  res = TestGridTransform(filename) && res;
  res = TestCompositeTransform(filename) && res;

  if (res)
    {
    std::cout << "Success" << std::endl;
    }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
bool TestBSplineTransform(const char *filename)
{
  // Read a BSpline transform from a scene
  vtkMRMLScene *scene = vtkMRMLScene::New();

  scene->SetURL(filename);
  scene->Import();

  vtkMRMLBSplineTransformNode *bsplineTransformNode = vtkMRMLBSplineTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLBSplineTransformNode1"));

  if (bsplineTransformNode == 0)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }

  // Test if both the transform and its inverse are available
  vtkITKBSplineTransform *xfp = vtkITKBSplineTransform::SafeDownCast(
    bsplineTransformNode->GetTransformFromParentAs("vtkITKBSplineTransform"));
  vtkITKBSplineTransform *xtp = vtkITKBSplineTransform::SafeDownCast(
    bsplineTransformNode->GetTransformToParentAs("vtkITKBSplineTransform"));

  if (xfp == 0 || xtp == 0)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }

  // Test if the transform actually changes point positions
  double inp[3] = {0,0,0};
  double outp[3] = {0,0,0};
  xfp->TransformPoint(inp, outp);
  if (fabs(outp[0]-inp[0]) < 0.1 || fabs(outp[1]-inp[1]) < 0.1 || fabs(outp[2]-inp[2]) < 0.1)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }

  // Test if the inverse transform moves back the point to its original position
  double outpInv[3] = {-100, -100, -100};
  xtp->TransformPoint(outp, outpInv);
  if (fabs(outpInv[0]-inp[0]) > 0.5 || fabs(outpInv[1]-inp[1]) > 0.5 || fabs(outpInv[2]-inp[2]) > 0.5)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }

  // Test if node copy creates an independent copy
  vtkNew<vtkMRMLBSplineTransformNode> bsplineTransformNodeCopy;
  bsplineTransformNodeCopy->Copy(bsplineTransformNode);
  // Reset the original transform to make sure that it was not a shallow copy (transforms from the original transforms are not reused)
  vtkITKBSplineTransform* emptyTransform=vtkITKBSplineTransform::New();
  bsplineTransformNode->SetAndObserveTransformToParent(emptyTransform);
  emptyTransform->Delete();

  vtkITKBSplineTransform *xfpCopy = vtkITKBSplineTransform::SafeDownCast(bsplineTransformNodeCopy->GetTransformFromParentAs("vtkITKBSplineTransform"));
  vtkITKBSplineTransform *xtpCopy = vtkITKBSplineTransform::SafeDownCast(bsplineTransformNodeCopy->GetTransformToParentAs("vtkITKBSplineTransform"));

  // Test if the copied transform gives the same results as the original
  double outpCopy[3] = {0,0,0};
  xfpCopy->TransformPoint(inp, outpCopy);
  if (fabs(outpCopy[0]-outp[0]) > 0.1 || fabs(outpCopy[1]-outp[1]) > 0.1 || fabs(outpCopy[2]-outp[2]) > 0.1)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }

  // Test if the inverse transform moves back the point to its original position
  double outpInvCopy[3] = {-100, -100, -100};
  xtpCopy->TransformPoint(outpCopy, outpInvCopy);
  if (fabs(outpInvCopy[0]-outpInv[0]) > 0.1 || fabs(outpInvCopy[1]-outpInv[1]) > 0.1 || fabs(outpInvCopy[2]-outpInv[2]) > 0.1)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }


  // Cleanup
  scene->Clear(1);
  scene->Delete();
  return true;
}

//---------------------------------------------------------------------------
bool TestGridTransform(const char *filename)
{
  vtkMRMLScene *scene = vtkMRMLScene::New();

  scene->SetURL(filename);
  scene->Import();

  vtkMRMLGridTransformNode *gridTransformNode = vtkMRMLGridTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLGridTransformNode1"));

  if (gridTransformNode == 0)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }

  vtkGridTransform *xfp = vtkGridTransform::SafeDownCast(
    gridTransformNode->GetTransformFromParentAs("vtkGridTransform"));
  vtkGridTransform *xtp = vtkGridTransform::SafeDownCast(
    gridTransformNode->GetTransformToParentAs("vtkGridTransform"));

  if (xfp == 0 || xtp == 0)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }

  double inp[] = {0,0,0};
  double outp[3];
  xfp->TransformPoint(inp, outp);
  if (fabs(outp[0]) < 0.1 || fabs(outp[1]) < 0.1 || fabs(outp[2]) < 0.1)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }

  xtp->TransformPoint(outp, inp);
  if (fabs(inp[0]) > 0.1 || fabs(inp[1]) > 0.1 || fabs(inp[2]) > 0.1)
    {
    std::cout << __LINE__ << ": TestBSplineTransform failed" << std::endl;
    return false;
    }

  scene->Clear(1);
  scene->Delete();
  return true;
}

//---------------------------------------------------------------------------
bool TestCompositeTransform(const char *filename)
{
  // Read a BSpline transform from a scene
  vtkMRMLScene *scene = vtkMRMLScene::New();

  scene->SetURL(filename);
  scene->Import();

  // Create a composite transform from bspline and a grid transform

  vtkMRMLBSplineTransformNode *bsplineTransformNode = vtkMRMLBSplineTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLBSplineTransformNode1"));

  vtkMRMLGridTransformNode *gridTransformNode = vtkMRMLGridTransformNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLGridTransformNode1"));

  bsplineTransformNode->SetAndObserveTransformNodeID(gridTransformNode->GetID());

  vtkNew<vtkGeneralTransform> transformToWorldBeforeHardening;
  bsplineTransformNode->GetTransformToWorld(transformToWorldBeforeHardening.GetPointer());
  vtkNew<vtkGeneralTransform> transformFromWorldBeforeHardening;
  bsplineTransformNode->GetTransformFromWorld(transformFromWorldBeforeHardening.GetPointer());

  // Test if the transform actually changes point positions
  double inp[3] = {0,0,0};
  double outp[3] = {0,0,0};
  transformToWorldBeforeHardening->TransformPoint(inp, outp);
  if (fabs(outp[0]-inp[0]) < 0.1 || fabs(outp[1]-inp[1]) < 0.1 || fabs(outp[2]-inp[2]) < 0.1)
    {
    std::cout << __LINE__ << ": TestCompositeTransform failed" << std::endl;
    return false;
    }

  // Test if the inverse transform moves back the point to its original position
  double outpInv[3] = {-100, -100, -100};
  transformFromWorldBeforeHardening->TransformPoint(outp, outpInv);
  if (fabs(outpInv[0]-inp[0]) > 0.3 || fabs(outpInv[1]-inp[1]) > 0.3 || fabs(outpInv[2]-inp[2]) > 0.3)
    {
    std::cout << __LINE__ << ": TestCompositeTransform failed" << std::endl;
    return false;
    }

  // Test if transform to world is the same after hardening

  // Harden transform
  vtkNew<vtkGeneralTransform> hardeningTransform;
  bsplineTransformNode->GetParentTransformNode()->GetTransformToWorld(hardeningTransform.GetPointer());
  hardeningTransform->Update();

  bsplineTransformNode->ApplyTransform(hardeningTransform.GetPointer());

  bsplineTransformNode->SetAndObserveTransformNodeID(NULL);

  // Get transforms from hardened transform
  vtkNew<vtkGeneralTransform> transformToWorldAfterHardening;
  bsplineTransformNode->GetTransformToWorld(transformToWorldAfterHardening.GetPointer());
  vtkNew<vtkGeneralTransform> transformFromWorldAfterHardening;
  bsplineTransformNode->GetTransformFromWorld(transformFromWorldAfterHardening.GetPointer());

  // Test if the copied transform gives the same results as the original
  double outpCopy[3] = {0,0,0};
  transformToWorldAfterHardening->TransformPoint(inp, outpCopy);
  if (fabs(outpCopy[0]-outp[0]) > 0.1 || fabs(outpCopy[1]-outp[1]) > 0.1 || fabs(outpCopy[2]-outp[2]) > 0.1)
    {
    std::cout << __LINE__ << ": TestCompositeTransform failed" << std::endl;
    return false;
    }

  // Test if the inverse transform moves back the point to its original position
  double outpInvCopy[3] = {-100, -100, -100};
  transformFromWorldAfterHardening->TransformPoint(outpCopy, outpInvCopy);
  if (fabs(outpInvCopy[0]-outpInv[0]) > 0.1 || fabs(outpInvCopy[1]-outpInv[1]) > 0.1 || fabs(outpInvCopy[2]-outpInv[2]) > 0.1)
    {
    std::cout << __LINE__ << ": TestCompositeTransform failed" << std::endl;
    return false;
    }

  // Cleanup
  scene->Clear(1);
  scene->Delete();
  return true;
}
