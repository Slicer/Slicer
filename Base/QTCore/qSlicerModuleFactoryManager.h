/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <QObject>
#include <QString>

// CTK includes
#include <ctkAbstractFactory.h>

#include "qSlicerBaseQTCoreExport.h"

class qSlicerAbstractCoreModule;

class qSlicerModuleFactoryManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleFactoryManager : public QObject
{
  Q_OBJECT
public:
  typedef ctkAbstractFactory<qSlicerAbstractCoreModule> qSlicerAbstractModuleFactory;
 
  typedef QObject Superclass;
  qSlicerModuleFactoryManager(QObject * newParent = 0);

  /// Destructor, Deallocates resources
  virtual ~qSlicerModuleFactoryManager();

  /// Print internal state using qDebug()
  virtual void printAdditionalInfo();

  /// \brief Register a \a factory
  /// \a factoryName will be used to get the reference of the registered factory.
  void registerFactory(const QString& factoryName, qSlicerAbstractModuleFactory* factory);

  /// Register all modules
  void registerAllModules();

  /// Register modules associated with factory identified by \a factoryName
  void registerModules(const QString& factoryName);

  /// Instanciate all modules
  void instantiateAllModules();
  
  /// Instanciate modules associated with factory identified by \a factoryName
  void instantiateModules(const QString& factoryName);

  /// Get a moduleName given its \a title
  QString moduleName(const QString & title) const;

  /// Get a module title given its \a name
  QString moduleTitle(const QString & name) const;

  /// Convenient method returning the list of all registered module names
  Q_INVOKABLE QStringList moduleNames() const;

  /// Convenient method returning the list of module names for the factory identified by \a factoryName
  Q_INVOKABLE QStringList moduleNames(const QString& factoryName) const;

  /// List of registered and instantiated modules
  Q_INVOKABLE QStringList instantiatedModuleNames() const;

  /// Instantiate a module given its \a name
  qSlicerAbstractCoreModule* instantiateModule(const QString& name);

  /// Uninstantiate a module given its \a name
  void uninstantiateModule(const QString& name);

  /// Uninstantiate all registered modules
  void uninstantiateAll();

  /// Indicate if a module has been registered
  Q_INVOKABLE bool isRegistered(const QString& name)const;

  /// Indicate if a module has been instantiated
  Q_INVOKABLE bool isInstantiated(const QString& name)const;

  /// Enable/Disable verbose output during module discovery process
  void setVerboseModuleDiscovery(bool value);

signals:
  /// \brief This signal is emitted when all the modules associated with the
  /// registered factories have been loaded
  void allModulesRegistered();

protected:
  QScopedPointer<qSlicerModuleFactoryManagerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModuleFactoryManager);
  Q_DISABLE_COPY(qSlicerModuleFactoryManager);
};

#endif
