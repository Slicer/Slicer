/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// GUI includes
#include "vtkSlicerModelsGUI.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelHierarchyNode.h>

// STD includes
#include <stdlib.h>

#include "TestingMacros.h"

int vtkSlicerModelsGUITest1(int argc, char * argv [] )
{
  vtkSlicerModelsGUI* modelsGUI = vtkSlicerModelsGUI::New();
  
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  modelsGUI->SetMRMLScene(mrmlScene);

  vtkMRMLModelHierarchyNode *node = vtkMRMLModelHierarchyNode::New();
  node->SetHideFromEditors(0);
  node->SetSelectable(1);
  mrmlScene->AddNode(node);
  node->Delete();

  vtkMRMLModelDisplayNode *dnode = vtkMRMLModelDisplayNode::New();
  mrmlScene->AddNode(dnode);
  dnode->Delete();

  node->SetAndObserveDisplayNodeID(dnode->GetID());

  modelsGUI->Delete();

  mrmlScene->Delete();

  return EXIT_SUCCESS;
}

