/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkIntArray.h"

#include "vtkMRMLHierarchyNode.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLScene.h"

typedef std::map<std::string, std::vector< vtkMRMLHierarchyNode *> > HierarchyChildrenNodesType;

std::map< vtkMRMLScene*, HierarchyChildrenNodesType> vtkMRMLHierarchyNode::SceneHierarchyChildrenNodes = std::map< vtkMRMLScene*, HierarchyChildrenNodesType>();
std::map< vtkMRMLScene*, unsigned long> vtkMRMLHierarchyNode::SceneHierarchyChildrenNodesMTime = std::map< vtkMRMLScene*, unsigned long>();

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode::vtkMRMLHierarchyNode()
{
  this->HideFromEditors = 0;

  this->ParentNodeID = NULL;

}

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode::~vtkMRMLHierarchyNode()
{
  if (this->ParentNodeID) 
    {
    delete [] this->ParentNodeID;
    this->ParentNodeID = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->ParentNodeID != NULL) 
    {
    of << indent << " parentNodeRef=\"" << this->ParentNodeID << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->ParentNodeID && !strcmp(oldID, this->ParentNodeID))
    {
    this->SetParentNodeID(ParentNodeID);
    }
}
//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "parentNodeRef")) 
      {
      this->SetParentNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ParentNodeID, this);
      }
    }

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLHierarchyNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLHierarchyNode *node = (vtkMRMLHierarchyNode *) anode;
  this->SetParentNodeID(node->ParentNodeID);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "ParentNodeID: " <<
    (this->ParentNodeID ? this->ParentNodeID : "(none)") << "\n";
}

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode* vtkMRMLHierarchyNode::GetParentNode()
{
  vtkMRMLHierarchyNode* node = NULL;
  if (this->GetScene() && this->ParentNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->ParentNodeID);
    node = vtkMRMLHierarchyNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------
void vtkMRMLHierarchyNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkMRMLHierarchyNode::UpdateReferences()
{
  Superclass::UpdateReferences();
  
  if (this->ParentNodeID != NULL && this->Scene->GetNodeByID(this->ParentNodeID) == NULL)
    {
    this->SetParentNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode* vtkMRMLHierarchyNode::GetTopParentNode()
{
  vtkMRMLHierarchyNode *node = NULL;
  vtkMRMLHierarchyNode *parent = vtkMRMLHierarchyNode::SafeDownCast(this->GetParentNode());
  if (parent == NULL) 
    {
    node = this;
    }
  else 
    {
    node =  parent->GetTopParentNode();
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::GetAllChildrenNodes(std::vector< vtkMRMLHierarchyNode *> &childrenNodes)
{
  if (this->GetScene() == NULL)
    {
    return;
    }
  
  this->UpdateChildrenMap();

  std::map< vtkMRMLScene*, HierarchyChildrenNodesType>::iterator siter = 
        SceneHierarchyChildrenNodes.find(this->GetScene());
  
  HierarchyChildrenNodesType::iterator iter = 
    siter->second.find(std::string(this->GetID()));
  if (iter == siter->second.end()) 
    {
    return;
    }
  for (unsigned int i=0; i<iter->second.size(); i++)
    {
    childrenNodes.push_back(iter->second[i]);
    iter->second[i]->GetAllChildrenNodes(childrenNodes);
    }
}

//----------------------------------------------------------------------------
std::vector< vtkMRMLHierarchyNode *> vtkMRMLHierarchyNode::GetChildrenNodes()
{
  std::vector< vtkMRMLHierarchyNode *> childrenNodes;
  if (this->GetScene() == NULL)
    {
    return childrenNodes;
    }
  
  this->UpdateChildrenMap();

  std::map< vtkMRMLScene*, HierarchyChildrenNodesType>::iterator siter = 
        SceneHierarchyChildrenNodes.find(this->GetScene());

  HierarchyChildrenNodesType::iterator iter =
    siter->second.find(std::string(this->GetID()));
  if (iter == siter->second.end()) 
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
void vtkMRMLHierarchyNode::UpdateChildrenMap()
{
  if (this->GetScene() == NULL)
    {
    this->SceneHierarchyChildrenNodes.clear();
    this->SceneHierarchyChildrenNodesMTime.clear();
    return;
    }

  std::map< vtkMRMLScene*, HierarchyChildrenNodesType>::iterator siter = 
        SceneHierarchyChildrenNodes.find(this->GetScene());
  if (siter == SceneHierarchyChildrenNodes.end())
    {
    HierarchyChildrenNodesType h;
    SceneHierarchyChildrenNodes[this->GetScene()] = h;
    siter = SceneHierarchyChildrenNodes.find(this->GetScene());
    SceneHierarchyChildrenNodesMTime[this->GetScene()] = 0;
    }

  std::map< vtkMRMLScene*, unsigned long>::iterator titer = 
        SceneHierarchyChildrenNodesMTime.find(this->GetScene());

  std::map<std::string, std::vector< vtkMRMLHierarchyNode *> >::iterator iter;
  if (this->GetScene() == 0)
    {
    for (iter  = siter->second.begin();
         iter != siter->second.end();
         iter++)
      {
      iter->second.clear();
      }
    siter->second.clear();
    }
    
  if (this->GetScene()->GetSceneModifiedTime() > titer->second)
  {
    for (iter  = siter->second.begin();
         iter != siter->second.end();
         iter++)
      {
      iter->second.clear();
      }
    siter->second.clear();
    
    std::vector<vtkMRMLNode *> nodes;
    int nnodes = this->GetScene()->GetNodesByClass("vtkMRMLHierarchyNode", nodes);
  
    for (int i=0; i<nnodes; i++)
      {
      vtkMRMLHierarchyNode *node =  vtkMRMLHierarchyNode::SafeDownCast(nodes[i]);
      if (node)
        {
        vtkMRMLHierarchyNode *pnode = vtkMRMLHierarchyNode::SafeDownCast(node->GetParentNode());
        if (pnode)
          {
          iter = siter->second.find(std::string(pnode->GetID()));
          if (iter == siter->second.end())
            {
            std::vector< vtkMRMLHierarchyNode *> children;
            children.push_back(node);
            siter->second[std::string(pnode->GetID())] = children;
            }
          else
            {
            iter->second.push_back(node);
            }
          }
        }
      }
    titer->second = this->GetScene()->GetSceneModifiedTime();
  }
}

void vtkMRMLHierarchyNode::HierarchyIsModified()
{
  if (this->GetScene() == NULL)
    {
    return;
    }

  SceneHierarchyChildrenNodesMTime[this->GetScene()] = 0;
}

// End
