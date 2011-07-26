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
#include "qMRMLSceneHierarchyModel.h"
#include "qMRMLSceneModel_p.h"
//#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLHierarchyNode.h>

// VTK includes


//------------------------------------------------------------------------------
class qMRMLSceneHierarchyModelPrivate: public qMRMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneHierarchyModel);
public:
  qMRMLSceneHierarchyModelPrivate(qMRMLSceneHierarchyModel& object);

};

//------------------------------------------------------------------------------
qMRMLSceneHierarchyModelPrivate
::qMRMLSceneHierarchyModelPrivate(qMRMLSceneHierarchyModel& object)
  : qMRMLSceneModelPrivate(object)
{

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneHierarchyModel::qMRMLSceneHierarchyModel(QObject *vparent)
  :qMRMLSceneModel(new qMRMLSceneHierarchyModelPrivate(*this), vparent)
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
  vtkCollection* sceneCollection = node->GetScene()->GetCurrentScene();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (n = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
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
  if (node == NULL)
    {
    return 0;
    }
  
  // MRML nodes
  vtkMRMLNode *mrmlNode = vtkMRMLNode::SafeDownCast(node);
  vtkMRMLHierarchyNode * hierarchyNode = NULL; 
  if (mrmlNode &&
      mrmlNode->GetScene() &&
      mrmlNode->GetID())
    {
    // get the hierarchy node associated with this node
    hierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(mrmlNode->GetScene(), mrmlNode->GetID());
    if (hierarchyNode)
      {
      if (hierarchyNode->GetHideFromEditors())
        {
        // this is a hidden hierarchy node, so we do not want to display it
        // instead, we will return the parent of the hidden hierarchy node
        // to be used as the parent for the mrmlNode
        return hierarchyNode->GetParentNode();
        }
      return hierarchyNode;
      }
    }
  if (hierarchyNode == NULL)
    {
    // the passed in node might have been a hierarchy node instead, try to
    // cast it
    hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(node);
    }
  if (hierarchyNode)
    {
    // return it's parent
    return hierarchyNode->GetParentNode();
    }
  return 0;
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
  const char* nId = 0;
  int index = -1;
  vtkMRMLNode* parent = this->parentNode(node);

  // if it's part of a hierarchy, use the GetIndexInParent call
  if (parent)
    {
    vtkMRMLHierarchyNode *hnode = vtkMRMLHierarchyNode::SafeDownCast(node);
    if (hnode)
      {
      // for each hierarchy node, the iterated index is 2*the index in the parent
      int adjustedIndexFromParent = 2*(hnode->GetIndexInParent());
      //std::cout << "nodeIndex: found a hierarchy parent for node " << node->GetID() << ", parent is " << parent->GetID() << ", index in parent is " << hnode->GetIndexInParent() << ", iterated index of " << index << ", returning adjusted index from parent = " <<  adjustedIndexFromParent << std::endl;
      return adjustedIndexFromParent;
      }
    
    else
      {
      // is there a hierarchy node associated with this node?
      vtkMRMLHierarchyNode *assocHierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(d->MRMLScene, node->GetID());
      if (assocHierarchyNode)
        {
        // in this case, the current node is at an index one greater than
        // it's associated hierarchy node via the index from the parent
        int adjustedIndexFromParent = 2*(assocHierarchyNode->GetIndexInParent()) + 1;
        //std::cout << "nodeIndex: found an associated hierarchy node for node " << node->GetID() << ", with an ID of " << assocHierarchyNode->GetID() << ". It's index in it's parent is " << assocHierarchyNode->GetIndexInParent() << ", iterated index = " << index << ". Returning adjusted index from parent of " << adjustedIndexFromParent << std::endl;
        return adjustedIndexFromParent;
        }
      }
    }

  // otherwise, iterate through the scene
  vtkCollection* sceneCollection = d->MRMLScene->GetCurrentScene();
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
//      std::cout << "nodeIndex:  no parent for node " << node->GetID() << " index = " << index << std::endl;
        return index;
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
//  if (node->IsA("vtkMRMLNode"))
//    {
//    return true;
//    }
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneHierarchyModel::reparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
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
    if (mrmlParentNode &&
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
      }
    if (mrmlParentNode && mrmlParentNode->GetScene() &&  mrmlParentNode->GetID())
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
