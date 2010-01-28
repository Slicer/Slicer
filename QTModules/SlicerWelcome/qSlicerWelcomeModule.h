#ifndef __qSlicerWelcomeModule_h
#define __qSlicerWelcomeModule_h

// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerWelcomeModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerWelcomeModulePrivate;

class Q_SLICER_QTMODULES_WELCOME_EXPORT qSlicerWelcomeModule :
  public qSlicerAbstractLoadableModule
{
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerWelcomeModule(QObject *parent=0);
  virtual ~qSlicerWelcomeModule(){}

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:

  // Description:
  // Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleWidget * createWidgetRepresentation();

  // Description:
  // Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  QCTK_DECLARE_PRIVATE(qSlicerWelcomeModule);
};

#endif
