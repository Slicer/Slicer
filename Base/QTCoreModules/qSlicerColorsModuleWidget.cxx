// SlicerQt includes
#include "qSlicerColorsModuleWidget.h"
#include "ui_qSlicerColorsModule.h"

//-----------------------------------------------------------------------------
class qSlicerColorsModuleWidgetPrivate: public ctkPrivate<qSlicerColorsModuleWidget>,
                                         public Ui_qSlicerColorsModule
{
public:
  CTK_DECLARE_PUBLIC(qSlicerColorsModuleWidget);
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerColorsModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setup()
{
  CTK_D(qSlicerColorsModuleWidget);
  d->setupUi(this);
}
