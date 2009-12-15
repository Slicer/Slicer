/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactory.h"

#include <stdlib.h>
#include <iostream>

int qSlicerModuleManagerTest1(int argc, char * argv [] )
{
  qSlicerModuleManager moduleManager;

  qSlicerModuleManager * modulePtr = moduleManager.instance();

  if( modulePtr != & moduleManager )
    {
    std::cerr << "Error in instance() method" << std::endl;
    return EXIT_FAILURE;
    }

  moduleManager.printAdditionalInfo();

  qSlicerModuleFactory * factory = moduleManager.factory();

  if( factory == NULL )
    {
    std::cerr << "Error in factory()" << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleName;

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

  qSlicerAbstractModule * module = moduleManager.getModule( moduleName );
  
  if( module == NULL )
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

