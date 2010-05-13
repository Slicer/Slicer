/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLFreeSurferModelOverlayStorageNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLFreeSurferModelOverlayStorageNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLFreeSurferModelOverlayStorageNode > node1 = vtkSmartPointer< vtkMRMLFreeSurferModelOverlayStorageNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_STORAGE_MRML_METHODS(vtkMRMLFreeSurferModelOverlayStorageNode, node1);

  return EXIT_SUCCESS;
}
