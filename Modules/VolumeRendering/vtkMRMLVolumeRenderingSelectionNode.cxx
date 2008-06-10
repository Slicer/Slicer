/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeRenderingSelectionNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeRenderingSelectionNode.h"

#include "vtkMatrix4x4.h"

//------------------------------------------------------------------------------
vtkMRMLVolumeRenderingSelectionNode* vtkMRMLVolumeRenderingSelectionNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingSelectionNode");
  if(ret)
    {
    return (vtkMRMLVolumeRenderingSelectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeRenderingSelectionNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeRenderingSelectionNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingSelectionNode");
  if(ret)
    {
    return (vtkMRMLVolumeRenderingSelectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeRenderingSelectionNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingSelectionNode::vtkMRMLVolumeRenderingSelectionNode()
{
  this->SingletonTag = "vtkMRMLVolumeRenderingSelectionNode";
  this->HideFromEditors = 1;
  this->ActiveVolumeID = NULL;
  this->ActiveVolumeRenderingID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingSelectionNode::~vtkMRMLVolumeRenderingSelectionNode()
{
  if (this->ActiveVolumeID)
    {
    delete [] this->ActiveVolumeID;
    this->ActiveVolumeID = NULL;
    }
  if (this->ActiveVolumeRenderingID)
    {
    delete [] this->ActiveVolumeRenderingID;
    this->ActiveVolumeRenderingID = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingSelectionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " activeVolumeID=\"" << (this->ActiveVolumeID ? this->ActiveVolumeID : "NULL") << "\"";
  of << indent << " activeVolumeRenderingID=\"" << (this->ActiveVolumeRenderingID ? this->ActiveVolumeRenderingID : "NULL") << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingSelectionNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->ActiveVolumeID && !strcmp(oldID, this->ActiveVolumeID))
    {
    this->SetActiveVolumeID(newID);
    }
  if (this->ActiveVolumeRenderingID && !strcmp(oldID, this->ActiveVolumeRenderingID))
    {
    this->SetActiveVolumeRenderingID(newID);
    }
}

//-----------------------------------------------------------
void vtkMRMLVolumeRenderingSelectionNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->ActiveVolumeID != NULL && this->Scene->GetNodeByID(this->ActiveVolumeID) == NULL)
    {
    this->SetActiveVolumeID(NULL);
    }
  if (this->ActiveVolumeRenderingID != NULL && this->Scene->GetNodeByID(this->ActiveVolumeRenderingID) == NULL)
    {
    this->SetActiveVolumeRenderingID(NULL);
    }
}
//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingSelectionNode::ReadXMLAttributes(const char** atts)
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
      //this->Scene->AddReferencedNodeID(this->ActiveVolumeID, this);
      }
    if (!strcmp(attName, "activeVolumeRenderingID")) 
      {
      this->SetActiveVolumeRenderingID(attValue);
      //this->Scene->AddReferencedNodeID(this->ActiveVolumeRenderingID, this);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLVolumeRenderingSelectionNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVolumeRenderingSelectionNode *node = vtkMRMLVolumeRenderingSelectionNode::SafeDownCast(anode);

  this->SetActiveVolumeID(node->GetActiveVolumeID());
  this->SetActiveVolumeRenderingID(node->GetActiveVolumeRenderingID());
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingSelectionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "ActiveVolumeID: " << ( (this->ActiveVolumeID) ? this->ActiveVolumeID : "None" ) << "\n";
  os << "ActiveVolumeRenderingID: " << ( (this->ActiveVolumeRenderingID) ? this->ActiveVolumeRenderingID : "None" ) << "\n";
}


// End
