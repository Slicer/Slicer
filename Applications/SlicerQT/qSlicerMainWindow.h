#ifndef __qSlicerMainWindow_h
#define __qSlicerMainWindow_h 

#include <QMainWindow>

#include "qSlicerQTWin32Header.h"

class qSlicerMainWindowCore; 

class Q_SLICERQT_EXPORT qSlicerMainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  
  typedef QMainWindow Superclass;
  qSlicerMainWindow(QWidget *parent=0);
  virtual ~qSlicerMainWindow();
  
  // Description:
  // Return the main window core.
  qSlicerMainWindowCore* core(); 
  
protected:

  // Description:
  // Connect menu action with slots defined in MainWindowCore
  void setupMenuActions();

private:
  struct qInternal;
  qInternal* Internal;
};

#endif
