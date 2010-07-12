#ifndef __qSlicerColorsModuleWidget_h
#define __qSlicerColorsModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerColorsModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerColorsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerColorsModuleWidget(QWidget *parent=0);

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerColorsModuleWidget);
};

#endif
