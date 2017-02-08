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

} // end namespace

//-----------------------------------------------------------------------------
int vtkSlicerTransformLogicTest2(int argc, char * argv [])
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
  vtkSmartPointer<vtkMRMLModelNode> singleChild = LoadModelInScene(argv[1], scene);

  vtkSmartPointer<vtkMRMLModelNode> child1 = LoadModelInScene(argv[1], scene);
  vtkSmartPointer<vtkMRMLModelNode> child2 = LoadModelInScene(argv[1], scene);

  vtkNew<vtkMRMLTransformNode> childTransform;
  scene->AddNode(childTransform.GetPointer());
  vtkSmartPointer<vtkMRMLModelNode> grandChild1 = LoadModelInScene(argv[1], scene);
  vtkSmartPointer<vtkMRMLModelNode> grandChild2 = LoadModelInScene(argv[1], scene);
  vtkSmartPointer<vtkMRMLModelNode> child3 = LoadModelInScene(argv[1], scene);

  // Transform with no descendant
  vtkNew<vtkMRMLTransformNode> childlessTransform;
  scene->AddNode(childlessTransform.GetPointer());

  // Transform with one child:
  // oneChildTransform -> singleChild
  vtkNew<vtkMRMLTransformNode> oneChildTransform;
  scene->AddNode(oneChildTransform.GetPointer());
  singleChild->SetAndObserveTransformNodeID(oneChildTransform->GetID());

  // Transform with two children:
  // twoChildrenTransform -> child1
  //                      -> child2
  vtkNew<vtkMRMLTransformNode> twoChildrenTransform;
  scene->AddNode(twoChildrenTransform.GetPointer());
  child1->SetAndObserveTransformNodeID(twoChildrenTransform->GetID());
  child2->SetAndObserveTransformNodeID(twoChildrenTransform->GetID());

  // Transform tree:
  // transformTree -> childTransform -> grandChild1
  //                                 -> grandChild2
  //               -> child3
  vtkNew<vtkMRMLTransformNode> transformTree;
  scene->AddNode(transformTree.GetPointer());
  childTransform->SetAndObserveTransformNodeID(transformTree->GetID());
  child3->SetAndObserveTransformNodeID(transformTree->GetID());
  grandChild1->SetAndObserveTransformNodeID(childTransform->GetID());
  grandChild2->SetAndObserveTransformNodeID(childTransform->GetID());

  vtkNew<vtkMRMLTransformNode> notInSceneTransform;

  // Test childlessTransform
  std::vector<vtkMRMLDisplayableNode*> results;
  vtkSlicerTransformLogic::GetTransformedNodes(
    scene, childlessTransform.GetPointer(), results);
  if (results.size() != 0)
    {
    std::cout << "Error, expected results.size() == 0, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Test oneChildTransform
  vtkSlicerTransformLogic::GetTransformedNodes(
    scene, oneChildTransform.GetPointer(), results);
  if (results.size() != 1)
    {
    std::cout << "Error, expected results.size() == 1, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[0]->GetID(), singleChild->GetID()) != 0)
    {
    std::cout << "Error, expected results[0] == singleChild " << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Test twoChildrenTransform
  vtkSlicerTransformLogic::GetTransformedNodes(
    scene, twoChildrenTransform.GetPointer(), results);
  if (results.size() != 2)
    {
    std::cout << "Error, expected results.size() == 2, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[0]->GetID(), child1->GetID()) != 0)
    {
    std::cout << "Error, expected results[0] == child1 " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[1]->GetID(), child2->GetID()) != 0)
    {
    std::cout << "Error, expected results[1] == child2 " << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Try node not in the scene
  vtkSlicerTransformLogic::GetTransformedNodes(
    scene, notInSceneTransform.GetPointer(), results);
  if (results.size() != 0)
    {
    std::cout << "Error, expected results.size() == 0, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Test transform tree
  // Not recursively first
  vtkSlicerTransformLogic::GetTransformedNodes(
    scene, transformTree.GetPointer(), results, false);
  if (results.size() != 2)
    {
    std::cout << "Error, expected results.size() == 2, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[0]->GetID(), childTransform->GetID()) != 0)
    {
    std::cout << "Error, expected results[0] == childTransform " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[1]->GetID(), child3->GetID()) != 0)
    {
    std::cout << "Error, expected results[1] == child3 " << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Now recursively
  vtkSlicerTransformLogic::GetTransformedNodes(
    scene, transformTree.GetPointer(), results);
  if (results.size() != 4)
    {
    std::cout << "Error, expected results.size() == 4, got "
      << results.size() << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[0]->GetID(), childTransform->GetID()) != 0)
    {
    std::cout << "Error, expected results[0] == childTransform " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[1]->GetID(), grandChild1->GetID()) != 0)
    {
    std::cout << "Error, expected results[1] == grandChild1 " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[2]->GetID(), grandChild2->GetID()) != 0)
    {
    std::cout << "Error, expected results[2] == grandChild2 " << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(results[3]->GetID(), child3->GetID()) != 0)
    {
    std::cout << "Error, expected results[3] == child3 " << std::endl;
    return EXIT_FAILURE;
    }
  results.clear();

  // Clean-up
  scene->Delete();

  return EXIT_SUCCESS;
}
