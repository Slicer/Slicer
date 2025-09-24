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

#ifndef __vtkSlicerSegmentEditorLogic_h
#define __vtkSlicerSegmentEditorLogic_h

// Module export include
#include "vtkSlicerSegmentationsModuleLogicExport.h"

// MRML includes
class vtkMRMLAbstractViewNode;
class vtkMRMLApplicationLogic;
class vtkMRMLNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLScene;
class vtkMRMLSegmentationNode;
class vtkMRMLSegmentEditorNode;
class vtkMRMLSliceLogic;
class vtkMRMLSliceNode;
class vtkMRMLTransformNode;
class vtkMRMLVolumeNode;

// vtkSegmentationCore includes
class vtkOrientedImageData;
class vtkSegment;
class vtkSegmentation;
class vtkSegmentationHistory;

// VTK includes
#include <vtkCommand.h>
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
class vtkMatrix4x4;
class vtkPolyData;

// STD includes
#include <string>
#include <vector>

/// \brief Helper segment editor logic for qMRMLSegmentEditorWidget and its associated effects
///
/// Provides common logic to access and modify the segmentation.
class VTK_SLICER_SEGMENTATIONS_LOGIC_EXPORT vtkSlicerSegmentEditorLogic : public vtkObject
{
public:
  static vtkSlicerSegmentEditorLogic* New();
  vtkTypeMacro(vtkSlicerSegmentEditorLogic, vtkObject);

  enum ModificationMode
  {
    ModificationModeSet,
    ModificationModeAdd,
    ModificationModeRemove,
    ModificationModeRemoveAll
  };

  enum Events
  {
    SegmentationHistoryChangedEvent = vtkCommand::UserEvent + 1,
    PauseRenderEvent,
    ResumeRenderEvent,
  };

  /// Add empty segment in the current segmentation node
  /// \param segmentId ID of added segment. If empty then a default unique ID will be generated.
  /// \param segmentStatus Status of added segment from \sa vtkSlicerSegmentationsModuleLogic::SegmentStatus.
  ///   Default: NotStarted
  /// \return ID of the added segment if successful. Empty string otherwise.
  std::string AddEmptySegment(const std::string& segmentId = "", int segmentStatus = 0) const;

  /// Append image onto image. Resamples appended image and saves result in input image
  static void AppendImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* appendedImage);

  /// Rasterize a poly data onto the input image into the slice view
  static void AppendPolyMask(vtkOrientedImageData* input, vtkPolyData* polyData, vtkMRMLSliceNode* sliceNode, vtkMRMLSegmentationNode* segmentationNode = nullptr);

  /// \return true if volume, segmentation and reference geometry are defined
  bool CanAddSegments() const;

  /// \return true if segmentation history has at least one state to redo
  bool CanRedo() const;

  /// \return true if current selected segment ID is valid
  /// \sa IsSegmentIDValid
  bool CanRemoveSegments() const;

  /// \return true if the current segmentation is empty or is already set to binary label map
  bool CanTriviallyConvertSourceRepresentationToBinaryLabelMap() const;

  /// \return true if the segmentation history has at least one state
  bool CanUndo() const;

  /// \brief Clears the Undo/Redo history
  void ClearUndoState() const;

  /// \brief true if the current segmentation is valid and contains a closed surface representation
  bool ContainsClosedSurfaceRepresentation() const;

  /// \brief Creates a blank source volume matching the segmentation geometry.
  /// Usually called when no source volume is currently set.
  void CreateAndSetBlankSourceVolumeFromSegmentationGeometry() const;

  /// If no source volume is selected but a valid geometry is specified then create and store a blank source volume
  void CreateAndSetBlankSourceVolumeIfNeeded() const;

  /// Create a slice view screen space (2D) mask image for the given polydata
  static void CreateMaskImageFromPolyData(vtkPolyData* polyData, vtkOrientedImageData* outputMask, vtkMRMLSliceNode* sliceNode);

  /// \brief Export the current segmentation to a new color table node
  void ExportSegmentationToColorTableNode() const;

  /// \brief Current aligned source volume
  vtkOrientedImageData* GetAlignedSourceVolume() const;

  /// Get segment ID of selected segment
  /// \sa GetSelectedSegmentID
  std::string GetCurrentSegmentID() const;

  /// \brief Returns the current segmentation segment index. -1 if invalid.
  int GetCurrentSegmentIndex() const;

  /// \brief Returns the terminology entry value used as default when adding empty segments
  std::string GetDefaultTerminologyEntry() const;

  /// \brief Returns the current mask labelmap
  vtkOrientedImageData* GetMaskLabelmap() const;

  /// Get maximum number of saved undo/redo states.
  int GetMaximumNumberOfUndoStates() const;

  /// \brief Returns the current modifier label map
  vtkOrientedImageData* GetModifierLabelmap() const;

  /// \brief Returns the segment ID at given offset
  /// \param offset: Either a positive or negative offset value
  /// \param visibleOnly: if true, offset will represent the number of visible segments skipped.
  /// \return Segment ID or empty string if segmentation is invalid or offset out of range.
  std::string GetNextSegmentID(int offset, bool visibleOnly) const;

  /// \brief Returns the current reference geometry image
  vtkOrientedImageData* GetReferenceGeometryImage() const;

  /// \brief Returns the current reference geometry string
  std::string GetReferenceImageGeometryString() const;

  /// \brief Returns the reference geometry string matching the input segmentation
  static std::string GetReferenceImageGeometryStringFromSegmentation(vtkSegmentation* segmentation);

  /// \brief Current segmentation associated with the segmentation node
  /// \sa GetSegmentationNode
  vtkSegmentation* GetSegmentation() const;

  /// Return segmentation node's internal labelmap IJK to renderer world coordinate transform.
  /// If cannot be retrieved (segmentation is not defined, non-linearly transformed, etc.)
  /// then false is returned;
  bool GetSegmentationIJKToRAS(vtkMatrix4x4* ijkToRas) const;

  /// Get the segment editor parameter set node
  vtkMRMLSegmentEditorNode* GetSegmentEditorNode() const;

  /// Get currently selected segmentation MRML node
  vtkMRMLSegmentationNode* GetSegmentationNode() const;

  /// Get the current segments labelmap
  vtkOrientedImageData* GetSelectedSegmentLabelmap() const;

  /// Get the current volume node
  vtkMRMLScalarVolumeNode* GetSourceVolumeNode() const;

  /// Get the current segment. nullptr if invalid or no selection.
  vtkSegment* GetSelectedSegment() const;

  /// Return all the segment IDS present in the segmentation. Empty if invalid or no segment.
  std::vector<std::string> GetSegmentIDs() const;

  /// Return all segment currently visible in any view
  std::vector<std::string> GetVisibleSegmentIDs() const;

  /// Return the slice spacing matching the input slice node
  double GetSliceSpacing(vtkMRMLSliceNode* sliceNode) const;
  static double GetSliceSpacing(vtkMRMLSliceNode* sliceNode, vtkMRMLSliceLogic* sliceLogic);

  /// Return matrix for volume node that takes into account the IJKToRAS
  /// and any linear transforms that have been applied
  static void ImageToWorldMatrix(vtkMRMLVolumeNode* node, vtkMatrix4x4* ijkToRas);

  /// Return matrix for oriented image data that takes into account the image to world
  /// and any linear transforms that have been applied on the given segmentation
  static void ImageToWorldMatrix(vtkOrientedImageData* image, vtkMRMLSegmentationNode* node, vtkMatrix4x4* ijkToRas);

  /// Return true if the current segmentation is displayed in the input view node. False if invalid segmentation or view node.
  bool IsSegmentationDisplayableInView(vtkMRMLAbstractViewNode* viewNode) const;

  /// Return true if the current segmentation node is valid
  bool IsSegmentationNodeValid() const;

  /// Return true if the segment ID is present in the input list
  static bool IsSegmentIdInList(const std::string& segmentID, const std::vector<std::string>& visibleSegmentIDs);

  /// Return true when Segment ID is defined and present in the current segmentation
  bool IsSegmentIdValid(const std::string& segmentId) const;

  /// Return true if the segment ID is visible in any view. False if not visible or invalid.
  bool IsSegmentIdVisible(const std::string& segmentID) const;

  /// Return true if the current segment is visible in any view
  bool IsSelectedSegmentVisible() const;

  void ModifySegmentByLabelmap(vtkMRMLSegmentationNode* segmentationNode,
                               const char* segmentID,
                               vtkOrientedImageData* modifierLabelmap,
                               ModificationMode modificationMode,
                               bool isPerSegment,
                               bool bypassMasking);

  void ModifySegmentByLabelmap(vtkMRMLSegmentationNode* segmentationNode,
                               const char* segmentID,
                               vtkOrientedImageData* modifierLabelmap,
                               ModificationMode modificationMode,
                               const int modificationExtent[6],
                               bool isPerSegment,
                               bool bypassMasking);

  /// Trigger the PauseRenderEvent
  void PauseRender();

  /// Convert RAS position to XY in-slice position
  static std::array<int, 2> RasToXy(double ras[3], vtkMRMLSliceNode* sliceNode);

  /// Restores next saved state of the segmentation
  void Redo() const;

  /// Remove the current segment and return the following segment in the list if any valid.
  std::string RemoveSelectedSegment() const;

  /// Updates default modifier labelmap based on reference geometry (to set origin, spacing, and directions)
  /// and existing segments (to set extents). If reference geometry conversion parameter is empty
  /// then existing segments are used for determining origin, spacing, and directions and the resulting
  /// geometry is written to reference geometry conversion parameter.
  bool ResetModifierLabelmapToDefault() const;

  /// Trigger the ResumeRenderEvent
  void ResumeRender();

  /// Save current segmentation before performing an edit operation
  /// to allow reverting to the current state by using undo
  bool SaveStateForUndo() const;

  /// Selects the previous segment in the current segmentation
  void SelectPreviousSegment(bool visibleOnly) const;

  /// Selects the next segment in the current segmentation
  void SelectNextSegment(bool visibleOnly) const;

  /// Selects the first segment in the current segmentation
  void SelectFirstSegment(bool visibleOnly) const;

  /// Select the segment offset from the currently selected one
  /// Positive offset will move down the segmentation list
  /// Negative offset will move up the segmentation list
  void SelectSegmentAtOffset(int offset, bool visibleOnly) const;

  /// Set the current MRML application logic
  void SetApplicationLogic(vtkMRMLApplicationLogic* applicationLogic);

  /// Set the default terminology to use when adding new segments
  void SetDefaultTerminologyEntry(const std::string& entry);

  /// Set maximum number of saved undo/redo states.
  void SetMaximumNumberOfUndoStates(int) const;

  /// Set the MRML \a scene associated with the widget
  void SetScene(vtkMRMLScene* newScene);

  /// Set selected segment by its ID
  void SetCurrentSegmentID(const std::string& segmentID) const;

  /// Set the segment editor parameter set node
  void SetSegmentEditorNode(vtkMRMLSegmentEditorNode* newSegmentEditorNode);

  /// Set segmentation MRML node
  void SetSegmentationNode(vtkMRMLNode* node) const;

  /// Set segmentation MRML node by its ID
  void SetSegmentationNodeID(const std::string& nodeID) const;

  /// Set the segmentation history
  /// By default, the logic creates and uses an empty segmentation history at creation.
  void SetSegmentationHistory(const vtkSmartPointer<vtkSegmentationHistory>& segmentationHistory);

  /// Set source volume MRML node.
  /// If source volume has multiple scalar components
  /// then only the first scalar component is used.
  void SetSourceVolumeNode(vtkMRMLNode* node) const;

  /// Set source volume MRML node by its ID
  void SetSourceVolumeNodeID(const std::string& nodeID) const;

  /// Sets the source representation to binary labelmap.
  /// This method will convert any representation currently stored in the segmentation.
  /// User confirmation should be requested before this operation.
  ///
  /// \sa CanTriviallyConvertSourceRepresentationToBinaryLabelMap
  /// \sa TrivialSetSourceRepresentationToBinaryLabelmap
  bool SetSourceRepresentationToBinaryLabelMap() const;

  /// Create/remove closed surface model for the segmentation that is automatically updated when editing
  void ToggleSegmentationSurfaceRepresentation(bool isSurfaceRepresentationOn) const;

  /// Toggle the intensity mask currently set on the segmentation
  void ToggleSourceVolumeIntensityMask() const;

  /// Set source representation to binary label map.
  /// Should be called only when \sa CanTriviallyConvertSourceRepresentationToBinaryLabelMap is true.
  /// This method will not update any close surface representation or display if they already exist.
  ///
  /// \sa SetSourceRepresentationToBinaryLabelMap
  bool TrivialSetSourceRepresentationToBinaryLabelmap() const;

  /// Restores previous saved state of the segmentation
  void Undo() const;

  /// Update the input volume if the pointer matches one of :
  ///   \sa GetAlignedSourceVolume
  ///   \sa GetModifierLabelmap
  ///   \sa GetMaskLabelmap
  ///   \sa GetSelectedSegmentLabelmap
  ///   \sa GetReferenceGeometryImage
  void UpdateVolume(void* volumeToUpdate, bool& success);

  /// Updates selected segment labelmap in a geometry aligned with default modifierLabelmap.
  bool UpdateSelectedSegmentLabelmap() const;

  /// Updates a resampled source volume in a geometry aligned with default modifierLabelmap.
  bool UpdateAlignedSourceVolume();

  /// Updates mask labelmap.
  /// Geometry of mask will be the same as current modifierLabelmap.
  /// This mask only considers segment-based regions (and ignores masking based on
  /// source volume intensity).
  bool UpdateMaskLabelmap() const;

  /// Update the reference geometry image to the current segmentation
  bool UpdateReferenceGeometryImage() const;

  /// Convert XYZ slice view position to RAS position:
  /// x,y uses slice (canvas) coordinate system and actually has a 3rd z component (index into the
  /// slice you're looking at), hence xyToRAS is really performing xyzToRAS. RAS is patient world
  /// coordinate system. Note the 1 is because the transform uses homogeneous coordinates.
  static void XyzToRas(double inputXyz[3], double outputRas[3], vtkMRMLSliceNode* sliceNode);
  static std::array<double, 3> XyzToRas(double inputXyz[3], vtkMRMLSliceNode* sliceNode);

  /// Convert XY in-slice position to RAS position
  static void XyToRas(int xy[2], double outputRas[3], vtkMRMLSliceNode* sliceNode);
  /// Convert XY in-slice position to RAS position
  static void XyToRas(double xy[2], double outputRas[3], vtkMRMLSliceNode* sliceNode);
  /// Convert XY in-slice position to RAS position, python accessor method
  static std::array<double, 3> XyToRas(int xy[2], vtkMRMLSliceNode* sliceNode);
  /// Convert XYZ slice view position to image IJK position, \sa xyzToRas
  static void XyzToIjk(double inputXyz[3], int outputIjk[3], vtkMRMLSliceNode* sliceNode, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransform = nullptr);
  /// Convert XYZ slice view position to image IJK position, python accessor method, \sa xyzToRas
  static std::array<int, 3> XyzToIjk(double inputXyz[3], vtkMRMLSliceNode* sliceNode, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransform = nullptr);
  /// Convert XY in-slice position to image IJK position
  static void XyToIjk(int xy[2], int outputIjk[3], vtkMRMLSliceNode* sliceNode, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransform = nullptr);
  /// Convert XY in-slice position to image IJK position
  static void XyToIjk(double xy[2], int outputIjk[3], vtkMRMLSliceNode* sliceNode, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransform = nullptr);
  /// Convert XY in-slice position to image IJK position, python accessor method
  static std::array<int, 3> XyToIjk(int xy[2], vtkMRMLSliceNode* sliceNode, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransform = nullptr);

protected:
  vtkSlicerSegmentEditorLogic();
  ~vtkSlicerSegmentEditorLogic() override;

private:
  void ReconnectSegmentationNodeObserver();
  void SynchronizeSegmentationHistorySegmentation() const;

  /// Segment editor parameter set node containing all selections and working images
  vtkWeakPointer<vtkMRMLSegmentEditorNode> SegmentEditorNode;
  vtkSmartPointer<vtkSegmentationHistory> SegmentationHistory;

  /// These volumes are owned by this widget and a pointer is given to each effect
  /// so that they can access and modify it
  vtkSmartPointer<vtkOrientedImageData> AlignedSourceVolume;
  /// Modifier labelmap that is kept in memory to avoid memory reallocations on each editing operation.
  /// When update of this labelmap is requested its geometry is reset and its content is cleared.
  vtkSmartPointer<vtkOrientedImageData> ModifierLabelmap;
  vtkSmartPointer<vtkOrientedImageData> SelectedSegmentLabelmap;
  vtkSmartPointer<vtkOrientedImageData> MaskLabelmap;
  /// Image that contains reference geometry. Scalars are not allocated.
  vtkSmartPointer<vtkOrientedImageData> ReferenceGeometryImage;

  /// Input data that is used for computing AlignedSourceVolume.
  /// It is stored so that it can be determined that the source volume has to be updated
  vtkMRMLVolumeNode* AlignedSourceVolumeUpdateSourceVolumeNode;
  vtkMRMLTransformNode* AlignedSourceVolumeUpdateSourceVolumeNodeTransform;
  vtkMRMLTransformNode* AlignedSourceVolumeUpdateSegmentationNodeTransform;

  std::string DefaultTerminologyEntry;

  vtkMRMLScene* MRMLScene;

  unsigned long SegmentEditorNodeObs;
  unsigned long SegmentHistoryObs;
  std::tuple<unsigned long, vtkWeakPointer<vtkMRMLSegmentationNode>> SegmentationObs;
  vtkMRMLApplicationLogic* ApplicationLogic;
};

#endif
