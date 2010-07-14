// Qt includes
#include <QDebug>
#include <QStyleOptionSlider>

// CTK includes
#include "qMRMLRangeWidget.h"

// --------------------------------------------------------------------------
qMRMLRangeWidget::qMRMLRangeWidget(QWidget* parentWidget)
  : ctkRangeWidget(parentWidget)
{
  this->setSlider(new qMRMLDoubleRangeSlider(0));
}

// --------------------------------------------------------------------------
QPalette qMRMLRangeWidget::minimumHandlePalette()const
{
  return qobject_cast<qMRMLDoubleRangeSlider*>(this->slider())
    ->minimumHandlePalette();
}

// --------------------------------------------------------------------------
QPalette qMRMLRangeWidget::maximumHandlePalette()const
{
  return qobject_cast<qMRMLDoubleRangeSlider*>(this->slider())
    ->maximumHandlePalette();
}

// --------------------------------------------------------------------------
void qMRMLRangeWidget::setMinimumHandlePalette(const QPalette& palette)
{
  qobject_cast<qMRMLDoubleRangeSlider*>(this->slider())
    ->setMinimumHandlePalette(palette);
}

// --------------------------------------------------------------------------
void qMRMLRangeWidget::setMaximumHandlePalette(const QPalette& palette)
{
  qobject_cast<qMRMLDoubleRangeSlider*>(this->slider())
    ->setMaximumHandlePalette(palette);
}

// --------------------------------------------------------------------------
qMRMLDoubleRangeSlider::qMRMLDoubleRangeSlider(QWidget* parentWidget)
  :ctkDoubleRangeSlider(parentWidget)
{
  this->setSlider(new qMRMLRangeSlider(0));
}

// --------------------------------------------------------------------------
QPalette qMRMLDoubleRangeSlider::minimumHandlePalette()const
{
  return qobject_cast<qMRMLRangeSlider*>(this->slider())
    ->minimumHandlePalette();
}

// --------------------------------------------------------------------------
QPalette qMRMLDoubleRangeSlider::maximumHandlePalette()const
{
  return qobject_cast<qMRMLRangeSlider*>(this->slider())
    ->maximumHandlePalette();
}

// --------------------------------------------------------------------------
void qMRMLDoubleRangeSlider::setMinimumHandlePalette(const QPalette& palette)
{
  qobject_cast<qMRMLRangeSlider*>(this->slider())
    ->setMinimumHandlePalette(palette);
}

// --------------------------------------------------------------------------
void qMRMLDoubleRangeSlider::setMaximumHandlePalette(const QPalette& palette)
{
  qobject_cast<qMRMLRangeSlider*>(this->slider())
    ->setMaximumHandlePalette(palette);
}

// --------------------------------------------------------------------------
class qMRMLRangeSliderPrivate: public ctkPrivate<qMRMLRangeSlider>
{
public:
  QPalette MinimumPalette;
  QPalette MaximumPalette;
};

// --------------------------------------------------------------------------
qMRMLRangeSlider::qMRMLRangeSlider(QWidget* parentWidget)
  :ctkRangeSlider(parentWidget)
{
}

// --------------------------------------------------------------------------
QPalette qMRMLRangeSlider::minimumHandlePalette()const
{
  CTK_D(const qMRMLRangeSlider);
  return d->MinimumPalette;
}

// --------------------------------------------------------------------------
QPalette qMRMLRangeSlider::maximumHandlePalette()const
{
  CTK_D(const qMRMLRangeSlider);
  return d->MaximumPalette;
}

// --------------------------------------------------------------------------
void qMRMLRangeSlider::setMinimumHandlePalette(const QPalette& palette)
{
  CTK_D(qMRMLRangeSlider);
  d->MinimumPalette = palette;
  this->update();
}

// --------------------------------------------------------------------------
void qMRMLRangeSlider::setMaximumHandlePalette(const QPalette& palette)
{
  CTK_D(qMRMLRangeSlider);
  d->MaximumPalette = palette;
  this->update();
}

//---------------------------------------------------------------------------
void qMRMLRangeSlider::initMinimumSliderStyleOption(QStyleOptionSlider* option) const
{
  CTK_D(const qMRMLRangeSlider);
  this->ctkRangeSlider::initStyleOption(option);
  option->palette = d->MinimumPalette;
}

//---------------------------------------------------------------------------
void qMRMLRangeSlider::initMaximumSliderStyleOption(QStyleOptionSlider* option) const
{
  CTK_D(const qMRMLRangeSlider);
  this->ctkRangeSlider::initStyleOption(option);
  option->palette = d->MaximumPalette;
}
