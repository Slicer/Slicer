/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLROIListNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLROIListNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLROIListNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLROIListNode, node1.GetPointer());

  return EXIT_SUCCESS;
}
