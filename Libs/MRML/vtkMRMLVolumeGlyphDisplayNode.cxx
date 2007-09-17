/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeGlyphDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLVolumeGlyphDisplayNode.h"
#include "vtkMRMLScene.h"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLVolumeGlyphDisplayNode* vtkMRMLVolumeGlyphDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeGlyphDisplayNode");
  if(ret)
    {
    return (vtkMRMLVolumeGlyphDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeGlyphDisplayNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeGlyphDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeGlyphDisplayNode");
  if(ret)
    {
    return (vtkMRMLVolumeGlyphDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeGlyphDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeGlyphDisplayNode::vtkMRMLVolumeGlyphDisplayNode()
{
  // Strings

  this->GlyphColorNodeID = NULL;
  this->GlyphColorNode = NULL;
  // try setting a default greyscale color map
  //this->SetDefaultColorMap(0);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::SetDefaultColorMap(int isLabelMap)
 {
  // set up a default color node
   // TODO: figure out if can use vtkSlicerColorLogic's helper methods
   if (isLabelMap)
     {
     this->SetAndObserveGlyphColorNodeID("vtkMRMLColorTableNodeLabels");
     }
   else
     {
     this->SetAndObserveGlyphColorNodeID("vtkMRMLColorTableNodeGrey");
     }
  if (this->GlyphColorNode == NULL)
    {
    vtkDebugMacro("vtkMRMLVolumeGlyphDisplayNode: FAILED setting default  color node, it's still null\n")
    }
  else
    {
    vtkDebugMacro("vtkMRMLVolumeGlyphDisplayNode: set up the default color node\n");
    }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeGlyphDisplayNode::~vtkMRMLVolumeGlyphDisplayNode()
{
  this->SetAndObserveGlyphColorNodeID( NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->GlyphColorNodeID != NULL) 
    {
    of << indent << " glyphColorNodeRef=\"" << this->GlyphColorNodeID << "\"";
    }

  std::stringstream ss;
  ss << this->VisualizationMode;
  of << indent << " visualizationMode=\"" << ss.str() << "\"";


}

//----------------------------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->GlyphColorNodeID && !strcmp(oldID, this->GlyphColorNodeID))
    {
    this->SetGlyphColorNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "glyphColorNodeRef")) 
      {
      this->SetGlyphColorNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->GlyphColorNodeID, this);
      }
    if (!strcmp(attName, "visualizationMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->VisualizationMode;
      }

    }  
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVolumeGlyphDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVolumeGlyphDisplayNode *node = (vtkMRMLVolumeGlyphDisplayNode *) anode;

 this->SetGlyphColorNodeID(node->GlyphColorNodeID);

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

 os << indent << "GlyphColorNodeID: " <<
    (this->GlyphColorNodeID ? this->GlyphColorNodeID : "(none)") << "\n";
 os << indent << "Visualization Mode:   " << this->VisualizationMode << "\n";
}

//-----------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveGlyphColorNodeID(this->GetGlyphColorNodeID());
}

//-----------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->GlyphColorNodeID != NULL && this->Scene->GetNodeByID(this->GlyphColorNodeID) == NULL)
    {
    this->SetAndObserveGlyphColorNodeID(NULL);
    }
}
//-----------------------------------------------------------
vtkPolyData* vtkMRMLVolumeGlyphDisplayNode::ExecuteGlyphPipeLineAndGetPolyData( vtkImageData* )
{
  return NULL;
}
//----------------------------------------------------------------------------
vtkMRMLColorNode* vtkMRMLVolumeGlyphDisplayNode::GetGlyphColorNode()
{
  vtkMRMLColorNode* node = NULL;
  if (this->GetScene() && this->GetGlyphColorNodeID() )
    {
    vtkMRMLNode* cnode = this->GetScene()->GetNodeByID(this->GlyphColorNodeID);
    node = vtkMRMLColorNode::SafeDownCast(cnode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::SetAndObserveGlyphColorNodeID(std::string glyphColorNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->GlyphColorNode, NULL);

  this->SetGlyphColorNodeID(glyphColorNodeID.c_str());
 
  vtkMRMLColorNode *cnode = this->GetGlyphColorNode();
  if (cnode != NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->GlyphColorNode, cnode);
    }
}
  
//----------------------------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::SetAndObserveGlyphColorNodeID(const char *glyphColorNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->GlyphColorNode, NULL);

  this->SetGlyphColorNodeID(glyphColorNodeID);

  vtkMRMLColorNode *cnode = this->GetGlyphColorNode();
  if (cnode != NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->GlyphColorNode, cnode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLColorNode *cnode = this->GetGlyphColorNode();
  if (cnode != NULL && cnode == vtkMRMLColorNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  return;
}


