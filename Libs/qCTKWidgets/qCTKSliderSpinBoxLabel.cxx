
#include "qCTKSliderSpinBoxLabel.h"
#include "ui_qCTKSliderSpinBoxLabel.h"

#include <QDebug>

//-----------------------------------------------------------------------------
namespace
{
int getDecimalCount(double d)
  {
  // Convert double to QString
  QString dbl = QString::number(d);
  
  // Extract number of decimals
  int decimalPointIndex = dbl.indexOf("."); 
  return (decimalPointIndex == -1) ? 0 : (dbl.size() - 1 - decimalPointIndex); 
  } 
}

//-----------------------------------------------------------------------------
class qCTKSliderSpinBoxLabel::qInternal: public Ui::qCTKSliderSpinBoxLabel
{
public:
  qInternal()
    {
    }
};

// --------------------------------------------------------------------------
qCTKSliderSpinBoxLabel::qCTKSliderSpinBoxLabel(QWidget* parent) : Superclass(parent)
{
  this->Internal = new qInternal; 
  this->Internal->setupUi(this); 
  
  this->Internal->Slider->setTracking(false);
  this->Internal->SpinBox->setKeyboardTracking(false); 
  
  this->connect(this->Internal->Slider, SIGNAL(sliderMoved(double)), SIGNAL(sliderMoved(double))); 
  this->connect(this->Internal->SpinBox, SIGNAL(valueChanged(double)), 
                SLOT(onSpinBoxValueChanged(double)));
}

// --------------------------------------------------------------------------
qCTKSliderSpinBoxLabel::~qCTKSliderSpinBoxLabel()
{
  delete this->Internal; 
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setLabelTextVisible(bool visible)
{
  this->Internal->Label->setVisible(visible);
}

// --------------------------------------------------------------------------
bool qCTKSliderSpinBoxLabel::isLabelTextVisible()
{
  return this->Internal->Label->isVisible();
}

// --------------------------------------------------------------------------
QString qCTKSliderSpinBoxLabel::labelText()
{
  return this->Internal->Label->text(); 
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setLabelText(const QString & label)
{
  this->Internal->Label->setText(label); 
}
  
// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setRange(double min, double max)
{
  this->Internal->Slider->setRange(min, max);
  this->Internal->SpinBox->setRange(min, max);
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::minimum()
{
  return this->Internal->Slider->minimumAsDbl();
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::maximum()
{
  return this->Internal->Slider->maximumAsDbl(); 
}
  
// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::sliderPosition()
{
  return this->Internal->Slider->sliderPositionAsDbl();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setSliderPosition(double position)
{
  this->Internal->Slider->setSliderPosition(position);
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::previousSliderPosition()
{
  return this->Internal->Slider->previousSliderPosition(); 
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::value()
{
  return this->Internal->Slider->valueAsDbl();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setValue(double value)
{
  this->Internal->Slider->setValue(value);
  
  this->Internal->SpinBox->blockSignals(true);
  this->Internal->SpinBox->setValue(value); 
  this->Internal->SpinBox->blockSignals(false);
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::singleStep()
{
  return this->Internal->Slider->singleStepAsDbl(); 
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setSingleStep(double step)
{
  this->Internal->Slider->setSingleStep(step); 
  this->Internal->SpinBox->setSingleStep(step);
  this->Internal->SpinBox->setDecimals(getDecimalCount(step));
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::tickInterval()
{
  return this->Internal->Slider->tickIntervalAsDbl(); 
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setTickInterval(double ti)
{
  this->Internal->Slider->setTickInterval(ti); 
}

// -------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::onSpinBoxValueChanged(double value)
{
  this->Internal->Slider->setSliderPosition(value); 
  emit this->sliderMoved(value); 
}

// -------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::reset()
{
  this->Internal->Slider->reset();
  this->Internal->SpinBox->blockSignals(true);
  this->Internal->SpinBox->setValue(0);
  this->Internal->SpinBox->blockSignals(false);
}
