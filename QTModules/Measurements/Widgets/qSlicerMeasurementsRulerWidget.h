#ifndef __qSlicerMeasurementsRulerWidget_h
#define __qSlicerMeasurementsRulerWidget_h 

#include "qSlicerWidget.h"

#include "qSlicerMeasurementsModuleWin32Header.h"

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsRulerWidget : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerMeasurementsRulerWidget(QWidget *parent=0);
  virtual ~qSlicerMeasurementsRulerWidget(); 
  
  virtual void printAdditionalInfo();

private:
  class qInternal;
  qInternal* Internal;
};

#endif
