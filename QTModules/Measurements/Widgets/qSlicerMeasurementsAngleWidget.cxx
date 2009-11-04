#include "qSlicerMeasurementsAngleWidget.h" 

#include "ui_qSlicerMeasurementsAngleWidget.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerMeasurementsAngleWidget::qInternal : public Ui::qSlicerMeasurementsAngleWidget
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerMeasurementsAngleWidget::qSlicerMeasurementsAngleWidget(QWidget *parent) : Superclass(parent)
{
  this->Internal = new qInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerMeasurementsAngleWidget::~qSlicerMeasurementsAngleWidget()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsAngleWidget::printAdditionalInfo()
{
}
