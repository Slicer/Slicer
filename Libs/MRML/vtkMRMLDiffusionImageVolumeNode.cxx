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

#include "vtkMRMLDiffusionImageVolumeNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLDiffusionImageVolumeNode* vtkMRMLDiffusionImageVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionImageVolumeNode");
  if(ret)
    {
    return (vtkMRMLDiffusionImageVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionImageVolumeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLDiffusionImageVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionImageVolumeNode");
  if(ret)
    {
    return (vtkMRMLDiffusionImageVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionImageVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionImageVolumeNode::vtkMRMLDiffusionImageVolumeNode()
{
  this->BaselineNodeID = NULL;
  this->MaskNodeID = NULL;
  this->DiffusionWeightedNodeID = NULL;
  //Pair of ID-pointer for observing the corresponding Display node.
  //this->DisplayNodeID = NULL;
  this->Order = 2; //Second order Tensor
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionImageVolumeNode::~vtkMRMLDiffusionImageVolumeNode()
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
void vtkMRMLDiffusionImageVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
 
  vtkIndent indent(nIndent);
  std::stringstream ss;
  if (this->BaselineNodeID != NULL) 
    {
    of << indent << "baselineNodeRef=\"" << this->BaselineNodeID << "\"";
    }
  if (this->DiffusionWeightedNodeID != NULL) 
    {
    of << indent << "diffusionWeightedNodeRef=\"" << this->DiffusionWeightedNodeID << "\"";
    }
  if (this->MaskNodeID != NULL) 
    {
    of << indent << "maskNodeRef=\"" << this->MaskNodeID << "\"";
    }

}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionImageVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    std::stringstream ss;
    ss<<attValue;

    if (!strcmp(attName, "baselineNodeRef"))
      {
        ss>>this->BaselineNodeID;
      }
    if (!strcmp(attName, "diffusionWeightedNodeRef"))
      {
        ss>>this->DiffusionWeightedNodeID;
      }
    if (!strcmp(attName, "maskNodeRef"))
      {
        ss>>this->MaskNodeID;
      }
  }      

} 


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDiffusionImageVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLDiffusionImageVolumeNode *node = (vtkMRMLDiffusionImageVolumeNode *) anode;

}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLDiffusionImageVolumeNode::GetBaselineNode()
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
vtkMRMLVolumeNode* vtkMRMLDiffusionImageVolumeNode::GetMaskNode()
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
vtkMRMLDiffusionWeightedVolumeNode* vtkMRMLDiffusionImageVolumeNode::GetDiffusionWeightedNode()
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
//  vtkMRMLDiffusionImageVolumeDisplayNode* node = NULL;
//  if (this->GetScene() && this->GetDisplayNodeID() )
//    {
//    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->DisplayNodeID);
//    node = vtkMRMLDiffusionImageVolumeDisplayNode::SafeDownCast(snode);
//    }
//  return node;
//}


//-----------------------------------------------------------
//void vtkMRMLDiffusionImageVolumeNode::UpdateScene(vtkMRMLScene *scene)
//{
//  Superclass::UpdateScene(scene);

//  if (this->GetDiffusionWeightedNodeID()) 
//    {
//    this->SetAndObserveDisplayNodeID(this->GetDiffusionWeightedNodeID());
//    }
//}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionImageVolumeNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->BaselineNodeID && !strcmp(oldID, this->BaselineNodeID))
    {
    this->SetBaselineNodeID(newID);
    }
  if (this->MaskNodeID && !strcmp(oldID, this->MaskNodeID))
    {
    this->SetMaskNodeID(newID);
    }
  if (this->DiffusionWeightedNodeID && !strcmp(oldID, this->DiffusionWeightedNodeID))
    {
    this->SetDiffusionWeightedNodeID(newID);
    }
  Superclass::UpdateReferenceID(oldID,newID);
}

//-----------------------------------------------------------
void vtkMRMLDiffusionImageVolumeNode::UpdateReferences()
{
  Superclass::UpdateReferences();

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
void vtkMRMLDiffusionImageVolumeNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionImageVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  
  os << indent << "DiffusionWeightedNodeID: " <<
    (this->DiffusionWeightedNodeID ? this->DiffusionWeightedNodeID : "(none)") << "\n";

  os << indent << "BaselineNodeID: " <<
    (this->BaselineNodeID ? this->BaselineNodeID : "(none)") << "\n";

  os << indent << "MaskNodeID: " <<
    (this->MaskNodeID ? this->MaskNodeID : "(none)") << "\n";

}


 
