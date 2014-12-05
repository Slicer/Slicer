/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelHierarchyLogic.cxx,v $
  Date:      $Date: 2010-02-15 16:35:35 -0500 (Mon, 15 Feb 2010) $
  Version:   $Revision: 12142 $

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLModelHierarchyLogic.h"

// MRML includes
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLModelHierarchyLogic);

// The number should large enough so smaller branches can be shown/hidden
// without entering batch processing mode. The number should be small enough
// so that showing/hiding nodes one-by one doesn't take too long time.
// 30 nodes seems to be a reasonble compromise.
int vtkMRMLModelHierarchyLogic::ChildrenVisibilitySetBatchUpdateThreshold = 30;

//----------------------------------------------------------------------------
vtkMRMLModelHierarchyLogic::vtkMRMLModelHierarchyLogic()
{
  this->ModelHierarchyNodesMTime = 0;
  this->HierarchyChildrenNodesMTime = 0;
}

//----------------------------------------------------------------------------
vtkMRMLModelHierarchyLogic::~vtkMRMLModelHierarchyLogic()
{
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);
  if (!modelNode || this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }
  // A model hierarchy node is useless without a model node. Delete it.
  vtkMRMLModelHierarchyNode* modelHierarchyNode =
    this->GetModelHierarchyNode(modelNode->GetID());
  if (modelHierarchyNode)
    {
    this->GetMRMLScene()->RemoveNode(modelHierarchyNode);
    }
}

//----------------------------------------------------------------------------
int vtkMRMLModelHierarchyLogic::UpdateModelToHierarchyMap()
{
  if (this->GetMRMLScene() == 0)
    {
    this->ModelHierarchyNodes.clear();
    }
  else if (this->GetMRMLScene()->GetNodes()->GetMTime() > this->ModelHierarchyNodesMTime)
  {
    this->ModelHierarchyNodes.clear();

    std::vector<vtkMRMLNode *> nodes;
    int nnodes = this->GetMRMLScene()->GetNodesByClass("vtkMRMLModelHierarchyNode", nodes);

    for (int i=0; i<nnodes; i++)
      {
      vtkMRMLModelHierarchyNode *node =  vtkMRMLModelHierarchyNode::SafeDownCast(nodes[i]);
      if (node)
        {
        vtkMRMLModelNode *mnode = node->GetModelNode();
        if (mnode)
          {
          this->ModelHierarchyNodes[std::string(mnode->GetID())] = node;
          }
        }
      }
    this->ModelHierarchyNodesMTime = this->GetMRMLScene()->GetNodes()->GetMTime();
  }
  return static_cast<int>(ModelHierarchyNodes.size());
}

//---------------------------------------------------------------------------
vtkMRMLModelHierarchyNode* vtkMRMLModelHierarchyLogic::GetModelHierarchyNode(const char *modelNodeID)
{
  if (modelNodeID == 0)
    {
    return 0;
    }
  this->UpdateModelToHierarchyMap();

  std::map<std::string, vtkMRMLModelHierarchyNode *>::iterator iter;

  iter = this->ModelHierarchyNodes.find(modelNodeID);
  if (iter != this->ModelHierarchyNodes.end())
    {
    return iter->second;
    }
  else
    {
    return 0;
    }
}
//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyLogic::GetHierarchyChildrenNodes(
  vtkMRMLModelHierarchyNode *parentNode,
  vtkMRMLModelHierarchyNodeList &childrenNodes)
{
  if (!parentNode)
    {
    return;
    }
  this->UpdateHierarchyChildrenMap();

  HierarchyChildrenNodesType::iterator iter =
    this->HierarchyChildrenNodes.find(std::string(parentNode->GetID()));
  if (iter == this->HierarchyChildrenNodes.end())
    {
    return;
    }
  for (unsigned int i=0; i<iter->second.size(); i++)
    {
    childrenNodes.push_back(iter->second[i]);
    this->GetHierarchyChildrenNodes(iter->second[i], childrenNodes);
    }
}

//----------------------------------------------------------------------------
vtkMRMLModelHierarchyNodeList vtkMRMLModelHierarchyLogic
::GetHierarchyChildrenNodes(vtkMRMLModelHierarchyNode *parentNode)
{
  vtkMRMLModelHierarchyNodeList childrenNodes;

  if (!parentNode)
    {
    return childrenNodes;
    }

  this->UpdateHierarchyChildrenMap();

  HierarchyChildrenNodesType::iterator iter =
    this->HierarchyChildrenNodes.find(std::string(parentNode->GetID()));
  if (iter == this->HierarchyChildrenNodes.end())
    {
    return childrenNodes;
    }
  for (unsigned int i=0; i<iter->second.size(); i++)
    {
    childrenNodes.push_back(iter->second[i]);
    }
  return childrenNodes;
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyLogic::UpdateHierarchyChildrenMap()
{
  // Clear cache if scene is invalid
  std::map<std::string, std::vector< vtkMRMLModelHierarchyNode *> >::iterator iter;
  if (!this->GetMRMLScene())
    {
    for (iter  = this->HierarchyChildrenNodes.begin();
         iter != this->HierarchyChildrenNodes.end();
         iter++)
      {
      iter->second.clear();
      }
    this->HierarchyChildrenNodes.clear();
    return;
    }

  // Skip update if nodes were not modified since last update
  if (this->GetMRMLScene()->GetNodes()->GetMTime() <= this->HierarchyChildrenNodesMTime)
    {
    return;
    }

  // Clear hierarchy children nodes cache
  for (iter  = this->HierarchyChildrenNodes.begin();
       iter != this->HierarchyChildrenNodes.end();
       iter++)
    {
    iter->second.clear();
    }
  this->HierarchyChildrenNodes.clear();

  // Update hierarchy children nodes cache
  std::vector<vtkMRMLNode *> nodes;
  int nnodes = this->GetMRMLScene()->GetNodesByClass("vtkMRMLModelHierarchyNode", nodes);

  for (int i=0; i<nnodes; i++)
    {
    vtkMRMLModelHierarchyNode *node =  vtkMRMLModelHierarchyNode::SafeDownCast(nodes[i]);
    if (node)
      {
      vtkMRMLModelHierarchyNode *pnode = vtkMRMLModelHierarchyNode::SafeDownCast(node->GetParentNode());
      if (pnode)
        {
        iter = this->HierarchyChildrenNodes.find(std::string(pnode->GetID()));
        if (iter == this->HierarchyChildrenNodes.end())
          {
          std::vector< vtkMRMLModelHierarchyNode *> children;
          children.push_back(node);
          this->HierarchyChildrenNodes[std::string(pnode->GetID())] = children;
          }
        else
          {
          iter->second.push_back(node);
          }
        }
      }
    }
  this->HierarchyChildrenNodesMTime = this->GetMRMLScene()->GetNodes()->GetMTime();
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyLogic::SetChildrenVisibility(vtkMRMLDisplayableHierarchyNode *displayableHierarchyNode,
                                                       const char *displayableNodeClass, const char *displayNodeClass,
                                                       int visibility)
{
  if (displayableHierarchyNode==NULL)
  {
    std::cerr << "vtkMRMLModelHierarchyLogic::SetChildrenVisibility failed: displayableHierarchyNode is invalid" << std::endl;
    return;
  }
  vtkMRMLDisplayNode *displayNode = displayableHierarchyNode->GetDisplayNode();
  if (displayNode)
    {
    displayNode->SetVisibility(visibility);
    }

  std::vector< vtkMRMLHierarchyNode *> children;
  displayableHierarchyNode->GetAllChildrenNodes(children);
  vtkMRMLModelNode *model = NULL;
  vtkMRMLNode      *node = NULL;
  // When there are many child nodes in a hierarchy then show/hide is much more efficient if batch processing is enabled.
  // However, if there are few nodes only then a full refresh at the end of a batch processing takes longer than doing
  // the update on each node separately.
  bool batchProcess = (static_cast<int>(children.size()) > vtkMRMLModelHierarchyLogic::ChildrenVisibilitySetBatchUpdateThreshold);
  vtkMRMLScene* scene=NULL;
  if (batchProcess)
    {
    if (!children.empty() && children.front()!=NULL)
      {
      scene=children.front()->GetScene();
      }
    if (scene!=NULL)
      {
      scene->StartState(vtkMRMLScene::BatchProcessState);
      }
    else
      {
      vtkWarningWithObjectMacro(displayableHierarchyNode,"SetChildrenVisibility cannot be performed using batch processing because scene is invalid");
      batchProcess=false;
      }
    }
  for (unsigned int i=0; i<children.size(); i++)
    {
    node = children[i]->GetAssociatedNode();
    if (node)
      {
      model = vtkMRMLModelNode::SafeDownCast(node);
      if (model)
        {
        if (displayableNodeClass && model->IsA(displayableNodeClass))
          {
          model->SetDisplayClassVisibility(displayNodeClass, visibility);
          }
        else
          {
          model->SetDisplayClassVisibility(0, visibility);
          }
        }
      }
    vtkMRMLDisplayableHierarchyNode *dhnode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(children[i]);
    if (dhnode)
      {
      displayNode = dhnode->GetDisplayNode();
      if (displayNode)
        {
        displayNode->SetVisibility(visibility);
        }
      }
    }
  if (batchProcess && scene!=NULL)
    {
    scene->EndState(vtkMRMLScene::BatchProcessState);
    }
}
