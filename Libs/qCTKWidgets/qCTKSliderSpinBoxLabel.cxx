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

  d->Slider->setTracking(false);
  d->SpinBox->setKeyboardTracking(false);

  this->connect(d->Slider, SIGNAL(sliderMoved(double)), SIGNAL(sliderMoved(double)));
  this->connect(d->SpinBox, SIGNAL(valueChanged(double)),
                SLOT(onSpinBoxValueChanged(double)));
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::minimum()const
{
  return qctk_d()->Slider->minimumAsDbl();
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::maximum()const
{
  return qctk_d()->Slider->maximumAsDbl();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setMinimum(double min)
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  
  d->Slider->setMinimum(min);
  d->SpinBox->setMinimum(min);
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setMaximum(double max)
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  
  d->Slider->setMaximum(max);
  d->SpinBox->setMaximum(max);
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setRange(double min, double max)
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  
  d->Slider->setRange(min, max);
  d->SpinBox->setRange(min, max);
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::sliderPosition()
{
  return qctk_d()->Slider->sliderPositionAsDbl();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setSliderPosition(double position)
{
  qctk_d()->Slider->setSliderPosition(position);
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::previousSliderPosition()
{
  return qctk_d()->Slider->previousSliderPosition();
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::value()
{
  return qctk_d()->Slider->valueAsDbl();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setValue(double _value)
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  
  d->Slider->setValue(_value);

  d->SpinBox->blockSignals(true);
  d->SpinBox->setValue(_value);
  d->SpinBox->blockSignals(false);
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::singleStep()
{
  return qctk_d()->Slider->singleStepAsDbl();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setSingleStep(double step)
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  
  d->Slider->setSingleStep(step);
  d->SpinBox->setSingleStep(step);
  d->SpinBox->setDecimals(::getDecimalCount(step));
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxLabel::tickInterval()
{
  return qctk_d()->Slider->tickIntervalAsDbl();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::setTickInterval(double ti)
{
  qctk_d()->Slider->setTickInterval(ti);
}

// -------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::onSpinBoxValueChanged(double _value)
{
  qctk_d()->Slider->setSliderPosition(_value);
  emit this->sliderMoved(_value);
  emit this->valueChanged(_value);
}

// -------------------------------------------------------------------------
void qCTKSliderSpinBoxLabel::reset()
{
  QCTK_D(qCTKSliderSpinBoxLabel);
  
  d->Slider->reset();
  d->SpinBox->blockSignals(true);
  d->SpinBox->setValue(0);
  d->SpinBox->blockSignals(false);
}
