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

#include <vtkCylinderSource.h>
#include <vtkGeometryFilter.h>
#include <vtkNew.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVoxel.h>

//---------------------------------------------------------------------------
int TestReadWriteData(vtkMRMLScene* scene, const char* extension, vtkPointSet*mesh);
void CreateVoxelMeshes(vtkUnstructuredGrid* ug, vtkPolyData* poly);

//---------------------------------------------------------------------------
int vtkMRMLModelStorageNodeTest1(int argc, char * argv[] )
{
  if (argc != 2)
    {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLModelStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  vtkNew<vtkMRMLScene> scene;
  const char* tempDir = argv[1];
  scene->SetRootDirectory(tempDir);

  vtkNew<vtkUnstructuredGrid> ug;
  vtkNew<vtkPolyData> poly;
  CreateVoxelMeshes(ug.GetPointer(), poly.GetPointer());
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtk", ug.GetPointer()));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtu", ug.GetPointer()));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtk", poly.GetPointer()));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtp", poly.GetPointer()));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".stl", poly.GetPointer()));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".ply", poly.GetPointer()));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".obj", poly.GetPointer()));

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadWriteData(vtkMRMLScene* scene, const char *extension, vtkPointSet *mesh)
{
  std::string fileName = std::string(scene->GetRootDirectory()) +
                         std::string("/vtkMRMLModelNodeTest1") +
                         std::string(extension);

  int numberOfPoints = mesh->GetNumberOfPoints();
  CHECK_BOOL(numberOfPoints > 0, true);

  // Add model node
  vtkNew<vtkMRMLModelNode> modelNode;
  modelNode->SetAndObserveMesh(mesh);
  CHECK_NOT_NULL(modelNode->GetMesh());
  CHECK_NOT_NULL(scene->AddNode(modelNode.GetPointer()));

  bool isPoly = (modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType);
  std::cout << "Testing " << extension << " for "
            <<  (isPoly ? "polydata" : "unstructured grid")
            << " mesh." << std::endl;

  // Add storage node
  modelNode->AddDefaultStorageNode();
  vtkMRMLStorageNode* storageNode = modelNode->GetStorageNode();
  CHECK_NOT_NULL(storageNode);
  storageNode->SetFileName(fileName.c_str());

  // Test writing
  CHECK_BOOL(storageNode->WriteData(modelNode.GetPointer()), true);

  // Clear data from model node
  modelNode->SetAndObservePolyData(nullptr);
  CHECK_NULL(modelNode->GetMesh());

  // Test reading
  CHECK_BOOL(storageNode->ReadData(modelNode.GetPointer()), true);
  vtkPointSet* mesh2 = modelNode->GetMesh();
  CHECK_NOT_NULL(mesh2);
  CHECK_INT(mesh2->GetNumberOfPoints(), numberOfPoints);

  return EXIT_SUCCESS;
}
