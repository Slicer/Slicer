#include "qSlicerMeasurementsRulerWidget.h" 

#include "ui_qSlicerMeasurementsRulerWidget.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerMeasurementsRulerWidget::qInternal : public Ui::qSlicerMeasurementsRulerWidget
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerMeasurementsRulerWidget::qSlicerMeasurementsRulerWidget(QWidget *parent) : Superclass(parent)
{
  this->Internal = new qInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerMeasurementsRulerWidget::~qSlicerMeasurementsRulerWidget()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsRulerWidget::printAdditionalInfo()
{
}
