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
vtkMRMLVectorVolumeNode::vtkMRMLVectorVolumeNode() = default;

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeNode::~vtkMRMLVectorVolumeNode() = default;

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
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLVolumeArchetypeStorageNode"));
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeNode::CreateDefaultDisplayNodes()
{
  if (vtkMRMLVectorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode())!=nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==nullptr)
    {
    vtkErrorMacro("vtkMRMLVectorVolumeNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkMRMLVectorVolumeDisplayNode* dispNode = vtkMRMLVectorVolumeDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkMRMLVectorVolumeDisplayNode") );
  dispNode->SetDefaultColorMap();
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}
