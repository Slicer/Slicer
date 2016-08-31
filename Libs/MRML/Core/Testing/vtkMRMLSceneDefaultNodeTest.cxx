/*=auto=========================================================================

  Portions (c) Copyright 2016 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkMRMLCoreTestingMacros.h"

//------------------------------------------------------------------------------
int vtkMRMLSceneDefaultNodeTest(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene1;

  // Test default node setting in scene
  vtkNew<vtkMRMLModelStorageNode> defaultStorageNode;
  defaultStorageNode->SetDefaultWriteFileExtension("stl");
  scene1->AddDefaultNode(defaultStorageNode.GetPointer());
  CHECK_POINTER(scene1->GetDefaultNodeByClass("vtkMRMLModelStorageNode"), defaultStorageNode.GetPointer());

  // Test if storage node created by AddDefaultStorage node
  // is overridden by default storage node set in the scene
  vtkNew<vtkMRMLModelNode> modelNode;
  scene1->AddNode(modelNode.GetPointer());
  CHECK_BOOL(modelNode->AddDefaultStorageNode(), true);
  vtkMRMLStorageNode* storageNode = modelNode->GetStorageNode();
  CHECK_NOT_NULL(storageNode);
  CHECK_STRING(storageNode->GetDefaultWriteFileExtension(), "stl");

  // Test if default node can be modified
  vtkMRMLModelStorageNode* defaultStorageNode2 = vtkMRMLModelStorageNode::SafeDownCast(scene1->GetDefaultNodeByClass("vtkMRMLModelStorageNode"));
  CHECK_NOT_NULL(defaultStorageNode2);
  defaultStorageNode2->SetDefaultWriteFileExtension("vtp");
  vtkNew<vtkMRMLModelNode> modelNode2;
  scene1->AddNode(modelNode2.GetPointer());
  CHECK_BOOL(modelNode2->AddDefaultStorageNode(), true);
  vtkMRMLStorageNode* storageNode2 = modelNode2->GetStorageNode();
  CHECK_NOT_NULL(storageNode2);
  CHECK_STRING(storageNode2->GetDefaultWriteFileExtension(), "vtp");

  return EXIT_SUCCESS;
}
