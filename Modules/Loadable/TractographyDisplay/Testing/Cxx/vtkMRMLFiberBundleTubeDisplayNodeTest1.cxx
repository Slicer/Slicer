/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"

int vtkMRMLFiberBundleTubeDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLFiberBundleTubeDisplayNode> node1;
  // exercising the display mrml methods fails on the ScalarVisibility boolean test
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseBasicMRMLMethods(node1.GetPointer()));
  return EXIT_SUCCESS;
}
