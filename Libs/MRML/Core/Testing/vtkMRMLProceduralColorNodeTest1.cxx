/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLProceduralColorNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLProceduralColorNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLProceduralColorNode > node1 = vtkSmartPointer< vtkMRMLProceduralColorNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  // missing storage node defs, for now test the transformable methods
  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLProceduralColorNode, node1);

  return EXIT_SUCCESS;
}
