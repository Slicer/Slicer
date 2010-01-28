/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCLIModuleFactoryHelper.h"

// SlicerQT includes
#include "qSlicerCoreApplication.h"

// For:
//  - Slicer3_INSTALL_PLUGINS_BIN_DIR
#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
const QStringList qSlicerCLIModuleFactoryHelper::modulePaths()
{
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  Q_ASSERT(app);

  // slicerHome shouldn't be empty
  Q_ASSERT(!app->slicerHome().isEmpty());
  
  QStringList defaultCmdLineModulePaths;

  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  defaultCmdLineModulePaths << app->slicerHome() + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR;
  if (!app->intDir().isEmpty())
     {
     defaultCmdLineModulePaths << app->slicerHome() + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR + "/" + app->intDir();
     }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QStringList cmdLineModulePaths = /*userModulePaths + PathSep +*/ defaultCmdLineModulePaths;
  foreach(const QString& path, cmdLineModulePaths)
    {
    app->addLibraryPath(path);
    }
    
  //qDebug() << "cmdLineModulePaths:" << cmdLineModulePaths;
  return cmdLineModulePaths; 
}
