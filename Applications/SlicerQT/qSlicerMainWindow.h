#ifndef __qSlicerMainWindow_h
#define __qSlicerMainWindow_h 

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QMainWindow>

#include "qSlicerQTWin32Header.h"

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
  // Return the moduleToolBar
  QToolBar* moduleToolBar()const;
  
protected:

  // Description:
  // Connect menu action with slots defined in MainWindowCore
  void setupMenuActions();

private:
  QCTK_DECLARE_PRIVATE(qSlicerMainWindow);
};

#endif
