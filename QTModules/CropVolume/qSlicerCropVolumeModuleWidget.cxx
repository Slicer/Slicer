// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerCropVolumeModuleWidget.h"
#include "ui_qSlicerCropVolumeModule.h"

//-----------------------------------------------------------------------------
class qSlicerCropVolumeModuleWidgetPrivate: public Ui_qSlicerCropVolumeModule
{
public:
  qSlicerCropVolumeModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCropVolumeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidgetPrivate::qSlicerCropVolumeModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCropVolumeModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidget::qSlicerCropVolumeModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerCropVolumeModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidget::~qSlicerCropVolumeModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setup()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

