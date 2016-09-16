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

class vtkMRMLScalarVolumeNode;
class vtkMRMLSegmentationStorageNode;
class vtkMRMLLabelMapVolumeNode;
class vtkMRMLModelNode;

/// \ingroup SlicerRt_QtModules_Segmentations
class VTK_SLICER_SEGMENTATIONS_LOGIC_EXPORT vtkSlicerSegmentationsModuleLogic :
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerSegmentationsModuleLogic *New();
  vtkTypeMacro(vtkSlicerSegmentationsModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Get segmentation node containing a segmentation object. As segmentation objects are out-of-MRML
  /// VTK objects, there is no direct link from it to its parent node, so must be found from the MRML scene.
  /// \param scene MRML scene
  /// \param segmentation Segmentation to find
  /// \return Segmentation node containing the given segmentation if any, NULL otherwise
  static vtkMRMLSegmentationNode* GetSegmentationNodeForSegmentation(vtkMRMLScene* scene, vtkSegmentation* segmentation);

  /// Get segmentation node containing a given segment. As segments are out-of-MRML
  /// VTK objects, there is no direct link from it to its parent node, so must be found from the MRML scene.
  /// \param scene MRML scene
  /// \param segment Segment to find
  /// \param segmentId Output argument for the ID of the found segment
  /// \return Segmentation node containing the given segment if any, NULL otherwise
  static vtkMRMLSegmentationNode* GetSegmentationNodeForSegment(vtkMRMLScene* scene, vtkSegment* segment, std::string& segmentId);

  /// Load segmentation from file
  vtkMRMLSegmentationNode* LoadSegmentationFromFile(const char* filename);

  /// Create labelmap volume MRML node from oriented image data
  /// \param orientedImageData Oriented image data to create labelmap from
  /// \param labelmapVolumeNode Labelmap volume to be populated with the oriented image data. The volume node needs to exist
  ///   and be added to the MRML scene
  /// \return Success flag
  static bool CreateLabelmapVolumeFromOrientedImageData(vtkOrientedImageData* orientedImageData, vtkMRMLLabelMapVolumeNode* labelmapVolumeNode);

  /// Create oriented image data from a volume node
  /// \param outputParentTransformNode Specifies the parent transform node where the created image data can be placed.
  /// NOTE: Need to take ownership of the created object! For example using vtkSmartPointer<vtkOrientedImageData>::Take
  static vtkOrientedImageData* CreateOrientedImageDataFromVolumeNode(vtkMRMLScalarVolumeNode* volumeNode, vtkMRMLTransformNode* outputParentTransformNode = NULL);

  /// Utility function to determine if a labelmap contains a single label
  /// \return 0 if contains no label or multiple labels, the label if it contains a single one
  static int DoesLabelmapContainSingleLabel(vtkMRMLLabelMapVolumeNode* labelmapVolumeNode);

  /// Utility function that returns all non-empty label values in a labelmap
  static void GetAllLabelValues(vtkIntArray* labels, vtkImageData* labelmap);

  /// Create segment from labelmap volume MRML node. The contents are set as binary labelmap representation in the segment.
  /// Returns NULL if labelmap contains more than one label. In that case \sa ImportLabelmapToSegmentationNode needs to be used.
  /// NOTE: Need to take ownership of the created object! For example using vtkSmartPointer<vtkSegment>::Take
  /// \param labelmapVolumeNode Model node containing image data that will be the binary labelmap representation in the created segment
  /// \param segmentationNode Segmentation node that will be the container of the segment. It is used to get parent transform to
  ///   make sure the created segment will be located the same place the image was, considering all transforms involved. NULL value
  ///   means that this consideration is not needed. Default value is NULL.
  /// \return Created segment that then can be added to the segmentation if needed. Need to take ownership of the created
  ///   object! For example using vtkSmartPointer<vtkSegment>::Take
  static vtkSegment* CreateSegmentFromLabelmapVolumeNode(vtkMRMLLabelMapVolumeNode* labelmapVolumeNode, vtkMRMLSegmentationNode* segmentationNode=NULL);

  /// Create segment from model MRML node.
  /// The contents are set as closed surface model representation in the segment.
  /// NOTE: Need to take ownership of the created object! For example using vtkSmartPointer<vtkSegment>::Take
  /// \param modelNode Model node containing poly data that will be the closed surface representation in the created segment
  /// \param segmentationNode Segmentation node that will be the container of the segment. It is used to get parent transform to
  ///   make sure the created segment will be located the same place the model was, considering all transforms involved. NULL value
  ///   means that this consideration is not needed. Default value is NULL.
  /// \return Created segment that then can be added to the segmentation if needed. Need to take ownership of the created
  ///   object! For example using vtkSmartPointer<vtkSegment>::Take
  static vtkSegment* CreateSegmentFromModelNode(vtkMRMLModelNode* modelNode, vtkMRMLSegmentationNode* segmentationNode=NULL);

  /// Utility function for getting the segmentation node for a segment subject hierarchy node
  static vtkMRMLSegmentationNode* GetSegmentationNodeForSegmentSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* segmentShNode);

  /// Utility function for getting the segment object for a segment subject hierarchy node
  static vtkSegment* GetSegmentForSegmentSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* segmentShNode);

  /// Export segment to representation MRML node.
  /// 1. If representation node is a labelmap node, then the binary labelmap representation of the
  ///    segment is copied
  /// 2. If representation node is a model node, then the closed surface representation is copied
  /// Otherwise return with failure.
  static bool ExportSegmentToRepresentationNode(vtkSegment* segment, vtkMRMLNode* representationNode);

  /// Export multiple segments into a multi-label labelmap volume node
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param segmentIds List of segment IDs to export
  /// \param labelmapNode Labelmap node to export the segments to
  static bool ExportSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode, std::vector<std::string>& segmentIDs, vtkMRMLLabelMapVolumeNode* labelmapNode);

  /// Export all segments into a multi-label labelmap volume node
  /// \param segmentationNode Segmentation node from which the the segments are exported
  /// \param labelmapNode Labelmap node to export the segments to
  static bool ExportAllSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode, vtkMRMLLabelMapVolumeNode* labelmapNode);

  /// Import all labels from a labelmap node to a segmentation node, each label to a separate segment.
  /// The colors of the new segments are set from the color table corresponding to the labelmap volume.
  static bool ImportLabelmapToSegmentationNode(vtkMRMLLabelMapVolumeNode* labelmapNode, vtkMRMLSegmentationNode* segmentationNode);

  /// Import all labels from a labelmap image to a segmentation node, each label to a separate segment
  /// The colors of the new segments are randomly generated.
  /// \param baseSegmentName Prefix for the names of the new segments. Empty by default, in which case the prefix will be "Label"
  static bool ImportLabelmapToSegmentationNode(vtkOrientedImageData* labelmapImage, vtkMRMLSegmentationNode* segmentationNode, std::string baseSegmentName="");

  /// Import model into the segmentation as a segment.
  static bool ImportModelToSegmentationNode(vtkMRMLModelNode* modelNode, vtkMRMLSegmentationNode* segmentationNode);

  /// Create representation of only one segment in a segmentation.
  /// Useful if only one segment is processed, and we do not want to convert all segments to a certain
  /// segmentation to save time.
  /// NOTE: Need to take ownership of the created object! For example using vtkSmartPointer<vtkDataObject>::Take
  /// \return Representation of the specified segment if found or can be created, NULL otherwise
  static vtkDataObject* CreateRepresentationForOneSegment(vtkSegmentation* segmentation, std::string segmentID, std::string representationName);

  /// Apply the parent transform of a node to an oriented image data.
  /// Useful if we want to get a labelmap representation of a segmentation in the proper geometry for processing.
  /// \return Success flag
  static bool ApplyParentTransformToOrientedImageData(vtkMRMLTransformableNode* transformableNode, vtkOrientedImageData* orientedImageData);

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
  static bool GetTransformBetweenRepresentationAndSegmentation(vtkMRMLTransformableNode* representationNode, vtkMRMLSegmentationNode* segmentationNode, vtkGeneralTransform* representationToSegmentationTransform);

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
  /// Further useful functions can be found in segmentations logic (this class).
  /// \param segmentationNode Input segmentation node containing the segment to extract
  /// \param segmentID Segment identifier of the segment to extract
  /// \param imageData Output oriented image data into which the segment binary labelmap is copied
  /// \param applyParentTransform Flag determining whether to apply parent transform of the segmentation node.
  ///   If on, then the oriented image data is in RAS, otherwise in the segmentation node's coordinate frame. On by default
  /// \return Success flag
  static bool GetSegmentBinaryLabelmapRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, vtkOrientedImageData* imageData, bool applyParentTransform=true);

  /// Set a labelmap image as binary labelmap representation into the segment defined by the segmentation node and segment ID.
  /// Master representation must be binary labelmap! Master representation changed event is disabled to prevent deletion of all
  /// other representation in all segments. The other representations in the given segment are re-converted. The extent of the
  /// segment binary labelmap is shrunk to the effective extent. Display update is triggered.
  /// \param mergeMode Determines if the labelmap should replace the segment, or combined with a maximum or minimum operation.
  /// \param extent If extent is specified then only that extent of the labelmap is used.
  enum
    {
    MODE_REPLACE = 0,
    MODE_MERGE_MAX,
    MODE_MERGE_MIN
    };
  static bool SetBinaryLabelmapToSegment(vtkOrientedImageData* labelmap, vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, int mergeMode=MODE_REPLACE, const int extent[6]=0);

protected:
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();

  /// Callback function observing UID added events for subject hierarchy nodes.
  /// In case the newly added UID is a volume node referenced from a segmentation,
  /// its geometry will be set as image geometry conversion parameter.
  /// The "other order", i.e. when the volume is loaded first and the segmentation second,
  /// should be handled at loading time of the segmentation (because then we already know about the volume)
  static void OnSubjectHierarchyUIDAdded(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Handle MRML node added events
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);

  /// Handle MRML node removed events
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

protected:
  vtkSlicerSegmentationsModuleLogic();
  virtual ~vtkSlicerSegmentationsModuleLogic();

  /// Command handling subject hierarchy UID added events
  vtkCallbackCommand* SubjectHierarchyUIDCallbackCommand;

private:
  vtkSlicerSegmentationsModuleLogic(const vtkSlicerSegmentationsModuleLogic&); // Not implemented
  void operator=(const vtkSlicerSegmentationsModuleLogic&);               // Not implemented
};

#endif
