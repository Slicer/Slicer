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

#ifndef __qSlicerAbstractModuleFactoryManager_h
#define __qSlicerAbstractModuleFactoryManager_h

// Qt includes
#include <QObject>
#include <QString>

// CTK includes
#include <ctkAbstractFileBasedFactory.h>

#include "qSlicerBaseQTCoreExport.h"

class qSlicerAbstractCoreModule;

class qSlicerAbstractModuleFactoryManagerPrivate;

/// Loading modules into slicer happens in multiple steps:
/// 1) module factories must be registered into the factory manager:
///   qSlicerModuleFactoryManager* factoryManager = app->moduleManager()->factoryManager();
///   factoryManager->registerFactory(new qSlicerLoadableModuleFactory);
///   ...
/// 2) directories where the modules to load are located must be passed to the factory manager
///   factoryManager->addSearchPath(app->slicerHome() + "/" +  Slicer_QTSCRIPTEDMODULES_LIB_DIR + "/" );
///   factoryManager->addSearchPath(app->slicerHome() + "/" + Slicer_CLIMODULES_LIB_DIR + "/" );
///   ...
/// 3) Optionally:
///    - specify module names to ignore: For startup speed-up and memory consummation,
///    it can be useful to not load some modules:
///     factoryManager->setModulesToIgnore(QStringList(QLatin1String("Data")));
///    - specify an explicit list of modules to register/instantiate/load. All other discovered
///    modules won't be loaded.
/// 4) scan the directories and test which file is a module and register it (not instantiated yet)
/// For each file in the search paths, the factory manager asks each registered
/// factory if they can load the file. If there is a factory that supports the
/// file, the manager associates the factory to the file path, otherwise the
/// file is discarded.
///   factoryManager->registerModules();
/// 5) Instantiate all the registered modules
///   factoryManager->instantiateModules();
/// 6) Connect each module with the scene and the application
/// The application logic and the scene are passed to each module.
/// The order of initialization is defined with the dependencies of the modules.
/// If module B depends of module A, it is assured that module B is initialized/setup after A.
///   factoryManager->loadModules();
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerAbstractModuleFactoryManager : public QObject
{
  Q_OBJECT
  /// This property holds the paths where the modules are located.
  /// At registration time (registerModules), the paths are scanned and module
  /// discovered.
  /// A module can be a library (dll, so),
  /// an executable (exe), a python file (py) or any other file type supported
  /// by the registered factories.
  /// The search is not recursive, you need to provide each subdirectory
  /// manually.
  ///
  /// \todo In qSlicerAbstractModuleFactoryManager, should the module search recursively descend \a searchPaths
  Q_PROPERTY(QStringList searchPaths READ searchPaths WRITE setSearchPaths)

  /// This property holds the names of the modules to ignore at registration time.
  ///
  /// Due to the large amount of modules to load, it can be faster (and less
  /// overwhelming) to load only a subset of the modules.
  Q_PROPERTY(QStringList modulesToIgnore READ modulesToIgnore WRITE setModulesToIgnore NOTIFY modulesToIgnoreChanged)
public:
  typedef ctkAbstractFileBasedFactory<qSlicerAbstractCoreModule> qSlicerFileBasedModuleFactory;
  typedef ctkAbstractFactory<qSlicerAbstractCoreModule> qSlicerModuleFactory;

  typedef QObject Superclass;
  qSlicerAbstractModuleFactoryManager(QObject * newParent = nullptr);

  /// Destructor, Deallocates resources
  /// Unregister (and delete) all registered factories.
  ~qSlicerAbstractModuleFactoryManager() override;

  /// Print internal state using qDebug()
  virtual void printAdditionalInfo();

  /// \brief Register a \a factory
  /// The factory will be deleted when unregistered
  /// (e.g. in ~qSlicerAbstractModuleFactoryManager())
  ///
  /// Example:
  ///
  /// \code{.cpp}
  /// qSlicerAbstractModuleFactoryManager factoryManager;
  /// factoryManager.registerFactory(new qSlicerCoreModuleFactory);
  /// factoryManager.registerFactory(new qSlicerLoadableModuleFactory);
  /// \endcode
  ///
  /// The order in which factories are registered is important. When scanning
  /// directories, registered factories are browse and the first factory that
  /// can read a file is used.
  /// Priority is used when multiple factories can register the same module.
  /// The factory with the higher priority wins.
  void registerFactory(qSlicerModuleFactory* factory, int priority = 0);
  void unregisterFactory(qSlicerModuleFactory* factory);
  void unregisterFactories();

  void setSearchPaths(const QStringList& searchPaths);
  QStringList searchPaths()const;

  /// Utility function that adds a list of path to the current \a searchPaths
  /// list.
  inline void addSearchPaths(const QStringList& paths);
  /// Utility function that adds a path to the current \a searchPaths list.
  inline void addSearchPath(const QString& path);

  /// Utility function that removes a list of path to the current \a searchPaths
  ///  list.
  inline void removeSearchPaths(const QStringList& paths);

  /// Utility function that removes a path from the current \a searchPaths list.
  inline void removeSearchPath(const QString& path);

  void setExplicitModules(const QStringList& moduleNames);
  QStringList explicitModules()const;


  /// Set or get the \a modulesToIgnore list.
  ///
  /// If list is modified, the signal
  /// modulesToIgnoreChanged(const QStringLists&) is emitted.
  void setModulesToIgnore(const QStringList& modulesNames);
  QStringList modulesToIgnore()const;

  /// Utility function that adds a module to the \a modulesToIgnore list.
  /// \sa removeModuleToIgnore(const QString& moduleName)
  inline void addModuleToIgnore(const QString& moduleName);

  /// Utility function that removes a module to the \a modulesToIgnore list.
  /// \sa addModuleToIgnore(const QString& moduleName)
  inline void removeModuleToIgnore(const QString& moduleName);

  /// After the modules are registered, ignoredModules contains the list
  /// of all the modules that were ignored.
  QStringList ignoredModuleNames()const;

  /// Scan the paths in \a searchPaths and for each file, attempt to register
  /// using one of the registered factories.
  void registerModules();

  Q_INVOKABLE void registerModule(const QFileInfo& file);

  /// Convenient method returning the list of all registered module names
  Q_INVOKABLE QStringList registeredModuleNames() const;

  /// Return true if a module has been registered, false otherwise
  Q_INVOKABLE bool isRegistered(const QString& name)const;

  /// Instantiate all previously registered modules.
  virtual void instantiateModules();

  /// List of registered and instantiated modules
  Q_INVOKABLE QStringList instantiatedModuleNames() const;

  /// Return true if a module has been instantiated, false otherwise
  Q_INVOKABLE bool isInstantiated(const QString& name)const;

  /// Return the instance of a module if already instantiated, 0 otherwise
  Q_INVOKABLE qSlicerAbstractCoreModule* moduleInstance(const QString& moduleName)const;

  /// Uninstantiate all instantiated modules
  void uninstantiateModules();

  /// Enable/Disable verbose output during module discovery process
  void setVerboseModuleDiscovery(bool value);

  /// Return the list of modules that have \a module as a dependency.
  /// Note that the list can contain unloaded modules.
  /// \sa qSlicerAbstractCoreModule::dependencies(), moduleDependees()
  QStringList dependentModules(const QString& module)const;

  /// Return the list of modules that depend on \a module.
  /// Note that the list can contain unloaded modules.
  /// \sa dependentModules(), qSlicerAbstractCoreModule::dependencies()
  QStringList moduleDependees(const QString& module)const;

signals:
  /// \brief This signal is emitted when all the modules associated with the
  /// registered factories have been loaded
  void modulesRegistered(const QStringList& moduleNames);
  void moduleRegistered(const QString& moduleName);

  void explicitModulesChanged(const QStringList& moduleNames);

  void modulesToIgnoreChanged(const QStringList& moduleNames);
  void moduleIgnored(const QString& moduleName);

  void modulesInstantiated(const QStringList& moduleNames);
  void moduleInstantiated(const QString& moduleName);

  void modulesAboutToBeUninstantiated(const QStringList& moduleNames);
  void moduleAboutToBeUninstantiated(const QString& moduleName);

  void modulesUninstantiated(const QStringList& moduleNames);
  void moduleUninstantiated(const QString& moduleName);

public slots:
  /// Enable/disable local verbose output during module discovery and
  /// uninstantiating, set by setVerboseModuleDiscovery()
  /// \sa setVerboseModuleDiscovery()
  void setIsVerbose(bool flag);
  bool isVerbose()const;
protected:
  QScopedPointer<qSlicerAbstractModuleFactoryManagerPrivate> d_ptr;

  void registerModules(const QString& directoryPath);

  /// Instantiate a module given its \a name
  qSlicerAbstractCoreModule* instantiateModule(const QString& name);

  /// Uninstantiate a module given its \a moduleName
  virtual void uninstantiateModule(const QString& moduleName);

private:
  Q_DECLARE_PRIVATE(qSlicerAbstractModuleFactoryManager);
  Q_DISABLE_COPY(qSlicerAbstractModuleFactoryManager);
};

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::addSearchPaths(const QStringList& paths)
{
  this->setSearchPaths(this->searchPaths() << paths);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::addSearchPath(const QString& path)
{
  this->setSearchPaths(this->searchPaths() << path);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::removeSearchPaths(const QStringList& paths)
{
  foreach(const QString& path, paths)
    {
    this->removeSearchPath(path);
    }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::removeSearchPath(const QString& path)
{
  QStringList newSearchPaths = this->searchPaths();
  newSearchPaths.removeAll(path);
  this->setSearchPaths(newSearchPaths);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::addModuleToIgnore(const QString& moduleName)
{
  QStringList modules = this->modulesToIgnore();
  if (modules.contains(moduleName))
    {
    return;
    }
  modules << moduleName;
  this->setModulesToIgnore(modules);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleFactoryManager::removeModuleToIgnore(const QString& moduleName)
{
  QStringList modules = this->modulesToIgnore();
  modules.removeAll(moduleName);
  this->setModulesToIgnore(modules);
}

#endif
