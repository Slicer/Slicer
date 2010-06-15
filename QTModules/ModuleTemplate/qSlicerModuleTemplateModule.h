#ifndef __qSlicerModuleTemplateModule_h
#define __qSlicerModuleTemplateModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerModuleTemplateModuleExport.h"

class qSlicerModuleTemplateModulePrivate;

class Q_SLICER_QTMODULES_MODULETEMPLATE_EXPORT qSlicerModuleTemplateModule :
  public qSlicerAbstractLoadableModule
{
  Q_INTERFACES(qSlicerAbstractLoadableModule);

public:

  typedef qSlicerAbstractLoadableModule Superclass;
  qSlicerModuleTemplateModule(QObject *parent=0);

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:
  ///
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  CTK_DECLARE_PRIVATE(qSlicerModuleTemplateModule);
};

#endif
