/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

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
class qMRMLRangeSliderPrivate
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
qMRMLRangeSlider::~qMRMLRangeSlider()
{
}

// --------------------------------------------------------------------------
QPalette qMRMLRangeSlider::minimumHandlePalette()const
{
  Q_D(const qMRMLRangeSlider);
  return d->MinimumPalette;
}

// --------------------------------------------------------------------------
QPalette qMRMLRangeSlider::maximumHandlePalette()const
{
  Q_D(const qMRMLRangeSlider);
  return d->MaximumPalette;
}

// --------------------------------------------------------------------------
void qMRMLRangeSlider::setMinimumHandlePalette(const QPalette& palette)
{
  Q_D(qMRMLRangeSlider);
  d->MinimumPalette = palette;
  this->update();
}

// --------------------------------------------------------------------------
void qMRMLRangeSlider::setMaximumHandlePalette(const QPalette& palette)
{
  Q_D(qMRMLRangeSlider);
  d->MaximumPalette = palette;
  this->update();
}

//---------------------------------------------------------------------------
void qMRMLRangeSlider::initMinimumSliderStyleOption(QStyleOptionSlider* option) const
{
  Q_D(const qMRMLRangeSlider);
  this->ctkRangeSlider::initStyleOption(option);
  option->palette = d->MinimumPalette;
}

//---------------------------------------------------------------------------
void qMRMLRangeSlider::initMaximumSliderStyleOption(QStyleOptionSlider* option) const
{
  Q_D(const qMRMLRangeSlider);
  this->ctkRangeSlider::initStyleOption(option);
  option->palette = d->MaximumPalette;
}
