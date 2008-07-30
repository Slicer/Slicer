/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

//------------------------------------------------------------------------------
vtkMRMLFiberBundleNode* vtkMRMLFiberBundleNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFiberBundleNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleNode;
}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::GetLineDisplayNode()
{
  int nnodes = this->GetNumberOfDisplayNodes();
  vtkMRMLFiberBundleLineDisplayNode *node = NULL;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLFiberBundleLineDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node) 
      {
      break;
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::GetTubeDisplayNode()
{
  int nnodes = this->GetNumberOfDisplayNodes();
  vtkMRMLFiberBundleTubeDisplayNode *node = NULL;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLFiberBundleTubeDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node) 
      {
      break;
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::GetGlyphDisplayNode()
{
  int nnodes = this->GetNumberOfDisplayNodes();
  vtkMRMLFiberBundleGlyphDisplayNode *node = NULL;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLFiberBundleGlyphDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node) 
      {
      break;
      }
    }
  return node;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::AddLineDisplayNode()
{
  vtkMRMLFiberBundleDisplayNode *node = this->GetLineDisplayNode();
  if (node == NULL)
    {
    node = vtkMRMLFiberBundleLineDisplayNode::New();
    if (this->GetScene())
      {
      this->GetScene()->AddNode(node);
      node->Delete();

      vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
      this->GetScene()->AddNode(glyphDTDPN);
      node->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(glyphDTDPN->GetID());
      node->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
      glyphDTDPN->Delete();

      this->AddAndObserveDisplayNodeID(node->GetID());
      node->SetPolyData(this->GetPolyData());
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::AddTubeDisplayNode()
{
  vtkMRMLFiberBundleDisplayNode *node = this->GetTubeDisplayNode();
  if (node == NULL)
    {
    node = vtkMRMLFiberBundleTubeDisplayNode::New();
    if (this->GetScene())
      {
      this->GetScene()->AddNode(node);
      node->Delete();

      vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
      this->GetScene()->AddNode(glyphDTDPN);
      node->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(glyphDTDPN->GetID());
      glyphDTDPN->Delete();
      node->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
      
      this->AddAndObserveDisplayNodeID(node->GetID());
      node->SetPolyData(this->GetPolyData());
      }
    }
  return node;
}
//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::AddGlyphDisplayNode()
{
  vtkMRMLFiberBundleDisplayNode *node = this->GetGlyphDisplayNode();
  if (node == NULL)
    {
    node = vtkMRMLFiberBundleGlyphDisplayNode::New();
    if (this->GetScene())
      {
      this->GetScene()->AddNode(node);
      node->Delete();

      vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
      this->GetScene()->AddNode(glyphDTDPN);
      node->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(glyphDTDPN->GetID());
      glyphDTDPN->Delete();
      node->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");

      this->AddAndObserveDisplayNodeID(node->GetID());
      node->SetPolyData(this->GetPolyData());
      }
    }
  return node;
}
