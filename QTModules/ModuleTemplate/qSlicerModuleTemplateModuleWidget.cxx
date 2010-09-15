// Qt includes
#include <QDebug>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerModuleTemplateModuleWidget.h"
#include "ui_qSlicerModuleTemplateModule.h"

//-----------------------------------------------------------------------------
class qSlicerModuleTemplateModuleWidgetPrivate: public Ui_qSlicerModuleTemplateModule
{
public:
  qSlicerModuleTemplateModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerModuleTemplateModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModuleWidgetPrivate::qSlicerModuleTemplateModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerModuleTemplateModuleWidget methods

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerModuleTemplateModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModuleWidget::~qSlicerModuleTemplateModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerModuleTemplateModuleWidget::setup()
{
  Q_D(qSlicerModuleTemplateModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

