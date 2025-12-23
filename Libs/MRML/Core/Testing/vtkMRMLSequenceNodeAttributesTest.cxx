/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLLinearTransformSequenceStorageNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLTransformSequenceStorageNode.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtksys/SystemTools.hxx>
#include <vtkOrientedGridTransform.h>
#include "vtkMRMLGridTransformNode.h"

// STD includes
#include <iostream>

/// Test that node attributes of volume nodes and transform nodes in sequences can be stored
/// in volume sequence and transform sequence files.

namespace
{
std::string tempFilename(std::string tempDir, std::string suffix, std::string fileExtension, bool remove = false)
{
  std::string filename = tempDir + "/vtkMRMLSequenceNodeAttributeTest_" + suffix + "." + fileExtension;
  // remove file if exists
  if (remove && vtksys::SystemTools::FileExists(filename.c_str(), true))
  {
    vtksys::SystemTools::RemoveFile(filename.c_str());
  }
  return filename;
}
} // namespace

//---------------------------------------------------------------------------
int TestVolumeSequenceNodeAttributes(const std::string& tempDir)
{
  std::cout << "Testing volume sequence node attributes..." << std::endl;

  vtkNew<vtkMRMLScene> scene;

  // Create a volume sequence with attributes
  vtkNew<vtkMRMLSequenceNode> volumeSequenceNode;
  scene->AddNode(volumeSequenceNode);
  volumeSequenceNode->SetIndexName("time");
  volumeSequenceNode->SetIndexUnit("s");

  const int numberOfFrames = 25;
  const int missingMetadataFrameIndex = 11;

  // Create some test volumes with attributes
  for (int i = 0; i < numberOfFrames; ++i)
  {
    vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
    vtkNew<vtkImageData> imageData;
    imageData->SetDimensions(10, 10, 10);
    imageData->AllocateScalars(VTK_SHORT, 1);
    imageData->GetPointData()->GetScalars()->Fill(i * 100);
    volumeNode->SetAndObserveImageData(imageData);

    if (i != missingMetadataFrameIndex)
    {
      // Add some test attributes
      std::string testAttribute1 = "TestValue" + std::to_string(i);
      std::string testAttribute2 = "AnotherTestValue" + std::to_string(i * 2);
      volumeNode->SetAttribute("TestAttribute1", testAttribute1.c_str());
      volumeNode->SetAttribute("TestAttribute2", testAttribute2.c_str());
    }

    std::string indexValue = std::to_string(i * 0.5);
    volumeSequenceNode->SetDataNodeAtValue(volumeNode, indexValue);
  }

  // Save the sequence
  vtkNew<vtkMRMLVolumeSequenceStorageNode> storageNode;
  scene->AddNode(storageNode);
  std::string filename = tempFilename(tempDir, "volume_attributes", "seq.nrrd", true);
  storageNode->SetFileName(filename.c_str());
  CHECK_BOOL(storageNode->WriteData(volumeSequenceNode), true);

  // Read the sequence back
  vtkNew<vtkMRMLSequenceNode> loadedSequenceNode;
  scene->AddNode(loadedSequenceNode);

  vtkNew<vtkMRMLVolumeSequenceStorageNode> loadStorageNode;
  scene->AddNode(loadStorageNode);
  loadStorageNode->SetFileName(filename.c_str());
  CHECK_BOOL(loadStorageNode->ReadData(loadedSequenceNode), true);

  // Verify attributes are preserved
  CHECK_INT(loadedSequenceNode->GetNumberOfDataNodes(), numberOfFrames);

  for (int i = 0; i < numberOfFrames; ++i)
  {
    vtkMRMLVolumeNode* loadedVolumeNode = vtkMRMLVolumeNode::SafeDownCast(loadedSequenceNode->GetNthDataNode(i));
    CHECK_NOT_NULL(loadedVolumeNode);

    std::string expectedAttribute1 = "TestValue" + std::to_string(i);
    std::string expectedAttribute2 = "AnotherTestValue" + std::to_string(i * 2);

    const char* actualAttribute1 = loadedVolumeNode->GetAttribute("TestAttribute1");
    const char* actualAttribute2 = loadedVolumeNode->GetAttribute("TestAttribute2");

    if (i != missingMetadataFrameIndex)
    {
      CHECK_NOT_NULL(actualAttribute1);
      CHECK_NOT_NULL(actualAttribute2);
      CHECK_STRING(actualAttribute1, expectedAttribute1.c_str());
      CHECK_STRING(actualAttribute2, expectedAttribute2.c_str());
    }
    else
    {
      CHECK_NULL(actualAttribute1);
      CHECK_NULL(actualAttribute2);
    }
  }

  // Clean up
  vtksys::SystemTools::RemoveFile(filename.c_str());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGridTransformSequenceNodeAttributes(const std::string& tempDir)
{
  std::cout << "Testing grid transform sequence node attributes..." << std::endl;

  vtkNew<vtkMRMLScene> scene;

  // Create a transform sequence with attributes
  vtkNew<vtkMRMLSequenceNode> transformSequenceNode;
  scene->AddNode(transformSequenceNode);
  transformSequenceNode->SetIndexName("time");
  transformSequenceNode->SetIndexUnit("s");

  const int numberOfFrames = 22;
  const int missingMetadataFrameIndex = 15;

  // Create some test grid transforms with attributes
  for (int i = 0; i < numberOfFrames; ++i)
  {
    vtkNew<vtkMRMLTransformNode> transformNode;

    // Create a simple displacement field
    vtkNew<vtkImageData> displacementField;
    displacementField->SetDimensions(10, 10, 10);
    displacementField->SetOrigin(0, 0, 0);
    displacementField->SetSpacing(1, 1, 1);
    displacementField->AllocateScalars(VTK_DOUBLE, 3);
    displacementField->GetPointData()->GetScalars()->Fill(0.0); // Identity transform

    vtkNew<vtkOrientedGridTransform> gridTransform;
    gridTransform->SetDisplacementGridData(displacementField);

    transformNode->SetAndObserveTransformFromParent(gridTransform);

    if (i != missingMetadataFrameIndex)
    {

      // Add some test attributes
      std::string testAttribute1 = "GridTransformTestValue" + std::to_string(i);
      std::string testAttribute2 = "AnotherGridTransformTestValue" + std::to_string(i * 3);
      transformNode->SetAttribute("GridTransformTestAttribute1", testAttribute1.c_str());
      transformNode->SetAttribute("GridTransformTestAttribute2", testAttribute2.c_str());
    }

    std::string indexValue = std::to_string(i * 0.5);
    transformSequenceNode->SetDataNodeAtValue(transformNode, indexValue);
  }

  // Save the sequence
  vtkNew<vtkMRMLTransformSequenceStorageNode> storageNode;
  scene->AddNode(storageNode);
  std::string filename = tempFilename(tempDir, "grid_transform_attributes", "seq.nrrd", true);
  storageNode->SetFileName(filename.c_str());
  CHECK_BOOL(storageNode->WriteData(transformSequenceNode), true);

  // Read the sequence back
  vtkNew<vtkMRMLSequenceNode> loadedSequenceNode;
  scene->AddNode(loadedSequenceNode);

  vtkNew<vtkMRMLTransformSequenceStorageNode> loadStorageNode;
  scene->AddNode(loadStorageNode);
  loadStorageNode->SetFileName(filename.c_str());
  CHECK_BOOL(loadStorageNode->ReadData(loadedSequenceNode), true);

  // Verify attributes are preserved
  CHECK_INT(loadedSequenceNode->GetNumberOfDataNodes(), numberOfFrames);

  for (int i = 0; i < numberOfFrames; ++i)
  {
    vtkMRMLTransformNode* loadedTransformNode = vtkMRMLTransformNode::SafeDownCast(loadedSequenceNode->GetNthDataNode(i));
    CHECK_NOT_NULL(loadedTransformNode);

    std::string expectedAttribute1 = "GridTransformTestValue" + std::to_string(i);
    std::string expectedAttribute2 = "AnotherGridTransformTestValue" + std::to_string(i * 3);

    const char* actualAttribute1 = loadedTransformNode->GetAttribute("GridTransformTestAttribute1");
    const char* actualAttribute2 = loadedTransformNode->GetAttribute("GridTransformTestAttribute2");

    if (i != missingMetadataFrameIndex)
    {
      CHECK_NOT_NULL(actualAttribute1);
      CHECK_NOT_NULL(actualAttribute2);
      CHECK_STRING(actualAttribute1, expectedAttribute1.c_str());
      CHECK_STRING(actualAttribute2, expectedAttribute2.c_str());
    }
    else
    {
      CHECK_NULL(actualAttribute1);
      CHECK_NULL(actualAttribute2);
    }
  }

  // Clean up
  vtksys::SystemTools::RemoveFile(filename.c_str());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int vtkMRMLSequenceNodeAttributesTest(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " tempDir" << std::endl;
    return EXIT_FAILURE;
  }

  std::string tempDir = argv[1];

  CHECK_EXIT_SUCCESS(TestVolumeSequenceNodeAttributes(tempDir));
  CHECK_EXIT_SUCCESS(TestGridTransformSequenceNodeAttributes(tempDir));

  std::cout << "All tests passed!" << std::endl;
  return EXIT_SUCCESS;
}
