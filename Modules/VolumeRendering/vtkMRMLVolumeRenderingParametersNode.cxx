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

  this->PresetsNodeID = NULL;

  this->VolumePropertyNodeID = NULL;
  this->VolumePropertyNode = NULL;

  this->FgVolumeNodeID = NULL;
  this->FgVolumeNode = NULL;

  this->FgPresetsNodeID = NULL;

  this->FgVolumePropertyNodeID = NULL;
  this->FgVolumePropertyNode = NULL;

  this->ROINodeID = NULL;
  this->ROINode = NULL;

  this->CroppingEnabled = 0;
  for(int i = 0; i < COUNT_CROPPING_REGION_PLANES; i++)
    this->CroppingRegionPlanes[i] = 0;

  this->ExpectedFPS = 5;
  this->EstimatedSampleDistance = 1.0;

  this->CurrentVolumeMapper = -1;
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

  if (this->FgVolumeNodeID)
    {
    SetAndObserveFgVolumeNodeID(NULL);
    }

  if (this->FgVolumePropertyNodeID)
    {
    SetAndObserveFgVolumePropertyNodeID(NULL);
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
      continue;
    }
    if (!strcmp(attName, "presetsNodeID"))
    {
      this->SetPresetsNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "fgVolumeNodeID"))
    {
      this->SetFgVolumeNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "fgPresetsNodeID"))
    {
      this->SetFgPresetsNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "ROINodeID"))
    {
      this->SetROINodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "volumePropertyNodeID"))
    {
      this->SetVolumePropertyNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "fgVolumePropertyNodeID"))
    {
      this->SetFgVolumePropertyNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "currentVolumeMapper"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CurrentVolumeMapper;
      continue;
    }
    if (!strcmp(attName,"croppingEnabled"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CroppingEnabled;
      continue;
    }
    if (!strcmp(attName,"croppingRegionPlanes"))
    {
      std::stringstream ss;
      ss << attValue;
      for(int i = 0; i < COUNT_CROPPING_REGION_PLANES; i++)
        ss >> this->CroppingRegionPlanes[i];
      continue;
    }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " volumeNodeID=\"" << (this->VolumeNodeID ? this->VolumeNodeID : "NULL") << "\"";
  of << indent << " fgVolumeNodeID=\"" << (this->FgVolumeNodeID ? this->FgVolumeNodeID : "NULL") << "\"";
  of << indent << " currentVolumeMapper=\"" << this->CurrentVolumeMapper << "\"";
  of << indent << " croppingEnabled=\""<< this->CroppingEnabled << "\"";
  of << indent << " ROINodeID=\"" << (this->ROINodeID ? this->ROINodeID : "NULL") << "\"";
  of << indent << " volumePropertyNodeID=\"" << (this->VolumePropertyNodeID ? this->VolumePropertyNodeID : "NULL") << "\"";
  of << indent << " fgVolumePropertyNodeID=\"" << (this->FgVolumePropertyNodeID ? this->FgVolumePropertyNodeID : "NULL") << "\"";
  of << indent << " presetsNodeID=\"" << (this->PresetsNodeID ? this->PresetsNodeID : "NULL") << "\"";
  of << indent << " fgPresetsNodeID=\"" << (this->FgPresetsNodeID ? this->FgPresetsNodeID : "NULL") << "\"";

  of << indent << " croppingRegionPlanes=\"";
  for(int i = 0; i < COUNT_CROPPING_REGION_PLANES; i++)
  {
    of << this->CroppingRegionPlanes[i];
    if(i != COUNT_CROPPING_REGION_PLANES - 1)
      of<<" ";
  }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->VolumeNodeID && !strcmp(oldID, this->VolumeNodeID))
    {
    this->SetAndObserveVolumeNodeID(newID);
    }
  if (this->FgVolumeNodeID && !strcmp(oldID, this->FgVolumeNodeID))
    {
    this->SetAndObserveFgVolumeNodeID(newID);
    }
  if (this->ROINodeID && !strcmp(oldID, this->ROINodeID))
    {
    this->SetAndObserveROINodeID(newID);
    }
  if (this->VolumePropertyNodeID && !strcmp(oldID, this->VolumePropertyNodeID))
    {
    this->SetAndObserveVolumePropertyNodeID(newID);
    }
  if (this->FgVolumePropertyNodeID && !strcmp(oldID, this->FgVolumePropertyNodeID))
    {
    this->SetAndObserveFgVolumePropertyNodeID(newID);
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
  if (this->FgVolumeNodeID != NULL && this->Scene->GetNodeByID(this->FgVolumeNodeID) == NULL)
    {
    this->SetAndObserveFgVolumeNodeID(NULL);
    }
  if (this->ROINodeID != NULL && this->Scene->GetNodeByID(this->ROINodeID) == NULL)
    {
    this->SetAndObserveROINodeID(NULL);
    }
  if (this->VolumePropertyNodeID != NULL && this->Scene->GetNodeByID(this->VolumePropertyNodeID) == NULL)
    {
    this->SetAndObserveVolumePropertyNodeID(NULL);
    }
  if (this->FgVolumePropertyNodeID != NULL && this->Scene->GetNodeByID(this->FgVolumePropertyNodeID) == NULL)
    {
    this->SetAndObserveFgVolumePropertyNodeID(NULL);
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
  this->SetFgVolumeNodeID(node->GetFgVolumeNodeID());
  this->SetPresetsNodeID(node->GetPresetsNodeID());
  this->SetFgPresetsNodeID(node->GetFgPresetsNodeID());
  this->SetVolumePropertyNodeID(node->GetVolumePropertyNodeID());
  this->SetFgVolumePropertyNodeID(node->GetFgVolumePropertyNodeID());
  this->SetROINodeID(node->GetROINodeID());
  this->SetCroppingEnabled(node->GetCroppingEnabled());

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::SetAndObserveVolumeNodeID(const char *volumeNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->VolumeNode, NULL);

  if (volumeNodeID != NULL)
  {
    this->SetVolumeNodeID(volumeNodeID);
    vtkMRMLVolumeNode *node = this->GetVolumeNode();
    vtkSetAndObserveMRMLObjectMacro(this->VolumeNode, node);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::SetAndObserveFgVolumeNodeID(const char *volumeNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->FgVolumeNode, NULL);

  if (volumeNodeID != NULL)
  {
    this->SetFgVolumeNodeID(volumeNodeID);
    vtkMRMLVolumeNode *node = this->GetFgVolumeNode();
    vtkSetAndObserveMRMLObjectMacro(this->FgVolumeNode, node);
  }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeRenderingParametersNode::GetVolumeNode()
{
  if (this->VolumeNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->VolumeNode, NULL);
    }
  else if (this->GetScene() &&
           ((this->VolumeNode != NULL && strcmp(this->VolumeNode->GetID(), this->VolumeNodeID)) ||
            (this->VolumeNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->VolumeNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->VolumeNode, vtkMRMLVolumeNode::SafeDownCast(snode));
    }
  return this->VolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeRenderingParametersNode::GetFgVolumeNode()
{
  if (this->FgVolumeNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->FgVolumeNode, NULL);
    }
  else if (this->GetScene() &&
           ((this->FgVolumeNode != NULL && strcmp(this->FgVolumeNode->GetID(), this->FgVolumeNodeID)) ||
            (this->FgVolumeNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->FgVolumeNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->FgVolumeNode, vtkMRMLVolumeNode::SafeDownCast(snode));
    }
  return this->FgVolumeNode;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::SetAndObserveVolumePropertyNodeID(const char *VolumePropertyNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->VolumePropertyNode, NULL);

  if (VolumePropertyNodeID != NULL)
  {
    this->SetVolumePropertyNodeID(VolumePropertyNodeID);
    vtkMRMLVolumePropertyNode *node = this->GetVolumePropertyNode();
    vtkSetAndObserveMRMLObjectMacro(this->VolumePropertyNode, node);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingParametersNode::SetAndObserveFgVolumePropertyNodeID(const char *VolumePropertyNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->FgVolumePropertyNode, NULL);

  if (VolumePropertyNodeID != NULL)
  {
    this->SetFgVolumePropertyNodeID(VolumePropertyNodeID);
    vtkMRMLVolumePropertyNode *node = this->GetFgVolumePropertyNode();
    vtkSetAndObserveMRMLObjectMacro(this->FgVolumePropertyNode, node);
  }
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLVolumeRenderingParametersNode::GetVolumePropertyNode()
{
  if (this->VolumePropertyNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->VolumePropertyNode, NULL);
    }
  else if (this->GetScene() &&
           ((this->VolumePropertyNode != NULL && strcmp(this->VolumePropertyNode->GetID(), this->VolumePropertyNodeID)) ||
            (this->VolumePropertyNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->VolumePropertyNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->VolumePropertyNode, vtkMRMLVolumePropertyNode::SafeDownCast(snode));
    }
  return this->VolumePropertyNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLVolumeRenderingParametersNode::GetFgVolumePropertyNode()
{
  if (this->FgVolumePropertyNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->FgVolumePropertyNode, NULL);
    }
  else if (this->GetScene() &&
           ((this->FgVolumePropertyNode != NULL && strcmp(this->FgVolumePropertyNode->GetID(), this->FgVolumePropertyNodeID)) ||
            (this->FgVolumePropertyNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->FgVolumePropertyNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->FgVolumePropertyNode, vtkMRMLVolumePropertyNode::SafeDownCast(snode));
    }
  return this->FgVolumePropertyNode;
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
  this->SetAndObserveFgVolumeNodeID(this->FgVolumeNodeID);
  this->SetAndObserveFgVolumePropertyNodeID(this->FgVolumePropertyNodeID);
  this->SetAndObserveROINodeID(this->ROINodeID);
}

void vtkMRMLVolumeRenderingParametersNode::Reset()
{
  this->SetAndObserveVolumeNodeID(NULL);
  this->SetAndObserveVolumePropertyNodeID(NULL);
  this->SetPresetsNodeID(NULL);

  this->SetAndObserveROINodeID(NULL);

  this->SetAndObserveFgVolumeNodeID(NULL);
  this->SetAndObserveFgVolumePropertyNodeID(NULL);
  this->SetFgPresetsNodeID(NULL);

  this->CroppingEnabled = 0;
  for(int i = 0; i < COUNT_CROPPING_REGION_PLANES; i++)
    this->CroppingRegionPlanes[i] = 0;

  this->ExpectedFPS = 5;
  this->EstimatedSampleDistance = 1.0;

  this->CurrentVolumeMapper = -1;
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
  os << "PresetsNodeID: " << ( (this->PresetsNodeID) ? this->PresetsNodeID : "None" ) << "\n";
  os << "FgVolumeNodeID: " << ( (this->FgVolumeNodeID) ? this->FgVolumeNodeID : "None" ) << "\n";
  os << "FgPresetsNodeID: " << ( (this->FgPresetsNodeID) ? this->FgPresetsNodeID : "None" ) << "\n";
  os << "ROINodeID: " << ( (this->VolumeNodeID) ? this->ROINodeID : "None" ) << "\n";
  os << "VolumePropertyNodeID: " << ( (this->VolumePropertyNodeID) ? this->VolumePropertyNodeID : "None" ) << "\n";
  os << "FgVolumePropertyNodeID: " << ( (this->FgVolumePropertyNodeID) ? this->FgVolumePropertyNodeID : "None" ) << "\n";
  os << "CroppingEnabled: " << this->CroppingEnabled << "\n";
}

// End
