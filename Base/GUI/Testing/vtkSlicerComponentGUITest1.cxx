/*=auto=========================================================================

  Portions (c) Copyright 2010 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// GUI includes
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationGUI.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>

// STD includes
#include <stdlib.h>

#include "TestingMacros.h"

int vtkSlicerComponentGUITest1(int vtkNotUsed(argc), char * vtkNotUsed(argv) [] )
{
  vtkSlicerComponentGUI* gui = vtkSlicerComponentGUI::New();

  EXERCISE_BASIC_OBJECT_METHODS( gui );
  
  TEST_SET_GET_STRING(gui, GUIName);

  
  //vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::New();
  //gui->SetApplicationGUI(appGUI);
  
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  gui->SetAndObserveMRMLScene(mrmlScene);

  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::New();
  vtkMRMLModelNode *oldNode = vtkMRMLModelNode::New();
  mrmlScene->AddNode(modelNode);
  mrmlScene->AddNode(oldNode);
  modelNode->Delete();
  oldNode->Delete();
  
  
  gui->SetGUIName("testing gui");

  gui->BuildGUI ( );
  gui->AddGUIObservers ( );

          
  //appGUI->Delete();
  gui->TearDownGUI();
  gui->RemoveGUIObservers();

  gui->Delete();

  mrmlScene->Delete();

  return EXIT_SUCCESS;
}

