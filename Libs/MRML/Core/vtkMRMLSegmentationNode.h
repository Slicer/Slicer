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

#ifndef __vtkMRMLSegmentationNode_h
#define __vtkMRMLSegmentationNode_h

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLColorTableNode.h>

// STD includes
#include <cstdlib>

// vtkSegmentationCore includes
#include "vtkSegmentation.h"

class vtkCallbackCommand;
class vtkMRMLScene;
class vtkMRMLSubjectHierarchyNode;
class vtkMRMLScalarVolumeNode;

/// \brief MRML node containing segmentations
/// \ingroup Segmentations
///
/// Segmentation node stores a set of segments (also known as contours or segmented regions).
/// Segments may overlap and may be stored in various representations (binary labelmap image,
/// closed surface mesh, fractional labelmap image, etc). Segments can be stored in multiple data
/// representations to facilitate visualization and processing.
///
/// Storage and automatic conversion between representations are provided by \sa vtkSegmentation object.
///
class VTK_MRML_EXPORT vtkMRMLSegmentationNode : public vtkMRMLDisplayableNode
{
public:
  // Define constants
  static const char* GetSegmentIDAttributeName() { return "segmentID"; };

  static vtkMRMLSegmentationNode *New();
  vtkTypeMacro(vtkMRMLSegmentationNode, vtkMRMLDisplayableNode);
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

  /// Get bounding box in global RAS form (xmin,xmax, ymin,ymax, zmin,zmax).
  /// This method returns the bounds of the object with any transforms that may
  /// be applied to it.
  /// \sa GetBounds()
  virtual void GetRASBounds(double bounds[6]);

  /// Get bounding box in global RAS form (xmin,xmax, ymin,ymax, zmin,zmax).
  /// This method always returns the bounds of the untransformed object.
  /// \sa GetRASBounds()
  virtual void GetBounds(double bounds[6]);

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

  /// Function called from segmentation logic when UID is added in a subject hierarchy node.
  /// In case the newly added UID is a volume node referenced from this segmentation,
  /// its geometry will be set as image geometry conversion parameter.
  /// The "other order", i.e. when the volume is loaded first and the segmentation second,
  /// should be handled at loading time of the segmentation (because then we already know about the volume)
  /// \param shNode Subject hierarchy node that contains the item (should be the only SH node in the scene)
  /// \param itemWithNewUID ID of subject hierarchy item that just got a new UID
  virtual void OnSubjectHierarchyUIDAdded(vtkMRMLSubjectHierarchyNode* shNode, vtkIdType itemWithNewUID);

  /// Get subject hierarchy item belonging to a certain segment
  /// \param segmentID ID of segment contained by this segmentation to get the subject hierarchy virtual item for
  /// \param shNode Subject hierarchy node to search in (there should be only one in the scene and it can be got
  ///   using vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode)
  virtual vtkIdType GetSegmentSubjectHierarchyItem(std::string segmentID, vtkMRMLSubjectHierarchyNode* shNode);

#ifndef __VTK_WRAP__
//BTX
  /// Build merged labelmap of the binary labelmap representations of the specified segments
  /// \param mergedImageData Output image data for the merged labelmap image data. Voxels of background volume will be
  /// of signed short type. Label value of n-th segment in segmentIDs list will be (n + 1). Label value of background = 0.
  /// \param extentComputationMode Determines how to compute extents (EXTENT_REFERENCE_GEOMETRY, EXTENT_UNION_OF_SEGMENTS,
  ///   EXTENT_UNION_OF_SEGMENTS_PADDED, EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, or EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_PADDED).
  /// \param mergedLabelmapGeometry Determines geometry of merged labelmap if not NULL, automatically determined otherwise
  /// \param segmentIDs List of IDs of segments to include in the merged labelmap. If empty or missing, then all segments are included
  /// \return Success flag
  virtual bool GenerateMergedLabelmap(vtkOrientedImageData* mergedImageData, int extentComputationMode, vtkOrientedImageData* mergedLabelmapGeometry = NULL, const std::vector<std::string>& segmentIDs = std::vector<std::string>());
//ETX
#endif // __VTK_WRAP__

  /// Python-accessible version of the more generic \sa GenerateMergedLabelmap.
  /// The last argument specifying the list of segments to be included is omitted, which means that
  /// all the segments will be merged.
  /// \sa GenerateMergedLabelmap
  virtual bool GenerateMergedLabelmapForAllSegments(vtkOrientedImageData* mergedImageData, int extentComputationMode,
    vtkOrientedImageData* mergedLabelmapGeometry = NULL, vtkStringArray* segmentIDs = NULL);

  /// Expose reference identifier to get the volume node defining the reference image geometry if any
  static std::string GetReferenceImageGeometryReferenceRole() { return "referenceImageGeometryRef"; };
  /// Set reference image geometry conversion parameter from the volume node, keeping reference
  virtual void SetReferenceImageGeometryParameterFromVolumeNode(vtkMRMLScalarVolumeNode* volumeNode);

  /// Get segmentation object
  vtkGetObjectMacro(Segmentation, vtkSegmentation);
  /// Set and observe segmentation object
  void SetAndObserveSegmentation(vtkSegmentation* segmentation);

protected:
  /// Set segmentation object
  vtkSetObjectMacro(Segmentation, vtkSegmentation);

  /// Callback function for all events from the segmentation object.
  static void SegmentationModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Callback function observing the master representation of the segmentation (and each segment within)
  /// Invalidates all representations other than the master. These representations will be automatically converted later on demand.
  void OnMasterRepresentationModified();

  /// Callback function observing segment added events.
  /// Triggers update of display properties
  void OnSegmentAdded(const char* segmentId);

  /// Callback function observing segment removed events.
  /// Triggers update of display properties
  void OnSegmentRemoved(const char* segmentId);

  /// Callback function observing segment modified events.
  /// Forwards event from the node.
  void OnSegmentModified(const char* segmentId);

protected:
  vtkMRMLSegmentationNode();
  virtual ~vtkMRMLSegmentationNode();
  vtkMRMLSegmentationNode(const vtkMRMLSegmentationNode&);
  void operator=(const vtkMRMLSegmentationNode&);

  /// Segmentation object to store the actual data
  vtkSegmentation* Segmentation;

  /// Command handling events from segmentation object
  vtkSmartPointer<vtkCallbackCommand> SegmentationModifiedCallbackCommand;
};

#endif // __vtkMRMLSegmentationNode_h
