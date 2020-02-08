/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

// SegmentationCore includes
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkSegmentation.h"
#include "vtkSegmentationConverter.h"
#include "vtkSegmentationModifier.h"

// VTK includes
#include <vtkImageConstantPad.h>
#include <vtkImageThreshold.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

// STD includes
#include <algorithm>

vtkStandardNewMacro(vtkSegmentationModifier);

//-----------------------------------------------------------------------------
vtkSegmentationModifier::vtkSegmentationModifier() = default;

//-----------------------------------------------------------------------------
vtkSegmentationModifier::~vtkSegmentationModifier() = default;

//-----------------------------------------------------------------------------
bool vtkSegmentationModifier::ModifyBinaryLabelmap(
  vtkOrientedImageData* labelmap, vtkSegmentation* segmentation, std::string segmentID, int mergeMode/*=MODE_REPLACE*/, const int extent[6]/*=0*/,
  bool minimumOfAllSegments/*=false*/, bool masterRepresentationModifiedEnabled/*=false*/, std::vector<std::string> segmentIDsToOverwrite/*={}*/,
  std::vector<std::string>* modifiedSegmentIDs/*=nullptr*/)
{
  if (!segmentation || segmentID.empty() || !labelmap)
    {
    vtkGenericWarningMacro("vtkSegmentationModifier::SetBinaryLabelmapToSegment: Invalid inputs");
    return false;
    }
  if (labelmap->GetPointData()->GetScalars() == nullptr)
    {
    vtkErrorWithObjectMacro(segmentation, "vtkSegmentationModifier::SetBinaryLabelmapToSegment: Invalid input labelmap");
    return false;
    }

  // If there are segments on the same layer that we should not overwrite, determine if there are any under the modifier labelmap
  if (vtkSegmentationModifier::SharedLabelmapShouldOverlap(segmentation, segmentID, segmentIDsToOverwrite))
    {
    vtkSegmentationModifier::SeparateModifiedSegmentFromSharedLabelmap(labelmap, segmentation, segmentID, extent, segmentIDsToOverwrite);
    }

  if (modifiedSegmentIDs)
    {
    modifiedSegmentIDs->clear();
    }

  // Get binary labelmap representation of selected segment
  vtkSegment* selectedSegment = segmentation->GetSegment(segmentID);
  if (!selectedSegment)
    {
    vtkGenericWarningMacro("vtkSegmentationModifier::SetBinaryLabelmapToSegment: Invalid selected segment");
    return false;
    }

  vtkOrientedImageData* segmentLabelmap = vtkOrientedImageData::SafeDownCast(
    selectedSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) );
  if (!segmentLabelmap)
    {
    vtkErrorWithObjectMacro(segmentation, "vtkSegmentationModifier::SetBinaryLabelmapToSegment: Failed to get binary labelmap representation in "
      << "segmentation");
    return false;
    }

  bool wasMasterRepresentationModifiedEnabled = segmentation->SetMasterRepresentationModifiedEnabled(masterRepresentationModifiedEnabled);

  bool segmentLabelmapModified = true;
  if (!vtkSegmentationModifier::AppendLabelmapToSegment(labelmap, segmentation, segmentID, mergeMode, extent, minimumOfAllSegments, modifiedSegmentIDs,
    segmentLabelmapModified))
    {
    segmentation->SetMasterRepresentationModifiedEnabled(wasMasterRepresentationModifiedEnabled);
    return false;
    }

  // Shrink the image data extent to only contain the effective data (extent of non-zero voxels)
  vtkSegmentationModifier::ShrinkSegmentToEffectiveExtent(segmentLabelmap);

  // Re-enable master representation modified event
  segmentation->SetMasterRepresentationModifiedEnabled(wasMasterRepresentationModifiedEnabled);
  if (segmentLabelmapModified)
    {
    const char* segmentIdChar = segmentID.c_str();
    segmentation->InvokeEvent(vtkSegmentation::MasterRepresentationModified, (void*)segmentIdChar);
    segmentation->InvokeEvent(vtkSegmentation::RepresentationModified, (void*)segmentIdChar);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSegmentationModifier::GetSharedSegmentIDsInMask(
  vtkSegmentation* segmentation, std::string sharedSegmentID, vtkOrientedImageData* maskLabelmap, const int extent[6],
  std::vector<std::string>& segmentIDs, int maskThreshold/*=0*/, bool includeInputSegmentID/*=false*/)
{
  segmentIDs.clear();
  if (!segmentation)
    {
    vtkErrorWithObjectMacro(nullptr, "Invalid segmentation!");
    return false;
    }

  std::vector<std::string> sharedSegmentIDs;
  segmentation->GetSegmentIDsSharingBinaryLabelmapRepresentation(sharedSegmentID, sharedSegmentIDs, includeInputSegmentID);
  if (sharedSegmentIDs.empty())
    {
    // No shared segments to compare against, so there are no relevant IDs in the mask
    return true;
    }

  std::map<int, std::string> segmentValues;
  for (std::string currentSegmentID : sharedSegmentIDs)
    {
    vtkSegment* segment = segmentation->GetSegment(currentSegmentID);
    segmentValues[segment->GetLabelValue()] = currentSegmentID;
    }

  vtkOrientedImageData* binaryLabelmap = vtkOrientedImageData::SafeDownCast(
    segmentation->GetSegment(sharedSegmentID)->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));

  std::vector<int> labelValuesInMask;
  vtkOrientedImageDataResample::GetLabelValuesInMask(labelValuesInMask, binaryLabelmap, maskLabelmap, extent, maskThreshold);

  for (int labelValue : labelValuesInMask)
    {
    if (labelValue == 0 || segmentValues.find(labelValue) == segmentValues.end())
      {
      continue;
      }
    segmentIDs.push_back(segmentValues[labelValue]);
    }
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSegmentationModifier::SharedLabelmapShouldOverlap(vtkSegmentation* segmentation, std::string segmentID, std::vector<std::string>& segmentIDsToOverwrite)
{
  std::vector<std::string> sharedSegmentIDs;
  segmentation->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentID, sharedSegmentIDs, false);

  // Determine if there are any segments on the same layer that we should not overwrite
  bool segmentsOnLayerShouldOverlap = false;
  for (std::string sharedID : sharedSegmentIDs)
    {
    if (std::find(segmentIDsToOverwrite.begin(), segmentIDsToOverwrite.end(), sharedID) == segmentIDsToOverwrite.end())
      {
      segmentsOnLayerShouldOverlap = true;
      break;
      }
    }
  return segmentsOnLayerShouldOverlap;
}

//-----------------------------------------------------------------------------
void vtkSegmentationModifier::SeparateModifiedSegmentFromSharedLabelmap(vtkOrientedImageData* labelmap, vtkSegmentation* segmentation, std::string segmentID,
  const int extent[6], const std::vector<std::string>& segmentIDsToOverwrite)
{
  std::vector<std::string> sharedSegmentsUnderModifier;
  vtkSegmentationModifier::GetSharedSegmentIDsInMask(segmentation, segmentID, labelmap,
    extent, sharedSegmentsUnderModifier, 0.0, false);

  for (std::string sharedSegmentID : sharedSegmentsUnderModifier)
    {
    std::vector<std::string>::const_iterator foundOverwriteIDIt = std::find(segmentIDsToOverwrite.begin(), segmentIDsToOverwrite.end(), sharedSegmentID);
    if (foundOverwriteIDIt == segmentIDsToOverwrite.end())
      {
      // TODO: Implement more robust handling of segment separation and layers.
      //       ex. Rather than a completely new layer, we could move the segment to the next availiable layer

      // We would overwrite a segment that should not be overwritten. Separate the modifier segment to a new layer
      segmentation->SeparateSegmentLabelmap(segmentID);
      break;
      }
    }
}

//-----------------------------------------------------------------------------
bool vtkSegmentationModifier::AppendLabelmapToSegment(vtkOrientedImageData* labelmap, vtkSegmentation* segmentation, std::string segmentID, int mergeMode,
  const int extent[6], bool minimumOfAllSegments, std::vector<std::string>* modifiedSegmentIDs, bool& segmentLabelmapModified)
{
  // Get binary labelmap representation of selected segment
  vtkSegment* selectedSegment = segmentation->GetSegment(segmentID);
  if (!selectedSegment)
    {
    vtkGenericWarningMacro("vtkSegmentationModifier::SetBinaryLabelmapToSegment: Invalid selected segment");
    return false;
    }

  vtkOrientedImageData* segmentLabelmap = vtkOrientedImageData::SafeDownCast(
    selectedSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));
  if (!segmentLabelmap)
    {
    vtkErrorWithObjectMacro(segmentation, "vtkSegmentationModifier::SetBinaryLabelmapToSegment: Failed to get binary labelmap representation in "
      << "segmentation");
    return false;
    }

  int* segmentLabelmapExtent = segmentLabelmap->GetExtent();
  bool segmentLabelmapEmpty = (segmentLabelmapExtent[0] > segmentLabelmapExtent[1] ||
    segmentLabelmapExtent[2] > segmentLabelmapExtent[3] ||
    segmentLabelmapExtent[4] > segmentLabelmapExtent[5]);
  if (segmentLabelmapEmpty)
    {
    if (mergeMode == MODE_MERGE_MIN)
      {
      // empty image is assumed to have minimum value everywhere, combining it with MAX operation
      // results an empty image, so we don't need to do anything.
      return true;
      }
    // Replace the empty image with the modifier image
    mergeMode = MODE_REPLACE;
    }

  int labelValue = selectedSegment->GetLabelValue();
  // Ensure that the value for the segment can be contained in the labelmap.
  vtkOrientedImageDataResample::CastImageForValue(segmentLabelmap, labelValue);

  if (mergeMode == MODE_REPLACE)
    {
    std::vector<std::string> sharedSegmentIDs;
    segmentation->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentID, sharedSegmentIDs, false);
    if (sharedSegmentIDs.size() != 0)
      {
      // There are other labelmaps that share the same representation.
      // Clear the representation and use mask mode.
      mergeMode = MODE_MERGE_MASK;
      segmentation->ClearSegment(segmentID);
      }
    else
      {
      vtkSmartPointer<vtkOrientedImageData> modifierLabelmap = labelmap;
      if (labelValue != 1)
        {
        // If the label value is not the same as the modifier (which should be 1), change the label value of the modifier labelmap to be
        // the same as the segment label value.
        vtkNew<vtkImageThreshold> threshold;
        threshold->SetInputData(labelmap);
        threshold->ThresholdByLower(0);
        threshold->SetInValue(0);
        threshold->SetOutValue(labelValue);
        threshold->Update();
        modifierLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
        modifierLabelmap->ShallowCopy(threshold->GetOutput());
        modifierLabelmap->CopyDirections(labelmap);
        }

      if (modifiedSegmentIDs)
        {
        modifiedSegmentIDs->push_back(segmentID);
        }

      if (!vtkOrientedImageDataResample::CopyImage(modifierLabelmap, segmentLabelmap, extent))
        {
        vtkErrorWithObjectMacro(segmentation, "vtkSegmentationModifier::AppendLabelmapToSegment: Failed to copy labelmap");
        return false;
        }
      }
    }

  if (mergeMode != MODE_REPLACE)
    {
    int operation = vtkOrientedImageDataResample::OPERATION_MINIMUM;
    switch (mergeMode)
      {
      case MODE_MERGE_MAX:
        operation = vtkOrientedImageDataResample::OPERATION_MAXIMUM;
        break;
      case MODE_MERGE_MASK:
        operation = vtkOrientedImageDataResample::OPERATION_MASKING;
        break;
      default:
        operation = vtkOrientedImageDataResample::OPERATION_MINIMUM;
      }

    vtkSmartPointer<vtkOrientedImageData> modifierLabelmap = labelmap;
    if (operation == vtkOrientedImageDataResample::OPERATION_MINIMUM)
      {
      vtkNew<vtkImageThreshold> threshold;
      threshold->SetInputData(labelmap);
      threshold->ThresholdByLower(0);
      threshold->SetInValue(0);
      threshold->SetOutValue(segmentLabelmap->GetScalarTypeMax());
      threshold->SetOutputScalarType(segmentLabelmap->GetScalarType());
      threshold->Update();
      modifierLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
      modifierLabelmap->ShallowCopy(threshold->GetOutput());
      modifierLabelmap->CopyDirections(labelmap);
      }
    else
      {
      if (modifiedSegmentIDs)
        {
        vtkSegmentationModifier::GetSharedSegmentIDsInMask(segmentation, segmentID, labelmap, extent, *modifiedSegmentIDs);
        }
      }
    if (modifiedSegmentIDs)
      {
      modifiedSegmentIDs->push_back(segmentID);
      }

    vtkSmartPointer<vtkOrientedImageData> resampledSegmentLabelmap;
    if (!vtkOrientedImageDataResample::DoGeometriesMatch(segmentLabelmap, modifierLabelmap))
      {
      // Make sure appended image has the same lattice as the input image
      resampledSegmentLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
      vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(
        segmentLabelmap, modifierLabelmap, resampledSegmentLabelmap, false /*interpolate*/, true /*pad*/);
      }
    else
      {
      resampledSegmentLabelmap = segmentLabelmap;
      }

    if (operation == vtkOrientedImageDataResample::OPERATION_MINIMUM && !minimumOfAllSegments)
      {
      vtkNew<vtkOrientedImageData> segmentMask;
      vtkNew<vtkImageThreshold> thresholdSegment;
      thresholdSegment->SetInputData(resampledSegmentLabelmap);
      thresholdSegment->ThresholdBetween(labelValue, labelValue);
      thresholdSegment->SetInValue(1);
      thresholdSegment->SetOutValue(0);
      thresholdSegment->SetOutputScalarTypeToUnsignedChar();
      thresholdSegment->Update();
      segmentMask->ShallowCopy(thresholdSegment->GetOutput());
      segmentMask->CopyDirections(resampledSegmentLabelmap);
      vtkOrientedImageDataResample::ApplyImageMask(modifierLabelmap, segmentMask, modifierLabelmap->GetScalarTypeMax());
      }

    if (!vtkOrientedImageDataResample::MergeImage(
      resampledSegmentLabelmap, modifierLabelmap, segmentLabelmap, operation, extent, 0, labelValue, &segmentLabelmapModified))
      {
      vtkErrorWithObjectMacro(segmentation, "vtkSegmentationModifier::SetBinaryLabelmapToSegment: Failed to merge labelmap (max)");
      return false;
      }
    }
    return true;
}

//-----------------------------------------------------------------------------
void vtkSegmentationModifier::ShrinkSegmentToEffectiveExtent(vtkOrientedImageData* segmentLabelmap)
{
  int effectiveExtent[6] = {0,-1,0,-1,0,-1};
  vtkOrientedImageDataResample::CalculateEffectiveExtent(segmentLabelmap, effectiveExtent); // TODO: use the update extent? maybe crop when changing segment?
  if (effectiveExtent[0] > effectiveExtent[1] || effectiveExtent[2] > effectiveExtent[3] || effectiveExtent[4] > effectiveExtent[5])
    {
    vtkDebugWithObjectMacro(segmentLabelmap,
      "vtkSegmentationModifier::SetBinaryLabelmapToSegment: effective extent of the labelmap to set is invalid (labelmap is empty)");
    }
  else
    {
    bool isPaddingRequired = false;
    int segmentExtent[6] = { 0 };
    segmentLabelmap->GetExtent(segmentExtent);
    for (int i = 0; i < 3; ++i)
      {
      if (effectiveExtent[2 * i] != segmentExtent[2 * i] || effectiveExtent[2 * i + 1] != segmentExtent[2 * i + 1])
        {
        isPaddingRequired = true;
        break;
        }
      }
    if (isPaddingRequired)
      {
      vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
      padder->SetInputData(segmentLabelmap);
      padder->SetOutputWholeExtent(effectiveExtent);
      padder->Update();
      segmentLabelmap->ShallowCopy(padder->GetOutput());
      }
    }
}
