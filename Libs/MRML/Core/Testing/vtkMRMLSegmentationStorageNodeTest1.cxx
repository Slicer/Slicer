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
#include "vtkMRMLScene.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationStorageNode.h"
#include "vtkOrientedImageData.h"
#include "vtkSegmentationConverterFactory.h"

// Converter rules
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"

#include "vtkFractionalLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToFractionalLabelmapConversionRule.h"

int vtkMRMLSegmentationStorageNodeTest1(int argc, char* argv[])
{
  vtkNew<vtkMRMLSegmentationStorageNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  if (argc != 5)
  {
    std::cerr << "Line " << __LINE__
      << " - Missing or extra parameters!\n"
      << "Usage: " << argv[0] << " /path/to/ITKSnapSegmentation.nii.gz /path/to/OldSlicerSegmentation.seg.nrrd /path/to/SlicerSegmentation.seg.nrrd"
      << std::endl;
    return EXIT_FAILURE;
  }

  vtkSegmentationConverterFactory* converterFactory = vtkSegmentationConverterFactory::GetInstance();
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkFractionalLabelmapToClosedSurfaceConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToFractionalLabelmapConversionRule>::New());

  const char* itkSnapSegmentationFilename = argv[1]; // ITKSnapSegmentation.nii.gz
  const char* oldSlicerSegmentationFilename = argv[2]; // OldSlicerSegmentation.seg.nrrd: Segmentation before shared labelmaps implemented.
  const char* slicerSegmentationFilename = argv[3]; // SlicerSegmentation.seg.nrrd: Segmentation with shared labelmaps.
  const char* tempDir = argv[4]; // Temporary folder where test segmentation files will be created

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
    std::string segmentationGeometryString = segmentation->DetermineCommonLabelmapGeometry(
      vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_AND_REFERENCE_GEOMETRY);
    CHECK_STD_STRING(segmentationGeometryString, "");

    // Clean up
    vtksys::SystemTools::RemoveFile(emptySegmentationFilename);
  }

  return EXIT_SUCCESS;
}
