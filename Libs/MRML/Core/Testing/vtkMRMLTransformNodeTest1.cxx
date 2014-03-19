/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLTransformNode.h"

#include <vtkGeneralTransform.h>
 
int vtkMRMLTransformNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLTransformNode> node1;
  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());
  EXERCISE_BASIC_TRANSFORM_MRML_METHODS(vtkMRMLTransformNode, node1.GetPointer());

  return EXIT_SUCCESS;
}
