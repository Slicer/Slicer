#ifndef __qSlicerVolumesModule_h
#define __qSlicerVolumesModule_h

// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerVolumesModuleWin32Header.h"

class qSlicerVolumesModulePrivate;

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumesModule :
  public qSlicerAbstractLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerVolumesModule(QWidget *parent=0);

  virtual void printAdditionalInfo();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  virtual QAction* showModuleAction();

protected:
  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerVolumesModule);
};

#endif
