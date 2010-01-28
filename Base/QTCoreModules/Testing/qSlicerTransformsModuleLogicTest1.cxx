/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// QTCoreModule includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerTransformLogic.h" 
#include "qSlicerTransformsModule.h"

// MRML includes
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// std includes
#include <stdlib.h>

#include "TestingMacros.h"

int qSlicerTransformsModuleLogicTest1(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Missing arguments" << std::endl;
    return EXIT_FAILURE;
    }

  int argc2 = argc - 1;
  char ** argv2 = argv + 1;

  qSlicerCoreApplication app( argc2, argv2 );

  qSlicerCoreApplication * aptr = app.application();

  if( aptr != (&app) )
    {
    std::cerr << "Problem with the application() singleton" << std::endl;
    return EXIT_FAILURE;
    }

  aptr->initialize();

  qSlicerTransformsModule * transformModule = new qSlicerTransformsModule;
  transformModule->initialize(aptr->appLogic());
  transformModule->setMRMLScene(aptr->mrmlScene());

  vtkSlicerLogic * moduleLogic = transformModule->logic();
  vtkSlicerTransformLogic* transformModuleLogic = vtkSlicerTransformLogic::SafeDownCast(moduleLogic);
  if (!transformModuleLogic)
    {
    std::cerr << "The transform module didn't create a transform module logic." << std::endl;
    return EXIT_FAILURE;
    }
  
  /* protected, maybe it won't be protected later... :-P
  if (transformModuleLogic->mrmlScene())
    {
    std::cerr << "A MRML Scene must be set to go further." << std::endl;
    return EXIT_FAILURE;
    }
  */
  
  if (argc2 < 1) 
    {
    std::cerr << "Missing transform file name." << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLTransformNode* transform = transformModuleLogic->AddTransform(argv2[0], transformModule->mrmlScene());
  if (transform == 0)
    {
    std::cerr << "Could not read transform file: " << argv2[0] << std::endl;
    return EXIT_FAILURE;
    }

  delete transformModule;

  return EXIT_SUCCESS;
}

