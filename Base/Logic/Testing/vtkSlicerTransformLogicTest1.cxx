/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// Logic includes
#include "vtkSlicerTransformLogic.h"

// MRML includes
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes
#include <stdlib.h>

#include "TestingMacros.h"

int vtkSlicerTransformLogicTest1(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Missing arguments" << std::endl;
    return EXIT_FAILURE;
    }

  int argc2 = argc - 1;
  char ** argv2 = argv + 1;
  vtkMRMLScene* scene = vtkMRMLScene::New();
  
  vtkSlicerTransformLogic* transformModuleLogic = vtkSlicerTransformLogic::New();
  transformModuleLogic->SetMRMLScene(scene);
  if (transformModuleLogic->GetMRMLScene() != scene)
    {
    std::cerr << "A MRML Scene must be set to go further." << std::endl;
    return EXIT_FAILURE;
    }
  
  if (argc2 < 1) 
    {
    std::cerr << "Missing transform file name." << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLTransformNode* transform = transformModuleLogic->AddTransform(argv2[0], scene);
  if (transform == 0)
    {
    std::cerr << "Could not read transform file: " << argv2[0] << std::endl;
    return EXIT_FAILURE;
    }

  transformModuleLogic->Delete();
  scene->Delete();

  return EXIT_SUCCESS;
}

