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
  void onEditUndoActionTriggered();
  void onEditRedoActionTriggered();
  void onWindowPythonInteractorActionTriggered();
  
protected:
  qSlicerMainWindow* widget() const;

private:
  CTK_DECLARE_PRIVATE(qSlicerMainWindowCore);
};

#endif
