/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLFiberBundleTubeDisplayNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLFiberBundleTubeDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLFiberBundleTubeDisplayNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());
  // exercising the display mrml methods fails on the ScalarVisibility boolean test
  EXERCISE_BASIC_MRML_METHODS(vtkMRMLFiberBundleTubeDisplayNode, node1.GetPointer());


  return EXIT_SUCCESS;
}
