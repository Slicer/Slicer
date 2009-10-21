
#include "qCTKSlider.h"

#include <QDebug>

#include <math.h>

#define qCTKSlider_IS_DBL(X) (!(X == static_cast<double>(static_cast<int>(X))))

//-----------------------------------------------------------------------------
class qCTKSlider::qInternal
{
public:
  qInternal()
    {
    this->SingleStep = 1; 
    this->HandleDouble = false; 
    this->RangeMinimum = 0;
    this->RangeMaximum = 100; 
    this->PreviousPosition = 0; 
    }
  double      SingleStep; 
  bool        HandleDouble; 
  double      RangeMinimum; 
  double      RangeMaximum; 
  double      PreviousPosition; 
};

// --------------------------------------------------------------------------
qCTKSlider::qCTKSlider(QWidget* parent) : Superclass(parent)
{
  this->Internal = new qInternal; 
  
  this->connect(this, SIGNAL(valueChanged(int)), SLOT(onValueChanged(int))); 
  this->connect(this, SIGNAL(sliderMoved(int)), SLOT(onSliderMoved(int))); 
}

// --------------------------------------------------------------------------
qCTKSlider::~qCTKSlider()
{
  delete this->Internal; 
}

// --------------------------------------------------------------------------
void qCTKSlider::setRange(double min, double max)
{
  if (this->Internal->HandleDouble)
    {
    min = min / this->Internal->SingleStep;
    max = max / this->Internal->SingleStep;
    }
  this->Superclass::setRange(static_cast<int>(min), static_cast<int>(max));
}

// --------------------------------------------------------------------------
double qCTKSlider::minimumAsDbl()
{
  return this->Superclass::minimum() * this->Internal->SingleStep; 
}

// --------------------------------------------------------------------------
double qCTKSlider::maximumAsDbl()
{
  return this->Superclass::maximum() * this->Internal->SingleStep;
}
  
// --------------------------------------------------------------------------
double qCTKSlider::sliderPositionAsDbl()
{
  return this->Superclass::sliderPosition() * this->Internal->SingleStep;
}

// --------------------------------------------------------------------------
void qCTKSlider::setSliderPosition(double position)
{
  this->Superclass::setSliderPosition(this->fromDoubleToInteger(position));
}

// -------------------------------------------------------------------------
double qCTKSlider::previousSliderPosition()
{
  return this->Internal->PreviousPosition; 
}

// --------------------------------------------------------------------------
double qCTKSlider::valueAsDbl()
{
  return this->Superclass::value() * this->Internal->SingleStep;
}

// --------------------------------------------------------------------------
void qCTKSlider::setValue(double value)
{
  this->Superclass::setValue(this->fromDoubleToInteger(value));
  emit this->valueChanged(value); 
  this->Internal->PreviousPosition = value; 
}

// --------------------------------------------------------------------------
double qCTKSlider::singleStepAsDbl()
{
  return this->Internal->SingleStep; 
}

// --------------------------------------------------------------------------
void qCTKSlider::setSingleStep(double step)
{
  this->Internal->HandleDouble = qCTKSlider_IS_DBL(step);
  this->Internal->SingleStep = step; 
}

// --------------------------------------------------------------------------
double qCTKSlider::tickIntervalAsDbl()
{
  return this->Superclass::tickInterval() * this->Internal->SingleStep; 
}

// --------------------------------------------------------------------------
void qCTKSlider::setTickInterval(double ti)
{
  this->Superclass::setTickInterval(this->fromDoubleToInteger(ti));
}

// --------------------------------------------------------------------------
int qCTKSlider::fromDoubleToInteger(double d)
{
  int res = static_cast<int>(d); 
  if (this->Internal->HandleDouble)
    {
    double tmp = d / this->Internal->SingleStep; 
    res = static_cast<int>( (tmp > 0) ? floor(tmp + 0.5) : ceil(tmp - 0.5) ); 
    
    }
  return res;
}

// --------------------------------------------------------------------------
void qCTKSlider::onValueChanged(int value)
{
  emit this->valueChanged(value * this->Internal->SingleStep);
}

// --------------------------------------------------------------------------
void qCTKSlider::onSliderMoved(int position)
{
  emit this->sliderMoved(position * this->Internal->SingleStep);
}

// --------------------------------------------------------------------------
void qCTKSlider::reset()
{
  this->setValue(0.0);
  this->setSliderPosition(0.0); 
  this->Internal->PreviousPosition = 0.0; 
}

#undef qCTKSlider_IS_DBL
