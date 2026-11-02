/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLScene.h"

int vtkMRMLVectorVolumeNodeTest1(int, char*[])
{
  vtkNew<vtkMRMLVectorVolumeNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  // Named as a vector volume from the start, not only after it is written (node selectors show
  // the name of the registered node, which is never written)
  CHECK_STD_STRING(node1->GetTypeDisplayName(), "Vector Volume");
  CHECK_STD_STRING(scene->GetTypeDisplayNameByClassName("vtkMRMLVectorVolumeNode"), "Vector Volume");
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}
