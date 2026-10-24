/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkCodedEntry.h"
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

namespace
{

//---------------------------------------------------------------------------
std::string readNrrdHeader(const std::string& fileName)
{
  // The NRRD header is the text before the first empty line
  std::ifstream file(fileName, std::ios::binary);
  std::string header;
  std::string line;
  while (std::getline(file, line))
  {
    if (line.empty() || line == "\r")
    {
      break;
    }
    header += line + "\n";
  }
  return header;
}

//---------------------------------------------------------------------------
int TestVoxelValueUnits(const std::string& tempDir)
{
  // The unit of the voxel values is stored in the standard "units" field of the pixel (component) axis
  // of the NRRD file and in VoxelValueUnits of the volume nodes.
  std::cout << "TestVoxelValueUnits" << std::endl;
  const int numberOfFrames = 2;
  vtkNew<vtkMRMLScene> scene;

  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  CHECK_NOT_NULL(sequenceNode);
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    vtkNew<vtkMRMLVectorVolumeNode> volumeNode;
    vtkNew<vtkImageData> imageData;
    imageData->SetDimensions(4, 3, 2);
    imageData->AllocateScalars(VTK_FLOAT, 3);
    imageData->GetPointData()->GetScalars()->Fill(frameIndex + 0.5);
    volumeNode->SetAndObserveImageData(imageData);
    vtkNew<vtkCodedEntry> units;
    units->SetValueSchemeMeaning("mm/s", "UCUM", "millimeter per second");
    volumeNode->SetVoxelValueUnits(units);
    volumeNode->SetVoxelValueQuantityFromLabel("velocity");
    sequenceNode->SetDataNodeAtValue(volumeNode, std::to_string(frameIndex));
  }

  // Units are copied into the sequence items
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    vtkMRMLScalarVolumeNode* itemVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(frameIndex));
    CHECK_NOT_NULL(itemVolumeNode);
    CHECK_NOT_NULL(itemVolumeNode->GetVoxelValueUnits());
    CHECK_STRING(itemVolumeNode->GetVoxelValueUnits()->GetCodeValue(), "mm/s");
  }

  // Write
  vtkMRMLVolumeSequenceStorageNode* storageNode = vtkMRMLVolumeSequenceStorageNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVolumeSequenceStorageNode"));
  CHECK_NOT_NULL(storageNode);
  std::string fileName = tempFilename(tempDir, "voxel_value_units", "seq.nrrd", true);
  storageNode->SetFileName(fileName.c_str());
  CHECK_BOOL(storageNode->WriteData(sequenceNode), true);

  // The unit is in the standard NRRD "units" field, as the unit of the first (pixel) axis
  std::string header = readNrrdHeader(fileName);
  std::cout << header << std::endl;
  CHECK_BOOL(header.find("kinds: vector domain domain domain list") != std::string::npos, true);
  CHECK_BOOL(header.find("units: \"mm/s\"") != std::string::npos, true);
  // The quantity is in the standard NRRD "labels" field, as the label of the first (pixel) axis
  CHECK_BOOL(header.find("labels: \"velocity\"") != std::string::npos, true);

  // Read
  vtkMRMLSequenceNode* readSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  CHECK_NOT_NULL(readSequenceNode);
  CHECK_BOOL(storageNode->ReadData(readSequenceNode), true);
  CHECK_INT(readSequenceNode->GetNumberOfDataNodes(), numberOfFrames);
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    vtkMRMLScalarVolumeNode* readVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(readSequenceNode->GetNthDataNode(frameIndex));
    CHECK_NOT_NULL(readVolumeNode);
    CHECK_NOT_NULL(readVolumeNode->GetVoxelValueUnits());
    CHECK_STRING(readVolumeNode->GetVoxelValueUnits()->GetCodeValue(), "mm/s");
    CHECK_STRING(readVolumeNode->GetVoxelValueUnits()->GetCodingSchemeDesignator(), "UCUM");
    CHECK_NOT_NULL(readVolumeNode->GetVoxelValueQuantity());
    CHECK_STRING(readVolumeNode->GetVoxelValueQuantity()->GetCodeMeaning(), "velocity");
  }

  // A scalar volume sequence has no pixel axis, so the unit cannot be stored in the file:
  // writing must still succeed, and the sequence is read back without units.
  vtkMRMLSequenceNode* scalarSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  CHECK_NOT_NULL(scalarSequenceNode);
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
    vtkNew<vtkImageData> imageData;
    imageData->SetDimensions(4, 3, 2);
    imageData->AllocateScalars(VTK_SHORT, 1);
    imageData->GetPointData()->GetScalars()->Fill(frameIndex);
    volumeNode->SetAndObserveImageData(imageData);
    vtkNew<vtkCodedEntry> units;
    units->SetValueSchemeMeaning("[hnsf'U]", "UCUM", "Hounsfield unit");
    volumeNode->SetVoxelValueUnits(units);
    scalarSequenceNode->SetDataNodeAtValue(volumeNode, std::to_string(frameIndex));
  }
  std::string scalarFileName = tempFilename(tempDir, "voxel_value_units_scalar", "seq.nrrd", true);
  storageNode->SetFileName(scalarFileName.c_str());
  CHECK_BOOL(storageNode->WriteData(scalarSequenceNode), true);
  CHECK_BOOL(readNrrdHeader(scalarFileName).find("hnsf") == std::string::npos, true);
  vtkMRMLSequenceNode* readScalarSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  CHECK_NOT_NULL(readScalarSequenceNode);
  CHECK_BOOL(storageNode->ReadData(readScalarSequenceNode), true);
  CHECK_INT(readScalarSequenceNode->GetNumberOfDataNodes(), numberOfFrames);
  vtkMRMLScalarVolumeNode* readScalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(readScalarSequenceNode->GetNthDataNode(0));
  CHECK_NOT_NULL(readScalarVolumeNode);
  CHECK_NULL(readScalarVolumeNode->GetVoxelValueUnits());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestVoxelValueUnitsWithoutLabels(const std::string& tempDir)
{
  // Units must be written for an axis that has no label (voxel values without quantity), and an axis
  // that has a label but no unit (sequence axis with index name but no index unit) must not break writing.
  // ITK's NrrdIO used to print the units of an axis only if the axis had a label, and crashed on a labeled axis without unit.
  std::cout << "TestVoxelValueUnitsWithoutLabels" << std::endl;
  const int numberOfFrames = 2;
  vtkNew<vtkMRMLScene> scene;

  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  CHECK_NOT_NULL(sequenceNode);
  sequenceNode->SetIndexName("frame");
  sequenceNode->SetIndexUnit("");
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    vtkNew<vtkMRMLVectorVolumeNode> volumeNode;
    vtkNew<vtkImageData> imageData;
    imageData->SetDimensions(4, 3, 2);
    imageData->AllocateScalars(VTK_FLOAT, 3);
    imageData->GetPointData()->GetScalars()->Fill(frameIndex + 0.5);
    volumeNode->SetAndObserveImageData(imageData);
    vtkNew<vtkCodedEntry> units;
    units->SetValueSchemeMeaning("mm/s", "UCUM", "millimeter per second");
    volumeNode->SetVoxelValueUnits(units);
    // No voxel value quantity: the pixel axis has units but no label
    sequenceNode->SetDataNodeAtValue(volumeNode, std::to_string(frameIndex));
  }

  vtkMRMLVolumeSequenceStorageNode* storageNode = vtkMRMLVolumeSequenceStorageNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVolumeSequenceStorageNode"));
  CHECK_NOT_NULL(storageNode);
  std::string fileName = tempFilename(tempDir, "voxel_value_units_without_labels", "seq.nrrd", true);
  storageNode->SetFileName(fileName.c_str());
  CHECK_BOOL(storageNode->WriteData(sequenceNode), true);

  std::string header = readNrrdHeader(fileName);
  std::cout << header << std::endl;
  CHECK_BOOL(header.find("units: \"mm/s\" \"\" \"\" \"\" \"\"") != std::string::npos, true);
  CHECK_BOOL(header.find("labels: \"\" \"\" \"\" \"\" \"frame\"") != std::string::npos, true);

  vtkMRMLSequenceNode* readSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  CHECK_NOT_NULL(readSequenceNode);
  CHECK_BOOL(storageNode->ReadData(readSequenceNode), true);
  CHECK_INT(readSequenceNode->GetNumberOfDataNodes(), numberOfFrames);
  CHECK_STD_STRING(readSequenceNode->GetIndexName(), "frame");
  CHECK_STD_STRING(readSequenceNode->GetIndexUnit(), "");
  vtkMRMLScalarVolumeNode* readVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(readSequenceNode->GetNthDataNode(0));
  CHECK_NOT_NULL(readVolumeNode);
  CHECK_NOT_NULL(readVolumeNode->GetVoxelValueUnits());
  CHECK_STRING(readVolumeNode->GetVoxelValueUnits()->GetCodeValue(), "mm/s");
  CHECK_NULL(readVolumeNode->GetVoxelValueQuantity());

  return EXIT_SUCCESS;
}

} // namespace

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

  // Unit of the voxel values
  CHECK_EXIT_SUCCESS(TestVoxelValueUnits(tempDir));
  CHECK_EXIT_SUCCESS(TestVoxelValueUnitsWithoutLabels(tempDir));

  std::cout << "-----------------------------------------------------" << std::endl;

  vtkNew<vtkMRMLVolumeSequenceStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  std::cout << "\nTest passed." << std::endl;
  return EXIT_SUCCESS;
}
