#include "qSlicerMeasurementsRulerWidget.h" 
#include "ui_qSlicerMeasurementsRulerWidget.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerMeasurementsRulerWidgetPrivate : public qCTKPrivate<qSlicerMeasurementsRulerWidget>,
                                               public Ui_qSlicerMeasurementsRulerWidget
{
  qSlicerMeasurementsRulerWidgetPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerMeasurementsRulerWidget::qSlicerMeasurementsRulerWidget(QWidget *parent) : Superclass(parent)
{
  QCTK_INIT_PRIVATE(qSlicerMeasurementsRulerWidget);
  QCTK_D(qSlicerMeasurementsRulerWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsRulerWidget::printAdditionalInfo()
{
}
