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

#ifndef __qSlicerModuleFactoryManager_h
#define __qSlicerModuleFactoryManager_h

// Qt includes
#include <QStringList>

// Slicer includes
#include "qSlicerAbstractModuleFactoryManager.h"
class qSlicerModuleFactoryManagerPrivate;

// Slicer logics includes
class vtkSlicerApplicationLogic;

// MRML includes
class vtkMRMLScene;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleFactoryManager
  : public qSlicerAbstractModuleFactoryManager
{
  Q_OBJECT
public:
  typedef qSlicerAbstractModuleFactoryManager Superclass;
  qSlicerModuleFactoryManager(QObject* newParent = nullptr);

  /// Unloads all the modules previously loaded.
  ~qSlicerModuleFactoryManager() override;

  void printAdditionalInfo() override;

  /// Load all the instantiated modules.
  /// To register and initialize modules, please use
  /// qSlicerModuleFactoryManager::registerModules();
  /// qSlicerModuleFactoryManager::initializeModules();
  /// Returns the number of loaded modules.
  /// \sa qSlicerModuleFactoryManager::registerModules()
  /// \sa qSlicerModuleFactoryManager::instantiateModules()
  Q_INVOKABLE int loadModules();

  /// Return the list of all the loaded modules
  Q_INVOKABLE QStringList loadedModuleNames()const;

  /// Unload all the loaded modules. Unloading a module simply uninstantiate it.
  /// To respect dependencies, the order is reverse to the
  /// order of load.
  Q_INVOKABLE void unloadModules();

  /// Return true if module \a name has been loaded, false otherwise
  Q_INVOKABLE bool isLoaded(const QString& name)const;

  /// Return the loaded module identified by \a name, 0 if no module
  /// has been loaded yet, even if the module has been instantiated.
  Q_INVOKABLE qSlicerAbstractCoreModule* loadedModule(const QString& name)const;

  /// Set the application logic to pass to modules at "load" time.
  void setAppLogic(vtkSlicerApplicationLogic* applicationLogic);
  vtkSlicerApplicationLogic* appLogic()const;

  /// Return the mrml scene passed to loaded modules
  vtkMRMLScene* mrmlScene()const;

  /// Load specified modules.
  ///
  /// This attempts to load the specified modules, instantiating them first if
  /// necessary.
  Q_INVOKABLE bool loadModules(const QStringList& modules);

  /// Load module identified by \a name
  /// \todo move it as protected
  bool loadModule(const QString& name);

  /// Return all module paths that are direct child of \a basePath.
  QStringList modulePaths(const QString& basePath);

public slots:
  /// Set the MRML scene to pass to modules at "load" time.
  void setMRMLScene(vtkMRMLScene* mrmlScene);

signals:

  void modulesLoaded(const QStringList& modulesNames);
  void moduleLoaded(const QString& moduleName);

  void modulesAboutToBeUnloaded(const QStringList& modulesNames);
  void moduleAboutToBeUnloaded(const QString& moduleName);

  void modulesUnloaded(const QStringList& modulesNames);
  void moduleUnloaded(const QString& moduleName);

  void mrmlSceneChanged(vtkMRMLScene* newScene);
protected:
  QScopedPointer<qSlicerModuleFactoryManagerPrivate> d_ptr;

  bool loadModule(const QString& name, const QString& dependee);

  /// Unload module identified by \a name
  void unloadModule(const QString& name);

  /// Uninstantiate a module given its \a moduleName
  void uninstantiateModule(const QString& moduleName) override;

  /// Reimplemented to ensure order
  virtual void uninstantiateModules();
private:
  Q_DECLARE_PRIVATE(qSlicerModuleFactoryManager);
  Q_DISABLE_COPY(qSlicerModuleFactoryManager);
};

#endif
