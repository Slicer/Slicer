/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLNRRDStorageNode.h"
#include "vtkURIHandler.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLNRRDStorageNodeTest1(int , char * [])
{
  vtkNew<vtkMRMLNRRDStorageNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_STORAGE_MRML_METHODS(vtkMRMLNRRDStorageNode, node1.GetPointer());

  return EXIT_SUCCESS;
}
