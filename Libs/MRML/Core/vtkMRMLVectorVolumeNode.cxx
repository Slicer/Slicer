/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include "vtkMRMLScene.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkImageExtractComponents.h"


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVectorVolumeNode);

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
vtkMRMLVectorVolumeDisplayNode* vtkMRMLVectorVolumeNode::GetVectorVolumeDisplayNode()
{
  return vtkMRMLVectorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLVectorVolumeNode::CreateDefaultStorageNode()
{
  return vtkMRMLVolumeArchetypeStorageNode::New();
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeNode::CreateDefaultDisplayNodes()
{
  if (vtkMRMLVectorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode())!=NULL)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==NULL)
    {
    vtkErrorMacro("vtkMRMLVectorVolumeNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkNew<vtkMRMLVectorVolumeDisplayNode> dispNode;
  this->GetScene()->AddNode(dispNode.GetPointer());
  dispNode->SetDefaultColorMap();
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}
