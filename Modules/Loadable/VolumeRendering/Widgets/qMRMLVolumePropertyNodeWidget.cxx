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

// QT includes

// qMRML includes
#include "qMRMLVolumePropertyNodeWidget.h"
#include "ui_qMRMLVolumePropertyNodeWidget.h"

// MRML includes
#include <vtkMRMLVolumePropertyNode.h>

// VTK includes
#include <vtkVolumeProperty.h>

//-----------------------------------------------------------------------------
class qMRMLVolumePropertyNodeWidgetPrivate
  : public Ui_qMRMLVolumePropertyNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLVolumePropertyNodeWidget);

protected:
  qMRMLVolumePropertyNodeWidget* const q_ptr;

public:
  qMRMLVolumePropertyNodeWidgetPrivate(qMRMLVolumePropertyNodeWidget& object);
  virtual ~qMRMLVolumePropertyNodeWidgetPrivate();

  virtual void setupUi();

  vtkMRMLVolumePropertyNode*                   VolumePropertyNode;
};

// --------------------------------------------------------------------------
qMRMLVolumePropertyNodeWidgetPrivate::qMRMLVolumePropertyNodeWidgetPrivate(
  qMRMLVolumePropertyNodeWidget& object)
  : q_ptr(&object)
{
  this->VolumePropertyNode = 0;
}

// --------------------------------------------------------------------------
qMRMLVolumePropertyNodeWidgetPrivate::~qMRMLVolumePropertyNodeWidgetPrivate()
{
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidgetPrivate::setupUi()
{
  Q_Q(qMRMLVolumePropertyNodeWidget);
  this->Ui_qMRMLVolumePropertyNodeWidget::setupUi(q);
  QObject::connect(this->VolumeProperty, SIGNAL(chartsExtentChanged()),
                   q, SIGNAL(chartsExtentChanged()));
  QObject::connect(this->VolumeProperty, SIGNAL(thresholdEnabledChanged(bool)),
                   q, SIGNAL(thresholdChanged(bool)));
}

// --------------------------------------------------------------------------
// qMRMLVolumePropertyNodeWidget
// --------------------------------------------------------------------------
qMRMLVolumePropertyNodeWidget::qMRMLVolumePropertyNodeWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLVolumePropertyNodeWidgetPrivate(*this))
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  d->setupUi();
}

// --------------------------------------------------------------------------
qMRMLVolumePropertyNodeWidget::~qMRMLVolumePropertyNodeWidget()
{
}

// --------------------------------------------------------------------------
vtkVolumeProperty* qMRMLVolumePropertyNodeWidget::volumeProperty()const
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  return d->VolumeProperty->volumeProperty();
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::setMRMLVolumePropertyNode(
  vtkMRMLNode* volumePropertyNode)
{
  this->setMRMLVolumePropertyNode(
    vtkMRMLVolumePropertyNode::SafeDownCast(volumePropertyNode));
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::setMRMLVolumePropertyNode(
  vtkMRMLVolumePropertyNode* volumePropertyNode)
{
   Q_D(qMRMLVolumePropertyNodeWidget);
   this->qvtkReconnect(d->VolumePropertyNode, volumePropertyNode,
                        vtkCommand::ModifiedEvent,
                        this, SLOT(updateFromVolumePropertyNode()));
   d->VolumePropertyNode = volumePropertyNode;
   this->updateFromVolumePropertyNode();
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::updateFromVolumePropertyNode()
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  vtkVolumeProperty* newVolumeProperty =
    d->VolumePropertyNode ? d->VolumePropertyNode->GetVolumeProperty() : 0;
  qvtkReconnect(d->VolumeProperty->volumeProperty(), newVolumeProperty,
                vtkCommand::ModifiedEvent, this, SIGNAL(volumePropertyChanged()));
  d->VolumeProperty->setVolumeProperty(newVolumeProperty);
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::chartsBounds(double bounds[4])const
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  d->VolumeProperty->chartsBounds(bounds);
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::chartsExtent(double extent[4])const
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  d->VolumeProperty->chartsExtent(extent);
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::setThreshold(bool enable)
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  d->VolumeProperty->setThresholdEnabled(enable);
}

// --------------------------------------------------------------------------
bool qMRMLVolumePropertyNodeWidget::hasThreshold()const
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  return d->VolumeProperty->isThresholdEnabled();
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget
::moveAllPoints(double x, double y, bool dontMoveFirstAndLast)
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  return d->VolumeProperty->moveAllPoints(x, y, dontMoveFirstAndLast);
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget
::spreadAllPoints(double factor, bool dontSpreadFirstAndLast)
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  return d->VolumeProperty->spreadAllPoints(factor, dontSpreadFirstAndLast);
}
