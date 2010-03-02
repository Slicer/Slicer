#ifndef __qSlicerVolumesModuleWidget_h
#define __qSlicerVolumesModuleWidget_h


/// qCTK includes
#include <qCTKPimpl.h>

/// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVolumesModuleExport.h"

class qSlicerVolumesModuleWidgetPrivate;

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumesModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumesModuleWidget(QWidget *parent=0);

  virtual QAction* showModuleAction();

protected:
  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerVolumesModuleWidget);
};

#endif
