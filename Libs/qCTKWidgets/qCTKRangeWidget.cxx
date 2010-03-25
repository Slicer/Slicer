/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/


#include "qCTKRangeWidget.h"
#include "ui_qCTKRangeWidget.h"

#include <QDebug>
#include <QMouseEvent>

//-----------------------------------------------------------------------------
class qCTKRangeWidgetPrivate: public qCTKPrivate<qCTKRangeWidget>,
                                      public Ui_qCTKRangeWidget
{
public:
  qCTKRangeWidgetPrivate();
  void updateSpinBoxWidth();
  int synchronizedSpinBoxWidth()const;
  void synchronizeSiblingSpinBox(int newWidth);
  static bool equal(double v1, double v2);


  bool   Tracking;
  bool   Changing;
  double MinimumValueBeforeChange;
  double MaximumValueBeforeChange;
  bool   AutoSpinBoxWidth;
};

// --------------------------------------------------------------------------
bool qCTKRangeWidgetPrivate::equal(double v1, double v2)
{
  return qAbs(v1 - v2) < 0.0001;
}

// --------------------------------------------------------------------------
qCTKRangeWidgetPrivate::qCTKRangeWidgetPrivate()
{
  this->Tracking = true;
  this->Changing = false;
  this->MinimumValueBeforeChange = 0.;
  this->MaximumValueBeforeChange = 0.;
  this->AutoSpinBoxWidth = true;
}

// --------------------------------------------------------------------------
void qCTKRangeWidgetPrivate::updateSpinBoxWidth()
{
  int spinBoxWidth = this->synchronizedSpinBoxWidth();
  if (this->AutoSpinBoxWidth)
    {
    this->MinimumSpinBox->setMinimumWidth(spinBoxWidth);
    this->MaximumSpinBox->setMinimumWidth(spinBoxWidth);
    }
  else
    {
    this->MinimumSpinBox->setMinimumWidth(0);
    this->MaximumSpinBox->setMinimumWidth(0);
    }
  this->synchronizeSiblingSpinBox(spinBoxWidth);
}

// --------------------------------------------------------------------------
int qCTKRangeWidgetPrivate::synchronizedSpinBoxWidth()const
{
  QCTK_P(const qCTKRangeWidget);
  //Q_ASSERT(this->MinimumSpinBox->sizeHint() == this->MaximumSpinBox->sizeHint());
  int maxWidth = qMax(this->MinimumSpinBox->sizeHint().width(),
                      this->MaximumSpinBox->sizeHint().width());
  if (!p->parent())
    {
    return maxWidth;
    }
  QList<qCTKRangeWidget*> siblings = 
    p->parent()->findChildren<qCTKRangeWidget*>();
  foreach(qCTKRangeWidget* sibling, siblings)
    {
    maxWidth = qMax(maxWidth, qMax(sibling->qctk_d()->MaximumSpinBox->sizeHint().width(),
                                   sibling->qctk_d()->MaximumSpinBox->sizeHint().width()));
    }
  return maxWidth;
}

// --------------------------------------------------------------------------
void qCTKRangeWidgetPrivate::synchronizeSiblingSpinBox(int width)
{
  QCTK_P(const qCTKRangeWidget);
  QList<qCTKRangeWidget*> siblings = 
    p->parent()->findChildren<qCTKRangeWidget*>();
  foreach(qCTKRangeWidget* sibling, siblings)
    {
    if (sibling != p && sibling->isAutoSpinBoxWidth())
      {
      sibling->qctk_d()->MinimumSpinBox->setMinimumWidth(width);
      sibling->qctk_d()->MaximumSpinBox->setMinimumWidth(width);
      }
    }
}

// --------------------------------------------------------------------------
qCTKRangeWidget::qCTKRangeWidget(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKRangeWidget);
  QCTK_D(qCTKRangeWidget);
  
  d->setupUi(this);

  d->MinimumSpinBox->setMinimum(d->Slider->minimum());
  d->MinimumSpinBox->setMaximum(d->Slider->maximum());
  d->MaximumSpinBox->setMinimum(d->Slider->minimum());
  d->MaximumSpinBox->setMaximum(d->Slider->maximum());
  d->MinimumSpinBox->setValue(d->Slider->minimumValue());
  d->MaximumSpinBox->setValue(d->Slider->maximumValue());
  this->connect(d->Slider, SIGNAL(minimumValueChanged(double)), d->MinimumSpinBox, SLOT(setValue(double)));
  this->connect(d->MinimumSpinBox, SIGNAL(valueChanged(double)), d->Slider, SLOT(setMinimumValue(double)));
  this->connect(d->Slider, SIGNAL(maximumValueChanged(double)), d->MaximumSpinBox, SLOT(setValue(double)));
  this->connect(d->MaximumSpinBox, SIGNAL(valueChanged(double)), d->Slider, SLOT(setMaximumValue(double)));
  this->connect(d->MinimumSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setMinimumToMaximumSpinBox(double)));
  this->connect(d->MaximumSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setMaximumToMinimumSpinBox(double)));

  this->connect(d->Slider, SIGNAL(sliderPressed()), this, SLOT(startChanging()));
  this->connect(d->Slider, SIGNAL(sliderReleased()), this, SLOT(stopChanging()));
  this->connect(d->Slider, SIGNAL(minimumValueChanged(double)), this, SLOT(changeMinimumValue(double)));
  this->connect(d->Slider, SIGNAL(maximumValueChanged(double)), this, SLOT(changeMaximumValue(double)));
  d->MinimumSpinBox->installEventFilter(this);
  d->MaximumSpinBox->installEventFilter(this);
}

// --------------------------------------------------------------------------
double qCTKRangeWidget::minimum()const
{
  QCTK_D(const qCTKRangeWidget);
  Q_ASSERT(d->equal(d->MinimumSpinBox->minimum(),d->Slider->minimum()));
  return d->Slider->minimum();
}

// --------------------------------------------------------------------------
double qCTKRangeWidget::maximum()const
{
  QCTK_D(const qCTKRangeWidget);
  Q_ASSERT(d->equal(d->MaximumSpinBox->maximum(), d->Slider->maximum()));
  return d->Slider->maximum();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setMinimum(double min)
{
  QCTK_D(qCTKRangeWidget);
  d->MinimumSpinBox->setMinimum(min);
  // SpinBox can truncate min (depending on decimals).
  // use Spinbox's min to set Slider's min
  d->Slider->setMinimum(d->MinimumSpinBox->minimum());
  Q_ASSERT(d->equal(d->MinimumSpinBox->minimum(),d->Slider->minimum()));
  d->updateSpinBoxWidth();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setMaximum(double max)
{
  QCTK_D(qCTKRangeWidget);
  d->MaximumSpinBox->setMaximum(max);
  // SpinBox can truncate max (depending on decimals).
  // use Spinbox's max to set Slider's max
  d->Slider->setMaximum(d->MaximumSpinBox->maximum());
  Q_ASSERT(d->equal(d->MaximumSpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setRange(double min, double max)
{
  QCTK_D(qCTKRangeWidget);
  
  d->MinimumSpinBox->setMinimum(qMin(min,max));
  d->MaximumSpinBox->setMaximum(qMax(min,max));
  // SpinBox can truncate the range (depending on decimals).
  // use Spinbox's range to set Slider's range
  d->Slider->setRange(d->MinimumSpinBox->minimum(), d->MaximumSpinBox->maximum());
  Q_ASSERT(d->equal(d->MinimumSpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->MaximumSpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}
/*
// --------------------------------------------------------------------------
double qCTKRangeWidget::sliderPosition()const
{
  return qctk_d()->Slider->sliderPosition();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setSliderPosition(double position)
{
  qctk_d()->Slider->setSliderPosition(position);
}
*/
/*
// --------------------------------------------------------------------------
double qCTKRangeWidget::previousSliderPosition()
{
  return qctk_d()->Slider->previousSliderPosition();
}
*/

// --------------------------------------------------------------------------
double qCTKRangeWidget::minimumValue()const
{
  QCTK_D(const qCTKRangeWidget);
  Q_ASSERT(d->equal(d->Slider->minimumValue(), d->MinimumSpinBox->value()));
  return d->Changing ? d->MinimumValueBeforeChange : d->Slider->minimumValue();
}

// --------------------------------------------------------------------------
double qCTKRangeWidget::maximumValue()const
{
  QCTK_D(const qCTKRangeWidget);
  Q_ASSERT(d->equal(d->Slider->maximumValue(), d->MaximumSpinBox->value()));
  return d->Changing ? d->MaximumValueBeforeChange : d->Slider->maximumValue();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setMinimumValue(double _value)
{
  QCTK_D(qCTKRangeWidget);
  // disable the tracking temporally to emit the
  // signal valueChanged if changeValue() is called
  bool isChanging = d->Changing;
  d->Changing = false;
  d->MinimumSpinBox->setValue(_value);
  // Why do we need to set the value to the slider ?
  //d->Slider->setValue(d->SpinBox->value());
  Q_ASSERT(d->equal(d->Slider->minimumValue(), d->MinimumSpinBox->value()));
  // restore the prop
  d->Changing = isChanging;
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setMaximumValue(double _value)
{
  QCTK_D(qCTKRangeWidget);
  // disable the tracking temporally to emit the
  // signal valueChanged if changeValue() is called
  bool isChanging = d->Changing;
  d->Changing = false;
  d->MaximumSpinBox->setValue(_value);
  // Why do we need to set the value to the slider ?
  //d->Slider->setValue(d->SpinBox->value());
  Q_ASSERT(d->equal(d->Slider->maximumValue(), d->MaximumSpinBox->value()));
  // restore the prop
  d->Changing = isChanging;
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setMinimumToMaximumSpinBox(double minimum)
{
  qctk_d()->MaximumSpinBox->setMinimum(minimum);
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setMaximumToMinimumSpinBox(double maximum)
{
  qctk_d()->MinimumSpinBox->setMaximum(maximum);
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::startChanging()
{
  QCTK_D(qCTKRangeWidget);
  if (d->Tracking)
    {
    return;
    }
  d->Changing = true;
  d->MinimumValueBeforeChange = this->minimumValue();
  d->MaximumValueBeforeChange = this->maximumValue();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::stopChanging()
{
  QCTK_D(qCTKRangeWidget);
  if (d->Tracking)
    {
    return;
    }
  d->Changing = false;
  if (qAbs(this->minimumValue() - d->MinimumValueBeforeChange) > (this->singleStep() * 0.000000001))
    {
    emit this->minimumValueChanged(this->minimumValue());
    }
  if (qAbs(this->maximumValue() - d->MaximumValueBeforeChange) > (this->singleStep() * 0.000000001))
    {
    emit this->maximumValueChanged(this->maximumValue());
    }
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::changeMinimumValue(double newValue)
{
  QCTK_D(qCTKRangeWidget);
  //if (d->Tracking)
    {
    emit this->minimumValueIsChanging(newValue);
    }
  if (!d->Changing)
    {
    emit this->minimumValueChanged(newValue);
    }
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::changeMaximumValue(double newValue)
{
  QCTK_D(qCTKRangeWidget);
  //if (d->Tracking)
    {
    emit this->maximumValueIsChanging(newValue);
    }
  if (!d->Changing)
    {
    emit this->maximumValueChanged(newValue);
    }
}

// --------------------------------------------------------------------------
bool qCTKRangeWidget::eventFilter(QObject *obj, QEvent *event)
 {
   if (event->type() == QEvent::MouseButtonPress)
     {
     QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
     if (mouseEvent->button() & Qt::LeftButton)
       {
       this->startChanging();
       }
     }
   else if (event->type() == QEvent::MouseButtonRelease) 
     {
     QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
     if (mouseEvent->button() & Qt::LeftButton)
       {
       // here we might prevent qCTKRangeWidget::stopChanging
       // from sending a valueChanged() event as the spinbox might
       // send a valueChanged() after eventFilter() is done.
       this->stopChanging();
       }
     } 
   // standard event processing
   return this->Superclass::eventFilter(obj, event);
 }

// --------------------------------------------------------------------------
double qCTKRangeWidget::singleStep()const
{
  QCTK_D(const qCTKRangeWidget);
  Q_ASSERT(d->equal(d->Slider->singleStep(), d->MinimumSpinBox->singleStep()) &&
           d->equal(d->Slider->singleStep(), d->MaximumSpinBox->singleStep()));
  return d->Slider->singleStep();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setSingleStep(double step)
{
  QCTK_D(qCTKRangeWidget);
  d->MinimumSpinBox->setSingleStep(step);
  d->MaximumSpinBox->setSingleStep(step);
  d->Slider->setSingleStep(d->MinimumSpinBox->singleStep());
  Q_ASSERT(d->equal(d->Slider->singleStep(), d->MinimumSpinBox->singleStep()) &&
           d->equal(d->Slider->singleStep(), d->MaximumSpinBox->singleStep()));
}

// --------------------------------------------------------------------------
int qCTKRangeWidget::decimals()const
{
  QCTK_D(const qCTKRangeWidget);
  Q_ASSERT(d->MinimumSpinBox->decimals() == d->MaximumSpinBox->decimals());
  return d->MinimumSpinBox->decimals();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setDecimals(int newDecimals)
{
  QCTK_D(qCTKRangeWidget);
  d->MinimumSpinBox->setDecimals(newDecimals);
  d->MaximumSpinBox->setDecimals(newDecimals);
}

// --------------------------------------------------------------------------
QString qCTKRangeWidget::prefix()const
{
  QCTK_D(const qCTKRangeWidget);
  Q_ASSERT(d->MinimumSpinBox->prefix() == d->MaximumSpinBox->prefix());
  return d->MinimumSpinBox->prefix();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setPrefix(const QString& newPrefix)
{
  QCTK_D(qCTKRangeWidget);
  d->MinimumSpinBox->setPrefix(newPrefix);
  d->MaximumSpinBox->setPrefix(newPrefix);
}

// --------------------------------------------------------------------------
QString qCTKRangeWidget::suffix()const
{
  QCTK_D(const qCTKRangeWidget);
 Q_ASSERT(d->MinimumSpinBox->suffix() == d->MaximumSpinBox->suffix());
  return d->MinimumSpinBox->suffix();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setSuffix(const QString& newSuffix)
{
  QCTK_D(qCTKRangeWidget);
  d->MinimumSpinBox->setSuffix(newSuffix);
  d->MaximumSpinBox->setSuffix(newSuffix);
}

// --------------------------------------------------------------------------
double qCTKRangeWidget::tickInterval()const
{
  QCTK_D(const qCTKRangeWidget);
  return d->Slider->tickInterval();
}

// --------------------------------------------------------------------------
void qCTKRangeWidget::setTickInterval(double ti)
{ 
  QCTK_D(qCTKRangeWidget);
  d->Slider->setTickInterval(ti);
}

// -------------------------------------------------------------------------
void qCTKRangeWidget::reset()
{
  this->setMinimumValue(this->minimum());
  this->setMaximumValue(this->maximum());
}

// -------------------------------------------------------------------------
void qCTKRangeWidget::setSpinBoxAlignment(Qt::Alignment alignment)
{
  QCTK_D(qCTKRangeWidget);
  d->MinimumSpinBox->setAlignment(alignment);
  d->MaximumSpinBox->setAlignment(alignment);
}

// -------------------------------------------------------------------------
Qt::Alignment qCTKRangeWidget::spinBoxAlignment()const
{
  QCTK_D(const qCTKRangeWidget);
  Q_ASSERT(d->MinimumSpinBox->alignment() == d->MaximumSpinBox->alignment());
  return d->MinimumSpinBox->alignment();
}

// -------------------------------------------------------------------------
void qCTKRangeWidget::setTracking(bool enable)
{
  QCTK_D(qCTKRangeWidget);
  d->Tracking = enable;
}

// -------------------------------------------------------------------------
bool qCTKRangeWidget::hasTracking()const
{
  QCTK_D(const qCTKRangeWidget);
  return d->Tracking;
}

// -------------------------------------------------------------------------
bool qCTKRangeWidget::isAutoSpinBoxWidth()const
{
  QCTK_D(const qCTKRangeWidget);
  return d->AutoSpinBoxWidth;
}

// -------------------------------------------------------------------------
void qCTKRangeWidget::setAutoSpinBoxWidth(bool autoWidth)
{
  QCTK_D(qCTKRangeWidget);
  d->AutoSpinBoxWidth = autoWidth;
  d->updateSpinBoxWidth();
}
