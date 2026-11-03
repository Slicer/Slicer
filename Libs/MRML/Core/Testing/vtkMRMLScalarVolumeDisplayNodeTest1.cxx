/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

#include <vtkImageData.h>
#include <vtkTrivialProducer.h>

static int TestAutoThresholdRange()
{
  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(256, 256, 1);
  imageData->AllocateScalars(VTK_SHORT, 1);
  short* scalars = static_cast<short*>(imageData->GetScalarPointer());
  const vtkIdType numberOfVoxels = imageData->GetNumberOfPoints();
  // Keep the maximum rare enough to be excluded from auto window/level.
  for (vtkIdType index = 0; index < numberOfVoxels; ++index)
  {
    scalars[index] = static_cast<short>(-1024 + index % 2049);
  }
  scalars[numberOfVoxels - 1] = 1500;
  imageData->Modified();

  vtkNew<vtkTrivialProducer> producer;
  producer->SetOutput(imageData.GetPointer());

  vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
  displayNode->SetInputImageDataConnection(producer->GetOutputPort());
  const double windowMinimum = displayNode->GetWindowLevelMin();
  const double windowMaximum = displayNode->GetWindowLevelMax();
  displayNode->SetAutoThreshold(1);

  if (displayNode->GetLowerThreshold() != -1024 || displayNode->GetUpperThreshold() != 1500)
  {
    std::cerr << "Auto threshold must include the full scalar range, got [" << displayNode->GetLowerThreshold() << ", " << displayNode->GetUpperThreshold() << "]" << std::endl;
    return EXIT_FAILURE;
  }
  if (windowMaximum >= 1500 || displayNode->GetWindowLevelMin() != windowMinimum || displayNode->GetWindowLevelMax() != windowMaximum)
  {
    std::cerr << "Auto threshold changed the percentile-based window/level range" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkMRMLScalarVolumeDisplayNode> thresholdOnlyNode;
  thresholdOnlyNode->SetAutoWindowLevel(0);
  thresholdOnlyNode->SetInputImageDataConnection(producer->GetOutputPort());
  thresholdOnlyNode->SetAutoThreshold(1);
  if (thresholdOnlyNode->GetLowerThreshold() != -1024 || thresholdOnlyNode->GetUpperThreshold() != 1500)
  {
    std::cerr << "Auto threshold without auto window/level must include the full scalar range" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int vtkMRMLScalarVolumeDisplayNodeTest1(int, char*[])
{
  vtkNew<vtkMRMLScalarVolumeDisplayNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return TestAutoThresholdRange();
}
