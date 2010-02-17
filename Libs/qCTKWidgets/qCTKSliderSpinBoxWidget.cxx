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
#include <QMouseEvent>

bool equal(double v1, double v2)
{
  return qAbs(v1 - v2) < 0.0001;
}

//-----------------------------------------------------------------------------
class qCTKSliderSpinBoxWidgetPrivate: public qCTKPrivate<qCTKSliderSpinBoxWidget>,
                                      public Ui_qCTKSliderSpinBoxWidget
{
public:
  qCTKSliderSpinBoxWidgetPrivate();
  void updateSpinBoxWidth();
  int synchronizedSpinBoxWidth()const;
  void synchronizeSiblingSpinBox(int newWidth);

  bool   Tracking;
  bool   Changing;
  double ValueBeforeChange;
  bool   AutoSpinBoxWidth;
};

// --------------------------------------------------------------------------
qCTKSliderSpinBoxWidgetPrivate::qCTKSliderSpinBoxWidgetPrivate()
{
  this->Tracking = true;
  this->Changing = false;
  this->ValueBeforeChange = 0.;
  this->AutoSpinBoxWidth = true;
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidgetPrivate::updateSpinBoxWidth()
{
  int spinBoxWidth = this->synchronizedSpinBoxWidth();
  if (this->AutoSpinBoxWidth)
    {
    this->SpinBox->setMinimumWidth(spinBoxWidth);
    }
  else
    {
    this->SpinBox->setMinimumWidth(0);
    }
  this->synchronizeSiblingSpinBox(spinBoxWidth);
}

// --------------------------------------------------------------------------
int qCTKSliderSpinBoxWidgetPrivate::synchronizedSpinBoxWidth()const
{
  QCTK_P(const qCTKSliderSpinBoxWidget);
  int maxWidth = this->SpinBox->sizeHint().width();
  if (!p->parent())
    {
    return maxWidth;
    }
  QList<qCTKSliderSpinBoxWidget*> siblings = 
    p->parent()->findChildren<qCTKSliderSpinBoxWidget*>();
  foreach(qCTKSliderSpinBoxWidget* sibling, siblings)
    {
    maxWidth = qMax(maxWidth, sibling->qctk_d()->SpinBox->sizeHint().width());
    }
  return maxWidth;
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidgetPrivate::synchronizeSiblingSpinBox(int width)
{
  QCTK_P(const qCTKSliderSpinBoxWidget);
  QList<qCTKSliderSpinBoxWidget*> siblings = 
    p->parent()->findChildren<qCTKSliderSpinBoxWidget*>();
  foreach(qCTKSliderSpinBoxWidget* sibling, siblings)
    {
    if (sibling != p && sibling->isAutoSpinBoxWidth())
      {
      sibling->qctk_d()->SpinBox->setMinimumWidth(width);
      }
    }
}

// --------------------------------------------------------------------------
qCTKSliderSpinBoxWidget::qCTKSliderSpinBoxWidget(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKSliderSpinBoxWidget);
  QCTK_D(qCTKSliderSpinBoxWidget);
  
  d->setupUi(this);

  d->Slider->setMaximum(d->SpinBox->maximum());
  d->Slider->setMinimum(d->SpinBox->minimum());

  this->connect(d->Slider, SIGNAL(valueChanged(double)), d->SpinBox, SLOT(setValue(double)));
  this->connect(d->SpinBox, SIGNAL(valueChanged(double)), d->Slider, SLOT(setValue(double)));

  //this->connect(d->Slider, SIGNAL(valueChanged(double)), SIGNAL(valueChanged(double)));
  this->connect(d->Slider, SIGNAL(sliderPressed()), this, SLOT(startChanging()));
  this->connect(d->Slider, SIGNAL(sliderReleased()), this, SLOT(stopChanging()));
  this->connect(d->Slider, SIGNAL(valueChanged(double)), this, SLOT(changeValue(double)));
  d->SpinBox->installEventFilter(this);
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
  d->SpinBox->setMinimum(min);
  // SpinBox can truncate min (depending on decimals).
  // use Spinbox's min to set Slider's min
  d->Slider->setMinimum(d->SpinBox->minimum());
  Q_ASSERT(equal(d->SpinBox->minimum(),d->Slider->minimum()));
  d->updateSpinBoxWidth();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setMaximum(double max)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->SpinBox->setMaximum(max);
  // SpinBox can truncate max (depending on decimals).
  // use Spinbox's max to set Slider's max
  d->Slider->setMaximum(d->SpinBox->maximum());
  Q_ASSERT(equal(d->SpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setRange(double min, double max)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  
  d->SpinBox->setRange(min, max);
  // SpinBox can truncate the range (depending on decimals).
  // use Spinbox's range to set Slider's range
  d->Slider->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
  Q_ASSERT(equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(equal(d->SpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}
/*
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
*/
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
  return d->Changing ? d->ValueBeforeChange : d->Slider->value();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setValue(double _value)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  // disable the tracking temporally to emit the
  // signal valueChanged if changeValue() is called
  bool isChanging = d->Changing;
  d->Changing = false;
  d->SpinBox->setValue(_value);
  // Why do we need to set the value to the slider ?
  //d->Slider->setValue(d->SpinBox->value());
  Q_ASSERT(equal(d->Slider->value(), d->SpinBox->value()));
  // restore the prop
  d->Changing = isChanging;
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::startChanging()
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  if (d->Tracking)
    {
    return;
    }
  d->Changing = true;
  d->ValueBeforeChange = this->value();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::stopChanging()
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  if (d->Tracking)
    {
    return;
    }
  d->Changing = false;
  if (qAbs(this->value() - d->ValueBeforeChange) > (this->singleStep() * 0.000000001))
    {
    emit this->valueChanged(this->value());
    }
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::changeValue(double newValue)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  //if (d->Tracking)
    {
    emit this->valueIsChanging(newValue);
    }
  if (!d->Changing)
    {
    emit this->valueChanged(newValue);
    }
}

// --------------------------------------------------------------------------
bool qCTKSliderSpinBoxWidget::eventFilter(QObject *obj, QEvent *event)
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
       // here we might prevent qCTKSliderSpinBoxWidget::stopChanging
       // from sending a valueChanged() event as the spinbox might
       // send a valueChanged() after eventFilter() is done.
       this->stopChanging();
       }
     } 
   // standard event processing
   return this->Superclass::eventFilter(obj, event);
 }

// --------------------------------------------------------------------------
double qCTKSliderSpinBoxWidget::singleStep()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  Q_ASSERT(equal(d->Slider->singleStep(), d->SpinBox->singleStep()));
  return d->Slider->singleStep();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setSingleStep(double step)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->SpinBox->setSingleStep(step);
  d->Slider->setSingleStep(d->SpinBox->singleStep());
  Q_ASSERT(equal(d->Slider->singleStep(), d->SpinBox->singleStep()));
}

// --------------------------------------------------------------------------
int qCTKSliderSpinBoxWidget::decimals()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  return d->SpinBox->decimals();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setDecimals(int newDecimals)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->SpinBox->setDecimals(newDecimals);
}

// --------------------------------------------------------------------------
QString qCTKSliderSpinBoxWidget::prefix()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  return d->SpinBox->prefix();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setPrefix(const QString& newPrefix)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->SpinBox->setPrefix(newPrefix);
}

// --------------------------------------------------------------------------
QString qCTKSliderSpinBoxWidget::suffix()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  return d->SpinBox->suffix();
}

// --------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setSuffix(const QString& newSuffix)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->SpinBox->setSuffix(newSuffix);
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
  this->setValue(0.);
}

// -------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setSpinBoxAlignment(Qt::Alignment alignment)
{
  return qctk_d()->SpinBox->setAlignment(alignment);
}

// -------------------------------------------------------------------------
Qt::Alignment qCTKSliderSpinBoxWidget::spinBoxAlignment()const
{
  return qctk_d()->SpinBox->alignment();
}

// -------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setTracking(bool enable)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->Tracking = enable;
}

// -------------------------------------------------------------------------
bool qCTKSliderSpinBoxWidget::hasTracking()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  return d->Tracking;
}

// -------------------------------------------------------------------------
bool qCTKSliderSpinBoxWidget::isAutoSpinBoxWidth()const
{
  QCTK_D(const qCTKSliderSpinBoxWidget);
  return d->AutoSpinBoxWidth;
}

// -------------------------------------------------------------------------
void qCTKSliderSpinBoxWidget::setAutoSpinBoxWidth(bool autoWidth)
{
  QCTK_D(qCTKSliderSpinBoxWidget);
  d->AutoSpinBoxWidth = autoWidth;
  d->updateSpinBoxWidth();
}
