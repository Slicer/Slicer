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
#include <algorithm>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkIntArray.h"

#include "vtkMRMLHierarchyNode.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLScene.h"

typedef std::map<std::string, std::vector< vtkMRMLHierarchyNode *> > HierarchyChildrenNodesType;

std::map< vtkMRMLScene*, HierarchyChildrenNodesType> vtkMRMLHierarchyNode::SceneHierarchyChildrenNodes = std::map< vtkMRMLScene*, HierarchyChildrenNodesType>();
std::map< vtkMRMLScene*, unsigned long> vtkMRMLHierarchyNode::SceneHierarchyChildrenNodesMTime = std::map< vtkMRMLScene*, unsigned long>();

double vtkMRMLHierarchyNode::MaximumSortingValue = 0;

typedef vtkMRMLHierarchyNode* const vtkMRMLHierarchyNodePointer; 
bool vtkMRMLHierarchyNodeSortPredicate(vtkMRMLHierarchyNodePointer d1, vtkMRMLHierarchyNodePointer d2);
bool vtkMRMLHierarchyNodeSortPredicate(vtkMRMLHierarchyNodePointer d1, vtkMRMLHierarchyNodePointer d2)
{
  return d1->GetSortingValue() < d2->GetSortingValue();
}

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode::vtkMRMLHierarchyNode()
{
  this->HideFromEditors = 0;

  this->ParentNodeIDReference = NULL;

  this->SortingValue = 0;

}

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode::~vtkMRMLHierarchyNode()
{
  if (this->ParentNodeIDReference) 
    {
    delete [] this->ParentNodeIDReference;
    this->ParentNodeIDReference = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->ParentNodeIDReference != NULL) 
    {
    of << indent << " parentNodeRef=\"" << this->ParentNodeIDReference << "\"";
    }
  of << indent << " sortingValue=\"" << this->SortingValue << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->ParentNodeIDReference && !strcmp(oldID, this->ParentNodeIDReference))
    {
    this->SetParentNodeID(newID);
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
      // dont reset SortingValue
      double soringValue = this->GetSortingValue();
      this->SetParentNodeID(attValue);
      this->SetSortingValue(soringValue);
      //this->Scene->AddReferencedNodeID(this->ParentNodeIDReference, this);
      }
    else if (!strcmp(attName, "sortingValue")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> SortingValue;
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
  this->SetParentNodeID(node->ParentNodeIDReference);
  this->SetSortingValue(node->SortingValue);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "ParentNodeID: " <<
    (this->ParentNodeIDReference ? this->ParentNodeIDReference : "(none)") << "\n";
  os << indent << "SortingValue:     " << this->SortingValue << "\n";

}

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode* vtkMRMLHierarchyNode::GetParentNode()
{
  vtkMRMLHierarchyNode* node = NULL;
  if (this->GetScene() && this->ParentNodeIDReference != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->ParentNodeIDReference);
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
  
  if (this->ParentNodeIDReference != NULL && this->Scene->GetNodeByID(this->ParentNodeIDReference) == NULL)
    {
    this->SetParentNodeID(NULL);
    }
}

//-----------------------------------------------------------
void vtkMRMLHierarchyNode::SetParentNodeID(const char* ref) 
{
  if ((this->ParentNodeIDReference && ref && strcmp(ref, this->ParentNodeIDReference)) ||
      (this->ParentNodeIDReference != ref))
    {
    this->SetSortingValue(MaximumSortingValue+1);
    this->SetParentNodeIDReference(ref);
    this->HierarchyIsModified(this->GetScene());
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

  // Sort the vector using predicate and std::sort
  std::sort(childrenNodes.begin(), childrenNodes.end(), vtkMRMLHierarchyNodeSortPredicate);

  return childrenNodes;
}

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode* vtkMRMLHierarchyNode::GetNthChildNode(int index)
{
  std::vector< vtkMRMLHierarchyNode *> childrenNodes = this->GetChildrenNodes();
  if (index < 0 || index > (int)(childrenNodes.size()-1))
    {
    vtkErrorMacro("vtkMRMLHierarchyNode::GetNthChildNode() index " << index << " outside the range 0-" << childrenNodes.size()-1 );
    return NULL;
    }
  else
    {
    return childrenNodes[index];
    }
}

//----------------------------------------------------------------------------

int vtkMRMLHierarchyNode::GetIndexInParent()
{
  vtkMRMLHierarchyNode *pnode = this->GetParentNode();
  if (pnode == NULL)
    {
    vtkErrorMacro("vtkMRMLHierarchyNode::GetIndexInParent() no parent");
    return -1;
    }
  else
    {
    std::vector< vtkMRMLHierarchyNode *> childrenNodes = pnode->GetChildrenNodes();
    for (unsigned int i=0; i<childrenNodes.size(); i++)
      {
      if (childrenNodes[i] == this)
        {
        return i;
        }
      }
    return -1;
    }
}

//----------------------------------------------------------------------------

void vtkMRMLHierarchyNode::SetIndexInParent(int index)
{
  vtkMRMLHierarchyNode *pnode = this->GetParentNode();
  if (pnode == NULL)
    {
    vtkErrorMacro("vtkMRMLHierarchyNode::SetIndexInParent() no parent");
    return;
    }
  else
    {
    std::vector< vtkMRMLHierarchyNode *> childrenNodes = pnode->GetChildrenNodes();
    if (index < 0 || index >= (int)childrenNodes.size())
      {
      vtkErrorMacro("vtkMRMLHierarchyNode::SetIndexInParent() index " << index << ", outside the range 0-" << childrenNodes.size()-1);
      return;
      }
    double sortValue = childrenNodes[index]->GetSortingValue();
    if (index == 0) 
      {
      sortValue -= 1;
      }
    else
      {
      sortValue = 0.5*(sortValue + childrenNodes[index-1]->GetSortingValue());
      }
    }
  return;
}

//----------------------------------------------------------------------------

void vtkMRMLHierarchyNode::RemoveHierarchyChildrenNodes()
{
  if (this->GetScene() == NULL)
    {
    return;
    }

  char *parentID = this->GetParentNodeID();
  std::vector< vtkMRMLHierarchyNode *> children = this->GetChildrenNodes();
  for (unsigned int i=0; i<children.size(); i++)
    {
    vtkMRMLHierarchyNode *child = children[i];
    std::vector< vtkMRMLHierarchyNode *> childChildern = child->GetChildrenNodes();
    for (unsigned int j=0; i<childChildern.size(); j++)
      {
      childChildern[j]->SetParentNodeID(parentID);
      }
    this->GetScene()->RemoveNode(child);
    }
}
//----------------------------------------------------------------------------

void vtkMRMLHierarchyNode::RemoveAllHierarchyChildrenNodes()
{
  if (this->GetScene() == NULL)
    {
    return;
    }

  std::vector< vtkMRMLHierarchyNode *> children = this->GetChildrenNodes();
  for (unsigned int i=0; i<children.size(); i++)
    {
    vtkMRMLHierarchyNode *child = children[i];
    std::vector< vtkMRMLHierarchyNode *> childChildern = child->GetChildrenNodes();
    for (unsigned int j=0; i<childChildern.size(); j++)
      {
      childChildern[j]->RemoveAllHierarchyChildrenNodes();
      }
    this->GetScene()->RemoveNode(child);
    }
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

    double maxSortingValue = this->MaximumSortingValue;

    for (int i=0; i<nnodes; i++)
      {
      vtkMRMLHierarchyNode *node =  vtkMRMLHierarchyNode::SafeDownCast(nodes[i]);
      if (node)
        {
        vtkMRMLHierarchyNode *pnode = vtkMRMLHierarchyNode::SafeDownCast(node->GetParentNode());
        if (pnode)
          {
          if (pnode->GetSortingValue() > maxSortingValue)
            {
            maxSortingValue = pnode->GetSortingValue();
            }
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
    this->MaximumSortingValue = maxSortingValue;
  }
}

void vtkMRMLHierarchyNode::HierarchyIsModified(vtkMRMLScene *scene)
{
  if (scene == NULL)
    {
    return;
    }

  SceneHierarchyChildrenNodesMTime[scene] = 0;
}

// End
