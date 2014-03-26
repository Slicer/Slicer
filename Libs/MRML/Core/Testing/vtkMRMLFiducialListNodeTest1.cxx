/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLFiducialListNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLFiducialListNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLFiducialListNode > node1 = vtkSmartPointer< vtkMRMLFiducialListNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  //EXERCISE_BASIC_MRML_METHODS(vtkMRMLFiducialListNode, node1);

  // EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLFiducialListNode, node1);

  EXERCISE_BASIC_STORABLE_MRML_METHODS(vtkMRMLFiducialListNode, node1);

  return EXIT_SUCCESS;
}
