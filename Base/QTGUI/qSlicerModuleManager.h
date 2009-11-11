#ifndef __qSlicerModuleManager_h
#define __qSlicerModuleManager_h 

#include <QObject>

#include "qSlicerBaseQTGUIWin32Header.h"

class qSlicerAbstractModule; 
class qSlicerModuleManagerInitialize; 
class qSlicerModuleFactory;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleManager
{ 
public:
  typedef qSlicerModuleManager Self; 
  
  // Description:
  static qSlicerModuleManager* instance();
  
  virtual void printAdditionalInfo();
  
  // Description:
  qSlicerModuleFactory* factory();
  
  // Description:
  qSlicerAbstractModule* loadModule(const QString& moduleTitle); 
  qSlicerAbstractModule* loadModuleByName(const QString& moduleName);
  
  // Description:
  void unLoadModule(const QString& moduleTitle);
  void unLoadModuleByName(const QString& moduleName); 
  
  // Description:
  qSlicerAbstractModule* getModule(const QString& moduleTitle); 
  qSlicerAbstractModule* getModuleByName(const QString& moduleName); 
  
  // Description:
  // Get module title given it's name
  const QString moduleTitle(const QString& moduleName); 
  
  // Description:
  // Hide / Show module
  void showModule(const QString& moduleTitle); 
  void hideModule(const QString& moduleTitle); 
  
protected:
  qSlicerModuleManager();
  ~qSlicerModuleManager(); 
  
  static qSlicerModuleManager* Instance;
  
  // Description:
  static void classInitialize();
  static void classFinalize();
  
  friend class qSlicerModuleManagerInitialize;

private:
  qSlicerModuleManager(const qSlicerModuleManager&);  // Not implemented.
  void operator=(const qSlicerModuleManager&);  // Not implemented.
  
  class qInternal;
  qInternal* Internal;
};


// Utility class to make sure qSlicerModuleManager is initialized before it is used.
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleManagerInitialize
{
public:
  typedef qSlicerModuleManagerInitialize Self; 
  
  qSlicerModuleManagerInitialize();
  ~qSlicerModuleManagerInitialize();
private:
  static unsigned int Count;
};

// This instance will show up in any translation unit that uses
// qSlicerModuleManager.  It will make sure qSlicerModuleManager is initialized
// before it is used.
static qSlicerModuleManagerInitialize qSlicerModuleManagerInitializer;

#endif
