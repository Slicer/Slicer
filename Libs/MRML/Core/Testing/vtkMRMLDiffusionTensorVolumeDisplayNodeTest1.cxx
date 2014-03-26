/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLDiffusionTensorVolumeDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLDiffusionTensorVolumeDisplayNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_DISPLAY_MRML_METHODS(vtkMRMLDiffusionTensorVolumeDisplayNode, node1.GetPointer());

  return EXIT_SUCCESS;
}
