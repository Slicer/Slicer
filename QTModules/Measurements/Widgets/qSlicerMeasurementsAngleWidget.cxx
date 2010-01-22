#include "qSlicerMeasurementsAngleWidget.h" 
#include "ui_qSlicerMeasurementsAngleWidget.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerMeasurementsAngleWidgetPrivate : public qCTKPrivate<qSlicerMeasurementsAngleWidget>,
                                               public Ui_qSlicerMeasurementsAngleWidget
{
public:
  qSlicerMeasurementsAngleWidgetPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerMeasurementsAngleWidget::qSlicerMeasurementsAngleWidget(QWidget *_parent):
Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qSlicerMeasurementsAngleWidget);
  QCTK_D(qSlicerMeasurementsAngleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsAngleWidget::printAdditionalInfo()
{
}
