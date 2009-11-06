#ifndef __qSlicerMainWindowCore_h
#define __qSlicerMainWindowCoreCore_h 

#include "qSlicerQTWin32Header.h"

#include <QObject>

class QMainWindow; 

class Q_SLICERQT_EXPORT qSlicerMainWindowCore : public QObject
{
  Q_OBJECT
  
public:
  
  typedef QObject Superclass;
  qSlicerMainWindowCore(QMainWindow *parent);
  virtual ~qSlicerMainWindowCore();

public slots: 
  // Description:
  // Handle actions
  
protected:
  QMainWindow* widget();

private:
  struct qInternal;
  qInternal* Internal;
};

#endif
