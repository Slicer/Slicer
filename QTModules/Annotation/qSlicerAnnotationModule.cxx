#include "qSlicerAnnotationModule.h"

// SlicerQT includes
#include "qSlicerAnnotationModuleWidget.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// QT includes
#include <QtPlugin>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerAnnotationModule, qSlicerAnnotationModule);

//-----------------------------------------------------------------------------
class qSlicerAnnotationModulePrivate: public ctkPrivate<qSlicerAnnotationModule>
{
  public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAnnotationModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerAnnotationModule::createWidgetRepresentation()
{
  return new qSlicerAnnotationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerAnnotationModule::createLogic()
{
  return vtkSlicerAnnotationModuleLogic::New();
}

/**
  * Return the help text.
  */
QString qSlicerAbstractLoadableModule::helpText() const {
  return "The Annotation module.";
}
