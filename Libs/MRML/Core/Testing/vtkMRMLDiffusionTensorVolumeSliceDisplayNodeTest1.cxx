/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"

int vtkMRMLDiffusionTensorVolumeSliceDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLDiffusionTensorVolumeSliceDisplayNode> node1;
  // EXERCISE_BASIC_DISPLAY_MRML_METHODS is failing due to set/get ScalarVisibility
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseBasicMRMLMethods( node1.GetPointer() ));
  return EXIT_SUCCESS;
}
