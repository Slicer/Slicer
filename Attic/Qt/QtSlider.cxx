#include "QtSlider.h"

// QT includes
#include <QLineEdit>
#include <QSlider>
#include <QHBoxLayout>
#include <QDoubleValidator>

QtSlider::QtSlider(QWidget* p)
  : QWidget(p)
{
  this->BlockUpdate = false;
  this->Value = 0;
  this->Minimum = 0;
  this->Maximum = 1;
  this->Resolution = 100;
  this->StrictRange = false;

  QHBoxLayout* l = new QHBoxLayout(this);
  l->setMargin(0);
  this->Slider = new QSlider(Qt::Horizontal, this);
  this->Slider->setRange(0, this->Resolution);
  l->addWidget(this->Slider);
  this->Slider->setObjectName("Slider");
  this->LineEdit = new QLineEdit(this);
  l->addWidget(this->LineEdit);
  this->LineEdit->setObjectName("LineEdit");
  this->LineEdit->setValidator(new QDoubleValidator(this->LineEdit));
  this->LineEdit->setText(QString().setNum(this->Value));

  QObject::connect(this->Slider, SIGNAL(valueChanged(int)),
                   this, SLOT(sliderChanged(int)));
  QObject::connect(this->LineEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(textChanged(const QString&)));
  QObject::connect(this->LineEdit, SIGNAL(editingFinished()),
                   this, SLOT(editingFinished()));

}

//-----------------------------------------------------------------------------
QtSlider::~QtSlider()
{
}

//-----------------------------------------------------------------------------
int QtSlider::resolution() const
{
  return this->Resolution;
}

//-----------------------------------------------------------------------------
void QtSlider::setResolution(int val)
{
  this->Resolution = val;
  this->Slider->setRange(0, this->Resolution);
  this->updateSlider();
}

//-----------------------------------------------------------------------------
double QtSlider::value() const
{
  return this->Value;
}

//-----------------------------------------------------------------------------
void QtSlider::setValue(double val)
{
  if(this->Value == val)
    {
    return;
    }

  this->Value = val;

  if(!this->BlockUpdate)
    {
    // set the slider
    this->updateSlider();

    // set the text
    this->BlockUpdate = true;
    this->LineEdit->setText(QString().setNum(val));
    this->BlockUpdate = false;
    }

  emit this->valueChanged(this->Value);
}

//-----------------------------------------------------------------------------
double QtSlider::maximum() const
{
  return this->Maximum;
}

//-----------------------------------------------------------------------------
void QtSlider::setMaximum(double val)
{
  this->Maximum = val;
  this->updateValidator();
  this->updateSlider();
}

//-----------------------------------------------------------------------------
double QtSlider::minimum() const
{
  return this->Minimum;
}

//-----------------------------------------------------------------------------
void QtSlider::setMinimum(double val)
{
  this->Minimum = val;
  this->updateValidator();
  this->updateSlider();
}

//-----------------------------------------------------------------------------
void QtSlider::updateValidator()
{
  if(this->StrictRange)
    {
    this->LineEdit->setValidator(new QDoubleValidator(this->minimum(),
        this->maximum(), 100, this->LineEdit));
    }
  else
    {
    this->LineEdit->setValidator(new QDoubleValidator(this->LineEdit));
    }
}

//-----------------------------------------------------------------------------
bool QtSlider::strictRange() const
{
  const QDoubleValidator* dv =
    qobject_cast<const QDoubleValidator*>(this->LineEdit->validator());
  return dv->bottom() == this->minimum() && dv->top() == this->maximum();
}

void QtSlider::setStrictRange(bool s)
{
  this->StrictRange = s;
  this->updateValidator();
}

//-----------------------------------------------------------------------------
void QtSlider::sliderChanged(int val)
{
  if(!this->BlockUpdate)
    {
    double fraction = val / static_cast<double>(this->Resolution);
    double range = this->Maximum - this->Minimum;
    double v = (fraction * range) + this->Minimum;
    this->BlockUpdate = true;
    this->LineEdit->setText(QString().setNum(v));
    this->setValue(v);
    emit this->valueEdited(v);
    this->BlockUpdate = false;
    }
}

//-----------------------------------------------------------------------------
void QtSlider::textChanged(const QString& text)
{
  if(!this->BlockUpdate)
    {
    double val = text.toDouble();
    this->BlockUpdate = true;
    double range = this->Maximum - this->Minimum;
    double fraction = (val - this->Minimum) / range;
    int sliderVal = qRound(fraction * static_cast<double>(this->Resolution));
    this->Slider->setValue(sliderVal);
    this->setValue(val);
    this->BlockUpdate = false;
    }
}

//-----------------------------------------------------------------------------
void QtSlider::editingFinished()
{
  emit this->valueEdited(this->Value);
}

//-----------------------------------------------------------------------------
void QtSlider::updateSlider()
{
  this->Slider->blockSignals(true);
  double range = this->Maximum - this->Minimum;
  double fraction = (this->Value - this->Minimum) / range;
  int v = qRound(fraction * static_cast<double>(this->Resolution));
  this->Slider->setValue(v);
  this->Slider->blockSignals(false);
}

