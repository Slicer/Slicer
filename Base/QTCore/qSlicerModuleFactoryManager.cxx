/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QHash>

// SlicerQt includes
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerAbstractCoreModule.h"

//-----------------------------------------------------------------------------
class qSlicerModuleFactoryManagerPrivate
{
  Q_DECLARE_PUBLIC(qSlicerModuleFactoryManager);
protected:
  qSlicerModuleFactoryManager* const q_ptr;
public:
  qSlicerModuleFactoryManagerPrivate(qSlicerModuleFactoryManager& object);
  // Convenient typedefs
  typedef qSlicerModuleFactoryManagerPrivate Self; 
  typedef ctkAbstractFactory<qSlicerAbstractCoreModule> qSlicerAbstractCoreModuleFactory;

  // Instantiate a module
  qSlicerAbstractCoreModule* instantiateModule(qSlicerAbstractCoreModuleFactory* factory,
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
  typedef QHash<QString, qSlicerAbstractCoreModuleFactory*>::const_iterator Map2ConstIterator;
  typedef QHash<QString, qSlicerAbstractCoreModuleFactory*>::iterator       Map2Iterator;
  // Maps
  QHash<QString, qSlicerModuleFactoryManager::qSlicerAbstractModuleFactory*> RegisteredFactories;
  QHash<QString, qSlicerModuleFactoryManager::qSlicerAbstractModuleFactory*> ModuleNameToFactoryCache;
};

//-----------------------------------------------------------------------------
// qSlicerModuleFactoryManagerPrivate methods
qSlicerModuleFactoryManagerPrivate::qSlicerModuleFactoryManagerPrivate(qSlicerModuleFactoryManager& object)
  : q_ptr(&object)
{
}

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
qSlicerAbstractCoreModule* qSlicerModuleFactoryManagerPrivate::instantiateModule(
  qSlicerAbstractCoreModuleFactory* factory, const QString& name)
{
  qSlicerAbstractCoreModule* module = 0;
  // Try to instantiate a module
  module = factory->instantiate(name);
  if ( !module )
    {
    return 0;
    }
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
  Q_Q(qSlicerModuleFactoryManager);
  
  // Retrieve the factoryType associated with the module
  Self::Map2ConstIterator iter = this->ModuleNameToFactoryCache.constFind(name);

  Q_ASSERT(iter != this->ModuleNameToFactoryCache.constEnd());
  if (iter == this->ModuleNameToFactoryCache.constEnd())
    {
    qWarning() << "Failed to retrieve factory name for module:" << name;
    return;
    }
  QString title = q->moduleTitle(name);
  
  iter.value()->uninstantiate(name);
  
  this->MapTitleToName.remove(title);
  this->MapNameToTitle.remove(name);
}

//-----------------------------------------------------------------------------
// qSlicerModuleFactoryManager methods

//-----------------------------------------------------------------------------
qSlicerModuleFactoryManager::qSlicerModuleFactoryManager(QObject * newParent)
  : Superclass(newParent), d_ptr(new qSlicerModuleFactoryManagerPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerModuleFactoryManager::~qSlicerModuleFactoryManager()
{
  this->uninstantiateAll();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::printAdditionalInfo()
{
  Q_D(qSlicerModuleFactoryManager);
  
  qDebug() << "qSlicerModuleFactoryManager (" << this << ")";
  d->printAdditionalInfo();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::registerFactory(const QString& factoryName,
                                                  qSlicerAbstractModuleFactory* factory)
{
  Q_D(qSlicerModuleFactoryManager);
  Q_ASSERT(!d->RegisteredFactories.contains(factoryName));
  d->RegisteredFactories[factoryName] = factory;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::registerAllModules()
{
  Q_D(qSlicerModuleFactoryManager);
  foreach (const QString& factoryName, d->RegisteredFactories.keys())
    {
    this->registerModules(factoryName);
    }
  emit this->allModulesRegistered();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::registerModules(const QString& factoryName)
{
  Q_D(qSlicerModuleFactoryManager);
  Q_ASSERT(d->RegisteredFactories.contains(factoryName));
  qSlicerAbstractModuleFactory * factory = d->RegisteredFactories[factoryName];

  factory->registerItems();
  
  // Keep track of the registered modules and their associated factory
  foreach(const QString& key, factory->keys())
    {
    d->ModuleNameToFactoryCache[key] = factory;
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::instantiateAllModules()
{
  Q_D(qSlicerModuleFactoryManager);
  foreach (const QString& factoryName, d->RegisteredFactories.keys())
    {
    this->instantiateModules(factoryName);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::instantiateModules(const QString& factoryName)
{
  Q_D(qSlicerModuleFactoryManager);
  Q_ASSERT(d->RegisteredFactories.contains(factoryName));
  qSlicerAbstractModuleFactory * factory = d->RegisteredFactories[factoryName];

  foreach(const QString& key, factory->keys())
    {
    d->instantiateModule(factory, key);
    }
}
  
//-----------------------------------------------------------------------------
QString qSlicerModuleFactoryManager::moduleName(const QString & title) const
{
  Q_D(const qSlicerModuleFactoryManager);
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
  Q_D(const qSlicerModuleFactoryManager);
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
  Q_D(const qSlicerModuleFactoryManager);
  QStringList names;
  foreach (const QString& factoryName, d->RegisteredFactories.keys())
    {
    names << d->RegisteredFactories[factoryName]->keys();
    }
  return names;
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactoryManager::moduleNames(const QString& factoryName) const
{
  Q_D(const qSlicerModuleFactoryManager);
  Q_ASSERT(d->RegisteredFactories.contains(factoryName));
  qSlicerAbstractModuleFactory * factory = d->RegisteredFactories[factoryName];
  return factory->keys();
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerModuleFactoryManager::instantiateModule(const QString& name)
{
  Q_D(qSlicerModuleFactoryManager);
  
  // Retrieve the factory name associated with the module
  qSlicerModuleFactoryManagerPrivate::Map2ConstIterator iter =
    d->ModuleNameToFactoryCache.constFind(name);

  Q_ASSERT(iter != d->ModuleNameToFactoryCache.constEnd());
  if (iter == d->ModuleNameToFactoryCache.constEnd())
    {
    qWarning() << "Failed to retrieve factory name for module:" << name;
    return 0;
    }

  qSlicerAbstractCoreModule* module = d->instantiateModule(*iter, name);
  Q_ASSERT(module);
  
  return module;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::uninstantiateModule(const QString& name)
{
  Q_D(qSlicerModuleFactoryManager);
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
  Q_D(const qSlicerModuleFactoryManager);
  return d->MapNameToTitle.contains(name);
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryManager::setVerboseModuleDiscovery(bool value)
{
  Q_D(qSlicerModuleFactoryManager);
  
  foreach (const QString& factoryName, d->RegisteredFactories.keys())
    {
    d->RegisteredFactories[factoryName]->setVerbose(value);
    }
}
