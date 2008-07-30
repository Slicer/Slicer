/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleDisplayNode::New()
{
  return NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFiberBundleDisplayNode::CreateNodeInstance()
{
  return NULL;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::vtkMRMLFiberBundleDisplayNode()
{
  this->BackfaceCulling = 0;

  // Enumerated
  this->ColorMode = this->colorModeSolid;

  this->DiffusionTensorDisplayPropertiesNode = NULL;
  this->DiffusionTensorDisplayPropertiesNodeID = NULL;

}



//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::~vtkMRMLFiberBundleDisplayNode()
{
  this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " colorMode =\"" << this->ColorMode << "\"";

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL) 
    {
    of << indent << " DiffusionTensorDisplayPropertiesNodeRef=\"" << this->DiffusionTensorDisplayPropertiesNodeID << "\"";
    }
}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "colorMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ColorMode;
      }

    else if (!strcmp(attName, "DiffusionTensorDisplayPropertiesNodeRef")) 
      {
      this->SetDiffusionTensorDisplayPropertiesNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->FiberLineDiffusionTensorDisplayPropertiesNodeID, this);
      }
    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFiberBundleDisplayNode *node = (vtkMRMLFiberBundleDisplayNode *) anode;

  this->SetColorMode(node->ColorMode);
  this->SetDiffusionTensorDisplayPropertiesNodeID(node->DiffusionTensorDisplayPropertiesNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
  os << indent << "ColorMode:             " << this->ColorMode << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(this->GetDiffusionTensorDisplayPropertiesNodeID());
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL && this->Scene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID) == NULL)
    {
    this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
    }
}


//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->DiffusionTensorDisplayPropertiesNodeID && !strcmp(oldID, this->DiffusionTensorDisplayPropertiesNodeID))
    {
    this->SetDiffusionTensorDisplayPropertiesNodeID(newID);
    }
}



//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLFiberBundleDisplayNode::GetDiffusionTensorDisplayPropertiesNode ( )
{
  vtkMRMLDiffusionTensorDisplayPropertiesNode* node = NULL;

  // Find the node corresponding to the ID we have saved.
  if  ( this->GetScene ( ) && this->GetDiffusionTensorDisplayPropertiesNodeID ( ) )
    {
    vtkMRMLNode* cnode = this->GetScene ( ) -> GetNodeByID ( this->DiffusionTensorDisplayPropertiesNodeID );
    node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast ( cnode );
    }

  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *ID )
{
  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode, NULL );

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetDiffusionTensorDisplayPropertiesNodeID ( ID );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetDiffusionTensorDisplayPropertiesNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode , cnode );

}

