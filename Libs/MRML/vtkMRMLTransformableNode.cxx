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

#include "vtkMRMLTransformableNode.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLScene.h"


//----------------------------------------------------------------------------
vtkMRMLTransformableNode::vtkMRMLTransformableNode()
{
  this->TransformNodeID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLTransformableNode::~vtkMRMLTransformableNode()
{
  if (this->TransformNodeID) 
    {
    SetAndObserveTransformNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformableNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->TransformNodeID != NULL) 
    {
    of << indent << "transformNodeRef=\"" << this->TransformNodeID << "\" ";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformableNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "transformNodeRef")) 
      {
      this->SetAndObserveTransformNodeID(attValue);
      }
    }  
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTransformableNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLTransformNode *node = (vtkMRMLTransformNode *) anode;
  this->SetTransformNodeID(node->TransformNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "TransformNodeID: " <<
    (this->TransformNodeID ? this->TransformNodeID : "(none)") << "\n";
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLTransformableNode::GetParentTransformNode()
{
  vtkMRMLTransformNode* node = NULL;
  if (this->GetScene() && this->TransformNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->TransformNodeID);
    node = vtkMRMLTransformNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformableNode::SetAndObserveTransformNodeID(const char *transformNodeID)
{
  if (this->TransformNodeID != NULL)
    {
    vtkMRMLTransformNode *tnode = this->GetParentTransformNode();
    if (tnode != NULL)
      {
      tnode->RemoveObservers ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
      this->SetTransformNodeID(NULL);
      }
    }
  this->SetTransformNodeID(transformNodeID);
  vtkMRMLTransformNode *tnode = this->GetParentTransformNode();
  if (tnode != NULL) 
    {
      tnode->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkMRMLTransformableNode::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
  vtkMRMLTransformNode *tnode = this->GetParentTransformNode();
  if (tnode != NULL && tnode == vtkMRMLTransformNode::SafeDownCast(caller) &&
      event ==  vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    //TODO don't send even on the scene but rather have vtkSlicerSliceLayerLogic listen to
    // TransformModifiedEvent
    //this->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    this->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent, NULL);
    }
}

//-----------------------------------------------------------
void vtkMRMLTransformableNode::UpdateScene(vtkMRMLScene *scene)
{
}

//-----------------------------------------------------------
void vtkMRMLTransformableNode::UpdateReferences()
{
  if (this->TransformNodeID != NULL && this->Scene->GetNodeByID(this->TransformNodeID) == NULL)
    {
    this->SetAndObserveTransformNodeID(NULL);
    }
}
// End
