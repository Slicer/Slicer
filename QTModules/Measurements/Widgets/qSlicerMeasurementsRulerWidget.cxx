#include "qSlicerMeasurementsRulerWidget.h" 
#include "ui_qSlicerMeasurementsRulerWidget.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerMeasurementsRulerWidgetPrivate : public qCTKPrivate<qSlicerMeasurementsRulerWidget>,
                                               public Ui_qSlicerMeasurementsRulerWidget
{
public:
  qSlicerMeasurementsRulerWidgetPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerMeasurementsRulerWidget::qSlicerMeasurementsRulerWidget(QWidget *_parent):
Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qSlicerMeasurementsRulerWidget);
  QCTK_D(qSlicerMeasurementsRulerWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsRulerWidget::printAdditionalInfo()
{
}
