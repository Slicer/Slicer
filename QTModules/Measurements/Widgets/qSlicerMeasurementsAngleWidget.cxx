#include "qSlicerMeasurementsAngleWidget.h" 
#include "ui_qSlicerMeasurementsAngleWidget.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerMeasurementsAngleWidgetPrivate : public qCTKPrivate<qSlicerMeasurementsAngleWidget>,
                                               public Ui_qSlicerMeasurementsAngleWidget
{
  qSlicerMeasurementsAngleWidgetPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerMeasurementsAngleWidget::qSlicerMeasurementsAngleWidget(QWidget *parent) : Superclass(parent)
{
  QCTK_INIT_PRIVATE(qSlicerMeasurementsAngleWidget);
  QCTK_D(qSlicerMeasurementsAngleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsAngleWidget::printAdditionalInfo()
{
}
