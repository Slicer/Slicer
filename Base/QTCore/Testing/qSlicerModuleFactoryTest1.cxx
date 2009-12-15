/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerModuleFactory.h"

#include <stdlib.h>
#include <QString>
#include <iostream>

int qSlicerModuleFactoryTest1(int argc, char * argv [] )
{
  qSlicerModuleFactory moduleFactory;

  moduleFactory.printAdditionalInfo();

  QString moduleName = "module";

  QString moduleTitle = moduleFactory.getModuleTitle( moduleName ); 

  QString moduleName1 = moduleFactory.getModuleName( moduleTitle ); 
  
  if( moduleName1 != moduleName )
    {
    std::cerr << "Error in getModuleTitle() or getModuleName()" << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleTitle1 = moduleFactory.getModuleTitle( moduleName ); 

  return EXIT_SUCCESS;
}

