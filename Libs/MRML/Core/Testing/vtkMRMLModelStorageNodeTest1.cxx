/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLModelStorageNode.h"
#include "vtkURIHandler.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLModelStorageNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLModelStorageNode > node1 = vtkSmartPointer< vtkMRMLModelStorageNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_STORAGE_MRML_METHODS(vtkMRMLModelStorageNode, node1);

  return EXIT_SUCCESS;
}
