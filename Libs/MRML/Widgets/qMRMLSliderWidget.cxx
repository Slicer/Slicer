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

  void setAndObserveSelectionNode();

  QString Quantity;
  vtkMRMLScene* MRMLScene;
  vtkMRMLSelectionNode* SelectionNode;
  qMRMLSliderWidget::UnitAwareProperties Flags;
};

// --------------------------------------------------------------------------
qMRMLSliderWidgetPrivate::qMRMLSliderWidgetPrivate(qMRMLSliderWidget& object)
  : q_ptr(&object)
{
  this->Quantity = "";
  this->MRMLScene = 0;
  this->SelectionNode = 0;
  this->Flags = qMRMLSliderWidget::Prefix | qMRMLSliderWidget::Suffix
    | qMRMLSliderWidget::Precision
    | qMRMLSliderWidget::MinimumValue | qMRMLSliderWidget::MaximumValue;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidgetPrivate::setAndObserveSelectionNode()
{
  Q_Q(qMRMLSliderWidget);

  vtkMRMLSelectionNode* selectionNode = 0;
  if (this->MRMLScene)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
    Q_ASSERT(selectionNode);
    }

  q->qvtkReconnect(this->SelectionNode, selectionNode,
    vtkMRMLSelectionNode::UnitModifiedEvent,
    q, SLOT(updateWidgetFromUnitNode()));
  this->SelectionNode = selectionNode;
  q->updateWidgetFromUnitNode();
}

// --------------------------------------------------------------------------
// qMRMLSliderWidget

// --------------------------------------------------------------------------
qMRMLSliderWidget::qMRMLSliderWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLSliderWidgetPrivate(*this))
{
  Q_D(qMRMLSliderWidget);
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::~qMRMLSliderWidget()
{
}

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

  this->setEnabled(scene != 0);
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
        d->SelectionNode->GetUnitNodeID(d->Quantity.toLatin1())));

    if (unitNode)
      {
      if (d->Flags.testFlag(qMRMLSliderWidget::Precision))
        {
        this->setDecimals(unitNode->GetPrecision());
        this->setSingleStep(pow(10.0, -unitNode->GetPrecision()));
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
      if (d->Flags.testFlag(qMRMLSliderWidget::MinimumValue))
        {
        this->setMaximum(unitNode->GetMaximumValue());
        }
      }
    }
}
