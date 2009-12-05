#ifndef __qSlicerModuleManager_h
#define __qSlicerModuleManager_h

#include "qSlicerCoreModuleManager.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreWin32Header.h"

class qSlicerAbstractModule;
class qSlicerModuleManagerInitialize;
class qSlicerModuleFactory;
class qSlicerAbstractModulePanel;
class QToolBar;

class qSlicerModuleManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleManager : public qSlicerCoreModuleManager
{
  Q_OBJECT
public:
  typedef qSlicerModuleManager Self;
  typedef qSlicerCoreModuleManager Superclass;
  qSlicerModuleManager();
  virtual ~qSlicerModuleManager(){}

  virtual void printAdditionalInfo();

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

public slots:

  // Description:
  // Show a module
  // Note: A module is contained in the module panel associated with the module manager.
  // The module panel will update itself each time a module is shown.
  void showModule(const QString& moduleTitle);
  void showModuleByName(const QString& moduleName);

private:
  qSlicerModuleManager(const qSlicerModuleManager&);  // Not implemented.
  void operator=(const qSlicerModuleManager&);  // Not implemented.

  QCTK_DECLARE_PRIVATE(qSlicerModuleManager);
};

#endif
