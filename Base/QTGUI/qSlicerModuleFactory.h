#ifndef __qSlicerModuleFactory_h
#define __qSlicerModuleFactory_h

#include "qSlicerAbstractModule.h"
#include "qSlicerBaseQTGUIWin32Header.h"

#include <QFileInfo>

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleFactory
{
public:

  // Description:
  // Instantiate the factory
  // Note: Core modules are registred when the factory is instantiated
  qSlicerModuleFactory();
  virtual ~qSlicerModuleFactory();

  virtual void printAdditionalInfo();

  // Description:
  // Get a moduleName given its title
  QString getModuleName(const QString & moduleTitle) const;

  // Description:
  // Get a module title given its name
  QString getModuleTitle(const QString & moduleName) const;

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
  struct qInternal;
  qInternal* Internal;
};

#endif
