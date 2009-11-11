#ifndef __qSlicerModuleLogic_h
#define __qSlicerModuleLogic_h 

#include <QObject>
#include "qSlicerMacros.h"

#include "qSlicerBaseQTBaseWin32Header.h"

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerModuleLogic : public QObject
{ 
  Q_OBJECT
public:
  
  typedef QObject Superclass;
  qSlicerModuleLogic(QObject *parent=0);
  virtual ~qSlicerModuleLogic();
  
  virtual void printAdditionalInfo(); 

private:

  class qInternal;
  qInternal* Internal;
};

#endif
