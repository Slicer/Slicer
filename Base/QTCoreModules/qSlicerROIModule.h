#ifndef __qSlicerROIModule_h
#define __qSlicerROIModule_h

/// CTK includes
#include <ctkPimpl.h>

/// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerROIModulePrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerROIModule :
  public qSlicerAbstractCoreModule
{
  Q_OBJECT
public:
  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerROIModule(QObject *parent=0);

  qSlicerGetTitleMacro("ROI");

  /// Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;

protected:

  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  CTK_DECLARE_PRIVATE(qSlicerROIModule);
};

#endif
