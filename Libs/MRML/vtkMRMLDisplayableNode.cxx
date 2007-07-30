/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDisplayableNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLScene.h"

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode::vtkMRMLDisplayableNode()
{
  this->DisplayNodeID = NULL;
  this->DisplayNode = NULL;
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

  if (this->DisplayNodeID != NULL) 
    {
    of << indent << "displayNodeRef=\"" << this->DisplayNodeID << "\" ";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->DisplayNodeID && !strcmp(oldID, this->DisplayNodeID))
    {
    this->SetDisplayNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "displayNodeRef")) 
      {
      this->SetDisplayNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->DisplayNodeID, this);
      }    
    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDisplayableNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLDisplayableNode *node = (vtkMRMLDisplayableNode *) anode;

  this->SetDisplayNodeID(node->DisplayNodeID);
  if (node->PolyData)
    {
    this->SetPolyData(node->PolyData);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "DisplayNodeID: " <<
    (this->DisplayNodeID ? this->DisplayNodeID : "(none)") << "\n";

  os << "\nPoly Data:\n";
  if (this->PolyData) 
    {
    this->PolyData->PrintSelf(os, indent.GetNextIndent());
    }
}

//-----------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
   this->SetAndObservePolyData(this->GetPolyData());
   this->SetAndObserveDisplayNodeID(this->GetDisplayNodeID());

}

//-----------------------------------------------------------
void vtkMRMLDisplayableNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->DisplayNodeID != NULL && this->Scene->GetNodeByID(this->DisplayNodeID) == NULL)
    {
    this->SetAndObserveDisplayNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLDisplayableNode::GetDisplayNode()
{
  vtkMRMLDisplayNode* node = NULL;
  if (this->GetScene() && this->GetDisplayNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->DisplayNodeID);
    node = vtkMRMLDisplayNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->DisplayNode, NULL);

  this->SetDisplayNodeID(displayNodeID);

  vtkMRMLDisplayNode *dnode = this->GetDisplayNode();

  vtkSetAndObserveMRMLObjectMacro(this->DisplayNode, dnode);

}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableNode::SetAndObservePolyData(vtkPolyData *polyData)
{
if (this->PolyData != NULL)
    {
    this->PolyData->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  unsigned long mtime1, mtime2;
  mtime1 = this->GetMTime();
  this->SetPolyData(polyData);
  mtime2 = this->GetMTime();

  if (this->PolyData != NULL)
    {
    this->PolyData->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
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

  vtkMRMLDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkMRMLDisplayableNode::DisplayModifiedEvent, NULL);
    }
  else if (this->PolyData == vtkPolyData::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    this->ModifiedSinceRead = true;
    this->InvokeEvent(vtkMRMLDisplayableNode::PolyDataModifiedEvent, NULL);
    }
  return;
}

