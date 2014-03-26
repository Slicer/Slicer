/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLColorTableStorageNode.h"
#include "vtkURIHandler.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLColorTableStorageNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLColorTableStorageNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_STORAGE_MRML_METHODS(vtkMRMLColorTableStorageNode, node1.GetPointer());

  return EXIT_SUCCESS;
}
