/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDiffusionTensorVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode* vtkMRMLDiffusionTensorVolumeDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorVolumeDisplayNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLDiffusionTensorVolumeDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorVolumeDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode::vtkMRMLDiffusionTensorVolumeDisplayNode()
{
 this->VisualizationMode = 0;
 this->DiffusionTensorDisplayPropertiesNode = NULL;
 this->DiffusionTensorDisplayPropertiesNodeID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode::~vtkMRMLDiffusionTensorVolumeDisplayNode()
{
  this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL); 
}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  std::stringstream ss;
  ss << this->VisualizationMode;
  of << indent << "visualizationMode=\"" << ss.str() << "\" ";

  ss.clear();
  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL)
    {
    ss << this->DiffusionTensorDisplayPropertiesNodeID;
    of << indent << "diffussionTensorDisplayPropertiesNodeID=\"" << ss.str() << "\" ";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "visualizationMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->VisualizationMode;
      }
    else if (!strcmp(attName, "diffusionTensorDisplayPropertiesNodeID"))
      {
      this->SetDiffusionTensorDisplayPropertiesNodeID(attValue);
      this->Scene->AddReferencedNodeID(this->DiffusionTensorDisplayPropertiesNodeID, this);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDiffusionTensorVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLDiffusionTensorVolumeDisplayNode *node = (vtkMRMLDiffusionTensorVolumeDisplayNode *) anode;

  this->SetVisualizationMode(node->VisualizationMode);
  this->SetDiffusionTensorDisplayPropertiesNodeID(node->DiffusionTensorDisplayPropertiesNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "Visualization Mode:   " << this->VisualizationMode << "\n";
  os << indent << "DiffusionTensorDisplayPropertiesNodeID:   " << this->DiffusionTensorDisplayPropertiesNodeID << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(this->GetDiffusionTensorDisplayPropertiesNodeID());

}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->DiffusionTensorDisplayPropertiesNodeID && !strcmp(oldID, this->DiffusionTensorDisplayPropertiesNodeID))
    {
    this->SetDiffusionTensorDisplayPropertiesNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *ID )
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

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLDiffusionTensorVolumeDisplayNode::GetDiffusionTensorDisplayPropertiesNode ( )
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

