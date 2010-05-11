/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// GUI includes
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerApplicationGUI.h"

// MRML includes
#include <vtkMRMLScene.h>

// STD includes
#include <stdlib.h>

#include "TestingMacros.h"

int vtkSlicerModuleGUITest1(int vtkNotUsed(argc), char * vtkNotUsed(argv) [] )
{
  vtkSlicerModuleGUI* moduleGUI = vtkSlicerModuleGUI::New();

  EXERCISE_BASIC_OBJECT_METHODS( moduleGUI );
  
  TEST_SET_GET_STRING(moduleGUI, Category);
  TEST_SET_GET_STRING(moduleGUI, ModuleName);
  
  moduleGUI->SetIndex(10);
  int index = moduleGUI->GetIndex();
  if (index != 10)
    {
    std::cout << "Error getting back index, 10 != " << index << std::endl;
    return EXIT_FAILURE;
    }
  
  //vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::New();
  //moduleGUI->SetApplicationGUI(appGUI);
  
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  moduleGUI->SetAndObserveMRMLScene(mrmlScene);
  
  moduleGUI->SetGUIName("testing gui");
  moduleGUI->GetUIPanel()->SetName("ui panel");
  moduleGUI->GetUIPanel()->Create( );

  moduleGUI->BuildGUI ( );
  moduleGUI->AddGUIObservers ( );
  moduleGUI->Init();

          
  //appGUI->Delete();
  moduleGUI->TearDownGUI();
  moduleGUI->RemoveGUIObservers();

  moduleGUI->Delete();

  mrmlScene->Delete();

  return EXIT_SUCCESS;
}

