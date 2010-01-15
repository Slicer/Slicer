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

int qSlicerModuleFactoryTest1(int, char * [] )
{
  qSlicerModuleFactory moduleFactory;

  moduleFactory.registerCoreModules();

  moduleFactory.printAdditionalInfo();

  QString moduleName = "qSlicerTransformsModule";
  
  std::cout << "Module Name = " << qPrintable( moduleName ) << std::endl;
  
  if (!moduleFactory.moduleTitle( moduleName ).isEmpty())
    {
    std::cerr << "A registered module can't have a valid title yet."
              << " Titles get set after the module get instantiated." << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerAbstractModule * abstractModule = moduleFactory.instantiateModule( moduleName );

  if( abstractModule == NULL )
    {
    std::cerr << "Error in instantiateModule()" << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleTitle = moduleFactory.moduleTitle( moduleName ); 

  std::cout << "Module Title = " << qPrintable( moduleTitle ) << std::endl;

  QString moduleName1 = moduleFactory.moduleName( moduleTitle ); 
  
  if( moduleName1 != moduleName )
    {
    std::cerr << "Error in moduleTitle() or moduleName()" << std::endl;
    std::cerr << "Module Name  = " << qPrintable( moduleName ) << std::endl;
    std::cerr << "Module Name1 = " << qPrintable( moduleName1 ) << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleTitle1 = moduleFactory.moduleTitle( moduleName ); 

  if( moduleTitle1 != moduleTitle )
    {
    std::cerr << "Error in getModuleTitle()" << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleTitle2 = abstractModule->title(); 
  
  if( moduleTitle2 != moduleTitle )
    {
    std::cerr << "Error in title():" << qPrintable(moduleTitle2) << std::endl;
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

  return EXIT_SUCCESS;
}

