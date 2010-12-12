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

// qMRML includes
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSceneModel_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLTransformableNode.h>

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneTransformModel::parentNode(vtkMRMLNode* node)
{
  // MRML Transformable nodes
  vtkMRMLTransformableNode* transformableNode =
    vtkMRMLTransformableNode::SafeDownCast(node);
  if (transformableNode)
    {
    return transformableNode->GetParentTransformNode();
    }
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLSceneTransformModel::nodeIndex(vtkMRMLNode* node)
{
  const char* nodeId = node ? node->GetID() : 0;
  if (nodeId == 0)
    {
    return -1;
    }
  const char* nId = 0;
  int index = -1;
  vtkMRMLNode* parent = qMRMLSceneTransformModel::parentNode(node);
  vtkCollection* sceneCollection = node->GetScene()->GetCurrentScene();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (n = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be NULL, it means that the scene is the parent
    if (parent == qMRMLSceneTransformModel::parentNode(n))
      {
      ++index;
      nId = n->GetID();
      if (nId && !strcmp(nodeId, nId))
        {
        return index;
        }
      }
    }
  return -1;
}

//------------------------------------------------------------------------------
bool qMRMLSceneTransformModel::canBeAChild(vtkMRMLNode* node)
{
  return node ? node->IsA("vtkMRMLTransformableNode") : false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneTransformModel::canBeAParent(vtkMRMLNode* node)
{
  return node ? node->IsA("vtkMRMLTransformNode") : false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneTransformModel::reparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
  Q_ASSERT(node);
  if (!node || qMRMLSceneTransformModel::parentNode(node) == newParent)
    {
    return false;
    }
  Q_ASSERT(newParent != node);
  // MRML Transformable Nodes
  vtkMRMLTransformableNode* transformableNode =
    vtkMRMLTransformableNode::SafeDownCast(node);
  if (transformableNode)
    {
    transformableNode->SetAndObserveTransformNodeID( newParent ? newParent->GetID() : 0 );
    transformableNode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
class qMRMLSceneTransformModelPrivate: public qMRMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneTransformModel);
public:
  qMRMLSceneTransformModelPrivate(qMRMLSceneTransformModel& object);

};

//------------------------------------------------------------------------------
qMRMLSceneTransformModelPrivate
::qMRMLSceneTransformModelPrivate(qMRMLSceneTransformModel& object)
  : qMRMLSceneModelPrivate(object)
{

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneTransformModel::qMRMLSceneTransformModel(QObject *vparent)
  :qMRMLSceneModel(new qMRMLSceneTransformModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneTransformModel::~qMRMLSceneTransformModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneTransformModel::populateScene()
{
  Q_D(qMRMLSceneTransformModel);
  Q_ASSERT(d->MRMLScene);
  // Add nodes
  vtkMRMLNode *node = 0;
  vtkCollectionSimpleIterator it;
  for (d->MRMLScene->GetCurrentScene()->InitTraversal(it);
       (node = (vtkMRMLNode*)d->MRMLScene->GetCurrentScene()->GetNextItemAsObject(it)) ;)
    {
    // WARNING: works only if the nodes are in the scene in the correct order:
    // parents are before children
    this->insertNode(node);
    }
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneTransformModel::insertNode(vtkMRMLNode* node)
{
  QStandardItem* nodeItem = this->itemFromNode(node);
  if (nodeItem)
    {
    // the item is already into the scene, don't add it again
    return nodeItem;
    }
  vtkMRMLNode* parentNode = qMRMLSceneTransformModel::parentNode(node);
  QStandardItem* parentItem =
    parentNode ? this->itemFromNode(parentNode) : this->mrmlSceneItem();
  if (!parentItem)
    {
    Q_ASSERT(parentNode);
    parentItem = this->insertNode(parentNode);
    }
  int min = this->preItems(parentItem).count();
  int max = parentItem->rowCount() - this->postItems(parentItem).count();
  return this->insertNode(node, parentItem, qMin(min + qMRMLSceneTransformModel::nodeIndex(node), max));
}

//------------------------------------------------------------------------------
void qMRMLSceneTransformModel::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  this->qMRMLSceneModel::updateItemFromNode(item, node, column);
  QStandardItem* parentItem = item->parent();
  QStandardItem* newParentItem = this->itemFromNode(qMRMLSceneTransformModel::parentNode(node));
  if (newParentItem == 0)
    {
    newParentItem = this->mrmlSceneItem();
    }
  // if the item has no parent, then it means it hasn't been put into the scene yet.
  // and it will do it automatically.
  if (parentItem != 0 && (parentItem != newParentItem || qMRMLSceneTransformModel::nodeIndex(node) != item->row()))
    {
    QList<QStandardItem*> children = parentItem->takeRow(item->row());
    int min = this->preItems(newParentItem).count();
    int max = newParentItem->rowCount() - this->postItems(newParentItem).count();
    int pos = qMin(min + qMRMLSceneTransformModel::nodeIndex(node), max);
    newParentItem->insertRow(pos, children);
    printStandardItem(this->invisibleRootItem(), "  ");
    }
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneTransformModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  Q_D(const qMRMLSceneTransformModel);
  Q_UNUSED(column);
  QFlags<Qt::ItemFlag> flags = this->qMRMLSceneModel::nodeFlags(node, column);
  if (qMRMLSceneTransformModel::canBeAChild(node))
    {
    flags = flags | Qt::ItemIsDragEnabled;
    }
  if (qMRMLSceneTransformModel::canBeAParent(node))
    {
    flags = flags | Qt::ItemIsDropEnabled;
    }
  return flags;
}

//------------------------------------------------------------------------------
void qMRMLSceneTransformModel::updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item)
{
  Q_D(qMRMLSceneTransformModel);
  this->qMRMLSceneModel::updateNodeFromItem(node, item);
  Q_ASSERT(node != this->mrmlNodeFromItem(item->parent()));
  
  QStandardItem* parentItem = item->parent();
  
  // Don't do the following if the row is not complete (reparenting an
  // incomplete row might lead to errors). (if there is no child yet for a given
  // column, it will get there next time updateNodeFromItem is called).
  // updateNodeFromItem() is called for every visible item drag&dropped 
  // (not called for the items in a hidden column)
  
  // warning, it works only if it's 1 line dnd at a time
  /*
  foreach(const QModelIndex& draggedIndex, d->LastMimeData)
    {
    if (parentItem->child(item->row(), draggedIndex.column()) == 0)
      {
      return;
      }
    }
  */
  for (int i = 0; i < parentItem->columnCount(); ++i)
    {
    if (parentItem->child(item->row(), i) == 0)
      {
      return;
      }
    }

  vtkMRMLNode* parent = this->mrmlNodeFromItem(parentItem);
  if (qMRMLSceneTransformModel::parentNode(node) != parent)
    {
    qMRMLSceneTransformModel::reparent(node, parent);
    }
  else if (qMRMLSceneTransformModel::nodeIndex(node) != item->row())
    {
    this->updateItemFromNode(item, node, item->column());
    }
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneTransformModel::supportedDropActions()const
{
  return Qt::MoveAction;
}
