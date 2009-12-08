#include "qSlicerAbstractModuleWidget.h"

// SlicerLogic includes
// #include "vtkSlicerApplicationLogic.h"

//-----------------------------------------------------------------------------
struct qSlicerAbstractModuleWidgetPrivate: public qCTKPrivate<qSlicerAbstractModuleWidget>
{
  QString Name;
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerAbstractModuleWidget, const QString&, setName, Name);
QCTK_GET_CXX(qSlicerAbstractModuleWidget, QString, name, Name);

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::initialize(/*vtkSlicerApplicationLogic* appLogic*/)
{
  //QCTK_D(qSlicerAbstractModuleWidget);
  //Q_ASSERT(appLogic);
  //this->setAppLogic(appLogic);
  this->setup();
}
