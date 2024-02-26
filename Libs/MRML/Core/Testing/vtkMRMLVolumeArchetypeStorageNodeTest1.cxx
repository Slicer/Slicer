/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include <vtksys/SystemTools.hxx>

std::string tempFilename(std::string tempDir, std::string suffix, std::string fileExtension, bool remove = false)
{
  std::string filename = tempDir + "/vtkMRMLVolumeArchetypeStorageNodeTest1_" + suffix + "." + fileExtension;
  // remove file if exists
  if (remove && vtksys::SystemTools::FileExists(filename.c_str(), true))
  {
    vtksys::SystemTools::RemoveFile(filename.c_str());
  }
  return filename;
}

//---------------------------------------------------------------------------
int TestVoxelVectorType(const std::string& tempDir,
                        const std::string& fileExtension,
                        bool canWriteUndefinedVector,
                        bool canWriteSpatialVector,
                        bool canWriteColorRGB,
                        bool canWriteColorRGBA)
{
  // Check if voxel vector type is correctly saved and restored.
  std::cout << "TestVoxelVectorType: " << fileExtension << std::endl;

  vtkNew<vtkMRMLScene> scene;

  vtkMRMLVectorVolumeNode* vectorVolumeNode =
    vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  CHECK_NOT_NULL(vectorVolumeNode);

  // Add some image data
  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(10, 20, 30);
  imageData->AllocateScalars(VTK_FLOAT, 3);
  imageData->GetPointData()->GetScalars()->Fill(12.5);
  vectorVolumeNode->SetAndObserveImageData(imageData);

  vtkMRMLVolumeArchetypeStorageNode* storageNode =
    vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVolumeArchetypeStorageNode"));
  CHECK_NOT_NULL(storageNode);
  storageNode->SetSingleFile(true);
  vectorVolumeNode->SetAndObserveStorageNodeID(storageNode->GetID());

  // Test writing

  if (canWriteUndefinedVector)
  {
    vectorVolumeNode->SetVoxelVectorType(vtkMRMLVolumeNode::VoxelVectorTypeUndefined);
    storageNode->SetFileName(tempFilename(tempDir, "vector_undefined", fileExtension, true).c_str());
    CHECK_BOOL(storageNode->WriteData(vectorVolumeNode), true);
  }

  if (canWriteSpatialVector)
  {
    vectorVolumeNode->SetVoxelVectorType(vtkMRMLVolumeNode::VoxelVectorTypeSpatial);
    storageNode->SetFileName(tempFilename(tempDir, "vector_spatial", fileExtension, true).c_str());
    CHECK_BOOL(storageNode->WriteData(vectorVolumeNode), canWriteSpatialVector);
  }

  if (canWriteColorRGB)
  {
    imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    imageData->GetPointData()->GetScalars()->Fill(25);
    vectorVolumeNode->SetVoxelVectorType(vtkMRMLVolumeNode::VoxelVectorTypeColorRGB);
    storageNode->SetFileName(tempFilename(tempDir, "vector_rgb", fileExtension, true).c_str());
    CHECK_BOOL(storageNode->WriteData(vectorVolumeNode), true);
  }

  if (canWriteColorRGBA)
  {
    imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 4);
    imageData->GetPointData()->GetScalars()->Fill(36);
    vectorVolumeNode->SetVoxelVectorType(vtkMRMLVolumeNode::VoxelVectorTypeColorRGBA);
    storageNode->SetFileName(tempFilename(tempDir, "vector_rgba", fileExtension, true).c_str());
    CHECK_BOOL(storageNode->WriteData(vectorVolumeNode), true);
  }

  // Test reading

  if (canWriteUndefinedVector)
  {
    storageNode->SetFileName(tempFilename(tempDir, "vector_undefined", fileExtension).c_str());
    CHECK_BOOL(storageNode->ReadData(vectorVolumeNode), true);
    CHECK_INT(vectorVolumeNode->GetVoxelVectorType(), vtkMRMLVolumeNode::VoxelVectorTypeUndefined);
  }

  if (canWriteSpatialVector)
  {
    storageNode->SetFileName(tempFilename(tempDir, "vector_spatial", fileExtension).c_str());
    CHECK_BOOL(storageNode->ReadData(vectorVolumeNode), true);
    CHECK_INT(vectorVolumeNode->GetVoxelVectorType(), vtkMRMLVolumeNode::VoxelVectorTypeSpatial);
  }

  if (canWriteColorRGB)
  {
    storageNode->SetFileName(tempFilename(tempDir, "vector_rgb", fileExtension).c_str());
    CHECK_BOOL(storageNode->ReadData(vectorVolumeNode), true);
    CHECK_INT(vectorVolumeNode->GetVoxelVectorType(), vtkMRMLVolumeNode::VoxelVectorTypeColorRGB);
  }

  if (canWriteColorRGBA)
  {
    storageNode->SetFileName(tempFilename(tempDir, "vector_rgba", fileExtension).c_str());
    CHECK_BOOL(storageNode->ReadData(vectorVolumeNode), true);
    CHECK_INT(vectorVolumeNode->GetVoxelVectorType(), vtkMRMLVolumeNode::VoxelVectorTypeColorRGBA);
  }

  return EXIT_SUCCESS;
}

int vtkMRMLVolumeArchetypeStorageNodeTest1(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }
  const char* tempDir = argv[1];

  vtkNew<vtkMRMLVolumeArchetypeStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  // tested vector types ->                               undefined  spatial  RGB    RGBA
  CHECK_EXIT_SUCCESS(TestVoxelVectorType(tempDir, "nrrd", true, true, true, true));
  CHECK_EXIT_SUCCESS(TestVoxelVectorType(tempDir, "mha", true, false, false, false));
  CHECK_EXIT_SUCCESS(TestVoxelVectorType(tempDir, "nii", true, false, true, true));
  CHECK_EXIT_SUCCESS(TestVoxelVectorType(tempDir, "png", false, false, true, true));
  CHECK_EXIT_SUCCESS(TestVoxelVectorType(tempDir, "tif", false, false, true, false));
  CHECK_EXIT_SUCCESS(TestVoxelVectorType(tempDir, "jpg", false, false, true, false));

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}
