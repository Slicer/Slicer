/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// SlicerQT includes
#include "qSlicerCoreApplication.h"
#include "qSlicerTransformsModule.h"
#include "qSlicerCoreCommandOptions.h"

// STD includes
#include <stdlib.h>

#include "TestingMacros.h"

int qSlicerTransformsModuleTest1(int argc, char * argv [] )
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

  QSettings * settings = app.settings();
  if( settings == 0 )
    {
    std::cerr << "Problem with settings()" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerCoreCommandOptions * coreCommandOptions = new qSlicerCoreCommandOptions(settings);

  app.setCoreCommandOptions( coreCommandOptions );

  qSlicerCoreCommandOptions * coreCommandOptions2 = app.coreCommandOptions();

  if( coreCommandOptions2 != coreCommandOptions )
    {
    std::cerr << "Problem with setCoreCommandOptions()/coreCommandOptions()" << std::endl;
    return EXIT_FAILURE;
    }

  bool exitWhenDone = false;
  aptr->initialize(exitWhenDone);
  if (exitWhenDone == true)
    {
    std::cerr << "Problem with the application::initialize function" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerTransformsModule * transformsModule = new qSlicerTransformsModule;
  transformsModule->initialize(aptr->appLogic());

  if (transformsModule->logic() != transformsModule->logic())
    {
    std::cerr << "The logic must be always the same." << std::endl;
    return EXIT_FAILURE;
    }

  if (transformsModule->widgetRepresentation() != transformsModule->widgetRepresentation())
    {
    std::cerr << "The logic must be always the same." << std::endl;
    return EXIT_FAILURE;
    }
  delete transformsModule;

  return EXIT_SUCCESS;
}

