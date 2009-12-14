/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerModuleManager.h"

// SlicerQT includes
#include "qSlicerCoreApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerModuleFactory.h"

// MRML includes
#include <vtkMRMLScene.h>

// QT includes
#include <QHash>
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerModuleManagerPrivate: public qCTKPrivate<qSlicerModuleManager>
{
  QCTK_DECLARE_PUBLIC(qSlicerModuleManager);

  // Description:
  // Handle post-load initialization
  void onModuleLoaded(qSlicerAbstractModule* module);

  typedef QHash<QString, qSlicerAbstractModule*>::const_iterator ModuleListConstIterator;
  typedef QHash<QString, qSlicerAbstractModule*>::iterator       ModuleListIterator;

  QHash<QString, qSlicerAbstractModule*> ModuleList;  // Store Pair<ModuleName, ModuleObject>
  qSlicerModuleFactory                   ModuleFactory;
};

//-----------------------------------------------------------------------------
qSlicerModuleManager::qSlicerModuleManager()
{
  QCTK_INIT_PRIVATE(qSlicerModuleManager);
  //QCTK_D(qSlicerModuleManager);
}

//-----------------------------------------------------------------------------
void qSlicerModuleManager::printAdditionalInfo()
{
  QCTK_D(qSlicerModuleManager);
  qDebug() << "qSlicerModuleManager (" << this << ")";
  qDebug() << "ModuleList";

  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.constBegin();
  while(iter != d->ModuleList.constEnd())
    {
    qDebug() << "Name:" << iter.key();
    iter.value()->printAdditionalInfo();
    ++iter;
    }
  d->ModuleFactory.printAdditionalInfo();
}

//---------------------------------------------------------------------------
qSlicerModuleFactory* qSlicerModuleManager::factory()
{
  return &qctk_d()->ModuleFactory;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::isLoaded(const QString& moduleName)
{
  QCTK_D(qSlicerModuleManager);
  return d->ModuleList.contains(moduleName);
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::loadModule(const QString& moduleName)
{
  QCTK_D(qSlicerModuleManager);
  // Check if module has been loaded already
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.constFind(moduleName);
  if (iter != d->ModuleList.constEnd())
    {
    //return iter.value();
    return true;
    }

  // Instantiate the module
  qSlicerAbstractModule * module = d->ModuleFactory.instantiateModule(moduleName);
  if (!module)
    {
    qWarning() << "Failed to instanciate module: " << moduleName;
    return 0;
    }

  // Update internal Map
  d->ModuleList[moduleName] = module;

  // Initialize module
  module->initialize(qSlicerCoreApplication::application()->appLogic());

  // Retrieve module title
  QString moduleTitle = d->ModuleFactory.getModuleTitle(moduleName);
  Q_ASSERT(!moduleTitle.isEmpty());
  if (moduleTitle.isEmpty())
    {
    qWarning() << "Failed to retrieve module title: " << moduleName;
    return 0;
    }

  // Set module title
  //module->setWindowTitle(moduleTitle);
  //qDebug() << module << " - title:" << moduleTitle;

  // Set the MRML scene
  module->setMRMLScene(qSlicerCoreApplication::application()->mrmlScene());

  // Module should also be aware if current MRML scene has changed
  this->connect(qSlicerCoreApplication::application(),
                SIGNAL(currentMRMLSceneChanged(vtkMRMLScene*)),
                module,
                SLOT(setMRMLScene(vtkMRMLScene*)));

  // Handle post-load initialization
  emit this->moduleLoaded(module);
  
  return true;
  //return module;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::unLoadModule(const QString& moduleName)
{
  QCTK_D(qSlicerModuleManager);
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find( moduleName );
  if (iter == d->ModuleList.constEnd())
    {
    qWarning() << "Failed to unload module: " << moduleName << " - Module wasn't loaded";
    return false;
    }

  qSlicerAbstractModule * module = iter.value();
  Q_ASSERT(module);
  
  // Handle pre-unload
  emit this->moduleAboutToBeUnloaded(module);
  
  // Tells Qt to delete the object when appropriate
  module->deleteLater();

  // Remove the object from the list
  d->ModuleList.remove(iter.key());

  return true;
}

//---------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleManager::getModule(const QString& moduleName)
{
  QCTK_D(qSlicerModuleManager);
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find( moduleName );
  if ( iter == d->ModuleList.constEnd() )
    {
    return 0;
    }
  return iter.value();
}

//---------------------------------------------------------------------------
QString qSlicerModuleManager::moduleTitle(const QString& moduleName) const
{
  return qctk_d()->ModuleFactory.getModuleTitle(moduleName);
}

//---------------------------------------------------------------------------
QString qSlicerModuleManager::moduleName(const QString& moduleTitle) const
{
  return qctk_d()->ModuleFactory.getModuleName(moduleTitle);
}
