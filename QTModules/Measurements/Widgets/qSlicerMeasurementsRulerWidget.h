#ifndef __qSlicerMeasurementsRulerWidget_h
#define __qSlicerMeasurementsRulerWidget_h 

/// SlicerQT includes
#include "qSlicerWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerMeasurementsModuleExport.h"

class qSlicerMeasurementsRulerWidgetPrivate;

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsRulerWidget : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerMeasurementsRulerWidget(QWidget *parent=0);
  
  virtual void printAdditionalInfo();

private:
  QCTK_DECLARE_PRIVATE(qSlicerMeasurementsRulerWidget);
};

#endif
