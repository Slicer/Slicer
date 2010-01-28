/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// QTCoreModule includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCamerasModuleLogic.h" 

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLCameraNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// std includes
#include <stdlib.h>

#include "TestingMacros.h"

int qSlicerCamerasModuleLogicTest1(int argc, char * argv [] )
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

  return EXIT_SUCCESS;
}

