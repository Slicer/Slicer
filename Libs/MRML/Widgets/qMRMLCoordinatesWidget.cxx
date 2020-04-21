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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLCoordinatesWidget.h"

// CTK includes
#include <ctkLinearValueProxy.h>

// Qt includes
#include <QDebug>
#include <QHBoxLayout>

// qMRML includes
#include "qMRMLSpinBox.h"

// MRML nodes includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>

// STD
#include <cmath>

// --------------------------------------------------------------------------
class qMRMLCoordinatesWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLCoordinatesWidget);
protected:
  qMRMLCoordinatesWidget* const q_ptr;
public:
  qMRMLCoordinatesWidgetPrivate(qMRMLCoordinatesWidget& object);
  ~qMRMLCoordinatesWidgetPrivate();

  void setAndObserveSelectionNode();
  void updateValueProxy(vtkMRMLUnitNode* unitNode);

  QString Quantity;
  vtkMRMLScene* MRMLScene;
  vtkMRMLSelectionNode* SelectionNode;
  qMRMLCoordinatesWidget::UnitAwareProperties Flags;
  ctkLinearValueProxy* Proxy;
};

// --------------------------------------------------------------------------
qMRMLCoordinatesWidgetPrivate
::qMRMLCoordinatesWidgetPrivate(qMRMLCoordinatesWidget& object)
  : q_ptr(&object)
{
  this->MRMLScene = nullptr;
  this->SelectionNode = nullptr;
  this->Flags = qMRMLCoordinatesWidget::Prefix | qMRMLCoordinatesWidget::Suffix
    | qMRMLCoordinatesWidget::Precision | qMRMLCoordinatesWidget::MinimumValue
    | qMRMLCoordinatesWidget::MaximumValue;
  this->Proxy = new ctkLinearValueProxy;
}

// --------------------------------------------------------------------------
qMRMLCoordinatesWidgetPrivate::~qMRMLCoordinatesWidgetPrivate()
{
  delete this->Proxy;
}

// --------------------------------------------------------------------------
void qMRMLCoordinatesWidgetPrivate::setAndObserveSelectionNode()
{
  Q_Q(qMRMLCoordinatesWidget);

  vtkMRMLSelectionNode* selectionNode = nullptr;
  if (this->MRMLScene)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->MRMLScene->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
    }

  q->qvtkReconnect(this->SelectionNode, selectionNode,
    vtkMRMLSelectionNode::UnitModifiedEvent,
    q, SLOT(updateWidgetFromUnitNode()));
  this->SelectionNode = selectionNode;
  q->updateWidgetFromUnitNode();
}

// --------------------------------------------------------------------------
void qMRMLCoordinatesWidgetPrivate::updateValueProxy(vtkMRMLUnitNode* unitNode)
{
  Q_Q(qMRMLCoordinatesWidget);
  if (!unitNode)
    {
    q->setValueProxy(nullptr);
    this->Proxy->setCoefficient(1.0);
    this->Proxy->setOffset(0.0);
    return;
    }

  this->Proxy->setOffset(unitNode->GetDisplayOffset());
  this->Proxy->setCoefficient(unitNode->GetDisplayCoefficient());
  q->setValueProxy(this->Proxy);
}

// --------------------------------------------------------------------------
// qMRMLCoordinatesWidget

//------------------------------------------------------------------------------
qMRMLCoordinatesWidget::qMRMLCoordinatesWidget(QWidget* _parent)
: Superclass(_parent), d_ptr(new qMRMLCoordinatesWidgetPrivate(*this))
{
}

//------------------------------------------------------------------------------
qMRMLCoordinatesWidget::~qMRMLCoordinatesWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLCoordinatesWidget::setQuantity(const QString& quantity)
{
  Q_D(qMRMLCoordinatesWidget);
  if (quantity == d->Quantity)
    {
    return;
    }

  d->Quantity = quantity;
  this->updateWidgetFromUnitNode();
}

//-----------------------------------------------------------------------------
QString qMRMLCoordinatesWidget::quantity()const
{
  Q_D(const qMRMLCoordinatesWidget);
  return d->Quantity;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLCoordinatesWidget::mrmlScene()const
{
  Q_D(const qMRMLCoordinatesWidget);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLCoordinatesWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLCoordinatesWidget);

  if (this->mrmlScene() == scene)
    {
    return;
    }

  d->MRMLScene = scene;
  d->setAndObserveSelectionNode();
}

// --------------------------------------------------------------------------
qMRMLCoordinatesWidget::UnitAwareProperties
qMRMLCoordinatesWidget::unitAwareProperties()const
{
  Q_D(const qMRMLCoordinatesWidget);
  return d->Flags;
}

// --------------------------------------------------------------------------
void qMRMLCoordinatesWidget
::setUnitAwareProperties(UnitAwareProperties newFlags)
{
  Q_D(qMRMLCoordinatesWidget);
  if (newFlags == d->Flags)
    {
    return;
    }

  d->Flags = newFlags;
}

// --------------------------------------------------------------------------
void qMRMLCoordinatesWidget::updateWidgetFromUnitNode()
{
  Q_D(qMRMLCoordinatesWidget);

  if (d->SelectionNode)
    {
    vtkMRMLUnitNode* unitNode =
      vtkMRMLUnitNode::SafeDownCast(d->MRMLScene->GetNodeByID(
        d->SelectionNode->GetUnitNodeID(d->Quantity.toUtf8())));

    if (unitNode)
      {
      if (d->Flags.testFlag(qMRMLCoordinatesWidget::Scaling))
        {
        d->updateValueProxy(unitNode);
        }

      if (d->Flags.testFlag(qMRMLCoordinatesWidget::Precision))
        {
        this->setDecimals(unitNode->GetPrecision());
        this->setSingleStep(pow(10.0, -unitNode->GetPrecision()));
        }
      if (d->Flags.testFlag(qMRMLCoordinatesWidget::MinimumValue) &&
          d->Flags.testFlag(qMRMLCoordinatesWidget::MaximumValue))
        {
        this->setRange(unitNode->GetMinimumValue(), unitNode->GetMaximumValue());
        }
      else if (d->Flags.testFlag(qMRMLCoordinatesWidget::MinimumValue))
        {
        this->setMinimum(unitNode->GetMinimumValue());
        }
      else if (d->Flags.testFlag(qMRMLCoordinatesWidget::MaximumValue))
        {
        this->setMaximum(unitNode->GetMaximumValue());
        }
      for (int i = 0; i < this->dimension(); ++i)
        {
        if (d->Flags.testFlag(qMRMLCoordinatesWidget::Prefix))
          {
          this->spinBox(i)->setPrefix(unitNode->GetPrefix());
          }
        if (d->Flags.testFlag(qMRMLCoordinatesWidget::Suffix))
          {
          this->spinBox(i)->setSuffix(unitNode->GetSuffix());
          }
        }
      }
    }
}
