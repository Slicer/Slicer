#ifndef __qSlicerMeasurementsModuleWidget_h
#define __qSlicerMeasurementsModuleWidget_h

/// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerMeasurementsModuleExport.h"

class qSlicerMeasurementsModuleWidgetPrivate;

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMeasurementsModuleWidget(QWidget *parent=0);

  virtual QAction* showModuleAction();

protected:
  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerMeasurementsModuleWidget);
};

#endif
