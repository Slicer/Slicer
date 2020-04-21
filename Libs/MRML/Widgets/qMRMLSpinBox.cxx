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

#include "qMRMLSpinBox.h"

// CTK includes
#include <ctkLinearValueProxy.h>

// Qt includes
#include <QDebug>

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>

#include <cmath>

// VTK includes
#include <vtkCommand.h>

// --------------------------------------------------------------------------
class qMRMLSpinBoxPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSpinBox);
protected:
  qMRMLSpinBox* const q_ptr;
public:
  qMRMLSpinBoxPrivate(qMRMLSpinBox& object);
  ~qMRMLSpinBoxPrivate();

  void setAndObserveSelectionNode();
  void updateValueProxy(vtkMRMLUnitNode* unitNode);

  QString Quantity;
  vtkMRMLScene* MRMLScene;
  vtkMRMLSelectionNode* SelectionNode;
  qMRMLSpinBox::UnitAwareProperties Flags;
  ctkLinearValueProxy* Proxy;
};

// --------------------------------------------------------------------------
qMRMLSpinBoxPrivate::qMRMLSpinBoxPrivate(qMRMLSpinBox& object)
  :q_ptr(&object)
{
  this->Quantity = "";
  this->MRMLScene = nullptr;
  this->SelectionNode = nullptr;
  this->Flags = qMRMLSpinBox::Prefix | qMRMLSpinBox::Suffix
    | qMRMLSpinBox::Precision
    | qMRMLSpinBox::MinimumValue | qMRMLSpinBox::MaximumValue;
  this->Proxy = new ctkLinearValueProxy();
}

// --------------------------------------------------------------------------
qMRMLSpinBoxPrivate::~qMRMLSpinBoxPrivate()
{
  delete this->Proxy;
}

// --------------------------------------------------------------------------
void qMRMLSpinBoxPrivate::setAndObserveSelectionNode()
{
  Q_Q(qMRMLSpinBox);

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
void qMRMLSpinBoxPrivate::updateValueProxy(vtkMRMLUnitNode* unitNode)
{
  Q_Q(qMRMLSpinBox);
  if (!unitNode)
    {
    q->setValueProxy(nullptr);
    this->Proxy->setCoefficient(1.0);
    this->Proxy->setOffset(0.0);
    return;
    }

  this->Proxy->setOffset(unitNode->GetDisplayOffset());
  q->setValueProxy(this->Proxy);
  this->Proxy->setCoefficient(unitNode->GetDisplayCoefficient());
}

// --------------------------------------------------------------------------
// qMRMLSpinBox

// --------------------------------------------------------------------------
qMRMLSpinBox::qMRMLSpinBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLSpinBoxPrivate(*this))
{
}

// --------------------------------------------------------------------------
qMRMLSpinBox::~qMRMLSpinBox() = default;

//-----------------------------------------------------------------------------
void qMRMLSpinBox::setQuantity(const QString& quantity)
{
  Q_D(qMRMLSpinBox);
  if (quantity == d->Quantity)
    {
    return;
    }

  d->Quantity = quantity;
  this->updateWidgetFromUnitNode();
}

//-----------------------------------------------------------------------------
QString qMRMLSpinBox::quantity()const
{
  Q_D(const qMRMLSpinBox);
  return d->Quantity;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLSpinBox::mrmlScene()const
{
  Q_D(const qMRMLSpinBox);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLSpinBox::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSpinBox);

  if (this->mrmlScene() == scene)
    {
    return;
    }

  d->MRMLScene = scene;
  d->setAndObserveSelectionNode();

  this->setEnabled(scene != nullptr);
}

// --------------------------------------------------------------------------
qMRMLSpinBox::UnitAwareProperties qMRMLSpinBox::unitAwareProperties()const
{
  Q_D(const qMRMLSpinBox);
  return d->Flags;
}

// --------------------------------------------------------------------------
void qMRMLSpinBox::setUnitAwareProperties(UnitAwareProperties newFlags)
{
  Q_D(qMRMLSpinBox);
  if (newFlags == d->Flags)
    {
    return;
    }

  d->Flags = newFlags;
}

// --------------------------------------------------------------------------
void qMRMLSpinBox::updateWidgetFromUnitNode()
{
  Q_D(qMRMLSpinBox);

  if (d->SelectionNode)
    {
    vtkMRMLUnitNode* unitNode =
      vtkMRMLUnitNode::SafeDownCast(d->MRMLScene->GetNodeByID(
        d->SelectionNode->GetUnitNodeID(d->Quantity.toUtf8())));

    if (unitNode)
      {
      if (d->Flags.testFlag(qMRMLSpinBox::Precision))
        {
        this->setDecimals(unitNode->GetPrecision());
        this->setSingleStep(pow(10.0, -unitNode->GetPrecision()));
        }
      if (d->Flags.testFlag(qMRMLSpinBox::Prefix))
        {
        this->setPrefix(unitNode->GetPrefix());
        }
      if (d->Flags.testFlag(qMRMLSpinBox::Suffix))
        {
        this->setSuffix(unitNode->GetSuffix());
        }
      if (d->Flags.testFlag(qMRMLSpinBox::MinimumValue))
        {
        this->setMinimum(unitNode->GetMinimumValue());
        }
      if (d->Flags.testFlag(qMRMLSpinBox::MaximumValue))
        {
        this->setMaximum(unitNode->GetMaximumValue());
        }
      if (d->Flags.testFlag(qMRMLSpinBox::Scaling))
        {
        d->updateValueProxy(unitNode);
        }
      }
    }
}
