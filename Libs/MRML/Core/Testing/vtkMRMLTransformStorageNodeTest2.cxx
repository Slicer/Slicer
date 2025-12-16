/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// Test storage of displacement field transforms

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"

#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtksys/SystemTools.hxx>
#include <vtkOrientedGridTransform.h>
#include <vtkGeneralTransform.h>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace
{
std::string tempFilename(std::string tempDir, std::string suffix, std::string fileExtension, bool remove = false)
{
  std::string filename = tempDir + "/vtkMRMLTransformStorageNodeTest2_" + suffix + fileExtension;
  // remove file if exists
  if (remove && vtksys::SystemTools::FileExists(filename.c_str(), true))
  {
    vtksys::SystemTools::RemoveFile(filename.c_str());
  }
  return filename;
}
} // namespace

//---------------------------------------------------------------------------

int TestGridTransformStorage(const std::string& tempDir, bool useTransformFromParent, const std::string& fileExtension, std::vector<int> dfDims = { 6, 8, 11 })
{
  CHECK_INT(dfDims.size(), 3);

  std::string transformDirection = useTransformFromParent ? "FromParent" : "ToParent";
  std::cout << "Testing grid transform " << transformDirection << " as " << fileExtension;
  std::cout << " file with size " << dfDims[0] << "x" << dfDims[1] << "x" << dfDims[2] << std::endl;
  auto timePoint0 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkMRMLScene> scene;

  int dimensions[3] = { dfDims[0], dfDims[1], dfDims[2] };

  // Create a grid transform node
  vtkNew<vtkMRMLTransformNode> transformNode;

  // Create a displacement grid
  vtkNew<vtkImageData> displacementGrid;
  displacementGrid->SetDimensions(dimensions);
  displacementGrid->SetOrigin(0.0, 0.0, 0.0);
  displacementGrid->SetSpacing(1.0, 1.0, 1.0);
  displacementGrid->AllocateScalars(VTK_DOUBLE, 3); // 3 components for X, Y, Z displacement

  // Fill the displacement grid with simple test data that varies with transformIndex
  double* displacementData = static_cast<double*>(displacementGrid->GetScalarPointer());
  int index = 0;
  const double scaleFactor = 1.5;
  for (int k = 0; k < dimensions[2]; k++)
  {
    for (int j = 0; j < dimensions[1]; j++)
    {
      for (int i = 0; i < dimensions[0]; i++)
      {
        // Displacement pattern varies with transform index: displacement = (i*scaleFactor, j*scaleFactor, k*scaleFactor)
        displacementData[index * 3 + 0] = i * scaleFactor; // X displacement
        displacementData[index * 3 + 1] = j * scaleFactor; // Y displacement
        displacementData[index * 3 + 2] = k * scaleFactor; // Z displacement
        index++;
      }
    }
  }

  // Create the oriented grid transform
  vtkNew<vtkOrientedGridTransform> gridTransform;
  gridTransform->SetDisplacementGridData(displacementGrid);
  auto timePoint1 = std::chrono::high_resolution_clock::now();
  auto duration0 = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint1 - timePoint0);
  std::cout << "Transform creation took: " << duration0.count() << " milliseconds." << std::endl;

  // Set up the transform direction
  if (useTransformFromParent)
  {
    transformNode->SetAndObserveTransformFromParent(gridTransform);
  }
  else
  {
    transformNode->SetAndObserveTransformToParent(gridTransform);
  }

  // Create storage node and write the transform
  vtkNew<vtkMRMLTransformStorageNode> storageNode;
  scene->AddNode(storageNode);

  auto timePoint2 = std::chrono::high_resolution_clock::now();
  auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint2 - timePoint1);
  std::cout << "Setting transform to node took: " << duration1.count() << " milliseconds." << std::endl;

  std::string filename = tempFilename(tempDir, "gridTransform_" + transformDirection, fileExtension, true);
  storageNode->SetFileName(filename.c_str());
  CHECK_BOOL(storageNode->WriteData(transformNode), true);
  auto timePoint3 = std::chrono::high_resolution_clock::now();
  auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint3 - timePoint2);
  std::cout << "Transform writing took: " << duration2.count() << " milliseconds." << std::endl;

  // Create a new transform node and read back the data
  vtkNew<vtkMRMLTransformNode> transformNodeRead;
  scene->AddNode(transformNodeRead);

  CHECK_BOOL(storageNode->ReadData(transformNodeRead), true);
  auto timePoint4 = std::chrono::high_resolution_clock::now();
  auto duration3 = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint4 - timePoint3);
  std::cout << "Transform reading took: " << duration3.count() << " milliseconds." << std::endl;

  // Verify the transform has the correct direction
  vtkAbstractTransform* readTransformFromParent = transformNodeRead->GetTransformFromParent();
  vtkAbstractTransform* readTransformToParent = transformNodeRead->GetTransformToParent();

  if (useTransformFromParent)
  {
    CHECK_NOT_NULL(readTransformFromParent);
  }
  else
  {
    CHECK_NOT_NULL(readTransformToParent);
  }

  // Get the grid transform to verify content
  vtkOrientedGridTransform* readGridTransform = nullptr;
  if (useTransformFromParent && readTransformFromParent)
  {
    readGridTransform = vtkOrientedGridTransform::SafeDownCast(readTransformFromParent);
  }
  else if (!useTransformFromParent && readTransformToParent)
  {
    readGridTransform = vtkOrientedGridTransform::SafeDownCast(readTransformToParent);
  }

  CHECK_NOT_NULL(readGridTransform);

  // Verify the displacement grid data matches at a few test points
  vtkImageData* readDisplacementGrid = readGridTransform->GetDisplacementGrid();
  CHECK_NOT_NULL(readDisplacementGrid);

  // Check dimensions
  int readDimensions[3];
  readDisplacementGrid->GetDimensions(readDimensions);
  CHECK_INT(readDimensions[0], dimensions[0]);
  CHECK_INT(readDimensions[1], dimensions[1]);
  CHECK_INT(readDimensions[2], dimensions[2]);

  // Check some specific displacement values
  double* readData = static_cast<double*>(readDisplacementGrid->GetScalarPointer());

  // Test points
  int testPoints[3][3] = { { 0, 0, 0 }, { 1, 1, 1 }, { 5, 3, 2 } };
  for (int testPointIndex = 0; testPointIndex < 3; testPointIndex++)
  {
    int i = testPoints[testPointIndex][0];
    int j = testPoints[testPointIndex][1];
    int k = testPoints[testPointIndex][2];
    int testIndex = i + j * dimensions[0] + k * dimensions[0] * dimensions[1];
    double expectedX = i * scaleFactor;
    double expectedY = j * scaleFactor;
    double expectedZ = k * scaleFactor;
    CHECK_DOUBLE_TOLERANCE(readData[testIndex * 3 + 0], expectedX, 1e-6); // X displacement
    CHECK_DOUBLE_TOLERANCE(readData[testIndex * 3 + 1], expectedY, 1e-6); // Y displacement
    CHECK_DOUBLE_TOLERANCE(readData[testIndex * 3 + 2], expectedZ, 1e-6); // Z displacement
  }

  std::cout << "Testing grid transform " << transformDirection << " as " << fileExtension << " file - success." << std::endl;

  return EXIT_SUCCESS;
}

int vtkMRMLTransformStorageNodeTest2(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }

  const char* tempDir = argv[1];

  // Test grid transform storage in both directions
  std::cout << "Testing grid transform storage..." << std::endl;

  // Test NRRD
  CHECK_EXIT_SUCCESS(TestGridTransformStorage(tempDir, true /*useTransformFromParent*/, ".nrrd"));
  CHECK_EXIT_SUCCESS(TestGridTransformStorage(tempDir, false /*useTransformFromParent*/, ".nrrd"));

  // Test NIfTI
  CHECK_EXIT_SUCCESS(TestGridTransformStorage(tempDir, true /*useTransformFromParent*/, ".nii.gz"));
  // Writing ToParent as NIfTI should fail
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  int exitCode = TestGridTransformStorage(tempDir, false /*useTransformFromParent*/, ".nii.gz");
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_BOOL(exitCode != EXIT_SUCCESS, true);

  // Test with a large image
  std::vector<int> largeDims = { 64, 64, 4 };
  // std::vector<int> largeDims = { 1024, 1024, 64 }; // uncomment for a realisticly large test image
  CHECK_EXIT_SUCCESS(TestGridTransformStorage(tempDir, true /*useTransformFromParent*/, ".h5", largeDims));

  std::cout << "\nTest passed." << std::endl;
  return EXIT_SUCCESS;
}
