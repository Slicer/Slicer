#ifndef __qSlicerMeasurementsRulerWidget_h
#define __qSlicerMeasurementsRulerWidget_h 

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerWidget.h"

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
  CTK_DECLARE_PRIVATE(qSlicerMeasurementsRulerWidget);
};

#endif
