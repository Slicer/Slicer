/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// Logic includes
#include "vtkSlicerTransformLogic.h"

// MRML includes

// VTK includes

// STD includes


int vtkSlicerTransformLogicTest1(int argc, char * argv [])
{
  if(argc < 2)
    {
    std::cerr << "Missing transform file name." << std::endl;
    return EXIT_FAILURE;
    }
    
  vtkMRMLScene* scene = vtkMRMLScene::New();
  
  vtkSlicerTransformLogic* transformModuleLogic = vtkSlicerTransformLogic::New();
  transformModuleLogic->SetMRMLScene(scene);
  if (transformModuleLogic->GetMRMLScene() != scene)
    {
    std::cerr << "A MRML Scene must be set to go further." << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLTransformNode* transform = transformModuleLogic->AddTransform(argv[1], scene);
  if (transform == 0)
    {
    std::cerr << "Could not read transform file: " << argv[1] << std::endl;
    return EXIT_FAILURE;
    }

  transformModuleLogic->Delete();
  scene->Delete();

  return EXIT_SUCCESS;
}

