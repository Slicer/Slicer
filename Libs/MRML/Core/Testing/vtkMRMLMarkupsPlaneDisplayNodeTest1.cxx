/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMarkupsPlaneDisplayNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <iostream>

// vtksys includes
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
int TestNormalDisplayPropertiesSceneRoundTrip(const char* tempDirectory);

//----------------------------------------------------------------------------
int vtkMRMLMarkupsPlaneDisplayNodeTest1(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Missing temporary directory argument" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkMRMLMarkupsPlaneDisplayNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  CHECK_EXIT_SUCCESS(TestNormalDisplayPropertiesSceneRoundTrip(argv[1]));

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestNormalDisplayPropertiesSceneRoundTrip(const char* tempDirectory)
{
  std::string sceneFilePath = std::string(tempDirectory) + "/vtkMRMLMarkupsPlaneDisplayNodeTest1.mrml";
  vtksys::SystemTools::RemoveFile(sceneFilePath);

  vtkNew<vtkMRMLScene> scene;
  scene->SetRootDirectory(tempDirectory);
  scene->SetURL(sceneFilePath.c_str());

  vtkMRMLMarkupsPlaneDisplayNode* displayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsPlaneDisplayNode"));
  CHECK_NOT_NULL(displayNode);

  displayNode->SetNormalVisibility(false);
  displayNode->SetNormalOpacity(0.25);

  // Scene persistence is the path used by MRB save/load for display-node
  // properties.
  CHECK_BOOL(scene->Commit() != 0, true);

  vtkNew<vtkMRMLScene> reloadedScene;
  reloadedScene->SetURL(sceneFilePath.c_str());
  reloadedScene->SetRootDirectory(tempDirectory);
  CHECK_BOOL(reloadedScene->Import() != 0, true);

  vtkMRMLMarkupsPlaneDisplayNode* reloadedDisplayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(reloadedScene->GetFirstNodeByClass("vtkMRMLMarkupsPlaneDisplayNode"));
  CHECK_NOT_NULL(reloadedDisplayNode);
  CHECK_BOOL(reloadedDisplayNode->GetNormalVisibility(), false);
  CHECK_DOUBLE(reloadedDisplayNode->GetNormalOpacity(), 0.25);

  vtkNew<vtkMRMLMarkupsPlaneDisplayNode> copiedDisplayNode;
  copiedDisplayNode->CopyContent(reloadedDisplayNode);
  CHECK_BOOL(copiedDisplayNode->GetNormalVisibility(), false);
  CHECK_DOUBLE(copiedDisplayNode->GetNormalOpacity(), 0.25);

  vtksys::SystemTools::RemoveFile(sceneFilePath);

  return EXIT_SUCCESS;
}
