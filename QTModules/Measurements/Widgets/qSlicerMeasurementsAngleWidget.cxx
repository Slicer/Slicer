// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerMeasurementsAngleWidget.h" 
#include "ui_qSlicerMeasurementsAngleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerMeasurementsAngleWidgetPrivate : public ctkPrivate<qSlicerMeasurementsAngleWidget>,
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
  CTK_INIT_PRIVATE(qSlicerMeasurementsAngleWidget);
  CTK_D(qSlicerMeasurementsAngleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsAngleWidget::printAdditionalInfo()
{
}
