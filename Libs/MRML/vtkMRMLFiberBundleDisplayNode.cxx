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
  this->FiberTubeNumberOfSides = 6;

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
vtkMRMLFiberBundleDisplayNode::~vtkMRMLFiberBundleDisplayNode()
{
  this->SetAndObserveFiberLineDTDisplayPropertiesNodeID(NULL);
  this->SetAndObserveFiberTubeDTDisplayPropertiesNodeID(NULL);
  this->SetAndObserveFiberGlyphDTDisplayPropertiesNodeID(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " fiberLineOpacity =\"" << this->FiberLineOpacity << "\"";
  of << indent << " fiberTubeOpacity =\"" << this->FiberTubeOpacity << "\"";
  of << indent << " fiberGlyphOpacity =\"" << this->FiberGlyphOpacity << "\"";

  of << indent << " fiberTubeRadius =\"" << this->FiberTubeRadius << "\"";
  of << indent << " fiberTubeNumberOfSides =\"" << this->FiberTubeNumberOfSides << "\"";

  of << indent << " colorModeForFiberLines =\"" << this->ColorModeForFiberLines << "\"";
  of << indent << " colorModeForFiberTubes =\"" << this->ColorModeForFiberTubes << "\"";
  of << indent << " colorModeForFiberGlyphs =\"" << this->ColorModeForFiberGlyphs << "\"";

  of << indent << " fiberLineVisibility=\"" << (this->FiberLineVisibility ? "true" : "false") << "\"";
  of << indent << " fiberTubeVisibility=\"" << (this->FiberTubeVisibility ? "true" : "false") << "\"";
  of << indent << " fiberGlyphVisibility=\"" << (this->FiberGlyphVisibility ? "true" : "false") << "\"";
  of << indent << " twoDimensionalVisibility=\"" << (this->TwoDimensionalVisibility ? "true" : "false") << "\"";
  if (this->FiberLineDTDisplayPropertiesNodeID != NULL) 
    {
    of << indent << " fiberLineDisplayNodeRef=\"" << this->FiberLineDTDisplayPropertiesNodeID << "\" ";
    }
  if (this->FiberTubeDTDisplayPropertiesNodeID != NULL) 
    {
    of << indent << "fiberTubeDisplayNodeRef=\"" << this->FiberTubeDTDisplayPropertiesNodeID << "\" ";
    }
  if (this->FiberGlyphDTDisplayPropertiesNodeID != NULL) 
    {
    of << indent << "fiberGlyphDisplayNodeRef=\"" << this->FiberGlyphDTDisplayPropertiesNodeID << "\" ";
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

    if (!strcmp(attName, "fiberLineOpacity")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FiberLineOpacity;
      }

    if (!strcmp(attName, "fiberTubeOpacity")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FiberTubeOpacity;
      }

    if (!strcmp(attName, "fiberGlyphOpacity")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FiberGlyphOpacity;
      }

    if (!strcmp(attName, "fiberTubeRadius")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FiberTubeRadius;
      }

    if (!strcmp(attName, "fiberTubeNumberOfSides")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FiberTubeNumberOfSides;
      }

    if (!strcmp(attName, "colorModeForFiberLines")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ColorModeForFiberLines;
      }
    if (!strcmp(attName, "colorModeForFiberTubes")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ColorModeForFiberTubes;
      }
    if (!strcmp(attName, "colorModeForFiberGlyphs")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ColorModeForFiberGlyphs;
      }

    else if (!strcmp(attName, "fiberLineVisibility")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->FiberLineVisibility = 1;
        }
      else
        {
        this->FiberLineVisibility = 0;
        }
      }
    else if (!strcmp(attName, "fiberTubeVisibility")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->FiberTubeVisibility = 1;
        }
      else
        {
        this->FiberTubeVisibility = 0;
        }
      }
    else if (!strcmp(attName, "fiberGlyphVisibility")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->FiberGlyphVisibility = 1;
        }
      else
        {
        this->FiberGlyphVisibility = 0;
        }
      }
    else if (!strcmp(attName, "twoDimensionalVisibility")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->TwoDimensionalVisibility  = 1;
        }
      else
        {
        this->TwoDimensionalVisibility = 0;
        }
      }

    else if (!strcmp(attName, "fiberLineDisplayNodeRef")) 
      {
      this->SetFiberLineDTDisplayPropertiesNodeID(attValue);
      this->Scene->AddReferencedNodeID(this->FiberLineDTDisplayPropertiesNodeID, this);
      }
    else if (!strcmp(attName, "fiberTubeDisplayNodeRef")) 
      {
      this->SetFiberTubeDTDisplayPropertiesNodeID(attValue);
      this->Scene->AddReferencedNodeID(this->FiberTubeDTDisplayPropertiesNodeID, this);
      }
    else if (!strcmp(attName, "fiberGlyphDisplayNodeRef")) 
      {
      this->SetFiberGlyphDTDisplayPropertiesNodeID(attValue);
      this->Scene->AddReferencedNodeID(this->FiberGlyphDTDisplayPropertiesNodeID, this);
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

  this->SetFiberLineDTDisplayPropertiesNodeID(node->FiberLineDTDisplayPropertiesNodeID);
  this->SetFiberTubeDTDisplayPropertiesNodeID(node->FiberTubeDTDisplayPropertiesNodeID);
  this->SetFiberGlyphDTDisplayPropertiesNodeID(node->FiberGlyphDTDisplayPropertiesNodeID);
  this->SetFiberLineOpacity(node->FiberLineOpacity);
  this->SetFiberTubeOpacity(node->FiberTubeOpacity);
  this->SetFiberGlyphOpacity(node->FiberGlyphOpacity);
  this->SetFiberTubeRadius(node->FiberTubeRadius);
  this->SetFiberTubeNumberOfSides(node->FiberTubeNumberOfSides);
  this->SetColorModeForFiberLines(node->ColorModeForFiberLines);
  this->SetColorModeForFiberTubes(node->ColorModeForFiberTubes);
  this->SetColorModeForFiberGlyphs(node->ColorModeForFiberGlyphs);
  this->SetFiberLineVisibility(node->FiberLineVisibility);
  this->SetFiberTubeVisibility(node->FiberTubeVisibility);
  this->SetFiberGlyphVisibility(node->FiberGlyphVisibility);
  this->SetTwoDimensionalVisibility(node->TwoDimensionalVisibility);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
  os << indent << "FiberLineOpacity:             " << this->FiberLineOpacity << "\n";
  os << indent << "FiberTubeOpacity:             " << this->FiberTubeOpacity << "\n";
  os << indent << "FiberGlyphOpacity:             " << this->FiberGlyphOpacity << "\n";
  os << indent << "FiberTubeRadius:             " << this->FiberTubeRadius << "\n";
  os << indent << "FiberTubeNumberOfSides:             " << this->FiberTubeNumberOfSides << "\n";
  os << indent << "ColorModeForFiberLines:             " << this->ColorModeForFiberLines << "\n";
  os << indent << "ColorModeForFiberTubes:             " << this->ColorModeForFiberTubes << "\n";
  os << indent << "ColorModeForFiberGlyphs:             " << this->ColorModeForFiberGlyphs << "\n";
  os << indent << "FiberLineVisibility:             " << this->FiberLineVisibility << "\n";
  os << indent << "FiberTubeVisibility:             " << this->FiberTubeVisibility << "\n";
  os << indent << "FiberGlyphVisibility:             " << this->FiberGlyphVisibility << "\n";
  os << indent << "TwoDimensionalVisibility:             " << this->TwoDimensionalVisibility << "\n";

  os << indent << "FiberTubeDTDisplayPropertiesNodeID: " <<
    (this->FiberTubeDTDisplayPropertiesNodeID ? this->FiberTubeDTDisplayPropertiesNodeID : "(none)") << "\n";
  os << indent << "FiberLineDTDisplayPropertiesNodeID: " <<
    (this->FiberLineDTDisplayPropertiesNodeID ? this->FiberLineDTDisplayPropertiesNodeID : "(none)") << "\n";
  os << indent << "FiberGlyphDTDisplayPropertiesNodeID: " <<
    (this->FiberGlyphDTDisplayPropertiesNodeID ? this->FiberGlyphDTDisplayPropertiesNodeID : "(none)") << "\n";
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

   this->SetAndObserveFiberTubeDTDisplayPropertiesNodeID(this->GetFiberTubeDTDisplayPropertiesNodeID());
   this->SetAndObserveFiberLineDTDisplayPropertiesNodeID(this->GetFiberLineDTDisplayPropertiesNodeID());
   this->SetAndObserveFiberGlyphDTDisplayPropertiesNodeID(this->GetFiberGlyphDTDisplayPropertiesNodeID());
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->FiberTubeDTDisplayPropertiesNodeID != NULL && this->Scene->GetNodeByID(this->FiberTubeDTDisplayPropertiesNodeID) == NULL)
    {
    this->SetAndObserveFiberTubeDTDisplayPropertiesNodeID(NULL);
    }
  if (this->FiberLineDTDisplayPropertiesNodeID != NULL && this->Scene->GetNodeByID(this->FiberLineDTDisplayPropertiesNodeID) == NULL)
    {
    this->SetAndObserveFiberLineDTDisplayPropertiesNodeID(NULL);
    }
  if (this->FiberGlyphDTDisplayPropertiesNodeID != NULL && this->Scene->GetNodeByID(this->FiberGlyphDTDisplayPropertiesNodeID) == NULL)
    {
    this->SetAndObserveFiberGlyphDTDisplayPropertiesNodeID(NULL);
    }
}


//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->FiberTubeDTDisplayPropertiesNodeID && !strcmp(oldID, this->FiberTubeDTDisplayPropertiesNodeID))
    {
    this->SetFiberTubeDTDisplayPropertiesNodeID(newID);
    }
  if (this->FiberLineDTDisplayPropertiesNodeID && !strcmp(oldID, this->FiberLineDTDisplayPropertiesNodeID))
    {
    this->SetFiberLineDTDisplayPropertiesNodeID(newID);
    }
  if (this->FiberGlyphDTDisplayPropertiesNodeID && !strcmp(oldID, this->FiberGlyphDTDisplayPropertiesNodeID))
    {
    this->SetFiberGlyphDTDisplayPropertiesNodeID(newID);
    }
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

