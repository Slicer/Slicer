#ifndef __qSlicerWelcomeModuleWidget_h
#define __qSlicerWelcomeModuleWidget_h

/// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

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
  QCTK_DECLARE_PRIVATE(qSlicerWelcomeModuleWidget);
};

#endif
