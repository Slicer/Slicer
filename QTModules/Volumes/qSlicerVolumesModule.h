#ifndef __qSlicerVolumesModule_h
#define __qSlicerVolumesModule_h

// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerVolumesModuleWin32Header.h"

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

  // Description:
  // Create and return a widget representation of the object
  virtual qSlicerAbstractModuleWidget * createWidgetRepresentation();

private:
  QCTK_DECLARE_PRIVATE(qSlicerVolumesModule);
};

#endif
