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

#include "vtkSlicerSegmentEditorLogic.h"

// Slicer includes
#include <vtkSlicerSegmentationsModuleLogic.h>
#include <vtkSlicerTerminologiesModuleLogic.h>
#include <vtkSlicerTerminologyEntry.h>

// MRML includes
#include <vtkMRMLCameraWidget.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSegmentEditorNode.h>
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// vtkSegmentationCore includes
#include <vtkOrientedImageData.h>
#include <vtkOrientedImageDataResample.h>
#include <vtkSegment.h>
#include <vtkSegmentation.h>
#include <vtkSegmentationHistory.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageFillROI.h>
#include <vtkImageMathematics.h>
#include <vtkImageThreshold.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkWeakPointer.h>

//----------------------------------------------------------------------------
/// \brief Helper class to emit pause / resume events on construction and destruction
class RenderBlocker
{
public:
  explicit RenderBlocker(vtkSlicerSegmentEditorLogic* logic)
    : m_logic{ logic }
  {
    if (m_logic)
    {
      m_logic->PauseRender();
    }
  }

  ~RenderBlocker()
  {
    if (m_logic)
    {
      m_logic->ResumeRender();
    }
  }

private:
  vtkWeakPointer<vtkSlicerSegmentEditorLogic> m_logic;
};

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSegmentEditorLogic);

//-----------------------------------------------------------------------------
std::string vtkSlicerSegmentEditorLogic::AddEmptySegment(const std::string& segmentId, int segmentStatus) const
{
  if (!this->SegmentEditorNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment editor parameter set node");
    }
    return {};
  }

  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode->GetSegmentationNode();
  if (!segmentationNode)
  {
    return {};
  }

  if (!this->SaveStateForUndo())
  {
    if (this->IsVerbose)
    {
      vtkWarningMacro("" << __func__ << ": Failed to properly save changes for Undo.");
    }
  }

  // Create empty segment in current segmentation
  std::string addedSegmentID = segmentationNode->GetSegmentation()->AddEmptySegment(segmentId);

  // Set default terminology entry from application settings
  vtkSegment* addedSegment = segmentationNode->GetSegmentation()->GetSegment(addedSegmentID);
  if (addedSegment)
  {
    std::string defaultTerminologyEntryStr;
    // Default terminology in the segmentation node has the highest priority
    vtkNew<vtkSlicerTerminologyEntry> entry;
    if (vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntry(segmentationNode, entry) && !entry->IsEmpty())
    {
      defaultTerminologyEntryStr = vtkSlicerTerminologiesModuleLogic::SerializeTerminologyEntry(entry);
    }
    if (defaultTerminologyEntryStr.empty())
    {
      defaultTerminologyEntryStr = this->DefaultTerminologyEntry;
    }

    if (!defaultTerminologyEntryStr.empty())
    {
      addedSegment->SetTerminology(defaultTerminologyEntryStr);
    }
  }

  // Set segment status to one that is visible by current filtering criteria
  vtkSlicerSegmentationsModuleLogic::SetSegmentStatus(addedSegment, segmentStatus);

  // Select the new segment
  if (!addedSegmentID.empty())
  {
    this->SegmentEditorNode->SetSelectedSegmentID(addedSegmentID.c_str());
  }

  // Assign the new segment the terminology of the (now second) last segment
  if (segmentationNode->GetSegmentation()->GetNumberOfSegments() > 1)
  {
    vtkSegment* secondLastSegment = segmentationNode->GetSegmentation()->GetNthSegment(segmentationNode->GetSegmentation()->GetNumberOfSegments() - 2);
    std::string repeatedTerminologyEntry = secondLastSegment->GetTerminology();
    segmentationNode->GetSegmentation()->GetSegment(addedSegmentID)->SetTerminology(repeatedTerminologyEntry);
  }
  return addedSegmentID;
}

//------------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::CanAddSegments() const
{
  // Disable adding new segments until source volume is set (or reference geometry is specified for the segmentation).
  // This forces the user to select a source volume before start adding segments.
  return (this->GetSegmentation() != nullptr) && (this->GetSourceVolumeNode() != nullptr) && (!this->GetReferenceImageGeometryString().empty());
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::CanRedo() const
{
  return this->SegmentationHistory && this->SegmentationHistory->IsRestoreNextStateAvailable();
}

//------------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::CanRemoveSegments() const
{
  // Only enable remove button if a segment is selected
  return this->IsSegmentIdValid(GetCurrentSegmentID());
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::CanTriviallyConvertSourceRepresentationToBinaryLabelMap() const
{
  if (!this->IsSegmentationNodeValid())
  {
    return false;
  }

  if (this->GetSegmentation()->GetNumberOfSegments() < 1)
  {
    return true;
  }

  if (this->GetSegmentation()->GetSourceRepresentationName() == vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
  {
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::CanUndo() const
{
  return this->SegmentationHistory && this->SegmentationHistory->IsRestorePreviousStateAvailable();
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ClearUndoState() const
{
  if (this->SegmentationHistory)
  {
    this->SegmentationHistory->RemoveAllStates();
  }
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::ContainsClosedSurfaceRepresentation() const
{
  if (!this->GetSegmentation())
  {
    return false;
  }

  return this->GetSegmentation()->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::CreateAndSetBlankSourceVolumeFromSegmentationGeometry() const
{
  if (!this->IsSegmentationNodeValid() || !this->GetMRMLScene())
  {
    return;
  }

  std::string referenceImageGeometry = vtkSlicerSegmentEditorLogic::GetReferenceImageGeometryStringFromSegmentation(GetSegmentation());
  vtkNew<vtkMatrix4x4> referenceGeometryMatrix;
  int referenceExtent[6] = { 0, -1, 0, -1, 0, -1 };
  vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, referenceGeometryMatrix.GetPointer(), referenceExtent);
  if (referenceExtent[0] <= referenceExtent[1]    //
      && referenceExtent[2] <= referenceExtent[3] //
      && referenceExtent[4] <= referenceExtent[5])
  {
    // Create new image, allocate memory
    vtkNew<vtkOrientedImageData> blankImage;
    vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, blankImage.GetPointer());
    vtkOrientedImageDataResample::FillImage(blankImage.GetPointer(), 0.0);

    // Create volume node from blank image
    vtkMRMLSegmentationNode* segmentationNode = this->GetSegmentationNode();
    std::string sourceVolumeNodeName = (segmentationNode->GetName() ? segmentationNode->GetName() : "Volume") + std::string(" source volume");
    vtkMRMLScalarVolumeNode* sourceVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLScalarVolumeNode", sourceVolumeNodeName));
    sourceVolumeNode->SetAndObserveTransformNodeID(segmentationNode->GetTransformNodeID());
    vtkSlicerSegmentationsModuleLogic::CopyOrientedImageDataToVolumeNode(blankImage.GetPointer(), sourceVolumeNode);

    // Use blank volume as master
    this->SetSourceVolumeNode(sourceVolumeNode);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::CreateAndSetBlankSourceVolumeIfNeeded() const
{
  // If no source volume is selected but a valid geometry is specified then create a blank source volume
  if (this->GetSourceVolumeNode() != nullptr)
  {
    return;
  }
  this->CreateAndSetBlankSourceVolumeFromSegmentationGeometry();
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ExportSegmentationToColorTableNode() const
{
  vtkMRMLSegmentationNode* segmentationNode = this->GetSegmentationNode();
  if (!segmentationNode)
  {
    return;
  }

  vtkMRMLColorTableNode* newColorTable = vtkSlicerSegmentationsModuleLogic::AddColorTableNodeForSegmentation(segmentationNode);
  if (newColorTable == nullptr)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Failed to create color table node for segmentation " << segmentationNode->GetName());
    }
    return;
  }

  // Export all segments to the new color table
  std::vector<std::string> segmentIDs;
  if (!vtkSlicerSegmentationsModuleLogic::ExportSegmentsToColorTableNode(segmentationNode, segmentIDs, newColorTable))
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Failed to export color and terminology information from segmentation " << segmentationNode->GetName() << " to color table "
                       << newColorTable->GetName());
    }
  }
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* vtkSlicerSegmentEditorLogic::GetAlignedSourceVolume() const
{
  return this->AlignedSourceVolume;
}

//-----------------------------------------------------------------------------
std::string vtkSlicerSegmentEditorLogic::GetCurrentSegmentID() const
{
  vtkMRMLSegmentEditorNode* editorNode = this->GetSegmentEditorNode();
  if (!editorNode)
  {
    return {};
  }

  // Selected ID may be nullptr in the segment editor node which will make std::string ctor crash.
  char* selectedSegmentID = editorNode->GetSelectedSegmentID();
  if (!selectedSegmentID)
  {
    return {};
  }
  return selectedSegmentID;
}

std::string vtkSlicerSegmentEditorLogic::GetDefaultTerminologyEntry() const
{
  return this->DefaultTerminologyEntry;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* vtkSlicerSegmentEditorLogic::GetMaskLabelmap() const
{
  return this->MaskLabelmap;
}

//-----------------------------------------------------------------------------
int vtkSlicerSegmentEditorLogic::GetMaximumNumberOfUndoStates() const
{
  if (!this->SegmentationHistory)
  {
    return 0;
  }
  return static_cast<int>(this->SegmentationHistory->GetMaximumNumberOfStates());
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* vtkSlicerSegmentEditorLogic::GetModifierLabelmap() const
{
  return this->ModifierLabelmap;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* vtkSlicerSegmentEditorLogic::GetReferenceGeometryImage() const
{
  return this->ReferenceGeometryImage;
}

//-----------------------------------------------------------------------------
std::string vtkSlicerSegmentEditorLogic::GetReferenceImageGeometryString() const
{
  if (!this->SegmentEditorNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment editor parameter set node");
    }
    return "";
  }

  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode->GetSegmentationNode();
  vtkSegmentation* segmentation = segmentationNode ? segmentationNode->GetSegmentation() : nullptr;
  if (!segmentationNode || !segmentation)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segmentation");
    }
    return "";
  }

  std::string referenceImageGeometry = vtkSlicerSegmentEditorLogic::GetReferenceImageGeometryStringFromSegmentation(segmentation);
  if (referenceImageGeometry.empty())
  {
    // If no reference image geometry could be determined then use the source volume's geometry
    vtkMRMLScalarVolumeNode* sourceVolumeNode = this->SegmentEditorNode->GetSourceVolumeNode();
    if (!sourceVolumeNode)
    {
      // cannot determine reference geometry
      return "";
    }
    // Update the referenceImageGeometry parameter for next time
    segmentationNode->SetReferenceImageGeometryParameterFromVolumeNode(sourceVolumeNode);
    // Update extents to include all existing segments
    referenceImageGeometry = vtkSlicerSegmentEditorLogic::GetReferenceImageGeometryStringFromSegmentation(segmentation);
  }
  return referenceImageGeometry;
}

//-----------------------------------------------------------------------------
std::string vtkSlicerSegmentEditorLogic::GetReferenceImageGeometryStringFromSegmentation(vtkSegmentation* segmentation)
{
  if (!segmentation)
  {
    return "";
  }

  // If "reference image geometry" conversion parameter is set then use that
  std::string referenceImageGeometry = segmentation->GetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName());
  if (!referenceImageGeometry.empty())
  {
    // Extend reference image geometry to contain all segments (needed for example for properly handling imported segments
    // that do not fit into the reference image geometry)
    vtkSmartPointer<vtkOrientedImageData> commonGeometryImage = vtkSmartPointer<vtkOrientedImageData>::New();
    vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, commonGeometryImage, false);
    // Determine extent that contains all segments
    int commonSegmentExtent[6] = { 0, -1, 0, -1, 0, -1 };
    segmentation->DetermineCommonLabelmapExtent(commonSegmentExtent, commonGeometryImage);
    if (commonSegmentExtent[0] <= commonSegmentExtent[1]    //
        && commonSegmentExtent[2] <= commonSegmentExtent[3] //
        && commonSegmentExtent[4] <= commonSegmentExtent[5])
    {
      // Expand commonGeometryExtent as needed to contain commonSegmentExtent
      int commonGeometryExtent[6] = { 0, -1, 0, -1, 0, -1 };
      commonGeometryImage->GetExtent(commonGeometryExtent);
      for (int i = 0; i < 3; i++)
      {
        commonGeometryExtent[i * 2] = std::min(commonSegmentExtent[i * 2], commonGeometryExtent[i * 2]);
        commonGeometryExtent[i * 2 + 1] = std::max(commonSegmentExtent[i * 2 + 1], commonGeometryExtent[i * 2 + 1]);
      }
      commonGeometryImage->SetExtent(commonGeometryExtent);
      referenceImageGeometry = vtkSegmentationConverter::SerializeImageGeometry(commonGeometryImage);
    }

    // TODO: Use oversampling (if it's 'A' then ignore and changed to 1)
    return referenceImageGeometry;
  }
  if (segmentation->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
  {
    // If no reference image geometry is specified but there are labels already then determine geometry from that
    referenceImageGeometry = segmentation->DetermineCommonLabelmapGeometry();
    return referenceImageGeometry;
  }
  return "";
}

//-----------------------------------------------------------------------------
vtkSegmentation* vtkSlicerSegmentEditorLogic::GetSegmentation() const
{
  if (!this->IsSegmentationNodeValid())
  {
    return nullptr;
  }
  return this->GetSegmentationNode()->GetSegmentation();
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::GetSegmentationIJKToRAS(vtkMatrix4x4* ijkToRas) const
{
  if (!this->SegmentEditorNode)
  {
    return false;
  }
  if (!this->UpdateReferenceGeometryImage())
  {
    return false;
  }

  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode->GetSegmentationNode();
  if (!segmentationNode || !segmentationNode->GetSegmentation())
  {
    return false;
  }
  if (!segmentationNode->GetSegmentation()->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
  {
    return false;
  }
  this->ReferenceGeometryImage->GetImageToWorldMatrix(ijkToRas);
  if (vtkMRMLTransformNode* transformNode = segmentationNode->GetParentTransformNode())
  {
    if (!transformNode->IsTransformToWorldLinear())
    {
      return false;
    }
    vtkSmartPointer<vtkMatrix4x4> volumeRasToWorldRas = vtkSmartPointer<vtkMatrix4x4>::New();
    transformNode->GetMatrixTransformToWorld(volumeRasToWorldRas);
    vtkMatrix4x4::Multiply4x4(volumeRasToWorldRas, ijkToRas, ijkToRas);
  }
  return true;
}

//------------------------------------------------------------------------------
vtkMRMLSegmentEditorNode* vtkSlicerSegmentEditorLogic::GetSegmentEditorNode() const
{
  return this->SegmentEditorNode;
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkSlicerSegmentEditorLogic::GetSegmentationNode() const
{
  return this->SegmentEditorNode ? this->SegmentEditorNode->GetSegmentationNode() : nullptr;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* vtkSlicerSegmentEditorLogic::GetSelectedSegmentLabelmap() const
{
  return this->SelectedSegmentLabelmap;
}

//-----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerSegmentEditorLogic::GetSourceVolumeNode() const
{
  if (!this->SegmentEditorNode)
  {
    return nullptr;
  }
  return this->SegmentEditorNode->GetSourceVolumeNode();
}

//-----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerSegmentEditorLogic::GetSegmentIDs() const
{
  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode ? this->SegmentEditorNode->GetSegmentationNode() : nullptr;
  vtkSegmentation* segmentation = segmentationNode ? segmentationNode->GetSegmentation() : nullptr;
  if (!segmentation)
  {
    return {};
  }

  std::vector<std::string> segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  return segmentIDs;
}

//---------------------------------------------------------------------------
std::vector<std::string> vtkSlicerSegmentEditorLogic::GetVisibleSegmentIDs() const
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetSegmentationNode());
  if (segmentationNode == nullptr || segmentationNode->GetDisplayNode() == nullptr)
  {
    return {};
  }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  if (displayNode == nullptr)
  {
    return {};
  }
  std::vector<std::string> visibleSegmentIDs;
  displayNode->GetVisibleSegmentIDs(visibleSegmentIDs);
  return visibleSegmentIDs;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::IsSegmentationDisplayableInView(vtkMRMLAbstractViewNode* viewNode) const
{
  if (!viewNode)
  {
    return false;
  }
  if (!this->SegmentEditorNode)
  {
    return false;
  }
  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode->GetSegmentationNode();
  if (!segmentationNode)
  {
    return false;
  }
  const char* viewNodeID = viewNode->GetID();
  int numberOfDisplayNodes = segmentationNode->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
  {
    vtkMRMLDisplayNode* segmentationDisplayNode = segmentationNode->GetNthDisplayNode(displayNodeIndex);
    if (segmentationDisplayNode && segmentationDisplayNode->IsDisplayableInView(viewNodeID))
    {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::IsSegmentationNodeValid() const
{
  return this->GetSegmentationNode() != nullptr;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::IsSegmentIdValid(const std::string& segmentId) const
{
  if (segmentId.empty())
  {
    return false;
  }

  vtkSegmentation* segmentation = this->GetSegmentation();
  if (!segmentation)
  {
    return false;
  }
  return segmentation->GetSegmentIndex(segmentId) >= 0;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::Redo() const
{
  if (!this->IsSegmentationNodeValid() || !this->SegmentationHistory)
  {
    return;
  }

  vtkMRMLSegmentationNode* segmentationNode = this->GetSegmentationNode();
  MRMLNodeModifyBlocker blocker(segmentationNode);
  this->SegmentationHistory->RestoreNextState();
  segmentationNode->InvokeCustomModifiedEvent(vtkMRMLDisplayableNode::DisplayModifiedEvent, segmentationNode->GetDisplayNode());
}

//-----------------------------------------------------------------------------
std::string vtkSlicerSegmentEditorLogic::RemoveSelectedSegment() const
{
  if (!this->SegmentEditorNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment editor parameter set node");
    }
    return {};
  }

  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode->GetSegmentationNode();
  std::string selectedSegmentID = this->SegmentEditorNode->GetSelectedSegmentID();
  if (!segmentationNode || selectedSegmentID.empty())
  {
    return {};
  }

  if (!this->SaveStateForUndo())
  {
    if (this->IsVerbose)
    {
      vtkWarningMacro("" << __func__ << ": Failed to properly save state for Undo.");
    }
  }

  // Switch to a new valid segment now (to avoid transient state when no segments are selected
  // as it could deactivate current effect).
  std::string newId = this->GetNextSegmentID(1, false);
  if (newId.empty())
  {
    newId = this->GetNextSegmentID(-1, false);
  }
  if (!newId.empty())
  {
    this->SegmentEditorNode->SetSelectedSegmentID(newId.c_str());
  }

  // Remove segment
  segmentationNode->GetSegmentation()->RemoveSegment(selectedSegmentID);
  return newId;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::ResetModifierLabelmapToDefault() const
{
  constexpr int BINARY_LABELMAP_SCALAR_TYPE = VTK_UNSIGNED_CHAR;
  constexpr unsigned char BINARY_LABELMAP_VOXEL_EMPTY = 0;

  std::string referenceImageGeometry = this->GetReferenceImageGeometryString();
  if (referenceImageGeometry.empty())
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Cannot determine default modifierLabelmap geometry");
    }
    return false;
  }

  std::string modifierLabelmapReferenceImageGeometryBaseline = vtkSegmentationConverter::SerializeImageGeometry(this->ModifierLabelmap);

  // Set reference geometry to labelmap (origin, spacing, directions, extents) and allocate scalars
  vtkNew<vtkMatrix4x4> referenceGeometryMatrix;
  int referenceExtent[6] = { 0, -1, 0, -1, 0, -1 };
  vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, referenceGeometryMatrix.GetPointer(), referenceExtent);
  vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, this->ModifierLabelmap, true, BINARY_LABELMAP_SCALAR_TYPE, 1);

  vtkOrientedImageDataResample::FillImage(this->ModifierLabelmap, BINARY_LABELMAP_VOXEL_EMPTY);

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::SaveStateForUndo() const
{
  if (this->SegmentationHistory && this->SegmentationHistory->GetMaximumNumberOfStates() > 0)
  {
    return this->SegmentationHistory->SaveState();
  }
  return false;
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SelectPreviousSegment(bool visibleOnly) const
{
  this->SelectSegmentAtOffset(-1, visibleOnly);
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SelectNextSegment(bool visibleOnly) const
{
  this->SelectSegmentAtOffset(1, visibleOnly);
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SelectFirstSegment(bool visibleOnly) const
{
  std::vector<std::string> segmentIds = this->GetSegmentIDs();
  std::vector<std::string> filterIds = visibleOnly ? this->GetVisibleSegmentIDs() : segmentIds;
  for (const auto& segmentId : segmentIds)
  {
    if (vtkSlicerSegmentEditorLogic::IsSegmentIdInList(segmentId, filterIds))
    {
      this->SetCurrentSegmentID(segmentId);
      return;
    }
  }
}

void vtkSlicerSegmentEditorLogic::SetDefaultTerminologyEntry(const std::string& entry)
{
  this->DefaultTerminologyEntry = entry;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SetMaximumNumberOfUndoStates(int maxNumberOfStates) const
{
  if (!this->SegmentationHistory)
  {
    return;
  }
  this->SegmentationHistory->SetMaximumNumberOfStates(maxNumberOfStates);
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SetCurrentSegmentID(const std::string& segmentID) const
{
  if (this->SegmentEditorNode)
  {
    this->SegmentEditorNode->SetSelectedSegmentID(segmentID.c_str());
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SetSegmentEditorNode(vtkMRMLSegmentEditorNode* newSegmentEditorNode)
{
  if (this->SegmentEditorNode == newSegmentEditorNode)
  {
    return;
  }

  vtkSetAndObserveMRMLNodeMacro(this->SegmentEditorNode, newSegmentEditorNode);
  this->ProcessMRMLNodesEvents(this->SegmentEditorNode, vtkCommand::ModifiedEvent, nullptr);
}

//------------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SetSegmentationNode(vtkMRMLNode* node) const
{
  if (!this->SegmentEditorNode)
  {
    if (node)
    {
      if (this->IsVerbose)
      {
        vtkErrorMacro("" << __func__ << ": need to set segment editor node first");
      }
    }
    return;
  }

  this->SegmentEditorNode->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SetSegmentationNodeID(const std::string& nodeID) const
{
  if (!this->GetMRMLScene())
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": MRML scene is not set");
    }
    return;
  }
  this->SetSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID.c_str())));
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SetSegmentationHistory(const vtkSmartPointer<vtkSegmentationHistory>& segmentationHistory)
{
  // Segmentation history doesn't inherit from MRML nodes and cannot use the vtkSetAndObserveMRMLNodeMacro
  // Method connects the segmentation history manually and forwards update to ProcessMRMLNodesEvents.
  if (segmentationHistory == this->SegmentationHistory)
  {
    return;
  }
  if (this->SegmentationHistory)
  {
    this->SegmentationHistory->RemoveObserver(this->SegmentHistoryObs);
  }

  this->SegmentationHistory = segmentationHistory;
  if (!this->SegmentationHistory)
  {
    return;
  }

  // Forward segmentation history modified event
  vtkNew<vtkCallbackCommand> updateCommand;
  updateCommand->SetClientData(this);
  updateCommand->SetCallback(
    [](vtkObject* caller, unsigned long eid, void* clientData, void* callData)
    {
      auto client = static_cast<vtkSlicerSegmentEditorLogic*>(clientData);
      client->ProcessMRMLNodesEvents(caller, eid, callData);
    });

  this->SegmentationHistory->AddObserver(vtkCommand::ModifiedEvent, updateCommand);
  this->ProcessMRMLNodesEvents(this->SegmentationHistory, vtkCommand::ModifiedEvent, nullptr);
}

//------------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SetSourceVolumeNode(vtkMRMLNode* node) const
{
  if (!this->SegmentEditorNode || !this->IsSegmentationNodeValid())
  {
    if (node)
    {
      if (this->IsVerbose)
      {
        vtkErrorMacro("" << __func__ << ": need to set segment editor and segmentation nodes first");
      }
    }
    return;
  }

  // Set source volume to parameter set node
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  if (this->SegmentEditorNode->GetSourceVolumeNode() == volumeNode)
  {
    return;
  }

  this->SegmentEditorNode->SetAndObserveSourceVolumeNode(volumeNode);
}

//------------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SetSourceVolumeNodeID(const std::string& nodeID) const
{
  if (!this->GetMRMLScene())
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": MRML scene is not set");
    }
    return;
  }
  this->SetSourceVolumeNode(this->GetMRMLScene()->GetNodeByID(nodeID.c_str()));
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::SetSourceRepresentationToBinaryLabelMap() const
{
  if (!this->GetSegmentation())
  {
    return false;
  }

  MRMLNodeModifyBlocker blocker(this->GetSegmentationNode());

  // All other representations are invalidated when changing to binary labelmap.
  // Re-creating closed surface if it was present before, so that changes can be seen.
  bool closedSurfacePresent = this->ContainsClosedSurfaceRepresentation();
  bool createBinaryLabelmapRepresentationSuccess = this->GetSegmentation()->CreateRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  if (!createBinaryLabelmapRepresentationSuccess)
  {
    return false;
  }

  this->GetSegmentation()->SetSourceRepresentationName(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());

  if (closedSurfacePresent)
  {
    this->GetSegmentation()->CreateRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  }

  // Show binary labelmap in 2D
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(GetSegmentationNode()->GetDisplayNode());
  if (displayNode)
  {
    displayNode->SetPreferredDisplayRepresentationName2D(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  }

  return true;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SetVerbose(bool isVerbose)
{
  this->IsVerbose = isVerbose;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ToggleSegmentationSurfaceRepresentation(bool isSurfaceRepresentationOn) const
{
  if (!this->SegmentEditorNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment editor parameter set node");
    }
    return;
  }

  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode->GetSegmentationNode();
  if (!segmentationNode)
  {
    return;
  }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  if (!displayNode)
  {
    return;
  }

  MRMLNodeModifyBlocker segmentationNodeBlocker(segmentationNode);
  MRMLNodeModifyBlocker displayNodeBlocker(displayNode);

  // If just have been checked, then create closed surface representation and show it
  if (isSurfaceRepresentationOn)
  {
    // Make sure closed surface representation exists
    if (segmentationNode->GetSegmentation()->CreateRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()))
    {
      // Set closed surface as displayed poly data representation
      displayNode->SetPreferredDisplayRepresentationName3D(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
      // But keep binary labelmap for 2D
      if (segmentationNode->GetSegmentation()->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
      {
        displayNode->SetPreferredDisplayRepresentationName2D(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
      }
    }
  }
  // If unchecked, then remove representation (but only if it's not the source representation)
  else if (segmentationNode->GetSegmentation()->GetSourceRepresentationName() != vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName())
  {
    segmentationNode->GetSegmentation()->RemoveRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ToggleSourceVolumeIntensityMask() const
{
  if (!this->SegmentEditorNode)
  {
    return;
  }
  this->SegmentEditorNode->SetSourceVolumeIntensityMask(!this->SegmentEditorNode->GetSourceVolumeIntensityMask());
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::TrivialSetSourceRepresentationToBinaryLabelmap() const
{
  if (this->CanTriviallyConvertSourceRepresentationToBinaryLabelMap())
  {
    MRMLNodeModifyBlocker blocker(this->GetSegmentationNode());
    this->GetSegmentationNode()->GetSegmentation()->SetSourceRepresentationName(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::Undo() const
{
  if (!this->GetSegmentationNode() || !this->SegmentationHistory)
  {
    return;
  }

  MRMLNodeModifyBlocker blocker(this->GetSegmentationNode());
  this->SegmentationHistory->RestorePreviousState();
  this->GetSegmentationNode()->InvokeCustomModifiedEvent(vtkMRMLDisplayableNode::DisplayModifiedEvent, this->GetSegmentationNode()->GetDisplayNode());
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::UpdateVolume(void* volumeToUpdate, bool& success)
{
  if (volumeToUpdate == this->AlignedSourceVolume)
  {
    success = this->UpdateAlignedSourceVolume();
  }
  else if (volumeToUpdate == this->ModifierLabelmap)
  {
    success = this->ResetModifierLabelmapToDefault();
  }
  else if (volumeToUpdate == this->MaskLabelmap)
  {
    success = this->UpdateMaskLabelmap();
  }
  else if (volumeToUpdate == this->SelectedSegmentLabelmap)
  {
    success = this->UpdateSelectedSegmentLabelmap();
  }
  else if (volumeToUpdate == this->ReferenceGeometryImage)
  {
    success = this->UpdateReferenceGeometryImage();
  }
  else
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Failed to update volume");
    }
    success = false;
  }
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::UpdateSelectedSegmentLabelmap() const
{
  if (!this->SegmentEditorNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment editor parameter set node");
    }
    return false;
  }

  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode->GetSegmentationNode();
  std::string referenceImageGeometry = this->GetReferenceImageGeometryString();
  if (!segmentationNode || referenceImageGeometry.empty())
  {
    return false;
  }
  const char* selectedSegmentID = this->SegmentEditorNode->GetSelectedSegmentID();
  if (!selectedSegmentID)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment selection");
    }
    return false;
  }

  vtkSegment* selectedSegment = segmentationNode->GetSegmentation()->GetSegment(selectedSegmentID);
  if (selectedSegment == nullptr)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Failed: Segment " << selectedSegmentID << " not found in segmentation");
    }
    return false;
  }
  vtkOrientedImageData* segmentLabelmap =
    vtkOrientedImageData::SafeDownCast(selectedSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));
  if (!segmentLabelmap)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Failed to get binary labelmap representation in segmentation " << segmentationNode->GetName());
    }
    return false;
  }
  int* extent = segmentLabelmap->GetExtent();
  if (extent[0] > extent[1] || extent[2] > extent[3] || extent[4] > extent[5])
  {
    vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, this->SelectedSegmentLabelmap, false);
    this->SelectedSegmentLabelmap->SetExtent(0, -1, 0, -1, 0, -1);
    this->SelectedSegmentLabelmap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    return true;
  }

  vtkNew<vtkImageThreshold> threshold;
  threshold->SetInputData(segmentLabelmap);
  threshold->ThresholdBetween(selectedSegment->GetLabelValue(), selectedSegment->GetLabelValue());
  threshold->SetInValue(1);
  threshold->SetOutValue(0);
  threshold->Update();

  vtkSmartPointer<vtkOrientedImageData> thresholdedSegmentLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
  thresholdedSegmentLabelmap->ShallowCopy(threshold->GetOutput());
  thresholdedSegmentLabelmap->CopyDirections(segmentLabelmap);

  vtkNew<vtkOrientedImageData> referenceImage;
  vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, referenceImage.GetPointer(), false);
  vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(
    thresholdedSegmentLabelmap, referenceImage.GetPointer(), this->SelectedSegmentLabelmap, /*linearInterpolation=*/false);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::UpdateAlignedSourceVolume()
{
  if (!this->SegmentEditorNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment editor parameter set node");
    }
    return false;
  }

  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode->GetSegmentationNode();
  vtkMRMLScalarVolumeNode* sourceVolumeNode = this->SegmentEditorNode->GetSourceVolumeNode();
  std::string referenceImageGeometry = this->GetReferenceImageGeometryString();
  if (!segmentationNode || !sourceVolumeNode || !sourceVolumeNode->GetImageData() //
      || !sourceVolumeNode->GetImageData()->GetPointData() || referenceImageGeometry.empty())
  {
    return false;
  }

  vtkNew<vtkOrientedImageData> referenceImage;
  vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, referenceImage.GetPointer(), false);

  int* referenceImageExtent = referenceImage->GetExtent();
  int* alignedSourceVolumeExtent = this->AlignedSourceVolume->GetExtent();
  // If source volume node and transform nodes did not change and the aligned source volume covers the entire reference geometry
  // then we don't need to update the aligned source volume.
  if (vtkOrientedImageDataResample::DoGeometriesMatch(referenceImage.GetPointer(), this->AlignedSourceVolume)               //
      && alignedSourceVolumeExtent[0] <= referenceImageExtent[0] && alignedSourceVolumeExtent[1] >= referenceImageExtent[1] //
      && alignedSourceVolumeExtent[2] <= referenceImageExtent[2] && alignedSourceVolumeExtent[3] >= referenceImageExtent[3] //
      && alignedSourceVolumeExtent[4] <= referenceImageExtent[4] && alignedSourceVolumeExtent[5] >= referenceImageExtent[5] //
      && vtkOrientedImageDataResample::DoExtentsMatch(referenceImage.GetPointer(), this->AlignedSourceVolume)               //
      && this->AlignedSourceVolumeUpdateSourceVolumeNode == sourceVolumeNode                                                //
      && this->AlignedSourceVolumeUpdateSourceVolumeNodeTransform == sourceVolumeNode->GetParentTransformNode()             //
      && this->AlignedSourceVolumeUpdateSegmentationNodeTransform == segmentationNode->GetParentTransformNode())
  {
    // Extents and nodes are matching, check if they have not been modified since the aligned master
    // volume generation.
    bool updateAlignedSourceVolumeRequired = false;
    if (sourceVolumeNode->GetMTime() > this->AlignedSourceVolume->GetMTime())
    {
      updateAlignedSourceVolumeRequired = true;
    }
    else if (sourceVolumeNode->GetParentTransformNode() && sourceVolumeNode->GetParentTransformNode()->GetMTime() > this->AlignedSourceVolume->GetMTime())
    {
      updateAlignedSourceVolumeRequired = true;
    }
    else if (segmentationNode->GetParentTransformNode() && segmentationNode->GetParentTransformNode()->GetMTime() > this->AlignedSourceVolume->GetMTime())
    {
      updateAlignedSourceVolumeRequired = true;
    }
    if (!updateAlignedSourceVolumeRequired)
    {
      return true;
    }
  }

  // Get a read-only version of sourceVolume as a vtkOrientedImageData
  vtkNew<vtkOrientedImageData> sourceVolume;

  if (sourceVolumeNode->GetImageData()->GetNumberOfScalarComponents() == 1)
  {
    sourceVolume->vtkImageData::ShallowCopy(sourceVolumeNode->GetImageData());
  }
  else
  {
    vtkNew<vtkImageExtractComponents> extract;
    extract->SetInputData(sourceVolumeNode->GetImageData());
    extract->Update();
    extract->SetComponents(0); // TODO: allow user to configure this
    sourceVolume->vtkImageData::ShallowCopy(extract->GetOutput());
  }
  vtkSmartPointer<vtkMatrix4x4> ijkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  sourceVolumeNode->GetIJKToRASMatrix(ijkToRasMatrix);
  sourceVolume->SetGeometryFromImageToWorldMatrix(ijkToRasMatrix);

  vtkNew<vtkGeneralTransform> sourceVolumeToSegmentationTransform;
  vtkMRMLTransformNode::GetTransformBetweenNodes(
    sourceVolumeNode->GetParentTransformNode(), segmentationNode->GetParentTransformNode(), sourceVolumeToSegmentationTransform.GetPointer());

  double backgroundValue = sourceVolumeNode->GetImageBackgroundScalarComponentAsDouble(0);
  vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(sourceVolume,
                                                                              referenceImage,
                                                                              this->AlignedSourceVolume,
                                                                              /*linearInterpolation=*/true,
                                                                              /*padImage=*/false,
                                                                              sourceVolumeToSegmentationTransform,
                                                                              backgroundValue);

  this->AlignedSourceVolumeUpdateSourceVolumeNode = sourceVolumeNode;
  this->AlignedSourceVolumeUpdateSourceVolumeNodeTransform = sourceVolumeNode->GetParentTransformNode();
  this->AlignedSourceVolumeUpdateSegmentationNodeTransform = segmentationNode->GetParentTransformNode();

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::UpdateMaskLabelmap() const
{
  if (!this->SegmentEditorNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment editor parameter set node");
    }
    return false;
  }
  vtkMRMLSegmentationNode* segmentationNode = this->SegmentEditorNode->GetSegmentationNode();
  if (!segmentationNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segmentation node");
    }
    return false;
  }

  std::string referenceGeometryStr = this->GetReferenceImageGeometryString();
  if (referenceGeometryStr.empty())
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Cannot determine mask labelmap geometry");
    }
    return false;
  }
  vtkNew<vtkOrientedImageData> referenceGeometry;
  if (!vtkSegmentationConverter::DeserializeImageGeometry(referenceGeometryStr, referenceGeometry, false))
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Cannot determine mask labelmap geometry");
    }
    return false;
  }

  // GenerateEditMask can add intensity range based mask, too. We do not use it here, as currently
  // editable intensity range is taken into account in vtkSlicerSegmentEditorLogic::modifySelectedSegmentByLabelmap.
  // It would simplify implementation if we passed source volume and intensity range to GenerateEditMask here
  // and removed intensity range based masking from modifySelectedSegmentByLabelmap.
  if (!segmentationNode->GenerateEditMask(this->MaskLabelmap,
                                          this->SegmentEditorNode->GetMaskMode(),
                                          referenceGeometry,
                                          this->SegmentEditorNode->GetSelectedSegmentID() ? this->SegmentEditorNode->GetSelectedSegmentID() : "",
                                          this->SegmentEditorNode->GetMaskSegmentID() ? this->SegmentEditorNode->GetMaskSegmentID() : ""))
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Mask generation failed");
    }
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::UpdateReferenceGeometryImage() const
{
  std::string geometry = this->GetReferenceImageGeometryString();
  if (geometry.empty())
  {
    return false;
  }
  return vtkSegmentationConverter::DeserializeImageGeometry(geometry, this->ReferenceGeometryImage, false /* don't allocate scalars */);
}

//-----------------------------------------------------------------------------
vtkSlicerSegmentEditorLogic::vtkSlicerSegmentEditorLogic()
  : SegmentEditorNode(nullptr)
  , SegmentationHistory(nullptr)
  , AlignedSourceVolume(vtkSmartPointer<vtkOrientedImageData>::New())
  , ModifierLabelmap(vtkSmartPointer<vtkOrientedImageData>::New())
  , SelectedSegmentLabelmap(vtkSmartPointer<vtkOrientedImageData>::New())
  , MaskLabelmap(vtkSmartPointer<vtkOrientedImageData>::New())
  , ReferenceGeometryImage(vtkSmartPointer<vtkOrientedImageData>::New())
  , AlignedSourceVolumeUpdateSourceVolumeNode(nullptr)
  , AlignedSourceVolumeUpdateSourceVolumeNodeTransform(nullptr)
  , AlignedSourceVolumeUpdateSegmentationNodeTransform(nullptr)
  , SegmentHistoryObs(0)
  , SegmentationNodeObs(nullptr)
  , IsVerbose(false)
{
  SetSegmentationHistory(vtkSmartPointer<vtkSegmentationHistory>::New());
}

//-----------------------------------------------------------------------------
vtkSlicerSegmentEditorLogic::~vtkSlicerSegmentEditorLogic()
{
  SetSegmentEditorNode(nullptr);
  UpdateSegmentationNodeObserver(nullptr);
}

//---------------------------------------------------------------------------
bool IsSegmentIDValid(const std::string& segmentID)
{
  return !segmentID.empty();
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::AppendImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* appendedImage)
{
  if (!inputImage || !appendedImage)
  {
    vtkErrorWithObjectMacro(nullptr, "" << __func__ << ": Invalid inputs!");
    return;
  }

  // Make sure appended image has the same lattice as the input image
  vtkSmartPointer<vtkOrientedImageData> resampledAppendedImage = vtkSmartPointer<vtkOrientedImageData>::New();
  vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(appendedImage, inputImage, resampledAppendedImage);

  // Add image created from poly data to input image
  vtkSmartPointer<vtkImageMathematics> imageMath = vtkSmartPointer<vtkImageMathematics>::New();
  imageMath->SetInput1Data(inputImage);
  imageMath->SetInput2Data(resampledAppendedImage);
  imageMath->SetOperationToMax();
  imageMath->Update();
  inputImage->DeepCopy(imageMath->GetOutput());
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::AppendPolyMask(vtkOrientedImageData* input,
                                                 vtkPolyData* polyData,
                                                 vtkMRMLSliceNode* sliceWidget,
                                                 vtkMRMLSegmentationNode* segmentationNode /*=nullptr*/)
{
  // Rasterize a poly data onto the input image into the slice view
  // - Points are specified in current XY space
  vtkSmartPointer<vtkOrientedImageData> polyMaskImage = vtkSmartPointer<vtkOrientedImageData>::New();
  vtkSlicerSegmentEditorLogic::CreateMaskImageFromPolyData(polyData, polyMaskImage, sliceWidget);

  if (segmentationNode && segmentationNode->GetParentTransformNode())
  {
    if (segmentationNode->GetParentTransformNode()->IsTransformToWorldLinear())
    {
      vtkNew<vtkMatrix4x4> worldToSegmentation;
      segmentationNode->GetParentTransformNode()->GetMatrixTransformFromWorld(worldToSegmentation);
      vtkNew<vtkMatrix4x4> imageToWorldMatrix;
      polyMaskImage->GetImageToWorldMatrix(imageToWorldMatrix);
      vtkNew<vtkMatrix4x4> imageToSegmentation;
      vtkMatrix4x4::Multiply4x4(worldToSegmentation, imageToWorldMatrix, imageToSegmentation);
      polyMaskImage->SetImageToWorldMatrix(imageToSegmentation);
    }
    else
    {
      vtkErrorWithObjectMacro(nullptr, "" << __func__ << ": Parent transform is non-linear, which cannot be handled! Skipping.");
    }
  }

  // Append poly mask onto input image
  vtkSlicerSegmentEditorLogic::AppendImage(input, polyMaskImage);
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::CreateMaskImageFromPolyData(vtkPolyData* polyData, vtkOrientedImageData* outputMask, vtkMRMLSliceNode* sliceNode)
{
  if (!polyData || !outputMask)
  {
    vtkErrorWithObjectMacro(nullptr, "" << __func__ << ": Invalid inputs!");
    return;
  }
  if (!sliceNode)
  {
    vtkErrorWithObjectMacro(nullptr, "" << __func__ << ": Failed to get slice node!");
    return;
  }

  // Need to know the mapping from RAS into polygon space
  // so the painter can use this as a mask
  // - Need the bounds in RAS space
  // - Need to get an IJKToRAS for just the mask area
  // - Directions are the XYToRAS for this slice
  // - Origin is the lower left of the polygon bounds
  // - TODO: need to account for the boundary pixels
  //
  // Note: uses the slicer2-based vtkImageFillROI filter
  vtkSmartPointer<vtkMatrix4x4> maskIjkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  maskIjkToRasMatrix->DeepCopy(sliceNode->GetXYToRAS());

  polyData->GetPoints()->Modified();
  double bounds[6] = { 0, 0, 0, 0, 0, 0 };
  polyData->GetBounds(bounds);

  double xlo = bounds[0] - 1.0;
  double xhi = bounds[1];
  double ylo = bounds[2] - 1.0;
  double yhi = bounds[3];

  double originXYZ[3] = { xlo, ylo, 0.0 };
  double originRAS[3] = { 0.0, 0.0, 0.0 };
  vtkSlicerSegmentEditorLogic::XyzToRas(originXYZ, originRAS, sliceNode);

  maskIjkToRasMatrix->SetElement(0, 3, originRAS[0]);
  maskIjkToRasMatrix->SetElement(1, 3, originRAS[1]);
  maskIjkToRasMatrix->SetElement(2, 3, originRAS[2]);

  // Get a good size for the draw buffer
  // - Needs to include the full region of the polygon
  // - Plus a little extra
  //
  // Round to int and add extra pixel for both sides
  // TODO: figure out why we need to add buffer pixels on each
  //   side for the width in order to end up with a single extra
  //   pixel in the rasterized image map.  Probably has to
  //   do with how boundary conditions are handled in the filler
  int w = static_cast<int>(xhi - xlo) + 32;
  int h = static_cast<int>(yhi - ylo) + 32;

  vtkSmartPointer<vtkOrientedImageData> imageData = vtkSmartPointer<vtkOrientedImageData>::New();
  imageData->SetDimensions(w, h, 1);
  imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  // Move the points so the lower left corner of the bounding box is at 1, 1 (to avoid clipping)
  vtkSmartPointer<vtkTransform> translate = vtkSmartPointer<vtkTransform>::New();
  translate->Translate(-xlo, -ylo, 0.0);

  vtkSmartPointer<vtkPoints> drawPoints = vtkSmartPointer<vtkPoints>::New();
  drawPoints->Reset();
  translate->TransformPoints(polyData->GetPoints(), drawPoints);
  drawPoints->Modified();

  vtkSmartPointer<vtkImageFillROI> fill = vtkSmartPointer<vtkImageFillROI>::New();
  fill->SetInputData(imageData);
  fill->SetValue(1);
  fill->SetPoints(drawPoints);
  fill->Update();

  outputMask->DeepCopy(fill->GetOutput());
  outputMask->SetGeometryFromImageToWorldMatrix(maskIjkToRasMatrix);
}

//-----------------------------------------------------------------------------
int vtkSlicerSegmentEditorLogic::GetCurrentSegmentIndex() const
{
  std::vector<std::string> segmentIds = this->GetSegmentIDs();
  std::string currentId = this->GetCurrentSegmentID();
  if (!IsSegmentIdValid(currentId))
  {
    return -1;
  }

  auto it = std::find(std::begin(segmentIds), std::end(segmentIds), currentId);
  if (it == std::end(segmentIds))
  {
    return -1;
  }

  return static_cast<int>(std::distance(std::begin(segmentIds), it));
}

//---------------------------------------------------------------------------
std::string vtkSlicerSegmentEditorLogic::GetNextSegmentID(int offset, bool visibleOnly) const
{
  // Return current segment ID if offset is 0 or current index is invalid.
  int iCurrent = this->GetCurrentSegmentIndex();
  if (offset == 0 || iCurrent < 0)
  {
    return this->GetCurrentSegmentID();
  }

  // Iterate over segment IDs and filter offset by all or visible only depending on the input flag
  std::vector<std::string> segmentIds = this->GetSegmentIDs();
  std::vector<std::string> filterIds = visibleOnly ? this->GetVisibleSegmentIDs() : segmentIds;

  int absOffset = std::abs(offset);
  int count = 0;
  int increment = offset > 0 ? 1 : -1;
  while (count < absOffset)
  {
    iCurrent += increment;

    // If out of range break and return empty.
    if (iCurrent < 0 || iCurrent >= segmentIds.size())
    {
      break;
    }

    // Increment offset count if currentID matches filtered ID
    std::string currentId = segmentIds[iCurrent];
    if (IsSegmentIdInList(currentId, filterIds))
    {
      count++;
      if (count == absOffset)
      {
        return currentId;
      }
    }
  }

  return {};
}

vtkSegment* vtkSlicerSegmentEditorLogic::GetSelectedSegment() const
{
  if (!this->GetSegmentEditorNode())
  {
    return nullptr;
  }

  vtkMRMLSegmentationNode* segmentationNode = this->GetSegmentEditorNode()->GetSegmentationNode();
  if (!segmentationNode)
  {
    return nullptr;
  }

  char* segmentID = this->GetSegmentEditorNode()->GetSelectedSegmentID();
  if (!segmentID || strlen(segmentID) == 0)
  {
    return nullptr;
  }

  vtkSegment* selectedSegment = nullptr;
  if (segmentationNode->GetSegmentation())
  {
    selectedSegment = segmentationNode->GetSegmentation()->GetSegment(segmentID);
  }
  return selectedSegment;
}

//----------------------------------------------------------------------------
double vtkSlicerSegmentEditorLogic::GetSliceSpacing(vtkMRMLSliceNode* sliceNode) const
{
  vtkMRMLSliceLogic* sliceLogic = this->GetMRMLApplicationLogic() ? this->GetMRMLApplicationLogic()->GetSliceLogic(sliceNode) : nullptr;
  return vtkSlicerSegmentEditorLogic::GetSliceSpacing(sliceNode, sliceLogic);
}

//----------------------------------------------------------------------------
double vtkSlicerSegmentEditorLogic::GetSliceSpacing(vtkMRMLSliceNode* sliceNode, vtkMRMLSliceLogic* sliceLogic)
{
  // Implementation copied from vtkMRMLSliceViewInteractorStyle::GetSliceSpacing()
  double defaultSpacing = 1.0;
  if (!sliceNode)
  {
    return defaultSpacing;
  }

  if (sliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode)
  {
    return sliceNode->GetPrescribedSliceSpacing()[2];
  }

  if (!sliceLogic)
  {
    return defaultSpacing;
  }

  return sliceLogic->GetLowestVolumeSliceSpacing()[2];
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::GetVerbose() const
{
  return this->IsVerbose;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ImageToWorldMatrix(vtkMRMLVolumeNode* node, vtkMatrix4x4* ijkToRas)
{
  if (!node || !ijkToRas)
  {
    return;
  }

  node->GetIJKToRASMatrix(ijkToRas);

  if (vtkMRMLTransformNode* transformNode = node->GetParentTransformNode())
  {
    if (transformNode->IsTransformToWorldLinear())
    {
      vtkSmartPointer<vtkMatrix4x4> volumeRasToWorldRas = vtkSmartPointer<vtkMatrix4x4>::New();
      transformNode->GetMatrixTransformToWorld(volumeRasToWorldRas);
      vtkMatrix4x4::Multiply4x4(volumeRasToWorldRas, ijkToRas, ijkToRas);
    }
    else
    {
      vtkErrorWithObjectMacro(nullptr, "" << __func__ << ": Parent transform is non-linear, which cannot be handled! Skipping.");
    }
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ImageToWorldMatrix(vtkOrientedImageData* image, vtkMRMLSegmentationNode* node, vtkMatrix4x4* ijkToRas)
{
  if (!image || !node || !ijkToRas)
  {
    return;
  }

  image->GetImageToWorldMatrix(ijkToRas);

  if (vtkMRMLTransformNode* transformNode = node->GetParentTransformNode())
  {
    if (transformNode->IsTransformToWorldLinear())
    {
      vtkSmartPointer<vtkMatrix4x4> segmentationRasToWorldRas = vtkSmartPointer<vtkMatrix4x4>::New();
      transformNode->GetMatrixTransformToWorld(segmentationRasToWorldRas);
      vtkMatrix4x4::Multiply4x4(segmentationRasToWorldRas, ijkToRas, ijkToRas);
    }
    else
    {
      vtkErrorWithObjectMacro(nullptr, "" << __func__ << ": Parent transform is non-linear, which cannot be handled! Skipping.");
    }
  }
}

//---------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::IsSegmentIdInList(const std::string& segmentID, const std::vector<std::string>& visibleSegmentIDs)
{
  return std::find(std::begin(visibleSegmentIDs), std::end(visibleSegmentIDs), segmentID) != std::end(visibleSegmentIDs);
}

//---------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::IsSegmentIdVisible(const std::string& segmentID) const
{
  return vtkSlicerSegmentEditorLogic::IsSegmentIdInList(segmentID, this->GetVisibleSegmentIDs());
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentEditorLogic::IsSelectedSegmentVisible() const
{
  vtkSegment* selectedSegment = this->GetSelectedSegment();

  if (!selectedSegment)
  {
    return true;
  }

  vtkMRMLSegmentationNode* segmentationNode = this->GetSegmentEditorNode()->GetSegmentationNode();
  int numberOfDisplayNodes = segmentationNode->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
  {
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetNthDisplayNode(displayNodeIndex));
    if (displayNode && displayNode->GetVisibility() && displayNode->GetSegmentVisibility(this->GetSegmentEditorNode()->GetSelectedSegmentID()))
    {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ModifySegmentByLabelmap(vtkMRMLSegmentationNode* segmentationNode,
                                                          const char* segmentID,
                                                          vtkOrientedImageData* modifierLabelmap,
                                                          ModificationMode modificationMode,
                                                          bool isPerSegment,
                                                          bool bypassMasking)
{
  int modificationExtent[6] = { 0, -1, 0, -1, 0, -1 };
  this->ModifySegmentByLabelmap(segmentationNode, segmentID, modifierLabelmap, modificationMode, modificationExtent, isPerSegment, bypassMasking);
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ModifySegmentByLabelmap(vtkMRMLSegmentationNode* segmentationNode,
                                                          const char* segmentID,
                                                          vtkOrientedImageData* modifierLabelmapInput,
                                                          ModificationMode modificationMode,
                                                          const int modificationExtent[6],
                                                          bool isPerSegment,
                                                          bool bypassMasking)
{
  vtkMRMLSegmentEditorNode* parameterSetNode = this->GetSegmentEditorNode();
  if (!parameterSetNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment editor parameter set node");
    }
    return;
  }

  if (!segmentationNode)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segmentation");
    }
    return;
  }

  vtkSegment* segment = nullptr;
  if (segmentID)
  {
    segment = segmentationNode->GetSegmentation()->GetSegment(segmentID);
  }
  if (!segment)
  {
    if (this->IsVerbose)
    {
      vtkErrorMacro("" << __func__ << ": Invalid segment");
    }
    return;
  }

  if (!modifierLabelmapInput)
  {
    // If per-segment flag is off, then it is not an error (the effect itself has written it back to segmentation)
    if (isPerSegment)
    {
      if (this->IsVerbose)
      {
        vtkErrorMacro("" << __func__ << ": Cannot apply edit operation because modifier labelmap cannot be accessed");
      }
    }
    return;
  }

  // Prevent disappearing and reappearing of 3D representation during update
  RenderBlocker renderBlocker(this);
  constexpr float eraseValue{ 0.0 };

  vtkSmartPointer<vtkOrientedImageData> modifierLabelmap = modifierLabelmapInput;
  if ((!bypassMasking && parameterSetNode->GetMaskMode() != vtkMRMLSegmentationNode::EditAllowedEverywhere) || parameterSetNode->GetSourceVolumeIntensityMask())
  {
    constexpr float fillValue{ 1.0 };
    vtkNew<vtkOrientedImageData> maskImage;
    maskImage->SetExtent(modifierLabelmap->GetExtent());
    maskImage->SetSpacing(modifierLabelmap->GetSpacing());
    maskImage->SetOrigin(modifierLabelmap->GetOrigin());
    maskImage->CopyDirections(modifierLabelmap);
    maskImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    vtkOrientedImageDataResample::FillImage(maskImage, eraseValue);

    // Apply mask to modifier labelmap if masking is enabled
    if (!bypassMasking && parameterSetNode->GetMaskMode() != vtkMRMLSegmentationNode::EditAllowedEverywhere)
    {
      if (!this->UpdateMaskLabelmap())
      {
        if (this->IsVerbose)
        {
          vtkErrorMacro("" << __func__ << ": Failed to update mask labelmap");
        }
        return;
      }
      vtkOrientedImageDataResample::ModifyImage(maskImage, this->GetMaskLabelmap(), vtkOrientedImageDataResample::OPERATION_MAXIMUM);
    }

    // Apply threshold mask if paint threshold is turned on
    if (parameterSetNode->GetSourceVolumeIntensityMask())
    {
      if (!this->UpdateAlignedSourceVolume())
      {
        if (this->IsVerbose)
        {
          vtkErrorMacro("" << __func__ << ": Unable to update volume image");
        }
        return;
      }

      vtkOrientedImageData* sourceVolumeOrientedImageData = this->GetAlignedSourceVolume();
      if (!sourceVolumeOrientedImageData)
      {
        if (this->IsVerbose)
        {
          vtkErrorMacro("" << __func__ << ": Source volume image is empty");
        }
        return;
      }

      // Make sure the modifier labelmap has the same geometry as the source volume
      if (!vtkOrientedImageDataResample::DoGeometriesMatch(modifierLabelmap, sourceVolumeOrientedImageData))
      {
        if (this->IsVerbose)
        {
          vtkErrorMacro("" << __func__ << ": Modifier labelmap should have the same geometry as the source volume");
        }
        return;
      }

      // Create threshold image
      vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
      threshold->SetInputData(sourceVolumeOrientedImageData);
      threshold->ThresholdBetween(parameterSetNode->GetSourceVolumeIntensityMaskRange()[0], parameterSetNode->GetSourceVolumeIntensityMaskRange()[1]);
      threshold->SetInValue(eraseValue);
      threshold->SetOutValue(fillValue);
      threshold->SetOutputScalarTypeToUnsignedChar();
      threshold->Update();

      vtkSmartPointer<vtkOrientedImageData> thresholdMask = vtkSmartPointer<vtkOrientedImageData>::New();
      thresholdMask->ShallowCopy(threshold->GetOutput());
      vtkSmartPointer<vtkMatrix4x4> modifierLabelmapToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      modifierLabelmap->GetImageToWorldMatrix(modifierLabelmapToWorldMatrix);
      thresholdMask->SetGeometryFromImageToWorldMatrix(modifierLabelmapToWorldMatrix);
      vtkOrientedImageDataResample::ModifyImage(maskImage, thresholdMask, vtkOrientedImageDataResample::OPERATION_MAXIMUM);
    }

    vtkSmartPointer<vtkOrientedImageData> segmentLayerLabelmap =
      vtkOrientedImageData::SafeDownCast(segment->GetRepresentation(segmentationNode->GetSegmentation()->GetSourceRepresentationName()));
    if (segmentLayerLabelmap && this->GetSegmentEditorNode()->GetMaskMode() == vtkMRMLSegmentationNode::EditAllowedInsideSingleSegment
        && modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeRemove)
    {
      // If we are painting inside a segment, the erase effect can modify the current segment outside the masking region by adding back regions
      // in the current segment. Add the current segment to the editable area
      vtkNew<vtkImageThreshold> segmentInverter;
      segmentInverter->SetInputData(segmentLayerLabelmap);
      segmentInverter->SetInValue(eraseValue);
      segmentInverter->SetOutValue(fillValue);
      segmentInverter->ReplaceInOn();
      segmentInverter->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
      segmentInverter->SetOutputScalarTypeToUnsignedChar();
      segmentInverter->Update();

      vtkNew<vtkOrientedImageData> invertedSegment;
      invertedSegment->ShallowCopy(segmentInverter->GetOutput());
      invertedSegment->CopyDirections(segmentLayerLabelmap);
      vtkOrientedImageDataResample::ModifyImage(maskImage, invertedSegment, vtkOrientedImageDataResample::OPERATION_MINIMUM);
    }

    // Apply the mask to the modifier labelmap. Make a copy so that we don't modify the original.
    modifierLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
    modifierLabelmap->DeepCopy(modifierLabelmapInput);
    vtkOrientedImageDataResample::ApplyImageMask(modifierLabelmap, maskImage, eraseValue, true);

    if (segmentLayerLabelmap && modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeSet)
    {
      // If modification mode is "set", we don't want to erase the existing labelmap outside the mask region,
      // so we need to add it to the modifier labelmap
      vtkNew<vtkImageThreshold> segmentThreshold;
      segmentThreshold->SetInputData(segmentLayerLabelmap);
      segmentThreshold->SetInValue(fillValue);
      segmentThreshold->SetOutValue(eraseValue);
      segmentThreshold->ReplaceInOn();
      segmentThreshold->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
      segmentThreshold->SetOutputScalarTypeToUnsignedChar();
      segmentThreshold->Update();

      int segmentThresholdExtent[6] = { 0, -1, 0, -1, 0, -1 };
      segmentThreshold->GetOutput()->GetExtent(segmentThresholdExtent);
      if (segmentThresholdExtent[0] <= segmentThresholdExtent[1] && segmentThresholdExtent[2] <= segmentThresholdExtent[3]
          && segmentThresholdExtent[4] <= segmentThresholdExtent[5])
      {
        vtkNew<vtkOrientedImageData> segmentOutsideMask;
        segmentOutsideMask->ShallowCopy(segmentThreshold->GetOutput());
        segmentOutsideMask->CopyDirections(segmentLayerLabelmap);
        vtkOrientedImageDataResample::ModifyImage(segmentOutsideMask, maskImage, vtkOrientedImageDataResample::OPERATION_MINIMUM);
        vtkOrientedImageDataResample::ModifyImage(modifierLabelmap, segmentOutsideMask, vtkOrientedImageDataResample::OPERATION_MAXIMUM);
      }
    }
  }

  // Copy the temporary padded modifier labelmap to the segment.
  // Mask and threshold was already applied on modifier labelmap at this point if requested.
  const int* extent = modificationExtent;
  if (extent[0] > extent[1] || extent[2] > extent[3] || extent[4] > extent[5])
  {
    // invalid extent, it means we have to work with the entire modifier labelmap
    extent = nullptr;
  }

  std::vector<std::string> allSegmentIDs;
  segmentationNode->GetSegmentation()->GetSegmentIDs(allSegmentIDs);
  // remove selected segment, that is already handled
  allSegmentIDs.erase(std::remove(allSegmentIDs.begin(), allSegmentIDs.end(), segmentID), allSegmentIDs.end());

  std::vector<std::string> visibleSegmentIDs;
  if (vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode()))
  {
    for (const auto& segmentId : allSegmentIDs)
    {
      if (displayNode->GetSegmentVisibility(segmentId))
      {
        visibleSegmentIDs.push_back(segmentId);
      }
    }
  }

  std::vector<std::string> segmentIDsToOverwrite;
  switch (this->GetSegmentEditorNode()->GetOverwriteMode())
  {
    case vtkMRMLSegmentEditorNode::OverwriteNone:
      // nothing to overwrite
      break;
    case vtkMRMLSegmentEditorNode::OverwriteVisibleSegments: segmentIDsToOverwrite = visibleSegmentIDs; break;
    case vtkMRMLSegmentEditorNode::OverwriteAllSegments:
    default: segmentIDsToOverwrite = allSegmentIDs; break;
  }

  if (bypassMasking)
  {
    segmentIDsToOverwrite.clear();
  }

  if (modificationMode == ModificationModeRemoveAll)
  {
    // If we want to erase all segments, then mark all segments as overwritable
    segmentIDsToOverwrite = allSegmentIDs;
  }

  // Create inverted binary labelmap
  vtkSmartPointer<vtkImageThreshold> inverter = vtkSmartPointer<vtkImageThreshold>::New();
  inverter->SetInputData(modifierLabelmap);
  inverter->SetInValue(VTK_UNSIGNED_CHAR_MAX);
  inverter->SetOutValue(eraseValue);
  inverter->ThresholdByLower(0);
  inverter->SetOutputScalarTypeToUnsignedChar();

  if (modificationMode == ModificationModeSet)
  {
    vtkSmartPointer<vtkImageThreshold> segmentInverter = vtkSmartPointer<vtkImageThreshold>::New();
    segmentInverter->SetInputData(segment->GetRepresentation(segmentationNode->GetSegmentation()->GetSourceRepresentationName()));
    segmentInverter->SetInValue(eraseValue);
    segmentInverter->SetOutValue(VTK_UNSIGNED_CHAR_MAX);
    segmentInverter->ReplaceInOn();
    segmentInverter->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
    segmentInverter->SetOutputScalarTypeToUnsignedChar();
    segmentInverter->Update();
    vtkNew<vtkOrientedImageData> invertedModifierLabelmap;
    invertedModifierLabelmap->ShallowCopy(segmentInverter->GetOutput());
    vtkNew<vtkMatrix4x4> imageToWorldMatrix;
    modifierLabelmap->GetImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    invertedModifierLabelmap->SetGeometryFromImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
          invertedModifierLabelmap.GetPointer(), segmentationNode, segmentID, vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MIN, nullptr, false, segmentIDsToOverwrite))
    {
      if (this->IsVerbose)
      {
        vtkErrorMacro("" << __func__ << ": Failed to remove modifier labelmap from selected segment");
      }
    }
    if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
          modifierLabelmap, segmentationNode, segmentID, vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MASK, extent, false, segmentIDsToOverwrite))
    {
      if (this->IsVerbose)
      {
        vtkErrorMacro("" << __func__ << ": Failed to add modifier labelmap to selected segment");
      }
    }
  }
  else if (modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeAdd)
  {
    if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
          modifierLabelmap, segmentationNode, segmentID, vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MASK, extent, false, segmentIDsToOverwrite))
    {
      if (this->IsVerbose)
      {
        vtkErrorMacro("" << __func__ << ": Failed to add modifier labelmap to selected segment");
      }
    }
  }
  else if (modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeRemove || modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeRemoveAll)
  {
    inverter->Update();
    vtkNew<vtkOrientedImageData> invertedModifierLabelmap;
    invertedModifierLabelmap->ShallowCopy(inverter->GetOutput());
    vtkNew<vtkMatrix4x4> imageToWorldMatrix;
    modifierLabelmap->GetImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    invertedModifierLabelmap->SetGeometryFromImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    bool minimumOfAllSegments = modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeRemoveAll;
    if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(invertedModifierLabelmap.GetPointer(),
                                                                       segmentationNode,
                                                                       segmentID,
                                                                       vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MIN,
                                                                       extent,
                                                                       minimumOfAllSegments,
                                                                       segmentIDsToOverwrite))
    {
      if (this->IsVerbose)
      {
        vtkErrorMacro("" << __func__ << ": Failed to remove modifier labelmap from selected segment");
      }
    }
  }

  if (segment)
  {
    if (vtkSlicerSegmentationsModuleLogic::GetSegmentStatus(segment) == vtkSlicerSegmentationsModuleLogic::NotStarted)
    {
      vtkSlicerSegmentationsModuleLogic::SetSegmentStatus(segment, vtkSlicerSegmentationsModuleLogic::InProgress);
    }
  }

  std::vector<std::string> sharedSegmentIDs;
  segmentationNode->GetSegmentation()->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentID, sharedSegmentIDs, false);

  std::vector<std::string> segmentsToErase;
  for (const std::string& segmentIDToOverwrite : segmentIDsToOverwrite)
  {
    std::vector<std::string>::iterator foundSegmentIDIt = std::find(sharedSegmentIDs.begin(), sharedSegmentIDs.end(), segmentIDToOverwrite);
    if (foundSegmentIDIt == sharedSegmentIDs.end())
    {
      segmentsToErase.push_back(segmentIDToOverwrite);
    }
  }

  if (!segmentsToErase.empty()
      && (modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeSet    //
          || modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeAdd //
          || modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeRemoveAll))
  {
    inverter->Update();
    vtkNew<vtkOrientedImageData> invertedModifierLabelmap;
    invertedModifierLabelmap->ShallowCopy(inverter->GetOutput());
    vtkNew<vtkMatrix4x4> imageToWorldMatrix;
    modifierLabelmap->GetImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    invertedModifierLabelmap->SetGeometryFromImageToWorldMatrix(imageToWorldMatrix.GetPointer());

    std::map<vtkDataObject*, bool> erased;
    for (const std::string& eraseSegmentID : segmentsToErase)
    {
      vtkSegment* currentSegment = segmentationNode->GetSegmentation()->GetSegment(eraseSegmentID);
      vtkDataObject* dataObject = currentSegment->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
      if (erased[dataObject])
      {
        continue;
      }
      erased[dataObject] = true;

      vtkOrientedImageData* currentLabelmap = vtkOrientedImageData::SafeDownCast(dataObject);

      std::vector<std::string> dontOverwriteIDs;
      std::vector<std::string> currentSharedIDs;
      segmentationNode->GetSegmentation()->GetSegmentIDsSharingBinaryLabelmapRepresentation(eraseSegmentID, currentSharedIDs, true);
      for (const std::string& sharedSegmentID : currentSharedIDs)
      {
        if (std::find(segmentsToErase.begin(), segmentsToErase.end(), sharedSegmentID) == segmentsToErase.end())
        {
          dontOverwriteIDs.push_back(sharedSegmentID);
        }
      }

      vtkSmartPointer<vtkOrientedImageData> invertedModifierLabelmap2 = invertedModifierLabelmap;
      if (!dontOverwriteIDs.empty())
      {
        invertedModifierLabelmap2 = vtkSmartPointer<vtkOrientedImageData>::New();
        invertedModifierLabelmap2->DeepCopy(invertedModifierLabelmap);

        vtkNew<vtkOrientedImageData> maskImage;
        maskImage->CopyDirections(currentLabelmap);
        for (const std::string& dontOverwriteID : dontOverwriteIDs)
        {
          vtkSegment* dontOverwriteSegment = segmentationNode->GetSegmentation()->GetSegment(dontOverwriteID);
          vtkNew<vtkImageThreshold> threshold;
          threshold->SetInputData(currentLabelmap);
          threshold->ThresholdBetween(dontOverwriteSegment->GetLabelValue(), dontOverwriteSegment->GetLabelValue());
          threshold->SetInValue(1);
          threshold->SetOutValue(0);
          threshold->SetOutputScalarTypeToUnsignedChar();
          threshold->Update();
          maskImage->ShallowCopy(threshold->GetOutput());
          vtkOrientedImageDataResample::ApplyImageMask(invertedModifierLabelmap2, maskImage, VTK_UNSIGNED_CHAR_MAX, true);
        }
      }

      if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
            invertedModifierLabelmap2, segmentationNode, eraseSegmentID, vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MIN, extent, true, segmentIDsToOverwrite))
      {
        if (this->IsVerbose)
        {
          vtkErrorMacro("" << __func__ << ": Failed to set modifier labelmap to segment " + eraseSegmentID);
        }
      }
    }
  }
  else if (modificationMode == vtkSlicerSegmentEditorLogic::ModificationModeRemove
           && this->GetSegmentEditorNode()->GetMaskMode() == vtkMRMLSegmentationNode::EditAllowedInsideSingleSegment && this->GetSegmentEditorNode()->GetMaskSegmentID()
           && strcmp(this->GetSegmentEditorNode()->GetMaskSegmentID(), segmentID) != 0)
  {
    // In general, we don't try to "add back" areas to other segments when an area is removed from the selected segment.
    // The only exception is when we draw inside one specific segment. In that case erasing adds to the mask segment. It is useful
    // for splitting a segment into two by painting.
    if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(modifierLabelmap,
                                                                       segmentationNode,
                                                                       this->GetSegmentEditorNode()->GetMaskSegmentID(),
                                                                       vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MASK,
                                                                       extent,
                                                                       false,
                                                                       segmentIDsToOverwrite))
    {
      if (this->IsVerbose)
      {
        vtkErrorMacro("" << __func__ << ": Failed to add back modifier labelmap to segment " + std::string(this->GetSegmentEditorNode()->GetMaskSegmentID()));
      }
    }
  }

  // Make sure the segmentation node is under the same parent as the source volume
  if (vtkMRMLScalarVolumeNode* sourceVolumeNode = this->SegmentEditorNode->GetSourceVolumeNode())
  {
    if (vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->SegmentEditorNode->GetScene()))
    {
      vtkIdType segmentationId = shNode->GetItemByDataNode(segmentationNode);
      vtkIdType sourceVolumeShId = shNode->GetItemByDataNode(sourceVolumeNode);
      if (segmentationId && sourceVolumeShId)
      {
        shNode->SetItemParent(segmentationId, shNode->GetItemParent(sourceVolumeShId));
      }
      else
      {
        if (this->IsVerbose)
        {
          vtkErrorMacro("" << __func__ << ": Subject hierarchy items not found for segmentation or source volume");
        }
      }
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::PauseRender()
{
  this->InvokeEvent(PauseRenderEvent);
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long vtkNotUsed(event), void* vtkNotUsed(callData))
{
  if (vtkMRMLSegmentEditorNode::SafeDownCast(caller))
  {
    this->UpdateSegmentationNodeObserver(this->GetSegmentationNode());
  }

  if (vtkMRMLSegmentationNode::SafeDownCast(caller))
  {
    if (this->SegmentationHistory)
    {
      this->SegmentationHistory->SetSegmentation(GetSegmentation());
    }
  }

  if (vtkSegmentationHistory::SafeDownCast(caller))
  {
    this->InvokeEvent(SegmentationHistoryChangedEvent);
  }
}

//-----------------------------------------------------------------------------
std::array<int, 2> vtkSlicerSegmentEditorLogic::RasToXy(double ras[3], vtkMRMLSliceNode* sliceNode)
{
  std::array<int, 2> xy{};

  if (!sliceNode)
  {
    vtkErrorWithObjectMacro(nullptr, "" << __func__ << ": Failed to get slice node!");
    return xy;
  }

  double rast[4] = { ras[0], ras[1], ras[2], 1.0 };
  double xyzw[4] = { 0.0, 0.0, 0.0, 1.0 };
  vtkSmartPointer<vtkMatrix4x4> rasToXyMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  rasToXyMatrix->DeepCopy(sliceNode->GetXYToRAS());
  rasToXyMatrix->Invert();
  rasToXyMatrix->MultiplyPoint(rast, xyzw);

  xy[0] = static_cast<int>(xyzw[0]);
  xy[1] = static_cast<int>(xyzw[1]);
  return xy;
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::ResumeRender()
{
  this->InvokeEvent(ResumeRenderEvent);
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::SelectSegmentAtOffset(int offset, bool visibleOnly) const
{
  std::string newId = this->GetNextSegmentID(offset, visibleOnly);
  if (IsSegmentIdValid(newId))
  {
    this->SetCurrentSegmentID(newId);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::XyzToRas(double inputXyz[3], double outputRas[3], vtkMRMLSliceNode* sliceNode)
{
  outputRas[0] = outputRas[1] = outputRas[2] = 0.0;

  if (!sliceNode)
  {
    vtkErrorWithObjectMacro(nullptr, "" << __func__ << ": Failed to get slice node!");
    return;
  }

  // x,y uses slice (canvas) coordinate system and actually has a 3rd z component (index into the
  // slice you're looking at), hence xyToRAS is really performing xyzToRAS. RAS is patient world
  // coordinate system. Note the 1 is because the transform uses homogeneous coordinates.
  double xyzw[4] = { inputXyz[0], inputXyz[1], inputXyz[2], 1.0 };
  double rast[4] = { 0.0, 0.0, 0.0, 1.0 };
  sliceNode->GetXYToRAS()->MultiplyPoint(xyzw, rast);
  outputRas[0] = rast[0];
  outputRas[1] = rast[1];
  outputRas[2] = rast[2];
}

//-----------------------------------------------------------------------------
std::array<double, 3> vtkSlicerSegmentEditorLogic::XyzToRas(double inputXyz[3], vtkMRMLSliceNode* sliceNode)
{
  std::array<double, 3> outputRas = { 0.0, 0.0, 0.0 };
  vtkSlicerSegmentEditorLogic::XyzToRas(inputXyz, outputRas.data(), sliceNode);
  return outputRas;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::XyToRas(int xy[2], double outputRas[3], vtkMRMLSliceNode* sliceNode)
{
  double xyz[3] = { static_cast<double>(xy[0]), static_cast<double>(xy[1]), 0.0 };
  vtkSlicerSegmentEditorLogic::XyzToRas(xyz, outputRas, sliceNode);
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::XyToRas(double xy[2], double outputRas[3], vtkMRMLSliceNode* sliceNode)
{
  double xyz[3] = { xy[0], xy[1], 0.0 };
  vtkSlicerSegmentEditorLogic::XyzToRas(xyz, outputRas, sliceNode);
}

//-----------------------------------------------------------------------------
std::array<double, 3> vtkSlicerSegmentEditorLogic::XyToRas(int xy[2], vtkMRMLSliceNode* sliceNode)
{
  std::array<double, 3> outputRas = { 0.0, 0.0, 0.0 };
  vtkSlicerSegmentEditorLogic::XyToRas(xy, outputRas.data(), sliceNode);
  return outputRas;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::XyzToIjk(double inputXyz[3],
                                           int outputIjk[3],
                                           vtkMRMLSliceNode* sliceNode,
                                           vtkOrientedImageData* image,
                                           vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  outputIjk[0] = outputIjk[1] = outputIjk[2] = 0;

  if (!sliceNode || !image)
  {
    return;
  }

  // Convert from XY to RAS first
  double ras[3] = { 0.0, 0.0, 0.0 };
  vtkSlicerSegmentEditorLogic::XyzToRas(inputXyz, ras, sliceNode);

  // Move point from world to same transform as image
  if (parentTransformNode)
  {
    if (parentTransformNode->IsTransformToWorldLinear())
    {
      vtkNew<vtkMatrix4x4> worldToParentTransform;
      parentTransformNode->GetMatrixTransformFromWorld(worldToParentTransform);
      double worldPos[4] = { ras[0], ras[1], ras[2], 1.0 };
      double parentPos[4] = { 0.0 };
      worldToParentTransform->MultiplyPoint(worldPos, parentPos);
      ras[0] = parentPos[0];
      ras[1] = parentPos[1];
      ras[2] = parentPos[2];
    }
    else
    {
      vtkErrorWithObjectMacro(nullptr, "" << __func__ << ": Parent transform is non-linear, which cannot be handled! Skipping.");
    }
  }

  // Convert RAS to image IJK
  double rast[4] = { ras[0], ras[1], ras[2], 1.0 };
  double ijkl[4] = { 0.0, 0.0, 0.0, 1.0 };
  vtkSmartPointer<vtkMatrix4x4> rasToIjkMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  image->GetImageToWorldMatrix(rasToIjkMatrix);
  rasToIjkMatrix->Invert();
  rasToIjkMatrix->MultiplyPoint(rast, ijkl);

  outputIjk[0] = static_cast<int>(ijkl[0] + 0.5);
  outputIjk[1] = static_cast<int>(ijkl[1] + 0.5);
  outputIjk[2] = static_cast<int>(ijkl[2] + 0.5);
}

//-----------------------------------------------------------------------------
std::array<int, 3> vtkSlicerSegmentEditorLogic::XyzToIjk(double inputXyz[3],
                                                         vtkMRMLSliceNode* sliceNode,
                                                         vtkOrientedImageData* image,
                                                         vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  std::array<int, 3> outputIjk = { 0, 0, 0 };
  vtkSlicerSegmentEditorLogic::XyzToIjk(inputXyz, outputIjk.data(), sliceNode, image, parentTransformNode);
  return outputIjk;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::XyToIjk(int xy[2],
                                          int outputIjk[3],
                                          vtkMRMLSliceNode* sliceNode,
                                          vtkOrientedImageData* image,
                                          vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  double xyz[3] = { static_cast<double>(xy[0]), static_cast<double>(xy[1]), 0.0 };
  vtkSlicerSegmentEditorLogic::XyzToIjk(xyz, outputIjk, sliceNode, image, parentTransformNode);
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::XyToIjk(double xy[2],
                                          int outputIjk[3],
                                          vtkMRMLSliceNode* sliceNode,
                                          vtkOrientedImageData* image,
                                          vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  double xyz[3] = { xy[0], xy[0], 0.0 };
  vtkSlicerSegmentEditorLogic::XyzToIjk(xyz, outputIjk, sliceNode, image, parentTransformNode);
}

//-----------------------------------------------------------------------------
std::array<int, 3> vtkSlicerSegmentEditorLogic::XyToIjk(int xy[2], vtkMRMLSliceNode* sliceNode, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  std::array<int, 3> outputIjk = { 0, 0, 0 };
  vtkSlicerSegmentEditorLogic::XyToIjk(xy, outputIjk.data(), sliceNode, image, parentTransformNode);
  return outputIjk;
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentEditorLogic::UpdateSegmentationNodeObserver(vtkMRMLSegmentationNode* segmentationNode)
{
  if (this->SegmentationNodeObs == segmentationNode)
  {
    return;
  }

  vtkSetAndObserveMRMLNodeMacro(this->SegmentationNodeObs, segmentationNode);
  this->ProcessMRMLNodesEvents(this->SegmentationNodeObs, vtkCommand::ModifiedEvent, nullptr);
}
