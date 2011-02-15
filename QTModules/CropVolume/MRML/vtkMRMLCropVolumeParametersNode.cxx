/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLCropVolumeParametersNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLCropVolumeParametersNode.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLAnnotationROINode.h"


//------------------------------------------------------------------------------
vtkMRMLCropVolumeParametersNode* vtkMRMLCropVolumeParametersNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCropVolumeParametersNode");
  if(ret)
    {
    return (vtkMRMLCropVolumeParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCropVolumeParametersNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLCropVolumeParametersNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCropVolumeParametersNode");
  if(ret)
    {
    return (vtkMRMLCropVolumeParametersNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCropVolumeParametersNode;
}

//----------------------------------------------------------------------------
vtkMRMLCropVolumeParametersNode::vtkMRMLCropVolumeParametersNode()
{
  this->HideFromEditors = 1;

  this->InputVolumeNodeID = NULL;
  this->InputVolumeNode = NULL;

  this->OutputVolumeNodeID = NULL;
  this->OutputVolumeNode = NULL;

  this->ROINodeID = NULL;
  this->ROINode = NULL;

  this->ROIVisibility = false;
  this->InterpolationMode = 0;
}

//----------------------------------------------------------------------------
vtkMRMLCropVolumeParametersNode::~vtkMRMLCropVolumeParametersNode()
{
  if (this->InputVolumeNodeID)
    {
    this->SetAndObserveInputVolumeNodeID(NULL);
    }

  if (this->OutputVolumeNodeID)
    {
    this->SetAndObserveOutputVolumeNodeID(NULL);
    }

  if (this->ROINodeID)
    {
    this->SetAndObserveROINodeID(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
  {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "inputVolumeNodeID"))
    {
      this->SetInputVolumeNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "outputVolumeNodeID"))
    {
      this->SetOutputVolumeNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "ROINodeID"))
    {
      this->SetROINodeID(attValue);
      continue;
    }
    if (!strcmp(attName,"ROIVisibility"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ROIVisibility;
      continue;
    }
    if (!strcmp(attName,"interpolationMode"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->InterpolationMode;
      continue;
    }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " inputVolumeNodeID=\"" << (this->InputVolumeNodeID ? this->InputVolumeNodeID : "NULL") << "\"";
  of << indent << " outputVolumeNodeID=\"" << (this->OutputVolumeNodeID ? this->OutputVolumeNodeID : "NULL") << "\"";
  of << indent << " ROIVisibility=\""<< this->ROIVisibility << "\"";
  of << indent << " ROINodeID=\"" << (this->ROINodeID ? this->ROINodeID : "NULL") << "\"";
  of << indent << " interpolationMode=\"" << this->InterpolationMode << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->InputVolumeNodeID && !strcmp(oldID, this->InputVolumeNodeID))
    {
    this->SetAndObserveInputVolumeNodeID(newID);
    }
  if (this->OutputVolumeNodeID && !strcmp(oldID, this->OutputVolumeNodeID))
    {
    this->SetAndObserveOutputVolumeNodeID(newID);
    }
  if (this->ROINodeID && !strcmp(oldID, this->ROINodeID))
    {
    this->SetAndObserveROINodeID(newID);
    }
}

//-----------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->InputVolumeNodeID != NULL && this->Scene->GetNodeByID(this->InputVolumeNodeID) == NULL)
    {
    this->SetAndObserveInputVolumeNodeID(NULL);
    }
  if (this->OutputVolumeNodeID != NULL && this->Scene->GetNodeByID(this->OutputVolumeNodeID) == NULL)
    {
    this->SetAndObserveOutputVolumeNodeID(NULL);
    }
  if (this->ROINodeID != NULL && this->Scene->GetNodeByID(this->ROINodeID) == NULL)
    {
    this->SetAndObserveROINodeID(NULL);
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLCropVolumeParametersNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLCropVolumeParametersNode *node = vtkMRMLCropVolumeParametersNode::SafeDownCast(anode);
  this->DisableModifiedEventOn();

  this->SetInputVolumeNodeID(node->GetInputVolumeNodeID());
  this->SetOutputVolumeNodeID(node->GetOutputVolumeNodeID());
  this->SetROINodeID(node->GetROINodeID());
  this->SetInterpolationMode(node->GetInterpolationMode());
  this->SetROIVisibility(node->GetROIVisibility());
  
  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::SetAndObserveInputVolumeNodeID(const char *volumeNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->InputVolumeNode, NULL);

  if (volumeNodeID != NULL)
  {
    this->SetInputVolumeNodeID(volumeNodeID);
    vtkMRMLVolumeNode *node = this->GetInputVolumeNode();
    vtkSetAndObserveMRMLObjectMacro(this->InputVolumeNode, node);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::SetAndObserveOutputVolumeNodeID(const char *volumeNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->OutputVolumeNode, NULL);

  if (volumeNodeID != NULL)
  {
    this->SetOutputVolumeNodeID(volumeNodeID);
    vtkMRMLVolumeNode *node = this->GetOutputVolumeNode();
    vtkSetAndObserveMRMLObjectMacro(this->OutputVolumeNode, node);
  }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLCropVolumeParametersNode::GetInputVolumeNode()
{
  if (this->InputVolumeNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->InputVolumeNode, NULL);
    }
  else if (this->GetScene() &&
           ((this->InputVolumeNode != NULL && strcmp(this->InputVolumeNode->GetID(), this->InputVolumeNodeID)) ||
            (this->InputVolumeNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->InputVolumeNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->InputVolumeNode, vtkMRMLVolumeNode::SafeDownCast(snode));
    }
  return this->InputVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLCropVolumeParametersNode::GetOutputVolumeNode()
{
  if (this->OutputVolumeNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->OutputVolumeNode, NULL);
    }
  else if (this->GetScene() &&
           ((this->OutputVolumeNode != NULL && strcmp(this->OutputVolumeNode->GetID(), this->OutputVolumeNodeID)) ||
            (this->OutputVolumeNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->OutputVolumeNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->OutputVolumeNode, vtkMRMLVolumeNode::SafeDownCast(snode));
    }
  return this->OutputVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLCropVolumeParametersNode::GetROINode()
{
  if (this->ROINodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->ROINode, NULL);
    }
  else if (this->GetScene() &&
           ((this->ROINode != NULL && strcmp(this->ROINode->GetID(), this->ROINodeID)) ||
            (this->ROINode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->ROINodeID);
    vtkSetAndObserveMRMLObjectMacro(this->ROINode, vtkMRMLAnnotationROINode::SafeDownCast(snode));
    }
  return this->ROINode;
}

//-----------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  this->SetAndObserveInputVolumeNodeID(this->InputVolumeNodeID);
  this->SetAndObserveOutputVolumeNodeID(this->OutputVolumeNodeID);
  this->SetAndObserveROINodeID(this->ROINodeID);
}

//---------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
    Superclass::ProcessMRMLEvents(caller, event, callData);
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    return;
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "InputVolumeNodeID: " << ( (this->InputVolumeNodeID) ? this->InputVolumeNodeID : "None" ) << "\n";
  os << "OutputVolumeNodeID: " << ( (this->OutputVolumeNodeID) ? this->OutputVolumeNodeID : "None" ) << "\n";
  os << "ROINodeID: " << ( (this->ROINodeID) ? this->ROINodeID : "None" ) << "\n";
  os << "ROIVisibility: " << this->ROIVisibility << "\n";
  os << "InterpolationMode: " << this->InterpolationMode << "\n";
  os << "IsotropicResampling: " << this->IsotropicResampling << "\n";
}

// End
