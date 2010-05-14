/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLFreeSurferProceduralColorNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLFreeSurferProceduralColorNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLFreeSurferProceduralColorNode > node1 = vtkSmartPointer< vtkMRMLFreeSurferProceduralColorNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLFreeSurferProceduralColorNode, node1);

  return EXIT_SUCCESS;
}
