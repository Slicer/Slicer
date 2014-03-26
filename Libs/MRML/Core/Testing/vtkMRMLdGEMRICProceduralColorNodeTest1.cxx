/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLdGEMRICProceduralColorNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLdGEMRICProceduralColorNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLdGEMRICProceduralColorNode > node1 = vtkSmartPointer< vtkMRMLdGEMRICProceduralColorNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  // for now just test the transformable methods, to do: fix the storage nodes and revise
  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLdGEMRICProceduralColorNode, node1);

  return EXIT_SUCCESS;
}
