/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLViewNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLViewNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLViewNode > node1 = vtkSmartPointer< vtkMRMLViewNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLViewNode, node1);

  return EXIT_SUCCESS;
}
