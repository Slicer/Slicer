#include "qSlicerMeasurementsModuleWidget.h"
#include "ui_qSlicerMeasurementsModule.h"

//-----------------------------------------------------------------------------
class qSlicerMeasurementsModuleWidgetPrivate: public qCTKPrivate<qSlicerMeasurementsModuleWidget>,
                                               public Ui_qSlicerMeasurementsModule
{
public:
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerMeasurementsModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerMeasurementsModuleWidget::setup()
{
  QCTK_D(qSlicerMeasurementsModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
QAction* qSlicerMeasurementsModuleWidget::showModuleAction()
{
  return new QAction(QIcon(":/Icons/Measurements.png"), tr("Show Measurements module"), this);
}
