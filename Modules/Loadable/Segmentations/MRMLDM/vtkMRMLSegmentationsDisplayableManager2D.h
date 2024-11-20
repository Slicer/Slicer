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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkMRMLSegmentationsDisplayableManager2D_h
#define __vtkMRMLSegmentationsDisplayableManager2D_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"

#include "vtkSlicerSegmentationsModuleMRMLDisplayableManagerExport.h"

class vtkMRMLSegmentationDisplayNode;
class vtkStringArray;
class vtkDoubleArray;

/// \brief Displayable manager for showing segmentations in slice (2D) views.
///
/// Displays segmentations in slice viewers as labelmaps or contour lines
///
class VTK_SLICER_SEGMENTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLSegmentationsDisplayableManager2D
  : public vtkMRMLAbstractSliceViewDisplayableManager
{

public:
  static vtkMRMLSegmentationsDisplayableManager2D* New();
  vtkTypeMacro(vtkMRMLSegmentationsDisplayableManager2D, vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Assemble and return info string to display in Data probe for a given viewer XYZ position.
  /// \return Invalid string by default, meaning no information to display.
  std::string GetDataProbeInfoStringForPosition(double xyz[3]) override;

  /// Get list of segments visible at selected display position.
  /// segmentValues is optional, if not nullptr then it returns value for each segment for fractional representations
  virtual void GetVisibleSegmentsForPosition(double ras[3], vtkMRMLSegmentationDisplayNode* displayNode,
    vtkStringArray* segmentIDs, vtkDoubleArray* segmentValues = nullptr);

  /// Specify that a segment is temporarily displayed with a custom renderer, so that this displayable manager should not display it.
  /// Only one custom renderer can be added for a specific segment for each segmentation display node.
  /// \return An integer tag that can be used for removing the custom renderer using RemoveCustomRenderer().
  ///   If the returned tag value is 0 it means that this segment for this display node has already a custom renderer
  ///   and no additional custom renderer is registered.
  int AddCustomSegmentRenderer(const std::string& segmentationDisplayNodeID, const std::string& segmentID);

  /// Remove custom renderer that was added using AddCustomSegmentRenderer()
  /// \return true if there was a custom renderer with the specified tag.
  bool RemoveCustomSegmentRenderer(int tag);

  // Check if a custom segment renderer has been registered for a segment by AddCustomSegmentRenderer()
  // \return The tag that can be used to remove the custom segment renderer. 0 is no custom renderer is set.
  int GetCustomSegmentRendererTag(const std::string& segmentationDisplayNodeID, const std::string& segmentID);

  // Check if a custom segment renderer has been registered for a segment by AddCustomSegmentRenderer()
  // \return True if a custom renderer is set.
  bool HasCustomSegmentRenderer(const std::string& segmentationDisplayNodeID, const std::string& segmentID);

  // @{
  /// Get information on custom segment renderers. Intended for troubleshooting.
  int GetNumberOfCustomSegmentsRenderers();
  int GetCustomSegmentRendererTag(int index);
  std::string GetCustomSegmentRendererSegmentationDisplayNodeID(int index);
  std::string GetCustomSegmentRendererSegmentID(int index);
  // @}

protected:
  void UnobserveMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Update Actors based on transforms in the scene
  void UpdateFromMRML() override;

  void OnMRMLSceneStartClose() override;
  void OnMRMLSceneEndClose() override;

  void OnMRMLSceneEndBatchProcess() override;

  /// Initialize the displayable manager based on its associated vtkMRMLSliceNode
  void Create() override;

protected:
  vtkMRMLSegmentationsDisplayableManager2D();
  ~vtkMRMLSegmentationsDisplayableManager2D() override;

private:
  vtkMRMLSegmentationsDisplayableManager2D(const vtkMRMLSegmentationsDisplayableManager2D&) = delete;
  void operator=(const vtkMRMLSegmentationsDisplayableManager2D&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
  friend class vtkInternal;
};

#endif
