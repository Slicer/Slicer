#ifndef __qSlicerModuleFactory_h
#define __qSlicerModuleFactory_h 

#include "qSlicerAbstractModule.h"
#include "qSlicerBaseQTGUIWin32Header.h"

#include <QFileInfo>

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleFactory
{ 
public:
  
  qSlicerModuleFactory();
  virtual ~qSlicerModuleFactory(); 
  
  virtual void printAdditionalInfo();
  
  // Description:
  // Get a moduleName given its title
  QString getModuleName(const QString & moduleTitle);
  
  // Description:
  // Get a module title given its name
  QString getModuleTitle(const QString & moduleName); 
  
  // Description:
  // Instanciate a module given its name
  qSlicerAbstractModule* initializeModule(const QString& moduleName); 
  
  // Description:
  // Set/Get paths where the loadable modules are located
  void setLoadableModuleSearchPaths(const QStringList& paths); 
  QStringList loadableModuleSearchPaths();
  
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
  QStringList cmdLineModuleSearchPaths();
  
  // Description:
  void registerCmdLineModules(); 
  void registerCmdLineModule(const QFileInfo& fileInfo);
  
  // Description:
  // Return the list of all core/loadable modules
  QStringList coreModuleNames();
  QStringList loadableModuleNames();
  QStringList commandLineModuleNames();
  
  
protected:
  
  // Description:
  // Register the list of available core modules
  void registerCoreModules();

  // Description:
  // Add a module class to the core module factory
  template<typename ClassType>
  void registerCoreModule(); 

private:
  class qInternal;
  qInternal* Internal;
};

#endif
