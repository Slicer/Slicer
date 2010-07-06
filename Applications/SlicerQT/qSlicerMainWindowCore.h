#ifndef __qSlicerMainWindowCore_h
#define __qSlicerMainWindowCore_h 

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerQTExport.h"

class qSlicerMainWindow; 
class qSlicerMainWindowCorePrivate;

class Q_SLICERQT_EXPORT qSlicerMainWindowCore : public QObject
{
  Q_OBJECT
  
public:
  typedef QObject Superclass;
  qSlicerMainWindowCore(qSlicerMainWindow *parent);

public slots: 
  /// 
  /// Handle actions - See qSlicerMainWindow::setupMenuActions
  void onFileAddDataActionTriggered();
  void onFileImportSceneActionTriggered();
  void onFileLoadSceneActionTriggered();
  void onFileAddVolumeActionTriggered();
  void onFileAddTransformActionTriggered();
  void onFileSaveSceneActionTriggered();
  void onEditUndoActionTriggered();
  void onEditRedoActionTriggered();  
  void onHelpAboutSlicerQTActionTriggered();
  void onViewLayoutConventionalActionTriggered();
  void onViewLayoutFourUpActionTriggered();
  void onViewLayoutDual3DActionTriggered();
  void onViewLayoutOneUp3DActionTriggered();
  void onViewLayoutOneUpRedSliceActionTriggered();
  void onViewLayoutOneUpYellowSliceActionTriggered();
  void onViewLayoutOneUpGreenSliceActionTriggered();
  void onViewLayoutTabbed3DActionTriggered();
  void onViewLayoutTabbedSliceActionTriggered();
  void onViewLayoutCompareActionTriggered();
  void onViewLayoutSideBySideCompareActionTriggered();
  void onWindowPythonInteractorActionTriggered();


protected:
  qSlicerMainWindow* widget() const;

private:
  CTK_DECLARE_PRIVATE(qSlicerMainWindowCore);
};

#endif
