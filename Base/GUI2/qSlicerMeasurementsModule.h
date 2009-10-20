#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h 

#include "qSlicerAbstractModule.h"

#include "qSlicerBaseGUI2Win32Header.h"

class Q_SLICER_BASE_GUI2_EXPORT qSlicerMeasurementsModule : public qSlicerAbstractModule
{ 
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerMeasurementsModule(QWidget *parent=0);
  virtual ~qSlicerMeasurementsModule(); 

private:
  class qInternal;
  qInternal* Internal;
};

#endif
