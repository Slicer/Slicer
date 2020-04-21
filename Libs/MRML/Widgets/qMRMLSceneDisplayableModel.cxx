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

// qMRML includes
#include "qMRMLSceneDisplayableModel_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayableHierarchyNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLSelectionNode.h>

//------------------------------------------------------------------------------
qMRMLSceneDisplayableModelPrivate
::qMRMLSceneDisplayableModelPrivate(qMRMLSceneDisplayableModel& object)
  : qMRMLSceneHierarchyModelPrivate(object)
{
  this->OpacityColumn = -1;
  this->ColorColumn = -1;
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModelPrivate::init()
{
  Q_Q(qMRMLSceneDisplayableModel);
  q->setVisibilityColumn(q->nameColumn());
}

//------------------------------------------------------------------------------
vtkMRMLHierarchyNode* qMRMLSceneDisplayableModelPrivate::CreateHierarchyNode()const
{
  return vtkMRMLDisplayableHierarchyNode::New();
}

//------------------------------------------------------------------------------
vtkMRMLDisplayNode* qMRMLSceneDisplayableModelPrivate
::displayNode(vtkMRMLNode* node)const
{
  if (vtkMRMLDisplayNode::SafeDownCast(node))
    {
    return vtkMRMLDisplayNode::SafeDownCast(node);
    }

  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (displayableNode)
    {
    return displayableNode->GetDisplayNode();
    }

  vtkMRMLDisplayableHierarchyNode* displayableHierarchyNode
      = vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
  if (displayableHierarchyNode)
    {
    return displayableHierarchyNode->GetDisplayNode();
    }
  return nullptr;
}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneDisplayableModel::qMRMLSceneDisplayableModel(QObject *vparent)
  :Superclass(new qMRMLSceneDisplayableModelPrivate(*this), vparent)
{
  Q_D(qMRMLSceneDisplayableModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLSceneDisplayableModel::qMRMLSceneDisplayableModel(
  qMRMLSceneDisplayableModelPrivate* pimpl, QObject *vparent)
  :Superclass(pimpl, vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneDisplayableModel::~qMRMLSceneDisplayableModel() = default;

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneDisplayableModel::parentNode(vtkMRMLNode* node)const
{
  return vtkMRMLDisplayableHierarchyNode::SafeDownCast(
    this->Superclass::parentNode(node));
}

//------------------------------------------------------------------------------
bool qMRMLSceneDisplayableModel::canBeAChild(vtkMRMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  return node->IsA("vtkMRMLDisplayableNode") ||
         node->IsA("vtkMRMLDisplayableHierarchyNode");
}

//------------------------------------------------------------------------------
bool qMRMLSceneDisplayableModel::canBeAParent(vtkMRMLNode* node)const
{
  return this->canBeAChild(node);
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModel::observeNode(vtkMRMLNode* node)
{
  this->Superclass::observeNode(node);
  if (node->IsA("vtkMRMLDisplayableNode"))
    {
    qvtkConnect(node, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));
    }
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneDisplayableModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  Q_D(const qMRMLSceneDisplayableModel);
  QFlags<Qt::ItemFlag> flags = this->Superclass::nodeFlags(node, column);
  vtkMRMLNode *displayNode = d->displayNode(node);
  if (column == this->visibilityColumn() &&
      displayNode != nullptr)
    {
    flags |= Qt::ItemIsEditable;
    }
  if (column == this->colorColumn() &&
      displayNode != nullptr)
    {
    flags |= Qt::ItemIsEditable;
    }
  if (column == this->opacityColumn() &&
      displayNode != nullptr)
    {
    flags |= Qt::ItemIsEditable;
    }
  return flags;
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModel
::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_D(qMRMLSceneDisplayableModel);
  vtkMRMLDisplayNode* displayNode = d->displayNode(node);
  if (column == this->colorColumn())
    {
    if (displayNode)
      {
      double* rgbF = displayNode->GetColor();
      QColor color = QColor::fromRgbF(rgbF[0], rgbF[1], rgbF[2],
                                      displayNode->GetOpacity());
      item->setData(color, Qt::DecorationRole);
      item->setToolTip("Color");
      }
    }
  if (column == this->opacityColumn())
    {
    if (displayNode)
      {
      QString displayedOpacity
        = QString::number(displayNode->GetOpacity(), 'f', 2);
      item->setData(displayedOpacity, Qt::DisplayRole);
      item->setToolTip("Opacity");
      }
    }
  this->Superclass::updateItemDataFromNode(item, node, column);
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModel
::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
  Q_D(qMRMLSceneDisplayableModel);
  if (item->column() == this->colorColumn())
    {
    QColor color = item->data(Qt::DecorationRole).value<QColor>();
    // Invalid color can happen when the item hasn't been initialized yet
    if (color.isValid())
      {
      vtkMRMLDisplayNode* displayNode = d->displayNode(node);
      if (displayNode)
        {
        int wasModifying = displayNode->StartModify();
        // QColor looses precision, don't change color/opacity if not "really"
        // changed.
        QColor oldColor = QColor::fromRgbF(displayNode->GetColor()[0],
                                           displayNode->GetColor()[1],
                                           displayNode->GetColor()[2],
                                           displayNode->GetOpacity());
        if (oldColor != color)
          {
          displayNode->SetColor(color.redF(), color.greenF(), color.blueF());
          displayNode->SetOpacity(color.alphaF());
          }
        displayNode->EndModify(wasModifying);
        }
      }
    }
  if (item->column() == this->opacityColumn())
    {
    QString displayedOpacity = item->data(Qt::EditRole).toString();
    if (!displayedOpacity.isEmpty())
      {
      vtkMRMLDisplayNode* displayNode = d->displayNode(node);
      // Invalid color can happen when the item hasn't been initialized yet
      if (displayNode)
        {
        QString currentOpacity = QString::number( displayNode->GetOpacity(), 'f', 2);
        if (displayedOpacity != currentOpacity)
          {
          displayNode->SetOpacity(displayedOpacity.toDouble());
          }
        }
      }
    }
  return this->Superclass::updateNodeFromItemData(node, item);
}

//------------------------------------------------------------------------------
int qMRMLSceneDisplayableModel::colorColumn()const
{
  Q_D(const qMRMLSceneDisplayableModel);
  return d->ColorColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModel::setColorColumn(int column)
{
  Q_D(qMRMLSceneDisplayableModel);
  d->ColorColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneDisplayableModel::opacityColumn()const
{
  Q_D(const qMRMLSceneDisplayableModel);
  return d->OpacityColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModel::setOpacityColumn(int column)
{
  Q_D(qMRMLSceneDisplayableModel);
  d->OpacityColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneDisplayableModel::maxColumnId()const
{
  Q_D(const qMRMLSceneDisplayableModel);
  int maxId = this->Superclass::maxColumnId();
  maxId = qMax(maxId, d->ColorColumn);
  maxId = qMax(maxId, d->OpacityColumn);
  return maxId;
}
