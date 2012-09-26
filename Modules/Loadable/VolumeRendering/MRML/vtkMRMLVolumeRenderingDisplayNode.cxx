/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeRenderingDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// Annotations includes
#include "vtkMRMLAnnotationROINode.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>

//#include "vtkMatrix4x4.h"
#include <sstream>

//----------------------------------------------------------------------------
vtkCxxSetReferenceStringMacro(vtkMRMLVolumeRenderingDisplayNode, VolumeNodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLVolumeRenderingDisplayNode, VolumePropertyNodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLVolumeRenderingDisplayNode, ROINodeID);

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode::vtkMRMLVolumeRenderingDisplayNode()
{
  this->ObservedEvents = vtkIntArray::New();
  this->ObservedEvents->InsertNextValue(vtkCommand::StartEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::InteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndInteractionEvent);

  this->VolumeNodeID = NULL;
  this->VolumeNode = NULL;

  this->VolumePropertyNodeID = NULL;
  this->VolumePropertyNode = NULL;

  this->ROINodeID = NULL;
  this->ROINode = NULL;

  this->ExpectedFPS = 8.;
  this->EstimatedSampleDistance = 2.0;

  this->GPUMemorySize = 0; // means application default

  this->CroppingEnabled = 0;//by default cropping is not enabled

  this->Threshold[0] = 0.0;
  this->Threshold[1] = 1.0;

  //this->UseThreshold = 0; // by default volume property widget is used

  this->FollowVolumeDisplayNode = 0;// by default do not follow volume display node
  this->IgnoreVolumeDisplayNodeThreshold = 0;
  this->UseSingleVolumeProperty = 0;

  this->WindowLevel[0] = 0.0;
  this->WindowLevel[1] = 0.0;

  this->PerformanceControl = 0;

  this->SetHideFromEditors(false);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode::~vtkMRMLVolumeRenderingDisplayNode()
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

  this->ObservedEvents->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::ReadXMLAttributes(const char** atts)
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
    if (!strcmp(attName,"croppingEnabled"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CroppingEnabled;
      continue;
      }
    //if (!strcmp(attName,"useThreshold"))
    //  {
    //  std::stringstream ss;
    //  ss << attValue;
    //  ss >> this->UseThreshold;
    //  continue;
    //  }
    if (!strcmp(attName,"threshold"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Threshold[0];
      ss >> this->Threshold[1];
      continue;
      }
    if (!strcmp(attName,"windowLevel"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->WindowLevel[0];
      ss >> this->WindowLevel[1];
      continue;
      }
    if (!strcmp(attName,"followVolumeDisplayNode"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->FollowVolumeDisplayNode;
      continue;
      }
    if (!strcmp(attName,"ignoreVolumeDisplayNodeThreshold"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->IgnoreVolumeDisplayNodeThreshold;
      continue;
      }
    if (!strcmp(attName, "useSingleVolumeProperty"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->UseSingleVolumeProperty;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " volumeNodeID=\"" << (this->VolumeNodeID ? this->VolumeNodeID : "NULL") << "\"";
  of << indent << " croppingEnabled=\""<< this->CroppingEnabled << "\"";
  of << indent << " ROINodeID=\"" << (this->ROINodeID ? this->ROINodeID : "NULL") << "\"";
  of << indent << " volumePropertyNodeID=\"" << (this->VolumePropertyNodeID ? this->VolumePropertyNodeID : "NULL") << "\"";
  of << indent << " threshold=\"" << this->Threshold[0] << " " << this->Threshold[1] << "\"";
  //of << indent << " useThreshold=\"" << this->UseThreshold << "\"";
  of << indent << " followVolumeDisplayNode=\"" << this->FollowVolumeDisplayNode << "\"";
  of << indent << " ignoreVolumeDisplayNodeThreshold=\"" << this->IgnoreVolumeDisplayNodeThreshold << "\"";
  of << indent << " useSingleVolumeProperty=\"" << this->UseSingleVolumeProperty << "\"";
  of << indent << " windowLevel=\"" << this->WindowLevel[0] << " " << this->WindowLevel[1] << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->VolumeNodeID, this);
  this->Scene->AddReferencedNodeID(this->ROINodeID, this);
  this->Scene->AddReferencedNodeID(this->VolumePropertyNodeID, this);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
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
void vtkMRMLVolumeRenderingDisplayNode::UpdateReferences()
{
   this->Superclass::UpdateReferences();

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

//-----------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
  this->Superclass::UpdateScene(scene);
  this->SetAndObserveVolumeNodeID(this->VolumeNodeID);
  this->SetAndObserveVolumePropertyNodeID(this->VolumePropertyNodeID);
  this->SetAndObserveROINodeID(this->ROINodeID);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLVolumeRenderingDisplayNode::Copy(vtkMRMLNode *anode)
{
  int wasModifying = this->StartModify();
  this->Superclass::Copy(anode);
  vtkMRMLVolumeRenderingDisplayNode *node = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(anode);

  this->SetVolumeNodeID(node->GetVolumeNodeID());
  this->SetVolumePropertyNodeID(node->GetVolumePropertyNodeID());
  this->SetROINodeID(node->GetROINodeID());
  this->SetCroppingEnabled(node->GetCroppingEnabled());
  this->SetGPUMemorySize(node->GetGPUMemorySize());
  this->SetEstimatedSampleDistance(node->GetEstimatedSampleDistance());
  this->SetThreshold(node->GetThreshold());
  //this->SetUseThreshold(node->GetUseThreshold());
  this->SetWindowLevel(node->GetWindowLevel());
  this->SetFollowVolumeDisplayNode(node->GetFollowVolumeDisplayNode());
  this->SetIgnoreVolumeDisplayNodeThreshold(node->GetIgnoreVolumeDisplayNodeThreshold());
  this->SetUseSingleVolumeProperty(node->GetUseSingleVolumeProperty());
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::SetAndObserveVolumeNodeID(const char *volumeNodeID)
{
  this->SetVolumeNodeID(volumeNodeID);
  this->VolumeNode = vtkMRMLVolumeNode::SafeDownCast(
    this->GetScene() ? this->GetScene()->GetNodeByID(volumeNodeID) : 0);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeRenderingDisplayNode::GetVolumeNode()
{
  if (this->VolumeNodeID == NULL)
    {
    this->VolumeNode = NULL;
    }
  if (((this->VolumeNode != NULL && this->VolumeNodeID && strcmp(this->VolumeNode->GetID(), this->VolumeNodeID)) ||
      (this->VolumeNode == NULL)) )
    {
    this->VolumeNode = vtkMRMLVolumeNode::SafeDownCast(
      this->GetScene() ? this->GetScene()->GetNodeByID(this->VolumeNodeID) : 0);
    }
  return this->VolumeNode;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode
::SetAndObserveVolumePropertyNodeID(const char *volumePropertyNodeID)
{
  this->SetVolumePropertyNodeID(volumePropertyNodeID);
  vtkMRMLVolumePropertyNode *node = this->GetVolumePropertyNode();
  if (node != this->VolumePropertyNode)
    {
    vtkSetAndObserveMRMLObjectEventsMacro(this->VolumePropertyNode, node, this->ObservedEvents);
    }
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLVolumeRenderingDisplayNode::GetVolumePropertyNode()
{
  if (this->VolumePropertyNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectEventsMacro(
      this->VolumePropertyNode, NULL, this->ObservedEvents);
    }
  else if (this->GetScene() &&
           ((this->VolumePropertyNode != NULL &&
            strcmp(this->VolumePropertyNode->GetID(), this->VolumePropertyNodeID)) ||
            (this->VolumePropertyNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->VolumePropertyNodeID);
    vtkSetAndObserveMRMLObjectEventsMacro(
      this->VolumePropertyNode,
      vtkMRMLVolumePropertyNode::SafeDownCast(snode),
      this->ObservedEvents);
    }
  return this->VolumePropertyNode;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::SetAndObserveROINodeID(const char *ROINodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->ROINode, NULL);

  this->SetROINodeID(ROINodeID);

  vtkMRMLAnnotationROINode *node = this->GetROINode();

  vtkSetAndObserveMRMLObjectMacro(this->ROINode, node);
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLVolumeRenderingDisplayNode::GetROINode()
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

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::ProcessMRMLEvents(vtkObject *caller,
                                                          unsigned long event,
                                                          void *callData)
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);

  if (this->VolumePropertyNode != NULL &&
      this->VolumePropertyNode == vtkMRMLVolumePropertyNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  if (this->ROINode != NULL &&
      this->ROINode == vtkMRMLAnnotationROINode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }

  if (event == vtkCommand::StartEvent ||
      event == vtkCommand::EndEvent ||
      event == vtkCommand::StartInteractionEvent ||
      event == vtkCommand::InteractionEvent ||
      event == vtkCommand::EndInteractionEvent
      )
    {
    this->InvokeEvent(event);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "VolumeNodeID: " << ( (this->VolumeNodeID) ? this->VolumeNodeID : "None" ) << "\n";
  os << "ROINodeID: " << ( (this->VolumeNodeID) ? this->ROINodeID : "None" ) << "\n";
  os << "VolumePropertyNodeID: " << ( (this->VolumePropertyNodeID) ? this->VolumePropertyNodeID : "None" ) << "\n";
  os << "CroppingEnabled: " << this->CroppingEnabled << "\n";
}
