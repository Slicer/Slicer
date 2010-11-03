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
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVector>

// qMRML includes
#include "qMRMLSceneDisplayableModel.h"
#include "qMRMLSceneModel_p.h"
//#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayableHierarchyNode.h>

// VTK includes
#include <vtkVariantArray.h>
#include <typeinfo>


//------------------------------------------------------------------------------
class qMRMLSceneDisplayableModelPrivate: public qMRMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneDisplayableModel);
public:
  qMRMLSceneDisplayableModelPrivate(qMRMLSceneDisplayableModel& object);

};

//------------------------------------------------------------------------------
qMRMLSceneDisplayableModelPrivate
::qMRMLSceneDisplayableModelPrivate(qMRMLSceneDisplayableModel& object)
  : qMRMLSceneModelPrivate(object)
{

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneDisplayableModel::qMRMLSceneDisplayableModel(QObject *vparent)
  :qMRMLSceneModel(new qMRMLSceneDisplayableModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneDisplayableModel::~qMRMLSceneDisplayableModel()
{
}

/*

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneDisplayableModel::childNode(vtkMRMLNode* node, int childIndex)
{
  // shortcut the following search if we are sure that the node has no children
  if (childIndex < 0 || node == 0 || !qMRMLSceneDisplayableModel::canBeAParent(node))
    {
    return 0;
    }
  // MRML Transformable nodes
  QString nodeId = QString(node->GetID());
  vtkCollection* sceneCollection = node->GetScene()->GetCurrentScene();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (n = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLNode* parent = qMRMLSceneDisplayableModel::parentNode(n);
    if (parent == 0)
      {
      continue;
      }
    // compare IDs not pointers
    if (nodeId == parent->GetID())
      {
      if (childIndex-- == 0)
        {
        return n;
        }
      }
    }
  return 0;
}
*/
//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneDisplayableModel::parentNode(vtkMRMLNode* node)
{
  // MRML Displayable nodes (inherits from transformable)
  vtkMRMLDisplayableNode *displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (displayableNode &&
      displayableNode->GetScene() &&
      displayableNode->GetID())
    {
    // get the displayable hierarchy node associated with this displayable node
    vtkMRMLDisplayableHierarchyNode * displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(displayableNode->GetScene(), displayableNode->GetID());
    if (displayableHierarchyNode)
      {
      // return it's parent
      return displayableHierarchyNode->GetParentNode();
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLSceneDisplayableModel::nodeIndex(vtkMRMLNode* node)
{
  const char* nodeId = node ? node->GetID() : 0;
  if (nodeId == 0)
    {
    return -1;
    }
  const char* nId = 0;
  int index = -1;
  vtkMRMLNode* parent = qMRMLSceneDisplayableModel::parentNode(node);
  vtkCollection* sceneCollection = node->GetScene()->GetCurrentScene();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (n = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be NULL, it means that the scene is the parent
    if (parent == qMRMLSceneDisplayableModel::parentNode(n))
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
bool qMRMLSceneDisplayableModel::canBeAChild(vtkMRMLNode* node)
{
  if (!node)
    {
    return false;
    }
  if (node->IsA("vtkMRMLDisplayableNode"))
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneDisplayableModel::canBeAParent(vtkMRMLNode* node)
{
  if (!node)
    {
    return false;
    }
  if (node->IsA("vtkMRMLDisplayableNode"))
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneDisplayableModel::reparent(vtkMRMLNode* node, vtkMRMLNode* vtkNotUsed(newParent))
{
  Q_ASSERT(node);
  if (!node)
    {
    return false;
    }

  // MRML Displayable nodes (inherits from transformable)
  vtkMRMLDisplayableNode *displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (displayableNode)
    {
    vtkMRMLDisplayableNode *displayableParentNode = vtkMRMLDisplayableNode::SafeDownCast(node);
    if (displayableParentNode)
      {
      // get the hierarchy nodes associated with each node
      if (displayableNode->GetScene() &&
          displayableNode->GetID() &&
          displayableParentNode->GetScene() &&
          displayableParentNode->GetID())
        {
        vtkMRMLDisplayableHierarchyNode * displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(displayableNode->GetScene(), displayableNode->GetID());
        vtkMRMLDisplayableHierarchyNode * displayableHierarchyParentNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(displayableParentNode->GetScene(), displayableParentNode->GetID());
        if (displayableHierarchyNode)
          {
          if (displayableHierarchyParentNode)
            {
            displayableHierarchyNode->SetParentNodeID(displayableHierarchyParentNode->GetID());
            }
          else
            {
            // reparenting to top with null parent id
            displayableHierarchyNode->SetParentNodeID(NULL);
            }
          return true;
          }
        }
      }
    else
      {
      std::cout << "reparent: can only reparent to another displayable node\n";
      }
    }  
  return false;
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModel::populateScene()
{
  Q_D(qMRMLSceneDisplayableModel);
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
void qMRMLSceneDisplayableModel::insertNode(vtkMRMLNode* node)
{
  vtkMRMLNode* parentNode = qMRMLSceneDisplayableModel::parentNode(node);
  QStandardItem* parentItem =
    parentNode ? this->itemFromNode(parentNode) : this->mrmlSceneItem();
  Q_ASSERT(parentItem);
  int min = this->preItems(parentItem).count();
  int max = parentItem->rowCount() - this->postItems(parentItem).count();
  this->insertNode(node, parentItem, qMin(min + qMRMLSceneDisplayableModel::nodeIndex(node), max));
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModel::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  this->qMRMLSceneModel::updateItemFromNode(item, node, column);
  bool oldBlock = this->blockSignals(true);
  if (qMRMLSceneDisplayableModel::canBeAChild(node))
    {
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    }
  if (qMRMLSceneDisplayableModel::canBeAParent(node))
    {
    item->setFlags(item->flags() | Qt::ItemIsDropEnabled);
    }
  this->blockSignals(oldBlock);
  QStandardItem* parentItem = item->parent();
  QStandardItem* newParentItem = this->itemFromNode(qMRMLSceneDisplayableModel::parentNode(node));
  if (newParentItem == 0)
    {
    newParentItem = this->mrmlSceneItem();
    }
  // if the item has no parent, then it means it hasn't been put into the scene yet.
  // and it will do it automatically.
  if (parentItem != 0 && (parentItem != newParentItem || qMRMLSceneDisplayableModel::nodeIndex(node) != item->row()))
    {
    QList<QStandardItem*> children = parentItem->takeRow(item->row());
    int min = this->preItems(newParentItem).count();
    int max = newParentItem->rowCount() - this->postItems(newParentItem).count();
    int pos = qMin(min + qMRMLSceneDisplayableModel::nodeIndex(node), max);
    newParentItem->insertRow(pos, children);
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModel::updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item)
{
  this->qMRMLSceneModel::updateNodeFromItem(node, item);
  Q_ASSERT(node != this->mrmlNodeFromItem(item->parent()));

  // Don't do the following if the row is not complete (reparenting an
  // incomplete row might lead to errors). updateNodeFromItem is typically
  // called for every item changed, so it should be
  QStandardItem* parentItem = item->parent();
  for (int i = 0; i < parentItem->columnCount(); ++i)
    {
    if (parentItem->child(item->row(), i) == 0)
      {
      return;
      }
    }
  vtkMRMLNode* parent = this->mrmlNodeFromItem(parentItem);
  if (qMRMLSceneDisplayableModel::parentNode(node) != parent)
    {
    qMRMLSceneDisplayableModel::reparent(node, parent);
    }
  else if (qMRMLSceneDisplayableModel::nodeIndex(node) != item->row())
    {
    this->updateItemFromNode(item, node, item->column());
    }
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneDisplayableModel::supportedDropActions()const
{
  return Qt::MoveAction;
}
