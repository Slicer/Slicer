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
  this->VolumeNode = NULL;

  this->VolumePropertyNodeID = NULL;
  this->VolumePropertyNode = NULL;

  this->ROINodeID = NULL;
  this->ROINode = NULL;

  this->CroppingEnabled = 0;
  
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingParametersNode::~vtkMRMLVolumeRenderingParametersNode()
{
  if (this->VolumeNodeID)
    {
    SetAndObserveVolumeNodeID(NULL);
    }

  if (this->VolumePropertyNodeID)
    {
    SetAndObserveVolumePropertyNodeID(NULL);
    }
  if (this->ROINodeID)
    {
    SetAndObserveROINodeID(NULL);
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
    this->SetAndObserveVolumeNodeID(newID);
    }
  if (this->ROINodeID && !strcmp(oldID, this->ROINodeID))
    {
    this->SetAndObserveROINodeID(newID);
    }
  if (this->VolumePropertyNodeID && !strcmp(oldID, this->VolumePropertyNodeID))
    {
    this->SetAndObserveVolumePropertyNodeID(newID);
    }
}

//-----------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->VolumeNodeID != NULL && this->Scene->GetNodeByID(this->VolumeNodeID) == NULL)
    {
    this->SetAndObserveVolumeNodeID(NULL);
    }
  if (this->ROINodeID != NULL && this->Scene->GetNodeByID(this->ROINodeID) == NULL)
    {
    this->SetAndObserveROINodeID(NULL);
    }  
  if (this->VolumePropertyNodeID != NULL && this->Scene->GetNodeByID(this->VolumePropertyNodeID) == NULL)
    {
    this->SetAndObserveVolumePropertyNodeID(NULL);
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
void vtkMRMLVolumeRenderingParametersNode::SetAndObserveVolumeNodeID(const char *volumeNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->VolumeNode, NULL);

  this->SetVolumeNodeID(volumeNodeID);

  vtkMRMLVolumeNode *node = this->GetVolumeNode();

  vtkSetAndObserveMRMLObjectMacro(this->VolumeNode, node);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeRenderingParametersNode::GetVolumeNode()
{
  if (this->VolumeNodeID == NULL) 
    {
    vtkSetAndObserveMRMLObjectMacro(this->VolumeNode, NULL);
    }
  else if (this->GetScene() && this->VolumeNodeID != NULL &&
           ((this->VolumeNode != NULL && strcmp(this->VolumeNode->GetID(), this->VolumeNodeID)) ||
            (this->VolumeNode = NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->VolumeNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->VolumeNode, vtkMRMLVolumeNode::SafeDownCast(snode));
    }
  return this->VolumeNode;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::SetAndObserveVolumePropertyNodeID(const char *VolumePropertyNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->VolumePropertyNode, NULL);

  this->SetVolumePropertyNodeID(VolumePropertyNodeID);

  vtkMRMLVolumePropertyNode *node = this->GetVolumePropertyNode();

  vtkSetAndObserveMRMLObjectMacro(this->VolumePropertyNode, node);
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLVolumeRenderingParametersNode::GetVolumePropertyNode()
{
  if (this->VolumePropertyNodeID == NULL) 
    {
    vtkSetAndObserveMRMLObjectMacro(this->VolumePropertyNode, NULL);
    }
  else if (this->GetScene() && this->VolumePropertyNodeID != NULL &&
           ((this->VolumePropertyNode != NULL && strcmp(this->VolumePropertyNode->GetID(), this->VolumePropertyNodeID)) ||
            (this->VolumePropertyNode = NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->VolumePropertyNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->VolumePropertyNode, vtkMRMLVolumePropertyNode::SafeDownCast(snode));
    }
  return this->VolumePropertyNode;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::SetAndObserveROINodeID(const char *ROINodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->ROINode, NULL);

  this->SetROINodeID(ROINodeID);

  vtkMRMLROINode *node = this->GetROINode();

  vtkSetAndObserveMRMLObjectMacro(this->ROINode, node);
}

//----------------------------------------------------------------------------
vtkMRMLROINode* vtkMRMLVolumeRenderingParametersNode::GetROINode()
{
  if (this->ROINodeID == NULL) 
    {
    vtkSetAndObserveMRMLObjectMacro(this->ROINode, NULL);
    }
  else if (this->GetScene() && this->ROINodeID != NULL &&
           ((this->ROINode != NULL && strcmp(this->ROINode->GetID(), this->ROINodeID)) ||
            (this->ROINode = NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->ROINodeID);
    vtkSetAndObserveMRMLObjectMacro(this->ROINode, vtkMRMLROINode::SafeDownCast(snode));
    }
  return this->ROINode;
}

//-----------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  this->SetAndObserveVolumeNodeID(this->VolumeNodeID);
  this->SetAndObserveVolumePropertyNodeID(this->VolumePropertyNodeID);
  this->SetAndObserveROINodeID(this->ROINodeID);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
    Superclass::ProcessMRMLEvents(caller, event, callData);
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    return;
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
