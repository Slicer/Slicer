/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSelectionNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"

#include "vtkMatrix4x4.h"

//------------------------------------------------------------------------------
vtkMRMLSelectionNode* vtkMRMLSelectionNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSelectionNode");
  if(ret)
    {
    return (vtkMRMLSelectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSelectionNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLSelectionNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSelectionNode");
  if(ret)
    {
    return (vtkMRMLSelectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSelectionNode;
}

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::vtkMRMLSelectionNode()
{
  this->ActiveVolumeID = NULL;
  this->ActiveLabelVolumeID = NULL;
  this->ActiveFiducialListID = NULL;

  //--- by default, the application begins with the mouse
  //--- mode set to twiddle the 3D view.
  this->SetMouseInteractionMode ( this->MouseTransform );
}

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::~vtkMRMLSelectionNode()
{
  if (this->ActiveVolumeID)
    {
    delete [] this->ActiveVolumeID;
    this->ActiveVolumeID = NULL;
    }
  if (this->ActiveLabelVolumeID)
    {
    delete [] this->ActiveLabelVolumeID;
    this->ActiveLabelVolumeID = NULL;
    }
  if (this->ActiveFiducialListID)
    {
    delete [] this->ActiveFiducialListID;
    this->ActiveFiducialListID = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << "activeVolumeID=\"" << (this->ActiveVolumeID ? this->ActiveVolumeID : "NULL") << "\" ";
  of << indent << "activeLabelVolumeID=\"" << (this->ActiveLabelVolumeID ? this->ActiveLabelVolumeID : "NULL") << "\" ";
  of << indent << "activeFiducialListID=\"" << (this->ActiveFiducialListID ? this->ActiveFiducialListID : "NULL") << "\" ";
  of << indent << "mouseInteractionMode=\"" << this->GetMouseInteractionModeAsString() <<  "\" ";
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->ActiveVolumeID && !strcmp(oldID, this->ActiveVolumeID))
    {
    this->SetActiveVolumeID(newID);
    }
  if (this->ActiveLabelVolumeID && !strcmp(oldID, this->ActiveLabelVolumeID))
    {
    this->SetActiveLabelVolumeID(newID);
    }
  if (this->ActiveFiducialListID && !strcmp(oldID, this->ActiveFiducialListID))
    {
    this->SetActiveFiducialListID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "activeVolumeID")) 
      {
      this->SetActiveVolumeID(attValue);
      this->Scene->AddReferencedNodeID(this->ActiveVolumeID, this);
      }
    if (!strcmp(attName, "activeLabelVolumeID")) 
      {
      this->SetActiveLabelVolumeID(attValue);
      this->Scene->AddReferencedNodeID(this->ActiveLabelVolumeID, this);
      }
    if (!strcmp(attName, "activeFiducialListID")) 
      {
      this->SetActiveFiducialListID(attValue);
      this->Scene->AddReferencedNodeID(this->ActiveFiducialListID, this);
      }
    if (!strcmp(attName, "mouseInteractionMode"))
      {
      std::stringstream ss;
      int mode;
      ss << attValue;
      ss >> mode;
      this->SetMouseInteractionMode(mode);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLSelectionNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSelectionNode *node = vtkMRMLSelectionNode::SafeDownCast(anode);

  this->SetActiveVolumeID(node->GetActiveVolumeID());
  this->SetActiveLabelVolumeID(node->GetActiveLabelVolumeID());
  this->SetActiveFiducialListID(node->GetActiveFiducialListID());
  this->SetMouseInteractionMode(node->GetMouseInteractionMode());
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "ActiveVolumeID: " << ( (this->ActiveVolumeID) ? this->ActiveVolumeID : "None" ) << "\n";
  os << "ActiveLabelVolumeID: " << ( (this->ActiveLabelVolumeID) ? this->ActiveLabelVolumeID : "None" ) << "\n";
  os << "ActiveFiducialListID: " << ( (this->ActiveFiducialListID) ? this->ActiveFiducialListID : "None" ) << "\n";
  os << "MouseInteractionMode: " << this->GetMouseInteractionModeAsString() << "\n";

}

//---------------------------------------------------------------------------
char * vtkMRMLSelectionNode::GetMouseInteractionModeAsString()
{
  return GetMouseInteractionModeAsString(this->GetMouseInteractionMode());
}

//---------------------------------------------------------------------------
char * vtkMRMLSelectionNode::GetMouseInteractionModeAsString(int mode)
{
  if (mode == this->MouseSelect)
    {
    return "Select";
    }
  if (mode == this->MouseTransform)
    {
    return "Transform";
    }
  if (mode == this->MousePut)
    {
    return "Put";
    }
  return "(unknown)";
}

//---------------------------------------------------------------------------
void vtkMRMLSelectionNode::SetMouseInteractionMode(int mode)
{
  if (mode < this->MouseSelect ||
      mode > this->MousePut)
    {
    vtkErrorMacro("Mode " << mode << " is out of valid range " << this->MouseSelect << " - " << this->MousePut);
    return;
    }
  if (mode == this->MouseInteractionMode)
    {
    // no change
    //std::cout << "No change in mouse interaction mode\n";
    return;
    }
  
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting MouseInteractionMode to " << mode);
  this->MouseInteractionMode = mode;

  // invoke a modified event
  this->Modified();
}
// End
