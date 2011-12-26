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
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLDisplayNode.h"

// VTK includes

vtkCxxRevisionMacro(vtkMRMLModelHierarchyLogic, "$Revision: 12142 $");
vtkStandardNewMacro(vtkMRMLModelHierarchyLogic);

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
int vtkMRMLModelHierarchyLogic::UpdateModelToHierarchyMap()
{
  if (this->GetMRMLScene() == 0)
    {
    this->ModelHierarchyNodes.clear();
    }
  else if (this->GetMRMLScene()->GetSceneModifiedTime() > this->ModelHierarchyNodesMTime)
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
    this->ModelHierarchyNodesMTime = this->GetMRMLScene()->GetSceneModifiedTime();
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
  std::map<std::string, std::vector< vtkMRMLModelHierarchyNode *> >::iterator iter;
  if (this->GetMRMLScene() == 0)
    {
    for (iter  = this->HierarchyChildrenNodes.begin();
         iter != this->HierarchyChildrenNodes.end();
         iter++)
      {
      iter->second.clear();
      }
    this->HierarchyChildrenNodes.clear();
    }
    
  if (this->GetMRMLScene() &&
      (this->GetMRMLScene()->GetSceneModifiedTime() > this->HierarchyChildrenNodesMTime))
    {
    for (iter  = this->HierarchyChildrenNodes.begin();
         iter != this->HierarchyChildrenNodes.end();
         iter++)
      {
      iter->second.clear();
      }
    this->HierarchyChildrenNodes.clear();
    
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
    this->HierarchyChildrenNodesMTime = this->GetMRMLScene()->GetSceneModifiedTime();
    }
}


//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyLogic::SetChildrenVisibility(vtkMRMLDisplayableHierarchyNode *displayableHierarchyNode,
                                                      int visibility)
{
  vtkMRMLDisplayNode *displayNode = displayableHierarchyNode->GetDisplayNode();
  if (displayNode)
    {
    displayNode->SetVisibility(visibility);
    }

  std::vector< vtkMRMLHierarchyNode *> children;
  displayableHierarchyNode->GetAllChildrenNodes(children);
  vtkMRMLModelNode *model = NULL;
  vtkMRMLNode      *node = NULL;
  for (unsigned int i=0; i<children.size(); i++)
    {
    node = children[i]->GetAssociatedNode();
    if (node)
      {
      model = vtkMRMLModelNode::SafeDownCast(node);
      if (model)
        {
        displayNode = model->GetDisplayNode();
        if (displayNode)
          {
          displayNode->SetVisibility(visibility);
          }
        }
      }
    vtkMRMLDisplayableHierarchyNode *dhnode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(children[i]);
    displayNode = dhnode->GetDisplayNode();
    if (displayNode)
      {
      displayNode->SetVisibility(visibility);
      }
    }
}

