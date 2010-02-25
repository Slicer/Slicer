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
#include "qSlicerModuleFactoryManager.h"

// MRML includes
#include <vtkMRMLScene.h>

// QT includes
#include <QHash>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerModuleManagerPrivate: public qCTKPrivate<qSlicerModuleManager>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerModuleManager);

  ///
  /// Handle post-load initialization
  void onModuleLoaded(qSlicerAbstractModule* module);

  typedef QHash<QString, qSlicerAbstractModule*>::const_iterator ModuleListConstIterator;
  typedef QHash<QString, qSlicerAbstractModule*>::iterator       ModuleListIterator;

  ///
  /// Store Pair<ModuleName, ModuleObject>
  QHash<QString, qSlicerAbstractModule*> ModuleList;  
  qSlicerModuleFactoryManager            ModuleFactoryManager;
};

//-----------------------------------------------------------------------------
qSlicerModuleManager::qSlicerModuleManager()
{
  QCTK_INIT_PRIVATE(qSlicerModuleManager);
  //QCTK_D(qSlicerModuleManager);

  // The module manager should be instanciated only if a qSlicerCoreApplication exists
  Q_ASSERT(qSlicerCoreApplication::application());
  // and has been initialized
  Q_ASSERT(qSlicerCoreApplication::application()->initialized());
  
}

//-----------------------------------------------------------------------------
qSlicerModuleManager::~qSlicerModuleManager()
{

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
  d->ModuleFactoryManager.printAdditionalInfo();
}

//---------------------------------------------------------------------------
qSlicerModuleFactoryManager* qSlicerModuleManager::factoryManager()const
{
  QCTK_D(const qSlicerModuleManager);
  return const_cast<qSlicerModuleFactoryManager*>(&d->ModuleFactoryManager);
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::isLoaded(const QString& name)const
{
  QCTK_D(const qSlicerModuleManager);
  // If a module is not registered, we consider it isn't loaded
  if (!d->ModuleFactoryManager.isRegistered(name))
    {
    return false;
    }
  return d->ModuleList.contains(name);
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::loadModule(const QString& name)
{
  QCTK_D(qSlicerModuleManager);

  // A module should be registered when attempting to load it
  //Q_ASSERT(d->ModuleFactoryManager.isRegistered(name));
  if (!d->ModuleFactoryManager.isRegistered(name))
    {
    return false;
    }

  // Check if module has been loaded already
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.constFind(name);
  if (iter != d->ModuleList.constEnd())
    {
    return true;
    }

  // Instantiate the module
  qSlicerAbstractModule * _module = d->ModuleFactoryManager.instantiateModule(name);
  if (!_module)
    {
    qWarning() << "Failed to instanciate module: " << name;
    return 0;
    }

  // Update internal Map
  d->ModuleList[name] = _module;

  // Initialize module
  _module->initialize(qSlicerCoreApplication::application()->appLogic());

  // Retrieve module title
  QString _moduleTitle = d->ModuleFactoryManager.moduleTitle(name);
  Q_ASSERT(!_moduleTitle.isEmpty());
  if (_moduleTitle.isEmpty())
    {
    qWarning() << "Failed to retrieve module title corresponding to module name: " << name;
    return 0;
    }

  // Set the MRML scene
  _module->setMRMLScene(qSlicerCoreApplication::application()->mrmlScene());

  // Module should also be aware if current MRML scene has changed
  this->connect(qSlicerCoreApplication::application(),
                SIGNAL(currentMRMLSceneChanged(vtkMRMLScene*)),
                _module,
                SLOT(setMRMLScene(vtkMRMLScene*)));

  // Handle post-load initialization
  emit this->moduleLoaded(_module);
  
  return true;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::unLoadModule(const QString& name)
{
  QCTK_D(qSlicerModuleManager);

  // A module should be registered when attempting to unload it
  //Q_ASSERT(d->ModuleFactoryManager.isRegistered(name));
  if (!d->ModuleFactoryManager.isRegistered(name))
    {
    return false;
    }
  
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find( name );
  if (iter == d->ModuleList.constEnd())
    {
    qWarning() << "Failed to unload module: " << name << " - Module wasn't loaded";
    return false;
    }

  qSlicerAbstractModule * _module = iter.value();
  Q_ASSERT(_module);
  
  // Handle pre-unload
  emit this->moduleAboutToBeUnloaded(_module);
  
  // Tells Qt to delete the object when appropriate
  _module->deleteLater();

  // Remove the object from the list
  d->ModuleList.remove(iter.key());

  return true;
}

//---------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleManager::module(const QString& name)
{
  QCTK_D(qSlicerModuleManager);

  // A module should be registered when attempting to obtain it
  // assert causes a crash on linux64 when this check fails
//  Q_ASSERT(d->ModuleFactoryManager.isRegistered(name));
  if (!d->ModuleFactoryManager.isRegistered(name))
    {
    return 0;
    }
  
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find(name);
  if ( iter == d->ModuleList.constEnd() )
    {
    return 0;
    }
  return iter.value();
}

//---------------------------------------------------------------------------
QString qSlicerModuleManager::moduleTitle(const QString& name) const
{
  return qctk_d()->ModuleFactoryManager.moduleTitle(name);
}

//---------------------------------------------------------------------------
QString qSlicerModuleManager::moduleName(const QString& title) const
{
  return qctk_d()->ModuleFactoryManager.moduleName(title);
}
