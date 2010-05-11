/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// QT includes
#include <QString>
#include <QStringList>

// SlicerQt includes
#include <qSlicerLoadableModuleFactory.h>

// STD includes
#include <cstdlib>
#include <iostream>

#include "TestingMacros.h"

int qSlicerLoadableModuleFactoryTest1(int, char * [] )
{
  QStringList libraryNames;
  libraryNames << "ThresholdLib.dll"
               << "Threshold.dll"
               << "libThreshold.so"
               << "libThreshold.so.2.3"
               << "libThreshold.dylib";
             
  QString expectedModuleName = "threshold";

  foreach (const QString& libraryName, libraryNames)
    {
    QString moduleName = qSlicerLoadableModuleFactory::extractModuleName(libraryName);
    if (moduleName != expectedModuleName)
      {
      std::cerr << __LINE__ << " - Error in  extractModuleName()" << std::endl
                            << "moduleName = " << qPrintable(moduleName) << std::endl
                            << "expectedModuleName = " << qPrintable(expectedModuleName) << std::endl;
      return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}

