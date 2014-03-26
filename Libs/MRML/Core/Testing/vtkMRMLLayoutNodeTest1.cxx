/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLLayoutNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLLayoutNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLLayoutNode > node1 = vtkSmartPointer< vtkMRMLLayoutNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLLayoutNode, node1);

  return EXIT_SUCCESS;
}
