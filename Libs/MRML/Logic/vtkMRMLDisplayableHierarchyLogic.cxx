/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableHierarchyLogic.cxx,v $
  Date:      $Date: 2010-02-15 16:35:35 -0500 (Mon, 15 Feb 2010) $
  Version:   $Revision: 12142 $

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLDisplayableHierarchyLogic.h"

// MRML includes
#include "vtkMRMLDisplayableHierarchyNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLDisplayableHierarchyLogic);

//----------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyLogic::vtkMRMLDisplayableHierarchyLogic() = default;

//----------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyLogic::~vtkMRMLDisplayableHierarchyLogic() = default;

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!displayableNode || this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }
  // A displayable hierarchy node without children as well as a displayable
  // node is useless node. Delete it.
  vtkMRMLDisplayableHierarchyNode* displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(
    vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(this->GetMRMLScene(), node->GetID()) );
  if (displayableHierarchyNode &&
      displayableHierarchyNode->GetNumberOfChildrenNodes() == 0)
    {
    this->GetMRMLScene()->RemoveNode(displayableHierarchyNode);
    }
}


//----------------------------------------------------------------------------
char *vtkMRMLDisplayableHierarchyLogic::AddDisplayableHierarchyNodeForNode(vtkMRMLDisplayableNode *node)
{
  char *hierarchyNodeID = nullptr;

  if (!node)
    {
    vtkErrorMacro("AddDisplayableHierarchyNodeForNode: null node!");
    return hierarchyNodeID;
    }
  if (!node->GetScene())
    {
    vtkErrorMacro("AddDisplayableHierarchyNodeForNode: node isn't in a scene!");
    return hierarchyNodeID;
    }
  vtkMRMLDisplayableHierarchyNode *hierarchyNode = nullptr;
  hierarchyNode = vtkMRMLDisplayableHierarchyNode::New();
  // it's a stealth node:
  hierarchyNode->HideFromEditorsOn();

  // give it a unique name based on the node
  std::string hnodeName = std::string(node->GetName()) + std::string(" Hierarchy");
  hierarchyNode->SetName(node->GetScene()->GetUniqueNameByString(hnodeName.c_str()));

  node->GetScene()->AddNode(hierarchyNode);
  // with a parent node id of null, it's a child of the scene

  // now point to the  node, need disable modified event to avoid an assert in qMRMLSceneModel
  node->SetDisableModifiedEvent(1);
  hierarchyNode->SetDisplayableNodeID(node->GetID());
  node->SetDisableModifiedEvent(0);

  // save the id for return
  hierarchyNodeID = hierarchyNode->GetID();

  // clean up
  hierarchyNode->Delete();

  return hierarchyNodeID;
}

//----------------------------------------------------------------------------
bool vtkMRMLDisplayableHierarchyLogic::AddChildToParent(vtkMRMLDisplayableNode *child, vtkMRMLDisplayableNode *parent)
{
  if (!child)
    {
    vtkErrorMacro("AddChildToParent: null child node");
    return false;
    }
  if (!child->GetScene())
    {
    vtkErrorMacro("AddChildToParent: child is not in a scene");
    return false;
    }
  if (!parent)
    {
    vtkErrorMacro("AddChildToParent: null parent node");
    return false;
    }
  if (!parent->GetScene())
    {
    vtkErrorMacro("AddChildToParent: parent is not in a scene");
    return false;
    }

  // does the parent already have a hierarchy node associated with it?
  char *parentHierarchyNodeID = nullptr;
  vtkMRMLDisplayableHierarchyNode *hierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(
    vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(parent->GetScene(), parent->GetID()) );
  if (!hierarchyNode)
    {
    // create one and add to the scene
    parentHierarchyNodeID = this->AddDisplayableHierarchyNodeForNode(parent);
    }
  else
    {
    parentHierarchyNodeID = hierarchyNode->GetID();
    }
  if (!parentHierarchyNodeID)
    {
    vtkWarningMacro("AddChildToParent: unable to add or find a hierarchy node for the parent node " << parent->GetID() << ", so unable to place the child in a hierarchy");
    return false;
    }

  // does the child already have a hierarchy node associated with it?
  vtkMRMLDisplayableHierarchyNode *childHierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(
    vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(child->GetScene(), child->GetID()) );
  if (!childHierarchyNode)
    {
    char *childHierarchyNodeID = this->AddDisplayableHierarchyNodeForNode(child);
    if (childHierarchyNodeID)
      {
      vtkMRMLNode *mrmlNode = child->GetScene()->GetNodeByID(childHierarchyNodeID);
      if (mrmlNode)
        {
        childHierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(mrmlNode);
        }
      }
    }
  if (childHierarchyNode)
    {
    std::cout << "AddChildToParent: parentHierarchyID = " << parentHierarchyNodeID << ", childHierarchyNodeID = " << childHierarchyNode->GetID() << std::endl;
    // disable modified events on the parent
    vtkMRMLNode *parentNode = childHierarchyNode->GetScene()->GetNodeByID(parentHierarchyNodeID);
    parentNode->SetDisableModifiedEvent(1);
    childHierarchyNode->SetParentNodeID(parentHierarchyNodeID);
    parentNode->SetDisableModifiedEvent(0);

    return true;
    }
  else
    {
    vtkWarningMacro("AddChildToParent: unable to add or find a hierarchy node for the child node " << child->GetID() << ", so unable to place it in a hierarchy");
    return false;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLDisplayableHierarchyLogic::DeleteHierarchyNodeAndChildren(vtkMRMLDisplayableHierarchyNode *hnode)
{
  if (!hnode)
    {
    vtkErrorMacro("DeleteHierarchyNodeAndChildren: no hierarchy node given");
    return false;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("DeleteHierarchyNodeAndChildren: no scene defined on this class");
    return false;
    }

  // first off, set up batch processing mode on the scene
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  // get all the children nodes
  std::vector< vtkMRMLHierarchyNode *> allChildren;
  hnode->GetAllChildrenNodes(allChildren);

  // and loop over them
  for (unsigned int i = 0; i < allChildren.size(); ++i)
    {
    vtkMRMLDisplayableHierarchyNode *dispHierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(allChildren[i]);
    if (dispHierarchyNode)
      {
      // get any associated node
      vtkMRMLNode *associatedNode = dispHierarchyNode->GetAssociatedNode();
      if (associatedNode)
        {
        this->GetMRMLScene()->RemoveNode(associatedNode);
        }
      // remove the display node (hierarchy nodes aren't displayable so the
      // scene doesn't do the housekeeping automatically)
      vtkMRMLDisplayNode *dispDisplayNode = dispHierarchyNode->GetDisplayNode();
      if (dispDisplayNode)
        {
        this->GetMRMLScene()->RemoveNode(dispDisplayNode);
        }
      this->GetMRMLScene()->RemoveNode(dispHierarchyNode);
      }
    }
  // sanity check
  bool retval = true;
  if (hnode->GetNumberOfChildrenNodes() != 0)
    {
    vtkErrorMacro("Failed to delete all children hierarchy nodes! Still have " << hnode->GetNumberOfChildrenNodes() << " left");
    retval = false;
    }
  // delete it's display node
  vtkMRMLDisplayNode *dispNode = hnode->GetDisplayNode();
  if (dispNode)
    {
    this->GetMRMLScene()->RemoveNode(dispNode);
    }
  // and then delete itself
  this->GetMRMLScene()->RemoveNode(hnode);

  // end batch processing
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);

  return retval;
}
