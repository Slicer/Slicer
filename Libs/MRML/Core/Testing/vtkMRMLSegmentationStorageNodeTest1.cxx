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

// VTK includes
#include <vtkPointData.h>

// Converter rules
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"

#include "vtkFractionalLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToFractionalLabelmapConversionRule.h"

// STD includes
#include <thread>
#include <chrono>

//----------------------------------------------------------------------------
int TestConcurrentWriting(const char* tempDir)
{
  std::cout << "Testing concurrent writing protection..." << std::endl;

  std::vector<std::thread> threads;
  int numberOfThreads = 3;
  bool writeResults[3] = { false, false, false };

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSegmentationNode> segmentationNode;
  scene->AddNode(segmentationNode);

  vtkNew<vtkOrientedImageData> imageData;
  imageData->SetDimensions(500, 500, 500);
  imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  imageData->GetPointData()->GetScalars()->Fill(1);

  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  vtkNew<vtkSegment> segment;
  segment->SetName("TestSegment");
  segment->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), imageData);
  segmentation->AddSegment(segment);
  // Fill the segment with label value 1
  vtkOrientedImageData* segmentImageData = vtkOrientedImageData::SafeDownCast(segment->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()));
  if (segmentImageData)
  {
    void* ptr = segmentImageData->GetScalarPointer();
    if (ptr)
    {
      memset(ptr, 1, segmentImageData->GetScalarSize() * segmentImageData->GetNumberOfPoints());
    }
  }

  vtkNew<vtkMRMLSegmentationStorageNode> storageNode;
  scene->AddNode(storageNode);

  // IMPORTANT: Each thread writes to the SAME file to test locking
  std::string filename = vtksys::SystemTools::JoinPath({ tempDir, "/concurrent_segmentation.seg.nrrd" });
  storageNode->SetFileName(filename.c_str());

  // Lambda function for concurrent writing
  auto writeSegmentation = [&](int threadId, bool* result)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10 + threadId * 500));
    *result = storageNode->WriteData(segmentationNode);
    if (*result)
    {
      *result = vtksys::SystemTools::FileExists(filename);
    }
  };

  // Launch multiple threads to write concurrently to the SAME file
  for (int threadId = 0; threadId < numberOfThreads; ++threadId)
  {
    threads.emplace_back(writeSegmentation, threadId, &writeResults[threadId]);
  }

  // Wait for all threads to complete
  for (auto& thread : threads)
  {
    thread.join();
  }

  int successfulWrites = 0;
  for (int threadId = 0; threadId < numberOfThreads; ++threadId)
  {
    if (writeResults[threadId])
    {
      successfulWrites++;
      std::cout << "Thread " << threadId << " write succeeded" << std::endl;
    }
    else
    {
      std::cout << "Thread " << threadId << " write failed" << std::endl;
    }
  }

  if (successfulWrites != numberOfThreads)
  {
    std::cerr << "Expected exactly " << numberOfThreads << " successful writes, got " << successfulWrites << std::endl;
    std::cerr << "This suggests the atomic locking mechanism is not working properly" << std::endl;
    return EXIT_FAILURE;
  }

  // Clean up
  vtksys::SystemTools::RemoveFile(filename);

  std::cout << "Concurrent writing test passed - atomic locking is working!" << std::endl;
  return EXIT_SUCCESS;
}

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
    std::cout << "Write empty segmentation file: " << emptySegmentationFilename << std::endl;
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

  // Test concurrent writing protection
  CHECK_EXIT_SUCCESS(TestConcurrentWriting(tempDir));

  return EXIT_SUCCESS;
}
