/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLFiducialListStorageNode.h"
#include "vtkURIHandler.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLFiducialListStorageNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLFiducialListStorageNode > node1 = vtkSmartPointer< vtkMRMLFiducialListStorageNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_STORAGE_MRML_METHODS(vtkMRMLFiducialListStorageNode, node1);

  return EXIT_SUCCESS;
}
