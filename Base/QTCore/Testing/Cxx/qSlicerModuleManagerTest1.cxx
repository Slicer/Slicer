/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactory.h"

#include <cstdlib>
#include <iostream>

#include "vtkMRMLCoreTestingMacros.h"

int qSlicerModuleManagerTest1(int argc, char * argv [] )
{

  // By design, a ModuleManager should be instantiated only if a
  // qSlicerCoreApplication exists and has been initialized.
  // That we will be sure, an ApplicationLogic and a MRMLScene have also been instantiated
  // This enforced in the constructor of qSlicerModuleManager (using Q_ASSERTs)
  qSlicerCoreApplication app(argc, argv);
  app.initialize();

  qSlicerModuleManager moduleManager;

  moduleManager.factory()->registerCoreModules();
  moduleManager.factory()->instantiateCoreModules();

  moduleManager.printAdditionalInfo();

  qSlicerModuleFactory * factory = moduleManager.factory();

  if( factory == nullptr )
    {
    std::cerr << "Error in factory()" << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleName = "qSlicerTransformsModule";

  bool result0 = moduleManager.isLoaded( moduleName );

  if( result0 != false )
    {
    std::cerr << "Error in isLoaded() " << std::endl;
    return EXIT_FAILURE;
    }

  bool result1 = moduleManager.loadModule( moduleName );

  if( result1 == false )
    {
    std::cerr << "Error in loadModule() " << std::endl;
    return EXIT_FAILURE;
    }

  bool result2 = moduleManager.isLoaded( moduleName );

  if( result2 != true )
    {
    std::cerr << "Error in isLoaded() or loadModule() " << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerAbstractModule * module = moduleManager.module( moduleName );

  if( module == nullptr )
    {
    std::cerr << "Error in getModule() " << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleTitle = moduleManager.moduleTitle( moduleName );

  QString moduleName1 = moduleManager.moduleName( moduleTitle );

  if( moduleName != moduleName1 )
    {
    std::cerr << "Error in moduleName recovery" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Module Name = " << qPrintable( moduleName ) << std::endl;

  std::cout << "Module Title = " << qPrintable( moduleTitle ) << std::endl;


  bool result3 = moduleManager.unLoadModule( moduleName );

  if( result3 == false )
    {
    std::cerr << "Error in unLoadModule() " << std::endl;
    return EXIT_FAILURE;
    }

  bool result4 = moduleManager.isLoaded( moduleName );

  if( result4 != false )
    {
    std::cerr << "Error in isLoaded() or loadModule() " << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

