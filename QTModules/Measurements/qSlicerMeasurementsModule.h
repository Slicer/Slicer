#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h

#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerMeasurementsModuleWin32Header.h"

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsModule :
  public qSlicerAbstractLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule)

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerMeasurementsModule(QWidget *parent=0);
  virtual ~qSlicerMeasurementsModule();

  virtual void printAdditionalInfo();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:
  virtual void setup();

private:
  class qInternal;
  qInternal* Internal;
};

#endif
