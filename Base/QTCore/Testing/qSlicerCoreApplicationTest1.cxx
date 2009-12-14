/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCoreApplication.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerApplicationLogic.h"
#include "qSlicerModuleManager.h" 
#include "qSlicerCoreIOManager.h" 

#include <stdlib.h>

int qSlicerCoreApplicationTest1(int argc, char * argv [] )
{
  int argc2 = argc - 1;
  char ** argv2 = argv + 1;

  qSlicerCoreApplication app( argc2, argv2 );

  qSlicerCoreApplication * aptr = app.application();

  if( aptr != (&app) )
    {
    std::cerr << "Problem with the application() singleton" << std::endl;
    return EXIT_FAILURE;
    }

  app.initialize();

  qSlicerModuleManager * moduleManager = new qSlicerModuleManager;

  app.setModuleManager( moduleManager );

  qSlicerModuleManager * moduleManager2 = app.moduleManager();

  if( moduleManager2 != moduleManager )
    {
    std::cerr << "Problem with setModuleManager()/moduleManager()" << std::endl;
    return EXIT_FAILURE;
    }


  qSlicerCoreIOManager * coreIOManager = new qSlicerCoreIOManager;

  app.setCoreIOManager( coreIOManager );

  qSlicerCoreIOManager * coreIOManager2 = app.coreIOManager();

  if( coreIOManager2 != coreIOManager )
    {
    std::cerr << "Problem with setIOManager()/ioManager()" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLScene * scene = vtkMRMLScene::New();

  app.setMRMLScene( scene );

  vtkMRMLScene * scene1 = app.mrmlScene();

  if( scene1 != scene )
    {
    std::cerr << "Error in setMRMLScene()/mrmlScene() " << std::endl;
    return EXIT_FAILURE;
    }

  vtkSlicerApplicationLogic * logic = vtkSlicerApplicationLogic::New();

  app.setAppLogic( logic );

  vtkSlicerApplicationLogic * logic1 = app.appLogic();

  if( logic1 != logic )
    {
    std::cerr << "Error in setAppLogic()/appLogic() " << std::endl;
    return EXIT_FAILURE;
    }

  QString homeDirectory = app.slicerHome();

  std::cout << "Slicer Home Directory = " << qPrintable( homeDirectory ) << std::endl;

  QString newHome = homeDirectory;

  app.setSlicerHome( newHome );

  QString newHome1 = app.slicerHome();

  if( newHome1 != newHome )
    {
    std::cerr << "Error in setSlicerHome()/slicerHome() " << std::endl;
    return EXIT_FAILURE;
    }

  app.setSlicerHome( homeDirectory );

  return EXIT_SUCCESS;
}

