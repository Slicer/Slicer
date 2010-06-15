#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerMeasurementsModuleExport.h"

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

  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  CTK_DECLARE_PRIVATE(qSlicerMeasurementsModule);
};

#endif
