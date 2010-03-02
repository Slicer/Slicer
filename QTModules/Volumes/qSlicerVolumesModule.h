#ifndef __qSlicerVolumesModule_h
#define __qSlicerVolumesModule_h

// qCTK includes
#include <qCTKPimpl.h>

// SlicerQT includes
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
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleWidget * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  QCTK_DECLARE_PRIVATE(qSlicerVolumesModule);
};

#endif
