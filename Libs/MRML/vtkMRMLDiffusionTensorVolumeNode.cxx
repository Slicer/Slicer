/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeNode* vtkMRMLDiffusionTensorVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorVolumeNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorVolumeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLDiffusionTensorVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorVolumeNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeNode::vtkMRMLDiffusionTensorVolumeNode()
{
  this->BaselineNodeID = NULL;
  this->MaskNodeID = NULL;
  this->DiffusionWeightedNodeID = NULL;
  //Pair of ID-pointer for observing the corresponding Display node.
  this->DisplayNodeID = NULL;
  this->DiffusionTensorVolumeDisplayNode = NULL;
  this->Order = 2; //Second order Tensor
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeNode::~vtkMRMLDiffusionTensorVolumeNode()
{

  if (this->BaselineNodeID)
    {
    delete [] this->BaselineNodeID;
    this->BaselineNodeID = NULL;
    }
  if (this->MaskNodeID)
    {
    delete [] this->MaskNodeID;
    this->MaskNodeID = NULL;
    }

  if (this->DiffusionWeightedNodeID)
    {
    delete [] this->DiffusionWeightedNodeID;
    this->DiffusionWeightedNodeID = NULL;
    }
   this->SetAndObserveDisplayNodeID(NULL); 
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
 
  vtkIndent indent(nIndent);
  std::stringstream ss;
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "measurementFrame"))
      {
      }
  }      

} 


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDiffusionTensorVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLDiffusionTensorVolumeNode *node = (vtkMRMLDiffusionTensorVolumeNode *) anode;

}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLDiffusionTensorVolumeNode::GetBaselineNode()
{
  vtkMRMLVolumeNode* node = NULL;
  if (this->GetScene() && this->GetBaselineNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->BaselineNodeID);
    node = vtkMRMLVolumeNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLDiffusionTensorVolumeNode::GetMaskNode()
{
  vtkMRMLVolumeNode* node = NULL;
  if (this->GetScene() && this->GetMaskNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->MaskNodeID);
    node = vtkMRMLVolumeNode::SafeDownCast(snode);
    }
  return node;
}


//----------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeNode* vtkMRMLDiffusionTensorVolumeNode::GetDiffusionWeightedNode()
{
  vtkMRMLDiffusionWeightedVolumeNode* node = NULL;
  if (this->GetScene() && this->GetDiffusionWeightedNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->DiffusionWeightedNodeID);
    node = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
//vtkMRMLVolumeDisplayNode* vtkMRMLVolumeNode::GetDisplayNode()
//{
//  vtkMRMLDiffusionTensorVolumeDisplayNode* node = NULL;
//  if (this->GetScene() && this->GetDisplayNodeID() )
//    {
//    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->DisplayNodeID);
//    node = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(snode);
//    }
//  return node;
//}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->DiffusionTensorVolumeDisplayNode, NULL);

  this->SetDisplayNodeID(displayNodeID);

  vtkMRMLDiffusionTensorVolumeDisplayNode *dnode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast( this->GetDisplayNode());

  vtkSetAndObserveMRMLObjectMacro(this->DiffusionTensorVolumeDisplayNode, dnode);
}

//-----------------------------------------------------------
//void vtkMRMLDiffusionTensorVolumeNode::UpdateScene(vtkMRMLScene *scene)
//{
//  Superclass::UpdateScene(scene);

//  if (this->GetDiffusionWeightedNodeID()) 
//    {
//    this->SetAndObserveDisplayNodeID(this->GetDiffusionWeightedNodeID());
//    }
//}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->DisplayNodeID != NULL && this->Scene->GetNodeByID(this->DisplayNodeID) == NULL)
    {
    this->SetAndObserveDisplayNodeID(NULL);
    }
if (this->BaselineNodeID != NULL && this->Scene->GetNodeByID(this->BaselineNodeID) == NULL)
    {
    this->SetBaselineNodeID(NULL);
    }
if (this->MaskNodeID != NULL && this->Scene->GetNodeByID(this->MaskNodeID) == NULL)
    {
    this->SetMaskNodeID(NULL);
    }
if (this->DiffusionWeightedNodeID != NULL && this->Scene->GetNodeByID(this->DiffusionWeightedNodeID) == NULL)
    {
    this->SetDiffusionWeightedNodeID(NULL);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

// I don't know how to deal with this.
// Should we create a new event when DWI are modified, so the tensor can be updated?.

  vtkMRMLDiffusionTensorVolumeDisplayNode *dnode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
  if (dnode != NULL && dnode == vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  
  os << indent << "DiffusionWeightedNodeID: " <<
    (this->DiffusionWeightedNodeID ? this->DiffusionWeightedNodeID : "(none)") << "\n";

  os << indent << "BaselineNodeID: " <<
    (this->BaselineNodeID ? this->BaselineNodeID : "(none)") << "\n";

  os << indent << "MaskNodeID: " <<
    (this->MaskNodeID ? this->MaskNodeID : "(none)") << "\n";

}


 
