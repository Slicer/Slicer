/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLUnstructuredGridStorageNode.h"
#include "vtkURIHandler.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLUnstructuredGridStorageNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLUnstructuredGridStorageNode > node1 = vtkSmartPointer< vtkMRMLUnstructuredGridStorageNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_STORAGE_MRML_METHODS(vtkMRMLUnstructuredGridStorageNode, node1);

  return EXIT_SUCCESS;
}
