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
#include "vtkMRMLSliceDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkCollection.h"
#include "vtkCommand.h"
#include "vtkObjectFactory.h"

// STD includes
#include <sstream>

static const char* BackgroundVolumeNodeReferenceRole = "backgroundVolume";
static const char* BackgroundVolumeNodeReferenceMRMLAttributeName = "backgroundVolumeID";
static const char* ForegroundVolumeNodeReferenceRole = "foregroundVolume";
static const char* ForegroundVolumeNodeReferenceMRMLAttributeName = "foregroundVolumeID";
static const char* LabelVolumeNodeReferenceRole = "labelVolume";
static const char* LabelVolumeNodeReferenceMRMLAttributeName = "labelVolumeID";

static const char* AdditionalLayerVolumeNodeReferenceRole = "additionalLayerVolume";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSliceCompositeNode);

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::vtkMRMLSliceCompositeNode()
{
  this->TypeDisplayName = vtkMRMLTr("vtkMRMLSliceCompositeNode", "Slice Composite");

  this->HideFromEditors = 1;

  this->AddNodeReferenceRole(BackgroundVolumeNodeReferenceRole, BackgroundVolumeNodeReferenceMRMLAttributeName);
  this->AddNodeReferenceRole(ForegroundVolumeNodeReferenceRole, ForegroundVolumeNodeReferenceMRMLAttributeName);
  this->AddNodeReferenceRole(LabelVolumeNodeReferenceRole, LabelVolumeNodeReferenceMRMLAttributeName);
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::~vtkMRMLSliceCompositeNode() {}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLIntMacro(compositing, Compositing);
  vtkMRMLWriteXMLBooleanMacro(clipToBackgroundVolume, ClipToBackgroundVolume);
  vtkMRMLWriteXMLFloatMacro(foregroundOpacity, ForegroundOpacity);
  vtkMRMLWriteXMLFloatMacro(labelOpacity, LabelOpacity);
  vtkMRMLWriteXMLIntMacro(linkedControl, LinkedControl);
  vtkMRMLWriteXMLIntMacro(hotLinkedControl, HotLinkedControl);
  vtkMRMLWriteXMLIntMacro(fiducialVisibility, FiducialVisibility);
  vtkMRMLWriteXMLIntMacro(fiducialLabelVisibility, FiducialLabelVisibility);
  vtkMRMLWriteXMLStringMacro(layoutName, LayoutName);
  vtkMRMLWriteXMLIntMacro(doPropagateVolumeSelection, DoPropagateVolumeSelection);
  vtkMRMLWriteXMLEndMacro();
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
  this->InteractionFlagsModifier = (unsigned int)-1;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro(compositing, Compositing);
  vtkMRMLReadXMLBooleanMacro(clipToBackgroundVolume, ClipToBackgroundVolume);
  vtkMRMLReadXMLFloatMacro(foregroundOpacity, ForegroundOpacity);
  vtkMRMLReadXMLFloatMacro(labelOpacity, LabelOpacity);
  vtkMRMLReadXMLIntMacro(linkedControl, LinkedControl);
  vtkMRMLReadXMLIntMacro(hotLinkedControl, HotLinkedControl);
  vtkMRMLReadXMLIntMacro(fiducialVisibility, FiducialVisibility);
  vtkMRMLReadXMLIntMacro(fiducialLabelVisibility, FiducialLabelVisibility);
  vtkMRMLReadXMLStringMacro(layoutName, LayoutName);
  vtkMRMLReadXMLIntMacro(doPropagateVolumeSelection, DoPropagateVolumeSelection);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLSliceCompositeNode* node = vtkMRMLSliceCompositeNode::SafeDownCast(anode);

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyIntMacro(Compositing);
  vtkMRMLCopyBooleanMacro(ClipToBackgroundVolume);
  vtkMRMLCopyFloatMacro(ForegroundOpacity);
  vtkMRMLCopyFloatMacro(LabelOpacity);
  for (int additionalLayerIndex = 0; additionalLayerIndex < node->GetNumberOfAdditionalLayers(); ++additionalLayerIndex)
  {
    int layerIndex = vtkMRMLSliceCompositeNode::Layer_Last + additionalLayerIndex;
    this->SetNthLayerOpacity(layerIndex, node->GetNthLayerOpacity(layerIndex));
  }
  vtkMRMLCopyIntMacro(LinkedControl);
  vtkMRMLCopyIntMacro(HotLinkedControl);
  vtkMRMLCopyIntMacro(FiducialVisibility);
  vtkMRMLCopyIntMacro(FiducialLabelVisibility);
  // To avoid breaking current implementation, copy of the "LayoutName" attribute
  // will be enabled after revisiting the view initialization pipeline.
  // vtkMRMLCopyStringMacro(LayoutName);
  vtkMRMLCopyIntMacro(DoPropagateVolumeSelection);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStringMacro(BackgroundVolumeID);
  vtkMRMLPrintStringMacro(ForegroundVolumeID);
  vtkMRMLPrintStringMacro(LabelVolumeID);
  vtkMRMLPrintIntMacro(Compositing);
  vtkMRMLPrintBooleanMacro(ClipToBackgroundVolume);
  vtkMRMLPrintFloatMacro(ForegroundOpacity);
  vtkMRMLPrintFloatMacro(LabelOpacity);
  for (int additionalLayerIndex = 0; additionalLayerIndex < this->GetNumberOfAdditionalLayers(); ++additionalLayerIndex)
  {
    printOutputStream << printOutputIndent << "Additional Layer Opacity (N=" << additionalLayerIndex
                      << "): " << this->GetNthLayerOpacity(vtkMRMLSliceCompositeNode::Layer_Last + additionalLayerIndex) << "\n";
  }
  vtkMRMLPrintIntMacro(LinkedControl);
  vtkMRMLPrintIntMacro(HotLinkedControl);
  vtkMRMLPrintIntMacro(FiducialVisibility);
  vtkMRMLPrintIntMacro(FiducialLabelVisibility);
  vtkMRMLPrintStringMacro(LayoutName);
  vtkMRMLPrintIntMacro(DoPropagateVolumeSelection);
  vtkMRMLPrintEndMacro();

  os << indent << "Interacting: " << (this->Interacting ? "on" : "off") << "\n";
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetBackgroundVolumeID(const char* id)
{
  this->SetNthLayerVolumeID(vtkMRMLSliceCompositeNode::LayerBackground, id);
}

//-----------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetBackgroundVolumeID()
{
  return this->GetNthLayerVolumeID(vtkMRMLSliceCompositeNode::LayerBackground);
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetForegroundVolumeID(const char* id)
{
  this->SetNthLayerVolumeID(vtkMRMLSliceCompositeNode::LayerForeground, id);
}

//-----------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetForegroundVolumeID()
{
  return this->GetNthLayerVolumeID(vtkMRMLSliceCompositeNode::LayerForeground);
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetLabelVolumeID(const char* id)
{
  this->SetNthLayerVolumeID(vtkMRMLSliceCompositeNode::LayerLabel, id);
}

//-----------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetLabelVolumeID()
{
  return this->GetNthLayerVolumeID(vtkMRMLSliceCompositeNode::LayerLabel);
}

//----------------------------------------------------------------------------
int vtkMRMLSliceCompositeNode::GetNumberOfAdditionalLayers()
{
  return this->GetNumberOfNodeReferences(AdditionalLayerVolumeNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLSliceCompositeNode::GetNthLayerVolume(int layerIndex)
{
  if (layerIndex < 0)
  {
    vtkErrorMacro(<< "GetNthLayerVolume: Non-negative layer index is expected.");
    return nullptr;
  }
  if (layerIndex == vtkMRMLSliceCompositeNode::LayerBackground)
  {
    return vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(BackgroundVolumeNodeReferenceRole));
  }
  else if (layerIndex == vtkMRMLSliceCompositeNode::LayerForeground)
  {
    return vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(ForegroundVolumeNodeReferenceRole));
  }
  else if (layerIndex == vtkMRMLSliceCompositeNode::LayerLabel)
  {
    return vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(LabelVolumeNodeReferenceRole));
  }
  else if (layerIndex >= vtkMRMLSliceCompositeNode::Layer_Last)
  {
    return vtkMRMLVolumeNode::SafeDownCast(this->GetNthNodeReference(AdditionalLayerVolumeNodeReferenceRole, layerIndex - vtkMRMLSliceCompositeNode::Layer_Last));
  }
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetNthLayerVolume(int layerIndex, vtkMRMLVolumeNode* volumeNode)
{
  if (layerIndex < 0)
  {
    vtkErrorMacro(<< "SetNthLayerVolume: Non-negative layer index is expected.");
    return;
  }
  char* volumeNodeID = volumeNode != nullptr ? volumeNode->GetID() : nullptr;
  this->SetNthLayerVolumeID(layerIndex, volumeNodeID);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetNthLayerVolumeID(int layerIndex)
{
  if (layerIndex < 0)
  {
    vtkErrorMacro(<< "GetNthLayerVolumeID: Non-negative layer index is expected.");
    return nullptr;
  }
  vtkMRMLVolumeNode* volumeNode = this->GetNthLayerVolume(layerIndex);
  if (volumeNode)
  {
    return volumeNode->GetID();
  }
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetNthLayerVolumeID(int layerIndex, const char* volumeNodeID)
{
  if (layerIndex < 0)
  {
    vtkErrorMacro(<< "SetNthLayerVolumeID: Non-negative layer index is expected.");
    return;
  }
  if (layerIndex == vtkMRMLSliceCompositeNode::LayerBackground)
  {
    this->SetNodeReferenceID(BackgroundVolumeNodeReferenceRole, volumeNodeID);
  }
  else if (layerIndex == vtkMRMLSliceCompositeNode::LayerForeground)
  {
    this->SetNodeReferenceID(ForegroundVolumeNodeReferenceRole, volumeNodeID);
  }
  else if (layerIndex == vtkMRMLSliceCompositeNode::LayerLabel)
  {
    this->SetNodeReferenceID(LabelVolumeNodeReferenceRole, volumeNodeID);
  }
  else if (layerIndex >= vtkMRMLSliceCompositeNode::Layer_Last)
  {
    this->SetNthNodeReferenceID(AdditionalLayerVolumeNodeReferenceRole, layerIndex - vtkMRMLSliceCompositeNode::Layer_Last, volumeNodeID);
  }
}

//----------------------------------------------------------------------------
double vtkMRMLSliceCompositeNode::GetNthLayerOpacity(int layerIndex)
{
  if (layerIndex < 0)
  {
    vtkErrorMacro(<< "GetNthLayerOpacity: Non-negative layer index is expected.");
    return 1.0;
  }
  if (layerIndex < static_cast<int>(this->LayerOpacities.size()))
  {
    return this->LayerOpacities.at(layerIndex);
  }
  return 1.0;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetNthLayerOpacity(int layerIndex, double value)
{
  if (layerIndex < 0)
  {
    vtkErrorMacro(<< "SetNthLayerOpacity: Non-negative layer index is expected.");
    return;
  }
  if (layerIndex >= static_cast<int>(this->LayerOpacities.size()))
  {
    this->LayerOpacities.resize(layerIndex + 1);
  }
  if (this->LayerOpacities.at(layerIndex) != value)
  {
    this->LayerOpacities.at(layerIndex) = value;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
double vtkMRMLSliceCompositeNode::GetForegroundOpacity()
{
  return this->GetNthLayerOpacity(vtkMRMLSliceCompositeNode::LayerForeground);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetForegroundOpacity(double value)
{
  this->SetNthLayerOpacity(vtkMRMLSliceCompositeNode::LayerForeground, value);
}

//----------------------------------------------------------------------------
double vtkMRMLSliceCompositeNode::GetLabelOpacity()
{
  return this->GetNthLayerOpacity(vtkMRMLSliceCompositeNode::LayerLabel);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetLabelOpacity(double value)
{
  this->SetNthLayerOpacity(vtkMRMLSliceCompositeNode::LayerLabel, value);
}

//-----------------------------------------------------------
int vtkMRMLSliceCompositeNode::GetSliceIntersectionVisibility()
{
  vtkWarningMacro("GetSliceIntersectionVisibility method is deprecated. Use GetIntersectingSlicesVisibility method"
                  " of vtkMRMLSliceDisplayNode object instead.");
  vtkMRMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
  if (!sliceDisplayNode)
  {
    vtkWarningMacro("SetSliceIntersectionVisibility failed: no slice display node was found");
    return 0;
  }
  return sliceDisplayNode->GetIntersectingSlicesVisibility() ? 1 : 0;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetSliceIntersectionVisibility(int visibility)
{
  vtkWarningMacro("SetSliceIntersectionVisibility method is deprecated. Use SetIntersectingSlicesVisibility method"
                  " of vtkMRMLSliceDisplayNode object instead.");
  vtkMRMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
  if (!sliceDisplayNode)
  {
    vtkWarningMacro("SetSliceIntersectionVisibility failed: no slice display node was found");
    return;
  }
  sliceDisplayNode->SetIntersectingSlicesVisibility(visibility != 0);
}

//----------------------------------------------------------------------------
std::string vtkMRMLSliceCompositeNode::GetCompositeNodeIDFromSliceModelNode(vtkMRMLModelNode* sliceModelNode)
{
  // Description of slice model nodes contain a string that specifies related slice node and composite node IDs:
  // "SliceID vtkMRMLSliceNodeRed CompositeID vtkMRMLSliceCompositeNodeRed".
  // This is an old mechanism that may go away in the future but it is likely to
  // stay long enough for the lifetime of the deprecated methods
  // GetSliceIntersectionVisibility/SetSliceIntersectionVisibility.

  if (!sliceModelNode || !sliceModelNode->GetDescription())
  {
    return "";
  }

  // Iterate through the description split by spaces.
  // If "CompositeID" component is found then the next component
  // is returned as composite node ID.
  std::stringstream description(sliceModelNode->GetDescription());
  std::string previous;
  std::string current;
  while (true)
  {
    current.clear();
    while (current.empty())
    {
      // Get the next string in a while loop to ignore multiple spaces
      if (!std::getline(description, current, ' '))
      {
        return "";
      }
    }
    if (previous == "CompositeID")
    {
      return current;
    }
    previous = current;
  }
}

//----------------------------------------------------------------------------
vtkMRMLSliceDisplayNode* vtkMRMLSliceCompositeNode::GetSliceDisplayNode()
{
  if (this->Scene == nullptr || this->GetLayoutName() == nullptr)
  {
    return nullptr;
  }

  // It is an expensive operation to determine the displayable node
  // (need to iterate through the scene), so the last found value
  // is cached. If it is still valid then we use it.
  if (this->LastFoundSliceDisplayNode != nullptr //
      && this->LastFoundSliceDisplayNode->GetScene() == this->Scene)
  {
    vtkMRMLModelNode* sliceModelNode = vtkMRMLModelNode::SafeDownCast(this->LastFoundSliceDisplayNode->GetDisplayableNode());
    if (this->GetCompositeNodeIDFromSliceModelNode(sliceModelNode) == this->GetID())
    {
      return this->LastFoundSliceDisplayNode;
    }
  }

  vtkMRMLNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* sceneNodes = this->Scene->GetNodes();
  for (sceneNodes->InitTraversal(it); (node = vtkMRMLNode::SafeDownCast(sceneNodes->GetNextItemAsObject(it)));)
  {
    vtkMRMLModelNode* sliceModelNode = vtkMRMLModelNode::SafeDownCast(node);
    if (this->GetCompositeNodeIDFromSliceModelNode(sliceModelNode) == this->GetID())
    {
      this->LastFoundSliceDisplayNode = vtkMRMLSliceDisplayNode::SafeDownCast(sliceModelNode->GetDisplayNode());
      return this->LastFoundSliceDisplayNode;
    }
  }
  this->LastFoundSliceDisplayNode = nullptr;
  return nullptr;
}
