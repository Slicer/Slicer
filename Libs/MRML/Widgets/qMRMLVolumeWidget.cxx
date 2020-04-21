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
#include <QVBoxLayout>

// CTK includes
#include <ctkDoubleSpinBox.h>
#include <ctkRangeWidget.h>
#include <ctkPopupWidget.h>
#include <ctkUtils.h>

// qMRML includes
#include "qMRMLVolumeWidget.h"
#include "qMRMLVolumeWidget_p.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

// VTK includes
#include <vtkImageData.h>

// --------------------------------------------------------------------------
qMRMLVolumeWidgetPrivate
::qMRMLVolumeWidgetPrivate(qMRMLVolumeWidget& object)
  : q_ptr(&object)
{
  this->VolumeNode = nullptr;
  this->VolumeDisplayNode = nullptr;
  this->PopupWidget = nullptr;
  this->RangeWidget = nullptr;
  this->DisplayScalarRange[0] = 0;
  this->DisplayScalarRange[1] = 0;
}

// --------------------------------------------------------------------------
qMRMLVolumeWidgetPrivate::~qMRMLVolumeWidgetPrivate()
{
  delete this->PopupWidget;
  this->PopupWidget = nullptr;
  this->RangeWidget = nullptr;
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate::init()
{
  Q_Q(qMRMLVolumeWidget);

  this->setParent(q);
  // disable as there is not MRML Node associated with the widget
  q->setEnabled(this->VolumeDisplayNode != nullptr);

  // we can't use the flag Qt::Popup as it automatically closes when there is
  // a click outside of the rangewidget
  this->PopupWidget = new ctkPopupWidget(q);
  this->PopupWidget->setObjectName("RangeWidgetPopup");

  QPalette popupPalette = q->palette();
  QColor windowColor = popupPalette.color(QPalette::Window);
  windowColor.setAlpha(200);
  QColor darkColor = popupPalette.color(QPalette::Dark);
  darkColor.setAlpha(200);
  /*
  QLinearGradient gradient(QPointF(0.,0.),QPointF(0.,0.5));
  gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
  gradient.setColorAt(0, windowColor);
  gradient.setColorAt(1, darkColor);
  popupPalette.setBrush(QPalette::Window, gradient);
  */
  popupPalette.setColor(QPalette::Window, darkColor);
  this->PopupWidget->setPalette(popupPalette);
  this->PopupWidget->setAttribute(Qt::WA_TranslucentBackground, true);

  this->PopupWidget->setAutoShow(false);
  this->PopupWidget->setAutoHide(true);
  //this->PopupWidget->setBaseWidget(q);
  this->RangeWidget = new ctkRangeWidget;
  this->RangeWidget->minimumSpinBox()->setDecimalsOption(
    ctkDoubleSpinBox::DecimalsByKey|ctkDoubleSpinBox::DecimalsByShortcuts);
  this->RangeWidget->maximumSpinBox()->setDecimalsOption(
    ctkDoubleSpinBox::DecimalsByKey|ctkDoubleSpinBox::DecimalsByShortcuts);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(this->RangeWidget);
  this->PopupWidget->setLayout(layout);

  QMargins margins = layout->contentsMargins();
  margins.setTop(0);
  layout->setContentsMargins(margins);

  this->RangeWidget->setSpinBoxAlignment(Qt::AlignBottom);
  this->RangeWidget->setRange(-1000000., 1000000.);
  QObject::connect(this->RangeWidget, SIGNAL(valuesChanged(double,double)),
                   this, SLOT(setRange(double,double)));
  this->RangeWidget->setToolTip(
        qMRMLVolumeWidget::tr("Set the range boundaries to control large numbers or allow fine tuning"));
}


//------------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate
::updateRangeForVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* dNode)
{
  double range[2];
  this->scalarRange(dNode, range);
  this->DisplayScalarRange[0] = range[0];
  this->DisplayScalarRange[1] = range[1];
  // we don't want RangeWidget to fire any signal because we don't have
  // a display node correctly set here (it's done )
  this->RangeWidget->blockSignals(true);
  double interval = range[1] - range[0];
  Q_ASSERT(interval >= 0.);
  double min, max;

  if (interval <= 10.)
    {
    min = qMin(-10., range[0] - 2.*interval);
    max = qMax(10., range[1] + 2.*interval);
    }
  else
    {
    min = qMin(-1200., range[0] - 2.*interval);
    max = qMax(900., range[1] + 2.*interval);
    }

  this->RangeWidget->setRange(min, max);
  this->RangeWidget->blockSignals(false);

  if (interval < 10.)
    {
    //give us some space
    range[0] = range[0] - interval*0.1;
    range[1] = range[1] + interval*0.1;
    }
  else
    {
    //give us some space
    range[0] = qMin(-600., range[0] - interval*0.1);
    range[1] = qMax(600., range[1] + interval*0.1);
    }
  bool blocked = this->blockSignals(true);
  this->setRange(range[0], range[1]);
  this->blockSignals(blocked);
}

// --------------------------------------------------------------------------
bool qMRMLVolumeWidgetPrivate::blockSignals(bool block)
{
  return this->RangeWidget->blockSignals(block);
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate
::scalarRange(vtkMRMLScalarVolumeDisplayNode* dNode, double range[2])
{
  // vtkMRMLScalarVolumeDisplayNode::GetDisplayScalarRange() can be a bit
  // slow if there is no input as it searches the scene for the associated
  // volume node.
  // Here we already know the volumenode so we can manually use it to
  // retrieve the scalar range.
  if (dNode && dNode->GetInputImageData())
    {
    dNode->GetDisplayScalarRange(range);
    }
  else if (this->VolumeNode->GetImageData())
    {
    this->VolumeNode->GetImageData()->GetScalarRange(range);
    }
  else
    {
    range[0] = 0.;
    range[1] = 0.;
    }
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidgetPrivate::updateSingleStep(double min, double max)
{
  double interval = max - min;
  int order = interval != 0. ? ctk::orderOfMagnitude(interval) : -2;

  int ratio = 2;
  double singleStep = pow(10., order - ratio);
  int decimals = qMax(0, -order + ratio);

  this->setDecimals(decimals);
  this->setSingleStep(singleStep);

  // The RangeWidget doesn't have to be as precise as the sliders/spinboxes.
  ratio = 1;
  singleStep = pow(10., order - ratio);
  decimals = qMax(0, -order + ratio);

  this->RangeWidget->setDecimals(decimals);
  this->RangeWidget->setSingleStep(singleStep);
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
  this->RangeWidget->setValues(min, max);
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
qMRMLVolumeWidget
::qMRMLVolumeWidget(qMRMLVolumeWidgetPrivate* ptr, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(ptr)
{
}

// --------------------------------------------------------------------------
qMRMLVolumeWidget::~qMRMLVolumeWidget() = default;

// --------------------------------------------------------------------------
void qMRMLVolumeWidget
::setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* node)
{
  Q_D(qMRMLVolumeWidget);
  if (d->VolumeDisplayNode == node)
    {
    return;
    }

  // each time the node is modified, the qt widgets are updated
  this->qvtkReconnect(d->VolumeDisplayNode, node, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRMLDisplayNode()));

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

  this->qvtkReconnect(d->VolumeNode, volumeNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRMLVolumeNode()));

  d->VolumeNode = volumeNode;
  this->updateWidgetFromMRMLVolumeNode();
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qMRMLVolumeWidget::mrmlVolumeNode()const
{
  Q_D(const qMRMLVolumeWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode* qMRMLVolumeWidget::mrmlDisplayNode()const
{
  Q_D(const qMRMLVolumeWidget);
  return d->VolumeDisplayNode;
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidget::updateWidgetFromMRMLVolumeNode()
{
  Q_D(qMRMLVolumeWidget);
  this->setEnabled(d->VolumeDisplayNode != nullptr &&
                   d->VolumeNode != nullptr);

  vtkMRMLScalarVolumeDisplayNode* newVolumeDisplayNode = d->VolumeNode ?
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      d->VolumeNode->GetVolumeDisplayNode()) : nullptr;
/*
  if (d->VolumeNode && d->VolumeNode->GetImageData())
    {
    this->updateRangeForVolumeDisplayNode(newVolumeDisplayNode);
    }
*/
  this->setMRMLVolumeDisplayNode( newVolumeDisplayNode );
}

// --------------------------------------------------------------------------
void qMRMLVolumeWidget::updateWidgetFromMRMLDisplayNode()
{
  Q_D(qMRMLVolumeWidget);
  this->setEnabled(d->VolumeDisplayNode != nullptr &&
                   d->VolumeNode != nullptr);
  if (!d->VolumeDisplayNode)
    {
    return;
    }

  double range[2];
  d->scalarRange(d->VolumeDisplayNode, range);
  if (range[0] != d->DisplayScalarRange[0] ||
      range[1] != d->DisplayScalarRange[1])
    {
    d->updateRangeForVolumeDisplayNode(d->VolumeDisplayNode);
    }
}
