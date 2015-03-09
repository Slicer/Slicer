/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLMultiVolumeRenderingDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// Annotations includes
#include "vtkMRMLAnnotationROINode.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLMultiVolumeRenderingDisplayNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>

//#include "vtkMatrix4x4.h"
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMultiVolumeRenderingDisplayNode);

//----------------------------------------------------------------------------
vtkCxxSetReferenceStringMacro(vtkMRMLMultiVolumeRenderingDisplayNode, BgVolumeNodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLMultiVolumeRenderingDisplayNode, BgROINodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLMultiVolumeRenderingDisplayNode, BgVolumePropertyNodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLMultiVolumeRenderingDisplayNode, FgVolumeNodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLMultiVolumeRenderingDisplayNode, FgROINodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLMultiVolumeRenderingDisplayNode, FgVolumePropertyNodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLMultiVolumeRenderingDisplayNode, LabelmapVolumeNodeID);
vtkCxxSetReferenceStringMacro(vtkMRMLMultiVolumeRenderingDisplayNode, LabelmapROINodeID);

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeRenderingDisplayNode::vtkMRMLMultiVolumeRenderingDisplayNode()
{
  this->ObservedEvents = vtkIntArray::New();
  this->ObservedEvents->InsertNextValue(vtkCommand::StartEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::InteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndInteractionEvent);

  this->BgVolumeNodeID = NULL;
  this->BgVolumeNode = NULL;

  this->BgVolumePropertyNodeID = NULL;
  this->BgVolumePropertyNode = NULL;

  this->FgVolumeNodeID = NULL;
  this->FgVolumeNode = NULL;

  this->FgVolumePropertyNodeID = NULL;
  this->FgVolumePropertyNode = NULL;

  this->BgROINodeID = NULL;
  this->BgROINode = NULL;

  this->FgROINodeID = NULL;
  this->FgROINode = NULL;

  this->ExpectedFPS = 10.;
  this->EstimatedSampleDistance = 1.0;

  this->GPUMemorySize = 0; // means application default

  this->BgRaycastTechnique = vtkMRMLMultiVolumeRenderingDisplayNode::Composite;
  this->FgRaycastTechnique = vtkMRMLMultiVolumeRenderingDisplayNode::Composite;

  this->BgCroppingEnabled = 0;//by default cropping is not enabled
  this->FgCroppingEnabled = 0;//by default cropping is not enabled
  this->LabelmapCroppingEnabled = 0;//by default cropping is not enabled

  this->BgFgRatio = 0.5f;//default display bg volume

  this->MultiVolumeFusionMethod = vtkMRMLMultiVolumeRenderingDisplayNode::AlphaBlendingOR;
  this->MultiVolumeRenderingMode = vtkMRMLMultiVolumeRenderingDisplayNode::Linked;

  this->BgFollowVolumeDisplayNode = 0;// by default do not follow volume display node
  this->BgIgnoreVolumeDisplayNodeThreshold = 0;

  this->FgFollowVolumeDisplayNode = 0;// by default do not follow volume display node
  this->FgIgnoreVolumeDisplayNodeThreshold = 0;

  this->PerformanceControl = 0;

  this->SetHideFromEditors(false);

  this->BgVisibility = false;
  this->FgVisibility = false;
  this->LabelmapVisibility = false;
}

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeRenderingDisplayNode::~vtkMRMLMultiVolumeRenderingDisplayNode()
{
  if (this->BgVolumeNodeID)
    {
    SetAndObserveBgVolumeNodeID(NULL);
    }

  if (this->BgVolumePropertyNodeID)
    {
    SetAndObserveBgVolumePropertyNodeID(NULL);
    }

  if (this->BgROINodeID)
    {
    SetAndObserveBgROINodeID(NULL);
    }

  if (this->FgVolumeNodeID)
    {
    SetAndObserveFgVolumeNodeID(NULL);
    }

  if (this->FgVolumePropertyNodeID)
    {
    SetAndObserveFgVolumePropertyNodeID(NULL);
    }

  if (this->FgROINodeID)
    {
    SetAndObserveFgROINodeID(NULL);
    }

  if (this->LabelmapVolumeNodeID)
    {
    SetAndObserveLabelmapVolumeNodeID(NULL);
    }

  if (this->LabelmapROINodeID)
    {
    SetAndObserveLabelmapROINodeID(NULL);
    }

  this->ObservedEvents->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
  {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "bgVolumeNodeID"))
    {
      this->SetBgVolumeNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "fgVolumeNodeID"))
    {
      this->SetFgVolumeNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "labelmapVolumeNodeID"))
    {
      this->SetLabelmapVolumeNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "bgROINodeID"))
    {
      this->SetBgROINodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "fgROINodeID"))
    {
      this->SetFgROINodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "labelmapROINodeID"))
    {
      this->SetFgROINodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "bgVolumePropertyNodeID"))
    {
      this->SetBgVolumePropertyNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "fgVolumePropertyNodeID"))
    {
      this->SetFgVolumePropertyNodeID(attValue);
      continue;
    }
    if (!strcmp(attName,"bgCroppingEnabled"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->BgCroppingEnabled;
      continue;
    }
    if (!strcmp(attName,"fgCroppingEnabled"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->FgCroppingEnabled;
      continue;
    }
    if (!strcmp(attName,"labelmapCroppingEnabled"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LabelmapCroppingEnabled;
      continue;
    }
    if (!strcmp(attName,"bgFgRatio"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->BgFgRatio;
      continue;
    }
    if (!strcmp(attName,"BgRaycastTechnique"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->BgRaycastTechnique;
      continue;
    }
    if (!strcmp(attName,"FgRaycastTechniqueFg"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->FgRaycastTechnique;
      continue;
    }
    if (!strcmp(attName,"multiVolumeFusionMethod"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MultiVolumeFusionMethod;
      continue;
    }
    if (!strcmp(attName,"bgFollowVolumeDisplayNode"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->BgFollowVolumeDisplayNode;
      continue;
    }
    if (!strcmp(attName,"bgIgnoreVolumeDisplayNodeThreshold"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->BgIgnoreVolumeDisplayNodeThreshold;
      continue;
    }
    if (!strcmp(attName,"fgFollowVolumeDisplayNode"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->FgFollowVolumeDisplayNode;
      continue;
    }
    if (!strcmp(attName,"fgIgnoreVolumeDisplayNodeThreshold"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->FgIgnoreVolumeDisplayNodeThreshold;
      continue;
    }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " bgVolumeNodeID=\"" << (this->BgVolumeNodeID ? this->BgVolumeNodeID : "NULL") << "\"";
  of << indent << " fgVolumeNodeID=\"" << (this->FgVolumeNodeID ? this->FgVolumeNodeID : "NULL") << "\"";
  of << indent << " labelmapVolumeNodeID=\"" << (this->LabelmapVolumeNodeID ? this->LabelmapVolumeNodeID : "NULL") << "\"";

  of << indent << " bgCroppingEnabled=\""<< this->BgCroppingEnabled << "\"";
  of << indent << " fgCroppingEnabled=\""<< this->FgCroppingEnabled << "\"";
  of << indent << " labelmapCroppingEnabled=\""<< this->LabelmapCroppingEnabled << "\"";

  of << indent << " bgROINodeID=\"" << (this->BgROINodeID ? this->BgROINodeID : "NULL") << "\"";
  of << indent << " fgROINodeID=\"" << (this->FgROINodeID ? this->FgROINodeID : "NULL") << "\"";
  of << indent << " labelmapROINodeID=\"" << (this->LabelmapROINodeID ? this->LabelmapROINodeID : "NULL") << "\"";

  of << indent << " bgVolumePropertyNodeID=\"" << (this->BgVolumePropertyNodeID ? this->BgVolumePropertyNodeID : "NULL") << "\"";
  of << indent << " fgVolumePropertyNodeID=\"" << (this->FgVolumePropertyNodeID ? this->FgVolumePropertyNodeID : "NULL") << "\"";

  of << indent << " bgRaycastTechnique=\"" << this->BgRaycastTechnique << "\"";
  of << indent << " fgRaycastTechnique=\"" << this->FgRaycastTechnique << "\"";

  of << indent << " multiVolumeFusionMethod=\"" << this->MultiVolumeFusionMethod << "\"";

  of << indent << " bgFgRatio=\"" << this->BgFgRatio << "\"";

  of << indent << " bgFollowVolumeDisplayNode=\"" << this->BgFollowVolumeDisplayNode << "\"";
  of << indent << " bgIgnoreVolumeDisplayNodeThreshold=\"" << this->BgIgnoreVolumeDisplayNodeThreshold << "\"";

  of << indent << " fgFollowVolumeDisplayNode=\"" << this->FgFollowVolumeDisplayNode << "\"";
  of << indent << " fgIgnoreVolumeDisplayNodeThreshold=\"" << this->FgIgnoreVolumeDisplayNodeThreshold << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  if (this->BgVolumeNodeID && !strcmp(oldID, this->BgVolumeNodeID))
    {
    this->SetAndObserveBgVolumeNodeID(newID);
    }
  if (this->FgVolumeNodeID && !strcmp(oldID, this->FgVolumeNodeID))
    {
    this->SetAndObserveFgVolumeNodeID(newID);
    }
  if (this->LabelmapVolumeNodeID && !strcmp(oldID, this->LabelmapVolumeNodeID))
    {
    this->SetAndObserveLabelmapVolumeNodeID(newID);
    }

  if (this->BgROINodeID && !strcmp(oldID, this->BgROINodeID))
    {
    this->SetAndObserveBgROINodeID(newID);
    }
  if (this->FgROINodeID && !strcmp(oldID, this->FgROINodeID))
    {
    this->SetAndObserveFgROINodeID(newID);
    }
  if (this->LabelmapROINodeID && !strcmp(oldID, this->LabelmapROINodeID))
    {
    this->SetAndObserveLabelmapROINodeID(newID);
    }

  if (this->BgVolumePropertyNodeID && !strcmp(oldID, this->BgVolumePropertyNodeID))
    {
    this->SetAndObserveBgVolumePropertyNodeID(newID);
    }
  if (this->FgVolumePropertyNodeID && !strcmp(oldID, this->FgVolumePropertyNodeID))
    {
    this->SetAndObserveFgVolumePropertyNodeID(newID);
    }
}

//-----------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->BgVolumeNodeID != NULL && this->Scene->GetNodeByID(this->BgVolumeNodeID) == NULL)
    {
    this->SetAndObserveBgVolumeNodeID(NULL);
    }
  if (this->FgVolumeNodeID != NULL && this->Scene->GetNodeByID(this->FgVolumeNodeID) == NULL)
    {
    this->SetAndObserveFgVolumeNodeID(NULL);
    }
  if (this->LabelmapVolumeNodeID != NULL && this->Scene->GetNodeByID(this->LabelmapVolumeNodeID) == NULL)
    {
    this->SetAndObserveLabelmapVolumeNodeID(NULL);
    }

  if (this->BgROINodeID != NULL && this->Scene->GetNodeByID(this->BgROINodeID) == NULL)
    {
    this->SetAndObserveBgROINodeID(NULL);
    }
  if (this->FgROINodeID != NULL && this->Scene->GetNodeByID(this->FgROINodeID) == NULL)
    {
    this->SetAndObserveFgROINodeID(NULL);
    }
  if (this->LabelmapROINodeID != NULL && this->Scene->GetNodeByID(this->LabelmapROINodeID) == NULL)
    {
    this->SetAndObserveLabelmapROINodeID(NULL);
    }

  if (this->BgVolumePropertyNodeID != NULL && this->Scene->GetNodeByID(this->BgVolumePropertyNodeID) == NULL)
    {
    this->SetAndObserveBgVolumePropertyNodeID(NULL);
    }
  if (this->FgVolumePropertyNodeID != NULL && this->Scene->GetNodeByID(this->FgVolumePropertyNodeID) == NULL)
    {
    this->SetAndObserveFgVolumePropertyNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLMultiVolumeRenderingDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify(); 

  Superclass::Copy(anode);
  vtkMRMLMultiVolumeRenderingDisplayNode *node = vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(anode);

  this->SetBgVolumeNodeID(node->GetBgVolumeNodeID());
  this->SetFgVolumeNodeID(node->GetFgVolumeNodeID());
  this->SetLabelmapVolumeNodeID(node->GetLabelmapVolumeNodeID());

  this->SetBgVolumePropertyNodeID(node->GetBgVolumePropertyNodeID());
  this->SetFgVolumePropertyNodeID(node->GetFgVolumePropertyNodeID());

  this->SetBgROINodeID(node->GetBgROINodeID());
  this->SetFgROINodeID(node->GetFgROINodeID());
  this->SetLabelmapROINodeID(node->GetLabelmapROINodeID());

  this->SetBgCroppingEnabled(node->GetBgCroppingEnabled());
  this->SetFgCroppingEnabled(node->GetFgCroppingEnabled());
  this->SetLabelmapCroppingEnabled(node->GetLabelmapCroppingEnabled());

  this->SetGPUMemorySize(node->GetGPUMemorySize());
  this->SetEstimatedSampleDistance(node->GetEstimatedSampleDistance());

  this->SetBgRaycastTechnique(node->GetBgRaycastTechnique());
  this->SetFgRaycastTechnique(node->GetFgRaycastTechnique());

  this->SetBgFgRatio(node->GetBgFgRatio());
  this->SetMultiVolumeFusionMethod(node->GetMultiVolumeFusionMethod());

  this->SetBgFollowVolumeDisplayNode(node->GetBgFollowVolumeDisplayNode());
  this->SetBgIgnoreVolumeDisplayNodeThreshold(node->GetBgIgnoreVolumeDisplayNodeThreshold());

  this->SetFgFollowVolumeDisplayNode(node->GetFgFollowVolumeDisplayNode());
  this->SetFgIgnoreVolumeDisplayNodeThreshold(node->GetFgIgnoreVolumeDisplayNodeThreshold());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::SetAndObserveBgVolumeNodeID(const char *volumeNodeID)
{
  this->SetBgVolumeNodeID(volumeNodeID);
  this->BgVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
    this->GetScene() ? this->GetScene()->GetNodeByID(volumeNodeID) : 0);
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::SetAndObserveFgVolumeNodeID(const char *volumeNodeID)
{
  this->SetFgVolumeNodeID(volumeNodeID);
  this->FgVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
    this->GetScene() ? this->GetScene()->GetNodeByID(volumeNodeID) : 0);
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::SetAndObserveLabelmapVolumeNodeID(const char *volumeNodeID)
{
  this->SetLabelmapVolumeNodeID(volumeNodeID);
  this->LabelmapVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
    this->GetScene() ? this->GetScene()->GetNodeByID(volumeNodeID) : 0);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLMultiVolumeRenderingDisplayNode::GetBgVolumeNode()
{
  if (this->BgVolumeNodeID == NULL)
    {
    this->BgVolumeNode = NULL;
    }
  if (((this->BgVolumeNode != NULL && this->BgVolumeNodeID && strcmp(this->BgVolumeNode->GetID(), this->BgVolumeNodeID)) ||
      (this->BgVolumeNode == NULL)) )
    {
    this->BgVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
      this->GetScene() ? this->GetScene()->GetNodeByID(this->BgVolumeNodeID) : 0);
    }
  return this->BgVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLMultiVolumeRenderingDisplayNode::GetFgVolumeNode()
{
  if (this->FgVolumeNodeID == NULL)
    {
    this->FgVolumeNode = NULL;
    }
  if (((this->FgVolumeNode != NULL && strcmp(this->FgVolumeNode->GetID(), this->FgVolumeNodeID)) ||
      (this->FgVolumeNode == NULL)) )
    {
    this->FgVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
      this->GetScene() ? this->GetScene()->GetNodeByID(this->FgVolumeNodeID) : 0);
    }
  return this->FgVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLMultiVolumeRenderingDisplayNode::GetLabelmapVolumeNode()
{
  if (this->LabelmapVolumeNodeID == NULL)
    {
    this->LabelmapVolumeNode = NULL;
    }
  if (((this->LabelmapVolumeNode != NULL && strcmp(this->LabelmapVolumeNode->GetID(), this->LabelmapVolumeNodeID)) ||
      (this->LabelmapVolumeNode == NULL)) )
    {
    this->LabelmapVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
      this->GetScene() ? this->GetScene()->GetNodeByID(this->LabelmapVolumeNodeID) : 0);
    }
  return this->LabelmapVolumeNode;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode
::SetAndObserveBgVolumePropertyNodeID(const char *volumePropertyNodeID)
{
  this->SetBgVolumePropertyNodeID(volumePropertyNodeID);
  vtkMRMLVolumePropertyNode *node = this->GetBgVolumePropertyNode();
  if (node != this->BgVolumePropertyNode)
    {
    vtkSetAndObserveMRMLObjectEventsMacro(this->BgVolumePropertyNode, node, this->ObservedEvents);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode
::SetAndObserveFgVolumePropertyNodeID(const char *VolumePropertyNodeID)
{
  this->SetFgVolumePropertyNodeID(VolumePropertyNodeID);
  vtkMRMLVolumePropertyNode *node = this->GetFgVolumePropertyNode();

  if (node != this->FgVolumePropertyNode)
  {
    vtkSetAndObserveMRMLObjectEventsMacro(this->FgVolumePropertyNode, node, this->ObservedEvents);
  }
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLMultiVolumeRenderingDisplayNode::GetBgVolumePropertyNode()
{
  if (this->BgVolumePropertyNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectEventsMacro(this->BgVolumePropertyNode, NULL, this->ObservedEvents);
    }
  else if (this->GetScene() &&
           ((this->BgVolumePropertyNode != NULL &&
            strcmp(this->BgVolumePropertyNode->GetID(), this->BgVolumePropertyNodeID)) ||
            (this->BgVolumePropertyNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->BgVolumePropertyNodeID);
    vtkSetAndObserveMRMLObjectEventsMacro(
      this->BgVolumePropertyNode,
      vtkMRMLVolumePropertyNode::SafeDownCast(snode),
      this->ObservedEvents);
    }
  return this->BgVolumePropertyNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLMultiVolumeRenderingDisplayNode::GetFgVolumePropertyNode()
{
  if (this->FgVolumePropertyNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectEventsMacro(this->FgVolumePropertyNode, NULL, this->ObservedEvents);
    }
  else if (this->GetScene() &&
           ((this->FgVolumePropertyNode != NULL && strcmp(this->FgVolumePropertyNode->GetID(), this->FgVolumePropertyNodeID)) ||
            (this->FgVolumePropertyNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->FgVolumePropertyNodeID);
    vtkSetAndObserveMRMLObjectEventsMacro(
      this->FgVolumePropertyNode,
      vtkMRMLVolumePropertyNode::SafeDownCast(snode), this->ObservedEvents);
    }
  return this->FgVolumePropertyNode;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::SetAndObserveBgROINodeID(const char *ROINodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->BgROINode, NULL);

  this->SetBgROINodeID(ROINodeID);

  vtkMRMLAnnotationROINode *node = this->GetBgROINode();

  vtkSetAndObserveMRMLObjectMacro(this->BgROINode, node);
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLMultiVolumeRenderingDisplayNode::GetBgROINode()
{
  if (this->BgROINodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->BgROINode, NULL);
    }
  else if (this->GetScene() &&
           ((this->BgROINode != NULL && strcmp(this->BgROINode->GetID(), this->BgROINodeID)) ||
            (this->BgROINode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->BgROINodeID);
    vtkSetAndObserveMRMLObjectMacro(this->BgROINode, vtkMRMLAnnotationROINode::SafeDownCast(snode));
    }
  return this->BgROINode;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::SetAndObserveLabelmapROINodeID(const char *ROINodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->LabelmapROINode, NULL);

  this->SetLabelmapROINodeID(ROINodeID);

  vtkMRMLAnnotationROINode *node = this->GetLabelmapROINode();

  vtkSetAndObserveMRMLObjectMacro(this->LabelmapROINode, node);
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLMultiVolumeRenderingDisplayNode::GetLabelmapROINode()
{
  if (this->LabelmapROINodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->LabelmapROINode, NULL);
    }
  else if (this->GetScene() &&
           ((this->LabelmapROINode != NULL && strcmp(this->LabelmapROINode->GetID(), this->LabelmapROINodeID)) ||
            (this->LabelmapROINode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->LabelmapROINodeID);
    vtkSetAndObserveMRMLObjectMacro(this->LabelmapROINode, vtkMRMLAnnotationROINode::SafeDownCast(snode));
    }
  return this->LabelmapROINode;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::SetAndObserveFgROINodeID(const char *ROINodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->FgROINode, NULL);

  this->SetFgROINodeID(ROINodeID);

  vtkMRMLAnnotationROINode *node = this->GetFgROINode();

  vtkSetAndObserveMRMLObjectMacro(this->FgROINode, node);
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLMultiVolumeRenderingDisplayNode::GetFgROINode()
{
  if (this->FgROINodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->FgROINode, NULL);
    }
  else if (this->GetScene() &&
           ((this->FgROINode != NULL && strcmp(this->FgROINode->GetID(), this->FgROINodeID)) ||
            (this->FgROINode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->FgROINodeID);
    vtkSetAndObserveMRMLObjectMacro(this->FgROINode, vtkMRMLAnnotationROINode::SafeDownCast(snode));
    }
  return this->FgROINode;
}

//-----------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  this->SetAndObserveBgVolumeNodeID(this->BgVolumeNodeID);
  this->SetAndObserveBgVolumePropertyNodeID(this->BgVolumePropertyNodeID);
  this->SetAndObserveFgVolumeNodeID(this->FgVolumeNodeID);
  this->SetAndObserveFgVolumePropertyNodeID(this->FgVolumePropertyNodeID);

  this->SetAndObserveLabelmapVolumeNodeID(this->LabelmapVolumeNodeID);

  this->SetAndObserveBgROINodeID(this->BgROINodeID);
  this->SetAndObserveFgROINodeID(this->FgROINodeID);
  this->SetAndObserveLabelmapROINodeID(this->LabelmapROINodeID);
}

//---------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
//  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  if (event == vtkCommand::StartEvent ||
      // ModifiedEvent is already forwarded by Superclass::ProcessMRMLEvents
      event == vtkCommand::EndEvent ||
      event == vtkCommand::StartInteractionEvent ||
      event == vtkCommand::InteractionEvent ||
      event == vtkCommand::EndInteractionEvent
      )
    {
    this->InvokeEvent(event);
    }

  return;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeRenderingDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "BgVolumeNodeID: " << ( (this->BgVolumeNodeID) ? this->BgVolumeNodeID : "None" ) << "\n";
  os << "FgVolumeNodeID: " << ( (this->FgVolumeNodeID) ? this->FgVolumeNodeID : "None" ) << "\n";
  os << "LabelmapVolumeNodeID: " << ( (this->LabelmapVolumeNodeID) ? this->LabelmapVolumeNodeID : "None" ) << "\n";

  os << "BgROINodeID: " << ( (this->BgVolumeNodeID) ? this->BgROINodeID : "None" ) << "\n";
  os << "FgROINodeID: " << ( (this->FgVolumeNodeID) ? this->FgROINodeID : "None" ) << "\n";
  os << "LabelmapROINodeID: " << ( (this->LabelmapVolumeNodeID) ? this->LabelmapROINodeID : "None" ) << "\n";

  os << "BgVolumePropertyNodeID: " << ( (this->BgVolumePropertyNodeID) ? this->BgVolumePropertyNodeID : "None" ) << "\n";
  os << "FgVolumePropertyNodeID: " << ( (this->FgVolumePropertyNodeID) ? this->FgVolumePropertyNodeID : "None" ) << "\n";

  os << "BgCroppingEnabled: " << this->BgCroppingEnabled << "\n";
  os << "FgCroppingEnabled: " << this->FgCroppingEnabled << "\n";
  os << "LabelmapCroppingEnabled: " << this->LabelmapCroppingEnabled << "\n";
}

void vtkMRMLMultiVolumeRenderingDisplayNode::UpdateVisibility()
{
  if (BgVisibility || FgVisibility || LabelmapVisibility)
    this->SetVisibility(true);
  else
    this->SetVisibility(false);
}

// End
