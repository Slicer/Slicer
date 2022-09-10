/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeRenderingDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLShaderPropertyNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeRenderingDisplayNode::VolumePropertyNodeReferenceRole = "volumeProperty";
const char* vtkMRMLVolumeRenderingDisplayNode::VolumePropertyNodeReferenceMRMLAttributeName = "volumePropertyNodeID";
const char* vtkMRMLVolumeRenderingDisplayNode::ROINodeReferenceRole = "roi";
const char* vtkMRMLVolumeRenderingDisplayNode::ROINodeReferenceMRMLAttributeName = "ROINodeID";
const char* vtkMRMLVolumeRenderingDisplayNode::ShaderPropertyNodeReferenceRole = "shaderProperty";
const char* vtkMRMLVolumeRenderingDisplayNode::ShaderPropertyNodeReferenceMRMLAttributeName = "shaderPropertyNodeId";

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode::vtkMRMLVolumeRenderingDisplayNode()
{
  vtkNew<vtkIntArray> volumePropertyEvents;
  volumePropertyEvents->InsertNextValue(vtkCommand::StartEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::EndEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::InteractionEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::EndInteractionEvent);
  this->AddNodeReferenceRole(VolumePropertyNodeReferenceRole,
                             VolumePropertyNodeReferenceMRMLAttributeName,
                             volumePropertyEvents.GetPointer());

  vtkNew<vtkIntArray> roiEvents;
  roiEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(ROINodeReferenceRole,
                             ROINodeReferenceMRMLAttributeName,
                             roiEvents.GetPointer());

  vtkNew<vtkIntArray> shaderPropertyEvents;
  shaderPropertyEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(ShaderPropertyNodeReferenceRole,
                             ShaderPropertyNodeReferenceMRMLAttributeName,
                             shaderPropertyEvents.GetPointer());

  this->CroppingEnabled = 0;//by default cropping is not enabled

  this->Threshold[0] = 0.0;
  this->Threshold[1] = 1.0;

  this->FollowVolumeDisplayNode = 0;// by default do not follow volume display node
  this->IgnoreVolumeDisplayNodeThreshold = 0;
  this->UseSingleVolumeProperty = 0;

  this->WindowLevel[0] = 0.0;
  this->WindowLevel[1] = 0.0;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode::~vtkMRMLVolumeRenderingDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro(croppingEnabled, CroppingEnabled);
  vtkMRMLReadXMLVectorMacro(threshold, Threshold, double, 2);
  vtkMRMLReadXMLVectorMacro(windowLevel, WindowLevel, double, 2);
  vtkMRMLReadXMLIntMacro(followVolumeDisplayNode, FollowVolumeDisplayNode);
  vtkMRMLReadXMLIntMacro(ignoreVolumeDisplayNodeThreshold, IgnoreVolumeDisplayNodeThreshold);
  vtkMRMLReadXMLIntMacro(useSingleVolumeProperty, UseSingleVolumeProperty);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLIntMacro(croppingEnabled, CroppingEnabled);
  vtkMRMLWriteXMLVectorMacro(threshold, Threshold, double, 2);
  vtkMRMLWriteXMLVectorMacro(windowLevel, WindowLevel, double, 2);
  vtkMRMLWriteXMLIntMacro(followVolumeDisplayNode, FollowVolumeDisplayNode);
  vtkMRMLWriteXMLIntMacro(ignoreVolumeDisplayNodeThreshold, IgnoreVolumeDisplayNodeThreshold);
  vtkMRMLWriteXMLIntMacro(useSingleVolumeProperty, UseSingleVolumeProperty);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLVolumeRenderingDisplayNode::Copy(vtkMRMLNode *anode)
{
  int wasModifying = this->StartModify();
  this->Superclass::Copy(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyIntMacro(CroppingEnabled);
  vtkMRMLCopyVectorMacro(Threshold, double, 2);
  vtkMRMLCopyVectorMacro(WindowLevel, double, 2);
  vtkMRMLCopyIntMacro(FollowVolumeDisplayNode);
  vtkMRMLCopyIntMacro(IgnoreVolumeDisplayNodeThreshold);
  vtkMRMLCopyIntMacro(UseSingleVolumeProperty);
  vtkMRMLCopyEndMacro();

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os,indent);
  vtkMRMLPrintIntMacro(CroppingEnabled);
  vtkMRMLPrintVectorMacro(Threshold, double, 2);
  vtkMRMLPrintVectorMacro(WindowLevel, double, 2);
  vtkMRMLPrintIntMacro(FollowVolumeDisplayNode);
  vtkMRMLPrintIntMacro(IgnoreVolumeDisplayNodeThreshold);
  vtkMRMLPrintIntMacro(UseSingleVolumeProperty);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeRenderingDisplayNode::GetVolumeNodeID()
{
  vtkMRMLDisplayableNode* volumeNode = this->GetDisplayableNode();
  return (volumeNode ? volumeNode->GetID() : nullptr);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeRenderingDisplayNode::GetVolumeNode()
{
  return vtkMRMLVolumeNode::SafeDownCast(this->GetDisplayableNode());
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::SetAndObserveVolumePropertyNodeID(
  const char* volumePropertyNodeID)
{
  this->SetAndObserveNodeReferenceID(VolumePropertyNodeReferenceRole, volumePropertyNodeID);
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeRenderingDisplayNode::GetVolumePropertyNodeID()
{
  return this->GetNodeReferenceID(VolumePropertyNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLVolumeRenderingDisplayNode::GetVolumePropertyNode()
{
  return vtkMRMLVolumePropertyNode::SafeDownCast(
    this->GetNodeReference(VolumePropertyNodeReferenceRole));
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeRenderingDisplayNode::GetShaderPropertyNodeID()
{
    return this->GetNodeReferenceID(ShaderPropertyNodeReferenceRole);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::SetAndObserveShaderPropertyNodeID(const char *shaderPropertyNodeID)
{
    this->SetAndObserveNodeReferenceID(ShaderPropertyNodeReferenceRole, shaderPropertyNodeID);
}

//----------------------------------------------------------------------------
vtkMRMLShaderPropertyNode* vtkMRMLVolumeRenderingDisplayNode::GetShaderPropertyNode()
{
    return vtkMRMLShaderPropertyNode::SafeDownCast(
      this->GetNodeReference(ShaderPropertyNodeReferenceRole));
}

//----------------------------------------------------------------------------
vtkMRMLShaderPropertyNode* vtkMRMLVolumeRenderingDisplayNode::GetOrCreateShaderPropertyNode( vtkMRMLScene * mrmlScene )
{
  vtkMRMLShaderPropertyNode * sp = this->GetShaderPropertyNode();
  if( sp == nullptr )
    {
    vtkNew<vtkMRMLShaderPropertyNode> shaderNode;
    mrmlScene->AddNode(shaderNode);
    this->SetAndObserveShaderPropertyNodeID(shaderNode->GetID());
    sp = shaderNode.GetPointer();
    }
  return sp;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::SetAndObserveROINodeID(const char* roiNodeID)
{
  this->SetAndObserveNodeReferenceID(ROINodeReferenceRole, roiNodeID);
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeRenderingDisplayNode::GetROINodeID()
{
  return this->GetNodeReferenceID(ROINodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode* vtkMRMLVolumeRenderingDisplayNode::GetROINode()
{
  return vtkMRMLDisplayableNode::SafeDownCast(this->GetNodeReference(ROINodeReferenceRole));
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsROINode* vtkMRMLVolumeRenderingDisplayNode::GetMarkupsROINode()
{
  return vtkMRMLMarkupsROINode::SafeDownCast(this->GetNodeReference(ROINodeReferenceRole));
}

//-----------------------------------------------------------------------------
vtkMRMLViewNode* vtkMRMLVolumeRenderingDisplayNode::GetFirstViewNode()
{
  if (!this->GetScene())
    {
    return nullptr;
    }

  std::vector<vtkMRMLNode*> viewNodes;
  this->GetScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    if (this->IsDisplayableInView((*it)->GetID()))
      {
      return vtkMRMLViewNode::SafeDownCast(*it);
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
double vtkMRMLVolumeRenderingDisplayNode::GetSampleDistance()
{
  vtkMRMLViewNode* firstViewNode = this->GetFirstViewNode();
  if (!firstViewNode)
    {
    vtkErrorMacro("GetSampleDistance: Failed to access view node, returning 1mm");
    return 1.0;
    }
  vtkMRMLVolumeNode* volumeNode = this->GetVolumeNode();
  if (!volumeNode)
    {
    vtkErrorMacro("GetSampleDistance: Failed to access volume node, assuming 1mm voxel size");
    return 1.0 / firstViewNode->GetVolumeRenderingOversamplingFactor();
    }

  const double minSpacing = volumeNode->GetMinSpacing() > 0 ? volumeNode->GetMinSpacing() : 1.;
  double sampleDistance = minSpacing / firstViewNode->GetVolumeRenderingOversamplingFactor();
  if ( firstViewNode
    && firstViewNode->GetVolumeRenderingQuality() == vtkMRMLViewNode::Maximum)
    {
    sampleDistance = minSpacing / 10.; // =10x smaller than pixel is high quality
    }
  return sampleDistance;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::ProcessMRMLEvents(vtkObject *caller,
                                                          unsigned long event,
                                                          void *callData)
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLVolumePropertyNode* volumePropertyNode = this->GetVolumePropertyNode();
  if (volumePropertyNode != nullptr &&
      volumePropertyNode == vtkMRMLVolumePropertyNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
  vtkMRMLShaderPropertyNode* shaderPropertyNode = this->GetShaderPropertyNode();
  if (shaderPropertyNode != nullptr &&
      shaderPropertyNode == vtkMRMLShaderPropertyNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
  vtkMRMLMarkupsROINode* markupRoiNode = this->GetMarkupsROINode();
  if (markupRoiNode != nullptr &&
      markupRoiNode == vtkMRMLMarkupsROINode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
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
