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
#include "vtkSegmentationConverterFactory.h"

// Converter rules
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"

#include "vtkFractionalLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToFractionalLabelmapConversionRule.h"

int vtkMRMLSegmentationStorageNodeTest1(int argc, char * argv[] )
{
  vtkNew<vtkMRMLSegmentationStorageNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  if (argc != 3)
    {
    std::cerr << "Line " << __LINE__
              << " - Missing parameters !\n"
              << "Usage: " << argv[0] << " /path/to/temp /path/to/testdata"
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkSegmentationConverterFactory* converterFactory = vtkSegmentationConverterFactory::GetInstance();
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkFractionalLabelmapToClosedSurfaceConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToFractionalLabelmapConversionRule>::New());

  const char* tempDirArg = argv[1];
  const char* dataDirArg = argv[2];

  std::string dataDir = std::string(dataDirArg);
  std::string itkSnapSegmentationFilename = dataDir + "/ITKSnapSegmentation.nii.gz";
  std::string oldSlicerSegmentationFilename = dataDir + "/OldSlicerSegmentation.seg.nrrd"; // Segmentation before shared labelmaps implemented.
  std::string slicerSegmentationFilename = dataDir + "/SlicerSegmentation.seg.nrrd"; // Segmentation with shared labelmaps.

  // Test segmentation exported from ITK-SNAP
  std::cout << "Testing ITK-SNAP segmentation" << std::endl;
  {
    vtkNew<vtkMRMLSegmentationNode> segmentationNode;
    scene->AddNode(segmentationNode);
    vtkNew<vtkMRMLSegmentationStorageNode> segmentationStorageNode;
    scene->AddNode(segmentationStorageNode);
    segmentationStorageNode->SetFileName(itkSnapSegmentationFilename.c_str());
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
    segmentationStorageNode->SetFileName(oldSlicerSegmentationFilename.c_str());
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
    segmentationStorageNode->SetFileName(slicerSegmentationFilename.c_str());
    segmentationStorageNode->ReadData(segmentationNode);
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    CHECK_NOT_NULL(segmentation);

    int numberOfSegments = segmentation->GetNumberOfSegments();
    CHECK_INT(numberOfSegments, 3);

    int numberOfLayers = segmentation->GetNumberOfLayers(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    CHECK_INT(numberOfLayers, 2);
  }

  return EXIT_SUCCESS;
}
