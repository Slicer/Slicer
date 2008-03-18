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
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
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
  this->Order = 2; //Second order Tensor
}
//----------------------------------------------------------------------------

void vtkMRMLDiffusionTensorVolumeNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  if (this->GetScene() && vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetScene()->GetNodeByID(displayNodeID))!=NULL)
  {
    Superclass::SetAndObserveDisplayNodeID(displayNodeID);

  } else {
    vtkErrorMacro("The node to display can not display diffusion tensors");
  }
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeNode::~vtkMRMLDiffusionTensorVolumeNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
 
  vtkIndent indent(nIndent);

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
void vtkMRMLDiffusionTensorVolumeNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID,newID);
}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::UpdateReferences()
{
  Superclass::UpdateReferences();
}

//---------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
std::vector< vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> vtkMRMLDiffusionTensorVolumeNode::GetSliceGlyphDisplayNodes()
{
  std::vector< vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> nodes;
  int nnodes = this->GetNumberOfDisplayNodes();
  vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node = NULL;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node) 
      {
      nodes.push_back(node);
      }
    }
  return nodes;
}

//----------------------------------------------------------------------------
std::vector< vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> vtkMRMLDiffusionTensorVolumeNode::AddSliceGlyphDisplayNodes()
{
  std::vector< vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> nodes = this->GetSliceGlyphDisplayNodes();
  if (nodes.size() == 0)
    {
    for (int i=0; i<3; i++)
      {
      vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node = vtkMRMLDiffusionTensorVolumeSliceDisplayNode::New();
      if (this->GetScene())
        {
        this->GetScene()->AddNode(node);
        node->Delete();

        vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
        this->GetScene()->AddNode(glyphDTDPN);
        node->SetAndObserveDTDisplayPropertiesNodeID(glyphDTDPN->GetID());
        glyphDTDPN->Delete();
        node->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");

        this->AddAndObserveDisplayNodeID(node->GetID());
        
        if (i == 0) 
          {
          node->SetName("Red");
          }
        else if (i == 1) 
          {
          node->SetName("Yellow");
          }
        else if (i == 2) 
          {
          node->SetName("Green");
          }
        
        nodes.push_back(node);
        }
      }
    }
  return nodes;
}

 
