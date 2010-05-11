/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageErode.h,v $
  Date:      $Date: 2006/01/06 17:56:40 $
  Version:   $Revision: 1.17 $

=========================================================================auto=*/

// QT includes
#include <QStringList>

// SlicerQt includes
#include <qSlicerUtils.h>

// STD includes
#include <cstdlib>
#include <iostream>

int qSlicerUtilsTest1(int, char * [] )
{
  // Test isExecutableName()
  
  QStringList executableNames;
  executableNames << "Threshold.bat" << "Threshold.com"
                  << "Threshold.sh" << "Threshold.csh"
                  << "Threshold.tcsh" << "Threshold.pl"
                  << "Threshold.py" << "Threshold.tcl"
                  << "Threshold.m" << "Threshold.exe";

  foreach(const QString& executableName, executableNames)
    {
    bool isExecutable = qSlicerUtils::isExecutableName(executableName);
    if (!isExecutable)
      {
      std::cerr << __LINE__ << " - Error in  isExecutableName()" << std::endl
                            << "[" << qPrintable(executableName)
                            << "] should be an executable" << std::endl;
      }
    }

  QStringList notExecutableNames;
  notExecutableNames << "Threshold.ini" << "Threshold.txt" << "Threshold";
  foreach(const QString& notExecutableName, notExecutableNames)
    {
    bool isExecutable = qSlicerUtils::isExecutableName(notExecutableName);
    if (isExecutable)
      {
      std::cerr << __LINE__ << " - Error in  isExecutableName()" << std::endl
                            << "[" << qPrintable(notExecutableName)
                            << "] should *NOT* be an executable" << std::endl;
      }
    }

  // Test executableExtension()
#ifdef _WIN32
  QString expectedExecutableExtension = ".exe";
#else
  QString expectedExecutableExtension = "";
#endif
  QString executableExtension = qSlicerUtils::executableExtension(); 
  if (executableExtension != expectedExecutableExtension)
    {
    std::cerr << __LINE__ << " - Error in  executableExtension()" << std::endl
                          << "executableExtension = " << qPrintable(executableExtension) << std::endl
                          << "expectedExecutableExtension = " << qPrintable(expectedExecutableExtension) << std::endl;
    return EXIT_FAILURE;
    }

  // Test extractModuleNameFromLibraryName()
  
  QStringList libraryNames;
  libraryNames << "ThresholdLib.dll"
               << "Threshold.dll"
               << "libThreshold.so"
               << "libThreshold.so.2.3"
               << "libThreshold.dylib"
               << "qSlicerThresholdModule.so"
               << "qSlicerThreshold.dylib";
             
  QString expectedModuleName = "threshold";

  foreach (const QString& libraryName, libraryNames)
    {
    QString moduleName = qSlicerUtils::extractModuleNameFromLibraryName(libraryName);
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

