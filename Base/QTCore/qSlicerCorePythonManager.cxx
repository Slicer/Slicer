/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// CTK includes
#include <ctkLogger.h>

// PythonQt includes
#include <PythonQt.h>

// SlicerQt includes
#include "qSlicerUtils.h"
#include "qSlicerCorePythonManager.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerBaseQTCorePythonQtDecorators.h"

// VTK includes
#include <vtkPythonUtil.h>

#include "vtkSlicerConfigure.h" // For VTK_DIR

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtcore.qSlicerCorePythonManager");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
qSlicerCorePythonManager::qSlicerCorePythonManager(QObject* _parent) : Superclass(_parent)
{

}

//-----------------------------------------------------------------------------
qSlicerCorePythonManager::~qSlicerCorePythonManager()
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerCorePythonManager::pythonPaths()
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
void qSlicerCorePythonManager::preInitialization()
{
  Superclass::preInitialization();

  // Register decorators
  this->registerPythonQtDecorator(new qSlicerBaseQTBasePythonQtDecorators(this));

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  // Add object to python interpreter context
  this->addObjectToPythonMain("_qSlicerCoreApplicationInstance", app);

}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::addVTKObjectToPythonMain(const QString& name, vtkObject * object)
{
  if (name.isNull() || !object)
    {
    return;
    }
  // Split name using '.'
  QStringList moduleNameList = name.split('.', QString::SkipEmptyParts);

  // Remove the last part
  QString varName = moduleNameList.takeLast();

  PyObject * module = PythonQt::self()->getMainModule();

  // Loop over module name and try to import them one by one
  foreach(const QString& moduleName, moduleNameList)
    {
    module = PyImport_ImportModule(moduleName.toLatin1());
    Q_ASSERT(module);
    }

  // Add the object to the imported module
  int ret = PyModule_AddObject(module, varName.toLatin1(),
                               vtkPythonUtil::GetObjectFromPointer(object));
  Q_ASSERT(ret == 0);
  if (ret != 0)
    {
    logger.error(QString("Failed to add VTK object: %1").arg(name));
    }
}
