#ifndef __qSlicerWelcomeModuleWidget_h
#define __qSlicerWelcomeModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerWelcomeModuleExport.h"

class qSlicerWelcomeModuleWidgetPrivate;

class Q_SLICER_QTMODULES_WELCOME_EXPORT qSlicerWelcomeModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerWelcomeModuleWidget(QWidget *parent=0);
  virtual ~qSlicerWelcomeModuleWidget(){}

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerWelcomeModuleWidget);
};

#endif
