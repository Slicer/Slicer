/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// Logic includes
#include "vtkSlicerTransformLogic.h"

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkPolyDataReader.h>

namespace
{
//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMRMLModelNode> LoadModelInScene
(const char* filepath, vtkMRMLScene* scene)
{
  vtkNew<vtkPolyDataReader> reader;
  reader->SetFileName(filepath);
  reader->Update();

  assert(reader->GetOutput());
  vtkSmartPointer<vtkMRMLModelNode> model =
    vtkSmartPointer<vtkMRMLModelNode>::New();
  model->SetAndObservePolyData(reader->GetOutput());
  scene->AddNode(model);
  return model;
}

//-----------------------------------------------------------------------------
bool CompareBounds(double b[6], double e[6])
{
  bool success = true;
  for (int i = 0; i < 6; ++i)
    {
    if (fabs(b[i] - e[i]) > 1e-6)
      {
      success = false;
      }
    }
  if (!success)
    {
    std::cout << "Wrong bounds !" << std::endl;
    std::cout << "Expected: "
      << e[0] << " " << e[1] << " "
      << e[2] << " " << e[3] << " "
      << e[4] << " " << e[5] << std::endl;
    std::cout << "Got: "
      << b[0] << " " << b[1] << " "
      << b[2] << " " << b[3] << " "
      << b[4] << " " << b[5] << std::endl;
    }
  return success;
}
}// end namespace

//-----------------------------------------------------------------------------
int vtkSlicerTransformLogicTest3(int argc, char * argv [])
{
  if(argc < 2)
    {
    std::cerr << "Missing transform file name." << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Scene setup
  vtkMRMLScene* scene = vtkMRMLScene::New();

  // argv1 == cube.vtk
  vtkSmartPointer<vtkMRMLModelNode> cube = LoadModelInScene(argv[1], scene);
  vtkSmartPointer<vtkMRMLModelNode> tranformedCube = LoadModelInScene(argv[2], scene);

  std::vector<vtkMRMLDisplayableNode*> nodes;
  bool success = true;

  // Test bounds with no nodes
  double bounds[6] = {0.0, 0.0, 0.0, 0.0, 0.0};
  double expectedNoBounds[6] = {
    VTK_DOUBLE_MAX, VTK_DOUBLE_MIN,
    VTK_DOUBLE_MAX, VTK_DOUBLE_MIN,
    VTK_DOUBLE_MAX, VTK_DOUBLE_MIN
    };
  vtkSlicerTransformLogic::GetNodesBounds(nodes, bounds);
  // Also add RAS BOIUNDs
  success &= CompareBounds(bounds, expectedNoBounds);

  // Test bounds with 1 nodes
  nodes.push_back(cube.GetPointer());
  double expected1NodeBounds[6] = {-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
  vtkSlicerTransformLogic::GetNodesBounds(nodes, bounds);
  success &= CompareBounds(bounds, expected1NodeBounds);

  // Test bounds with transformed cube
  nodes.clear();
  nodes.push_back(tranformedCube.GetPointer());
  double expectedTransformedNodeBounds[6] = {
    9.13638973236, 10.8636102676,
    29.2609081268, 30.7390918732,
    -90.816947937, -89.183052063
    };
  vtkSlicerTransformLogic::GetNodesBounds(nodes, bounds);
  success &= CompareBounds(bounds, expectedTransformedNodeBounds);

  // Test bounds with transformed cube and the cube
  nodes.clear();
  nodes.push_back(cube.GetPointer());
  nodes.push_back(tranformedCube.GetPointer());
  double expectedBothNodeBounds[6] = {
    -0.5, 10.8636102676,
    -0.5, 30.7390918732,
    -90.816947937, 0.5
    };
  vtkSlicerTransformLogic::GetNodesBounds(nodes, bounds);
  success &= CompareBounds(bounds, expectedBothNodeBounds);

  // Clean-up
  scene->Delete();

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
