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
#include <qSlicerCLIExecutableModuleFactory.h>

// STD includes
#include <cstdlib>
#include <iostream>

#include "TestingMacros.h"

int qSlicerCLIExecutableModuleFactoryTest1(int, char * [] )
{
  QStringList executableNames;
  executableNames << "Threshold.exe"
                  << "threshold.exe"
                  << "Threshold";
             
  QString expectedModuleName = "threshold";

  foreach (const QString& executableName, executableNames)
    {
    QString moduleName = qSlicerCLIExecutableModuleFactory::extractModuleName(executableName);
    if (moduleName != expectedModuleName)
      {
      std::cerr << __LINE__ << " - Error in  extractModuleName()" << std::endl
                            << "executableName = " << qPrintable(executableName) << std::endl
                            << "moduleName = " << qPrintable(moduleName) << std::endl
                            << "expectedModuleName = " << qPrintable(expectedModuleName) << std::endl;
      return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}
