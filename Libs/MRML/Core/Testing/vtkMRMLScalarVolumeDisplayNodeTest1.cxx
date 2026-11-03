/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

#include <vtkImageData.h>

#include <algorithm>

int vtkMRMLScalarVolumeDisplayNodeTest1(int, char*[])
{
  vtkNew<vtkMRMLScalarVolumeDisplayNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(10000, 1, 1);
  imageData->AllocateScalars(VTK_SHORT, 1);
  short* voxels = static_cast<short*>(imageData->GetScalarPointer());
  std::fill(voxels, voxels + 10000, 0);
  voxels[0] = -1024;
  voxels[9999] = 1500;
  imageData->Modified();

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
  scene->AddNode(volumeNode.GetPointer());
  scene->AddNode(node1.GetPointer());
  volumeNode->SetAndObserveDisplayNodeID(node1->GetID());
  volumeNode->SetAndObserveImageData(imageData.GetPointer());

  node1->SetApplyThreshold(1);
  node1->SetAutoThreshold(1);
  CHECK_DOUBLE(node1->GetLowerThreshold(), -1024);
  CHECK_DOUBLE(node1->GetUpperThreshold(), 1500);
  CHECK_BOOL(node1->GetWindowLevelMin() > -1024, true);
  CHECK_BOOL(node1->GetWindowLevelMax() < 1500, true);
  return EXIT_SUCCESS;
}
