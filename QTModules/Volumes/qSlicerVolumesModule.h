#ifndef __qSlicerVolumesModule_h
#define __qSlicerVolumesModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerVolumesModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerVolumesModulePrivate;

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumesModule :
  public qSlicerAbstractLoadableModule
{
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerVolumesModule(QObject *parent=0);

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:
  ///
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  CTK_DECLARE_PRIVATE(qSlicerVolumesModule);
};

#endif
