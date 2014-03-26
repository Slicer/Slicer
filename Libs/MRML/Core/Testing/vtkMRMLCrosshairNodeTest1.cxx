/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCrosshairNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLCrosshairNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLCrosshairNode > node1 = vtkSmartPointer< vtkMRMLCrosshairNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLCrosshairNode, node1);

  return EXIT_SUCCESS;
}
