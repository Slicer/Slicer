#include "qSlicerAbstractModuleWidget.h"

// SlicerLogic includes
// #include "vtkSlicerApplicationLogic.h"

//-----------------------------------------------------------------------------
struct qSlicerAbstractModuleWidgetPrivate: public qCTKPrivate<qSlicerAbstractModuleWidget>
{

};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModuleWidget, QWidget*);

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
