/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerModuleFactory.h"

#include <stdlib.h>
#include <QString>
#include <QStringList>
#include <iostream>

#include "TestingMacros.h"

int qSlicerModuleFactoryTest1(int argc, char * argv [] )
{
  qSlicerModuleFactory moduleFactory;

  moduleFactory.registerCoreModules();

  moduleFactory.printAdditionalInfo();

  QString moduleName = "qSlicerTransformsModule";

  QString moduleTitle = moduleFactory.getModuleTitle( moduleName ); 

  std::cout << "Module Name = " << qPrintable( moduleName ) << std::endl;

  std::cout << "Module Title = " << qPrintable( moduleTitle ) << std::endl;

  QString moduleName1 = moduleFactory.getModuleName( moduleTitle ); 
  
  if( moduleName1 != moduleName )
    {
    std::cerr << "Error in getModuleTitle() or getModuleName()" << std::endl;
    std::cerr << "Module Name  = " << qPrintable( moduleName ) << std::endl;
    std::cerr << "Module Name1 = " << qPrintable( moduleName1 ) << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleTitle1 = moduleFactory.getModuleTitle( moduleName ); 

  if( moduleTitle1 != moduleTitle )
    {
    std::cerr << "Error in getModuleTitle()" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerAbstractModule * abstractModule = moduleFactory.instantiateModule( moduleName );

  if( abstractModule == NULL )
    {
    std::cerr << "Error in instantiateModule()" << std::endl;
    return EXIT_FAILURE;
    }

  moduleFactory.uninstantiateModule( moduleName );

  // Instantiate again
  abstractModule = moduleFactory.instantiateModule( moduleName );

  if( abstractModule == NULL )
    {
    std::cerr << "Error in instantiateModule()" << std::endl;
    return EXIT_FAILURE;
    }

  moduleFactory.uninstantiateAll();

  QStringList paths;  // FIXME: Initialize it from argv

  moduleFactory.setLoadableModuleSearchPaths( paths );

  QStringList loadableModulePaths = moduleFactory.loadableModuleSearchPaths();

  CHECK_FOR_VTK_MEMORY_LEAKS();

  return EXIT_SUCCESS;
}

