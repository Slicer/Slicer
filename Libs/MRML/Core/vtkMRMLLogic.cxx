
// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"

// MRMLLogic includes
#include "vtkMRMLLogic.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <set>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLogic);

//------------------------------------------------------------------------------
vtkMRMLLogic::vtkMRMLLogic()
{
  this->Scene = NULL;
}

//------------------------------------------------------------------------------
vtkMRMLLogic::~vtkMRMLLogic()
{
}

void vtkMRMLLogic::RemoveUnreferencedStorageNodes()
{
  if (this->Scene == NULL)
    {
    return;
    }
  std::set<vtkMRMLNode *> referencedNodes;
  std::set<vtkMRMLNode *>::iterator iter;
  std::vector<vtkMRMLNode *> storableNodes;
  std::vector<vtkMRMLNode *> storageNodes;
  this->Scene->GetNodesByClass("vtkMRMLStorableNode", storableNodes);
  this->Scene->GetNodesByClass("vtkMRMLStorageNode", storageNodes);

  vtkMRMLNode *node = NULL;
  vtkMRMLStorableNode *storableNode = NULL;
  vtkMRMLStorageNode *storageNode = NULL;
  unsigned int i;
  for (i=0; i<storableNodes.size(); i++)
    {
    node = storableNodes[i];
    if (node)
      {
      storableNode = vtkMRMLStorableNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    storageNode = storableNode->GetStorageNode();
    if (storageNode)
      {
      referencedNodes.insert(storageNode);
      }
    }

  for (i=0; i<storageNodes.size(); i++)
    {
    node = storageNodes[i];
    if (node)
      {
      storageNode = vtkMRMLStorageNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    iter = referencedNodes.find(storageNode);
    if (iter == referencedNodes.end())
      {
      this->Scene->RemoveNode(storageNode);
      }
    }
}

void vtkMRMLLogic::RemoveUnreferencedDisplayNodes()
{
  if (this->Scene == NULL)
    {
    return;
    }
  std::set<vtkMRMLNode *> referencedNodes;
  std::set<vtkMRMLNode *>::iterator iter;
  std::vector<vtkMRMLNode *> displayableNodes;
  std::vector<vtkMRMLNode *> displayNodes;
  this->Scene->GetNodesByClass("vtkMRMLDisplayableNode", displayableNodes);
  this->Scene->GetNodesByClass("vtkMRMLDisplayNode", displayNodes);

  vtkMRMLNode *node = NULL;
  vtkMRMLDisplayableNode *displayableNode = NULL;
  vtkMRMLDisplayNode *displayNode = NULL;
  unsigned int i;
  for (i=0; i<displayableNodes.size(); i++)
    {
    node = displayableNodes[i];
    if (node)
      {
      displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    int numDisplayNodes = displayableNode->GetNumberOfDisplayNodes();
    for (int n=0; n<numDisplayNodes; n++)
      {
      displayNode = displayableNode->GetNthDisplayNode(n);
      if (displayNode)
        {
        referencedNodes.insert(displayNode);
        }
      }
    }

  for (i=0; i<displayNodes.size(); i++)
    {
    node = displayNodes[i];
    if (node)
      {
      displayNode = vtkMRMLDisplayNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    iter = referencedNodes.find(displayNode);
    if (iter == referencedNodes.end())
      {
      this->Scene->RemoveNode(displayNode);
      }
    }
}


