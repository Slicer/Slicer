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
#include <QWidgetAction>

// CTK includes
#include <ctkUtils.h>

// qMRML includes
#include "qMRMLVolumeWidget.h"
#include "qMRMLVolumeWidget_p.h"
#include <qMRMLSpinBox.h>

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

// VTK includes
#include <vtkImageData.h>

// --------------------------------------------------------------------------
qMRMLVolumeWidgetPrivate ::qMRMLVolumeWidgetPrivate(qMRMLVolumeWidget& object)
  : q_ptr(&object)
{
  this->VolumeNode = nullptr;
  this->VolumeDisplayNode = nullptr;
  this->OptionsMenu = nullptr;
  this->MinRangeSpinBox = nullptr;
  this->MaxRangeSpinBox = nullptr;
  this->DisplayScalarRange[0] = 0;
  this->DisplayScalarRange[1] = 0;
}

// --------------------------------------------------------------------------
qMRMLVolumeWidgetPrivate::~qMRMLVolumeWidgetPrivate()
{
  delete this->OptionsMenu;
  this->OptionsMenu = nullptr;
  this->MinRangeSpinBox = nullptr;
  this->MaxRangeSpinBox = nullptr;
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate::init()
{
  Q_Q(qMRMLVolumeWidget);

  this->setParent(q);
  // disable as there is not MRML Node associated with the widget
  q->setEnabled(this->VolumeDisplayNode != nullptr);

  QWidget* rangeWidget = new QWidget(q);
  QHBoxLayout* rangeLayout = new QHBoxLayout;
  rangeWidget->setLayout(rangeLayout);
  rangeLayout->setContentsMargins(0, 0, 0, 0);

  this->MinRangeSpinBox = new qMRMLSpinBox(rangeWidget);
  this->MinRangeSpinBox->setPrefix("Min: ");
  this->MinRangeSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  this->MinRangeSpinBox->setValue(this->MinRangeSpinBox->minimum());
  this->MinRangeSpinBox->setToolTip(
    qMRMLVolumeWidget::tr("Set the range boundaries to control large numbers or allow fine tuning"));
  connect(this->MinRangeSpinBox, SIGNAL(editingFinished()), this, SLOT(updateRangeFromSpinBox()));
  rangeLayout->addWidget(this->MinRangeSpinBox);

  this->MaxRangeSpinBox = new qMRMLSpinBox(rangeWidget);
  this->MaxRangeSpinBox->setPrefix("Max: ");
  this->MaxRangeSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  this->MaxRangeSpinBox->setValue(this->MaxRangeSpinBox->maximum());
  this->MaxRangeSpinBox->setToolTip(
    qMRMLVolumeWidget::tr("Set the range boundaries to control large numbers or allow fine tuning"));
  connect(this->MaxRangeSpinBox, SIGNAL(editingFinished()), this, SLOT(updateRangeFromSpinBox()));
  rangeLayout->addWidget(this->MaxRangeSpinBox);

  QWidgetAction* rangeAction = new QWidgetAction(this);
  rangeAction->setDefaultWidget(rangeWidget);

  this->OptionsMenu = new QMenu(q);
  this->OptionsMenu->addAction(rangeAction);
}

// --------------------------------------------------------------------------
bool qMRMLVolumeWidgetPrivate::blockSignals(bool block)
{
  return this->MinRangeSpinBox->blockSignals(block) && this->MaxRangeSpinBox->blockSignals(block);
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate::updateSingleStep(double min, double max)
{
  double interval = max - min;
  int order = ctk::orderOfMagnitude(interval);
  double minRangeSliderMinimumStep = 0.0;
  double maxRangeSliderMinimumStep = 0.0;
  if (order == std::numeric_limits<int>::min())
  {
    // the order of magnitude can't be computed (e.g. 0, inf, Nan, denorm)...
    order = -2;
    // Use the same minimum step as in ctkDoubleRangeSlider::isValidStep
    minRangeSliderMinimumStep = qMax(this->MinRangeSpinBox->maximum() / std::numeric_limits<double>::max(),
                                     std::numeric_limits<double>::epsilon());
    maxRangeSliderMinimumStep = qMax(this->MaxRangeSpinBox->maximum() / std::numeric_limits<double>::max(),
                                     std::numeric_limits<double>::epsilon());
  }

  int ratio = 2;
  double singleStep = pow(10., order - ratio);
  int decimals = qMax(0, -order + ratio);

  this->setDecimals(decimals);
  this->setSingleStep(singleStep);

  // The RangeWidget doesn't have to be as precise as the sliders/spinboxes.
  ratio = 1;
  singleStep = pow(10., order - ratio);
  decimals = qMax(0, -order + ratio);

  this->MinRangeSpinBox->setSingleStep(qMax(singleStep, minRangeSliderMinimumStep));
  this->MaxRangeSpinBox->setSingleStep(qMax(singleStep, maxRangeSliderMinimumStep));
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate::setDecimals(int decimals)
{
  Q_UNUSED(decimals);
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate::setSingleStep(double singleStep)
{
  Q_UNUSED(singleStep);
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate::setRange(double min, double max)
{
  this->updateSingleStep(min, max);
  this->MinRangeSpinBox->setValue(min);
  this->MaxRangeSpinBox->setValue(max);
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate::updateRangeFromSpinBox()
{
  this->setRange(this->MinRangeSpinBox->value(), this->MaxRangeSpinBox->value());
}

// --------------------------------------------------------------------------
qMRMLVolumeWidget::qMRMLVolumeWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLVolumeWidgetPrivate(*this))
{
  Q_D(qMRMLVolumeWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLVolumeWidget ::qMRMLVolumeWidget(qMRMLVolumeWidgetPrivate* ptr, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(ptr)
{
}

// --------------------------------------------------------------------------
qMRMLVolumeWidget::~qMRMLVolumeWidget() = default;

// --------------------------------------------------------------------------
void qMRMLVolumeWidget ::setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* node)
{
  Q_D(qMRMLVolumeWidget);
  if (d->VolumeDisplayNode == node)
  {
    return;
  }

  // each time the node is modified, the qt widgets are updated
  this->qvtkReconnect(
    d->VolumeDisplayNode, node, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRMLDisplayNode()));

  d->VolumeDisplayNode = node;

  this->updateWidgetFromMRMLDisplayNode();
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidget::setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode)
{
  Q_D(qMRMLVolumeWidget);
  if (volumeNode == d->VolumeNode)
  {
    return;
  }

  this->qvtkReconnect(
    d->VolumeNode, volumeNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRMLVolumeNode()));

  d->VolumeNode = volumeNode;
  this->updateWidgetFromMRMLVolumeNode();
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qMRMLVolumeWidget::mrmlVolumeNode() const
{
  Q_D(const qMRMLVolumeWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode* qMRMLVolumeWidget::mrmlDisplayNode() const
{
  Q_D(const qMRMLVolumeWidget);
  return d->VolumeDisplayNode;
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidget::updateWidgetFromMRMLVolumeNode()
{
  Q_D(qMRMLVolumeWidget);

  // Make sure the display node reference is up-to-date
  vtkMRMLScalarVolumeDisplayNode* newVolumeDisplayNode =
    d->VolumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(d->VolumeNode->GetVolumeDisplayNode()) : nullptr;
  this->setMRMLVolumeDisplayNode(newVolumeDisplayNode);

  // We always need to set the slider values and range at the same time
  // to make sure that they are consistent. This is implemented in one place,
  // in updateWidgetFromMRMLDisplayNode().
  this->updateWidgetFromMRMLDisplayNode();
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidget::updateWidgetFromMRMLDisplayNode()
{
  Q_D(qMRMLVolumeWidget);
  this->setEnabled(d->VolumeDisplayNode != nullptr && d->VolumeNode != nullptr);
  if (d->VolumeDisplayNode && d->VolumeDisplayNode->GetInputImageData())
  {
    d->VolumeDisplayNode->GetDisplayScalarRange(d->DisplayScalarRange);
  }
  else
  {
    d->DisplayScalarRange[0] = 0.;
    d->DisplayScalarRange[1] = 0.;
  }
}
