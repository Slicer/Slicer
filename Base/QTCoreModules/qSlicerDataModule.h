#ifndef __qSlicerDataModule_h
#define __qSlicerDataModule_h

/// CTK includes
#include <ctkPimpl.h>

/// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerDataModulePrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerDataModule :
  public qSlicerAbstractCoreModule
{
  Q_OBJECT
public:
  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerDataModule(QObject *parent=0);

  qSlicerGetTitleMacro("Data");

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
  CTK_DECLARE_PRIVATE(qSlicerDataModule);
};

#endif
