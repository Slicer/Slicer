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
vtkMRMLNode* qMRMLSceneDisplayableModel::parentNode(vtkMRMLNode* node)const
{
  if (node == NULL)
    {
    return 0;
    }
  
  // MRML Displayable nodes (inherits from transformable)
  vtkMRMLDisplayableNode *displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  vtkMRMLDisplayableHierarchyNode * displayableHierarchyNode = NULL; 
  if (displayableNode &&
      displayableNode->GetScene() &&
      displayableNode->GetID())
    {
    // get the displayable hierarchy node associated with this displayable node
    displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(displayableNode->GetScene(), displayableNode->GetID());
    if (displayableHierarchyNode)
      {
      if (displayableHierarchyNode->GetHideFromEditors())
        {
        // this is a hidden hierarchy node, so we do not want to display it
        // instead, we will return the parent of the hidden hierarchy node
        // to be used as the parent for the displayableNode
        return displayableHierarchyNode->GetParentNode();
        }
      return displayableHierarchyNode;
      }
    }
  if (displayableHierarchyNode == NULL)
    {
    // the passed in node might have been a hierarchy node instead, try to
    // cast it
    displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
    }
  if (displayableHierarchyNode)
    {
    // return it's parent
    return displayableHierarchyNode->GetParentNode();
    }
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLSceneDisplayableModel::nodeIndex(vtkMRMLNode* node)const
{
  const char* nodeId = node ? node->GetID() : 0;
  if (nodeId == 0)
    {
    return -1;
    }
  const char* nId = 0;
  int index = -1;
  vtkMRMLNode* parent = this->parentNode(node);
  vtkCollection* sceneCollection = node->GetScene()->GetCurrentScene();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (n = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be NULL, it means that the scene is the parent
    if (parent == this->parentNode(n))
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
bool qMRMLSceneDisplayableModel::canBeAChild(vtkMRMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  if (node->IsA("vtkMRMLDisplayableNode"))
    {
    return true;
    }
  if (node->IsA("vtkMRMLDisplayableHierarchyNode"))
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneDisplayableModel::canBeAParent(vtkMRMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  if (node->IsA("vtkMRMLDisplayableNode"))
    {
    return true;
    }
  if (node->IsA("vtkMRMLDisplayableHierarchyNode"))
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneDisplayableModel::reparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
  if (!node)
    {
    return false;
    }

  // MRML Displayable nodes (inherits from transformable)
  vtkMRMLDisplayableNode *displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  vtkMRMLDisplayableNode *displayableParentNode = vtkMRMLDisplayableNode::SafeDownCast(newParent);
  vtkMRMLDisplayableHierarchyNode *hierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
  vtkMRMLDisplayableHierarchyNode *hierarchyParentNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(newParent);

  // we can be reparenting a hierarchy node to another hierarchy node, or a
  // displayable node (under it's hierarchy)
  if (hierarchyNode)
    {
    if (displayableParentNode &&
        displayableParentNode->GetScene() &&
        displayableParentNode->GetID())
      {
      // get it's hierarchy node
      hierarchyParentNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(displayableParentNode->GetScene(), displayableParentNode->GetID());
      }
    // else use the safe down cast of the parent node
    if (hierarchyParentNode &&
        hierarchyParentNode->GetID())
      {
      hierarchyNode->SetParentNodeID(hierarchyParentNode->GetID());
      }
    else
      {
      // reparenting to top with null parent id
      hierarchyNode->SetParentNodeID(NULL);
      }
    return true;
    }
  // we can be reparenting a displayable node to another displayable node or a
  // hierarchy node.
  else if (displayableNode)
    {
    if (displayableNode->GetScene() &&
        displayableNode->GetID())
      {
      hierarchyNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(displayableNode->GetScene(), displayableNode->GetID());
      }
    if (displayableParentNode && displayableParentNode->GetScene() &&  displayableParentNode->GetID())
      {
      hierarchyParentNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(displayableParentNode->GetScene(), displayableParentNode->GetID());
      }
    // else it uses the safe down cast to a hierarchy node of the newParent
    if (hierarchyNode)
      {
      if (hierarchyParentNode && hierarchyParentNode->GetID())
        {
        hierarchyNode->SetParentNodeID(hierarchyParentNode->GetID());
        }
      else
        {
        // reparenting to top with null parent id
        hierarchyNode->SetParentNodeID(NULL);
        }
      return true;
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
       (node = static_cast<vtkMRMLNode*>(d->MRMLScene->GetCurrentScene()->GetNextItemAsObject(it))) ;)
    {
    // WARNING: works only if the nodes are in the scene in the correct order:
    // parents are before children
    this->insertNode(node);
    }
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneDisplayableModel::insertNode(vtkMRMLNode* node)
{
  QStandardItem* nodeItem = this->itemFromNode(node);
  if (nodeItem)
    {
    return nodeItem;
    }
  vtkMRMLNode* parentNode = this->parentNode(node);
  //std::cout << "insertNode: parentNode is " << (parentNode == NULL ? "null" : "not null") << ", mrml scene item is " << (this->mrmlSceneItem() == NULL ? "null" : "not null") << std::endl;
  QStandardItem* parentItem =
    parentNode ? this->itemFromNode(parentNode) : this->mrmlSceneItem();
  if (!parentItem)
    {
    //std::cout << "insertNode: parent item is null, node id = " << node->GetID() << ", parent node id is " << (parentNode == NULL ? "null" : parentNode->GetID()) << std::endl;
    parentItem = this->insertNode(parentNode);
    }
  int min = 0;
  int max = 0;
  if (parentItem)
    {
    //Q_ASSERT(parentItem);
    min = this->preItems(parentItem).count();
    max = parentItem->rowCount() - this->postItems(parentItem).count();
    }
  nodeItem = this->insertNode(node, parentItem, qMin(min + this->nodeIndex(node), max));
  return nodeItem;
}

//------------------------------------------------------------------------------
void qMRMLSceneDisplayableModel::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  this->qMRMLSceneModel::updateItemFromNode(item, node, column);
  QStandardItem* parentItem = item->parent();
  QStandardItem* newParentItem = this->itemFromNode(this->parentNode(node));
  if (newParentItem == 0)
    {
    newParentItem = this->mrmlSceneItem();
    }
  // if the item has no parent, then it means it hasn't been put into the scene yet.
  // and it will do it automatically.
  if (parentItem != 0 && (parentItem != newParentItem || this->nodeIndex(node) != item->row()))
    {
    QList<QStandardItem*> children = parentItem->takeRow(item->row());
    int min = this->preItems(newParentItem).count();
    int max = newParentItem->rowCount() - this->postItems(newParentItem).count();
    int pos = qMin(min + this->nodeIndex(node), max);
    newParentItem->insertRow(pos, children);
    }
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneDisplayableModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  Q_D(const qMRMLSceneDisplayableModel);
  Q_UNUSED(column);
  QFlags<Qt::ItemFlag> flags = this->qMRMLSceneModel::nodeFlags(node, column);
  if (qMRMLSceneDisplayableModel::canBeAChild(node))
    {
    flags = flags | Qt::ItemIsDragEnabled;
    }
  if (qMRMLSceneDisplayableModel::canBeAParent(node))
    {
    flags = flags | Qt::ItemIsDropEnabled;
    }
  return flags;
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
  if (this->parentNode(node) != parent)
    {
    this->reparent(node, parent);
    }
  else if (this->nodeIndex(node) != item->row())
    {
    this->updateItemFromNode(item, node, item->column());
    }
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneDisplayableModel::supportedDropActions()const
{
  return Qt::MoveAction;
}
