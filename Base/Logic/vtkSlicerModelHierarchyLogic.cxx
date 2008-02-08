/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerModelHierarchyLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkSlicerModelHierarchyLogic.h"

vtkCxxRevisionMacro(vtkSlicerModelHierarchyLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerModelHierarchyLogic);

//----------------------------------------------------------------------------
vtkSlicerModelHierarchyLogic::vtkSlicerModelHierarchyLogic()
{
  this->ModelHierarchyNodesMTime = 0;
}

//----------------------------------------------------------------------------
vtkSlicerModelHierarchyLogic::~vtkSlicerModelHierarchyLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerModelHierarchyLogic::ProcessMRMLEvents(vtkObject * /*caller*/, 
                                            unsigned long /*event*/, 
                                            void * /*callData*/)
{
  // TODO: implement if needed
}

//----------------------------------------------------------------------------
int vtkSlicerModelHierarchyLogic::UpdateModelToHierarchyMap()
{
  if (this->MRMLScene == NULL)
    {
    this->ModelHierarchyNodes.clear();
    }
  else if (this->MRMLScene->GetSceneModifiedTime() > this->ModelHierarchyNodesMTime)
  {
    this->ModelHierarchyNodes.clear();
    
    std::vector<vtkMRMLNode *> nodes;
    int nnodes = this->MRMLScene->GetNodesByClass("vtkMRMLModelHierarchyNode", nodes);
  
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
    this->ModelHierarchyNodesMTime = this->MRMLScene->GetSceneModifiedTime();
  }
  return ModelHierarchyNodes.size();
}

//---------------------------------------------------------------------------
vtkMRMLModelHierarchyNode* vtkSlicerModelHierarchyLogic::GetModelHierarchyNode(const char *modelNodeID)
{
  if (modelNodeID == NULL)
    {
    return NULL;
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
    return NULL;
    }
  
}


//----------------------------------------------------------------------------
void vtkSlicerModelHierarchyLogic::UpdateHierarchyChildrenMap()
{
  std::map<std::string, std::vector< vtkMRMLModelHierarchyNode *> >::iterator iter;
  for (iter  = this->HierarchyChildrenNodes.begin();
       iter != this->HierarchyChildrenNodes.end();
       iter++)
    {
    iter->second.clear();
    }
  this->HierarchyChildrenNodes.clear();
    
  if (this->MRMLScene->GetSceneModifiedTime() > this->ModelHierarchyNodesMTime)
  {
    if (this->MRMLScene == NULL)
      {
      return;
      }
    std::vector<vtkMRMLNode *> nodes;
    int nnodes = this->MRMLScene->GetNodesByClass("vtkMRMLModelHierarchyNode", nodes);
  
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
    this->ModelHierarchyNodesMTime = this->MRMLScene->GetSceneModifiedTime();
  }
}
