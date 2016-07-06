/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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

#ifndef __vtkMRMLSegmentationNode_h
#define __vtkMRMLSegmentationNode_h

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLColorTableNode.h>

// STD includes
#include <cstdlib>

// vtkSegmentationCore includes
#include "vtkSegmentation.h"

class vtkCallbackCommand;
class vtkMRMLScene;
class vtkMRMLSubjectHierarchyNode;

/// \brief MRML node containing segmentations
/// \ingroup Segmentations
///
/// Segmentation node is a subclass of the LabelMapVolume node, and contains a segmentation object
/// \sa vtkSegmentation that manages a list of segmented structures (segments). Each segment can
/// contain multiple data representations for the same structure, which are automatically converted
/// on request using the cheapest conversion path.
///
/// The volume contained by the labelmap is the "merged labelmap" of the whole segmentation, which
/// is generated on request when the segmentation is displayed as a labelmap volume in the slice
/// views. Although the scalar type of the binary labelmap representations in the segments that are
/// used for the merged labelmap is unsigned char, the scalar type of the merged labelmap is short.
/// When merging, the finest resolution found in the segment binary labelmaps is used, and the segments
/// are "painted" on this merge image one by one, in the order of the segments contained. This may
/// result in missing information if segments overlap. Merged labelmap is a compatibility feature that
/// may be removed in the future due to the more advanced 2D displayable manager that can handle
/// transparency, overlapping, multiple representation types, and display multiple segmentations at the
/// same time.
///
class VTK_MRML_EXPORT vtkMRMLSegmentationNode : public vtkMRMLLabelMapVolumeNode
{
public:
  // Define constants
  static const char* GetSegmentIDAttributeName() { return "segmentID"; };

  static vtkMRMLSegmentationNode *New();
  vtkTypeMacro(vtkMRMLSegmentationNode, vtkMRMLLabelMapVolumeNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Create instance of a GAD node.
  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Copy the entire contents of the node into this node
  virtual void DeepCopy(vtkMRMLNode* node);

  /// Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Segmentation";};

  /// Get bounding box in global RAS in the form (xmin,xmax, ymin,ymax, zmin,zmax).
  virtual void GetRASBounds(double bounds[6]);

  /// Returns true if the transformable node can apply non linear transforms
  /// \sa ApplyTransform
  virtual bool CanApplyNonLinearTransforms()const;

  /// Apply a transform matrix on the segmentation
  /// \sa SetAndObserveTransformNodeID, ApplyTransform, CanApplyNonLinearTransforms
  virtual void ApplyTransformMatrix(vtkMatrix4x4* transformMatrix);

  /// Apply a transform on the segmentation
  /// \sa SetAndObserveTransformNodeID, CanApplyNonLinearTransforms
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  /// Create a segmentation storage node
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Create and observe a segmentation display node
  virtual void CreateDefaultDisplayNodes();

  /// Reimplemented to take into account the modified time of the internal data.
  /// Returns true if the node (default behavior) or the internal data are modified
  /// since read/written.
  /// Note: The MTime of the internal data is used to know if it has been modified.
  /// So if you invoke one of the data modified events without calling Modified() on the
  /// internal data, GetModifiedSinceRead() won't return true.
  /// \sa vtkMRMLStorableNode::GetModifiedSinceRead()
  virtual bool GetModifiedSinceRead();

  /// Function called from segmentation logic when UID is added in a subject hierarchy node.
  /// In case the newly added UID is a volume node referenced from this segmentation,
  /// its geometry will be set as image geometry conversion parameter.
  /// The "other order", i.e. when the volume is loaded first and the segmentation second,
  /// should be handled at loading time of the segmentation (because then we already know about the volume)
  /// \param shNodeWithNewUID Subject hierarchy node that just got a new UID
  void OnSubjectHierarchyUIDAdded(vtkMRMLSubjectHierarchyNode* shNodeWithNewUID);

  /// Get subject hierarchy node belonging to a certain segment
  vtkMRMLSubjectHierarchyNode* GetSegmentSubjectHierarchyNode(std::string segmentID);

//BTX
  /// Build merged labelmap of the binary labelmap representations of the specified segments
  /// \param mergedImageData Output image data for the merged labelmap image data
  /// \param extentComputationMode Determines how to compute extents (EXTENT_REFERENCE_GEOMETRY, EXTENT_UNION_OF_SEGMENTS, or EXTENT_UNION_OF_EFFECTIVE_SEGMENTS).
  /// \param mergedLabelmapGeometry Determines geometry of merged labelmap if not NULL, automatically determined otherwise
  /// \param segmentIDs List of IDs of segments to include in the merged labelmap. If empty or missing, then all segments are included
  /// \return Success flag
  virtual bool GenerateMergedLabelmap(vtkOrientedImageData* mergedImageData, int extentComputationMode, vtkOrientedImageData* mergedLabelmapGeometry = NULL, const std::vector<std::string>& segmentIDs = std::vector<std::string>());
//ETX

  /// Python-accessible version of the more generic \sa GenerateMergedLabelmap.
  /// The last argument specifying the list of segments to be included is omitted, which means that
  /// all the segments will be merged.
  /// \sa GenerateMergedLabelmap
  bool GenerateMergedLabelmapForAllSegments(vtkOrientedImageData* mergedImageData, int extentComputationMode, vtkOrientedImageData* mergedLabelmapGeometry);

  /// Re-generate displayed merged labelmap
  void ReGenerateDisplayedMergedLabelmap();

  /// Make sure image data of a volume node has extents that start at zero.
  /// This needs to be done for compatibility reasons, as many components assume the extent has a form of
  /// (0,dim[0],0,dim[1],0,dim[2]), which is not the case many times for segmentation merged labelmaps.
  static void ShiftVolumeNodeExtentToZeroStart(vtkMRMLScalarVolumeNode* volumeNode);

  /// Expose reference identifier to get the volume node defining the reference image geometry if any
  static std::string GetReferenceImageGeometryReferenceRole() { return "referenceImageGeometryRef"; };
  /// Set reference image geometry conversion parameter from the volume node, keeping reference
  void SetReferenceImageGeometryParameterFromVolumeNode(vtkMRMLScalarVolumeNode* volumeNode);

public:
  /// Get segmentation object
  vtkGetObjectMacro(Segmentation, vtkSegmentation);
  /// Set and observe segmentation object
  void SetAndObserveSegmentation(vtkSegmentation* segmentation);

  /// Get reference terminology color table node.
  /// This node contains the terminology from which the user can assign terminology to new segments
  vtkMRMLColorTableNode* GetReferenceTerminologyColorNode();
  /// Set and observe reference terminology color table node.
  /// This node contains the terminology from which the user can assign terminology to new segments
  void SetAndObserveReferenceTerminologyColorNode(vtkMRMLColorTableNode* node);

  /// Generate merged labelmap image data for display if needed
  virtual vtkImageData* GetImageData();
  /// Query existence or merged labelmap. Do not trigger merged labelmap generation
  bool HasMergedLabelmap();

  /// Set default reference terminology color table node when node is added to a scene
  virtual void SetSceneReferences();

protected:
  /// Add display properties for segment with given ID
  virtual bool AddSegmentDisplayProperties(std::string segmentId);

  /// Reset all display related data
  virtual void ResetSegmentDisplayProperties();

protected:
  /// Set segmentation object
  vtkSetObjectMacro(Segmentation, vtkSegmentation);

  /// Callback function observing the master representation of the segmentation (and each segment within)
  /// Invalidates all representations other than the master. These representations will be automatically converted later on demand.
  static void OnMasterRepresentationModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Callback function observing segment added events.
  /// Triggers update of display properties
  static void OnSegmentAdded(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Callback function observing segment removed events.
  /// Triggers update of display properties
  static void OnSegmentRemoved(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Callback function observing segment modified events.
  /// Forwards event from the node.
  static void OnSegmentModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Callback function observing representation created events.
  /// Forwards event from the node.
  static void OnRepresentationCreated(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Callback function observing representation removed events.
  /// Forwards event from the node.
  static void OnRepresentationRemoved(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

protected:
  vtkMRMLSegmentationNode();
  ~vtkMRMLSegmentationNode();
  vtkMRMLSegmentationNode(const vtkMRMLSegmentationNode&);
  void operator=(const vtkMRMLSegmentationNode&);

  /// Segmentation object to store the actual data
  vtkSegmentation* Segmentation;

  /// Keep track of merged labelmap modification time
  vtkTimeStamp LabelmapMergeTime;

  /// Command handling master representation modified events
  vtkCallbackCommand* MasterRepresentationCallbackCommand;

  /// Command handling segment added event
  vtkCallbackCommand* SegmentAddedCallbackCommand;

  /// Command handling segment removed event
  vtkCallbackCommand* SegmentRemovedCallbackCommand;

  /// Command handling segment modified event
  vtkCallbackCommand* SegmentModifiedCallbackCommand;

  /// Command handling representation created event
  vtkCallbackCommand* RepresentationCreatedCallbackCommand;

  /// Command handling representation removed event
  vtkCallbackCommand* RepresentationRemovedCallbackCommand;
};

#endif // __vtkMRMLSegmentationNode_h
