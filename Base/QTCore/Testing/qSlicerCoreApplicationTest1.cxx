/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h" 
#include "qSlicerCoreIOManager.h" 

#include "vtkMRMLScene.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSmartPointer.h"

#include <stdlib.h>

#include "TestingMacros.h"

int qSlicerCoreApplicationTest1(int argc, char * argv [] )
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

  qSlicerCoreIOManager * coreIOManager = new qSlicerCoreIOManager;

  app.setCoreIOManager( coreIOManager );

  qSlicerCoreIOManager * coreIOManager2 = app.coreIOManager();

  if( coreIOManager2 != coreIOManager )
    {
    std::cerr << "Problem with setIOManager()/ioManager()" << std::endl;
    return EXIT_FAILURE;
    }


  bool exitWhenDone = false;
  app.initialize(exitWhenDone);
  if (exitWhenDone == true)
    {
    std::cerr << "Problem with the application::initialize function" << std::endl;
    return EXIT_FAILURE;
    }

  // Since initialize has been called, the module manager should be available
  qSlicerModuleManager * moduleManager1 = app.moduleManager();
  
  if( !moduleManager1 )
    {
    std::cerr << "Problem with moduleManager()" << std::endl;
    return EXIT_FAILURE;
    }

  app.setModuleManager(0);

  moduleManager1 = app.moduleManager();

  if( moduleManager1 )
    {
    std::cerr << "Problem with moduleManager()" << std::endl;
    return EXIT_FAILURE;
    }
  
  qSlicerModuleManager * moduleManager = new qSlicerModuleManager;

  app.setModuleManager( moduleManager );

  qSlicerModuleManager * moduleManager2 = app.moduleManager();

  if( moduleManager2 != moduleManager )
    {
    std::cerr << "Problem with setModuleManager()/moduleManager()" << std::endl;
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


  vtkSlicerApplicationLogic * logic1 = app.appLogic();

  if( logic1 == NULL )
    {
    std::cerr << "Error in appLogic() " << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer< vtkSlicerApplicationLogic > logic = 
    vtkSmartPointer< vtkSlicerApplicationLogic >::New();

  app.setAppLogic( logic );

  vtkSlicerApplicationLogic * logic2 = app.appLogic();

  if( logic2 != logic )
    {
    std::cerr << "Error in setAppLogic()/appLogic() " << std::endl;
    return EXIT_FAILURE;
    }


  vtkMRMLScene * scene1 = app.mrmlScene();

  if( scene1 == NULL )
    {
    std::cerr << "Error in mrmlScene() " << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer< vtkMRMLScene > scene = vtkSmartPointer< vtkMRMLScene >::New();

  app.setMRMLScene( scene );

  vtkMRMLScene * scene2 = app.mrmlScene();

  if( scene2 != scene )
    {
    std::cerr << "Error in setMRMLScene()/mrmlScene() " << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "TEST PASSED !" << std::endl;

  return EXIT_SUCCESS;
}

