#ifndef __qSlicerSliceControllerModule_h
#define __qSlicerSliceControllerModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerSliceControllerModulePrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerSliceControllerModule :
  public qSlicerAbstractCoreModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerSliceControllerModule(QObject *parent=0);

  qSlicerGetTitleMacro("Slices");

protected:
  ///
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  CTK_DECLARE_PRIVATE(qSlicerSliceControllerModule);
};

#endif
