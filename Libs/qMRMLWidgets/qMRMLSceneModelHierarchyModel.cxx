/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

// Qt includes

// qMRML includes
#include "qMRMLSceneModelHierarchyModel.h"
#include "qMRMLSceneModel_p.h"

// MRMLLogic includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelHierarchyLogic.h>

// MRML includes

// VTK includes

//------------------------------------------------------------------------------
class qMRMLSceneModelHierarchyModelPrivate: public qMRMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneModelHierarchyModel);
public:
  qMRMLSceneModelHierarchyModelPrivate(qMRMLSceneModelHierarchyModel& object);
  void init();
  vtkMRMLDisplayNode* displayNode(vtkMRMLNode* node)const;

  vtkSmartPointer<vtkMRMLModelHierarchyLogic> ModelLogic;

  int ColorColumn;
  int OpacityColumn;
};

//------------------------------------------------------------------------------
qMRMLSceneModelHierarchyModelPrivate
::qMRMLSceneModelHierarchyModelPrivate(qMRMLSceneModelHierarchyModel& object)
  : qMRMLSceneModelPrivate(object)
{
  this->ModelLogic = vtkSmartPointer<vtkMRMLModelHierarchyLogic>::New();
  this->ColorColumn = -1;
  this->OpacityColumn = -1;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelHierarchyModelPrivate::init()
{
  Q_Q(qMRMLSceneModelHierarchyModel);
  q->setVisibilityColumn(qMRMLSceneModel::NameColumn);
}

//------------------------------------------------------------------------------
vtkMRMLDisplayNode* qMRMLSceneModelHierarchyModelPrivate
::displayNode(vtkMRMLNode* node)const
{
  if (vtkMRMLDisplayNode::SafeDownCast(node))
    {
    return vtkMRMLDisplayNode::SafeDownCast(node);
    }

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);
  if (modelNode)
    {
    return modelNode->GetDisplayNode();
    }

  vtkMRMLModelHierarchyNode* modelHierarchyNode
    = vtkMRMLModelHierarchyNode::SafeDownCast(node);
  if (modelHierarchyNode)
    {
    return modelHierarchyNode->GetDisplayNode();
    }
  return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneModelHierarchyModel::qMRMLSceneModelHierarchyModel(QObject *vparent)
  :qMRMLSceneModel(new qMRMLSceneModelHierarchyModelPrivate(*this), vparent)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  d->init();
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
Qt::DropActions qMRMLSceneModelHierarchyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneModelHierarchyModel::parentNode(vtkMRMLNode* node)const
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);
  vtkMRMLHierarchyNode* modelHierarchyNode = 0;
  if (modelNode)
    {
    vtkMRMLDisplayableHierarchyNode *displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(node->GetScene(), node->GetID());
    if (displayableHierarchyNode)
      {
      modelHierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(displayableHierarchyNode);
      if (modelHierarchyNode)
        {
        //std::cout << "qMRMLSceneModelHierarchyModel::parentNode: node " << (node->GetName() != NULL ? node->GetName() : node->GetID()) << " is a model node and it has a model hierarchy node " << (modelHierarchyNode->GetName() ? modelHierarchyNode->GetName() : modelHierarchyNode->GetID()) << std::endl;
        }
      }
    //Q_ASSERT(modelHierarchyNode);
    }
  else
    {
    modelHierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
    }
  return vtkMRMLModelHierarchyNode::SafeDownCast(
    modelHierarchyNode ? modelHierarchyNode->GetParentNode() : 0);
}

//------------------------------------------------------------------------------
int qMRMLSceneModelHierarchyModel::nodeIndex(vtkMRMLNode* node)const
{
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
    //vtkMRMLModelHierarchyNodeList children = d->ModelLogic->GetHierarchyChildrenNodes(parent);
    std::vector< vtkMRMLHierarchyNode *> children = parent->GetChildrenNodes();


    for(std::vector< vtkMRMLHierarchyNode *>::const_iterator it = children.begin();
      it !=children.end(); ++it)
      {
        vtkMRMLModelHierarchyNode* hierarchy = vtkMRMLModelHierarchyNode::SafeDownCast(*it);
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
bool qMRMLSceneModelHierarchyModel::canBeAChild(vtkMRMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  return node->IsA("vtkMRMLModelHierarchyNode") || node->IsA("vtkMRMLModelNode");
}

//------------------------------------------------------------------------------
bool qMRMLSceneModelHierarchyModel::canBeAParent(vtkMRMLNode* node)const
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

  vtkMRMLModelHierarchyNode* hierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(
    vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(node->GetScene(), node->GetID()));
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

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneModelHierarchyModel::insertNode(vtkMRMLNode* node, QStandardItem* parent, int row)
{
  QStandardItem* insertedItem = this->Superclass::insertNode(node, parent, row);
  if (this->listenNodeModifiedEvent())
    {
    qvtkConnect(node, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));
    }
  return insertedItem;
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneModelHierarchyModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  Q_D(const qMRMLSceneModelHierarchyModel);
  QFlags<Qt::ItemFlag> flags = this->Superclass::nodeFlags(node, column);
  if (column == d->ColorColumn &&
      d->displayNode(node) != 0)
    {
    flags |= Qt::ItemIsEditable;
    }
  if (column == d->OpacityColumn &&
      d->displayNode(node) != 0)
    {
    flags |= Qt::ItemIsEditable;
    }
  return flags;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelHierarchyModel
::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  if (column == d->ColorColumn)
    {
    vtkMRMLDisplayNode* displayNode = d->displayNode(node);
    if (displayNode)
      {
      double* rgbF = displayNode->GetColor();
      QColor color = QColor::fromRgbF(rgbF[0], rgbF[1], rgbF[2],
                                      displayNode->GetOpacity());
      item->setData(color, Qt::DecorationRole);
      }
    }
  else if (column == d->OpacityColumn)
    {
    vtkMRMLDisplayNode* displayNode = d->displayNode(node);
    if (displayNode)
      {
      double displayedOpacity
        = QString::number(displayNode->GetOpacity(), 'f', 2).toDouble();
      item->setData(displayedOpacity, Qt::DisplayRole);
      }
    }
  this->Superclass::updateItemDataFromNode(item, node, column);
}

//------------------------------------------------------------------------------
void qMRMLSceneModelHierarchyModel
::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  if (item->column() == d->ColorColumn)
    {
    vtkMRMLDisplayNode* displayNode = d->displayNode(node);
    if (displayNode)
      {
      QColor color = item->data(Qt::DecorationRole).value<QColor>();
      /// TODO: move {Start|End}Modify() in qMRMLSceneMode::updateNodeFromItem()
      int wasModifying = displayNode->StartModify();
      displayNode->SetColor(color.redF(), color.greenF(), color.blueF());
      displayNode->SetOpacity(color.alphaF());
      displayNode->EndModify(wasModifying);
      }
    }
 else if (item->column() == d->OpacityColumn)
    {
    vtkMRMLDisplayNode* displayNode = d->displayNode(node);
    if (displayNode)
      {
      QString displayedOpacity = item->data(Qt::DisplayRole).toString();
      QString currentOpacity = QString::number( displayNode->GetOpacity(), 'f', 2);
      if (displayedOpacity != currentOpacity)
        {
        displayNode->SetOpacity(displayedOpacity.toDouble());
        }
      }
    }
  return this->Superclass::updateNodeFromItemData(node, item);
}

//------------------------------------------------------------------------------
int qMRMLSceneModelHierarchyModel::colorColumn()const
{
  Q_D(const qMRMLSceneModelHierarchyModel);
  return d->ColorColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelHierarchyModel::setColorColumn(int column)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  d->ColorColumn = column;
  /// TODO: refresh the items
}

//------------------------------------------------------------------------------
int qMRMLSceneModelHierarchyModel::opacityColumn()const
{
  Q_D(const qMRMLSceneModelHierarchyModel);
  return d->OpacityColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelHierarchyModel::setOpacityColumn(int column)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  d->OpacityColumn = column;
  /// TODO: refresh the items
}
