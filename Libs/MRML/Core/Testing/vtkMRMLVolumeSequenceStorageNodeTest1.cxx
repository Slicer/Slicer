/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

#include <vtkImageData.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTransform.h>

namespace
{
std::string tempFilename(std::string tempDir, std::string suffix, std::string fileExtension, bool remove = false)
{
  std::string filename = tempDir + "/vtkMRMLVolumeSequenceStorageNodeTest1_" + suffix + "." + fileExtension;
  // remove file if exists
  if (remove && vtksys::SystemTools::FileExists(filename.c_str(), true))
  {
    vtksys::SystemTools::RemoveFile(filename.c_str());
  }
  return filename;
}
} // namespace

//---------------------------------------------------------------------------
int TestVolumeSequenceStorage(const std::string& inputFileName,
                              int dimC,
                              int dimR,
                              int dimA,
                              int dimS,
                              int dimT,
                              bool readable,
                              double coordC,
                              double coordR,
                              double coordA,
                              double coordS,
                              int coordT,
                              double expectedVoxelValue,
                              const std::string tempDir)

{
  // Test if a volume sequence can be read and written.
  std::cout << "TestVolumeSequenceStorage: " << inputFileName << std::endl;

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLVolumeSequenceStorageNode> storageNode;
  scene->AddNode(storageNode);

  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  scene->AddNode(sequenceNode);

  storageNode->SetFileName(inputFileName.c_str());
  CHECK_BOOL(storageNode->ReadData(sequenceNode), readable);

  if (!readable)
  {
    // Nothing more to test
    return EXIT_SUCCESS;
  }

  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(coordT));
  CHECK_NOT_NULL(volumeNode);

  CHECK_INT(volumeNode->GetImageData()->GetNumberOfScalarComponents(), dimC);
  CHECK_INT(volumeNode->GetImageData()->GetDimensions()[0], dimR);
  CHECK_INT(volumeNode->GetImageData()->GetDimensions()[1], dimA);
  CHECK_INT(volumeNode->GetImageData()->GetDimensions()[2], dimS);
  CHECK_INT(sequenceNode->GetNumberOfDataNodes(), dimT);

  double voxelValue = volumeNode->GetImageData()->GetScalarComponentAsDouble(coordR, coordA, coordS, coordC);
  if (fabs(expectedVoxelValue - voxelValue) > 1e-3)
  {
    std::cerr << "Voxel value mismatch at (C,R,A,S,T)=(" << coordC << "," << coordR << "," << coordA << "," << coordS << "," << coordT << "): "
              << " expected " << expectedVoxelValue << ", but got " << voxelValue << std::endl;
    return EXIT_FAILURE;
  }

  // Test writing
  storageNode->SetFileName(tempFilename(tempDir, "written", "seq.nrrd", true).c_str());
  CHECK_BOOL(storageNode->WriteData(sequenceNode), true);

  // Test reading of written data
  vtkNew<vtkMRMLSequenceNode> sequenceNodeRead;
  CHECK_BOOL(storageNode->ReadData(sequenceNodeRead), true);

  vtkMRMLVolumeNode* volumeNodeRead = vtkMRMLVolumeNode::SafeDownCast(sequenceNodeRead->GetNthDataNode(coordT));
  if (!volumeNodeRead)
  {
    std::cerr << "Failed to get volume node at index " << coordT << std::endl;
    return EXIT_FAILURE;
  }

  CHECK_INT(volumeNodeRead->GetImageData()->GetNumberOfScalarComponents(), dimC);
  CHECK_INT(volumeNodeRead->GetImageData()->GetDimensions()[0], dimR);
  CHECK_INT(volumeNodeRead->GetImageData()->GetDimensions()[1], dimA);
  CHECK_INT(volumeNodeRead->GetImageData()->GetDimensions()[2], dimS);
  CHECK_INT(sequenceNodeRead->GetNumberOfDataNodes(), dimT);

  double voxelValueRead = volumeNodeRead->GetImageData()->GetScalarComponentAsDouble(coordR, coordA, coordS, coordC);
  if (fabs(expectedVoxelValue - voxelValueRead) > 1e-3)
  {
    std::cerr << "Voxel value mismatch at (C,R,A,S,T)=(" << coordC << "," << coordR << "," << coordA << "," << coordS << "," << coordT << "): "
              << " expected " << expectedVoxelValue << ", but got " << voxelValue << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int vtkMRMLVolumeSequenceStorageNodeTest1(int argc, char* argv[])
{
  if (argc != 10)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }

  const char* tempDir = argv[1];

  // TestVolumeSequenceStorage: seq.nrrd file, dimensionC, dimensionR, dimensionA, dimensionS, dimensionT,
  // readable, coordC, coordR, coordA, coordS, coordT, value, tempDir

  // Standard
  const char* volume_DomainList = argv[2];
  const char* volume_ColorDomainList = argv[3];
  CHECK_EXIT_SUCCESS(TestVolumeSequenceStorage(volume_DomainList, 1, 32, 27, 15, 10, true, 0, 10, 20, 8, 5, 454, tempDir));
  CHECK_EXIT_SUCCESS(TestVolumeSequenceStorage(volume_ColorDomainList, 4, 16, 15, 1, 8, true, 1, 10, 7, 0, 5, 238, tempDir));

  // Non-standard
  const char* volume_ListDomain = argv[4];
  const char* volume_VectorDomain = argv[5];
  const char* volume_CharVectorDomain = argv[6];
  const char* volume_DomainListColor = argv[7];
  CHECK_EXIT_SUCCESS(TestVolumeSequenceStorage(volume_ListDomain, 1, 32, 27, 15, 10, true, 0, 16, 13, 8, 5, 516, tempDir));
  CHECK_EXIT_SUCCESS(TestVolumeSequenceStorage(volume_VectorDomain, 1, 32, 27, 15, 10, true, 0, 16, 13, 8, 5, 516, tempDir));
  CHECK_EXIT_SUCCESS(TestVolumeSequenceStorage(volume_CharVectorDomain, 1, 32, 27, 15, 10, true, 0, 16, 13, 8, 5, 187, tempDir));
  CHECK_EXIT_SUCCESS(TestVolumeSequenceStorage(volume_DomainListColor, 4, 16, 15, 1, 8, true, 2, 8, 9, 0, 3, 241, tempDir));

  // Transform
  const char* transform_VectorDomainList = argv[8];
  CHECK_EXIT_SUCCESS(TestVolumeSequenceStorage(transform_VectorDomainList, 3, 21, 21, 13, 26, true, 2, 16, 13, 8, 5, -0.480709, tempDir));

  // Unsupported
  const char* volume_ListDomainListColor = argv[9];
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestVolumeSequenceStorage(volume_ListDomainListColor, 0, 0, 0, 0, 0, false, 0, 0, 0, 0, 0, 1, tempDir));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  std::cout << "-----------------------------------------------------" << std::endl;

  vtkNew<vtkMRMLVolumeSequenceStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  std::cout << "\nTest passed." << std::endl;
  return EXIT_SUCCESS;
}
