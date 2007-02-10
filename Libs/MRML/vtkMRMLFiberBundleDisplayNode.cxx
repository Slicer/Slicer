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
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFiberBundleDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::vtkMRMLFiberBundleDisplayNode()
{

  // Strings

  // Numbers
  this->FiberLineOpacity = 1;
  this->FiberTubeOpacity = 1;
  this->FiberGlyphOpacity = 1;

  this->FiberTubeRadius = 0.5;
  this->FiberTubeNumberOfSides = 4;

  // Enumerated
  this->ColorModeForFiberLines = this->colorModeSolid;
  this->ColorModeForFiberTubes = this->colorModeSolid;
  this->ColorModeForFiberGlyphs = this->colorModeScalar;

  // Booleans
  this->FiberLineVisibility = 1;
  this->FiberTubeVisibility = 0;
  this->FiberGlyphVisibility = 0;

  // Arrays


  // Objects

  // MRML nodes
  this->FiberLineDTDisplayPropertiesNode = NULL;
  this->FiberTubeDTDisplayPropertiesNode = NULL;
  this->FiberGlyphDTDisplayPropertiesNode = NULL;

  // MRML IDs (strings)
  this->FiberLineDTDisplayPropertiesNodeID = NULL;
  this->FiberTubeDTDisplayPropertiesNodeID = NULL;
  this->FiberGlyphDTDisplayPropertiesNodeID = NULL;

}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLFiberBundleDisplayNode::GetFiberLineDTDisplayPropertiesNode ( )
{
  vtkMRMLDiffusionTensorDisplayPropertiesNode* node = NULL;

  // Find the node corresponding to the ID we have saved.
  if  ( this->GetScene ( ) && this->GetFiberLineDTDisplayPropertiesNodeID ( ) )
    {
    vtkMRMLNode* cnode = this->GetScene ( ) -> GetNodeByID ( this->FiberLineDTDisplayPropertiesNodeID );
    node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast ( cnode );
    }

  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::SetAndObserveFiberLineDTDisplayPropertiesNodeID ( const char *ID )
{
  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro ( this->FiberLineDTDisplayPropertiesNode, NULL );

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetFiberLineDTDisplayPropertiesNodeID ( ID );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetFiberLineDTDisplayPropertiesNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->FiberLineDTDisplayPropertiesNode , cnode );

}


//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLFiberBundleDisplayNode::GetFiberTubeDTDisplayPropertiesNode ( )
{
  vtkMRMLDiffusionTensorDisplayPropertiesNode* node = NULL;

  // Find the node corresponding to the ID we have saved.
  if  ( this->GetScene ( ) && this->GetFiberTubeDTDisplayPropertiesNodeID ( ) )
    {
    vtkMRMLNode* cnode = this->GetScene ( ) -> GetNodeByID ( this->FiberTubeDTDisplayPropertiesNodeID );
    node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast ( cnode );
    }

  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::SetAndObserveFiberTubeDTDisplayPropertiesNodeID ( const char *ID )
{
  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro ( this->FiberTubeDTDisplayPropertiesNode, NULL );

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetFiberTubeDTDisplayPropertiesNodeID ( ID );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetFiberTubeDTDisplayPropertiesNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->FiberTubeDTDisplayPropertiesNode , cnode );

}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLFiberBundleDisplayNode::GetFiberGlyphDTDisplayPropertiesNode ( )
{
  vtkMRMLDiffusionTensorDisplayPropertiesNode* node = NULL;

  // Find the node corresponding to the ID we have saved.
  if  ( this->GetScene ( ) && this->GetFiberGlyphDTDisplayPropertiesNodeID ( ) )
    {
    vtkMRMLNode* cnode = this->GetScene ( ) -> GetNodeByID ( this->FiberGlyphDTDisplayPropertiesNodeID );
    node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast ( cnode );
    }

  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::SetAndObserveFiberGlyphDTDisplayPropertiesNodeID ( const char *ID )
{
  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro ( this->FiberGlyphDTDisplayPropertiesNode, NULL );

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetFiberGlyphDTDisplayPropertiesNodeID ( ID );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetFiberGlyphDTDisplayPropertiesNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->FiberGlyphDTDisplayPropertiesNode , cnode );

}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::~vtkMRMLFiberBundleDisplayNode()
{

  //this->SetAndObserveColorNodeID( NULL);

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

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

    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFiberBundleDisplayNode *node = (vtkMRMLFiberBundleDisplayNode *) anode;

  // Strings
  //this->SetColor(node->Name);

  // Vectors

  
  // Numbers


}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);

}

//---------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  return;
}
