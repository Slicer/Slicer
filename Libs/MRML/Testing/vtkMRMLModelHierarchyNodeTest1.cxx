/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLModelHierarchyNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLModelHierarchyNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLModelHierarchyNode > node1 = vtkSmartPointer< vtkMRMLModelHierarchyNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLModelHierarchyNode, node1);

  return EXIT_SUCCESS;
}
