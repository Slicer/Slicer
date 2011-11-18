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

// Qt includes
#include <QDebug>

// qMRML includes
#include "qMRMLSceneHierarchyModel_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLHierarchyNode.h>

// VTK includes
#include <vtkNew.h>

//------------------------------------------------------------------------------
qMRMLSceneHierarchyModelPrivate
::qMRMLSceneHierarchyModelPrivate(qMRMLSceneHierarchyModel& object)
  : qMRMLSceneModelPrivate(object)
{
}

//------------------------------------------------------------------------------
vtkMRMLHierarchyNode* qMRMLSceneHierarchyModelPrivate::CreateHierarchyNode()const
{
  return vtkMRMLHierarchyNode::New();
}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneHierarchyModel::qMRMLSceneHierarchyModel(QObject *vparent)
  :Superclass(new qMRMLSceneHierarchyModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneHierarchyModel::qMRMLSceneHierarchyModel(
  qMRMLSceneHierarchyModelPrivate* pimpl, QObject *parent)
  :Superclass(pimpl, parent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneHierarchyModel::~qMRMLSceneHierarchyModel()
{
}

/*

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneHierarchyModel::childNode(vtkMRMLNode* node, int childIndex)
{
  // shortcut the following search if we are sure that the node has no children
  if (childIndex < 0 || node == 0 || !qMRMLSceneHierarchyModel::canBeAParent(node))
    {
    return 0;
    }
  // MRML Transformable nodes
  QString nodeId = QString(node->GetID());
  vtkCollection* nodes = node->GetScene()->GetNodes();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (n = (vtkMRMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLNode* parent = qMRMLSceneHierarchyModel::parentNode(n);
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
vtkMRMLNode* qMRMLSceneHierarchyModel::parentNode(vtkMRMLNode* node)const
{
  vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(node);
  if (!hierarchyNode)
    {
    hierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(node->GetScene(), node->GetID());
    }
  return hierarchyNode ? hierarchyNode->GetParentNode() : 0;
}


//------------------------------------------------------------------------------
int qMRMLSceneHierarchyModel::nodeIndex(vtkMRMLNode* node)const
{
  Q_D(const qMRMLSceneHierarchyModel);
  if (!d->MRMLScene)
    {
    return -1;
    }

  const char* nodeId = node ? node->GetID() : 0;
  if (nodeId == 0)
    {
    return -1;
    }

  // is there a hierarchy node associated with this node?
  vtkMRMLHierarchyNode *assocHierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(d->MRMLScene, node->GetID());
  if (assocHierarchyNode)
    {
    int assocHierarchyNodeIndex = this->nodeIndex(assocHierarchyNode);
    return assocHierarchyNodeIndex + 1;
    }

  const char* nId = 0;
  vtkMRMLNode* parent = this->parentNode(node);
  int index = 0;
  // if it's part of a hierarchy, use the GetIndexInParent call
  if (parent)
    {
    vtkMRMLHierarchyNode *hnode = vtkMRMLHierarchyNode::SafeDownCast(node);
    if (hnode)
      {
      vtkMRMLHierarchyNode* parentHierarchy = vtkMRMLHierarchyNode::SafeDownCast(parent);
      const int childrenCount = parentHierarchy->GetNumberOfChildrenNodes();
      for ( int i = 0; i < childrenCount ; ++i)
        {
        vtkMRMLHierarchyNode* child = parentHierarchy->GetNthChildNode(i);
        if (child == hnode)
          {
          return index;
          }
        ++index;
        // the associated node of a hierarchynode is displayed after the hierarchynode
        if (child->GetAssociatedNode())
          {
          ++index;
          }
        }
      }
    }

  // otherwise, iterate through the scene
  vtkCollection* nodes = d->MRMLScene->GetNodes();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;

  for (nodes->InitTraversal(it);
       (n = (vtkMRMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be NULL, it means that the scene is the parent
    if (parent == this->parentNode(n))
      {
      nId = n->GetID();
      if (nId && !strcmp(nodeId, nId))
        {
//      std::cout << "nodeIndex:  no parent for node " << node->GetID() << " index = " << index << std::endl;
        return index;
        }
      if (!vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(d->MRMLScene, n->GetID()))
        {
        ++index;
        }
      vtkMRMLHierarchyNode* hierarchy = vtkMRMLHierarchyNode::SafeDownCast(n);
      if (hierarchy && hierarchy->GetAssociatedNode())
        {
        // if the current node is a hierarchynode associated with the node,
        // then it should have been caught at the beginning of the function
        Q_ASSERT(strcmp(nodeId, hierarchy->GetAssociatedNodeID()));
        ++index;
        }
      }
    }
  return -1;
}

//------------------------------------------------------------------------------
bool qMRMLSceneHierarchyModel::canBeAChild(vtkMRMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  if (node->IsA("vtkMRMLNode"))
    {
    return true;
    }
  if (node->IsA("vtkMRMLHierarchyNode"))
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneHierarchyModel::canBeAParent(vtkMRMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  if (node->IsA("vtkMRMLHierarchyNode"))
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneHierarchyModel::reparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
  Q_D(qMRMLSceneHierarchyModel);
  if (!node)
    {
    return false;
    }

  vtkMRMLNode *mrmlNode = vtkMRMLNode::SafeDownCast(node);
  vtkMRMLHierarchyNode *hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(node);
  vtkMRMLNode *mrmlParentNode = NULL;
  vtkMRMLHierarchyNode *hierarchyParentNode = NULL;
  if (newParent)
    {
    mrmlParentNode = vtkMRMLNode::SafeDownCast(newParent);
    hierarchyParentNode = vtkMRMLHierarchyNode::SafeDownCast(newParent);
    }

  // we can be reparenting a hierarchy node to another hierarchy node, or a
  // mrml node (under it's hierarchy)
  if (hierarchyNode)
    {
    if (!hierarchyParentNode &&
        mrmlParentNode &&
        mrmlParentNode->GetScene() &&
        mrmlParentNode->GetID())
      {
      // get it's hierarchy node
      hierarchyParentNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(mrmlParentNode->GetScene(), mrmlParentNode->GetID());
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
  // we can be reparenting a mrml node to another mrml node or a
  // hierarchy node.
  else if (mrmlNode)
    {
    if (mrmlNode->GetScene() &&
        mrmlNode->GetID())
      {
      hierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(mrmlNode->GetScene(), mrmlNode->GetID());
      if (!hierarchyNode)
        {
        vtkMRMLHierarchyNode* newHierarchyNode = d->CreateHierarchyNode();
        newHierarchyNode->SetName(this->mrmlScene()->GetUniqueNameByString(
          newHierarchyNode->GetNodeTagName()));
        //newHierarchyNode->SetHideFromEditors(1);
        //newHierarchyNode->AllowMultipleChildrenOff();
        newHierarchyNode->SetAssociatedNodeID(mrmlNode->GetID());
        mrmlNode->GetScene()->AddNode(newHierarchyNode);
        qWarning() << "qMRMLSceneHierarchyModel::reparent: Added a new hierarchy node " << newHierarchyNode->GetID();
        newHierarchyNode->Delete();
        // try again
        hierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(mrmlNode->GetScene(), mrmlNode->GetID());
        }
      }
    Q_ASSERT_X(hierarchyNode != mrmlParentNode, "qMRMLSceneHierarchyNode::reparent", "Shouldn't be possible, maybe the droppable flag wasn't set");
    if (!hierarchyParentNode && mrmlParentNode && mrmlParentNode->GetScene() &&  mrmlParentNode->GetID())
      {
      hierarchyParentNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(mrmlParentNode->GetScene(), mrmlParentNode->GetID());
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
Qt::DropActions qMRMLSceneHierarchyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneHierarchyModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  QFlags<Qt::ItemFlag> flags = this->Superclass::nodeFlags(node, column);
  if (this->canBeAParent(node))
    {
    vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(node);
    if (hierarchyNode &&
        (hierarchyNode->GetAssociatedNode() ||
         (!hierarchyNode->GetAllowMultipleChildren() &&
           hierarchyNode->GetNumberOfChildrenNodes() > 0)))
      {
      flags &= ~Qt::ItemIsDropEnabled;
      }
    }
  return flags;
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneHierarchyModel::insertNode(vtkMRMLNode* node, QStandardItem* parent, int row)
{
  QStandardItem* insertedItem = this->Superclass::insertNode(node, parent, row);
  if (this->listenNodeModifiedEvent())
    {
    if (node)
      {
      qvtkConnect(node, vtkMRMLNode::HierarchyModifiedEvent,
                  this, SLOT(onMRMLNodeModified(vtkObject*)));
      }
    }
  return insertedItem;
}

