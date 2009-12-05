#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h

// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerMeasurementsModuleWin32Header.h"

class qSlicerMeasurementsModulePrivate;

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsModule :
  public qSlicerAbstractLoadableModule
{
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerMeasurementsModule(QObject *parent=0);
  virtual ~qSlicerMeasurementsModule(){}
  
  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:

  // Description:
  // Create and return a widget representation of the object
  virtual qSlicerAbstractModuleWidget * createWidgetRepresentation();

private:
  QCTK_DECLARE_PRIVATE(qSlicerMeasurementsModule); 
};

#endif
