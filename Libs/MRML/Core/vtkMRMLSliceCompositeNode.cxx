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

// STD includes
#include <sstream>

vtkCxxSetReferenceStringMacro(vtkMRMLSliceCompositeNode, BackgroundVolumeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSliceCompositeNode, ForegroundVolumeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSliceCompositeNode, LabelVolumeID);

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSliceCompositeNode);

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::vtkMRMLSliceCompositeNode()
{
  this->HideFromEditors = 1;

  this->BackgroundVolumeID = nullptr;
  this->ForegroundVolumeID = nullptr;
  this->LabelVolumeID = nullptr;
  this->Compositing = 0;
  this->ForegroundOpacity = 0.0; // start by showing only the background volume
  this->LabelOpacity = 1.0; // Show the label if there is one
  this->LinkedControl = 0;
  this->FiducialVisibility = 1;
  this->FiducialLabelVisibility = 1;
  this->AnnotationSpace = vtkMRMLSliceCompositeNode::IJKAndRAS;
  this->AnnotationMode = vtkMRMLSliceCompositeNode::All;
  this->SliceIntersectionVisibility = 0;
  this->DoPropagateVolumeSelection = true;
  this->Interacting = 0;
  this->InteractionFlags = 0;
  this->HotLinkedControl = 0;
  this->InteractionFlagsModifier = (unsigned int) -1;
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::~vtkMRMLSliceCompositeNode()
{
  if (this->BackgroundVolumeID)
    {
    this->SetBackgroundVolumeID(nullptr);
    }
  if (this->ForegroundVolumeID)
    {
    this->SetForegroundVolumeID(nullptr);
    }
  if (this->LabelVolumeID)
    {
    this->SetLabelVolumeID(nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLStringMacro(backgroundVolumeID, BackgroundVolumeID);
  vtkMRMLWriteXMLStringMacro(foregroundVolumeID, ForegroundVolumeID);
  vtkMRMLWriteXMLStringMacro(labelVolumeID, LabelVolumeID);
  vtkMRMLWriteXMLIntMacro(compositing, Compositing);
  vtkMRMLWriteXMLFloatMacro(foregroundOpacity, ForegroundOpacity);
  vtkMRMLWriteXMLFloatMacro(labelOpacity, LabelOpacity);
  vtkMRMLWriteXMLIntMacro(linkedControl, LinkedControl);
  vtkMRMLWriteXMLIntMacro(fiducialVisibility, FiducialVisibility);
  vtkMRMLWriteXMLIntMacro(fiducialLabelVisibility, FiducialLabelVisibility);
  vtkMRMLWriteXMLIntMacro(sliceIntersectionVisibility, SliceIntersectionVisibility);
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

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetSceneReferences()
{
   Superclass::SetSceneReferences();
   this->Scene->AddReferencedNodeID(this->BackgroundVolumeID, this);
   this->Scene->AddReferencedNodeID(this->ForegroundVolumeID, this);
   this->Scene->AddReferencedNodeID(this->LabelVolumeID, this);
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->BackgroundVolumeID != nullptr && this->Scene->GetNodeByID(this->BackgroundVolumeID) == nullptr)
    {
    this->SetBackgroundVolumeID(nullptr);
    }
  if (this->ForegroundVolumeID != nullptr && this->Scene->GetNodeByID(this->ForegroundVolumeID) == nullptr)
    {
    this->SetForegroundVolumeID(nullptr);
    }
  if (this->LabelVolumeID != nullptr && this->Scene->GetNodeByID(this->LabelVolumeID) == nullptr)
    {
    this->SetLabelVolumeID(nullptr);
    }


}
//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->BackgroundVolumeID && !strcmp(oldID, this->BackgroundVolumeID))
    {
    this->SetBackgroundVolumeID(newID);
    }
  if (this->ForegroundVolumeID && !strcmp(oldID, this->ForegroundVolumeID))
    {
    this->SetForegroundVolumeID(newID);
    }
  if (this->LabelVolumeID && !strcmp(oldID, this->LabelVolumeID))
    {
    this->SetLabelVolumeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLStringMacro(backgroundVolumeID, BackgroundVolumeID);
  vtkMRMLReadXMLStringMacro(foregroundVolumeID, ForegroundVolumeID);
  vtkMRMLReadXMLStringMacro(labelVolumeID, LabelVolumeID);
  vtkMRMLReadXMLIntMacro(compositing, Compositing);
  vtkMRMLReadXMLFloatMacro(foregroundOpacity, ForegroundOpacity);
  vtkMRMLReadXMLFloatMacro(labelOpacity, LabelOpacity);
  vtkMRMLReadXMLIntMacro(linkedControl, LinkedControl);
  vtkMRMLReadXMLIntMacro(hotLinkedControl, HotLinkedControl);
  vtkMRMLReadXMLIntMacro(fiducialVisibility, FiducialVisibility);
  vtkMRMLReadXMLIntMacro(fiducialLabelVisibility, FiducialLabelVisibility);
  vtkMRMLReadXMLIntMacro(sliceIntersectionVisibility, SliceIntersectionVisibility);
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
  vtkMRMLCopyStringMacro(BackgroundVolumeID);
  vtkMRMLCopyStringMacro(ForegroundVolumeID);
  vtkMRMLCopyStringMacro(LabelVolumeID);
  vtkMRMLCopyIntMacro(Compositing);
  vtkMRMLCopyFloatMacro(ForegroundOpacity);
  vtkMRMLCopyFloatMacro(LabelOpacity);
  vtkMRMLCopyIntMacro(LinkedControl);
  vtkMRMLCopyIntMacro(HotLinkedControl);
  vtkMRMLCopyIntMacro(FiducialVisibility);
  vtkMRMLCopyIntMacro(FiducialLabelVisibility);
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
  vtkMRMLPrintIntMacro(SliceIntersectionVisibility);
  vtkMRMLPrintEnumMacro(AnnotationSpace);
  vtkMRMLPrintEnumMacro(AnnotationMode);
  vtkMRMLPrintIntMacro(DoPropagateVolumeSelection);
  vtkMRMLPrintEndMacro();

  os << indent << "Interacting: " <<
    (this->Interacting ? "on" : "off") << "\n";
}

// End
