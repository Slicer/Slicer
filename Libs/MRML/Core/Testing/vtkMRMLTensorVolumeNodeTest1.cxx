/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLTensorVolumeNode.h"

#include <vtkPolyData.h>


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLTensorVolumeNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLTensorVolumeNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLTensorVolumeNode, node1.GetPointer());

  return EXIT_SUCCESS;
}
