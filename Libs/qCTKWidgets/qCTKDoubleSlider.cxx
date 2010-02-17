/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKDoubleSlider.h"

// QT includes
#include <QDebug>
#include <QHBoxLayout>

//-----------------------------------------------------------------------------
class qCTKDoubleSliderPrivate: public qCTKPrivate<qCTKDoubleSlider>
{
  public:
  qCTKDoubleSliderPrivate();
  int toInt(double _value)const;
  double fromInt(int _value)const;
  void init();
  void updateOffset(double value);

  double      Minimum;
  double      Maximum;
  double      Offset;
  QSlider*    Slider;
  double      SingleStep;
  double      Value;
};

// --------------------------------------------------------------------------
qCTKDoubleSliderPrivate::qCTKDoubleSliderPrivate()
{
  this->Slider = 0;
  this->SingleStep = 1.;
  this->Minimum = 0.;
  this->Maximum = 100.;
  this->Offset = 0.;
}

// --------------------------------------------------------------------------
void qCTKDoubleSliderPrivate::init()
{
  QCTK_P(qCTKDoubleSlider);
  this->Slider = new QSlider(p);
  QHBoxLayout* l = new QHBoxLayout(p);
  l->addWidget(this->Slider);
  l->setContentsMargins(0,0,0,0);
  
  this->Minimum = this->Slider->minimum();
  this->Maximum = this->Slider->maximum();
  this->SingleStep = this->Slider->singleStep();
  this->Value = this->Slider->value();

  p->connect(this->Slider, SIGNAL(valueChanged(int)), p, SLOT(onValueChanged(int)));
  p->connect(this->Slider, SIGNAL(sliderMoved(int)), p, SLOT(onSliderMoved(int)));
  p->connect(this->Slider, SIGNAL(sliderPressed()), p, SIGNAL(sliderPressed()));
  p->connect(this->Slider, SIGNAL(sliderReleased()), p, SIGNAL(sliderReleased()));
}
  
// --------------------------------------------------------------------------
int qCTKDoubleSliderPrivate::toInt(double doubleValue)const
{
  double tmp = doubleValue / this->SingleStep;
  int intValue = qRound(tmp);
  //qDebug() << __FUNCTION__ << doubleValue << intValue;
  return intValue;
}

// --------------------------------------------------------------------------
double qCTKDoubleSliderPrivate::fromInt(int intValue)const
{
  double doubleValue = this->SingleStep * (this->Offset + intValue) ;
  //qDebug() << __FUNCTION__ << intValue << doubleValue;
  return doubleValue;
}

// --------------------------------------------------------------------------
void qCTKDoubleSliderPrivate::updateOffset(double value)
{
  this->Offset = (value / this->SingleStep) - this->toInt(value);
}

// --------------------------------------------------------------------------
qCTKDoubleSlider::qCTKDoubleSlider(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKDoubleSlider);
  qctk_d()->init();
}

// --------------------------------------------------------------------------
qCTKDoubleSlider::qCTKDoubleSlider(Qt::Orientation _orientation, QWidget* _parent) 
  : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKDoubleSlider);
  qctk_d()->init();
  this->setOrientation(_orientation);
}

// --------------------------------------------------------------------------
qCTKDoubleSlider::~qCTKDoubleSlider()
{
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::setMinimum(double min)
{
  QCTK_D(qCTKDoubleSlider);
  d->Minimum = min;
  if (d->Minimum >= d->Value)
    {
    d->updateOffset(d->Minimum);
    }
  d->Slider->setMinimum(d->toInt(min));
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::setMaximum(double max)
{
  QCTK_D(qCTKDoubleSlider);
  d->Maximum = max;
  if (d->Maximum <= d->Value)
    {
    d->updateOffset(d->Maximum);
    }
  d->Slider->setMaximum(d->toInt(max));
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::setRange(double min, double max)
{
  QCTK_D(qCTKDoubleSlider);
  d->Minimum = min;
  d->Maximum = max;
  d->Slider->setRange(d->toInt(min), d->toInt(max));
}

// --------------------------------------------------------------------------
double qCTKDoubleSlider::minimum()const
{
  QCTK_D(const qCTKDoubleSlider);
  return d->Minimum;
}

// --------------------------------------------------------------------------
double qCTKDoubleSlider::maximum()const
{
  QCTK_D(const qCTKDoubleSlider);
  return d->Maximum;
}

// --------------------------------------------------------------------------
double qCTKDoubleSlider::sliderPosition()const
{
  QCTK_D(const qCTKDoubleSlider);
  return d->fromInt(d->Slider->sliderPosition());
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::setSliderPosition(double newSliderPosition)
{
  QCTK_D(qCTKDoubleSlider);
  d->Slider->setSliderPosition(d->toInt(newSliderPosition));
}

// --------------------------------------------------------------------------
double qCTKDoubleSlider::value()const
{
  QCTK_D(const qCTKDoubleSlider);
  return d->Value;
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::setValue(double newValue)
{
  QCTK_D(qCTKDoubleSlider);
  d->updateOffset(newValue);
  int newIntValue = d->toInt(newValue);
  if (newIntValue != d->Slider->value())
    {
    // d->Slider will emit a valueChanged signal that is connected to
    // qCTKDoubleSlider::onValueChanged
    d->Slider->setValue(newIntValue);
    }
  else
    {
    double oldValue = d->Value;
    d->Value = newValue;
    // don't emit a valuechanged signal if the new value is quite 
    // similar to the old value.
    if (qAbs(newValue - oldValue) > (d->SingleStep * 0.000000001))
      {
      emit this->valueChanged(newValue);
      }
    }
}

// --------------------------------------------------------------------------
double qCTKDoubleSlider::singleStep()const
{
  QCTK_D(const qCTKDoubleSlider);
  return d->SingleStep;
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::setSingleStep(double newStep)
{
  QCTK_D(qCTKDoubleSlider);
  d->SingleStep = newStep;
  d->updateOffset(d->Value);
  // update the new values of the QSlider
  double _value = d->Value;
  this->setValue(_value);
  this->setMinimum(d->Minimum);
  this->setMaximum(d->Maximum);
}

// --------------------------------------------------------------------------
double qCTKDoubleSlider::tickInterval()const
{
  QCTK_D(const qCTKDoubleSlider);
  return d->fromInt(d->Slider->tickInterval());
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::setTickInterval(double newTickInterval)
{
  QCTK_D(qCTKDoubleSlider);
  d->Slider->setTickInterval(d->toInt(newTickInterval));
}

// --------------------------------------------------------------------------
bool qCTKDoubleSlider::hasTracking()const
{
  QCTK_D(const qCTKDoubleSlider);
  return d->Slider->hasTracking();
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::setTracking(bool enable)
{
  QCTK_D(qCTKDoubleSlider);
  d->Slider->setTracking(enable);
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::triggerAction( QAbstractSlider::SliderAction action)
{
  QCTK_D(qCTKDoubleSlider);
  d->Slider->triggerAction(action);
}

// --------------------------------------------------------------------------
Qt::Orientation qCTKDoubleSlider::orientation()const
{
  QCTK_D(const qCTKDoubleSlider);
  return d->Slider->orientation();
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::setOrientation(Qt::Orientation newOrientation)
{
  QCTK_D(qCTKDoubleSlider);
  d->Slider->setOrientation(newOrientation);
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::onValueChanged(int newValue)
{
  QCTK_D(qCTKDoubleSlider);
  double doubleNewValue = d->fromInt(newValue);
/*
  qDebug() << "onValueChanged: " << newValue << "->"<< d->fromInt(newValue+d->Offset) 
           << " old: " << d->Value << "->" << d->toInt(d->Value) 
           << "offset:" << d->Offset << doubleNewValue;
*/
  if (d->Value == doubleNewValue)
    {
    return;
    }
  d->Value = doubleNewValue;
  emit this->valueChanged(d->Value);
}

// --------------------------------------------------------------------------
void qCTKDoubleSlider::onSliderMoved(int newPosition)
{
  QCTK_D(const qCTKDoubleSlider);
  emit this->sliderMoved(d->fromInt(newPosition));
}


