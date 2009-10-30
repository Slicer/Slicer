#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h 

#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerMeasurementsModule : public qSlicerAbstractLoadableModule
{ 
  Q_OBJECT

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerMeasurementsModule(QWidget *parent=0);
  virtual ~qSlicerMeasurementsModule(); 
  
  virtual void dumpObjectInfo();
  
  qSlicerGetModuleTitleDeclarationMacro();

private:
  class qInternal;
  qInternal* Internal;
};

#endif
