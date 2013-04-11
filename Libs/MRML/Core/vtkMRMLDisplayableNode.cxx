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
  this->DisplayNodeReferenceRole = 0;
  this->DisplayNodeReferenceRererenceMRMLAttributeName = 0;

  this->SetDisplayNodeReferenceRole("display");
  this->SetDisplayNodeReferenceRererenceMRMLAttributeName("displayNodeRef");

  this->AddNodeReferenceRole(this->GetDisplayNodeReferenceRole(),
                             this->GetDisplayNodeReferenceRererenceMRMLAttributeName() );
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode::~vtkMRMLDisplayableNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDisplayableNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  
  int numDisplayNodes = this->GetNumberOfNodeReferences(this->GetDisplayNodeReferenceRole());

  for (unsigned int i=0; i<numDisplayNodes; i++)
    {
    const char * id = this->GetNthNodeReferenceID(this->GetDisplayNodeReferenceRole(), i);

    os << indent << "DisplayNodeIDs[" << i << "]: " <<
     id << "\n";
    }
}


//----------------------------------------------------------------------------
const std::vector<vtkMRMLDisplayNode*>& vtkMRMLDisplayableNode::GetDisplayNodes()
{
  int numDisplayNodes = this->GetNumberOfNodeReferences(this->GetDisplayNodeReferenceRole());
  vtkMRMLDisplayNode *dnode;
  this->DisplayNodes.clear();
  for (int i=0; i<numDisplayNodes; i++)
    {
      dnode = vtkMRMLDisplayNode::SafeDownCast(
        this->GetNthNodeReference(this->GetDisplayNodeReferenceRole(), i));
      this->DisplayNodes.push_back(dnode);
    }

  return this->DisplayNodes;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLDisplayableNode::GetNthDisplayNode(int n)
{
  return vtkMRMLDisplayNode::SafeDownCast(
    this->GetNthNodeReference(this->GetDisplayNodeReferenceRole(), n));
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayableNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  int numDisplayNodes = this->GetNumberOfNodeReferences(this->GetDisplayNodeReferenceRole());
  for (int i=0; i<numDisplayNodes; i++)
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
  std::vector<vtkMRMLDisplayNode*> displayNodes = this->GetDisplayNodes();
  std::vector<vtkMRMLDisplayNode*>::iterator it = displayNodes.begin();
  if (it == displayNodes.end() ||
      !(*it))
    {
    return 0;
    }
  int visible = (*it)->GetVisibility();
  if (visible == 2)
    {
    return 2;
    }
  for ( ; it != displayNodes.end(); ++it)
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
  
  std::vector<vtkMRMLDisplayNode*> displayNodes = this->GetDisplayNodes();
  std::vector<vtkMRMLDisplayNode*>::iterator it;
  for (it = displayNodes.begin();
       it != displayNodes.end(); ++it)
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

