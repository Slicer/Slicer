/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerModuleFactory_h
#define __qSlicerModuleFactory_h

// SlicerQT includes
#include "qSlicerAbstractModule.h"

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QFileInfo>

#include "qSlicerBaseQTCoreExport.h"

class qSlicerModuleFactoryPrivate; 

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleFactory
{
public:

  // Description:
  // Instantiate the factory
  // Note: Core modules are registred when the factory is instantiated
  qSlicerModuleFactory();

  // Description:
  // Destructor, Deallocates resources
  virtual ~qSlicerModuleFactory();

  virtual void printAdditionalInfo();

  // Description:
  // Get a moduleName given its title
  QString moduleName(const QString & moduleTitle) const;

  // Description:
  // Get a module title given its name
  QString moduleTitle(const QString & moduleName) const;

  // Description:
  // Instantiate all the registered core modules
  void instantiateCoreModules();

  // Description:
  // Instantiate all the registered core modules
  void instantiateLoadableModules();

  // Description:
  // Instantiate all the registered core modules
  void instantiateCmdLineModules();

  // Description:
  // Instantiate a module given its name
  qSlicerAbstractModule* instantiateModule(const QString& moduleName);

  // Description:
  // Uninstantiate a module given its name
  void uninstantiateModule(const QString& moduleName);

  // Description:
  // Uninstantiate all registered modules
  void uninstantiateAll();

  // Description:
  // Indicate if a module has been registered
  bool isRegistered(const QString& moduleName)const;

  // Description:
  // Register the list of available core modules
  void registerCoreModules();

  // Description:
  // Set/Get paths where the loadable modules are located
  void setLoadableModuleSearchPaths(const QStringList& paths);
  QStringList loadableModuleSearchPaths() const;

  // Description:
  // Register all loadable modules discovered using the loadableModuleSearchPaths.
  // Note: For each entries in the searchPaths, if it's a valid library (check
  // execution flag and name), the method registerLoadableModule will be called.
  void registerLoadableModules();

  // Description:
  // Register a loadable module indicating its name
  void registerLoadableModule(const QFileInfo& fileInfo);

  // Description:
  // Set/Get paths where the loadable modules are located
  void setCmdLineModuleSearchPaths(const QStringList& paths);
  QStringList cmdLineModuleSearchPaths() const;

  // Description:
  void registerCmdLineModules();
  void registerCmdLineModule(const QFileInfo& fileInfo);

  // Description:
  // Return the list of all core/loadable/commandLine module names
  QStringList coreModuleNames() const;
  QStringList loadableModuleNames() const;
  QStringList commandLineModuleNames() const;

  // Description:
  // Convenient method returning the list of all module names
  QStringList moduleNames() const;


protected:

  // Description:
  // Add a module class to the core module factory
  template<typename ClassType>
  void registerCoreModule();

private:
  QCTK_DECLARE_PRIVATE(qSlicerModuleFactory);
};

#endif
