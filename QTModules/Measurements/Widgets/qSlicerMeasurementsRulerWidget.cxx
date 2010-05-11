// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerMeasurementsRulerWidget.h" 
#include "ui_qSlicerMeasurementsRulerWidget.h"

//-----------------------------------------------------------------------------
class qSlicerMeasurementsRulerWidgetPrivate : public ctkPrivate<qSlicerMeasurementsRulerWidget>,
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
  CTK_INIT_PRIVATE(qSlicerMeasurementsRulerWidget);
  CTK_D(qSlicerMeasurementsRulerWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsRulerWidget::printAdditionalInfo()
{
}
