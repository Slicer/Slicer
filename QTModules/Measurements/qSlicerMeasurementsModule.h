#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerMeasurementsModuleExport.h"

class qSlicerMeasurementsModulePrivate;

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerMeasurementsModule(QObject *parent=0);
  virtual ~qSlicerMeasurementsModule(){}

  virtual QIcon icon()const;
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
