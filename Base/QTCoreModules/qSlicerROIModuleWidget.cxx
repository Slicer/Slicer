// SlicerQt includes
#include "qSlicerROIModuleWidget.h"
#include "ui_qSlicerROIModule.h"

//-----------------------------------------------------------------------------
class qSlicerROIModuleWidgetPrivate: public ctkPrivate<qSlicerROIModuleWidget>,
                                         public Ui_qSlicerROIModule
{
public:
  CTK_DECLARE_PUBLIC(qSlicerROIModuleWidget);
};

//-----------------------------------------------------------------------------
qSlicerROIModuleWidget::qSlicerROIModuleWidget(QWidget* parentWidget)
  :qSlicerAbstractModuleWidget(parentWidget)
{
  CTK_INIT_PRIVATE(qSlicerROIModuleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerROIModuleWidget::setup()
{
  CTK_D(qSlicerROIModuleWidget);
  d->setupUi(this);
}
