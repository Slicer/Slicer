// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerModuleTemplateModuleWidget.h"
#include "ui_qSlicerModuleTemplateModule.h"

//-----------------------------------------------------------------------------
class qSlicerModuleTemplateModuleWidgetPrivate: public ctkPrivate<qSlicerModuleTemplateModuleWidget>,
                                         public Ui_qSlicerModuleTemplateModule
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerModuleTemplateModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerModuleTemplateModuleWidget::setup()
{
  CTK_D(qSlicerModuleTemplateModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

