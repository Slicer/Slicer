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
#include "vtkEventBroker.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLScene.h"

// when change the display node, update the scalars
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkPolyData.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkMRMLDisplayableNode, PolyData, vtkPolyData)

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode::vtkMRMLDisplayableNode()
{
  this->PolyData = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode::~vtkMRMLDisplayableNode()
{
  this->SetAndObserveDisplayNodeID( NULL);

  this->SetAndObservePolyData(NULL);
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
        this->AddDisplayNodeID(id.c_str());
        }

      //this->Scene->AddReferencedNodeID(this->DisplayNodeID, this);
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  bool modified = false;
  for (unsigned int i=0; i<this->DisplayNodeIDs.size(); i++)
    {
    if ( std::string(oldID) == this->DisplayNodeIDs[i])
      {
      this->SetNthDisplayNodeID(i, newID);
      modified = true;
      }
    }
  if (modified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDisplayableNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLDisplayableNode *node = (vtkMRMLDisplayableNode *) anode;
  this->SetAndObserveDisplayNodeID(NULL);

  int ndnodes = node->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    this->SetAndObserveNthDisplayNodeID(i, node->DisplayNodeIDs[i].c_str());
    }

  if (node->PolyData)
    {
    this->SetPolyData(node->PolyData);
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
      this->DisplayNodeIDs[i] << "\n";
    }
  os << indent << "\nPoly Data:\n";
  if (this->PolyData)
    {
    this->PolyData->PrintSelf(os, indent.GetNextIndent());
    }
}

//-----------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  // TBD: why do we remove the node ids ?
  for (unsigned int i=0; i<this->DisplayNodes.size(); i++)
    {
    this->SetAndObserveNthDisplayNodeID(i, NULL);

    //if (this->DisplayNodes[i])
    //  {
    //  this->DisplayNodes[i]->Delete();
    // }
    }
  this->DisplayNodes.clear();

  for (unsigned int i=0; i<this->DisplayNodeIDs.size(); i++)
    {
    vtkMRMLDisplayNode *pnode = 0;
    this->DisplayNodes.push_back(pnode);
    this->SetAndObserveNthDisplayNodeID(i, this->DisplayNodeIDs[i].c_str());
    // When calling UpdateScene, the scene is up-to-date (it is done to be
    // imported/restored). All the nodes are in it.
    // Therefor, there is no reason why there wouldn't be a node
    // for each node id.
    // If you have an assert here, check in the scene xml if
    // DisplayNodeIDs[i] really exists.
    assert(this->DisplayNodes[i]);
    }
}

//-----------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateReferences()
{
   Superclass::UpdateReferences();
  for (unsigned int i=0; i<this->DisplayNodeIDs.size(); i++)
    {
    if (this->Scene->GetNodeByID(this->DisplayNodeIDs[i]) == NULL)
      {
      this->SetAndObserveNthDisplayNodeID(i, NULL);
      }
    }
}

//----------------------------------------------------------------------------
std::vector<vtkMRMLDisplayNode*> vtkMRMLDisplayableNode::GetDisplayNodes()
{
  return this->DisplayNodes;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLDisplayableNode::GetNthDisplayNode(int n)
{
  vtkMRMLDisplayNode* node = NULL;
  if (this->GetScene())
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->GetNthDisplayNodeID(n));
    node = vtkMRMLDisplayNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetDisplayNodeID(const char *displayNodeID)
{
  if (this->DisplayNodeIDs.empty() && displayNodeID == NULL)
    {
    return;
    }
  if (this->DisplayNodeIDs.size() == 1 && displayNodeID != NULL && this->DisplayNodeIDs[0] == std::string(displayNodeID) )
    {
    return;
    }
  this->DisplayNodeIDs.clear();
  if (displayNodeID != NULL)
    {
    this->DisplayNodeIDs.push_back(std::string(displayNodeID));
    }
  if (displayNodeID)
    {
    this->Scene->AddReferencedNodeID(displayNodeID, this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetNthDisplayNodeID(int n, const char *displayNodeID)
{
  if (this->DisplayNodeIDs.empty() && displayNodeID == NULL)
    {
    return;
    }
  if ((int)this->DisplayNodeIDs.size() <= n)
    {
    return;
    }
  if (displayNodeID != NULL && this->DisplayNodeIDs[n] == std::string(displayNodeID) )
    {
    return;
    }
  if (displayNodeID != NULL)
    {
    this->DisplayNodeIDs[n] = std::string(displayNodeID);
    }
  else
    {
    this->DisplayNodeIDs.erase(this->DisplayNodeIDs.begin()+n);
    }
  if (displayNodeID)
    {
    this->Scene->AddReferencedNodeID(displayNodeID, this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::AddDisplayNodeID(const char *displayNodeID)
{
  if (displayNodeID == NULL)
    {
    return;
    }

  this->DisplayNodeIDs.push_back(std::string(displayNodeID));
  this->Scene->AddReferencedNodeID(displayNodeID, this);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  for (unsigned int i=0; i<this->DisplayNodes.size(); i++)
    {
    if (this->DisplayNodes[i])
      {
      vtkSetAndObserveMRMLObjectMacro(this->DisplayNodes[i], NULL);
      }
    }
  this->DisplayNodes.clear();

  this->SetDisplayNodeID(displayNodeID);

  vtkMRMLDisplayNode *dnode = this->GetDisplayNode();
  this->AddAndObserveDisplayNode(dnode);

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetAndObserveNthDisplayNodeID(int n, const char *displayNodeID)
{
  if (n >= (int)this->DisplayNodes.size())
    {
    this->AddAndObserveDisplayNodeID(displayNodeID);
    return;
    }
  vtkSetAndObserveMRMLObjectMacro(this->DisplayNodes[n], NULL);

  this->SetNthDisplayNodeID(n, displayNodeID);

  vtkMRMLDisplayNode *dnode = this->GetNthDisplayNode(n);
  if (dnode)
    {
    vtkSetAndObserveMRMLObjectMacro(this->DisplayNodes[n], dnode);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::AddAndObserveDisplayNodeID(const char *displayNodeID)
{
  this->AddDisplayNodeID(displayNodeID);

  vtkMRMLDisplayNode *dnode = vtkMRMLDisplayNode::SafeDownCast(
    this->GetScene() != 0 ? this->GetScene()->GetNodeByID(displayNodeID) : 0);
  this->AddAndObserveDisplayNode(dnode);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::AddAndObserveDisplayNode(vtkMRMLDisplayNode *dnode)
{
  if (dnode)
    {
    vtkMRMLDisplayNode *pnode = 0;
    vtkSetAndObserveMRMLObjectMacro(pnode, dnode);
    this->DisplayNodes.push_back(pnode);
    this->OnDisplayNodeAdded(pnode);
    //pnode->Delete();
    /*
    if (this->IsA("vtkMRMLVolumeNode"))
      {
      // set up the display node
      vtkDebugMacro("AddAndObserveDisplayNode: " << (this->GetID() == NULL ? "null self id" : this->GetID()) << ": a display node was added " << (dnode->GetID() == NULL ? "null disp node id" : dnode->GetID()) << ", have a volume node, setting up the display node");
      if (pnode->IsA("vtkMRMLVolumeDisplayNode"))
        {
        vtkDebugMacro("AddAndObserveDisplayNode: have a volume display node, calc auto levels");
        vtkMRMLVolumeNode::SafeDownCast(this)->CalculateAutoLevels(vtkMRMLVolumeDisplayNode::SafeDownCast(pnode));
        }
      }
    */
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::OnDisplayNodeAdded(vtkMRMLDisplayNode* vtkNotUsed(dNode))
{
  this->InvokeEvent(vtkMRMLDisplayableNode::DisplayModifiedEvent, NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetAndObservePolyData(vtkPolyData *polyData)
{
  if (this->PolyData != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations (
      this->PolyData, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  unsigned long mtime1, mtime2;
  mtime1 = this->GetMTime();
  this->SetPolyData(polyData);
  mtime2 = this->GetMTime();

  if (this->PolyData != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      this->PolyData, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  if (mtime1 != mtime2)
    {
    this->InvokeEvent( vtkMRMLDisplayableNode::PolyDataModifiedEvent , this);
    }
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
  if (this->PolyData && this->PolyData == vtkPolyData::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    this->ModifiedSinceRead = true;
    this->InvokeEvent(vtkMRMLDisplayableNode::PolyDataModifiedEvent, NULL);
    }
  return;
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

