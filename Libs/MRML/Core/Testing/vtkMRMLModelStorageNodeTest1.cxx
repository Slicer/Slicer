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

#include <array>

// STD includes
#include <fstream>
#include <iostream>

//---------------------------------------------------------------------------
int TestReadWriteData(vtkMRMLScene* scene, const char* extension, vtkPointSet* mesh, int coordinateSystem, bool cellsMayBeSubdivided = false);
int TestReadOBJWithCoordinateUnitComment(vtkMRMLScene* scene, const char* comment, double expectedScale);
void CreateVoxelMeshes(vtkUnstructuredGrid* ug, vtkPolyData* poly);

//---------------------------------------------------------------------------
int vtkMRMLModelStorageNodeTest1(int argc, char* argv[])
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
  for (int coordinateSystem : { vtkMRMLStorageNode::CoordinateSystemRAS, vtkMRMLStorageNode::CoordinateSystemLPS })
  {
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtk", poly.GetPointer(), coordinateSystem));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtp", poly.GetPointer(), coordinateSystem));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".stl", poly.GetPointer(), coordinateSystem, true));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".ply", poly.GetPointer(), coordinateSystem, true));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".obj", poly.GetPointer(), coordinateSystem));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtk", ug.GetPointer(), coordinateSystem));
    CHECK_EXIT_SUCCESS(TestReadWriteData(scene.GetPointer(), ".vtu", ug.GetPointer(), coordinateSystem));
  }

  // Test that vertex coordinate unit specified in OBJ file header comment
  // (written by Materialise Mimics and 3-matic) is used for scaling the coordinates to millimeters.
  CHECK_EXIT_SUCCESS(TestReadOBJWithCoordinateUnitComment(scene, "# vertex coordinates are measured in units, where 1 unit = 1.000000 mm", 1.0));
  CHECK_EXIT_SUCCESS(TestReadOBJWithCoordinateUnitComment(scene, "# vertex coordinates are measured in units, where 1 unit = 1000.000000 mm", 1000.0));

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadWriteData(vtkMRMLScene* scene, const char* extension, vtkPointSet* mesh, int coordinateSystem, bool cellsMayBeSubdivided /*=false*/)
{
  std::string fileName = std::string(scene->GetRootDirectory()) + std::string("/vtkMRMLModelNodeTest1") + std::string(extension);

  int numberOfPoints = mesh->GetNumberOfPoints();
  CHECK_BOOL(numberOfPoints > 0, true);

  int numberOfCells = mesh->GetNumberOfCells();
  CHECK_BOOL(numberOfCells > 0, true);

  // Add model node
  vtkNew<vtkMRMLModelNode> modelNode;
  modelNode->SetAndObserveMesh(mesh);
  CHECK_NOT_NULL(modelNode->GetMesh());
  CHECK_NOT_NULL(scene->AddNode(modelNode.GetPointer()));

  bool isPoly = (modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType);
  std::cout << "Testing " << extension << " for " << (isPoly ? "polydata" : "unstructured grid") << " mesh"
            << " in coordinate system: " << vtkMRMLStorageNode::GetCoordinateSystemTypeAsString(coordinateSystem) << std::endl;

  // Add storage node
  modelNode->AddDefaultStorageNode();
  vtkMRMLModelStorageNode* storageNode = vtkMRMLModelStorageNode::SafeDownCast(modelNode->GetStorageNode());
  CHECK_NOT_NULL(storageNode);
  storageNode->SetFileName(fileName.c_str());
  storageNode->SetCoordinateSystem(coordinateSystem);

  // Test writing
  CHECK_BOOL(storageNode->WriteData(modelNode.GetPointer()), true);

  // Clear data from model node
  modelNode->SetAndObservePolyData(nullptr);
  storageNode->SetCoordinateSystem(vtkMRMLStorageNode::CoordinateSystemLPS);
  CHECK_NULL(modelNode->GetMesh());

  // Test reading
  CHECK_BOOL(storageNode->ReadData(modelNode.GetPointer()), true);
  vtkPointSet* mesh2 = modelNode->GetMesh();
  CHECK_NOT_NULL(mesh2);
  CHECK_INT(mesh2->GetNumberOfPoints(), numberOfPoints);
  if (cellsMayBeSubdivided)
  {
    // cells may be subdivided when written to file, so we don't check for strict equality
    // but check if we have at least that many as in the original mesh
    CHECK_BOOL(mesh2->GetNumberOfCells() >= numberOfCells, true);
  }
  else
  {
    CHECK_INT(mesh2->GetNumberOfCells(), numberOfCells);
  }

  // Check extents to make sure there is no mirroring of the model due to coordinate system mismatch
  double originalBounds[6] = { 0.0 };
  mesh->GetBounds(originalBounds);
  double restoredBounds[6] = { 0.0 };
  mesh2->GetBounds(restoredBounds);
  for (int boundsIndex = 0; boundsIndex < 6; boundsIndex++)
  {
    CHECK_DOUBLE_TOLERANCE(restoredBounds[boundsIndex], originalBounds[boundsIndex], 1e-3);
  }

  // Coordinate system after writing must be the one that was requested
  CHECK_INT(storageNode->GetCoordinateSystem(), coordinateSystem);

  // Check if coordinate system hint is overridden by coordinate system specified in file
  int someDifferentCoordinateSystem =
    coordinateSystem == vtkMRMLStorageNode::CoordinateSystemRAS ? vtkMRMLStorageNode::CoordinateSystemLPS : vtkMRMLStorageNode::CoordinateSystemRAS;
  storageNode->SetCoordinateSystem(someDifferentCoordinateSystem);
  CHECK_BOOL(storageNode->ReadData(modelNode.GetPointer()), true);
  CHECK_INT(storageNode->GetCoordinateSystem(), coordinateSystem);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadOBJWithCoordinateUnitComment(vtkMRMLScene* scene, const char* comment, double expectedScale)
{
  std::cout << "Testing .obj with header comment: " << comment << std::endl;

  const double originalPoints[3][3] = { { 0.001, 0.002, 0.003 }, { 0.004, 0.005, 0.006 }, { 0.007, 0.008, 0.009 } };

  std::string fileName = std::string(scene->GetRootDirectory()) + std::string("/vtkMRMLModelNodeTest1UnitComment.obj");
  std::ofstream outputFile(fileName.c_str());
  CHECK_BOOL(outputFile.is_open(), true);
  outputFile << comment << "\n";
  // Specify RAS coordinate system in the file header so that the loaded point coordinates
  // are not flipped by RAS/LPS conversion.
  outputFile << "# SPACE=RAS\n";
  for (int pointIndex = 0; pointIndex < 3; pointIndex++)
  {
    outputFile << "v " << originalPoints[pointIndex][0] << " " << originalPoints[pointIndex][1] << " " << originalPoints[pointIndex][2] << "\n";
  }
  outputFile << "f 1 2 3\n";
  outputFile.close();

  // Add model node with storage node
  vtkNew<vtkMRMLModelNode> modelNode;
  CHECK_NOT_NULL(scene->AddNode(modelNode));
  modelNode->AddDefaultStorageNode();
  vtkMRMLModelStorageNode* storageNode = vtkMRMLModelStorageNode::SafeDownCast(modelNode->GetStorageNode());
  CHECK_NOT_NULL(storageNode);
  storageNode->SetFileName(fileName.c_str());

  // Test reading
  CHECK_BOOL(storageNode->ReadData(modelNode), true);
  vtkPointSet* mesh = modelNode->GetMesh();
  CHECK_NOT_NULL(mesh);
  CHECK_INT(mesh->GetNumberOfPoints(), 3);
  for (int pointIndex = 0; pointIndex < 3; pointIndex++)
  {
    double coordinate[3] = { 0.0, 0.0, 0.0 };
    mesh->GetPoint(pointIndex, coordinate);
    for (int componentIndex = 0; componentIndex < 3; componentIndex++)
    {
      CHECK_DOUBLE_TOLERANCE(coordinate[componentIndex], originalPoints[pointIndex][componentIndex] * expectedScale, 1e-4);
    }
  }

  scene->RemoveNode(modelNode);
  return EXIT_SUCCESS;
}
