/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==============================================================================*/

// Logic includes
#include "vtkSlicerTransformsReader.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"

// VTK includes
#include <vtkNew.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <fstream>
#include <iostream>

namespace
{
std::string WriteFile(const std::string& directory, const std::string& fileName, const std::string& content)
{
  std::string filePath = directory + "/" + fileName;
  std::ofstream file(filePath.c_str(), std::ios::out | std::ios::binary);
  file << content;
  return filePath;
}
} // namespace

//-----------------------------------------------------------------------------
int vtkSlicerTransformsReaderTest1(int argc, char* argv[])
{
  if (argc < 3)
  {
    std::cerr << "Usage: vtkSlicerTransformsReaderTest1 /path/to/affineTransform.txt /path/to/temporary/directory" << std::endl;
    return EXIT_FAILURE;
  }
  const std::string transformFilePath = argv[1];
  const std::string tempDir = argv[2];
  vtksys::SystemTools::MakeDirectory(tempDir);

  vtkNew<vtkSlicerTransformsReader> reader;
  // Confidence of readers that only check the file extension (such as the text file reader)
  const double defaultTextConfidence = 0.5 + 0.01 * std::string(".txt").size();

  // ITK transform file with .txt extension
  CHECK_BOOL(vtkSlicerTransformsReader::IsITKTextTransformFile(transformFilePath), true);
  CHECK_BOOL(reader->CanLoadFileConfidence(transformFilePath) > defaultTextConfidence, true);

  // ITK transform file without the "#Insight Transform File" header line
  std::string noHeaderFilePath = WriteFile(tempDir,
                                           "vtkSlicerTransformsReaderTest1_noHeader.txt",
                                           "\n# Transform 0\r\nTransform: AffineTransform_double_3_3\r\n"
                                           "Parameters: 1 0 0 0 1 0 0 0 1 0 0 0\r\nFixedParameters: 0 0 0\r\n");
  CHECK_BOOL(vtkSlicerTransformsReader::IsITKTextTransformFile(noHeaderFilePath), true);
  CHECK_BOOL(reader->CanLoadFileConfidence(noHeaderFilePath) > defaultTextConfidence, true);

  // Plain text file
  std::string plainTextFilePath = WriteFile(tempDir, "vtkSlicerTransformsReaderTest1_plain.txt", "just text\nTransform: is mentioned here\n");
  CHECK_BOOL(vtkSlicerTransformsReader::IsITKTextTransformFile(plainTextFilePath), false);
  double plainTextConfidence = reader->CanLoadFileConfidence(plainTextFilePath);
  CHECK_BOOL(plainTextConfidence > 0.0, true); // it can still be chosen explicitly
  CHECK_BOOL(plainTextConfidence < defaultTextConfidence, true);

  // Empty text file
  std::string emptyFilePath = WriteFile(tempDir, "vtkSlicerTransformsReaderTest1_empty.txt", "");
  CHECK_BOOL(vtkSlicerTransformsReader::IsITKTextTransformFile(emptyFilePath), false);

  // Files with other extensions are not inspected
  std::string tfmFilePath = WriteFile(tempDir, "vtkSlicerTransformsReaderTest1_plain.tfm", "just text\n");
  CHECK_BOOL(reader->CanLoadFileConfidence(tfmFilePath) >= 0.5, true);

  return EXIT_SUCCESS;
}
