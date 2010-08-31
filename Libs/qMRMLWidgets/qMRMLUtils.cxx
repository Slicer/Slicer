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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

//------------------------------------------------------------------------------
void qMRMLUtils::vtkMatrixToQVector(vtkMatrix4x4* matrix, QVector<double> & vector)
{
  if (!matrix) { return; }
  
  vector.clear();
  
  for (int i=0; i < 4; i++)
    {
    for (int j=0; j < 4; j++)
      {
      vector.append(matrix->GetElement(i,j)); 
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLUtils::getTransformInCoordinateSystem(vtkMRMLNode* node, bool global, 
    vtkTransform* transform)
{
  Self::getTransformInCoordinateSystem(vtkMRMLLinearTransformNode::SafeDownCast( node ), 
    global, transform); 
}

//------------------------------------------------------------------------------
void qMRMLUtils::getTransformInCoordinateSystem(vtkMRMLLinearTransformNode* transformNode, 
  bool global, vtkTransform* transform)
{
  Q_ASSERT(transform);
  if (!transform)
    {
    return;
    }

  transform->Identity();

  if (!transformNode) 
    { 
    return; 
    }

  vtkMatrix4x4 *matrix = transformNode->GetMatrixTransformToParent();
  Q_ASSERT(matrix);
  if (!matrix) 
    { 
    return; 
    }
  
  transform->SetMatrix(matrix);

  if ( global )
    {
    transform->PostMultiply();
    }
  else
    {
    transform->PreMultiply();
    }
}

//------------------------------------------------------------------------------
int qMRMLUtils::countVisibleViewNode(vtkMRMLScene* scene)
{
  Q_ASSERT(scene);
  int numberOfVisibleNodes = 0;
  const char* className = "vtkMRMLViewNode";
  int nnodes = scene->GetNumberOfNodesByClass(className);
  for (int n = 0; n < nnodes; n++)
    {
    vtkMRMLViewNode * node = vtkMRMLViewNode::SafeDownCast(scene->GetNthNodeByClass(n, className));
    if (node && node->GetVisibility())
      {
      numberOfVisibleNodes++;
      }
    }
  return numberOfVisibleNodes;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLUtils::topLevelNthNode(vtkMRMLScene* scene, int nodeIndex)
{
  if (scene == 0)
    {
    return 0;
    }
  vtkCollection* sceneCollection = scene->GetCurrentScene();
  vtkMRMLNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (node = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLNode* parent = qMRMLUtils::parentNode(node);
    if (parent != 0)
      {
      continue;
      }
    // compare IDs not pointers
    if (nodeIndex-- == 0)
      {
      return node;
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLUtils::childNode(vtkMRMLNode* node, int childIndex)
{
  // shortcut the following search if we are sure that the node has no children
  if (childIndex < 0 || node == 0 || !qMRMLUtils::canBeAParent(node))
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
    vtkMRMLNode* parent = qMRMLUtils::parentNode(n);
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

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLUtils::parentNode(vtkMRMLNode* node)
{
  // MRML Transformable nodes
  vtkMRMLTransformableNode* transformableNode = 
    vtkMRMLTransformableNode::SafeDownCast(node);
  if (transformableNode)
    {
    return transformableNode->GetParentTransformNode();
    }
  // MRML Color nodes
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLUtils::nodeIndex(vtkMRMLNode* node)
{
  const char* nodeId = node ? node->GetID() : 0;
  if (nodeId == 0)
    {
    return -1;
    }
  const char* nId = 0;
  int index = -1;
  vtkMRMLNode* parent = qMRMLUtils::parentNode(node);
  vtkCollection* sceneCollection = node->GetScene()->GetCurrentScene();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (n = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be NULL, it means that the scene is the parent
    if (parent == qMRMLUtils::parentNode(n))
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
int qMRMLUtils::childCount(vtkMRMLNode* node)
{
  // shortcut the following search if we are sure that the node has no children
  if (node == 0 ||
      vtkMRMLTransformNode::SafeDownCast(node) == 0)
    {
    return 0;
    }
  // MRML Transformable nodes
  QString nodeId = QString(node->GetID());
  int childCount = 0;
  vtkCollection* sceneCollection = node->GetScene()->GetCurrentScene();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (n = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLNode* parent = qMRMLUtils::parentNode(n);
    if (parent == 0)
      {
      continue;
      }
    // compare IDs not pointers
    if (nodeId == parent->GetID())
      {
      ++childCount;
      }
    }
  return childCount;
}

//------------------------------------------------------------------------------
int qMRMLUtils::childCount(vtkMRMLScene* scene)
{
  if (scene == 0)
    {
    return 0;
    }
  // MRML Transformable nodes
  int childCount = 0;
  vtkCollection* sceneCollection = scene->GetCurrentScene();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (n = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLNode* parent = qMRMLUtils::parentNode(n);
    if (parent)
      {
      continue;
      }
    ++childCount;
    }
  return childCount;
}

//------------------------------------------------------------------------------
bool qMRMLUtils::canBeAChild(vtkMRMLNode* node)
{
  if (!node)
    {
    return false;
    }
  if (node->IsA("vtkMRMLTransformableNode"))
    {
    return true;
    }
  // add other draggable nodes
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLUtils::canBeAParent(vtkMRMLNode* node)
{
  if (!node)
    {
    return false;
    }
  if (node->IsA("vtkMRMLTransformNode"))
    {
    return true;
    }
  // add other draggable nodes
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLUtils::reparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
  Q_ASSERT(node);
  if (!node)
    {
    return false;
    }
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
bool qMRMLUtils::canReparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
  Q_UNUSED(newParent);
  Q_ASSERT(node);
  if (!node)
    {
    return false;
    }
  // MRML Transformable Nodes
  if (node->IsA("vtkMRMLTransformableNode"))
    {
    return true;
    }
  return false;
}
