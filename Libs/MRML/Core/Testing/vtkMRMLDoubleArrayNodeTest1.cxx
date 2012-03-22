/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLDoubleArrayNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLDoubleArrayNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLDoubleArrayNode > node1 = vtkSmartPointer< vtkMRMLDoubleArrayNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLDoubleArrayNode, node1);
  
  return EXIT_SUCCESS;
}
