#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h 

#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerMeasurementsModuleWin32Header.h"

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsModule : 
  public qSlicerAbstractLoadableModule
{ 
  Q_OBJECT

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerMeasurementsModule(QWidget *parent=0);
  virtual ~qSlicerMeasurementsModule(); 
  
  virtual void printAdditionalInfo();
  
  qSlicerGetModuleTitleDeclarationMacro();

private:
  class qInternal;
  qInternal* Internal;
};

#endif
