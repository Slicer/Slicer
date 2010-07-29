#ifndef __qSlicerMainWindow_h
#define __qSlicerMainWindow_h 

// Qt includes
#include <QMainWindow>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include "qSlicerQTExport.h"

class qSlicerAbstractCoreModule;
class qSlicerModulePanel;
class qSlicerModuleSelectorToolBar;
class qSlicerMainWindowCore;
class qSlicerMainWindowPrivate;

class Q_SLICERQT_EXPORT qSlicerMainWindow : public QMainWindow
{
  Q_OBJECT
  QVTK_OBJECT
public:
  
  typedef QMainWindow Superclass;
  qSlicerMainWindow(QWidget *parent=0);
  
  // Description:
  // Return the main window core.
  qSlicerMainWindowCore* core()const;

  /// Return the module selector
  qSlicerModuleSelectorToolBar* moduleSelector()const;

public slots:
  void showMainToolBar(bool visible);
  void showUndoRedoToolBar(bool visible);
  void showViewToolBar(bool visible);
  void showLayoutToolBar(bool visible);
  void showMouseModeToolBar(bool visible);
  void showModuleToolBar(bool visible);
  void showModuleSelectorToolBar(bool visible);

protected slots:
  void onModuleLoaded(qSlicerAbstractCoreModule* module);
  void onModuleAboutToBeUnloaded(qSlicerAbstractCoreModule* module);

  void onMRMLSceneModified(vtkObject*);
protected:

  // Description:
  // Connect MainWindow action with slots defined in MainWindowCore
  void setupMenuActions();

private:
  CTK_DECLARE_PRIVATE(qSlicerMainWindow);
};

#endif
