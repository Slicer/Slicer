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

const std::string vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_ROLE = "display";
const std::string vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_MRML_ATTRIBUTE_NAME = "displayNodeRef";

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode::vtkMRMLDisplayableNode()
{
  this->AddNodeReferenceRole(vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_ROLE.c_str(),
                             vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_MRML_ATTRIBUTE_NAME.c_str() );
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
  
  int numDisplayNodes = this->GetNumberOfNodeReferences(vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_ROLE.c_str());

  for (unsigned int i=0; i<numDisplayNodes; i++)
    {
    const char * id = this->GetNthNodeReferenceID(vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_ROLE.c_str(), i);

    os << indent << "DisplayNodeIDs[" << i << "]: " <<
     id << "\n";
    }
}


//----------------------------------------------------------------------------
const std::vector<vtkMRMLDisplayNode*>& vtkMRMLDisplayableNode::GetDisplayNodes()
{
  int numDisplayNodes = this->GetNumberOfNodeReferences(vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_ROLE.c_str());
  vtkMRMLDisplayNode *dnode;
  this->DisplayNodes.clear();
  for (int i=0; i<numDisplayNodes; i++)
    {
      dnode = vtkMRMLDisplayNode::SafeDownCast(
        this->GetNthNodeReference(vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_ROLE.c_str(), i));
      this->DisplayNodes.push_back(dnode);
    }

  return this->DisplayNodes;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLDisplayableNode::GetNthDisplayNode(int n)
{
  return vtkMRMLDisplayNode::SafeDownCast(
    this->GetNthNodeReference(vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_ROLE.c_str(), n));
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayableNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  int numDisplayNodes = this->GetNumberOfNodeReferences(vtkMRMLDisplayableNode::DISPLAY_NODE_REFERENCE_ROLE.c_str());
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

