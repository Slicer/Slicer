/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QHash>

// SlicerQt includes
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerAbstractModule.h"

//-----------------------------------------------------------------------------
class qSlicerModuleFactoryManagerPrivate : public ctkPrivate<qSlicerModuleFactoryManager>
{
public:
  // Convenient typedefs
  typedef qSlicerModuleFactoryManagerPrivate Self; 
  typedef ctkAbstractFactory<qSlicerAbstractModule> qSlicerAbstractModuleFactory;

  // Instantiate a module
  qSlicerAbstractModule* instantiateModule(qSlicerAbstractModuleFactory* factory,
                                           const QString& name);

  void printAdditionalInfo();

  // Uninstantiate module
  void uninstantiateModule(const QString& name); 

  // Convenient typdefs
  typedef QHash<QString, QString>::const_iterator MapConstIterator;
  typedef QHash<QString, QString>::iterator       MapIterator;
  // Maps
  QHash<QString, QString> MapTitleToName;
  QHash<QString, QString> MapNameToTitle;

  // Convenient typdefs
  typedef QHash<QString, qSlicerAbstractModuleFactory*>::const_iterator Map2ConstIterator;
  typedef QHash<QString, qSlicerAbstractModuleFactory*>::iterator       Map2Iterator;
  // Maps
  QHash<QString, qSlicerAbstractModuleFactory*> RegisteredFactories;
  QHash<QString, qSlicerAbstractModuleFactory*> ModuleNameToFactoryCache;
};

//-----------------------------------------------------------------------------
// qSlicerModuleFactoryManagerPrivate methods

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManagerPrivate::printAdditionalInfo()
{
  qDebug() << "RegisteredFactories:" << this->RegisteredFactories.keys();
  qDebug() << "ModuleNameToFactoryCache:" << this->ModuleNameToFactoryCache.keys();
  qDebug() << "[MapTitleToName]";
  Self::MapConstIterator iter = this->MapTitleToName.constBegin();
  while(iter != this->MapTitleToName.constEnd())
    {
    qDebug() << "\tTitle:" << iter.key() << "-> Name:" << iter.value();
    ++iter;
    }
  qDebug() << "[MapNameToTitle]";
  iter = this->MapNameToTitle.constBegin();
  while(iter != this->MapNameToTitle.constEnd())
    {
    qDebug() << "\tName:" << iter.key() << "-> Title:" << iter.value();
    ++iter;
    }
  qDebug() << "[RegisteredFactories]";
  Self::Map2ConstIterator iter2 = this->RegisteredFactories.constBegin();
  while(iter2 != this->RegisteredFactories.constEnd())
    {
    iter2.value()->printAdditionalInfo();
    ++iter2;
    }
  qDebug() << "[ModuleNameToFactoryCache]";
  iter2 = this->ModuleNameToFactoryCache.constBegin();
  while(iter2 != this->ModuleNameToFactoryCache.constEnd())
    {
    qDebug() << "ModuleName:" << iter2.key() << "-> Factory:" << iter2.value();
    ++iter2;
    }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleFactoryManagerPrivate::instantiateModule(
  qSlicerAbstractModuleFactory* factory, const QString& name)
{
  qSlicerAbstractModule* module = 0;
  // Try to instantiate a module
  module = factory->instantiate(name);
  Q_ASSERT(module);
  module->setName(name);
  
  QString title = module->title();
  // Keep track of the relation Title -> name
  this->MapTitleToName[title] = name;

  // Keep track of the relation name -> Title
  this->MapNameToTitle[name] = title;
  
  return module;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManagerPrivate::uninstantiateModule(const QString& name)
{
  CTK_P(qSlicerModuleFactoryManager);
  
  // Retrieve the factoryType associated with the module
  Self::Map2ConstIterator iter = this->ModuleNameToFactoryCache.constFind(name);

  Q_ASSERT(iter != this->ModuleNameToFactoryCache.constEnd());
  if (iter == this->ModuleNameToFactoryCache.constEnd())
    {
    qWarning() << "Failed to retrieve factory name for module:" << name;
    return;
    }
  QString title = p->moduleTitle(name);
  
  iter.value()->uninstantiate(name);
  
  this->MapTitleToName.remove(title);
  this->MapNameToTitle.remove(name);
}

//-----------------------------------------------------------------------------
// qSlicerModuleFactoryManager methods

//-----------------------------------------------------------------------------
qSlicerModuleFactoryManager::qSlicerModuleFactoryManager()
{
  CTK_INIT_PRIVATE(qSlicerModuleFactoryManager);
}

//-----------------------------------------------------------------------------
qSlicerModuleFactoryManager::~qSlicerModuleFactoryManager()
{
  this->uninstantiateAll();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::printAdditionalInfo()
{
  CTK_D(qSlicerModuleFactoryManager);
  
  qDebug() << "qSlicerModuleFactoryManager (" << this << ")";
  d->printAdditionalInfo();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::registerFactory(const QString& factoryName,
                                                  qSlicerAbstractModuleFactory* factory)
{
  CTK_D(qSlicerModuleFactoryManager);
  Q_ASSERT(!d->RegisteredFactories.contains(factoryName));
  d->RegisteredFactories[factoryName] = factory;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::registerAllModules()
{
  CTK_D(qSlicerModuleFactoryManager);
  foreach (const QString& factoryName, d->RegisteredFactories.keys())
    {
    this->registerModules(factoryName);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::registerModules(const QString& factoryName)
{
  CTK_D(qSlicerModuleFactoryManager);
  Q_ASSERT(d->RegisteredFactories.contains(factoryName));
  qSlicerAbstractModuleFactory * factory = d->RegisteredFactories[factoryName];

  factory->registerItems();
  
  // Keep track of the registered modules and their associated factory
  foreach(const QString& name, factory->names())
    {
    d->ModuleNameToFactoryCache[name] = factory; 
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::instantiateAllModules()
{
  CTK_D(qSlicerModuleFactoryManager);
  foreach (const QString& factoryName, d->RegisteredFactories.keys())
    {
    this->instantiateModules(factoryName);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::instantiateModules(const QString& factoryName)
{
  CTK_D(qSlicerModuleFactoryManager);
  Q_ASSERT(d->RegisteredFactories.contains(factoryName));
  qSlicerAbstractModuleFactory * factory = d->RegisteredFactories[factoryName];

  foreach(const QString& name, factory->names())
    {
    d->instantiateModule(factory, name);
    }
}
  
//-----------------------------------------------------------------------------
QString qSlicerModuleFactoryManager::moduleName(const QString & title) const
{
  CTK_D(const qSlicerModuleFactoryManager);
  // Lookup module name
  qSlicerModuleFactoryManagerPrivate::MapConstIterator iter = d->MapTitleToName.constFind(title);

  if (iter == d->MapTitleToName.constEnd())
    {
    //qCritical() << "Failed to retrieve module name given its title:" << title;
    return QString();
    }
  return iter.value();
}

//-----------------------------------------------------------------------------
QString qSlicerModuleFactoryManager::moduleTitle(const QString & name) const
{
  CTK_D(const qSlicerModuleFactoryManager);
  // Lookup module name
  qSlicerModuleFactoryManagerPrivate::MapConstIterator iter = d->MapNameToTitle.constFind(name);

  if (iter == d->MapNameToTitle.constEnd())
    {
    //qCritical() << "Failed to retrieve module title given its name:" << name;
    return QString();
    }
  return iter.value();
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactoryManager::moduleNames() const
{
  CTK_D(const qSlicerModuleFactoryManager);
  QStringList names;
  foreach (const QString& factoryName, d->RegisteredFactories.keys())
    {
    names << d->RegisteredFactories[factoryName]->names();
    }
  return names;
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactoryManager::moduleNames(const QString& factoryName) const
{
  CTK_D(const qSlicerModuleFactoryManager);
  Q_ASSERT(d->RegisteredFactories.contains(factoryName));
  qSlicerAbstractModuleFactory * factory = d->RegisteredFactories[factoryName];
  return factory->names();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleFactoryManager::instantiateModule(const QString& name)
{
  CTK_D(qSlicerModuleFactoryManager);
  
  // Retrieve the factory name associated with the module
  qSlicerModuleFactoryManagerPrivate::Map2ConstIterator iter =
    d->ModuleNameToFactoryCache.constFind(name);

  Q_ASSERT(iter != d->ModuleNameToFactoryCache.constEnd());
  if (iter == d->ModuleNameToFactoryCache.constEnd())
    {
    qWarning() << "Failed to retrieve factory name for module:" << name;
    return 0;
    }

  qSlicerAbstractModule* module = d->instantiateModule(*iter, name);
  Q_ASSERT(module);
  
  return module;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::uninstantiateModule(const QString& name)
{
  CTK_D(qSlicerModuleFactoryManager);
  qDebug() << "Uninstantiating:" << name;
  d->uninstantiateModule(name);
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::uninstantiateAll()
{
  foreach(const QString& name, this->moduleNames())
    {
    this->uninstantiateModule(name);
    }
}

//-----------------------------------------------------------------------------
bool qSlicerModuleFactoryManager::isRegistered(const QString& name)const
{
  CTK_D(const qSlicerModuleFactoryManager);
  return d->MapNameToTitle.contains(name);
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::setVerboseModuleDiscovery(bool value)
{
  CTK_D(qSlicerModuleFactoryManager);
  
  foreach (const QString& factoryName, d->RegisteredFactories.keys())
    {
    d->RegisteredFactories[factoryName]->setVerbose(value);
    }
}
