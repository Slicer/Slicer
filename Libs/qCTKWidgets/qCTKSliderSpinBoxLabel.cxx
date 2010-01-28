/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/


#include "qCTKSliderSpinBoxLabel.h"
#include "ui_qCTKSliderSpinBoxLabel.h"

#include <QDebug>

bool equal(double v1, double v2)
{
  return qAbs(v1 - v2) < 0.0001;
}

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
class qCTKSliderSpinBoxLabelPrivate: public qCTKPrivate<qCTKSliderSpinBoxLabel>,
                                      public Ui_qCTKSliderSpinBoxLabel
{
};

// --------------------------------------------------------------------------
qCTKSliderSpinBoxLabel::qCTKSliderSpinBoxLabel(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKSliderSpinBoxLabel);
  QCTK_D(qCTKSliderSpinBoxLabel);
  
  d->setupUi(this);

  //d->Slider->setTracking(false);
  //d->SpinBox->setKeyboardTracking(false);
  d->Slider->setMaximum(d->SpinBox->maximum());
  d->Slider->setMinimum(d->SpinBox->minimum());

  this->connect(d->Slider, SIGNAL(sliderMoved(double)), SIGNAL(sliderMoved(double)));
  this->connect(d->Slider, SIGNAL(valueChanged(double)), d->SpinBox, SLOT(setValue(double)));
  this->connect(d->SpinBox, SIGNAL(valueChanged(double)), d->Slider, SLOT(setValue(double)));
  this->connect(d->Slider, SIGNAL(valueChanged(double)), SIGNAL(valueChanged(double)));
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::minimum()const
{
  QCTK_D(const qCTKSliderSpinBoxLabel);
  Q_ASSERT(equal(d->SpinBox->minimum(),d->Slider->minimum()));
  return d->Slider->minimum();
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::maximum()const
{
  QCTK_D(const qCTKSliderSpinBoxLabel);
  Q_ASSERT(equal(d->SpinBox->maximum(),d->Slider->maximum()));
  return d->Slider->maximum();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setMinimum(double min)
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  d->Slider->setMinimum(min);
  d->SpinBox->setMinimum(min);
  Q_ASSERT(equal(d->SpinBox->minimum(),d->Slider->minimum()));
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setMaximum(double max)
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  d->Slider->setMaximum(max);
  d->SpinBox->setMaximum(max);
  Q_ASSERT(equal(d->SpinBox->maximum(), d->Slider->maximum()));
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setRange(double min, double max)
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  
  d->Slider->setRange(min, max);
  d->SpinBox->setRange(min, max);
  Q_ASSERT(equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(equal(d->SpinBox->maximum(), d->Slider->maximum()));
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::sliderPosition()const
{
  return qctk_d()->Slider->sliderPosition();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setSliderPosition(double position)
{
  qctk_d()->Slider->setSliderPosition(position);
}

/*
// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::previousSliderPosition()
{
  return qctk_d()->Slider->previousSliderPosition();
}
*/

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::value()const
{
  QCTK_D(const qCTKSliderSpinBoxLabel);
  Q_ASSERT(equal(d->Slider->value(), d->SpinBox->value()));
  return d->Slider->value();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setValue(double _value)
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  //qDebug() << __FUNCTION__ << "set: " << _value;
  //qDebug() << __FUNCTION__ << "old values: " << d->Slider->value() << " " << d->SpinBox->value();
  d->SpinBox->setValue(_value);
  //qDebug() << __FUNCTION__ << "inter: " << d->Slider->value() << d->SpinBox->value();
//  d->SpinBox->blockSignals(true);
  d->Slider->setValue(d->SpinBox->value());
//  d->SpinBox->blockSignals(false);
  //qDebug() << __FUNCTION__ << "new values: " << d->Slider->value() << " " << d->SpinBox->value();
  Q_ASSERT(equal(d->Slider->value(), d->SpinBox->value()));
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::singleStep()const
{
  QCTK_D(const qCTKSliderSpinBoxLabel);
  Q_ASSERT(equal(d->Slider->singleStep(), d->SpinBox->singleStep()));
  return qctk_d()->Slider->singleStep();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setSingleStep(double step)
{
  QCTK_D(qCTKSliderSpinBoxLabel);;
  d->Slider->setSingleStep(step);
  d->SpinBox->setSingleStep(step);
  d->SpinBox->setDecimals(::getDecimalCount(step));
  Q_ASSERT(equal(d->Slider->singleStep(), d->SpinBox->singleStep()));
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::tickInterval()const
{
  QCTK_D(const qCTKSliderSpinBoxLabel);
  return d->Slider->tickInterval();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setTickInterval(double ti)
{ 
  QCTK_D(qCTKSliderSpinBoxLabel);
  d->Slider->setTickInterval(ti);
}

// -------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::reset()
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  
  d->Slider->setValue(0);
/*
  d->SpinBox->blockSignals(true);
  d->SpinBox->setValue(0);
  d->SpinBox->blockSignals(false);
*/
}
