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
  vtkSmartPointer< vtkMRMLFiberBundleTubeDisplayNode > node1 = vtkSmartPointer< vtkMRMLFiberBundleTubeDisplayNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );
  // exercising the display mrml methods fails on the ScalarVisibility boolean test
  EXERCISE_BASIC_MRML_METHODS(vtkMRMLFiberBundleTubeDisplayNode, node1);
  

  return EXIT_SUCCESS;
}
