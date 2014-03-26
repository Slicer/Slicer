/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLAnnotationDisplayNode.h"

int vtkMRMLAnnotationDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLAnnotationDisplayNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS( node1.GetPointer() );

  return EXIT_SUCCESS;
}
