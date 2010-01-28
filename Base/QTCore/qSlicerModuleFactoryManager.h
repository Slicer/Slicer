/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerModuleFactoryManager_h
#define __qSlicerModuleFactoryManager_h

// qCTK includes
#include <qCTKAbstractFactory.h>
#include <qCTKPimpl.h>

// QT includes
#include <QString>

#include "qSlicerBaseQTCoreExport.h"

class qSlicerAbstractModule;

class qSlicerModuleFactoryManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleFactoryManager
{
public:
  typedef qCTKAbstractFactory<qSlicerAbstractModule> qSlicerAbstractModuleFactory;
 
  qSlicerModuleFactoryManager();

  // Description:
  // Destructor, Deallocates resources
  virtual ~qSlicerModuleFactoryManager();

  virtual void printAdditionalInfo();

  // Description:
  // Register a module factory
  void registerFactory(const QString& factoryName, qSlicerAbstractModuleFactory* factory);

  // Description:
  // Register all modules
  void registerAllModules();

  // Register modules for the given factory
  void registerModules(const QString& factoryName);

  // Instanciate all modules
  void instantiateAllModules();
  
  // Instanciate modules for a given factory
  void instantiateModules(const QString& factoryName);

  // Description:
  // Get a moduleName given its title
  QString moduleName(const QString & title) const;

  // Description:
  // Get a module title given its name
  QString moduleTitle(const QString & name) const;
  
  // Description:
  // Convenient method returning the list of all module names
  QStringList moduleNames() const;

  // Description:
  // Convenient method returning the list of module names for a given factory
  QStringList moduleNames(const QString& factoryName) const;

  // Description:
  // Instantiate a module given its name
  qSlicerAbstractModule* instantiateModule(const QString& name);

  // Description:
  // Uninstantiate a module given its name
  void uninstantiateModule(const QString& name);

  // Description:
  // Uninstantiate all registered modules
  void uninstantiateAll();

  // Description:
  // Indicate if a module has been registered
  bool isRegistered(const QString& name)const;

private:
  QCTK_DECLARE_PRIVATE(qSlicerModuleFactoryManager);
};

#endif
