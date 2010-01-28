/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/


#include "qCTKSliderSpinBoxWidget.h"
#include "ui_qCTKSliderSpinBoxWidget.h"

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
class qCTKSliderSpinBoxWidgetPrivate: public qCTKPrivate<qCTKSliderSpinBoxWidget>,
                                      public Ui_qCTKSliderSpinBoxWidget
{
};

// --------------------------------------------------------------------------
qCTKSliderSpinBoxWidget::qCTKSliderSpinBoxWidget(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKSliderSpinBoxWidget);
  QCTK_D(qCTKSliderSpinBoxWidget);
  
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
double qCTKSliderSpinBoxWidget::minimum()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  Q_ASSERT(equal(d->SpinBox->minimum(),d->Slider->minimum()));
  return d->Slider->minimum();
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxWidget::maximum()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  Q_ASSERT(equal(d->SpinBox->maximum(),d->Slider->maximum()));
  return d->Slider->maximum();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setMinimum(double min)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->Slider->setMinimum(min);
  d->SpinBox->setMinimum(min);
  Q_ASSERT(equal(d->SpinBox->minimum(),d->Slider->minimum()));
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setMaximum(double max)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->Slider->setMaximum(max);
  d->SpinBox->setMaximum(max);
  Q_ASSERT(equal(d->SpinBox->maximum(), d->Slider->maximum()));
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setRange(double min, double max)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  
  d->Slider->setRange(min, max);
  d->SpinBox->setRange(min, max);
  Q_ASSERT(equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(equal(d->SpinBox->maximum(), d->Slider->maximum()));
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxWidget::sliderPosition()const
{
  return qctk_d()->Slider->sliderPosition();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setSliderPosition(double position)
{
  qctk_d()->Slider->setSliderPosition(position);
}

/*
// --------------------------------------------------------------------------
double qCTKSliderSpinBoxWidget::previousSliderPosition()
{
  return qctk_d()->Slider->previousSliderPosition();
}
*/

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxWidget::value()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  Q_ASSERT(equal(d->Slider->value(), d->SpinBox->value()));
  return d->Slider->value();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setValue(double _value)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
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
double qCTKSliderSpinBoxWidget::singleStep()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  Q_ASSERT(equal(d->Slider->singleStep(), d->SpinBox->singleStep()));
  return qctk_d()->Slider->singleStep();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setSingleStep(double step)
{
  QCTK_D(qCTKSliderSpinBoxWidget);;
  d->Slider->setSingleStep(step);
  d->SpinBox->setSingleStep(step);
  d->SpinBox->setDecimals(::getDecimalCount(step));
  Q_ASSERT(equal(d->Slider->singleStep(), d->SpinBox->singleStep()));
}

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxWidget::tickInterval()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  return d->Slider->tickInterval();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setTickInterval(double ti)
{ 
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->Slider->setTickInterval(ti);
}

// -------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::reset()
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  
  d->Slider->setValue(0);
/*
  d->SpinBox->blockSignals(true);
  d->SpinBox->setValue(0);
  d->SpinBox->blockSignals(false);
*/
}
