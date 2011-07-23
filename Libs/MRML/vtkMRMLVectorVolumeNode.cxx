/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

#include "vtkObjectFactory.h"
#include "vtkImageExtractComponents.h"


//------------------------------------------------------------------------------
vtkMRMLVectorVolumeNode* vtkMRMLVectorVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVectorVolumeNode");
  if(ret)
    {
    return (vtkMRMLVectorVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVectorVolumeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVectorVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVectorVolumeNode");
  if(ret)
    {
    return (vtkMRMLVectorVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVectorVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeNode::vtkMRMLVectorVolumeNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeNode::~vtkMRMLVectorVolumeNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVectorVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLVectorVolumeNode *node = (vtkMRMLVectorVolumeNode *) anode;
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}


 
//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeNode::CalculateAutoLevels(vtkMRMLScalarVolumeDisplayNode *refNode, vtkImageData *refData)
{
  if (!refNode && !this->GetDisplayNode())
    {
    vtkDebugMacro("CalculateAutoLevels: input display node is null, and cannot get local display node");
    return;
    }

  vtkMRMLVectorVolumeDisplayNode *displayNode;
  if (refNode == NULL)
    {
    displayNode = this->GetVectorVolumeDisplayNode();
    }
  else
    {
    displayNode = vtkMRMLVectorVolumeDisplayNode::SafeDownCast(refNode);
    }

  if (displayNode == NULL)
    {
    vtkWarningMacro("CalculateAutoLevels: unable to get a vector volume display node.");
    return;
    }

  if (!displayNode->GetAutoWindowLevel())
    {
    vtkDebugMacro("CalculateAutoLevels: " << (this->GetID() == NULL ? "nullid" : this->GetID()) << ": Auto window level not turned on, returning.");
    return;
    }
    
  vtkImageData *imageDataScalar;
  if (refData == NULL)
    {
    imageDataScalar = this->GetImageData();
    }
  else
    {
    imageDataScalar = refData;
    }

  if ( !imageDataScalar )
    {
    vtkDebugMacro("CalculateAutoLevels: image data is null");
    return;
    }

  if (displayNode != NULL ) 
    {
    imageDataScalar = displayNode->GetExtractIntensity()->GetOutput();
    }

  if (imageDataScalar != NULL)
    {
    // pass it up to the superclass
    this->CalculateScalarAutoLevels(displayNode, imageDataScalar);
    }
}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeDisplayNode* vtkMRMLVectorVolumeNode::GetVectorVolumeDisplayNode()
{
  return vtkMRMLVectorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLVectorVolumeNode::CreateDefaultStorageNode()
{
  return vtkMRMLVolumeArchetypeStorageNode::New();
}
