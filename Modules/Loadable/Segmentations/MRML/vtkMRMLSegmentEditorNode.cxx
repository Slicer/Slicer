/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations MRML includes
#include "vtkMRMLSegmentEditorNode.h"

#include "vtkOrientedImageDataResample.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
static const char* SEGMENTATION_REFERENCE_ROLE = "segmentationRef";
static const char* SOURCE_VOLUME_REFERENCE_ROLE = "masterVolumeRef";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSegmentEditorNode);

//----------------------------------------------------------------------------
vtkMRMLSegmentEditorNode::vtkMRMLSegmentEditorNode()
{
  this->SetHideFromEditors(true);
  this->SourceVolumeIntensityMaskRange[0] = 0.0;
  this->SourceVolumeIntensityMaskRange[1] = 0.0;
}

//----------------------------------------------------------------------------
vtkMRMLSegmentEditorNode::~vtkMRMLSegmentEditorNode()
{
  this->SetSelectedSegmentID(nullptr);
  this->SetActiveEffectName(nullptr);
  this->SetMaskSegmentID(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentEditorNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  of << " selectedSegmentID=\"" << (this->SelectedSegmentID?this->SelectedSegmentID:"") << "\"";
  of << " activeEffectName=\"" << (this->ActiveEffectName?this->ActiveEffectName:"") << "\"";
  of << " maskMode=\"" << vtkMRMLSegmentationNode::ConvertMaskModeToString(this->MaskMode) << "\"";
  of << " maskSegmentID=\"" << (this->MaskSegmentID?this->MaskSegmentID:"") << "\"";
  of << " masterVolumeIntensityMask=\"" << (this->SourceVolumeIntensityMask ? "true" : "false") << "\"";
  of << " masterVolumeIntensityMaskRange=\"" << this->SourceVolumeIntensityMaskRange[0] << " " << this->SourceVolumeIntensityMaskRange[1] << "\"";
  of << " overwriteMode=\"" << vtkMRMLSegmentEditorNode::ConvertOverwriteModeToString(this->OverwriteMode) << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentEditorNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName = nullptr;
  const char* attValue = nullptr;

  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "selectedSegmentID"))
      {
      this->SetSelectedSegmentID(attValue);
      }
    else if (!strcmp(attName, "activeEffectName"))
      {
      this->SetActiveEffectName(attValue);
      }
    else if (!strcmp(attName, "maskMode"))
      {
      this->SetMaskMode(vtkMRMLSegmentationNode::ConvertMaskModeFromString(attValue));
      }
    else if (!strcmp(attName, "maskSegmentID"))
      {
      this->SetMaskSegmentID(attValue);
      }
    else if (!strcmp(attName, "masterVolumeIntensityMask")
      || !strcmp(attName, "sourceVolumeIntensityMask")) // for future compatibility
      {
      this->SetSourceVolumeIntensityMask(!strcmp(attValue,"true"));
      }
    else if (!strcmp(attName, "masterVolumeIntensityMaskRange")
      || !strcmp(attName, "sourceVolumeIntensityMaskRange")) // for future compatibility)
      {
      std::stringstream ss;
      ss << attValue;
      double range[2]={0};
      ss >> range[0];
      ss >> range[1];
      this->SetSourceVolumeIntensityMaskRange(range);
      }
    else if (!strcmp(attName, "overwriteMode"))
      {
      this->SetOverwriteMode(vtkMRMLSegmentEditorNode::ConvertOverwriteModeFromString(attValue));
      }
    }

  // For future compatibility (when masterVolumeRef will be replaced by sourceVolumeRef)
  if (this->GetNodeReferenceID("sourceVolumeRef"))
    {
    this->SetNodeReferenceID(SOURCE_VOLUME_REFERENCE_ROLE, this->GetNodeReferenceID("sourceVolumeRef"));
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLSegmentEditorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  this->DisableModifiedEventOn();

  vtkMRMLSegmentEditorNode* otherNode = vtkMRMLSegmentEditorNode::SafeDownCast(anode);

  this->SetSelectedSegmentID(otherNode->SelectedSegmentID);
  this->SetActiveEffectName(otherNode->ActiveEffectName);
  this->SetMaskMode(otherNode->GetMaskMode());
  this->SetMaskSegmentID(otherNode->GetMaskSegmentID());
  this->SetSourceVolumeIntensityMask(otherNode->GetSourceVolumeIntensityMask());
  this->SetSourceVolumeIntensityMaskRange(otherNode->GetSourceVolumeIntensityMaskRange());
  this->SetOverwriteMode(otherNode->GetOverwriteMode());

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentEditorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "SelectedSegmentID: " << (this->SelectedSegmentID ? this->SelectedSegmentID : "") << "\n";
  os << indent << "ActiveEffectName: " << (this->ActiveEffectName ? this->ActiveEffectName : "") << "\n";
  os << indent << "MaskMode: " << vtkMRMLSegmentationNode::ConvertMaskModeToString(this->MaskMode) << "\n";
  os << indent << "MaskSegmentID: " << (this->MaskSegmentID?this->MaskSegmentID:"") << "\n";
  os << indent << "OverwriteMode: " << vtkMRMLSegmentEditorNode::ConvertOverwriteModeToString(this->OverwriteMode) << "\n";
  os << indent << "SourceVolumeIntensityMask: " << (this->SourceVolumeIntensityMask ? "true" : "false") << "\n";
  os << indent << "SourceVolumeIntensityMaskRange: " << this->SourceVolumeIntensityMaskRange[0] << " " << this->SourceVolumeIntensityMaskRange[1] << "\n";
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLSegmentEditorNode::GetSourceVolumeNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast( this->GetNodeReference(SOURCE_VOLUME_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLSegmentEditorNode::GetMasterVolumeNode()
{
  vtkWarningMacro("qSlicerSegmentEditorAbstractEffect::GetMasterVolumeNode() method is deprecated, use GetSourceVolumeNode method instead");
  return this->GetSourceVolumeNode();
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentEditorNode::SetAndObserveSourceVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  this->SetNodeReferenceID(SOURCE_VOLUME_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

void vtkMRMLSegmentEditorNode::SetAndObserveMasterVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  vtkWarningMacro("qSlicerSegmentEditorAbstractEffect::SetAndObserveMasterVolumeNode() method is deprecated, use SetAndObserveSourceVolumeNode method instead");
  this->SetAndObserveSourceVolumeNode(node);
}

//----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkMRMLSegmentEditorNode::GetSegmentationNode()
{
  return vtkMRMLSegmentationNode::SafeDownCast( this->GetNodeReference(SEGMENTATION_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentEditorNode::SetAndObserveSegmentationNode(vtkMRMLSegmentationNode* node)
{
  this->SetNodeReferenceID(SEGMENTATION_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
const char* vtkMRMLSegmentEditorNode::ConvertOverwriteModeToString(int mode)
{
  switch (mode)
  {
    case OverwriteAllSegments: return "OverwriteAllSegments";
    case OverwriteVisibleSegments: return "OverwriteVisibleSegments";
    case OverwriteNone: return "OverwriteNone";
    default: return "";
  }
}

//----------------------------------------------------------------------------
int vtkMRMLSegmentEditorNode::ConvertOverwriteModeFromString(const char* modeStr)
{
  if (!modeStr)
  {
    return -1;
  }
  for (int i=0; i<Overwrite_Last; i++)
  {
    if (strcmp(modeStr, vtkMRMLSegmentEditorNode::ConvertOverwriteModeToString(i))==0)
    {
      return i;
    }
  }
  return -1;
}
