#ifndef __qSlicerMeasurementsModuleWidget_h
#define __qSlicerMeasurementsModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerMeasurementsModuleExport.h"

class qSlicerMeasurementsModuleWidgetPrivate;

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMeasurementsModuleWidget(QWidget *parent=0);

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerMeasurementsModuleWidget);
};

#endif
