/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

#include "vtkITKImageSequenceReader.h"

#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTransform.h>
#include <fstream>
#include <iostream>

namespace
{
std::string tempFilename(std::string tempDir, std::string suffix, std::string fileExtension, bool remove = false)
{
  std::string filename = tempDir + "/vtkMRMLVolumeSequenceStorageNodeNrrdTest_" + suffix + "." + fileExtension;
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

//---------------------------------------------------------------------------
int TestLeftHandedNrrdVolumeSequence(const std::string& leftHandedSequenceFileName)
{
  // The test file (cardiac cine MRI) stores the image using left-handed IJK coordinate system.
  // By default, the IJK coordinate system of volume sequences is made right-handed when reading,
  // the same way as for scalar volumes.
  std::cout << "TestLeftHandedNrrdVolumeSequence" << std::endl;

  const int expectedNumberOfFrames = 15;
  const int expectedDimensions[3] = { 128, 128, 16 };
  const double geometryTolerance = 1e-6;
  const double voxelValueTolerance = 1e-6;

  vtkNew<vtkMRMLScene> scene;

  // Read with default settings: normalization is enabled
  vtkNew<vtkMRMLVolumeSequenceStorageNode> normalizedStorageNode;
  CHECK_BOOL(normalizedStorageNode->GetForceRightHandedIJKCoordinateSystem(), true);
  scene->AddNode(normalizedStorageNode);
  normalizedStorageNode->SetFileName(leftHandedSequenceFileName.c_str());
  vtkNew<vtkMRMLSequenceNode> normalizedSequenceNode;
  scene->AddNode(normalizedSequenceNode);
  CHECK_BOOL(normalizedStorageNode->ReadData(normalizedSequenceNode), true);
  CHECK_INT(normalizedSequenceNode->GetNumberOfDataNodes(), expectedNumberOfFrames);

  // Read without normalization
  vtkNew<vtkMRMLVolumeSequenceStorageNode> originalStorageNode;
  originalStorageNode->ForceRightHandedIJKCoordinateSystemOff();
  scene->AddNode(originalStorageNode);
  originalStorageNode->SetFileName(leftHandedSequenceFileName.c_str());
  vtkNew<vtkMRMLSequenceNode> originalSequenceNode;
  scene->AddNode(originalSequenceNode);
  CHECK_BOOL(originalStorageNode->ReadData(originalSequenceNode), true);
  CHECK_INT(originalSequenceNode->GetNumberOfDataNodes(), expectedNumberOfFrames);

  const int lastSliceIndex = expectedDimensions[2] - 1;
  const vtkIdType numberOfVoxelsInSlice = expectedDimensions[0] * expectedDimensions[1];
  for (int frameIndex = 0; frameIndex < expectedNumberOfFrames; ++frameIndex)
  {
    vtkMRMLScalarVolumeNode* originalVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(originalSequenceNode->GetNthDataNode(frameIndex));
    vtkMRMLScalarVolumeNode* normalizedVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(normalizedSequenceNode->GetNthDataNode(frameIndex));
    CHECK_NOT_NULL(originalVolumeNode);
    CHECK_NOT_NULL(normalizedVolumeNode);
    for (int axis = 0; axis < 3; ++axis)
    {
      CHECK_INT(originalVolumeNode->GetImageData()->GetDimensions()[axis], expectedDimensions[axis]);
      CHECK_INT(normalizedVolumeNode->GetImageData()->GetDimensions()[axis], expectedDimensions[axis]);
    }

    vtkNew<vtkMatrix4x4> originalIjkToRas;
    originalVolumeNode->GetIJKToRASMatrix(originalIjkToRas);
    vtkNew<vtkMatrix4x4> normalizedIjkToRas;
    normalizedVolumeNode->GetIJKToRASMatrix(normalizedIjkToRas);
    CHECK_BOOL(vtkMRMLVolumeNode::IsIJKCoordinateSystemRightHanded(originalIjkToRas), false);
    CHECK_BOOL(vtkMRMLVolumeNode::IsIJKCoordinateSystemRightHanded(normalizedIjkToRas), true);

    // Normalization flips the K axis: voxel (i, j, k) of the normalized volume is voxel (i, j, lastSliceIndex - k) of the original volume
    for (int row = 0; row < 3; ++row)
    {
      CHECK_DOUBLE_TOLERANCE(normalizedIjkToRas->GetElement(row, 0), originalIjkToRas->GetElement(row, 0), geometryTolerance);
      CHECK_DOUBLE_TOLERANCE(normalizedIjkToRas->GetElement(row, 1), originalIjkToRas->GetElement(row, 1), geometryTolerance);
      CHECK_DOUBLE_TOLERANCE(normalizedIjkToRas->GetElement(row, 2), -originalIjkToRas->GetElement(row, 2), geometryTolerance);
      CHECK_DOUBLE_TOLERANCE(
        normalizedIjkToRas->GetElement(row, 3), originalIjkToRas->GetElement(row, 3) + lastSliceIndex * originalIjkToRas->GetElement(row, 2), geometryTolerance);
    }
    vtkDataArray* originalVoxels = originalVolumeNode->GetImageData()->GetPointData()->GetScalars();
    vtkDataArray* normalizedVoxels = normalizedVolumeNode->GetImageData()->GetPointData()->GetScalars();
    for (int k = 0; k < expectedDimensions[2]; ++k)
    {
      for (vtkIdType voxelIndexInSlice = 0; voxelIndexInSlice < numberOfVoxelsInSlice; ++voxelIndexInSlice)
      {
        CHECK_DOUBLE_TOLERANCE(normalizedVoxels->GetComponent(k * numberOfVoxelsInSlice + voxelIndexInSlice, 0),
                               originalVoxels->GetComponent((lastSliceIndex - k) * numberOfVoxelsInSlice + voxelIndexInSlice, 0),
                               voxelValueTolerance);
      }
    }
  }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestNrrdVolumeSequenceWithoutListAxis(const std::string& tempDir)
{
  // 4D NRRD file where all axes are "domain" kind (there is no "list" axis).
  // Such an image cannot be read as a 3D volume, therefore it is read as a volume sequence, with frames along the last axis.
  std::cout << "TestNrrdVolumeSequenceWithoutListAxis" << std::endl;

  const int dimensions[3] = { 6, 5, 4 };
  const int numberOfFrames = 3;
  const std::string fileName = tempFilename(tempDir, "domain4d", "nrrd", true);
  {
    std::ofstream file(fileName, std::ios::binary);
    file << "NRRD0004\n"
         << "type: short\n"
         << "dimension: 4\n"
         << "sizes: " << dimensions[0] << " " << dimensions[1] << " " << dimensions[2] << " " << numberOfFrames << "\n"
         << "kinds: domain domain domain domain\n"
         << "endian: little\n"
         << "encoding: raw\n"
         << "\n";
    // Voxel value is the voxel index in the 4D image
    const vtkIdType numberOfVoxels = static_cast<vtkIdType>(dimensions[0]) * dimensions[1] * dimensions[2] * numberOfFrames;
    for (vtkIdType voxelIndex = 0; voxelIndex < numberOfVoxels; ++voxelIndex)
    {
      const vtkTypeInt16 voxelValue = static_cast<vtkTypeInt16>(voxelIndex);
      file.write(reinterpret_cast<const char*>(&voxelValue), sizeof(voxelValue));
    }
    CHECK_BOOL(file.good(), true);
  }

  CHECK_BOOL(vtkITKImageSequenceReader::IsImageSequenceFile(fileName.c_str()), true);

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLVolumeSequenceStorageNode> storageNode;
  scene->AddNode(storageNode);
  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  scene->AddNode(sequenceNode);
  storageNode->SetFileName(fileName.c_str());
  CHECK_BOOL(storageNode->ReadData(sequenceNode), true);
  CHECK_INT(sequenceNode->GetNumberOfDataNodes(), numberOfFrames);

  const vtkIdType numberOfVoxelsInFrame = static_cast<vtkIdType>(dimensions[0]) * dimensions[1] * dimensions[2];
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(frameIndex));
    CHECK_NOT_NULL(volumeNode);
    vtkImageData* imageData = volumeNode->GetImageData();
    CHECK_NOT_NULL(imageData);
    for (int axis = 0; axis < 3; ++axis)
    {
      CHECK_INT(imageData->GetDimensions()[axis], dimensions[axis]);
    }
    for (int k = 0; k < dimensions[2]; ++k)
    {
      for (int j = 0; j < dimensions[1]; ++j)
      {
        for (int i = 0; i < dimensions[0]; ++i)
        {
          const vtkIdType expectedVoxelValue = frameIndex * numberOfVoxelsInFrame + i + j * dimensions[0] + k * dimensions[0] * dimensions[1];
          CHECK_INT(static_cast<int>(imageData->GetScalarComponentAsDouble(i, j, k, 0)), static_cast<int>(expectedVoxelValue));
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

int vtkMRMLVolumeSequenceStorageNodeNrrdTest(int argc, char* argv[])
{
  if (argc != 11)
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

  // Image stored using left-handed IJK coordinate system
  const char* volume_LeftHandedDomainDomainDomainList = argv[10];
  CHECK_EXIT_SUCCESS(TestLeftHandedNrrdVolumeSequence(volume_LeftHandedDomainDomainDomainList));

  // 4D image without list axis
  CHECK_EXIT_SUCCESS(TestNrrdVolumeSequenceWithoutListAxis(tempDir));

  std::cout << "-----------------------------------------------------" << std::endl;

  vtkNew<vtkMRMLVolumeSequenceStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  std::cout << "\nTest passed." << std::endl;
  return EXIT_SUCCESS;
}
