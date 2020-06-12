/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <QHBoxLayout>
#include <QMenu>
#include <QStyleOptionSlider>
#include <QToolButton>
#include <QWidgetAction>

// qMRML includes
#include <qMRMLSpinBox.h>

#include "qMRMLRangeWidget.h"

// --------------------------------------------------------------------------
// qMRMLRangeWidget
//

// --------------------------------------------------------------------------
qMRMLRangeWidget::qMRMLRangeWidget(QWidget* parentWidget)
  : ctkRangeWidget(parentWidget)
{
  this->setSlider(new qMRMLDoubleRangeSlider(nullptr));

  QWidget* rangeWidget = new QWidget(this);
  QHBoxLayout* rangeLayout = new QHBoxLayout;
  rangeWidget->setLayout(rangeLayout);
  rangeLayout->setContentsMargins(0,0,0,0);

  this->MinSpinBox = new qMRMLSpinBox(rangeWidget);
  this->MinSpinBox->setPrefix("Min: ");
  this->MinSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  this->MinSpinBox->setValue(this->minimum());
  connect(this->MinSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateRange()));
  rangeLayout->addWidget(this->MinSpinBox);

  this->MaxSpinBox = new qMRMLSpinBox(rangeWidget);
  this->MaxSpinBox->setPrefix("Max: ");
  this->MaxSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  this->MaxSpinBox->setValue(this->maximum());
  connect(this->MaxSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateRange()));
  rangeLayout->addWidget(this->MaxSpinBox);

  connect(this->slider(), SIGNAL(rangeChanged(double,double)),
          this, SLOT(updateSpinBoxRange(double,double)));

  QWidgetAction* rangeAction = new QWidgetAction(this);
  rangeAction->setDefaultWidget(rangeWidget);

  this->SymmetricAction = new QAction(tr("Symmetric handles"),this);
  this->SymmetricAction->setCheckable(true);
  connect(this->SymmetricAction, SIGNAL(toggled(bool)),
          this, SLOT(updateSymmetricMoves(bool)));
  this->SymmetricAction->setChecked(this->symmetricMoves());

  QMenu* optionsMenu = new QMenu(this);
  optionsMenu->addAction(rangeAction);
  optionsMenu->addAction(this->SymmetricAction);

  QToolButton* optionsButton = new QToolButton(this);
  optionsButton->setIcon(QIcon(":Icons/SliceMoreOptions.png"));
  optionsButton->setMenu(optionsMenu);
  optionsButton->setPopupMode(QToolButton::InstantPopup);
  QGridLayout* gridLayout = qobject_cast<QGridLayout*>(this->layout());
  gridLayout->addWidget(optionsButton,0,3);
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
void qMRMLRangeWidget::updateSpinBoxRange(double min, double max)
{
  // We must set the values at the same time and update the pipeline
  // when the MinSpinBox is set but not the MaxSpinBox. This could generate
  // infinite loop
  bool minSpinBoxBlocked = this->MinSpinBox->blockSignals(true);
  bool maxSpinBoxBlocked = this->MaxSpinBox->blockSignals(true);
  this->MinSpinBox->setValue(min);
  this->MaxSpinBox->setValue(max);
  this->MinSpinBox->blockSignals(minSpinBoxBlocked);
  this->MaxSpinBox->blockSignals(maxSpinBoxBlocked);
  this->updateRange();
}

// --------------------------------------------------------------------------
void qMRMLRangeWidget::updateRange()
{
  this->setRange(this->MinSpinBox->value(),
                 this->MaxSpinBox->value());
}

// --------------------------------------------------------------------------
void qMRMLRangeWidget::updateSymmetricMoves(bool symmetric)
{
  this->setSymmetricMoves(symmetric);
}

// --------------------------------------------------------------------------
void qMRMLRangeWidget::setSymmetricMoves(bool symmetry)
{
  if (symmetry==this->symmetricMoves())
    {
    return;
    }
  ctkRangeWidget::setSymmetricMoves(symmetry);
  const QSignalBlocker blocker(this->SymmetricAction);
  this->SymmetricAction->setChecked(symmetry);
}

//-----------------------------------------------------------------------------
void qMRMLRangeWidget::setQuantity(const QString& quantity)
{
  if (quantity == this->quantity())
    {
    return;
    }

  this->MinSpinBox->setQuantity(quantity);
  this->MaxSpinBox->setQuantity(quantity);
}

//-----------------------------------------------------------------------------
QString qMRMLRangeWidget::quantity()const
{
  Q_ASSERT(this->MinSpinBox->quantity() == this->MaxSpinBox->quantity());
  return this->MinSpinBox->quantity();
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLRangeWidget::mrmlScene()const
{
  Q_ASSERT(this->MinSpinBox->mrmlScene() == this->MaxSpinBox->mrmlScene());
  return this->MinSpinBox->mrmlScene();
}

// --------------------------------------------------------------------------
void qMRMLRangeWidget::setMRMLScene(vtkMRMLScene* scene)
{
  if (this->mrmlScene() == scene)
    {
    return;
    }

  this->MinSpinBox->setMRMLScene(scene);
  this->MaxSpinBox->setMRMLScene(scene);
  this->setEnabled(this->isEnabled() && scene != nullptr);
}

// --------------------------------------------------------------------------
// qMRMLDoubleRangeSlider
//

// --------------------------------------------------------------------------
qMRMLDoubleRangeSlider::qMRMLDoubleRangeSlider(QWidget* parentWidget)
  :ctkDoubleRangeSlider(parentWidget)
{
  this->setSlider(new qMRMLRangeSlider(nullptr));
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
// qMRMLRangeSlider
//

// --------------------------------------------------------------------------
class qMRMLRangeSliderPrivate
{
  Q_DECLARE_PUBLIC(qMRMLRangeSlider);
protected:
  qMRMLRangeSlider* const q_ptr;

public:
  qMRMLRangeSliderPrivate(qMRMLRangeSlider* widget);

  QPalette MinimumPalette;
  QPalette MaximumPalette;
};

// --------------------------------------------------------------------------
qMRMLRangeSliderPrivate::qMRMLRangeSliderPrivate(qMRMLRangeSlider* pub)
  : q_ptr(pub)
{
}

// --------------------------------------------------------------------------
qMRMLRangeSlider::qMRMLRangeSlider(QWidget* parentWidget)
  :ctkRangeSlider(parentWidget)
   ,d_ptr(new qMRMLRangeSliderPrivate(this))
{
}

// --------------------------------------------------------------------------
qMRMLRangeSlider::~qMRMLRangeSlider() = default;

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
