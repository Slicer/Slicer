/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// SlicerQt includes
#include "qSlicerPythonManager.h"
#include "qSlicerApplication.h"
#include "qSlicerBaseQTGUIPythonQtDecorators.h"

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
  QStringList paths;  
  paths << Superclass::pythonPaths();
  
  return paths; 
}

//-----------------------------------------------------------------------------
void qSlicerPythonManager::preInitialization()
{
  Superclass::preInitialization();

  // Register decorators
  this->registerPythonQtDecorator(new qSlicerBaseQTGUIPythonQtDecorators(this));
  
  qSlicerApplication* app = qSlicerApplication::application();
  
  // Evaluate application script
  this->executeFile(app->slicerHome() + "/bin/Python/slicer/slicerqt.py");
}
