// Qt includes
#include <QDebug>

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
qSlicerModuleTemplateModuleWidget::qSlicerModuleTemplateModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerModuleTemplateModuleWidgetPrivate )
{
}

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

