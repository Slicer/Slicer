// SlicerQt includes
#include "qSlicerMeasurementsModuleWidget.h"
#include "ui_qSlicerMeasurementsModule.h"

//-----------------------------------------------------------------------------
class qSlicerMeasurementsModuleWidgetPrivate: public ctkPrivate<qSlicerMeasurementsModuleWidget>,
                                              public Ui_qSlicerMeasurementsModule
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerMeasurementsModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerMeasurementsModuleWidget::setup()
{
  CTK_D(qSlicerMeasurementsModuleWidget);
  d->setupUi(this);
}
