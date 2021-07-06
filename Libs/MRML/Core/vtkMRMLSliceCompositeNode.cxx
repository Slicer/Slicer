/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSliceCompositeNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

// STD includes
#include <sstream>

static const char* BackgroundVolumeNodeReferenceRole = "backgroundVolume";
static const char* BackgroundVolumeNodeReferenceMRMLAttributeName = "backgroundVolumeID";
static const char* ForegroundVolumeNodeReferenceRole = "foregroundVolume";
static const char* ForegroundVolumeNodeReferenceMRMLAttributeName = "foregroundVolumeID";
static const char* LabelVolumeNodeReferenceRole = "labelVolume";
static const char* LabelVolumeNodeReferenceMRMLAttributeName = "labelVolumeID";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSliceCompositeNode);

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::vtkMRMLSliceCompositeNode()
{
  this->HideFromEditors = 1;

  this->AddNodeReferenceRole(BackgroundVolumeNodeReferenceRole, BackgroundVolumeNodeReferenceMRMLAttributeName);
  this->AddNodeReferenceRole(ForegroundVolumeNodeReferenceRole, ForegroundVolumeNodeReferenceMRMLAttributeName);
  this->AddNodeReferenceRole(LabelVolumeNodeReferenceRole, LabelVolumeNodeReferenceMRMLAttributeName);
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::~vtkMRMLSliceCompositeNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLIntMacro(compositing, Compositing);
  vtkMRMLWriteXMLFloatMacro(foregroundOpacity, ForegroundOpacity);
  vtkMRMLWriteXMLFloatMacro(labelOpacity, LabelOpacity);
  vtkMRMLWriteXMLIntMacro(linkedControl, LinkedControl);
  vtkMRMLWriteXMLIntMacro(hotLinkedControl, HotLinkedControl);
  vtkMRMLWriteXMLIntMacro(fiducialVisibility, FiducialVisibility);
  vtkMRMLWriteXMLIntMacro(fiducialLabelVisibility, FiducialLabelVisibility);
  vtkMRMLWriteXMLStringMacro(layoutName, LayoutName);
  vtkMRMLWriteXMLEnumMacro(annotationSpace, AnnotationSpace);
  vtkMRMLWriteXMLEnumMacro(annotationMode, AnnotationMode);
  vtkMRMLWriteXMLIntMacro(doPropagateVolumeSelection, DoPropagateVolumeSelection);
  vtkMRMLWriteXMLEndMacro();
}

//-----------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetAnnotationSpaceAsString(int id)
    {
  switch (id)
  {
  case vtkMRMLSliceCompositeNode::XYZ: return "xyz";
  case vtkMRMLSliceCompositeNode::IJK: return "ijk";
  case vtkMRMLSliceCompositeNode::RAS: return "RAS";
  case vtkMRMLSliceCompositeNode::IJKAndRAS: return "IJKAndRAS";
  default:
    // invalid id
    return "";
  }
    }

//-----------------------------------------------------------
int vtkMRMLSliceCompositeNode::GetAnnotationSpaceFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < AnnotationSpace_Last; i++)
    {
    if (strcmp(name, this->GetAnnotationSpaceAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetAnnotationModeAsString(int id)
{
  switch (id)
    {
    case vtkMRMLSliceCompositeNode::NoAnnotation: return "NoAnnotation";
    case vtkMRMLSliceCompositeNode::All: return "All";
    case vtkMRMLSliceCompositeNode::LabelValuesOnly: return "LabelValuesOnly";
    case vtkMRMLSliceCompositeNode::LabelAndVoxelValuesOnly: return "LabelAndVoxelValuesOnly";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLSliceCompositeNode::GetAnnotationModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < AnnotationMode_Last; i++)
    {
    if (strcmp(name, this->GetAnnotationModeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetInteracting(int interacting)
{
  // Don't call Modified()
  this->Interacting = interacting;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetInteractionFlags(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlags = flags;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetInteractionFlagsModifier(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlagsModifier = flags;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::ResetInteractionFlagsModifier()
{
  // Don't call Modified()
  this->InteractionFlagsModifier = (unsigned int) -1;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro(compositing, Compositing);
  vtkMRMLReadXMLFloatMacro(foregroundOpacity, ForegroundOpacity);
  vtkMRMLReadXMLFloatMacro(labelOpacity, LabelOpacity);
  vtkMRMLReadXMLIntMacro(linkedControl, LinkedControl);
  vtkMRMLReadXMLIntMacro(hotLinkedControl, HotLinkedControl);
  vtkMRMLReadXMLIntMacro(fiducialVisibility, FiducialVisibility);
  vtkMRMLReadXMLIntMacro(fiducialLabelVisibility, FiducialLabelVisibility);
  vtkMRMLReadXMLStringMacro(layoutName, LayoutName);
  vtkMRMLReadXMLEnumMacro(annotationSpace, AnnotationSpace);
  vtkMRMLReadXMLEnumMacro(annotationMode, AnnotationMode);
  vtkMRMLReadXMLIntMacro(doPropagateVolumeSelection, DoPropagateVolumeSelection);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLSliceCompositeNode *node = vtkMRMLSliceCompositeNode::SafeDownCast(anode);

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyIntMacro(Compositing);
  vtkMRMLCopyFloatMacro(ForegroundOpacity);
  vtkMRMLCopyFloatMacro(LabelOpacity);
  vtkMRMLCopyIntMacro(LinkedControl);
  vtkMRMLCopyIntMacro(HotLinkedControl);
  vtkMRMLCopyIntMacro(FiducialVisibility);
  vtkMRMLCopyIntMacro(FiducialLabelVisibility);
  // To avoid breaking current implementation, copy of the "LayoutName" attribute
  // will be enabled after revisiting the view initialization pipeline.
  //vtkMRMLCopyStringMacro(LayoutName);
  vtkMRMLCopyEnumMacro(AnnotationSpace);
  vtkMRMLCopyEnumMacro(AnnotationMode);
  vtkMRMLCopyIntMacro(DoPropagateVolumeSelection);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStringMacro(BackgroundVolumeID);
  vtkMRMLPrintStringMacro(ForegroundVolumeID);
  vtkMRMLPrintStringMacro(LabelVolumeID);
  vtkMRMLPrintIntMacro(Compositing);
  vtkMRMLPrintFloatMacro(ForegroundOpacity);
  vtkMRMLPrintFloatMacro(LabelOpacity);
  vtkMRMLPrintIntMacro(LinkedControl);
  vtkMRMLPrintIntMacro(HotLinkedControl);
  vtkMRMLPrintIntMacro(FiducialVisibility);
  vtkMRMLPrintIntMacro(FiducialLabelVisibility);
  vtkMRMLPrintStringMacro(LayoutName);
  vtkMRMLPrintEnumMacro(AnnotationSpace);
  vtkMRMLPrintEnumMacro(AnnotationMode);
  vtkMRMLPrintIntMacro(DoPropagateVolumeSelection);
  vtkMRMLPrintEndMacro();

  os << indent << "Interacting: " <<
    (this->Interacting ? "on" : "off") << "\n";
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetBackgroundVolumeID(const char* id)
{
  this->SetNodeReferenceID(BackgroundVolumeNodeReferenceRole, id);
}

//-----------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetBackgroundVolumeID()
{
  return this->GetNodeReferenceID(BackgroundVolumeNodeReferenceRole);
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetForegroundVolumeID(const char* id)
{
  this->SetNodeReferenceID(ForegroundVolumeNodeReferenceRole, id);
}

//-----------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetForegroundVolumeID()
{
  return this->GetNodeReferenceID(ForegroundVolumeNodeReferenceRole);
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetLabelVolumeID(const char* id)
{
  this->SetNodeReferenceID(LabelVolumeNodeReferenceRole, id);
}

//-----------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetLabelVolumeID()
{
  return this->GetNodeReferenceID(LabelVolumeNodeReferenceRole);
}
