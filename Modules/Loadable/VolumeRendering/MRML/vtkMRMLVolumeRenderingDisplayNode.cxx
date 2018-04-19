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

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeRenderingDisplayNode::VolumeNodeReferenceRole = "volume";
const char* vtkMRMLVolumeRenderingDisplayNode::VolumeNodeReferenceMRMLAttributeName = "volumeNodeID";
const char* vtkMRMLVolumeRenderingDisplayNode::VolumePropertyNodeReferenceRole = "volumeProperty";
const char* vtkMRMLVolumeRenderingDisplayNode::VolumePropertyNodeReferenceMRMLAttributeName = "volumePropertyNodeID";
const char* vtkMRMLVolumeRenderingDisplayNode::ROINodeReferenceRole = "roi";
const char* vtkMRMLVolumeRenderingDisplayNode::ROINodeReferenceMRMLAttributeName = "ROINodeID";

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode::vtkMRMLVolumeRenderingDisplayNode()
{
  this->AddNodeReferenceRole(VolumeNodeReferenceRole,
                             VolumeNodeReferenceMRMLAttributeName);

  vtkIntArray* volumePropertyEvents = vtkIntArray::New();
  volumePropertyEvents->InsertNextValue(vtkCommand::StartEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::EndEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::InteractionEvent);
  volumePropertyEvents->InsertNextValue(vtkCommand::EndInteractionEvent);
  this->AddNodeReferenceRole(VolumePropertyNodeReferenceRole,
                             VolumePropertyNodeReferenceMRMLAttributeName,
                             volumePropertyEvents);
  volumePropertyEvents->Delete();

  vtkIntArray* roiEvents = vtkIntArray::New();
  roiEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(ROINodeReferenceRole,
                             ROINodeReferenceMRMLAttributeName,
                             roiEvents);
  roiEvents->Delete();

  this->ExpectedFPS = 8.;
  this->EstimatedSampleDistance = 2.0;

  this->GPUMemorySize = 0; // means application default

  this->CroppingEnabled = 0;//by default cropping is not enabled

  this->Threshold[0] = 0.0;
  this->Threshold[1] = 1.0;

  this->FollowVolumeDisplayNode = 0;// by default do not follow volume display node
  this->IgnoreVolumeDisplayNodeThreshold = 0;
  this->UseSingleVolumeProperty = 0;

  this->WindowLevel[0] = 0.0;
  this->WindowLevel[1] = 0.0;

  this->PerformanceControl = vtkMRMLVolumeRenderingDisplayNode::AdaptiveQuality;
  this->RaycastTechnique = vtkMRMLVolumeRenderingDisplayNode::Composite;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode::~vtkMRMLVolumeRenderingDisplayNode()
{
}

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
  vtkMRMLReadXMLIntMacro(gpuMemorySize, GPUMemorySize);
  vtkMRMLReadXMLFloatMacro(estimatedSampleDistance, EstimatedSampleDistance);
  vtkMRMLReadXMLFloatMacro(expectedFPS, ExpectedFPS);
  vtkMRMLReadXMLIntMacro(performanceControl, PerformanceControl);
  vtkMRMLReadXMLIntMacro(raycastTechnique, RaycastTechnique);
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
  vtkMRMLWriteXMLIntMacro(gpuMemorySize, GPUMemorySize);
  vtkMRMLWriteXMLFloatMacro(estimatedSampleDistance, EstimatedSampleDistance);
  vtkMRMLWriteXMLFloatMacro(expectedFPS, ExpectedFPS);
  vtkMRMLWriteXMLIntMacro(performanceControl, PerformanceControl);
  vtkMRMLWriteXMLIntMacro(raycastTechnique, RaycastTechnique);
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
  vtkMRMLCopyIntMacro(GPUMemorySize);
  vtkMRMLCopyFloatMacro(EstimatedSampleDistance);
  vtkMRMLCopyFloatMacro(ExpectedFPS);
  vtkMRMLCopyIntMacro(PerformanceControl);
  vtkMRMLCopyIntMacro(RaycastTechnique);
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
  vtkMRMLPrintIntMacro(GPUMemorySize);
  vtkMRMLPrintFloatMacro(EstimatedSampleDistance);
  vtkMRMLPrintFloatMacro(ExpectedFPS);
  vtkMRMLPrintIntMacro(PerformanceControl);
  vtkMRMLPrintIntMacro(RaycastTechnique);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::SetAndObserveVolumeNodeID(const char* volumeNodeID)
{
  this->SetAndObserveNodeReferenceID(VolumeNodeReferenceRole, volumeNodeID);
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeRenderingDisplayNode::GetVolumeNodeID()
{
  return this->GetNodeReferenceID(VolumeNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeRenderingDisplayNode::GetVolumeNode()
{
  return vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(VolumeNodeReferenceRole));
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
vtkMRMLAnnotationROINode* vtkMRMLVolumeRenderingDisplayNode::GetROINode()
{
  return vtkMRMLAnnotationROINode::SafeDownCast(this->GetNodeReference(ROINodeReferenceRole));
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::ProcessMRMLEvents(vtkObject *caller,
                                                          unsigned long event,
                                                          void *callData)
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLVolumePropertyNode* volumePropertyNode = this->GetVolumePropertyNode();
  if (volumePropertyNode != NULL &&
      volumePropertyNode == vtkMRMLVolumePropertyNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  vtkMRMLAnnotationROINode* roiNode = this->GetROINode();
  if (roiNode != NULL &&
      roiNode == vtkMRMLAnnotationROINode::SafeDownCast(caller) &&
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
