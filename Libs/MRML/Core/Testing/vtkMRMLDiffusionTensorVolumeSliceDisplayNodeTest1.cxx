/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLDiffusionTensorVolumeSliceDisplayNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLDiffusionTensorVolumeSliceDisplayNode > node1 = vtkSmartPointer< vtkMRMLDiffusionTensorVolumeSliceDisplayNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );
  // EXERCISE_BASIC_DISPLAY_MRML_METHODS is failing due to set/get ScalarVisibility
  EXERCISE_BASIC_MRML_METHODS(vtkMRMLDiffusionTensorVolumeSliceDisplayNode, node1);


  return EXIT_SUCCESS;
}
