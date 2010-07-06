#ifndef __qSlicerAnnotationModule_h
#define __qSlicerAnnotationModule_h

// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerAnnotationModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerAnnotationModulePrivate;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qSlicerAnnotationModule :
  public qSlicerAbstractLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule)
public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerAnnotationModule(QObject *parent=0);

  // Description:
  // Return the help text for the Annotation module.
  QString helpText();


  virtual ~qSlicerAnnotationModule(){}

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:

  // Description:
  // Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();

  // Description:
  // Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  CTK_DECLARE_PRIVATE(qSlicerAnnotationModule);
};

#endif
