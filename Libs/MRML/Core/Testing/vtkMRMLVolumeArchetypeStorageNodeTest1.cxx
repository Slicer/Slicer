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
#include "vtkMatrix3x3.h"
#include "vtkMatrix4x4.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include <vtksys/SystemTools.hxx>
#include <vtkTransform.h>
#include <iostream>

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

  vtkMRMLVectorVolumeNode* vectorVolumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  CHECK_NOT_NULL(vectorVolumeNode);

  // Add some image data
  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(10, 20, 30);
  imageData->AllocateScalars(VTK_FLOAT, 3);
  imageData->GetPointData()->GetScalars()->Fill(12.5);
  vectorVolumeNode->SetAndObserveImageData(imageData);

  vtkMRMLVolumeArchetypeStorageNode* storageNode = vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVolumeArchetypeStorageNode"));
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

    vectorVolumeNode->SetVoxelVectorType(vtkMRMLVolumeNode::VoxelVectorTypeSpatialCovariant);
    storageNode->SetFileName(tempFilename(tempDir, "vector_spatial_covariant", fileExtension, true).c_str());
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

    storageNode->SetFileName(tempFilename(tempDir, "vector_spatial_covariant", fileExtension).c_str());
    CHECK_BOOL(storageNode->ReadData(vectorVolumeNode), true);
    CHECK_INT(vectorVolumeNode->GetVoxelVectorType(), vtkMRMLVolumeNode::VoxelVectorTypeSpatialCovariant);
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

int TestFlipsLeftHandedVolumes(const std::string& tempDir)
{
  std::cout << "TestFlipsLeftHandedVolumes" << std::endl;
  // Create a flipped image data with arbitrary K values
  vtkNew<vtkMRMLScene> scene;

  auto volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode"));
  CHECK_NOT_NULL(volumeNode);

  int n_i{ 10 }, n_j{ 20 }, n_k{ 30 };
  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(n_i, n_j, n_k);
  imageData->AllocateScalars(VTK_FLOAT, 1);
  imageData->GetPointData()->GetScalars()->Fill(0);

  // Fill the volume with non-uniform content
  for (int i = 0; i < n_i; i++)
  {
    for (int j = 0; j < n_j; j++)
    {
      for (int k = 0; k < n_k; k++)
      {
        imageData->SetScalarComponentFromDouble(i, j, k, 0, i * 11 + j * 13 + k * 17);
      }
    }
  }

  // Generate an arbitrary left-handed IJK to RAS transform
  vtkNew<vtkTransform> ijkToRasTransform;
  ijkToRasTransform->Translate(10.0, 20.0, -35.0);
  ijkToRasTransform->RotateY(30.0);
  ijkToRasTransform->RotateZ(45.0);
  ijkToRasTransform->Scale(1.2, -0.3, 1.7); // left-handed due to single negative value
  volumeNode->SetIJKToRASMatrix(ijkToRasTransform->GetMatrix());

  // Verify that IJK coordinate system is left-handed
  CHECK_BOOL(vtkMRMLVolumeNode::IsIJKCoordinateSystemRightHanded(ijkToRasTransform->GetMatrix()), false);

  // Write the volume to a file and then read it
  const auto fileName = tempFilename(tempDir, "left_handed_ijk_volume", "mha", true);
  auto storageNode = vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVolumeArchetypeStorageNode"));
  CHECK_NOT_NULL(storageNode);
  storageNode->SetSingleFile(true);
  volumeNode->SetAndObserveImageData(imageData);
  volumeNode->SetAndObserveStorageNodeID(storageNode->GetID());
  storageNode->SetFileName(fileName.c_str());
  CHECK_BOOL(storageNode->WriteData(volumeNode), true);
  auto volumeNode2 = vtkMRMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode"));
  CHECK_BOOL(storageNode->ReadData(volumeNode2), true);

  // Check that in the saved and loaded image IJK is right-handed
  vtkNew<vtkMatrix4x4> ijkToRas2;
  volumeNode2->GetIJKToRASMatrix(ijkToRas2);
  CHECK_BOOL(vtkMRMLVolumeNode::IsIJKCoordinateSystemRightHanded(ijkToRas2), true);

  // Check that the loaded volume has the same voxel values at the same physical locations
  vtkNew<vtkMatrix4x4> rasToIjk2;
  volumeNode2->GetRASToIJKMatrix(rasToIjk2);
  for (int i = 0; i < n_i; i++)
  {
    for (int j = 0; j < n_j; j++)
    {
      for (int k = 0; k < n_k; k++)
      {
        // Get IJK position in flipped volume at the same physical location
        double ijk[4] = { static_cast<double>(i), static_cast<double>(j), static_cast<double>(k), 1 };
        double ras[4] = { 0, 0, 0, 1 };
        ijkToRasTransform->MultiplyPoint(ijk, ras);
        double ijk2[4] = { 0, 0, 0, 1 };
        rasToIjk2->MultiplyPoint(ras, ijk2);
        double i2 = static_cast<int>(ijk2[0] + 0.5);
        double j2 = static_cast<int>(ijk2[1] + 0.5);
        double k2 = static_cast<int>(ijk2[2] + 0.5);
        if (i != i2 || j != j2 || k != (volumeNode2->GetImageData()->GetDimensions()[2] - 1 - k2))
        {
          std::cerr << "IJK mismatch: volume[" << i << ", " << j << ", " << k << "] != volume2[" << i2 << ", " << j2 << ", " << k2 << "]" << std::endl;
          return EXIT_FAILURE;
        }
        // Check voxel values
        double voxelValue = volumeNode->GetImageData()->GetScalarComponentAsDouble(i, j, k, 0);
        double voxelValue2 = volumeNode2->GetImageData()->GetScalarComponentAsDouble(i2, j2, k2, 0);
        if (voxelValue != voxelValue2)
        {
          std::cerr << "Voxel value mismatch: volume[" << i << ", " << j << ", " << k << "]=" << voxelValue << " != volume2[" << i2 << ", " << j2 << ", " << k2
                    << "]=" << voxelValue2 << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
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

  // Expect warning about TIFF file format not recommended
  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  CHECK_EXIT_SUCCESS(TestVoxelVectorType(tempDir, "tif", false, false, true, false));
  TESTING_OUTPUT_ASSERT_WARNINGS_END();

  CHECK_EXIT_SUCCESS(TestVoxelVectorType(tempDir, "jpg", false, false, true, false));
  CHECK_EXIT_SUCCESS(TestFlipsLeftHandedVolumes(tempDir));

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}
