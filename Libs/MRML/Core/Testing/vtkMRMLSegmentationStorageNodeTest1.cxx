/*==============================================================================

Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
Queen's University, Kingston, ON, Canada. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationStorageNode.h"
#include "vtkOrientedImageData.h"
#include "vtkSegmentationConverterFactory.h"

// vtkITK includes
#include "vtkITKImageSequenceWriter.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtksys/SystemTools.hxx>

// Converter rules
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"

#include "vtkFractionalLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToFractionalLabelmapConversionRule.h"

// STD includes
#include <iostream>

int vtkMRMLSegmentationStorageNodeTest1(int argc, char* argv[])
{
  vtkNew<vtkMRMLSegmentationStorageNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  if (argc != 5)
  {
    std::cerr << "Line " << __LINE__ << " - Missing or extra parameters!\n"
              << "Usage: " << argv[0] << " /path/to/ITKSnapSegmentation.nii.gz /path/to/OldSlicerSegmentation.seg.nrrd /path/to/SlicerSegmentation.seg.nrrd" << std::endl;
    return EXIT_FAILURE;
  }

  vtkSegmentationConverterFactory* converterFactory = vtkSegmentationConverterFactory::GetInstance();
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkFractionalLabelmapToClosedSurfaceConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToFractionalLabelmapConversionRule>::New());

  const char* itkSnapSegmentationFilename = argv[1];   // ITKSnapSegmentation.nii.gz
  const char* oldSlicerSegmentationFilename = argv[2]; // OldSlicerSegmentation.seg.nrrd: Segmentation before shared labelmaps implemented.
  const char* slicerSegmentationFilename = argv[3];    // SlicerSegmentation.seg.nrrd: Segmentation with shared labelmaps.
  const char* tempDir = argv[4];                       // Temporary folder where test segmentation files will be created

  // Test segmentation exported from ITK-SNAP
  std::cout << "Testing ITK-SNAP segmentation" << std::endl;
  {
    vtkNew<vtkMRMLSegmentationNode> segmentationNode;
    scene->AddNode(segmentationNode);
    vtkNew<vtkMRMLSegmentationStorageNode> segmentationStorageNode;
    scene->AddNode(segmentationStorageNode);
    segmentationStorageNode->SetFileName(itkSnapSegmentationFilename);
    segmentationStorageNode->ReadData(segmentationNode);
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    CHECK_NOT_NULL(segmentation);

    int numberOfSegments = segmentation->GetNumberOfSegments();
    CHECK_INT(numberOfSegments, 4);

    int numberOfLayers = segmentation->GetNumberOfLayers(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    CHECK_INT(numberOfLayers, 1);
  }

  // Test segmentation saved with Slicer before shared labelmaps were implemented
  std::cout << "Testing pre shared labelmap segmentation" << std::endl;
  {
    vtkNew<vtkMRMLSegmentationNode> segmentationNode;
    scene->AddNode(segmentationNode);
    vtkNew<vtkMRMLSegmentationStorageNode> segmentationStorageNode;
    scene->AddNode(segmentationStorageNode);
    segmentationStorageNode->SetFileName(oldSlicerSegmentationFilename);
    segmentationStorageNode->ReadData(segmentationNode);
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    CHECK_NOT_NULL(segmentation);

    int numberOfSegments = segmentation->GetNumberOfSegments();
    CHECK_INT(numberOfSegments, 3);

    int numberOfLayers = segmentation->GetNumberOfLayers(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    CHECK_INT(numberOfLayers, 3);
  }

  std::cout << "Testing shared labelmap segmentation" << std::endl;
  {
    vtkNew<vtkMRMLSegmentationNode> segmentationNode;
    scene->AddNode(segmentationNode);
    vtkNew<vtkMRMLSegmentationStorageNode> segmentationStorageNode;
    scene->AddNode(segmentationStorageNode);
    segmentationStorageNode->SetFileName(slicerSegmentationFilename);
    segmentationStorageNode->ReadData(segmentationNode);
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    CHECK_NOT_NULL(segmentation);

    int numberOfSegments = segmentation->GetNumberOfSegments();
    CHECK_INT(numberOfSegments, 3);

    int numberOfLayers = segmentation->GetNumberOfLayers(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    CHECK_INT(numberOfLayers, 2);
  }

  std::cout << "Testing empty segmentation" << std::endl;
  {
    // Create empty segmentation
    vtkNew<vtkMRMLSegmentationNode> segmentationNode;
    scene->AddNode(segmentationNode);
    segmentationNode->GetSegmentation()->AddEmptySegment();
    segmentationNode->GetSegmentation()->AddEmptySegment();
    segmentationNode->GetSegmentation()->AddEmptySegment();

    // Write to file
    vtkNew<vtkMRMLSegmentationStorageNode> segmentationStorageNode;
    scene->AddNode(segmentationStorageNode);
    std::string emptySegmentationFilename = std::string(tempDir) + "/EmptySegmentation.seg.nrrd";
    std::cout << "Write empty segmentation file: " << emptySegmentationFilename;
    segmentationStorageNode->SetFileName(emptySegmentationFilename.c_str());
    CHECK_INT(segmentationStorageNode->WriteData(segmentationNode), 1);

    // Read from file
    vtkNew<vtkMRMLSegmentationNode> segmentationNodeFromFile;
    scene->AddNode(segmentationNodeFromFile);
    segmentationStorageNode->ReadData(segmentationNodeFromFile);

    // Check basic content
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    CHECK_NOT_NULL(segmentation);
    int numberOfSegments = segmentation->GetNumberOfSegments();
    CHECK_INT(numberOfSegments, 3);

    // Check that no valid geometry is found.
    // The segmentation is stored as a single voxel, which would specify a geometry,
    // the storage node should ignore that when reading the file (single-voxel volume is a special case).
    std::string segmentationGeometryString = segmentation->DetermineCommonLabelmapGeometry(vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_AND_REFERENCE_GEOMETRY);
    CHECK_STD_STRING(segmentationGeometryString, "");

    // Clean up
    vtksys::SystemTools::RemoveFile(emptySegmentationFilename);
  }

  std::cout << "Testing reading of multi-frame NIfTI file as segmentation" << std::endl;
  {
    // A 4D (3D+t) NIfTI file cannot be read as a segmentation (only the first frame would be read, without orientation).
    // Check that reading fails and the user gets a message that explains why.
    const int numberOfFrames = 3;
    std::string multiFrameFilename = std::string(tempDir) + "/MultiFrameSegmentation.nii";
    vtkNew<vtkITKImageSequenceWriter> writer;
    writer->SetFileName(multiFrameFilename.c_str());
    writer->SetImageIOClassName("NiftiImageIO");
    vtkNew<vtkMatrix4x4> rasToIjk;
    writer->SetRasToIJKMatrix(rasToIjk);
    for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
    {
      vtkNew<vtkImageData> frameImage;
      frameImage->SetDimensions(6, 5, 4);
      frameImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
      frameImage->GetPointData()->GetScalars()->Fill(frameIndex + 1);
      writer->AddInputData(frameImage);
    }
    writer->Write();
    CHECK_INT(writer->GetErrorCode(), 0);

    vtkNew<vtkMRMLSegmentationNode> segmentationNode;
    scene->AddNode(segmentationNode);
    vtkNew<vtkMRMLSegmentationStorageNode> segmentationStorageNode;
    scene->AddNode(segmentationStorageNode);
    segmentationStorageNode->SetFileName(multiFrameFilename.c_str());
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    CHECK_INT(segmentationStorageNode->ReadData(segmentationNode), 0);
    TESTING_OUTPUT_ASSERT_ERRORS_END();

    const std::string messages = segmentationStorageNode->GetUserMessages()->GetAllMessagesAsString();
    std::cout << "User messages:\n" << messages << std::endl;
    CHECK_BOOL(messages.find("Load the file as a sequence") != std::string::npos, true);
    CHECK_INT(segmentationNode->GetSegmentation()->GetNumberOfSegments(), 0);

    // Clean up
    vtksys::SystemTools::RemoveFile(multiFrameFilename);
  }

  return EXIT_SUCCESS;
}
