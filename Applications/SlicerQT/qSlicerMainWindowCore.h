#ifndef __qSlicerMainWindowCore_h
#define __qSlicerMainWindowCoreCore_h 

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#include "qSlicerQTWin32Header.h"

class QMainWindow; 
class qSlicerMainWindowCorePrivate;

class Q_SLICERQT_EXPORT qSlicerMainWindowCore : public QObject
{
  Q_OBJECT
  
public:
  typedef QObject Superclass;
  qSlicerMainWindowCore(QMainWindow *parent);

public slots: 
  // Description:
  // Handle actions
  
protected:
  QMainWindow* widget() const;

private:
  QCTK_DECLARE_PRIVATE(qSlicerMainWindowCore);
};

#endif
