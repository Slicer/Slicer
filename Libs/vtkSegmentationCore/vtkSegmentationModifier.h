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

#ifndef __vtkSegmentationModifier_h
#define __vtkSegmentationModifier_h

// Segmentation includes
#include "vtkSegmentationCoreConfigure.h"

// VTK includes
#include "vtkObject.h"

// STD includes
#include <vector>

class vtkOrientedImageData;
class vtkSegmentation;

/// \ingroup SegmentationCore
/// \brief Utility functions for resampling oriented image data
class vtkSegmentationCore_EXPORT vtkSegmentationModifier : public vtkObject
{
public:
  static vtkSegmentationModifier* New();
  vtkTypeMacro(vtkSegmentationModifier, vtkObject);

public:
  /// Set a labelmap image as binary labelmap representation into the segment defined by the segmentation node and segment ID.
  /// Master representation must be binary labelmap! Master representation changed event is disabled to prevent deletion of all
  /// other representation in all segments. The other representations in the given segment are re-converted. The extent of the
  /// segment binary labelmap is shrunk to the effective extent. Display update is triggered.
  /// \param mergeMode Determines if the labelmap should replace the segment, combined with a maximum or minimum operation, or set under the mask.
  /// \param extent If extent is specified then only that extent of the labelmap is used.
  enum
  {
    MODE_REPLACE = 0,
    MODE_MERGE_MAX,
    MODE_MERGE_MIN,
    MODE_MERGE_MASK
  };
  static bool ModifyBinaryLabelmap(vtkOrientedImageData* labelmap, vtkSegmentation* segmentation, std::string segmentID,
    int mergeMode = MODE_REPLACE, const int extent[6] = nullptr, bool minimumOfAllSegments = false, bool masterRepresentationModifiedEnabled = false,
    const std::vector<std::string> segmentIdsToOverwrite = {}, std::vector<std::string>* modifiedSegmentIDs = nullptr);

  /// Get the list of segment IDs in the same shared labelmap that are contained within the mask
  /// \param segmentationNode Node containing the segmentation
  /// \param sharedSegmentID Segment ID of the segment that contains the shared labelmap to be checked
  /// \param mask Mask labelmap
  /// \param segmentIDs Output list of segment IDs under the mask
  /// \param includeInputSharedSegmentID If false, sharedSegmentID will not be added to the list of output segment IDs even if it is within the mask
  static bool GetSharedSegmentIDsInMask(vtkSegmentation* segmentation, std::string sharedSegmentID, vtkOrientedImageData* mask, const int extent[6],
    std::vector<std::string>& segmentIDs, int maskThreshold = 0.0, bool includeInputSharedSegmentID = false);

protected:
  static bool AppendLabelmapToSegment(vtkOrientedImageData* labelmap, vtkSegmentation* segmentation, std::string segmentID, int mergeMode, const int extent[6],
    bool minimumOfAllSegments, std::vector<std::string>* modifiedSegmentIDs, bool& segmentLabelmapModified);

  static void ShrinkSegmentToEffectiveExtent(vtkOrientedImageData* segmentLabelmap);

  static bool SharedLabelmapShouldOverlap(vtkSegmentation* segmentation, std::string segmentID, std::vector<std::string>& segmentIDsToOverwrite);

  static void SeparateModifiedSegmentFromSharedLabelmap(vtkOrientedImageData* labelmap, vtkSegmentation* segmentation, std::string segmentID,
    const int extent[6], const std::vector<std::string>& segmentIDsToOverwrite);

protected:
  vtkSegmentationModifier();
  ~vtkSegmentationModifier() override;

private:
  vtkSegmentationModifier(const vtkSegmentationModifier&) = delete;
  void operator=(const vtkSegmentationModifier&) = delete;
};

#endif
