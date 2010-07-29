/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QHash>
#include <QDebug>

// SlicerQt includes
#include "qSlicerModuleManager.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerModuleFactoryManager.h"

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
class qSlicerModuleManagerPrivate: public ctkPrivate<qSlicerModuleManager>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerModuleManager);

  ///
  /// Handle post-load initialization
  void onModuleLoaded(qSlicerAbstractCoreModule* module);

  typedef QHash<QString, qSlicerAbstractCoreModule*>::const_iterator ModuleListConstIterator;
  typedef QHash<QString, qSlicerAbstractCoreModule*>::iterator       ModuleListIterator;

  ///
  /// Store Pair<ModuleName, ModuleObject>
  QHash<QString, qSlicerAbstractCoreModule*> ModuleList;
  qSlicerModuleFactoryManager            ModuleFactoryManager;
};

//-----------------------------------------------------------------------------
qSlicerModuleManager::qSlicerModuleManager()
{
  CTK_INIT_PRIVATE(qSlicerModuleManager);
  //CTK_D(qSlicerModuleManager);

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
  CTK_D(qSlicerModuleManager);
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
  CTK_D(const qSlicerModuleManager);
  return const_cast<qSlicerModuleFactoryManager*>(&d->ModuleFactoryManager);
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::isLoaded(const QString& name)const
{
  CTK_D(const qSlicerModuleManager);
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
  CTK_D(qSlicerModuleManager);

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
  qSlicerAbstractCoreModule * _module = d->ModuleFactoryManager.instantiateModule(name);
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
                SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                _module,
                SLOT(setMRMLScene(vtkMRMLScene*)));

  // Handle post-load initialization
  emit this->moduleLoaded(_module);
  
  return true;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::unLoadModule(const QString& name)
{
  CTK_D(qSlicerModuleManager);

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

  qSlicerAbstractCoreModule * _module = iter.value();
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
qSlicerAbstractCoreModule* qSlicerModuleManager::module(const QString& name)
{
  CTK_D(qSlicerModuleManager);

  // A module should be registered when attempting to obtain it
  // assert causes a crash on linux64 when this check fails
  //  Q_ASSERT(d->ModuleFactoryManager.isRegistered(name));
  if (!d->ModuleFactoryManager.isRegistered(name))
    {
    qDebug() << "The module \"" << name << "\" has not been registered.";
    qDebug() << "The following modules have been registered:"
             << d->ModuleFactoryManager.moduleNames();
    return 0;
    }

  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find(name);
  if ( iter == d->ModuleList.constEnd() )
    {
    qDebug()<< "The module \"" << name << "\" can not be found.";
    qDebug() << "The following modules exists:" << d->ModuleList.keys();
    return 0;
    }
  return iter.value();
}

//---------------------------------------------------------------------------
QString qSlicerModuleManager::moduleTitle(const QString& name) const
{
  return ctk_d()->ModuleFactoryManager.moduleTitle(name);
}

//---------------------------------------------------------------------------
QString qSlicerModuleManager::moduleName(const QString& title) const
{
  return ctk_d()->ModuleFactoryManager.moduleName(title);
}
