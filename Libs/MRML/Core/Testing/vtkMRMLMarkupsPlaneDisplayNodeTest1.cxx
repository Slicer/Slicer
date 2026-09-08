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

//----------------------------------------------------------------------------
int TestNormalDisplayPropertiesSceneRoundTrip();

//----------------------------------------------------------------------------
int vtkMRMLMarkupsPlaneDisplayNodeTest1(int, char*[])
{
  vtkNew<vtkMRMLMarkupsPlaneDisplayNode> node1;
  // Toggling scalar visibility on a display node without a markups node logs warnings
  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  TESTING_OUTPUT_ASSERT_WARNINGS(4);
  TESTING_OUTPUT_ASSERT_WARNINGS_END();

  CHECK_EXIT_SUCCESS(TestNormalDisplayPropertiesSceneRoundTrip());

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestNormalDisplayPropertiesSceneRoundTrip()
{
  vtkNew<vtkMRMLScene> scene;

  vtkMRMLMarkupsPlaneDisplayNode* displayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsPlaneDisplayNode"));
  CHECK_NOT_NULL(displayNode);

  displayNode->SetNormalVisibility(false);
  displayNode->SetNormalOpacity(0.25);

  // Write scene to XML string
  scene->SetSaveToXMLString(1);
  CHECK_BOOL(scene->Commit() != 0, true);
  std::string sceneXMLString = scene->GetSceneXMLString();

  // Read scene from XML string
  vtkNew<vtkMRMLScene> reloadedScene;
  reloadedScene->SetLoadFromXMLString(1);
  reloadedScene->SetSceneXMLString(sceneXMLString);
  CHECK_BOOL(reloadedScene->Import() != 0, true);

  vtkMRMLMarkupsPlaneDisplayNode* reloadedDisplayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(reloadedScene->GetFirstNodeByClass("vtkMRMLMarkupsPlaneDisplayNode"));
  CHECK_NOT_NULL(reloadedDisplayNode);
  CHECK_BOOL(reloadedDisplayNode->GetNormalVisibility(), false);
  CHECK_DOUBLE(reloadedDisplayNode->GetNormalOpacity(), 0.25);

  vtkNew<vtkMRMLMarkupsPlaneDisplayNode> copiedDisplayNode;
  copiedDisplayNode->CopyContent(reloadedDisplayNode);
  CHECK_BOOL(copiedDisplayNode->GetNormalVisibility(), false);
  CHECK_DOUBLE(copiedDisplayNode->GetNormalOpacity(), 0.25);

  return EXIT_SUCCESS;
}
