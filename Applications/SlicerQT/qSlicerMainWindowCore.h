#ifndef __qSlicerMainWindowCore_h
#define __qSlicerMainWindowCore_h 

// QT includes
#include <QObject>

// qCTK includes
#include <qCTKPimpl.h>

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
  // Description:
  // Handle actions
  void onEditUndoActionTriggered();
  void onEditRedoActionTriggered();
  void onWindowPythonInteractorActionTriggered();
  
protected:
  qSlicerMainWindow* widget() const;

private:
  QCTK_DECLARE_PRIVATE(qSlicerMainWindowCore);
};

#endif
