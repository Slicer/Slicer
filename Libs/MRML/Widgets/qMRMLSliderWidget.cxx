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

#include "qMRMLSliderWidget.h"

// CTK includes
#include <ctkLinearValueProxy.h>
#include <ctkUtils.h>

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>

// STD includes
#include <cmath>

// VTK includes
#include <vtkCommand.h>

// --------------------------------------------------------------------------
class qMRMLSliderWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSliderWidget);
protected:
  qMRMLSliderWidget* const q_ptr;
public:
  qMRMLSliderWidgetPrivate(qMRMLSliderWidget& object);
  ~qMRMLSliderWidgetPrivate();

  void setAndObserveSelectionNode();
  void updateValueProxy(vtkMRMLUnitNode* unitNode);

  QString Quantity;
  vtkMRMLScene* MRMLScene;
  vtkMRMLSelectionNode* SelectionNode;
  qMRMLSliderWidget::UnitAwareProperties Flags;
  ctkLinearValueProxy* Proxy;
};

// --------------------------------------------------------------------------
qMRMLSliderWidgetPrivate::qMRMLSliderWidgetPrivate(qMRMLSliderWidget& object)
  : q_ptr(&object)
{
  this->Quantity = "";
  this->MRMLScene = nullptr;
  this->SelectionNode = nullptr;
  this->Flags = qMRMLSliderWidget::Prefix | qMRMLSliderWidget::Suffix
    | qMRMLSliderWidget::Precision | qMRMLSliderWidget::Scaling;
  this->Proxy = new ctkLinearValueProxy;
}

// --------------------------------------------------------------------------
qMRMLSliderWidgetPrivate::~qMRMLSliderWidgetPrivate()
{
  delete this->Proxy;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidgetPrivate::setAndObserveSelectionNode()
{
  Q_Q(qMRMLSliderWidget);

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
void qMRMLSliderWidgetPrivate::updateValueProxy(vtkMRMLUnitNode* unitNode)
{
  Q_Q(qMRMLSliderWidget);
  if (!unitNode)
    {
    q->setValueProxy(nullptr);
    this->Proxy->setCoefficient(1.0);
    this->Proxy->setOffset(0.0);
    return;
    }

  q->setValueProxy(this->Proxy);
  this->Proxy->setOffset(unitNode->GetDisplayOffset());
  this->Proxy->setCoefficient(unitNode->GetDisplayCoefficient());
}

// --------------------------------------------------------------------------
// qMRMLSliderWidget

// --------------------------------------------------------------------------
qMRMLSliderWidget::qMRMLSliderWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLSliderWidgetPrivate(*this))
{
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::~qMRMLSliderWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLSliderWidget::setQuantity(const QString& quantity)
{
  Q_D(qMRMLSliderWidget);
  if (quantity == d->Quantity)
    {
    return;
    }

  d->Quantity = quantity;
  this->updateWidgetFromUnitNode();
}

//-----------------------------------------------------------------------------
QString qMRMLSliderWidget::quantity()const
{
  Q_D(const qMRMLSliderWidget);
  return d->Quantity;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLSliderWidget::mrmlScene()const
{
  Q_D(const qMRMLSliderWidget);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSliderWidget);

  if (this->mrmlScene() == scene)
    {
    return;
    }

  d->MRMLScene = scene;
  d->setAndObserveSelectionNode();

  this->setEnabled(scene != nullptr);
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::UnitAwareProperties
qMRMLSliderWidget::unitAwareProperties()const
{
  Q_D(const qMRMLSliderWidget);
  return d->Flags;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setUnitAwareProperties(UnitAwareProperties newFlags)
{
  Q_D(qMRMLSliderWidget);
  if (newFlags == d->Flags)
    {
    return;
    }

  d->Flags = newFlags;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::updateWidgetFromUnitNode()
{
  Q_D(qMRMLSliderWidget);

  if (d->SelectionNode)
    {
    vtkMRMLUnitNode* unitNode =
      vtkMRMLUnitNode::SafeDownCast(d->MRMLScene->GetNodeByID(
        d->SelectionNode->GetUnitNodeID(d->Quantity.toUtf8())));

    if (unitNode)
      {
      if (d->Flags.testFlag(qMRMLSliderWidget::Precision))
        {
        // setDecimals overwrites values therefore it is important
        // to call it only when it is necessary (without this check,
        // for example a setValue call may be ineffective if the min/max
        // value is changing at the same time)
        if (this->decimals()!=unitNode->GetPrecision())
          {
          this->setDecimals(unitNode->GetPrecision());
          }
        }
      if (d->Flags.testFlag(qMRMLSliderWidget::Prefix))
        {
        this->setPrefix(unitNode->GetPrefix());
        }
      if (d->Flags.testFlag(qMRMLSliderWidget::Suffix))
        {
        this->setSuffix(unitNode->GetSuffix());
        }
      if (d->Flags.testFlag(qMRMLSliderWidget::MinimumValue))
        {
        this->setMinimum(unitNode->GetMinimumValue());
        }
      if (d->Flags.testFlag(qMRMLSliderWidget::MaximumValue))
        {
        this->setMaximum(unitNode->GetMaximumValue());
        }
      if (d->Flags.testFlag(qMRMLSliderWidget::Scaling))
        {
        d->updateValueProxy(unitNode);
        }
      if (d->Flags.testFlag(qMRMLSliderWidget::Precision))
        {
        double range = this->maximum() - this->minimum();
        if (d->Flags.testFlag(qMRMLSliderWidget::Scaling))
          {
          range = unitNode->GetDisplayValueFromValue(this->maximum()) -
                  unitNode->GetDisplayValueFromValue(this->minimum());
          }
        double powerOfTen = ctk::closestPowerOfTen(range);
        if (powerOfTen != 0.)
          {
          this->setSingleStep(powerOfTen / 100);
          }
        }
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setMinimum(double newMinimumValue)
{
  this->Superclass::setMinimum(newMinimumValue);
  if (this->unitAwareProperties().testFlag(qMRMLSliderWidget::Precision))
    {
    this->updateWidgetFromUnitNode();
    }
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setMaximum(double newMaximumValue)
{
  this->Superclass::setMaximum(newMaximumValue);
  if (this->unitAwareProperties().testFlag(qMRMLSliderWidget::Precision))
    {
    this->updateWidgetFromUnitNode();
    }
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setRange(double newMinimumValue, double newMaximumValue)
{
  this->Superclass::setRange(newMinimumValue, newMaximumValue);
  if (this->unitAwareProperties().testFlag(qMRMLSliderWidget::Precision))
    {
    this->updateWidgetFromUnitNode();
    }
}
