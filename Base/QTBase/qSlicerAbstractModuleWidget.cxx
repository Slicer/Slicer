#include "qSlicerAbstractModuleWidget.h"

// SlicerLogic includes
// #include "vtkSlicerApplicationLogic.h"

//-----------------------------------------------------------------------------
struct qSlicerAbstractModuleWidgetPrivate: public qCTKPrivate<qSlicerAbstractModuleWidget>
{
  qSlicerAbstractModule* Module; 
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerAbstractModuleWidget, qSlicerAbstractModule*, setModule, Module);

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerAbstractModuleWidget::module()
{
  QCTK_D(qSlicerAbstractModuleWidget);
  Q_ASSERT(d->Module);
  return d->Module;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::initialize(/*vtkSlicerApplicationLogic* appLogic*/)
{
  //Q_ASSERT(appLogic);
  //this->setAppLogic(appLogic);
  this->setup();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}
