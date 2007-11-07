/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLScene.h"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode* vtkMRMLVolumeDisplayNode::New()
{
  return NULL;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeDisplayNode::CreateNodeInstance()
{
  return NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode::vtkMRMLVolumeDisplayNode()
{
  this->ColorNodeID = NULL;
  this->ColorNode = NULL;
  // try setting a default greyscale color map
  //this->SetDefaultColorMap(0);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode::~vtkMRMLVolumeDisplayNode()
{
  this->SetAndObserveColorNodeID( NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->ColorNodeID != NULL) 
    {
    of << indent << " colorNodeRef=\"" << this->ColorNodeID << "\"";
    }

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->ColorNodeID && !strcmp(oldID, this->ColorNodeID))
    {
    this->SetColorNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "colorNodeRef")) 
      {
      this->SetColorNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ColorNodeID, this);
      }

    }  
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVolumeDisplayNode *node = (vtkMRMLVolumeDisplayNode *) anode;

  this->SetColorNodeID(node->ColorNodeID);

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "ColorNodeID: " <<
    (this->ColorNodeID ? this->ColorNodeID : "(none)") << "\n";

}

//-----------------------------------------------------------
void vtkMRMLVolumeDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveColorNodeID(this->GetColorNodeID());
}

//-----------------------------------------------------------
void vtkMRMLVolumeDisplayNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->ColorNodeID != NULL && this->Scene->GetNodeByID(this->ColorNodeID) == NULL)
    {
    this->SetAndObserveColorNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
vtkMRMLColorNode* vtkMRMLVolumeDisplayNode::GetColorNode()
{
  vtkMRMLColorNode* node = NULL;
  if (this->GetScene() && this->GetColorNodeID() )
    {
    vtkMRMLNode* cnode = this->GetScene()->GetNodeByID(this->ColorNodeID);
    node = vtkMRMLColorNode::SafeDownCast(cnode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::SetAndObserveColorNodeID(std::string colorNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->ColorNode, NULL);

  this->SetColorNodeID(colorNodeID.c_str());
 
  vtkMRMLColorNode *cnode = this->GetColorNode();
  if (cnode != NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->ColorNode, cnode);
    }
}
  
//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::SetAndObserveColorNodeID(const char *colorNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->ColorNode, NULL);
  
  if (colorNodeID == NULL) 
    {
    if (this->ColorNodeID) 
      {
      delete [] this->ColorNodeID;
      this->ColorNodeID = NULL;
      return;
      }
    }
  this->SetColorNodeID(colorNodeID);

  vtkMRMLColorNode *cnode = this->GetColorNode();
  if (cnode != NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->ColorNode, cnode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLColorNode *cnode = this->GetColorNode();
  if (cnode != NULL && cnode == vtkMRMLColorNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  return;
}


//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::SetDefaultColorMap()
{
  this->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
}
