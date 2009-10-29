#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h 

#include "qSlicerAbstractModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerMeasurementsModule : public qSlicerAbstractModule
{ 
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerMeasurementsModule(QWidget *parent=0);
  virtual ~qSlicerMeasurementsModule(); 
  
  virtual void dumpObjectInfo();
  
  qSlicerGetModuleTitleDeclarationMacro();

private:
  class qInternal;
  qInternal* Internal;
};

#endif
