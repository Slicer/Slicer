/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeRenderingParametersNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeRenderingParametersNode.h"


//------------------------------------------------------------------------------
vtkMRMLVolumeRenderingParametersNode* vtkMRMLVolumeRenderingParametersNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingParametersNode");
  if(ret)
    {
    return (vtkMRMLVolumeRenderingParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeRenderingParametersNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeRenderingParametersNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingParametersNode");
  if(ret)
    {
    return (vtkMRMLVolumeRenderingParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeRenderingParametersNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingParametersNode::vtkMRMLVolumeRenderingParametersNode()
{
  this->HideFromEditors = 1;
  this->VolumeNodeID = NULL;
  this->VolumePropertyNodeID = NULL;
  this->ROINodeID = NULL;
  this->CroppingEnabled = 0;
  
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingParametersNode::~vtkMRMLVolumeRenderingParametersNode()
{
  if (this->VolumeNodeID)
    {
    delete [] this->VolumeNodeID;
    this->VolumeNodeID = NULL;
    }
  if (this->VolumePropertyNodeID)
    {
    delete [] this->VolumePropertyNodeID;
    this->VolumePropertyNodeID = NULL;
    }
  if (this->ROINodeID)
    {
    delete [] this->ROINodeID;
    this->ROINodeID = NULL;
    }  
}
//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "volumeNodeID")) 
      {
      this->SetVolumeNodeID(attValue);
      }
    if (!strcmp(attName, "ROINodeID")) 
      {
      this->SetROINodeID(attValue);
      }    
    if (!strcmp(attName, "volumePropertyNodeID")) 
      {
      this->SetVolumePropertyNodeID(attValue);
      }
    else if (!strcmp(attName,"croppingEnabled"))
      {
      std::stringstream ss;
      ss<<attValue;
      ss>>this->CroppingEnabled;  
      }

    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " volumeNodeID=\"" << (this->VolumeNodeID ? this->VolumeNodeID : "NULL") << "\"";
  of << " croppingEnabled=\""<< this->CroppingEnabled<< "\"";
  of << indent << " ROINodeID=\"" << (this->ROINodeID ? this->ROINodeID : "NULL") << "\"";
  of << indent << " volumePropertyNodeID=\"" << (this->VolumePropertyNodeID ? this->VolumePropertyNodeID : "NULL") << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->VolumeNodeID && !strcmp(oldID, this->VolumeNodeID))
    {
    this->SetVolumeNodeID(newID);
    }
  if (this->ROINodeID && !strcmp(oldID, this->ROINodeID))
    {
    this->SetROINodeID(newID);
    }
  if (this->VolumePropertyNodeID && !strcmp(oldID, this->VolumePropertyNodeID))
    {
    this->SetVolumePropertyNodeID(newID);
    }
}

//-----------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->VolumeNodeID != NULL && this->Scene->GetNodeByID(this->VolumeNodeID) == NULL)
    {
    this->SetVolumeNodeID(NULL);
    }
  if (this->ROINodeID != NULL && this->Scene->GetNodeByID(this->ROINodeID) == NULL)
    {
    this->SetROINodeID(NULL);
    }  
  if (this->VolumePropertyNodeID != NULL && this->Scene->GetNodeByID(this->VolumePropertyNodeID) == NULL)
    {
    this->SetVolumePropertyNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLVolumeRenderingParametersNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVolumeRenderingParametersNode *node = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(anode);
  this->DisableModifiedEventOn();

  this->SetVolumeNodeID(node->GetVolumeNodeID());
  this->SetVolumePropertyNodeID(node->GetVolumePropertyNodeID());
  this->SetROINodeID(node->GetROINodeID());
  this->SetCroppingEnabled(node->GetCroppingEnabled());
  
  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
  
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "VolumeNodeID: " << ( (this->VolumeNodeID) ? this->VolumeNodeID : "None" ) << "\n";
  os << "ROINodeID: " << ( (this->VolumeNodeID) ? this->ROINodeID : "None" ) << "\n";
  os << "VolumePropertyNodeID: " << ( (this->VolumePropertyNodeID) ? this->VolumePropertyNodeID : "None" ) << "\n";
  os << "CroppingEnabled: " << this->CroppingEnabled << "\n";
}


// End
