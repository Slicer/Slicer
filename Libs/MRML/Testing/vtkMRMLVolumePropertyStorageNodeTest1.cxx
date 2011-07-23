/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLVolumePropertyStorageNode.h"
#include "vtkURIHandler.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLVolumePropertyStorageNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLVolumePropertyStorageNode > node1 = vtkSmartPointer< vtkMRMLVolumePropertyStorageNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_STORAGE_MRML_METHODS( vtkMRMLVolumePropertyStorageNode, node1);

  return EXIT_SUCCESS;
}
