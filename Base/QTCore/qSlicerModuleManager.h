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

#ifndef __qSlicerModuleManager_h
#define __qSlicerModuleManager_h

// CTK includes
#include <ctkPimpl.h>

// Qt includes
#include <QObject>

#include "qSlicerBaseQTCoreExport.h"

class qSlicerAbstractCoreModule;
class qSlicerModuleFactoryManager;

class qSlicerModuleManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleManager : public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  qSlicerModuleManager(QObject* newParent = 0);
  virtual ~qSlicerModuleManager();

  /// Print internal state using qDebug()
  virtual void printAdditionalInfo();

  /// Return a pointer to the current module factory manager
  Q_INVOKABLE qSlicerModuleFactoryManager * factoryManager()const;

  /// Load module identified by \a name
  Q_INVOKABLE bool loadModule(const QString& name);

  /// Unload module identified by \a name
  Q_INVOKABLE bool unLoadModule(const QString& name);

  /// Return True if module \a name has been loaded
  Q_INVOKABLE bool isLoaded(const QString& name)const;

  /// Load all the registered modules. Instantiate them if not instantiated
  /// yet.
  /// To register modules, please use
  /// qSlicerModuleFactoryManager::registerAllModules()
  /// Returns the number of loaded modules.
  /// \sa qSlicerModuleFactoryManager::registerAllModules()
  /// \sa qSlicerModuleFactoryManager::instantiateAllModules()
  Q_INVOKABLE int loadAllModules();

  /// Return the loaded module identified by \a name
  Q_INVOKABLE qSlicerAbstractCoreModule* module(const QString& name);

  /// Convenient method to get module title given its name
  /// \deprecated
  QString moduleTitle(const QString& name) const;

  /// Convenient method to get module name given its title
  /// \deprecated
  QString moduleName(const QString& title) const;

  /// Return the list of all the registered modules
  QStringList moduleList()const;

  /// Return the list of all the loaded modules
  QStringList loadedModules()const;

signals:
  void moduleLoaded(qSlicerAbstractCoreModule* module);
  void moduleAboutToBeUnloaded(qSlicerAbstractCoreModule* module);

protected:
  QScopedPointer<qSlicerModuleManagerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModuleManager);
  Q_DISABLE_COPY(qSlicerModuleManager);
};

#endif
