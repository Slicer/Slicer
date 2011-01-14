// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerExtensionTestModuleWidget.h"
#include "ui_qSlicerExtensionTestModule.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionTestModuleWidgetPrivate: public Ui_qSlicerExtensionTestModule
{
public:
  qSlicerExtensionTestModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerExtensionTestModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerExtensionTestModuleWidgetPrivate::qSlicerExtensionTestModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerExtensionTestModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerExtensionTestModuleWidget::qSlicerExtensionTestModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerExtensionTestModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerExtensionTestModuleWidget::~qSlicerExtensionTestModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerExtensionTestModuleWidget::setup()
{
  Q_D(qSlicerExtensionTestModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

