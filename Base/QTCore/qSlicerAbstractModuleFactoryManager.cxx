/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <QDir>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerAbstractModuleFactoryManager.h"
#include "qSlicerAbstractCoreModule.h"

// STD includes
#include <csignal>
#include <typeinfo>

//-----------------------------------------------------------------------------
class qSlicerAbstractModuleFactoryManagerPrivate
{
  Q_DECLARE_PUBLIC(qSlicerAbstractModuleFactoryManager);
protected:
  qSlicerAbstractModuleFactoryManager* const q_ptr;
public:
  qSlicerAbstractModuleFactoryManagerPrivate(qSlicerAbstractModuleFactoryManager& object);

  void printAdditionalInfo();

  typedef qSlicerAbstractModuleFactoryManager::qSlicerModuleFactory
    qSlicerModuleFactory;
  typedef qSlicerAbstractModuleFactoryManager::qSlicerFileBasedModuleFactory
    qSlicerFileBasedModuleFactory;
  QVector<qSlicerFileBasedModuleFactory*> fileBasedFactories()const;
  QVector<qSlicerModuleFactory*> notFileBasedFactories()const;

  // Helper function that returns module factory for a module name, without
  // the risk of creating a nullptr entry if the module is not registered.
  qSlicerModuleFactory* registeredModuleFactory(const QString& moduleName)const;

  QStringList SearchPaths;
  QStringList ExplicitModules;
  QStringList ModulesToIgnore;
  QMap<QString, QFileInfo> IgnoredModules;
  QMap<qSlicerModuleFactory*, int> Factories;
  QMap<QString, qSlicerModuleFactory*> RegisteredModules;
  QMap<QString, QStringList> ModuleDependees;

  bool Verbose;
};

//-----------------------------------------------------------------------------
// qSlicerAbstractModuleFactoryManagerPrivate methods
qSlicerAbstractModuleFactoryManagerPrivate::qSlicerAbstractModuleFactoryManagerPrivate(qSlicerAbstractModuleFactoryManager& object)
  : q_ptr(&object)
{
  this->Verbose = false;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManagerPrivate::printAdditionalInfo()
{
  Q_Q(qSlicerAbstractModuleFactoryManager);
  qDebug() << "Factories:";
  foreach(qSlicerAbstractModuleFactoryManager::qSlicerModuleFactory* factory,
          this->Factories.keys())
    {
    // todo: qSlicerModuleFactory should derive from QObject.
    qDebug() << "\t" << typeid(factory).name() << ": ";
    factory->printAdditionalInfo();
    }
  qDebug() << "Modules to ignore:" << q->modulesToIgnore();
  qDebug() << "Registered modules:" << q->registeredModuleNames();
  qDebug() << "Ignored modules:" << q->ignoredModuleNames();
  qDebug() << "Instantiated modules:" << q->instantiatedModuleNames();
}

//-----------------------------------------------------------------------------
QVector<qSlicerAbstractModuleFactoryManagerPrivate::qSlicerFileBasedModuleFactory*>
qSlicerAbstractModuleFactoryManagerPrivate::fileBasedFactories()const
{
  QVector<qSlicerFileBasedModuleFactory*> factories;
  foreach(qSlicerModuleFactory* factory, this->Factories.keys())
    {
    if (dynamic_cast<qSlicerFileBasedModuleFactory*>(factory) != nullptr)
      {
      factories << dynamic_cast<qSlicerFileBasedModuleFactory*>(factory);
      }
    }
  return factories;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleFactoryManagerPrivate::qSlicerModuleFactory*
qSlicerAbstractModuleFactoryManagerPrivate
::registeredModuleFactory(const QString& moduleName)const
{
  if (!this->RegisteredModules.contains(moduleName))
    {
    return nullptr;
    }
  return this->RegisteredModules[moduleName];
}

//-----------------------------------------------------------------------------
QVector<qSlicerAbstractModuleFactoryManagerPrivate::qSlicerModuleFactory*>
qSlicerAbstractModuleFactoryManagerPrivate
::notFileBasedFactories()const
{
  QVector<qSlicerModuleFactory*> factories;
  foreach(qSlicerModuleFactory* factory, this->Factories.keys())
    {
    if (dynamic_cast<qSlicerFileBasedModuleFactory*>(factory) == nullptr)
      {
      factories << factory;
      }
    }
  return factories;
}

//-----------------------------------------------------------------------------
// qSlicerAbstractModuleFactoryManager methods

//-----------------------------------------------------------------------------
qSlicerAbstractModuleFactoryManager::qSlicerAbstractModuleFactoryManager(QObject * newParent)
  : Superclass(newParent), d_ptr(new qSlicerAbstractModuleFactoryManagerPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleFactoryManager::~qSlicerAbstractModuleFactoryManager()
{
  this->uninstantiateModules();
  this->unregisterFactories();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::printAdditionalInfo()
{
  Q_D(qSlicerAbstractModuleFactoryManager);

  qDebug() << "qSlicerAbstractModuleFactoryManager (" << this << ")";
  d->printAdditionalInfo();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager
::registerFactory(qSlicerModuleFactory* factory, int priority)
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  Q_ASSERT(!d->Factories.contains(factory));
  d->Factories[factory] = priority;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::unregisterFactory(qSlicerModuleFactory* factory)
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  Q_ASSERT(d->Factories.contains(factory));
  d->Factories.remove(factory);
  delete factory;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::unregisterFactories()
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  while (!d->Factories.isEmpty())
    {
    this->unregisterFactory(d->Factories.begin().key());
    }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::setSearchPaths(const QStringList& paths)
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  d->SearchPaths = paths;
  d->SearchPaths.removeDuplicates();
  d->SearchPaths.removeAll(QString());
}

//-----------------------------------------------------------------------------
QStringList qSlicerAbstractModuleFactoryManager::searchPaths()const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  return d->SearchPaths;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::setExplicitModules(const QStringList& moduleNames)
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  if (d->ModulesToIgnore == moduleNames)
    {
    return;
    }
  d->ExplicitModules = moduleNames;
  emit explicitModulesChanged(moduleNames);
}

//-----------------------------------------------------------------------------
QStringList qSlicerAbstractModuleFactoryManager::explicitModules()const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  return d->ExplicitModules;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::setModulesToIgnore(const QStringList& moduleNames)
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  if (d->ModulesToIgnore == moduleNames)
    {
    return;
    }
  d->ModulesToIgnore = moduleNames;
  emit modulesToIgnoreChanged(moduleNames);
}

//-----------------------------------------------------------------------------
QStringList qSlicerAbstractModuleFactoryManager::modulesToIgnore()const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  return d->ModulesToIgnore;
}

//-----------------------------------------------------------------------------
QStringList qSlicerAbstractModuleFactoryManager::ignoredModuleNames()const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  return d->IgnoredModules.keys();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::registerModules()
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  // Register "regular" factories first
  // \todo: don't support factories other than filebased factories
  foreach(qSlicerModuleFactory* factory, d->notFileBasedFactories())
    {
    factory->registerItems();
    foreach(const QString& moduleName, factory->itemKeys())
      {
      if (d->Verbose)
        {
        qDebug() << "Registering: " << moduleName;
        }
      d->RegisteredModules[moduleName] = factory;
      emit moduleRegistered(moduleName);
      }
    }
  // then register file based factories
  foreach(const QString& path, d->SearchPaths)
    {
    if (d->Verbose)
      {
      qDebug() << "Searching path: " << path;
      }
    this->registerModules(path);
    }
  emit this->modulesRegistered(d->RegisteredModules.keys());
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::registerModules(const QString& path)
{
  QDir directory(path);
  /// \tbd recursive search ?
  foreach (const QFileInfo& file,
           directory.entryInfoList(QDir::Files))
    {
    this->registerModule(file);
    }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::registerModule(const QFileInfo& file)
{
  Q_D(qSlicerAbstractModuleFactoryManager);

  qSlicerFileBasedModuleFactory* moduleFactory = nullptr;
  foreach(qSlicerFileBasedModuleFactory* factory, d->fileBasedFactories())
    {
    if (d->Verbose)
      {
      qDebug() << " checking file: " << file.absoluteFilePath() << " as a " << typeid(*factory).name();
      }
    if (!factory->isValidFile(file))
      {
      continue;
      }
    if (d->Verbose)
      {
      qDebug() << " recognized file: " << file.absoluteFilePath() << " as a " << typeid(*factory).name();
      }
    moduleFactory = factory;
    break;
    }
  // File not supported by any factory
  if (moduleFactory == nullptr)
    {
    return;
    }
  QString moduleName = moduleFactory->itemKey(file);
  bool dontEmitSignal = false;
  // Has the module been already registered
  qSlicerModuleFactory* existingModuleFactory = d->registeredModuleFactory(moduleName);
  if (existingModuleFactory)
    {
    if (d->Factories[existingModuleFactory] >=
        d->Factories[moduleFactory])
      {
      if (d->Verbose)
        {
        qDebug() << " file: " << file.absoluteFilePath() << " already registered";
        }
      return;
      }
    // Replace the factory of the registered module with this higher priority
    // factory.
    //existingModuleFactory->unregisterItem(file);
    dontEmitSignal = true;
    }
  if (d->ModulesToIgnore.contains(moduleName))
    {
    //qDebug() << "Ignore module" << moduleName;
    if (d->Verbose)
      {
      qDebug() << " file: " << file.absoluteFilePath() << " is in ignore list";
      }
    d->IgnoredModules[moduleName] = file;
    emit moduleIgnored(moduleName);
    return;
    }
  QString registeredModuleName = moduleFactory->registerFileItem(file);
  if (registeredModuleName != moduleName)
    {
    //qDebug() << "Ignore module" << moduleName;
    if (d->Verbose)
      {
      qDebug() << " file: " << file.absoluteFilePath() << " ignored because moduleName does not match registeredModuleName";
      }
    d->IgnoredModules[moduleName] = file;
    emit moduleIgnored(moduleName);
    return;
    }
  d->RegisteredModules[moduleName] = moduleFactory;
  if (!dontEmitSignal)
    {
    emit moduleRegistered(moduleName);
    }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::instantiateModules()
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  foreach (const QString& moduleName, d->RegisteredModules.keys())
    {
    this->instantiateModule(moduleName);
    }

  // XXX See issue #3804
  // Python maps SIGINT (control-c) to its own handler.  We will remap it
  // to the default so that control-c works. Note that this is already done in
  // "ctkAbstractPythonManager::initPythonQt" but the import of 'async'
  // module by 'gitdb' module (itself imported by the SlicerExtensionWizard)
  // resets the handler.
  #ifdef SIGINT
  signal(SIGINT, SIG_DFL);
  #endif

  emit this->modulesInstantiated(this->instantiatedModuleNames());
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerAbstractModuleFactoryManager
::instantiateModule(const QString& moduleName)
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  qSlicerModuleFactory* factory = d->registeredModuleFactory(moduleName);
  if (!factory)
    {
    qCritical() << "Fail to instantiate module " << moduleName << " (not registered)";
    return nullptr;
    }
  qSlicerAbstractCoreModule* module = factory->instantiate(moduleName);
  if (!module)
    {
    qCritical() << "Fail to instantiate module " << moduleName;
    return nullptr;
    }
  module->setName(moduleName);
  module->setObjectName(QString("%1Module").arg(moduleName));
  foreach(const QString& associatedNodeType, module->associatedNodeTypes())
    {
    qSlicerCoreApplication::application()->addModuleAssociatedNodeType(associatedNodeType, moduleName);
    }
  foreach(const QString& dependency, module->dependencies())
    {
    QStringList dependees = d->ModuleDependees.value(dependency);
    if (!dependees.contains(moduleName))
      {
      d->ModuleDependees.insert(dependency, dependees << moduleName);
      }
    }
  emit moduleInstantiated(moduleName);
  return module;
}

//-----------------------------------------------------------------------------
QStringList qSlicerAbstractModuleFactoryManager::registeredModuleNames() const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  return d->RegisteredModules.keys();
}

//-----------------------------------------------------------------------------
QStringList qSlicerAbstractModuleFactoryManager::instantiatedModuleNames() const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  QStringList instantiatedModules;
  foreach(const QString& moduleName, d->RegisteredModules.keys())
    {
    qSlicerModuleFactory* factory = d->registeredModuleFactory(moduleName);
    if (!factory)
      {
      continue;
      }
    if (factory->instance(moduleName))
      {
      instantiatedModules << moduleName;
      }
    }
  return instantiatedModules;
}


//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::uninstantiateModules()
{
  QStringList modulesToUninstantiate = this->instantiatedModuleNames();
  emit modulesAboutToBeUninstantiated(modulesToUninstantiate);
  foreach(const QString& name, modulesToUninstantiate)
    {
    this->uninstantiateModule(name);
    }
  emit modulesUninstantiated(modulesToUninstantiate);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::uninstantiateModule(const QString& moduleName)
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  if (d->Verbose)
    {
    qDebug() << "Uninstantiating:" << moduleName;
    }
  qSlicerModuleFactory* factory = d->registeredModuleFactory(moduleName);
  if (!factory)
    {
    qWarning() << "uninstantiateModule failed: module " << moduleName << " is not registered";
    return;
    }
  emit moduleAboutToBeUninstantiated(moduleName);
  factory->uninstantiate(moduleName);
  emit moduleUninstantiated(moduleName);
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerAbstractModuleFactoryManager::moduleInstance(const QString& moduleName)const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  qSlicerModuleFactory* factory = d->registeredModuleFactory(moduleName);
  return factory ? factory->instance(moduleName) : nullptr;
}

//-----------------------------------------------------------------------------
bool qSlicerAbstractModuleFactoryManager::isRegistered(const QString& moduleName)const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  return (d->registeredModuleFactory(moduleName) != nullptr);
}

//-----------------------------------------------------------------------------
bool qSlicerAbstractModuleFactoryManager::isInstantiated(const QString& moduleName)const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  bool instantiated = this->isRegistered(moduleName) &&
    d->RegisteredModules[moduleName]->instance(moduleName) != nullptr;
  return instantiated;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::setVerboseModuleDiscovery(bool verbose)
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  foreach (qSlicerModuleFactory* factory, d->Factories.keys())
    {
    factory->setVerbose(verbose);
    }
  this->setIsVerbose(verbose);
}

//---------------------------------------------------------------------------
QStringList qSlicerAbstractModuleFactoryManager::dependentModules(const QString& dependency)const
{
  QStringList dependents;
  foreach(const QString& moduleName, this->instantiatedModuleNames())
    {
    qSlicerAbstractCoreModule* coreModule = this->moduleInstance(moduleName);
    if (coreModule && coreModule->dependencies().contains(dependency))
      {
      dependents << moduleName;
      }
    }
  return dependents;
}

//---------------------------------------------------------------------------
QStringList qSlicerAbstractModuleFactoryManager::moduleDependees(const QString& module)const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  return d->ModuleDependees.value(module);
}

//---------------------------------------------------------------------------
bool  qSlicerAbstractModuleFactoryManager::isVerbose()const
{
  Q_D(const qSlicerAbstractModuleFactoryManager);
  return d->Verbose;
}

//---------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::setIsVerbose(bool flag)
{
  Q_D(qSlicerAbstractModuleFactoryManager);
  d->Verbose = flag;
}

