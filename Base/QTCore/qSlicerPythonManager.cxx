/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// SlicerQt includes
#include "qSlicerUtils.h"
#include "qSlicerPythonManager.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerBaseQTBasePythonQtDecorators.h"

#include "vtkSlicerConfigure.h" // For VTK_DIR

//-----------------------------------------------------------------------------
qSlicerPythonManager::qSlicerPythonManager(QObject* _parent) : Superclass(_parent)
{

}

//-----------------------------------------------------------------------------
qSlicerPythonManager::~qSlicerPythonManager()
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerPythonManager::pythonPaths()
{
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  
  QStringList paths;  
  paths << Superclass::pythonPaths();
  paths << app->slicerHome() + "/bin/" + app->intDir();
  paths << app->slicerHome() + "/bin/Python";

  QString executableExtension = qSlicerUtils::executableExtension();
  if (!app->isInstalled())
    {
    paths << qSlicerUtils::searchTargetInIntDir(QLatin1String(VTK_DIR)+"/bin",
                                                QString("vtk%1").arg(executableExtension));
    }
  else
    {
    paths << app->slicerHome() + QString("/lib/vtk-%1.%2").arg(VTK_MAJOR_VERSION).
                                                           arg(VTK_MINOR_VERSION);
    paths << app->slicerHome() + "/lib/MRML";
    paths << app->slicerHome() + "/lib/vtkTeem";
    paths << app->slicerHome() + "/lib/FreeSurfer";
    paths << app->slicerHome() + "/lib/RemoteIO";
    }
  
  return paths; 
}

//-----------------------------------------------------------------------------
void qSlicerPythonManager::preInitialization()
{
  Superclass::preInitialization();

  // Register decorators
  this->registerPythonQtDecorator(new qSlicerBaseQTBasePythonQtDecorators(this));

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  // Add object to python interpreter context
  this->addObjectToPythonMain("_qSlicerCoreApplicationInstance", app);

  // Evaluate application script
  this->executeFile(app->slicerHome() + "/bin/Python/slicer/slicerqt.py");
}
