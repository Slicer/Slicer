#ifndef __qSlicerEMSegmentModule_h
#define __qSlicerEMSegmentModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentModulePrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentModule :
  public qSlicerAbstractLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerEMSegmentModule(QObject *parent=0);

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  ///
  /// Help to use the module
  virtual QString helpText()const;

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
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentModule);
};

#endif
