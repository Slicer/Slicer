#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h

// Slicer includes
#include "qSlicerAbstractLoadableModule.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerMeasurementsModuleWin32Header.h"

class qSlicerMeasurementsModulePrivate;

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsModule :
  public qSlicerAbstractLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerMeasurementsModule(QWidget *parent=0);

  virtual void printAdditionalInfo();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  virtual QAction* showModuleAction();

protected:
  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerMeasurementsModule); 
};

#endif
