/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// Tests reading and writing of volume sequences in NIfTI file format.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

// vtkITK includes
#include "vtkITKImageSequenceWriter.h"

// ITK includes
#include <nifti1.h>

#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTransform.h>
#include <fstream>
#include <iostream>

namespace
{
// Voxel value in scalar test images = VOXEL_VALUE_FRAME_MULTIPLIER * frameIndex + voxelIndex
const int VOXEL_VALUE_FRAME_MULTIPLIER = 1000;
// NIfTI stores image orientation and origin with float precision
const double NIFTI_GEOMETRY_TOLERANCE = 1e-3;
const double VOXEL_VALUE_TOLERANCE = 1e-6;
// Index of the sequence (time) axis in the dim and pixdim arrays of the NIfTI header
const int NIFTI_TIME_AXIS_INDEX = 4;

std::string tempFilename(std::string tempDir, std::string suffix, std::string fileExtension, bool remove = false)
{
  std::string filename = tempDir + "/vtkMRMLVolumeSequenceStorageNodeNiftiTest_" + suffix + "." + fileExtension;
  // remove file if exists
  if (remove && vtksys::SystemTools::FileExists(filename.c_str(), true))
  {
    vtksys::SystemTools::RemoveFile(filename.c_str());
  }
  return filename;
}
} // namespace

//---------------------------------------------------------------------------
bool WriteNiftiVolumeSequence(const std::string& fileName, int numberOfFrames, const int dimensions[3], vtkMatrix4x4* rasToIjk)
{
  // Write 4D NIfTI file. Voxel value = VOXEL_VALUE_FRAME_MULTIPLIER * frameIndex + voxelIndex
  vtkNew<vtkITKImageSequenceWriter> writer;
  writer->SetFileName(fileName.c_str());
  writer->SetImageIOClassName("NiftiImageIO");
  writer->SetRasToIJKMatrix(rasToIjk);
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    vtkNew<vtkImageData> frameImage;
    frameImage->SetDimensions(dimensions[0], dimensions[1], dimensions[2]);
    frameImage->AllocateScalars(VTK_SHORT, 1);
    short* voxels = static_cast<short*>(frameImage->GetScalarPointer());
    for (vtkIdType voxelIndex = 0; voxelIndex < frameImage->GetNumberOfPoints(); ++voxelIndex)
    {
      voxels[voxelIndex] = static_cast<short>(VOXEL_VALUE_FRAME_MULTIPLIER * frameIndex + voxelIndex);
    }
    writer->AddInputData(frameImage);
  }
  writer->Write();
  return writer->GetErrorCode() == 0;
}

//---------------------------------------------------------------------------
bool SetNiftiSequenceAxisUnits(const std::string& fileName, int timeUnitCode, float spacing, float offset)
{
  // Modify the unit, spacing, and offset of the 4th (time) axis in the header of an uncompressed NIfTI-1 file.
  //
  // The header is modified directly in the file, because:
  // - ITK cannot be used for creating the test files: itk::NiftiImageIO always writes the time unit as seconds,
  //   therefore files with other units (milliseconds, microseconds, Hz, ppm, rad/s, unknown) cannot be written.
  // - The NIfTI C library (niftiio, bundled with ITK) is not a direct dependency of MRML tests.
  //   Since the file is uncompressed, it is simple to update the header fields directly, using only the
  //   header structure definition in nifti1.h.
  std::fstream file(fileName, std::ios::in | std::ios::out | std::ios::binary);
  if (!file)
  {
    std::cerr << "Failed to open file: " << fileName << std::endl;
    return false;
  }
  nifti_1_header header;
  file.read(reinterpret_cast<char*>(&header), sizeof(header));
  if (!file || header.sizeof_hdr != static_cast<int>(sizeof(nifti_1_header)))
  {
    std::cerr << "Not a NIfTI-1 file: " << fileName << std::endl;
    return false;
  }
  // Keep the spatial unit, replace the temporal unit
  header.xyzt_units = SPACE_TIME_TO_XYZT(XYZT_TO_SPACE(header.xyzt_units), timeUnitCode);
  header.pixdim[NIFTI_TIME_AXIS_INDEX] = spacing;
  header.toffset = offset;
  file.seekp(0);
  file.write(reinterpret_cast<const char*>(&header), sizeof(header));
  return file.good();
}

//---------------------------------------------------------------------------
int TestNiftiVolumeSequenceStorage(const std::string& tempDir, const std::string& fileExtension)
{
  // Test if a 4D NIfTI file can be read as a volume sequence.
  std::cout << "TestNiftiVolumeSequenceStorage: " << fileExtension << std::endl;

  const int numberOfFrames = 3;
  const int dimensions[3] = { 6, 5, 4 };

  // Oriented, anisotropic, shifted geometry
  vtkNew<vtkTransform> ijkToRasTransform;
  ijkToRasTransform->Translate(12.5, -30.0, 7.25);
  ijkToRasTransform->RotateWXYZ(30.0, 0.2, 0.5, 1.0);
  ijkToRasTransform->Scale(0.8, 1.2, 2.5);
  vtkNew<vtkMatrix4x4> ijkToRas;
  ijkToRas->DeepCopy(ijkToRasTransform->GetMatrix());
  vtkNew<vtkMatrix4x4> rasToIjk;
  vtkMatrix4x4::Invert(ijkToRas, rasToIjk);

  // Write 4D NIfTI file
  std::string fileName = tempFilename(tempDir, "nifti", fileExtension, true);
  CHECK_BOOL(WriteNiftiVolumeSequence(fileName, numberOfFrames, dimensions, rasToIjk), true);

  // Read 4D NIfTI file as sequence
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLVolumeSequenceStorageNode> storageNode;
  scene->AddNode(storageNode);
  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  scene->AddNode(sequenceNode);
  storageNode->SetFileName(fileName.c_str());
  CHECK_BOOL(storageNode->ReadData(sequenceNode), true);

  CHECK_INT(sequenceNode->GetNumberOfDataNodes(), numberOfFrames);
  // ITK writes time unit as seconds and time spacing of 1.0
  CHECK_STD_STRING(sequenceNode->GetIndexName(), "time");
  CHECK_STD_STRING(sequenceNode->GetIndexUnit(), "s");

  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    CHECK_STD_STRING(sequenceNode->GetNthIndexValue(frameIndex), std::to_string(frameIndex));

    vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(frameIndex));
    CHECK_NOT_NULL(volumeNode);
    vtkImageData* imageData = volumeNode->GetImageData();
    CHECK_NOT_NULL(imageData);
    CHECK_INT(imageData->GetScalarType(), VTK_SHORT);
    CHECK_INT(imageData->GetNumberOfScalarComponents(), 1);
    CHECK_INT(imageData->GetDimensions()[0], dimensions[0]);
    CHECK_INT(imageData->GetDimensions()[1], dimensions[1]);
    CHECK_INT(imageData->GetDimensions()[2], dimensions[2]);

    // Voxel values
    for (int k = 0; k < dimensions[2]; ++k)
    {
      for (int j = 0; j < dimensions[1]; ++j)
      {
        for (int i = 0; i < dimensions[0]; ++i)
        {
          int expectedVoxelValue = VOXEL_VALUE_FRAME_MULTIPLIER * frameIndex + i + j * dimensions[0] + k * dimensions[0] * dimensions[1];
          CHECK_INT(static_cast<int>(imageData->GetScalarComponentAsDouble(i, j, k, 0)), expectedVoxelValue);
        }
      }
    }

    // Geometry (NIfTI stores orientation with float precision)
    vtkNew<vtkMatrix4x4> readIjkToRas;
    volumeNode->GetIJKToRASMatrix(readIjkToRas);
    for (int row = 0; row < 4; ++row)
    {
      for (int col = 0; col < 4; ++col)
      {
        CHECK_DOUBLE_TOLERANCE(readIjkToRas->GetElement(row, col), ijkToRas->GetElement(row, col), NIFTI_GEOMETRY_TOLERANCE);
      }
    }
  }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestNiftiVolumeSequenceIndexUnits(const std::string& tempDir)
{
  // Test if sequence index name, unit, and values are set from the 4th axis unit, spacing, and offset of a NIfTI file.
  std::cout << "TestNiftiVolumeSequenceIndexUnits" << std::endl;

  struct TestCase
  {
    int TimeUnitCode;
    float Spacing;
    float Offset;
    const char* IndexName;
    const char* IndexUnit;
    const char* IndexValues[3];
  };
  const TestCase testCases[] = {
    { NIFTI_UNITS_UNKNOWN, 2.0f, 5.0f, "frame", "", { "0", "1", "2" } }, // unknown unit: frame index is used
    { NIFTI_UNITS_SEC, 0.5f, 1.0f, "time", "s", { "1", "1.5", "2" } },
    { NIFTI_UNITS_MSEC, 33.5f, 10.0f, "time", "ms", { "10", "43.5", "77" } },
    { NIFTI_UNITS_USEC, 250.0f, 0.0f, "time", "us", { "0", "250", "500" } },
    { NIFTI_UNITS_HZ, 0.5f, 100.0f, "frequency", "Hz", { "100", "100.5", "101" } },
    { NIFTI_UNITS_PPM, 0.25f, 1.0f, "chemical shift", "ppm", { "1", "1.25", "1.5" } },
    { NIFTI_UNITS_RADS, 1.5f, 0.0f, "frequency", "rad/s", { "0", "1.5", "3" } },
  };

  const int numberOfFrames = 3;
  const int dimensions[3] = { 4, 3, 2 };
  vtkNew<vtkMatrix4x4> rasToIjk;
  for (const TestCase& testCase : testCases)
  {
    std::cout << "  Time unit code: " << testCase.TimeUnitCode << std::endl;
    std::string fileName = tempFilename(tempDir, "nifti_units", "nii", true);
    CHECK_BOOL(WriteNiftiVolumeSequence(fileName, numberOfFrames, dimensions, rasToIjk), true);
    CHECK_BOOL(SetNiftiSequenceAxisUnits(fileName, testCase.TimeUnitCode, testCase.Spacing, testCase.Offset), true);

    vtkNew<vtkMRMLScene> scene;
    vtkNew<vtkMRMLVolumeSequenceStorageNode> storageNode;
    scene->AddNode(storageNode);
    vtkNew<vtkMRMLSequenceNode> sequenceNode;
    scene->AddNode(sequenceNode);
    storageNode->SetFileName(fileName.c_str());
    CHECK_BOOL(storageNode->ReadData(sequenceNode), true);

    CHECK_INT(sequenceNode->GetNumberOfDataNodes(), numberOfFrames);
    CHECK_STD_STRING(sequenceNode->GetIndexName(), testCase.IndexName);
    CHECK_STD_STRING(sequenceNode->GetIndexUnit(), testCase.IndexUnit);
    for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
    {
      CHECK_STD_STRING(sequenceNode->GetNthIndexValue(frameIndex), testCase.IndexValues[frameIndex]);
    }
  }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestNiftiVolumeSequenceWrite(const std::string& tempDir)
{
  // Test if a volume sequence can be written to NIfTI file and read back.
  std::cout << "TestNiftiVolumeSequenceWrite" << std::endl;

  struct TestCase
  {
    const char* FileExtension;
    int IndexType;
    const char* IndexUnit;
    const char* IndexValues[3];
    const char* SequenceAttributeName; // attribute is not set if nullptr
    bool ExpectedWarning;
    const char* ExpectedIndexValues[3];
  };
  const TestCase testCases[] = {
    // Evenly spaced time index values are saved (in seconds)
    { "nii.gz", vtkMRMLSequenceNode::NumericIndex, "ms", { "10", "43.5", "77" }, nullptr, false, { "0.01", "0.0435", "0.077" } },
    { "nii", vtkMRMLSequenceNode::NumericIndex, "s", { "0", "0.5", "1" }, nullptr, false, { "0", "0.5", "1" } },
    // Not a time unit: index values are saved, but they are loaded as time in seconds
    { "nii", vtkMRMLSequenceNode::NumericIndex, "Hz", { "100", "100.5", "101" }, nullptr, true, { "100", "100.5", "101" } },
    { "nii", vtkMRMLSequenceNode::NumericIndex, "", { "0", "1", "2" }, nullptr, true, { "0", "1", "2" } },
    // Unevenly spaced or non-numeric index values are not saved
    { "nii", vtkMRMLSequenceNode::NumericIndex, "s", { "0", "1", "5" }, nullptr, true, { "0", "1", "2" } },
    { "nii", vtkMRMLSequenceNode::TextIndex, "", { "first", "second", "third" }, nullptr, true, { "0", "1", "2" } },
    // Attributes are not saved
    { "nii", vtkMRMLSequenceNode::NumericIndex, "s", { "0", "1", "2" }, "SomeAttribute", true, { "0", "1", "2" } },
  };

  const int numberOfFrames = 3;
  const int dimensions[3] = { 4, 3, 2 };
  vtkNew<vtkTransform> ijkToRasTransform;
  ijkToRasTransform->Translate(-5.0, 20.0, 3.5);
  ijkToRasTransform->RotateWXYZ(40.0, 1.0, 0.3, 0.2);
  ijkToRasTransform->Scale(1.5, 0.7, 3.0);
  vtkNew<vtkMatrix4x4> ijkToRas;
  ijkToRas->DeepCopy(ijkToRasTransform->GetMatrix());

  for (const TestCase& testCase : testCases)
  {
    std::cout << "  Extension: " << testCase.FileExtension << ", index unit: '" << testCase.IndexUnit << "', index values: " << testCase.IndexValues[0] << " "
              << testCase.IndexValues[1] << " " << testCase.IndexValues[2] << std::endl;

    vtkNew<vtkMRMLScene> scene;
    vtkNew<vtkMRMLSequenceNode> sequenceNode;
    scene->AddNode(sequenceNode);
    sequenceNode->SetIndexType(testCase.IndexType);
    sequenceNode->SetIndexName("custom");
    sequenceNode->SetIndexUnit(testCase.IndexUnit);
    if (testCase.SequenceAttributeName)
    {
      sequenceNode->SetAttribute(testCase.SequenceAttributeName, "some value");
    }
    for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
    {
      vtkNew<vtkImageData> imageData;
      imageData->SetDimensions(dimensions[0], dimensions[1], dimensions[2]);
      imageData->AllocateScalars(VTK_SHORT, 1);
      short* voxels = static_cast<short*>(imageData->GetScalarPointer());
      for (vtkIdType voxelIndex = 0; voxelIndex < imageData->GetNumberOfPoints(); ++voxelIndex)
      {
        voxels[voxelIndex] = static_cast<short>(VOXEL_VALUE_FRAME_MULTIPLIER * frameIndex + voxelIndex);
      }
      vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
      volumeNode->SetAndObserveImageData(imageData);
      volumeNode->SetIJKToRASMatrix(ijkToRas);
      sequenceNode->SetDataNodeAtValue(volumeNode, testCase.IndexValues[frameIndex]);
    }

    // Write
    vtkNew<vtkMRMLVolumeSequenceStorageNode> storageNode;
    scene->AddNode(storageNode);
    std::string fileName = tempFilename(tempDir, "nifti_write", testCase.FileExtension, true);
    storageNode->SetFileName(fileName.c_str());
    // Warnings reported by the writer are collected in the storage node's user messages (they are not logged)
    CHECK_BOOL(storageNode->WriteData(sequenceNode), true);
    bool niftiWarningFound = false;
    vtkMRMLMessageCollection* messages = storageNode->GetUserMessages();
    for (int messageIndex = 0; messageIndex < messages->GetNumberOfMessages(); ++messageIndex)
    {
      if (messages->GetNthMessageType(messageIndex) == vtkCommand::WarningEvent && messages->GetNthMessageText(messageIndex).find("NIfTI") != std::string::npos)
      {
        niftiWarningFound = true;
      }
    }
    CHECK_BOOL(niftiWarningFound, testCase.ExpectedWarning);

    // Read
    vtkNew<vtkMRMLSequenceNode> readSequenceNode;
    scene->AddNode(readSequenceNode);
    CHECK_BOOL(storageNode->ReadData(readSequenceNode), true);
    CHECK_INT(readSequenceNode->GetNumberOfDataNodes(), numberOfFrames);
    CHECK_STD_STRING(readSequenceNode->GetIndexName(), "time");
    CHECK_STD_STRING(readSequenceNode->GetIndexUnit(), "s");
    for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
    {
      CHECK_STD_STRING(readSequenceNode->GetNthIndexValue(frameIndex), testCase.ExpectedIndexValues[frameIndex]);
      vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(readSequenceNode->GetNthDataNode(frameIndex));
      CHECK_NOT_NULL(volumeNode);
      vtkImageData* imageData = volumeNode->GetImageData();
      CHECK_NOT_NULL(imageData);
      CHECK_INT(imageData->GetDimensions()[0], dimensions[0]);
      CHECK_INT(imageData->GetDimensions()[1], dimensions[1]);
      CHECK_INT(imageData->GetDimensions()[2], dimensions[2]);
      // Check value of the last voxel
      const int lastVoxelIjk[3] = { dimensions[0] - 1, dimensions[1] - 1, dimensions[2] - 1 };
      const int lastVoxelIndex = lastVoxelIjk[0] + lastVoxelIjk[1] * dimensions[0] + lastVoxelIjk[2] * dimensions[0] * dimensions[1];
      CHECK_INT(static_cast<int>(imageData->GetScalarComponentAsDouble(lastVoxelIjk[0], lastVoxelIjk[1], lastVoxelIjk[2], 0)),
                VOXEL_VALUE_FRAME_MULTIPLIER * frameIndex + lastVoxelIndex);
      vtkNew<vtkMatrix4x4> readIjkToRas;
      volumeNode->GetIJKToRASMatrix(readIjkToRas);
      for (int row = 0; row < 4; ++row)
      {
        for (int col = 0; col < 4; ++col)
        {
          CHECK_DOUBLE_TOLERANCE(readIjkToRas->GetElement(row, col), ijkToRas->GetElement(row, col), NIFTI_GEOMETRY_TOLERANCE);
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestNiftiVectorVolumeSequence(const std::string& tempDir)
{
  // Test if color and vector volume sequences can be written to NIfTI file and read back.
  // NIfTI file format can only store color images with unsigned char components and spatial vectors
  // only as displacement vectors, therefore other vector volumes are saved with undefined voxel vector type.
  std::cout << "TestNiftiVectorVolumeSequence" << std::endl;

  struct TestCase
  {
    int ScalarType;
    int NumberOfComponents;
    int VoxelVectorType;
    int ExpectedVoxelVectorType;
    bool ExpectedWarning;
  };
  const TestCase testCases[] = {
    { VTK_UNSIGNED_CHAR, 3, vtkMRMLVolumeNode::VoxelVectorTypeColorRGB, vtkMRMLVolumeNode::VoxelVectorTypeColorRGB, false },
    { VTK_UNSIGNED_CHAR, 4, vtkMRMLVolumeNode::VoxelVectorTypeColorRGBA, vtkMRMLVolumeNode::VoxelVectorTypeColorRGBA, false },
    // Color images with components other than unsigned char are saved as vector images
    { VTK_FLOAT, 3, vtkMRMLVolumeNode::VoxelVectorTypeColorRGB, vtkMRMLVolumeNode::VoxelVectorTypeUndefined, true },
    { VTK_SHORT, 4, vtkMRMLVolumeNode::VoxelVectorTypeColorRGBA, vtkMRMLVolumeNode::VoxelVectorTypeUndefined, true },
    // Spatial vectors are saved as vector images, without coordinate system conversion
    { VTK_FLOAT, 3, vtkMRMLVolumeNode::VoxelVectorTypeSpatial, vtkMRMLVolumeNode::VoxelVectorTypeUndefined, true },
    { VTK_DOUBLE, 3, vtkMRMLVolumeNode::VoxelVectorTypeUndefined, vtkMRMLVolumeNode::VoxelVectorTypeUndefined, false },
  };

  const int numberOfFrames = 2;
  const int dimensions[3] = { 4, 3, 2 };
  vtkNew<vtkTransform> ijkToRasTransform;
  ijkToRasTransform->Translate(-5.0, 20.0, 3.5);
  ijkToRasTransform->Scale(1.5, 0.7, 3.0);
  vtkNew<vtkMatrix4x4> ijkToRas;
  ijkToRas->DeepCopy(ijkToRasTransform->GetMatrix());

  // Voxel values are positive, so that any sign change is detected, and small enough to be stored as unsigned char
  // (maximum value: (VOXEL_PATTERN_LENGTH - 1) * VOXEL_PATTERN_STEP + (numberOfFrames - 1) * FRAME_STEP + (maximum number of components - 1)).
  const int VOXEL_PATTERN_LENGTH = 5;
  const int VOXEL_PATTERN_STEP = 40;
  const int FRAME_STEP = 4;
  auto expectedVoxelValue = [=](int frameIndex, vtkIdType voxelIndex, int component)
  { return static_cast<double>((voxelIndex % VOXEL_PATTERN_LENGTH) * VOXEL_PATTERN_STEP + frameIndex * FRAME_STEP + component); };

  for (const TestCase& testCase : testCases)
  {
    std::cout << "  Scalar type: " << vtkImageScalarTypeNameMacro(testCase.ScalarType) << ", components: " << testCase.NumberOfComponents
              << ", voxel vector type: " << testCase.VoxelVectorType << std::endl;

    vtkNew<vtkMRMLScene> scene;
    vtkNew<vtkMRMLSequenceNode> sequenceNode;
    scene->AddNode(sequenceNode);
    for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
    {
      vtkNew<vtkImageData> imageData;
      imageData->SetDimensions(dimensions[0], dimensions[1], dimensions[2]);
      imageData->AllocateScalars(testCase.ScalarType, testCase.NumberOfComponents);
      vtkDataArray* scalars = imageData->GetPointData()->GetScalars();
      for (vtkIdType voxelIndex = 0; voxelIndex < scalars->GetNumberOfTuples(); ++voxelIndex)
      {
        for (int component = 0; component < testCase.NumberOfComponents; ++component)
        {
          scalars->SetComponent(voxelIndex, component, expectedVoxelValue(frameIndex, voxelIndex, component));
        }
      }
      vtkNew<vtkMRMLVectorVolumeNode> volumeNode;
      volumeNode->SetAndObserveImageData(imageData);
      volumeNode->SetIJKToRASMatrix(ijkToRas);
      volumeNode->SetVoxelVectorType(testCase.VoxelVectorType);
      sequenceNode->SetDataNodeAtValue(volumeNode, std::to_string(frameIndex));
    }

    // Write
    vtkNew<vtkMRMLVolumeSequenceStorageNode> storageNode;
    scene->AddNode(storageNode);
    std::string fileName = tempFilename(tempDir, "nifti_vector", "nii", true);
    storageNode->SetFileName(fileName.c_str());
    CHECK_BOOL(storageNode->WriteData(sequenceNode), true);
    CHECK_BOOL(storageNode->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::WarningEvent) > 0, testCase.ExpectedWarning);

    // Read
    vtkNew<vtkMRMLSequenceNode> readSequenceNode;
    scene->AddNode(readSequenceNode);
    CHECK_BOOL(storageNode->ReadData(readSequenceNode), true);
    CHECK_INT(readSequenceNode->GetNumberOfDataNodes(), numberOfFrames);
    for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
    {
      vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(readSequenceNode->GetNthDataNode(frameIndex));
      CHECK_NOT_NULL(volumeNode);
      CHECK_INT(volumeNode->GetVoxelVectorType(), testCase.ExpectedVoxelVectorType);
      vtkImageData* imageData = volumeNode->GetImageData();
      CHECK_NOT_NULL(imageData);
      CHECK_INT(imageData->GetScalarType(), testCase.ScalarType);
      CHECK_INT(imageData->GetNumberOfScalarComponents(), testCase.NumberOfComponents);
      CHECK_INT(imageData->GetDimensions()[0], dimensions[0]);
      CHECK_INT(imageData->GetDimensions()[1], dimensions[1]);
      CHECK_INT(imageData->GetDimensions()[2], dimensions[2]);
      vtkDataArray* scalars = imageData->GetPointData()->GetScalars();
      for (vtkIdType voxelIndex = 0; voxelIndex < scalars->GetNumberOfTuples(); ++voxelIndex)
      {
        for (int component = 0; component < testCase.NumberOfComponents; ++component)
        {
          CHECK_DOUBLE_TOLERANCE(scalars->GetComponent(voxelIndex, component), expectedVoxelValue(frameIndex, voxelIndex, component), VOXEL_VALUE_TOLERANCE);
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

namespace
{
//---------------------------------------------------------------------------
bool WriteNifti2VolumeSequenceFile(const std::string& fileName)
{
  // Write a minimal single-file 4D NIfTI-2 image (2x2x2 voxels, 3 frames, int16) in native byte order.
  // NIfTI-2 header is defined in nifti2.h, which is not available in ITK, therefore offsets are specified here.
  const vtkTypeInt32 nifti2HeaderSize = 540;
  const vtkTypeInt64 nifti2ExtensionSize = 4;
  const std::streamoff magicOffset = 4;
  const std::streamoff datatypeOffset = 12;
  const std::streamoff bitpixOffset = 14;
  const std::streamoff dimOffset = 16;
  const std::streamoff pixdimOffset = 104;
  const std::streamoff voxOffsetOffset = 168;
  const vtkTypeInt16 datatypeInt16 = NIFTI_TYPE_INT16;
  const vtkTypeInt16 bitsPerVoxel = 16;
  const vtkTypeInt64 dim[8] = { 4, 2, 2, 2, 3, 1, 1, 1 };
  const double pixdim[8] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
  const vtkTypeInt64 voxOffset = nifti2HeaderSize + nifti2ExtensionSize;
  const vtkTypeInt64 numberOfVoxels = 2 * 2 * 2 * 3;

  std::ofstream file(fileName, std::ios::binary);
  const std::string zeros(static_cast<size_t>(voxOffset + numberOfVoxels * sizeof(vtkTypeInt16)), '\0');
  file.write(zeros.data(), zeros.size());
  file.seekp(0);
  file.write(reinterpret_cast<const char*>(&nifti2HeaderSize), sizeof(nifti2HeaderSize));
  file.seekp(magicOffset);
  file.write("n+2\0\r\n\032\n", 8);
  file.seekp(datatypeOffset);
  file.write(reinterpret_cast<const char*>(&datatypeInt16), sizeof(datatypeInt16));
  file.seekp(bitpixOffset);
  file.write(reinterpret_cast<const char*>(&bitsPerVoxel), sizeof(bitsPerVoxel));
  file.seekp(dimOffset);
  file.write(reinterpret_cast<const char*>(dim), sizeof(dim));
  file.seekp(pixdimOffset);
  file.write(reinterpret_cast<const char*>(pixdim), sizeof(pixdim));
  file.seekp(voxOffsetOffset);
  file.write(reinterpret_cast<const char*>(&voxOffset), sizeof(voxOffset));
  return file.good();
}
} // namespace

//---------------------------------------------------------------------------
int TestNifti2VolumeSequenceReadError(const std::string& tempDir)
{
  // ITK cannot read NIfTI-2 files. Check that the user gets a message that explains why reading failed.
  std::cout << "TestNifti2VolumeSequenceReadError" << std::endl;

  const std::string fileName = tempFilename(tempDir, "nifti2", "nii", true);
  CHECK_BOOL(WriteNifti2VolumeSequenceFile(fileName), true);

  vtkNew<vtkMRMLScene> scene;
  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  CHECK_NOT_NULL(sequenceNode);
  vtkMRMLVolumeSequenceStorageNode* storageNode = vtkMRMLVolumeSequenceStorageNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVolumeSequenceStorageNode"));
  CHECK_NOT_NULL(storageNode);
  storageNode->SetFileName(fileName.c_str());

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(storageNode->ReadData(sequenceNode), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  const std::string messages = storageNode->GetUserMessages()->GetAllMessagesAsString();
  std::cout << "User messages:\n" << messages << std::endl;
  const size_t userMessagePosition = messages.find("NIfTI-2 file format is not supported");
  CHECK_BOOL(userMessagePosition != std::string::npos, true);
  // Source code location and object description are useful for developers, but they must be after the user-understandable message
  const size_t sourceLocationPosition = messages.find("vtkITKImageSequenceReader.cxx, line");
  CHECK_BOOL(sourceLocationPosition != std::string::npos && sourceLocationPosition > userMessagePosition, true);
  const size_t objectDescriptionPosition = messages.find("vtkITKImageSequenceReader (");
  CHECK_BOOL(objectDescriptionPosition != std::string::npos && objectDescriptionPosition > userMessagePosition, true);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeSequenceStorageNodeNiftiTest(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }

  const char* tempDir = argv[1];

  CHECK_EXIT_SUCCESS(TestNiftiVolumeSequenceStorage(tempDir, "nii"));
  CHECK_EXIT_SUCCESS(TestNiftiVolumeSequenceStorage(tempDir, "nii.gz"));
  CHECK_EXIT_SUCCESS(TestNiftiVolumeSequenceIndexUnits(tempDir));
  CHECK_EXIT_SUCCESS(TestNiftiVolumeSequenceWrite(tempDir));
  CHECK_EXIT_SUCCESS(TestNiftiVectorVolumeSequence(tempDir));
  CHECK_EXIT_SUCCESS(TestNifti2VolumeSequenceReadError(tempDir));

  std::cout << "\nTest passed." << std::endl;
  return EXIT_SUCCESS;
}
