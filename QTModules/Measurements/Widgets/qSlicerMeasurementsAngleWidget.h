#ifndef __qSlicerMeasurementsAngleWidget_h
#define __qSlicerMeasurementsAngleWidget_h 

#include "qSlicerWidget.h"

#include "qSlicerMeasurementsModuleWin32Header.h"

class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsAngleWidget : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerMeasurementsAngleWidget(QWidget *parent=0);
  virtual ~qSlicerMeasurementsAngleWidget(); 
  
  virtual void printAdditionalInfo();

private:
  class qInternal;
  qInternal* Internal;
};

#endif
