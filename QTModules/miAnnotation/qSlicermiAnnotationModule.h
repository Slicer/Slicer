#ifndef __qSlicermiAnnotationModule_h
#define __qSlicermiAnnotationModule_h

// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

// CTK includes
#include <ctkPimpl.h>

#include "qSlicermiAnnotationModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicermiAnnotationModulePrivate;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qSlicermiAnnotationModule :
  public qSlicerAbstractLoadableModule
{
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicermiAnnotationModule(QObject *parent=0);
  virtual ~qSlicermiAnnotationModule(){}

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:

  // Description:
  // Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleWidget * createWidgetRepresentation();

  // Description:
  // Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  CTK_DECLARE_PRIVATE(qSlicermiAnnotationModule);
};

#endif
