/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

#include <vtkCleanPolyData.h>
#include <vtkCylinderSource.h>
#include <vtkNew.h>
#include <vtkTriangleFilter.h>

int TestReadWriteData(const char* tempDir);

int vtkMRMLModelStorageNodeTest1(int argc, char * argv[] )
{
  if (argc != 2)
    {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLModelStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  const char* tempDir = argv[1];
  CHECK_EXIT_SUCCESS(TestReadWriteData(tempDir));

  return EXIT_SUCCESS;
}

int TestReadWriteData(const char* tempDir)
{
  std::string sceneFileName = std::string(tempDir) + "/vtkMRMLModelStorageNodeTest1.mrml";
  std::string modelFileNameBase = std::string(tempDir) + "/vtkMRMLModelNodeTest1";
  std::vector< std::string > modelFileNameExtensions;
  modelFileNameExtensions.push_back(".vtk");
  modelFileNameExtensions.push_back(".vtp");
  modelFileNameExtensions.push_back(".stl");
  modelFileNameExtensions.push_back(".ply");
  modelFileNameExtensions.push_back(".obj");

  // Generate test polydata (triangle mesh without coincident points)
  vtkNew<vtkCylinderSource> cylinderSource;
  vtkNew<vtkTriangleFilter> triangulator;
  triangulator->SetInputConnection(cylinderSource->GetOutputPort());
  vtkNew<vtkCleanPolyData> cleaner;
  cleaner->PointMergingOn();
  cleaner->SetInputConnection(triangulator->GetOutputPort());
  cleaner->Update();
  // Validate test polydata
  int numberOfPoints = cleaner->GetOutput()->GetNumberOfPoints();
  int numberOfCells = cleaner->GetOutput()->GetNumberOfCells();
  CHECK_BOOL(numberOfPoints > 0, true);
  CHECK_BOOL(numberOfCells > 0, true);

  // Create test scene
  vtkNew<vtkMRMLScene> scene;
  scene->SetRootDirectory(tempDir);
  // Add model node
  vtkNew<vtkMRMLModelNode> modelNode;
  modelNode->SetPolyDataConnection(cleaner->GetOutputPort());
  CHECK_NOT_NULL(scene->AddNode(modelNode.GetPointer()));
  // Add storage node
  modelNode->AddDefaultStorageNode();
  vtkMRMLStorageNode* storageNode = modelNode->GetStorageNode();
  CHECK_NOT_NULL(storageNode);

  // Test writing and re-reading of test polydata
  for (std::vector< std::string >::iterator modelFileNameExtensionIt = modelFileNameExtensions.begin();
    modelFileNameExtensionIt != modelFileNameExtensions.end(); ++modelFileNameExtensionIt)
    {
    std::cout << "Testing " << (*modelFileNameExtensionIt) << "\n";
    std::string fileName = modelFileNameBase + (*modelFileNameExtensionIt);
    storageNode->SetFileName(fileName.c_str());
    // Test writing
    CHECK_BOOL(storageNode->WriteData(modelNode.GetPointer()), true);
    // Clear data from model node
    modelNode->SetAndObservePolyData(NULL);
    // Test reading
    CHECK_BOOL(storageNode->ReadData(modelNode.GetPointer()), true);
    vtkNew<vtkCleanPolyData> cleaner2;
    cleaner2->PointMergingOn();
    cleaner2->SetInputConnection(modelNode->GetPolyDataConnection());
    cleaner2->Update();
    CHECK_INT(cleaner2->GetOutput()->GetNumberOfPoints(), numberOfPoints);
    CHECK_INT(cleaner2->GetOutput()->GetNumberOfCells(), numberOfCells);
    }

  return EXIT_SUCCESS;
}
