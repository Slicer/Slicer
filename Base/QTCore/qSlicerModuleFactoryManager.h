/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerModuleFactoryManager_h
#define __qSlicerModuleFactoryManager_h

// Qt includes
#include <QString>

// CTK includes
#include <ctkAbstractFactory.h>
#include <ctkPimpl.h>

#include "qSlicerBaseQTCoreExport.h"

class qSlicerAbstractModule;

class qSlicerModuleFactoryManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleFactoryManager
{
public:
  typedef ctkAbstractFactory<qSlicerAbstractModule> qSlicerAbstractModuleFactory;
 
  qSlicerModuleFactoryManager();

  /// 
  /// Destructor, Deallocates resources
  virtual ~qSlicerModuleFactoryManager();

  virtual void printAdditionalInfo();

  /// 
  /// Register a module factory
  void registerFactory(const QString& factoryName, qSlicerAbstractModuleFactory* factory);

  /// 
  /// Register all modules
  void registerAllModules();

  /// Register modules for the given factory
  void registerModules(const QString& factoryName);

  /// Instanciate all modules
  void instantiateAllModules();
  
  /// Instanciate modules for a given factory
  void instantiateModules(const QString& factoryName);

  /// 
  /// Get a moduleName given its title
  QString moduleName(const QString & title) const;

  /// 
  /// Get a module title given its name
  QString moduleTitle(const QString & name) const;
  
  /// 
  /// Convenient method returning the list of all module names
  QStringList moduleNames() const;

  /// 
  /// Convenient method returning the list of module names for a given factory
  QStringList moduleNames(const QString& factoryName) const;

  /// 
  /// Instantiate a module given its name
  qSlicerAbstractModule* instantiateModule(const QString& name);

  /// 
  /// Uninstantiate a module given its name
  void uninstantiateModule(const QString& name);

  /// 
  /// Uninstantiate all registered modules
  void uninstantiateAll();

  /// 
  /// Indicate if a module has been registered
  bool isRegistered(const QString& name)const;

  ///
  /// Enable/Disable verbose output during module discovery process
  void setVerboseModuleDiscovery(bool value);

private:
  CTK_DECLARE_PRIVATE(qSlicerModuleFactoryManager);
};

#endif
