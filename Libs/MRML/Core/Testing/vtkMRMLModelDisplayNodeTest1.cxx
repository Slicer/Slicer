/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLModelDisplayNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLModelDisplayNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLModelDisplayNode > node1 = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAY_MRML_METHODS(vtkMRMLModelDisplayNode, node1);

  return EXIT_SUCCESS;
}
