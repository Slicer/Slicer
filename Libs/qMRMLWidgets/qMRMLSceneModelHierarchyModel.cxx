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
#include "qMRMLSceneModelHierarchyModel.h"
#include "qMRMLSceneModel_p.h"

// MRMLLogic includes
#include <vtkMRMLModelHierarchyLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelHierarchyNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLSceneModelHierarchyModelPrivate: public qMRMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneModelHierarchyModel);
public:
  qMRMLSceneModelHierarchyModelPrivate(qMRMLSceneModelHierarchyModel& object);
  vtkSmartPointer<vtkMRMLModelHierarchyLogic> ModelLogic;
};

//------------------------------------------------------------------------------
qMRMLSceneModelHierarchyModelPrivate
::qMRMLSceneModelHierarchyModelPrivate(qMRMLSceneModelHierarchyModel& object)
  : qMRMLSceneModelPrivate(object)
{
  this->ModelLogic = vtkSmartPointer<vtkMRMLModelHierarchyLogic>::New();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneModelHierarchyModel::qMRMLSceneModelHierarchyModel(QObject *vparent)
  :qMRMLSceneModel(new qMRMLSceneModelHierarchyModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneModelHierarchyModel::~qMRMLSceneModelHierarchyModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneModelHierarchyModel::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  d->ModelLogic->SetMRMLScene(scene);
  this->qMRMLSceneModel::setMRMLScene(scene);
}

//------------------------------------------------------------------------------
void qMRMLSceneModelHierarchyModel::populateScene()
{
  Q_D(qMRMLSceneModelHierarchyModel);
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
void qMRMLSceneModelHierarchyModel::insertNode(vtkMRMLNode* node)
{
  vtkMRMLNode* parentNode = this->parentNode(node);
  QStandardItem* parentItem =
    parentNode ? this->itemFromNode(parentNode) : this->mrmlSceneItem();
  Q_ASSERT(parentItem);
  int min = this->preItems(parentItem).count();
  int max = parentItem->rowCount() - this->postItems(parentItem).count();
  this->insertNode(node, parentItem, qMin(min + qMRMLSceneModelHierarchyModel::nodeIndex(node), max));
}

//------------------------------------------------------------------------------
void qMRMLSceneModelHierarchyModel::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  this->qMRMLSceneModel::updateItemFromNode(item, node, column);
  bool oldBlock = this->blockSignals(true);
  if (this->canBeAChild(node))
    {
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    }
  if (this->canBeAParent(node))
    {
    item->setFlags(item->flags() | Qt::ItemIsDropEnabled);
    }
  this->blockSignals(oldBlock);
  QStandardItem* parentItem = item->parent();
  QStandardItem* newParentItem = this->itemFromNode(this->parentNode(node));
  if (newParentItem == 0)
    {
    newParentItem = this->mrmlSceneItem();
    }
  // if the item has no parent, then it means it hasn't been put into the scene yet.
  // but it will be done later automatically.
  if (parentItem != 0 && (parentItem != newParentItem || this->nodeIndex(node) != item->row()))
    {
    // The item has been reparented here (or moved?), manually reparent to the new parent
    QList<QStandardItem*> children = parentItem->takeRow(item->row());
    int min = this->preItems(newParentItem).count();
    int max = newParentItem->rowCount() - this->postItems(newParentItem).count();
    int pos = qMin(min + this->nodeIndex(node), max);
    newParentItem->insertRow(pos, children);
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModelHierarchyModel::updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item)
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
Qt::DropActions qMRMLSceneModelHierarchyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneModelHierarchyModel::parentNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);
  vtkMRMLHierarchyNode* modelHierarchyNode = 0;
  if (modelNode)
    {
    modelHierarchyNode =
      d->ModelLogic->GetModelHierarchyNode(node->GetID());
    //Q_ASSERT(modelHierarchyNode);
    }
  else
    {
    modelHierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
    }
  return modelHierarchyNode ? modelHierarchyNode->GetParentNode() : 0;
}

//------------------------------------------------------------------------------
int qMRMLSceneModelHierarchyModel::nodeIndex(vtkMRMLNode* node)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  const char* nodeId = node ? node->GetID() : 0;
  if (nodeId == 0)
    {
    return -1;
    }
  int index = 0;
  vtkMRMLModelHierarchyNode* parent = 
    vtkMRMLModelHierarchyNode::SafeDownCast(this->parentNode(node));
  if (!parent)
    {
    vtkCollection* sceneCollection = node->GetScene()->GetCurrentScene();
    vtkMRMLNode* n = 0;
    vtkCollectionSimpleIterator it;
    for (sceneCollection->InitTraversal(it);
         (n = static_cast<vtkMRMLNode*>(sceneCollection->GetNextItemAsObject(it))) ;)
      {
      // note: parent can be NULL, it means that the scene is the parent
      if (parent == this->parentNode(n))
        {
        const char* nId = n->GetID();
        if (nId && !strcmp(nodeId, nId))
          {
          return index;
          }
        vtkMRMLModelHierarchyNode* hierarchy = vtkMRMLModelHierarchyNode::SafeDownCast(n);
        index += (hierarchy && hierarchy->GetModelNodeID() != 0 ? 2 : 1);
        }
      }
    }
  else
    {
    vtkMRMLModelNode* mnode = vtkMRMLModelNode::SafeDownCast(node);
    vtkMRMLModelHierarchyNode* hnode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
    vtkMRMLModelHierarchyNodeList children = d->ModelLogic->GetHierarchyChildrenNodes(parent);

    for(vtkMRMLModelHierarchyNodeList::const_iterator it = children.begin();
      it !=children.end(); ++it)
      {
      vtkMRMLModelHierarchyNode* hierarchy = *it;
      if (mnode)
        {
        const char* nId = hierarchy->GetModelNodeID();
        if (nId && !strcmp(nodeId, nId))
          {
          return index;
          }
        }
      else if (hnode)
        {
        if (hierarchy == node)
          {
          return index;
          }
        }
      index += (hierarchy->GetModelNodeID() != 0 ? 2 : 1);
      }
    }
  return -1;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModelHierarchyModel::canBeAChild(vtkMRMLNode* node)
{
  if (!node)
    {
    return false;
    }
  return node->IsA("vtkMRMLModelHierarchyNode") || node->IsA("vtkMRMLModelNode");
}

//------------------------------------------------------------------------------
bool qMRMLSceneModelHierarchyModel::canBeAParent(vtkMRMLNode* node)
{
  vtkMRMLModelHierarchyNode* hnode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
  if (hnode && hnode->GetModelNodeID() == 0)
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModelHierarchyModel::reparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  Q_ASSERT(node);
  vtkMRMLModelHierarchyNode* modelParent =
    vtkMRMLModelHierarchyNode::SafeDownCast(this->parentNode(node));
  vtkMRMLModelHierarchyNode* newModelParent =
    vtkMRMLModelHierarchyNode::SafeDownCast(newParent);    
  if (!node || modelParent == newParent)
    {
    return false;
    }
  Q_ASSERT(newParent != node);
  vtkMRMLModelNode *mnode = vtkMRMLModelNode::SafeDownCast(node);
  vtkMRMLModelHierarchyNode *hnode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
  
  d->ModelLogic->HierarchyIsModified();
  
  if (hnode)
    {
    hnode->SetParentNodeID(newModelParent ? newModelParent->GetID() : 0);
    vtkMRMLModelNode* model = hnode ? hnode->GetModelNode() : 0;
    if (model)
      {
      model->Modified();
      }
    return true;
    }
  Q_ASSERT(mnode);
  vtkMRMLModelHierarchyNode* hierarchyNode = d->ModelLogic->GetModelHierarchyNode(node->GetID());
  if (newModelParent == 0)
    {
    if (hierarchyNode)
      {
      vtkMRMLDisplayNode *dnode = hierarchyNode->GetDisplayNode();
      if (dnode)
        {
        this->mrmlScene()->RemoveNode(dnode);
        }
      this->mrmlScene()->RemoveNode(hierarchyNode);
      }
    }
  else 
    {
    if (!hierarchyNode)
      {
      hierarchyNode = vtkMRMLModelHierarchyNode::New();
      hierarchyNode->SetName(this->mrmlScene()->GetUniqueNameByString(
        d->MRMLScene->GetTagByClassName("vtkMRMLModelHierarchyNode")));
      hierarchyNode->SetSelectable(0);
      hierarchyNode->SetHideFromEditors(1);
      hierarchyNode->SetModelNodeID(mnode->GetID());
      this->mrmlScene()->AddNode(hierarchyNode);
      hierarchyNode->Delete();
      d->ModelLogic->HierarchyIsModified();
      }
    //vtkMRMLModelHierarchyNode *oldParentNode = vtkMRMLModelHierarchyNode::SafeDownCast(modelParent->GetParentNode());
    //if (oldParentNode)
    //  {
    //  oldParentNode->SetModelNodeID(NULL);
    //  }
    hierarchyNode->SetParentNodeID(newModelParent->GetID());
    if (mnode)
      {
      mnode->Modified();
      }
    }
  return true;
}
