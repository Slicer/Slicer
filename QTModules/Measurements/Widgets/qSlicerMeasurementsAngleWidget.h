#ifndef __qSlicerMeasurementsAngleWidget_h
#define __qSlicerMeasurementsAngleWidget_h 

/// SlicerQT includes
#include "qSlicerWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerMeasurementsModuleExport.h"

class qSlicerMeasurementsAngleWidgetPrivate;

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsAngleWidget : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerMeasurementsAngleWidget(QWidget *parent=0);
  
  virtual void printAdditionalInfo();

private:
  QCTK_DECLARE_PRIVATE(qSlicerMeasurementsAngleWidget);
};

#endif
