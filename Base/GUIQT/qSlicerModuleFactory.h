#ifndef __qSlicerModuleFactory_h
#define __qSlicerModuleFactory_h 

#include "qSlicerAbstractModule.h"
#include "qSlicerBaseGUIQTWin32Header.h"

#include <qCTKAbstractObjectFactory.h>

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerModuleFactory : 
  protected qCTKAbstractObjectFactory<qSlicerAbstractModule, QString>
{ 
public:
  
  typedef qCTKAbstractObjectFactory<qSlicerAbstractModule, QString> Superclass;
  qSlicerModuleFactory();
  virtual ~qSlicerModuleFactory(); 
  
  virtual void dumpObjectInfo();
  
  // Description:
  // Get a moduleName given its title
  QString getModuleName(const QString & moduleTitle);
  
  // Description:
  // Get a module title given its name
  QString getModuleTitle(const QString & moduleName); 
  
  // Description:
  // Instanciate a module given its name
  qSlicerAbstractModule* createModule(const QString& moduleName); 

protected:
  
  // Description:
  // Register the list of available core modules
  void registerCoreModules();
  
  // Description:
  // Add a module class to the factory
  // Note: A valid module should expose a static method having the following signature:
  //  static const QString moduleTitle(); 
  template<typename ClassType>
  void registerModule(/*const QString& moduleTitle*/); 

private:
  class qInternal;
  qInternal* Internal;
};

#endif
