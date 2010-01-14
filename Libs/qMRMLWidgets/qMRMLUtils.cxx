#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

// QT includes
#include <QDebug>

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
vtkMRMLNode* qMRMLUtils::topLevelNthNode(vtkMRMLScene* scene, int nodeIndex)
{
  if (scene == 0)
    {
    return 0;
    }
  for (int i = 0; i < scene->GetNumberOfNodes(); ++i)
    {
    vtkMRMLNode* node = scene->GetNthNode(i);
    Q_ASSERT(node);
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
  vtkMRMLScene* scene = node->GetScene();
  for (int i = 0; i < scene->GetNumberOfNodes(); ++i)
    {
    vtkMRMLNode* n = scene->GetNthNode(i);
    Q_ASSERT(n);
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
  if (!node)
    {
    return -1;
    }
  QString nodeId = node->GetID();
  vtkMRMLNode* parent = qMRMLUtils::parentNode(node);
  vtkMRMLScene* scene = node->GetScene();
  int index = -1;
  for (int i = 0; scene->GetNumberOfNodes();++i)
    {
    vtkMRMLNode* n = scene->GetNthNode(i);
    Q_ASSERT(n);
    // note: parent can be NULL, it means that the scene is the parent
    if (parent == qMRMLUtils::parentNode(n))
      {
      ++index;
      if (nodeId == n->GetID())
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
  vtkMRMLScene* scene = node->GetScene();
  int childCount = 0;
  for (int i = 0; i < scene->GetNumberOfNodes(); ++i)
    {
    vtkMRMLNode* n = scene->GetNthNode(i);
    Q_ASSERT(n);
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
  for (int i = 0; i < scene->GetNumberOfNodes(); ++i)
    {
    vtkMRMLNode* n = scene->GetNthNode(i);
    Q_ASSERT(n);
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
