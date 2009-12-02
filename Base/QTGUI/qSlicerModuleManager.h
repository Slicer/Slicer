#ifndef __qSlicerModuleManager_h
#define __qSlicerModuleManager_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#include "qSlicerBaseQTGUIWin32Header.h"

class qSlicerAbstractModule;
class qSlicerModuleManagerInitialize;
class qSlicerModuleFactory;
class qSlicerAbstractModulePanel;
class qSlicerModuleManagerPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleManager : public QObject
{
  Q_OBJECT
public:
  typedef qSlicerModuleManager Self;

  // Description:
  static qSlicerModuleManager* instance();

  virtual void printAdditionalInfo();

  // Description:
  qSlicerModuleFactory* factory();

  // Description:
  bool loadModule(const QString& moduleTitle);
  bool loadModuleByName(const QString& moduleName);

  // Description:
  bool unLoadModule(const QString& moduleTitle);
  bool unLoadModuleByName(const QString& moduleName);

  // Description:
  bool isLoaded(const QString& moduleTitle);

  // Description:
  qSlicerAbstractModule* getModule(const QString& moduleTitle);
  qSlicerAbstractModule* getModuleByName(const QString& moduleName);

  // Description:
  // Get module title given it's name
  const QString moduleTitle(const QString& moduleName);

  // Description:
  // Show a module
  // Note: A module is contained in the module panel associated with the module manager.
  // The module panel will update itself each time a module is shown.
  void showModule(const QString& moduleTitle);

  // Description:
  // Hide/Show the module panel.
  // Note: The module panel contains a module and is responsible to display
  // the help and about frames.
  void setModulePanelVisible(bool visible);

  // Description:
  // Set module panel geometry
  void setModulePanelGeometry(int ax, int ay, int aw, int ah);

  // Description:
  // Return a reference to the Module Panel
  qSlicerAbstractModulePanel* modulePanel() const;

protected:
  qSlicerModuleManager();
  ~qSlicerModuleManager(){}

  static qSlicerModuleManager* Instance;

  // Description:
  static void classInitialize();
  static void classFinalize();

  friend class qSlicerModuleManagerInitialize;

private:
  qSlicerModuleManager(const qSlicerModuleManager&);  // Not implemented.
  void operator=(const qSlicerModuleManager&);  // Not implemented.

  QCTK_DECLARE_PRIVATE(qSlicerModuleManager);
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
