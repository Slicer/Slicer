/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDisplayableNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLScene.h"

// when change the display node, update the scalars
//#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkMath.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode::vtkMRMLDisplayableNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode::~vtkMRMLDisplayableNode()
{
  this->RemoveAllDisplayNodeIDs();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  std::stringstream ss;
  unsigned int n;
  for (n=0; n < this->DisplayNodeIDs.size(); n++)
    {
    ss << this->DisplayNodeIDs[n];
    if (n < DisplayNodeIDs.size()-1)
      {
      ss << " ";
      }
    }
  if (this->DisplayNodeIDs.size() > 0)
    {
    of << indent << " displayNodeRef=\"" << ss.str().c_str() << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "displayNodeRef"))
      {
      std::stringstream ss(attValue);
      while (!ss.eof())
        {
        std::string id;
        ss >> id;
        this->AddAndObserveDisplayNodeID(id.c_str());
        }
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  int wasModifying = this->StartModify();
  for (unsigned int i=0; i<this->DisplayNodeIDs.size(); i++)
    {
    if ( std::string(oldID) == this->DisplayNodeIDs[i])
      {
      this->SetAndObserveNthDisplayNodeID(i, newID);
      }
    }
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDisplayableNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);
  vtkMRMLDisplayableNode *node = (vtkMRMLDisplayableNode *) anode;
  this->RemoveAllDisplayNodeIDs();

  const int ndnodes = node->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    this->SetAndObserveNthDisplayNodeID(i, node->GetNthDisplayNodeID(i));
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  for (unsigned int i=0; i<this->DisplayNodeIDs.size(); i++)
    {
    os << indent << "DisplayNodeIDs[" << i << "]: " <<
      this->DisplayNodeIDs[i] << " -> " << this->DisplayNodes[i] << "\n";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  for (unsigned int i=0; i<this->DisplayNodeIDs.size(); ++i)
    {
    this->Scene->AddReferencedNodeID(this->DisplayNodeIDs[i].c_str(), this);
    }
}

//-----------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateScene(vtkMRMLScene *scene)
{
  this->Superclass::UpdateScene(scene);

  this->UpdateDisplayNodes();

  for (unsigned int i=0; i<this->DisplayNodeIDs.size(); ++i)
    {
    // When calling UpdateScene, the scene is up-to-date (it is done to be
    // imported/restored). All the nodes are in it.
    // Therefor, there is no reason why there wouldn't be a node
    // for each node id.
    // If you get an error message here, check in the scene xml if
    // DisplayNodeIDs[i] really exists.
    if (!this->DisplayNodes[i])
      {
      vtkErrorMacro("Displayable node (" << this->GetID() << ") cannot find display node (" << this->DisplayNodeIDs[i].c_str() << ")");
      }
    }
}

//-----------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateReferences()
{
  this->Superclass::UpdateReferences();
  for (unsigned int i=0; i<this->DisplayNodeIDs.size();)
    {
    if (this->Scene->GetNodeByID(this->DisplayNodeIDs[i]) == NULL)
      {
      this->RemoveNthDisplayNodeID(i);
      }
    else
      {
      ++i;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::RemoveAllDisplayNodeIDs()
{
  int wasModifying = this->StartModify();
  while(this->GetNumberOfDisplayNodes() > 0)
    {
    this->RemoveNthDisplayNodeID(0);
    }
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
const std::vector<vtkMRMLDisplayNode*>& vtkMRMLDisplayableNode::GetDisplayNodes()
{
  this->UpdateDisplayNodes();
  return this->DisplayNodes;
}

//----------------------------------------------------------------------------
const char * vtkMRMLDisplayableNode::GetNthDisplayNodeID(int n)
{
  if (n < 0 || n >= static_cast<int>(this->DisplayNodeIDs.size()))
    {
    return NULL;
    }
  return this->DisplayNodeIDs[n].c_str();
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLDisplayableNode::GetNthDisplayNode(int n)
{
  assert(this->DisplayNodes.size() == this->DisplayNodeIDs.size());
  if (n < 0 || n >= static_cast<int>(this->DisplayNodes.size()))
    {
    return NULL;
    }
  vtkMRMLDisplayNode* node = this->DisplayNodes[n];
  // Maybe the node was not yet in the scene when the node ID was set.
  // Check to see if it's now there.
  // Similarly, if the scene is 0, clear the display node if not already null.
  if ((!node || node->GetScene() != this->GetScene()) ||
      (node && this->GetScene() == 0))
    {
    this->UpdateNthDisplayNode(n);
    node = this->DisplayNodes[n];
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLDisplayableNode
::GetNthDisplayNodeByClass(int n, const char* className)
{
  for (int i = 0; i < this->GetNumberOfDisplayNodes(); ++i)
    {
    vtkMRMLDisplayNode* displayNode = this->GetNthDisplayNode(i);
    if (displayNode->IsA(className) && (n-- == 0))
      {
      return displayNode;
      }
    }
  return 0;
}

//-----------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateDisplayNodes()
{
  assert(this->DisplayNodeIDs.size() == this->DisplayNodes.size());
  int wasModifying = this->StartModify();

  for (unsigned int i=0; i<this->DisplayNodeIDs.size(); ++i)
    {
    this->UpdateNthDisplayNode(i);
    }

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateNthDisplayNode(int n)
{
  assert(n >= 0 && n < static_cast<int>(this->DisplayNodeIDs.size()));
  vtkMRMLNode* node = this->GetScene() ?
    this->GetScene()->GetNodeByID(this->DisplayNodeIDs[n]) : 0;
  this->SetAndObserveNthDisplayNode(n, vtkMRMLDisplayNode::SafeDownCast(node));
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetAndObserveNthDisplayNodeID(int n, const char *displayNodeID)
{
  std::string newDisplayNodeID;
  if (displayNodeID)
    {
    newDisplayNodeID = displayNodeID;
    }
  // Extend the list if needed. But don't do it if the node ID to add is null
  if (n >= static_cast<int>(this->DisplayNodeIDs.size()) &&
      displayNodeID != 0)
    {
    assert(n == static_cast<int>(this->DisplayNodeIDs.size()));
    this->DisplayNodeIDs.push_back(std::string());
    this->DisplayNodes.push_back(0);
    n = this->DisplayNodeIDs.size() - 1;
    }
  std::vector<std::string>::iterator displayNodeIDIt =
    this->DisplayNodeIDs.begin() + n;
  if (displayNodeIDIt == this->DisplayNodeIDs.end() ||
      *displayNodeIDIt == newDisplayNodeID)
    {
    if (n < static_cast<int>(this->DisplayNodeIDs.size()))
      {
      this->UpdateNthDisplayNode(n);
      }
    assert(this->DisplayNodeIDs.size() == this->DisplayNodes.size());
    return;
    }

  if (!displayNodeIDIt->empty() && this->Scene)
    {
    this->Scene->RemoveReferencedNodeID(
      displayNodeIDIt->c_str(), this);
    }

  // Delete the display node ID if the new value is 0.
  if (newDisplayNodeID.empty())
    {
    this->DisplayNodeIDs.erase(displayNodeIDIt);
    vtkMRMLDisplayNode* oldDisplayNode = this->DisplayNodes[n];
    /// Need to unobserve
    this->SetAndObserveNthDisplayNode(n, 0);
    this->DisplayNodes.erase(this->DisplayNodes.begin() + n);
    if (oldDisplayNode != 0)
      {
      this->OnDisplayNodeAdded(0);
      }
    }
  else
    {
    *displayNodeIDIt = newDisplayNodeID;
    if (this->Scene)
      {
      this->Scene->AddReferencedNodeID(newDisplayNodeID.c_str(), this);
      }
    this->UpdateNthDisplayNode(n);
    }
  assert(this->DisplayNodeIDs.size() == this->DisplayNodes.size());
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetAndObserveNthDisplayNode(int n, vtkMRMLDisplayNode *dnode)
{
  bool newNode = (this->DisplayNodes[n] != dnode &&
    // Don't call OnDisplayNodeAdded if deleting the entry
    dnode &&
    this->DisplayNodes.size() == this->DisplayNodeIDs.size());
  vtkSetAndObserveMRMLObjectMacro(this->DisplayNodes[n], dnode);
  if (newNode)
    {
    this->OnDisplayNodeAdded(dnode);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::OnDisplayNodeAdded(vtkMRMLDisplayNode* vtkNotUsed(dNode))
{
  this->InvokeEvent(vtkMRMLDisplayableNode::DisplayModifiedEvent, NULL);
}

//----------------------------------------------------------------------------
bool vtkMRMLDisplayableNode::HasDisplayNodeID(const char* displayNodeID)
{
  if (displayNodeID == 0)
    {
    return false;
    }
  return std::find(this->DisplayNodeIDs.begin(),
                   this->DisplayNodeIDs.end(),
                   std::string(displayNodeID)) != this->DisplayNodeIDs.end();
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayableNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  for (unsigned int i=0; i<this->DisplayNodes.size(); i++)
    {
    vtkMRMLDisplayNode *dnode = this->GetNthDisplayNode(i);
    if (dnode != NULL && dnode == vtkMRMLDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
      {
      this->InvokeEvent(vtkMRMLDisplayableNode::DisplayModifiedEvent, dnode);
      }
    }
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayableNode::CreateDefaultDisplayNodes()
{
  // does nothing by default
}

//---------------------------------------------------------------------------
int vtkMRMLDisplayableNode::GetDisplayVisibility()
{
  std::vector<vtkMRMLDisplayNode*>::iterator it =
    this->DisplayNodes.begin();
  if (it == this->DisplayNodes.end() ||
      !(*it))
    {
    return 0;
    }
  int visible = (*it)->GetVisibility();
  if (visible == 2)
    {
    return 2;
    }
  for ( ; it != this->DisplayNodes.end(); ++it)
    {
    vtkMRMLDisplayNode* displayNode = *it;
    if ( displayNode && displayNode->GetVisibility() != visible)
      {
      return 2;
      }
    }
  return visible;
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetDisplayVisibility(int visible)
{
  if (visible == 2)
    {
    return;
    }
  std::vector<vtkMRMLDisplayNode*>::iterator it;
  for (it = this->DisplayNodes.begin();
       it != this->DisplayNodes.end(); ++it)
    {
    vtkMRMLDisplayNode* displayNode = *it;
    if (displayNode)
      {
      displayNode->SetVisibility(visible);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayableNode::GetRASBounds(double bounds[6])
{
  vtkMath::UninitializeBounds(bounds);
}

