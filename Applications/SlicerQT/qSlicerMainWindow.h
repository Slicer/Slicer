#ifndef __qSlicerMainWindow_h
#define __qSlicerMainWindow_h 

// Qt includes
#include <QMainWindow>

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerQTExport.h"

class qSlicerAbstractCoreModule;
class qSlicerModulePanel;
class qSlicerModuleSelectorToolBar;
class qSlicerMainWindowCore;
class qSlicerMainWindowPrivate;

class Q_SLICERQT_EXPORT qSlicerMainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  
  typedef QMainWindow Superclass;
  qSlicerMainWindow(QWidget *parent=0);
  
  // Description:
  // Return the main window core.
  qSlicerMainWindowCore* core()const;

  // Description:
  // Return the module panel
  qSlicerModulePanel* modulePanel() const;

protected slots:
  void onModuleLoaded(qSlicerAbstractCoreModule* module);
  void onModuleAboutToBeUnloaded(qSlicerAbstractCoreModule* module);

protected:

  // Description:
  // Connect MainWindow action with slots defined in MainWindowCore
  void setupMenuActions();

private:
  CTK_DECLARE_PRIVATE(qSlicerMainWindow);
};

#endif
