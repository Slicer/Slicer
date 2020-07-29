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

// .NAME vtkSlicerSegmentationsModuleLogic - Logic class for segmentation handling
// .SECTION Description
// This class manages the logic associated with converting and handling
// segmentation node objects.

#ifndef __vtkSlicerSegmentationsModuleLogic_h
#define __vtkSlicerSegmentationsModuleLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerSegmentationsModuleLogicExport.h"

// Segmentations includes
#include "vtkMRMLSegmentationNode.h"

class vtkCallbackCommand;
class vtkOrientedImageData;
class vtkPolyData;
class vtkDataObject;
class vtkGeneralTransform;

class vtkMRMLSegmentationStorageNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLLabelMapVolumeNode;
class vtkMRMLVolumeNode;
class vtkMRMLModelNode;
class vtkSlicerTerminologiesModuleLogic;

/// \ingroup Slicer_QtModules_Segmentations
class VTK_SLICER_SEGMENTATIONS_LOGIC_EXPORT vtkSlicerSegmentationsModuleLogic :
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerSegmentationsModuleLogic *New();
  vtkTypeMacro(vtkSlicerSegmentationsModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Get segmentation node containing a segmentation object. As segmentation objects are out-of-MRML
  /// VTK objects, there is no direct link from it to its parent node, so must be found from the MRML scene.
  /// \param scene MRML scene
  /// \param segmentation Segmentation to find
  /// \return Segmentation node containing the given segmentation if any, nullptr otherwise
  static vtkMRMLSegmentationNode* GetSegmentationNodeForSegmentation(vtkMRMLScene* scene, vtkSegmentation* segmentation);

  /// Get segmentation node containing a given segment. As segments are out-of-MRML
  /// VTK objects, there is no direct link from it to its parent node, so must be found from the MRML scene.
  /// \param scene MRML scene
  /// \param segment Segment to find
  /// \param segmentId Output argument for the ID of the found segment
  /// \return Segmentation node containing the given segment if any, nullptr otherwise
  static vtkMRMLSegmentationNode* GetSegmentationNodeForSegment(vtkMRMLScene* scene, vtkSegment* segment, std::string& segmentId);

  /// Load segmentation from file
  /// \param filename Path and name of file containing segmentation (nrrd, vtm, etc.)
  /// \param autoOpacities Optional flag determining whether segment opacities are calculated automatically based on containment. True by default.
  /// \param nodeName Optional string to use for the segmentation node name.
  /// \param colorTableNode Optional color node used to name the segments and set segment color.
  /// \return Loaded segmentation node
  vtkMRMLSegmentationNode* LoadSegmentationFromFile(const char* filename, bool autoOpacities = true, const char* nodeName=nullptr,
    vtkMRMLColorTableNode* colorTableNode=nullptr);

  /// Create labelmap volume MRML node from oriented image data.
  /// Creates a display node if a display node does not exist. Shifts image extent to start from zero.
  /// Image is shallow-copied (voxel array is not duplicated).
  /// \param orientedImageData Oriented image data to create labelmap from
  /// \param labelmapVolumeNode Labelmap volume to be populated with the oriented image data. The volume node must exist and be added to the MRML scene.
  /// \return Success flag
  static bool CreateLabelmapVolumeFromOrientedImageData(vtkOrientedImageData* orientedImageData, vtkMRMLLabelMapVolumeNode* labelmapVolumeNode);

  /// Create volume MRML node from oriented image data. Display node is not created.
  /// \param orientedImageData Oriented image data to create volume node from
  /// \param scalarVolumeNode Volume to be populated with the oriented image data.
  /// \param shallowCopy If true then voxel array is not duplicated.
  /// \param shiftImageDataExtentToZeroStart: Adjust image origin to make image extents start from zero. May be necessary for compatibility with some algorithms
  ///        that assumes image extent start from 0.
  /// \return Success flag
  static bool CopyOrientedImageDataToVolumeNode(vtkOrientedImageData* orientedImageData,
    vtkMRMLVolumeNode* volumeNode, bool shallowCopy = true, bool shiftImageDataExtentToZeroStart = true);

  /// Create oriented image data from a volume node
  /// \param outputParentTransformNode Specifies the parent transform node where the created image data can be placed.
  /// NOTE: Need to take ownership of the created object! For example using vtkSmartPointer<vtkOrientedImageData>::Take
  static vtkOrientedImageData* CreateOrientedImageDataFromVolumeNode(vtkMRMLScalarVolumeNode* volumeNode, vtkMRMLTransformNode* outputParentTransformNode = nullptr);

  /// Utility function to determine if a labelmap contains a single label
  /// \return 0 if contains no label or multiple labels, the label if it contains a single one
  static int DoesLabelmapContainSingleLabel(vtkMRMLLabelMapVolumeNode* labelmapVolumeNode);

  /// Utility function that returns all non-empty label values in a labelmap
  static void GetAllLabelValues(vtkIntArray* labels, vtkImageData* labelmap);

  /// Create segment from labelmap volume MRML node. The contents are set as binary labelmap representation in the segment.
  /// Returns nullptr if labelmap contains more than one label. In that case \sa ImportLabelmapToSegmentationNode needs to be used.
  /// NOTE: Need to take ownership of the created object! For example using vtkSmartPointer<vtkSegment>::Take
  /// \param labelmapVolumeNode Model node containing image data that will be the binary labelmap representation in the created segment
  /// \param segmentationNode Segmentation node that will be the container of the segment. It is used to get parent transform to
  ///   make sure the created segment will be located the same place the image was, considering all transforms involved. nullptr value
  ///   means that this consideration is not needed. Default value is nullptr.
  /// \return Created segment that then can be added to the segmentation if needed. Need to take ownership of the created
  ///   object! For example using vtkSmartPointer<vtkSegment>::Take
  static vtkSegment* CreateSegmentFromLabelmapVolumeNode(vtkMRMLLabelMapVolumeNode* labelmapVolumeNode, vtkMRMLSegmentationNode* segmentationNode=nullptr);

  /// Create segment from model MRML node.
  /// The contents are set as closed surface model representation in the segment.
  /// NOTE: Need to take ownership of the created object! For example using vtkSmartPointer<vtkSegment>::Take
  /// \param modelNode Model node containing poly data that will be the closed surface representation in the created segment
  /// \param segmentationNode Segmentation node that will be the container of the segment. It is used to get parent transform to
  ///   make sure the created segment will be located the same place the model was, considering all transforms involved. nullptr value
  ///   means that this consideration is not needed. Default value is nullptr.
  /// \return Created segment that then can be added to the segmentation if needed. Need to take ownership of the created
  ///   object! For example using vtkSmartPointer<vtkSegment>::Take
  static vtkSegment* CreateSegmentFromModelNode(vtkMRMLModelNode* modelNode, vtkMRMLSegmentationNode* segmentationNode=nullptr);

  /// Utility function for getting the segmentation node for a segment subject hierarchy item
  static vtkMRMLSegmentationNode* GetSegmentationNodeForSegmentSubjectHierarchyItem(vtkIdType segmentShItemID, vtkMRMLScene* scene);

  /// Utility function for getting the segment object for a segment subject hierarchy item
  static vtkSegment* GetSegmentForSegmentSubjectHierarchyItem(vtkIdType segmentShItemID, vtkMRMLScene* scene);

  /// Export segment to representation MRML node.
  /// 1. If representation node is a labelmap node, then the binary labelmap representation of the
  ///    segment is copied
  /// 2. If representation node is a model node, then the closed surface representation is copied
  /// Otherwise return with failure.
  static bool ExportSegmentToRepresentationNode(vtkSegment* segment, vtkMRMLNode* representationNode);

  /// Export multiple segments into a folder, a model node from each segment
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param segmentIds List of segment IDs to export
  /// \param folderItemId Subject hierarchy folder item ID to export the segments to
  static bool ExportSegmentsToModels(vtkMRMLSegmentationNode* segmentationNode,
    std::vector<std::string>& segmentIDs, vtkIdType folderItemId);

  /// Export multiple segments into a folder, a model node from each segment
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param segmentIds List of segment IDs to export
  /// \param folderItemId Subject hierarchy folder item ID to export the segments to
  static bool ExportSegmentsToModels(vtkMRMLSegmentationNode* segmentationNode,
    vtkStringArray* segmentIds, vtkIdType folderItemId);

  /// Export visible segments into a folder, a model node from each segment
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param folderItemId Subject hierarchy folder item ID to export the segments to
  static bool ExportVisibleSegmentsToModels(vtkMRMLSegmentationNode* segmentationNode, vtkIdType folderItemId);

  /// Export all segments into a folder, a model node from each segment
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param folderItemId Subject hierarchy folder item ID to export the segments to
  static bool ExportAllSegmentsToModels(vtkMRMLSegmentationNode* segmentationNode, vtkIdType folderItemId);

  /// Export multiple segments into a multi-label labelmap volume node
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param segmentIds List of segment IDs to export
  /// \param labelmapNode Labelmap node to export the segments to
  /// \param referenceVolumeNode If specified, then the merged labelmap node will match the geometry of referenceVolumeNode
  /// \param extentComputationMode If referenceVolumeNode is not specified then labelmap extents will be determined based on this value.
  ///   By default, the minimum necessary size is used. Set value to vtkSegmentation::EXTENT_REFERENCE_GEOMETRY to use reference geometry extent.
  static bool ExportSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode, std::vector<std::string>& segmentIDs,
    vtkMRMLLabelMapVolumeNode* labelmapNode, vtkMRMLVolumeNode* referenceVolumeNode = nullptr,
    int extentComputationMode = vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, vtkMRMLColorTableNode* colorTableNode = nullptr);

  /// Export multiple segments into a multi-label labelmap volume node
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param segmentIds List of segment IDs to export
  /// \param labelmapNode Labelmap node to export the segments to
  /// \param referenceVolumeNode If specified, then the merged labelmap node will match the geometry of referenceVolumeNode
  /// \param extentComputationMode If referenceVolumeNode is not specified then labelmap extents will be determined based on this value.
  ///   By default, the minimum necessary size is used. Set value to vtkSegmentation::EXTENT_REFERENCE_GEOMETRY to use reference geometry extent.
  static bool ExportSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode, vtkStringArray* segmentIDs,
    vtkMRMLLabelMapVolumeNode* labelmapNode, vtkMRMLVolumeNode* referenceVolumeNode = nullptr,
    int extentComputationMode = vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, vtkMRMLColorTableNode* colorTableNode = nullptr);

  /// Export visible segments into a multi-label labelmap volume node
  /// \param segmentationNode Segmentation node from which the the visible segments are exported
  /// \param labelmapNode Labelmap node to export the segments to
  /// \param referenceVolumeNode If specified, then the merged labelmap node will match the geometry of referenceVolumeNode
  /// \param extentComputationMode If referenceVolumeNode is not specified then labelmap extents will be determined based on this value.
  ///   By default, the minimum necessary size is used. Set value to vtkSegmentation::EXTENT_REFERENCE_GEOMETRY to use reference geometry extent.
  static bool ExportVisibleSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode,
    vtkMRMLLabelMapVolumeNode* labelmapNode, vtkMRMLVolumeNode* referenceVolumeNode = nullptr,
    int extentComputationMode = vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS);

  /// Export all segments into a multi-label labelmap volume node
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param labelmapNode Labelmap node to export the segments to
  /// \param extentComputationMode Labelmap extents will be determined based on this value.
  ///   By default, the minimum necessary size is used. Set value to vtkSegmentation::EXTENT_REFERENCE_GEOMETRY to use reference geometry extent.
  static bool ExportAllSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode, vtkMRMLLabelMapVolumeNode* labelmapNode,
    int extentComputationMode = vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS);

  /// Import all labels from a labelmap node to a segmentation node, each label to a separate segment.
  /// The colors of the new segments are set from the color table corresponding to the labelmap volume.
  /// \param insertBeforeSegmentId New segments will be inserted before this segment.
  static bool ImportLabelmapToSegmentationNode(vtkMRMLLabelMapVolumeNode* labelmapNode,
    vtkMRMLSegmentationNode* segmentationNode, std::string insertBeforeSegmentId="");

  /// Import all labels from a labelmap image to a segmentation node, each label to a separate segment
  /// The colors of the new segments are randomly generated, unless terminology context is specified, in which case the terminology
  ///   entries are attempted to be mapped to the imported labels
  /// LabelmapImage is defined in the segmentation node's coordinate system
  /// (parent transform of the segmentation node is not used during import).
  /// \param baseSegmentName Prefix for the names of the new segments. Empty by default, in which case the prefix will be "Label"
  static bool ImportLabelmapToSegmentationNode(vtkOrientedImageData* labelmapImage,
    vtkMRMLSegmentationNode* segmentationNode, std::string baseSegmentName="", std::string insertBeforeSegmentId="") ;

  /// Update segmentation from segments in a labelmap node.
  /// \param updatedSegmentIDs Defines how label values 1..N are mapped to segment IDs (0..N-1).
  static bool ImportLabelmapToSegmentationNode(vtkMRMLLabelMapVolumeNode* labelmapNode,
    vtkMRMLSegmentationNode* segmentationNode, vtkStringArray* updatedSegmentIDs);

  /// Update segmentation from segments in a labelmap node.
  /// \param updatedSegmentIDs Defines how label values 1..N are mapped to segment IDs (0..N-1).
  static bool ImportLabelmapToSegmentationNode(vtkOrientedImageData* labelmapImage,
    vtkMRMLSegmentationNode* segmentationNode, vtkStringArray* updatedSegmentIDs,
    vtkGeneralTransform* labelmapToSegmentationTransform=nullptr );

  /// Import all labels from a labelmap node to a segmentation node, each label to a separate segment.
  /// Terminology and color is set to the segments based on the color table corresponding to the labelmap volume node.
  /// \param terminologyContextName Terminology context the entries of which are mapped to the labels imported from the labelmap node
  /// \param insertBeforeSegmentId New segments will be inserted before this segment.
  bool ImportLabelmapToSegmentationNodeWithTerminology(vtkMRMLLabelMapVolumeNode* labelmapNode,
    vtkMRMLSegmentationNode* segmentationNode, std::string terminologyContextName, std::string insertBeforeSegmentId="");

  /// Import model into the segmentation as a segment.
  static bool ImportModelToSegmentationNode(vtkMRMLModelNode* modelNode, vtkMRMLSegmentationNode* segmentationNode, std::string insertBeforeSegmentId = "");

  /// Import models in a folder into the segmentation as segments.
  static bool ImportModelsToSegmentationNode(
    vtkIdType folderItemId, vtkMRMLSegmentationNode* segmentationNode, std::string insertBeforeSegmentId = "" );

  /// Export closed surface representation of multiple segments to files. Typically used for writing 3D printable model files.
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param destinationFolder Folder name where segments will be exported to
  /// \param fileFormat Output file format (STL or OBJ).
  /// \param merge Merge all models into a single mesh. Only applicable to STL format.
  /// \param lps Save files in LPS coordinate system. If set to false then RAS coordinate system is used.
  /// \param segmentIds List of segment IDs to export
  static bool ExportSegmentsClosedSurfaceRepresentationToFiles(std::string destinationFolder,
    vtkMRMLSegmentationNode* segmentationNode, vtkStringArray* segmentIds = nullptr,
    std::string fileFormat = "STL", bool lps = true, double sizeScale = 1.0, bool merge = false);

  /// Gets the label values for the current segment from the color node reference.
  /// Label values found by matching color + segment name. If a segment name is not found in the table, then the label value returned for the segment
  /// is the first label value outside the table range.
  /// \param segmentationNode Segmentation node that has the export color node reference.
  /// \param colorTableNode Color table used to get the label values for the segments.
  /// \param segmentIds List of segment ids to get values for. The order of segmentIds dictates the order of the returned label values.
  /// \param labelValues Output label values from the color node. Lenght of the array will be the same as the number of segmentIds.
  static void GetLabelValuesFromColorNode(vtkMRMLSegmentationNode* segmentationNode, vtkMRMLColorTableNode* colorTableNode,
    vtkStringArray* segmentIds, vtkIntArray* labelValues);

  /// Export binary surface representation of multiple segments to a single output volume.
  /// \param destinationFolder Folder name where segments will be exported to
  /// \param segmentationNode Segmentation node that has the export color node reference.
  /// \param segmentIds List of segment ids to get values for. The order of segmentIds dictates the order of the returned label values.
  /// \param extension The file extension used for the output file. "nrrd" by default.
  /// \param useCompression If compression should be applied to the output file.
  /// \param colorTableNode Color table node used to set the exported labelmap values for the segments.
  static bool ExportSegmentsBinaryLabelmapRepresentationToFiles(std::string destinationFolder,
    vtkMRMLSegmentationNode* segmentationNode, vtkStringArray* segmentIds = nullptr, std::string extension = "nrrd", bool useCompression = false,
    vtkMRMLColorTableNode* colorTableNode = nullptr);

  /// Create representation of only one segment in a segmentation.
  /// Useful if only one segment is processed, and we do not want to convert all segments to a certain
  /// segmentation to save time.
  /// NOTE: Need to take ownership of the created object! For example using vtkSmartPointer<vtkDataObject>::Take
  /// \return Representation of the specified segment if found or can be created, nullptr otherwise
  static vtkDataObject* CreateRepresentationForOneSegment(vtkSegmentation* segmentation, std::string segmentID, std::string representationName);

  /// Apply the parent transform of a node to an oriented image data.
  /// Useful if we want to get a labelmap representation of a segmentation in the proper geometry for processing.
  /// \return Success flag
  static bool ApplyParentTransformToOrientedImageData(
    vtkMRMLTransformableNode* transformableNode, vtkOrientedImageData* orientedImageData, bool linearInterpolation=false, double backgroundColor[4]=nullptr );

  /// Apply the parent transform of a node to a poly data.
  /// Useful if we want to get a surface or contours representation of a segmentation in the proper geometry for processing.
  /// \return Success flag
  static bool ApplyParentTransformToPolyData(vtkMRMLTransformableNode* transformableNode, vtkPolyData* polyData);

  /// Get transform between a representation node (e.g. labelmap or model) and a segmentation node.
  /// Useful if we want to add a representation to a segment, and we want to make sure that the segment will be located the same place
  /// the representation node was. The output transform is the representation node's parent transform concatenated with the inverse
  /// of the segmentation's parent transform. It needs to be applied on the representation.
  /// \param representationNode Transformable node which contains the representation we want to add to the segment
  /// \param segmentationNode Segmentation node that will contain the segment to which the representation is added. It is the
  ///   representation node's parent transform concatenated with the inverse of the segmentation's parent transform.
  /// \param representationToSegmentationTransform General transform between the representation node and the segmentation node.
  /// \return Success flag
  static bool GetTransformBetweenRepresentationAndSegmentation(
    vtkMRMLTransformableNode* representationNode, vtkMRMLSegmentationNode* segmentationNode, vtkGeneralTransform* representationToSegmentationTransform );

  /// Convenience function to get a specified representation of a segment in a segmentation.
  /// A duplicate of the representation data object is copied into the argument output object, with the segmentation's parent transform
  /// applied if requested (on by default).
  /// \param segmentationNode Input segmentation node containing the segment to extract
  /// \param segmentID Segment identifier of the segment to extract
  /// \param representationName Name of the requested representation
  /// \param segmentRepresentation Output representation data object into which the given representation in the segment is copied
  /// \param applyParentTransform Flag determining whether to apply parent transform of the segmentation node. On by default
  /// \return Success flag
  static bool GetSegmentRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, std::string representationName, vtkDataObject* segmentRepresentation, bool applyParentTransform=true);

  /// Convenience function to get binary labelmap representation of a segment in a segmentation. Uses \sa GetSegmentRepresentation
  /// A duplicate of the oriented image data is copied into the argument image data, with the segmentation's parent transform
  /// applied if requested (on by default).
  /// The oriented image data can be used directly for processing, or to create a labelmap volume using \sa CreateLabelmapVolumeFromOrientedImageData.
  /// \param segmentationNode Input segmentation node containing the segment to extract
  /// \param segmentID Segment identifier of the segment to extract
  /// \param imageData Output oriented image data into which the segment binary labelmap is copied
  /// \param applyParentTransform Flag determining whether to apply parent transform of the segmentation node.
  ///   If on, then the oriented image data is in RAS, otherwise in the segmentation node's coordinate frame. On by default
  /// \return Success flag
  static bool GetSegmentBinaryLabelmapRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, vtkOrientedImageData* imageData, bool applyParentTransform=true);

  /// Convenience function to get closed surface representation of a segment in a segmentation. Uses \sa GetSegmentRepresentation
  /// A duplicate of the closed surface data is copied into the argument image data, with the segmentation's parent transform
  /// applied if requested (on by default).
  /// \param segmentationNode Input segmentation node containing the segment to extract
  /// \param segmentID Segment identifier of the segment to extract
  /// \param polyData Output polydata into which the segment polydata is copied
  /// \param applyParentTransform Flag determining whether to apply parent transform of the segmentation node.
  ///   If on, then the oriented image data is in RAS, otherwise in the segmentation node's coordinate frame. On by default
  /// \return Success flag
  static bool GetSegmentClosedSurfaceRepresentation(vtkMRMLSegmentationNode* segmentationNode,
    std::string segmentID, vtkPolyData* polyData, bool applyParentTransform = true);

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
  static bool SetBinaryLabelmapToSegment(vtkOrientedImageData* labelmap, vtkMRMLSegmentationNode* segmentationNode, std::string segmentID,
    int mergeMode=MODE_REPLACE, const int extent[6]=nullptr, bool minimumOfAllSegments=false, std::vector<std::string> segmentIdsToOverwrite={});

  /// Assign terminology to segments in a segmentation node based on the labels of a labelmap node. Match is made based on the
  /// 3dSlicerLabel terminology type attribute. If the terminology context does not contain that attribute, match cannot be made.
  /// \param terminologyContextName Terminology context the entries of which are mapped to the labels imported from the labelmap node
  bool SetTerminologyToSegmentationFromLabelmapNode(vtkMRMLSegmentationNode* segmentationNode,
    vtkMRMLLabelMapVolumeNode* labelmapNode, std::string terminologyContextName);

  /// Get default segmentation node. All new segmentation nodes are initialized to the content of this node.
  vtkMRMLSegmentationNode* GetDefaultSegmentationNode();

  /// Get/Set default closed surface smoothing enabled flag for new segmentation nodes.
  bool GetDefaultSurfaceSmoothingEnabled();
  void SetDefaultSurfaceSmoothingEnabled(bool enabled);

  enum SegmentStatus
  {
    NotStarted,
    InProgress,
    Completed,
    Flagged,
    LastStatus
  };
  /// Get the human readable segment status from the SegmentStatus enum value
  static const char* GetSegmentStatusAsHumanReadableString(int segmentStatus);
  /// Get the machine readable segment status from the SegmentStatus enum value
  static const char* GetSegmentStatusAsMachineReadableString(int segmentStatus);
  /// Get the enum segment status from a machine string
  static int GetSegmentStatusFromMachineReadableString(std::string statusString);

  /// Returns the name of the status tag
  static const char* GetStatusTagName();
  /// Returns the value of the status tag for the given segment
  static int GetSegmentStatus(vtkSegment* segment);
  /// Returns the value of the status tag for the given segment
  static void SetSegmentStatus(vtkSegment* segment, int status);
  /// Clear the contents of a single segment
  static bool ClearSegment(vtkMRMLSegmentationNode* segmentationNode, std::string segmentID);

  /// Get the list of segment IDs in the same shared labelmap that are contained within the mask
  /// \param segmentationNode Node containing the segmentation
  /// \param sharedSegmentID Segment ID of the segment that contains the shared labelmap to be checked
  /// \param mask Mask labelmap
  /// \param segmentIDs Output list of segment IDs under the mask
  /// \param includeInputSharedSegmentID If false, sharedSegmentID will not be added to the list of output segment IDs even if it is within the mask
  static bool GetSharedSegmentIDsInMask(vtkMRMLSegmentationNode* segmentationNode, std::string sharedSegmentID, vtkOrientedImageData* mask, const int extent[6],
    std::vector<std::string>& segmentIDs, int maskThreshold = 0.0, bool includeInputSharedSegmentID = false);

  /// Reconvert all representations in the segmentation from the master representation
  /// \param segmentationNode Node containing the segmentation
  /// \param sharedSegmentID Segment IDs to be converted. If empty, all segments will be converted.
  /// \return True if the representation was created, False otherwise
  static bool ReconvertAllRepresentations(vtkMRMLSegmentationNode* segmentationNode,
    const std::vector<std::string>& segmentIDs={});

  /// Collapse all segments into fewer shared labelmap layers
  /// \param segmentationNode Node containing the segmentation
  /// \param forceToSingleLayer If false, then the layers will not be overwritten by each other, if true then the layers can
  ///   overwrite each other, but the result is guaranteed to have one layer
  /// \return True if the representation was created, False otherwise
  static void CollapseBinaryLabelmaps(vtkMRMLSegmentationNode* segmentationNode, bool forceToSingleLayer);

public:
  /// Set Terminologies module logic
  void SetTerminologiesLogic(vtkSlicerTerminologiesModuleLogic* terminologiesLogic);

protected:
  void SetMRMLSceneInternal(vtkMRMLScene * newScene) override;

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;

  /// Callback function observing UID added events for subject hierarchy nodes.
  /// In case the newly added UID is a volume node referenced from a segmentation,
  /// its geometry will be set as image geometry conversion parameter.
  /// The "other order", i.e. when the volume is loaded first and the segmentation second,
  /// should be handled at loading time of the segmentation (because then we already know about the volume)
  static void OnSubjectHierarchyUIDAdded(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Handle MRML node added events
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;

  static bool ExportSegmentsClosedSurfaceRepresentationToStlFiles(std::string destinationFolder,
    vtkMRMLSegmentationNode* segmentationNode, std::vector<std::string>& segmentIDs, bool lps, double sizeScale, bool merge);
  static bool ExportSegmentsClosedSurfaceRepresentationToObjFile(std::string destinationFolder,
    vtkMRMLSegmentationNode* segmentationNode, std::vector<std::string>& segmentIDs, bool lps, double sizeScale);

  /// Generate a safe file name from a given string.
  /// The method is in this logic so that it does not cause confusion throughout Slicer
  /// (there is an implementation already in qSlicerSaveDataDialogPrivate::nodeFileInfo and it would be good to keep a central one)
  static std::string GetSafeFileName(std::string originalName);

protected:
  vtkSlicerSegmentationsModuleLogic();
  ~vtkSlicerSegmentationsModuleLogic() override;

  /// Command handling subject hierarchy UID added events
  vtkCallbackCommand* SubjectHierarchyUIDCallbackCommand;

  /// Terminologies module logic
  vtkSlicerTerminologiesModuleLogic* TerminologiesLogic{nullptr};

private:
  vtkSlicerSegmentationsModuleLogic(const vtkSlicerSegmentationsModuleLogic&) = delete;
  void operator=(const vtkSlicerSegmentationsModuleLogic&) = delete;
};

#endif
