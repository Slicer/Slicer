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

#ifndef __vtkSegmentation_h
#define __vtkSegmentation_h

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// STD includes
#include <map>
#include <deque>
#include <vector>

// SegmentationCore includes
#include "vtkSegment.h"
#include "vtkSegmentationConverter.h"
#include "vtkSegmentationConverterRule.h"

#include "vtkSegmentationCoreConfigure.h"

class vtkAbstractTransform;
class vtkCallbackCommand;
class vtkCollection;
class vtkIntArray;
class vtkStringArray;

/// \ingroup SegmentationCore
/// \brief This class encapsulates a segmentation that can contain multiple segments and multiple representations for each segment
/// \details
///   The primary purpose of this class is to serve as a container to store the segments (in labelmap analogy the "labels").
///   Also provides generic functions on the segmentation level. Performs conversion to a specified representation, extracts
///   geometry information etc.
///
///   Main points to remember:
///   * Each segment has the same set of representations. This means that if segments are copied/moved between segmentations,
///     then conversion will take place if possible (if not then copy will fail)
///   * Default representations types are
///     * Binary labelmap (vtkOrientedImageData)
///     * Closed surface (vtkPolyData)
///     * Fractional labelmap (vtkOrientedImageData)
///     * Additional representations can be defined (SlicerRT adds two: Planar contour, Ribbon model)
///       (https://github.com/SlicerRt/SlicerRT/tree/master/DicomRtImportExport/ConversionRules)
///   * Conversion between representations are driven by a conversion graph in which the nodes are the representations and the edges
///     are conversion rules
///     * When converting with the default method (\sa CreateRepresentation without specifying a path), then the path with the lowest
///       cost is used (rules have a cost field that gives a ballpark value for the conversion cost)
///     * Representation types can be defined by registering conversion algorithms (rules) that specify their source and target
///       representations, and an estimated cost metric
///   * Master representation
///     * Privileged representation type. Can be any of the available representations, but usually it's the original representation
///       of the data (binary labelmap for editing, binary or fractional labelmap for DICOM SEG, planar contour for DICOM RT, etc.)
///       * Using the proper master representation ensures that no information is lost, which is crucial to avoid discrepancies that can
///         never be solved when data is permanently lost in conversion
///     * Properties
///       * All conversions use it as source (up-to-date representations along conversion path are used if available)
///       * When changed all other representations are invalidated (and is re-converted later from master)
///       * It is the representation that is saved to disk
///
///  Schematic illustration of the segmentation container:
///
///                            +=============================================+
///                            |             Patient (vtkSegmentation)       |
///                            +======================+======================+
///                            |  Brain (vtkSegment)  |  Tumor (vtkSegment)  |
///                            +======================+======================+
///            Binary labelmap | vtkOrientedImageData | vtkOrientedImageData |
///                            +----------------------+----------------------+
///             Closed surface | vtkPolyData          | vtkPolyData          |
///                            +----------------------+----------------------+
///      Custom representation | vtkDataObject        | vtkDataObject        |
///                            +----------------------+----------------------+
///
class vtkSegmentationCore_EXPORT vtkSegmentation : public vtkObject
{
public:
  enum
    {
    /// Invoked when content of the master representation in a segment is changed.
    MasterRepresentationModified = 62100,
    /// Invoked when content of any representation (including the master representation) in a segment is changed.
    RepresentationModified,
    /// Invoked if new segment is added
    SegmentAdded,
    /// Invoked if a segment is removed
    SegmentRemoved,
    /// Invoked if a segment is modified (name changed, tags changed, etc).
    /// Note: the event is not invoked when content of a representation in a segment is changed.
    SegmentModified,
    /// Invoked if a representation is created or removed in the segments (e.g., created by conversion from master).
    ContainedRepresentationNamesModified,
    /// Invoked if segment IDs order is changed. Not called when a segment is added or removed.
    SegmentsOrderModified,
    };

  enum
    {
    /// Extent of common geometry is used as extent
    EXTENT_REFERENCE_GEOMETRY,
    /// Extent is computed as union of extent of all segments
    EXTENT_UNION_OF_SEGMENTS,
    /// Extent is computed as union of extent of all segments, with a single-voxel padding added on each side
    EXTENT_UNION_OF_SEGMENTS_PADDED,
    /// Extent is computed as union of effective extent of all segments
    EXTENT_UNION_OF_EFFECTIVE_SEGMENTS,
    /// Extent is computed as union of effective extent of all segments, with a single-voxel padding added on each side
    EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_PADDED
    };

  /// Container type for segments. Maps segment IDs to segment objects
  typedef std::map<std::string, vtkSmartPointer<vtkSegment> > SegmentMap;

public:
  static vtkSegmentation* New();
  vtkTypeMacro(vtkSegmentation, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set attributes from name/value pairs
  virtual void ReadXMLAttributes(const char** atts);

  /// Write this object's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Deep copy one segmentation into another
  virtual void DeepCopy(vtkSegmentation* aSegmentation);

  /// Copy conversion parameters from another segmentation
  virtual void CopyConversionParameters(vtkSegmentation* aSegmentation);

  /// Get bounding box in global RAS in the form (xmin,xmax, ymin,ymax, zmin,zmax).
  virtual void GetBounds(double bounds[6]);

  /// Apply a linear transform on the master representation of the segments. The others will be invalidated
  /// Harden transform if poly data, apply to directions if oriented image data.
  virtual void ApplyLinearTransform(vtkAbstractTransform* transform);

  /// Apply a non-linear transform on the master representation of the segments. The others will be invalidated
  /// Harden transform both if oriented image data and poly data.
  virtual void ApplyNonLinearTransform(vtkAbstractTransform* transform);

#ifndef __VTK_WRAP__
  /// Determine common labelmap geometry for whole segmentation.
  /// If the segmentation has reference image geometry conversion parameter, then oversample it to
  /// be at least as fine resolution as the highest resolution labelmap contained, otherwise just use
  /// the geometry of the highest resolution labelmap in the segments.
  /// \param extentComputationMode Determines how to compute extents (EXTENT_REFERENCE_GEOMETRY, EXTENT_UNION_OF_SEGMENTS, EXTENT_UNION_OF_SEGMENTS_PADDED,
  ///   EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, or EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_PADDED).
  /// \param segmentIDs List of IDs of segments to include in the merged labelmap. If empty or missing, then all segments are included
  /// \return Geometry string that can be deserialized using \sa vtkSegmentationConverter::SerializeImageGeometry
  std::string DetermineCommonLabelmapGeometry(int extentComputationMode = EXTENT_UNION_OF_SEGMENTS, const std::vector<std::string>& segmentIDs = std::vector<std::string>());

  /// Determine common labelmap extent for whole segmentation.
  /// \param commonGeometryExtent Computed extent that contains all the specified segments.
  /// \param commonGeometryImage Extent will be returned in this image geometry
  /// \param segmentIDs List of IDs of segments to include in the merged labelmap. If empty or missing, then all segments are included
  /// \param computeEffectiveExtent Specifies if the extent of a segment is the whole extent or the effective extent (where voxel values >0 found)
  void DetermineCommonLabelmapExtent(int commonGeometryExtent[6], vtkOrientedImageData* commonGeometryImage,
    const std::vector<std::string>& segmentIDs = std::vector<std::string>(), bool computeEffectiveExtent=false, bool addPadding=false);
#endif // __VTK_WRAP__

  /// Determine common labelmap geometry for whole segmentation, for python compatibility.
  std::string DetermineCommonLabelmapGeometry(int extentComputationMode, vtkStringArray* segmentIds);

  /// Determine common labelmap extent for whole segmentation, for python compatibility.
  void DetermineCommonLabelmapExtent(int commonGeometryExtent[6], vtkOrientedImageData* commonGeometryImage,
    vtkStringArray* segmentIds, bool computeEffectiveExtent=false, bool addPadding=false);

  /// Updates image geometry (origin, spacing, axis directions, extents) based on labelmaps stored in the segmentation.
  /// Does not allocate memory (to allow just retrieving geometry information without using memory).
  bool SetImageGeometryFromCommonLabelmapGeometry(vtkOrientedImageData* imageData, vtkStringArray* segmentIDs = nullptr,
    int extentComputationMode = vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS);

// Segment related methods

  /// Add a segment to this segmentation, do necessary conversions, and observe underlying
  /// data for changes.
  /// Necessary conversions:
  ///   1. If the segment can be added (\sa CanAcceptSegment), and it does
  ///   not contain the master representation, then the master representation is converted
  ///   using the cheapest available path.
  ///   2. Make sure that the segment contains the same types of representations that are
  ///   present in the existing segments of the segmentation (because we expect all segments
  ///   in a segmentation to contain the same types of representations).
  /// \param segment the segment to observe
  /// \param insertBeforeSegmentId if specified then the segment is inserted before insertBeforeSegmentId
  /// \return Success flag
  bool AddSegment(vtkSegment* segment, std::string segmentId = "", std::string insertBeforeSegmentId = "");

  /// Generate unique segment ID. If argument is empty then a new ID will be generated in the form "Segment_",
  /// where N is the number of segments. If argument is unique it is returned unchanged. If there is a segment
  /// with the given name, then it is postfixed by a number to make it unique.
  std::string GenerateUniqueSegmentID(std::string id);

  /// Remove a segment by ID
  /// \param segmentId Identifier of the segment to remove from the segmentation
  void RemoveSegment(std::string segmentId);

  /// Remove a segment by value
  /// \param segment the segment to remove from the segmentation
  void RemoveSegment(vtkSegment* segment);

  /// Remove all segments
  void RemoveAllSegments();

  /// Access a segment by ID
  /// \param segmentId Segment identifier in the container to access
  vtkSegment* GetSegment(std::string segmentId);

  /// Get IDs for all contained segments
  void GetSegmentIDs(std::vector<std::string> &segmentIds);

  /// Get IDs for all contained segments, for python compatibility
  void GetSegmentIDs(vtkStringArray* segmentIds);

  /// Request the total number of segments, primarily used for iterating over all segments
  int GetNumberOfSegments() const;

  /// Request segment by index
  vtkSegment* GetNthSegment(unsigned int index) const;

  /// Get n-th segment ID. Return with "" if no segment is found by that index.
  std::string GetNthSegmentID(unsigned int index) const;

  /// Get index of segment in the SegmentID list.
  /// Returns -1 if the segment ID is not in the SegmentID list.
  /// \sa GetSegmentIDs
  int GetSegmentIndex(const std::string& segmentId);

  /// Changes segment order. Segment order may be used for display and generating merged labelmaps.
  /// \return True if segment index has changed successfully (or the index has already been set).
  /// \sa ReorderSegments
  bool SetSegmentIndex(const std::string& segmentId, unsigned int newIndex);

  /// Reorder segment IDs so that the list of segment IDs are moved from their current position
  /// and inserted after the specified segment.
  /// If insertBeforeSegmentId is empty then segments are moved to the end of the segment list.
  /// \sa SetSegmentIndex
  void ReorderSegments(std::vector<std::string> segmentIdsToMove, std::string insertBeforeSegmentId = "");

  /// Find segment ID by segment instance
  /// Returns empty string if segment is not found.
  std::string GetSegmentIdBySegment(vtkSegment* segment);

  /// Find segment ID by segment name. Search is case-insensitive.
  /// If multiple segments have the same name, the first match is returned.
  /// Returns empty string if segment is not found.
  std::string GetSegmentIdBySegmentName(std::string name);

  /// Get segments that contain a certain tag
  /// \param tag Tag name to look for in segments
  /// \param value Tag value to look for in segments. If omitted or empty then any value is accepted
  /// \return Vector of segments containing the requested tag
  std::vector<vtkSegment*> GetSegmentsByTag(std::string tag, std::string value="");

  /// Get representation from segment
  vtkDataObject* GetSegmentRepresentation(std::string segmentId, std::string representationName);

  /// Copy segment from one segmentation to this one
  /// \param fromSegmentation Source segmentation
  /// \param segmentId ID of segment to copy
  /// \param removeFromSource If true, then delete segment from source segmentation after copying.
  ///                        Default value is false.
  /// \return Success flag
  bool CopySegmentFromSegmentation(vtkSegmentation* fromSegmentation, std::string segmentId, bool removeFromSource=false);

// Representation related methods

  /// Get representation names present in this segmentation in an output string vector
  /// Note: This assumes the first segment contains the same type of representations as
  ///       all segments (this should be the case by design)
  void GetContainedRepresentationNames(std::vector<std::string>& representationNames);

  /// Determine if segments contain a certain representation type
  /// Note: This assumes the first segment contains the same type of representations as
  ///       all segments (this should be the case by design)
  bool ContainsRepresentation(std::string representationName);

  /// Determine if master representation is poly data type
  bool IsMasterRepresentationPolyData();

  /// Determine if master representation is (oriented) image data type
  bool IsMasterRepresentationImageData();

  /// Get all representations supported by the converter
  void GetAvailableRepresentationNames(std::set<std::string>& representationNames) { this->Converter->GetAvailableRepresentationNames(representationNames); };

  /// Invalidate (remove) non-master representations in all the segments if this segmentation node
  void InvalidateNonMasterRepresentations();

  /// Merged labelmap functions

#ifndef __VTK_WRAP__
  /// Create a merged labelmap from the segment IDs
  /// If no segment IDs are specified, then all segments will be merged
  /// \param mergedImageData Output image data for the merged labelmap image data. Voxels of background volume will be
  /// of signed short type. Label value of n-th segment in segmentIDs list will be (n + 1), or will be specified in labelValues.
  /// Label value of background = 0.
  /// \param extentComputationMode Input that determines how to compute extents (EXTENT_REFERENCE_GEOMETRY, EXTENT_UNION_OF_SEGMENTS,
  ///   EXTENT_UNION_OF_SEGMENTS_PADDED, EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, or EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_PADDED).
  /// \param mergedLabelmapGeometry Determines geometry of merged labelmap if not nullptr, automatically determined otherwise
  /// \param segmentIDs List of IDs of segments to include in the merged labelmap. If empty or missing, then all segments are included
  /// \param labelValues Input list of label values that will be used in the merged labelmap.
  ///   If not specified, then the label values in the segmentation will be used.
  ///   The size of the array should match the number of segment IDs used in the merged labelmap.
  bool GenerateMergedLabelmap(vtkOrientedImageData* mergedImageData, int extentComputationMode, vtkOrientedImageData* mergedLabelmapGeometry = nullptr,
    const std::vector<std::string>& segmentIDs = std::vector<std::string>(), vtkIntArray* labelValues = nullptr);
#endif // __VTK_WRAP__

 /// Shared labelmap utility functions

  /// Returns true if the binary labelmap representation is shared
  bool IsSharedBinaryLabelmap(std::string segmentID);

  /// Gets the segment IDs that are shared with the specified segment
  /// \param originalSegmentId ID of the specified segment
  /// \param representationName Representation to check for shared segment IDs
  /// \param sharedSegmentIds Output segment IDs
  /// \param includeOriginalSegmentId If true, the original segment ID is included in the output
  void GetSegmentIDsSharingRepresentation(std::string originalSegmentId, std::string representationName,
    std::vector<std::string>& sharedSegmentIds, bool includeOriginalSegmentId=true);

  /// Gets the segment IDs that are shared with the specified segment binary labelmap
  /// \param originalSegmentId ID of the specified segment
  /// \param sharedSegmentIds Output segment IDs
  /// \param includeOriginalSegmentId If true, the original segment ID is included in the output
  void GetSegmentIDsSharingBinaryLabelmapRepresentation(std::string originalSegmentId, std::vector<std::string> &sharedSegmentIds,
    bool includeOriginalSegmentId=true);

  /// Gets a unique shared labelmap value for the specified segment
  /// Compares all of the other segments that are shared with the specified segmentId to determine the a unique
  /// labelmap value
  int GetUniqueLabelValueForSharedLabelmap(std::string segmentId);

  /// Gets a unique shared labelmap value for the specified labelmap
  /// Returns labelmap->GetScalarRange()[1] + 1
  int GetUniqueLabelValueForSharedLabelmap(vtkOrientedImageData* labelmap);

  /// Merges the specified segments into the same shared labelmap
  /// This method can cause segments to overwrite each other during during merge.
  /// Segments with a higher index will overwrite segments with a lower index.
  /// If no segment IDs are specified, then all segments will be shared
  void MergeSegmentLabelmaps(std::vector<std::string> mergeSegmentIds);

  /// Moves an existing segment from a shared labelmap into a separate labelmap containing only the specified segment
  void SeparateSegmentLabelmap(std::string segmentId);

  /// Clears the segment representation.
  /// If the segment is in a shared labelmap, it will be erased from the labelmap.
  /// Otherwise, the vtkDataObject will be initialized.
  void ClearSegment(std::string segmentId);

  /// Shared representation layer functions

  /// Get the number of unique vtkDataObject that are used for a particular representation type
  /// If representationName is not specified, it will be set to the master representation name
  int GetNumberOfLayers(std::string representationName="");

  /// Get the layer index for a particular segment
  /// If representationName is not specified, it will be set to the master representation name
  int GetLayerIndex(std::string segmentId, std::string representationName="");

  /// Get the data object for a particular layer index
  /// If representationName is not specified, it will be set to the master representation name
  vtkDataObject* GetLayerDataObject(int layer, std::string representationName="");

  /// Get a collection of all of the data objects in the segmentation
  /// If representationName is not specified, it will be set to the master representation name
  void GetLayerObjects(vtkCollection* layerObjects, std::string representationName = "");

  /// Get the segmentIDs contained in the specified layer
  /// If representationName is not specified, it will be set to the master representation name
  std::vector<std::string> GetSegmentIDsForLayer(int layer, std::string representationName = "");

  /// Get the segmentIDs that use a specific data object
  /// If representationName is not specified, it will be set to the master representation name
  std::vector<std::string> GetSegmentIDsForDataObject(vtkDataObject* dataObject, std::string representationName = "");

  /// Reduce the binary labelmap representation to as few layers as possible.
  /// \param forceToSingleLayer If false, then the layers will not be overwritten by each other, if true then the layers can
  ///   overwrite each other, but the result is guaranteed to have one layer
  void CollapseBinaryLabelmaps(bool forceToSingleLayer=false);

  // Conversion related methods

  /// Create a representation in all segments, using the conversion path with the
  /// lowest cost. The stored conversion parameters are used (which are the defaults if not changed by the user).
  /// Conversion starts from the master representation. If a representation along
  /// the path already exists then no conversion is performed.
  /// Note: The conversion functions are not in vtkSegmentationConverter, because
  ///       they need to know about the master representation which is segmentation-
  ///       specific, and also to allow optimizations (steps before per-segment conversion).
  /// \param targetRepresentationName Name of the representation to create
  /// \param alwaysConvert If true, then conversion takes place even if target representation exists. False by default.
  /// \return true on success
  bool CreateRepresentation(const std::string& targetRepresentationName, bool alwaysConvert=false);

  /// Generate or update a representation in all segments, using the specified conversion
  /// path and parameters.
  /// Conversion starts from the master representation, and all representations along the
  /// path get overwritten.
  /// \return true on success
  bool CreateRepresentation(vtkSegmentationConverter::ConversionPathType path,
                            vtkSegmentationConverterRule::ConversionParameterListType parameters);

  /// Removes a representation from all segments if present
  void RemoveRepresentation(const std::string& representationName);

  /// Determine if the segmentation is ready to accept a certain type of representation
  /// by copy/move or import. It can accept a representation if it is the master representation
  /// of this segment or it is possible to convert to master representation (or the segmentation
  /// is empty).
  bool CanAcceptRepresentation(std::string representationName);

  /// Determine if the segmentation is ready to accept a certain segment. It can accept a
  /// segment if it contains a representation that is acceptable, or if it is empty.
  bool CanAcceptSegment(vtkSegment* segment);

  /// Add empty segment containing empty instances of the contained representations
  /// \param segmentId ID of added segment. If empty then a default ID will be generated \sa GenerateUniqueSegmentId
  /// \param segmentName name of added segment. If empty then the segmentId will be used as name.
  /// \param color of added segment. If not specified then empty then vtkSegment::SEGMENT_COLOR_INVALID is used.
  /// \return ID of the added segment. Empty on failure
  std::string AddEmptySegment(std::string segmentId="", std::string segmentName="", double color[3]=nullptr);

  /// Get all possible conversions between the master representation and a specified target representation
  void GetPossibleConversions(const std::string& targetRepresentationName,
    vtkSegmentationConverter::ConversionPathAndCostListType &pathsCosts);

  /// Set a conversion parameter to all rules having this parameter
  void SetConversionParameter(const std::string& name, const std::string& value) { this->Converter->SetConversionParameter(name, value); };

  /// Get a conversion parameter from first rule containing this parameter
  /// Note: all parameters with the same name should contain the same value
  std::string GetConversionParameter(const std::string& name) { return this->Converter->GetConversionParameter(name); };

  /// Get names of all conversion parameters used by the selected conversion path
  void GetConversionParametersForPath(vtkSegmentationConverterRule::ConversionParameterListType& conversionParameters,
    const vtkSegmentationConverter::ConversionPathType& path) { this->Converter->GetConversionParametersForPath(conversionParameters, path); };

  /// Serialize all conversion parameters.
  /// The resulting string can be parsed in a segmentation object using /sa DeserializeConversionParameters
  std::string SerializeAllConversionParameters();

  /// Parse conversion parameters in string and set it to the segmentation converter
  /// Such a string can be constructed in a segmentation object using /sa SerializeAllConversionParameters
  void DeserializeConversionParameters(std::string conversionParametersString);

// Get/set methods

  /// Get master representation name
  vtkGetMacro(MasterRepresentationName, std::string);
  /// Set master representation name.
  /// Need to make sure before setting the name that the newly set master representation exists in
  /// the segmentation! Use \sa CreateRepresentation for that.
  virtual void SetMasterRepresentationName(const std::string& representationName);

  /// Deep copies source segment to destination segment. If the same representation is found in baseline
  /// with up-to-date timestamp then the representation is reused from baseline.
  static void CopySegment(vtkSegment* destination, vtkSegment* source, vtkSegment* baseline,
    std::map<vtkDataObject*, vtkDataObject*>& cachedRepresentations);

protected:
  bool ConvertSegmentsUsingPath(std::vector<std::string> segmentIDs, vtkSegmentationConverter::ConversionPathType path, bool overwriteExisting = false);

  /// Convert given segment along a specified path
  /// \param segment Segment to convert
  /// \param path Path to do the conversion along
  /// \param overwriteExisting If true then do each conversion step regardless the target representation
  ///   exists. If false then skip those conversion steps that would overwrite existing representation
  /// \return Success flag
  bool ConvertSegmentUsingPath(vtkSegment* segment, vtkSegmentationConverter::ConversionPathType path, bool overwriteExisting = false);

  /// Converts a single segment to a representation.
  bool ConvertSingleSegment(std::string segmentId, std::string targetRepresentationName);

  /// Remove segment by iterator. The two \sa RemoveSegment methods call this function after
  /// finding the iterator based on their different input arguments.
  void RemoveSegment(SegmentMap::iterator segmentIt);

  /// Temporarily enable/disable master representation modified event.
  /// \return Old value of MasterRepresentationModifiedEnabled.
  /// In general, the old value should be restored after modified is temporarily disabled to ensure proper
  /// state when calling SetMasterRepresentationModifiedEnabled in nested functions.
  bool SetMasterRepresentationModifiedEnabled(bool enabled);

  /// Temporarily enable/disable segment modified event.
  /// \return Old value of SegmentModifiedEnabled.
  /// In general, the old value should be restored after modified is temporarily disabled to ensure proper
  /// state when calling SetSegmentModifiedEnabled in nested functions.
  bool SetSegmentModifiedEnabled(bool enabled);

protected:
  /// Callback function invoked when segment is modified.
  /// It calls Modified on the segmentation and rebuilds observations on the master representation of each segment
  static void OnSegmentModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Callback function observing the master representation of each segment
  /// It fires a \sa MasterRepresentationModifiedEvent if master representation is changed in ANY segment
  static void OnMasterRepresentationModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Check to ensure that all master representations are being observed, and observers on master representations that
  /// are no longer in the segmentation are removed
  void UpdateMasterRepresentationObservers();

protected:
  vtkSegmentation();
  ~vtkSegmentation() override;

protected:
  /// Container of segments that belong to this segmentation
  SegmentMap Segments;

  /// Master representation type name.
  /// 1. This representation is saved on disk
  /// 2. If this representation is modified, the others are invalidated
  /// This value must be set by the creator of the segmentation object!
  std::string MasterRepresentationName;

  /// Converter instance
  vtkSegmentationConverter* Converter;

  /// Command handling segment modified events
  vtkCallbackCommand* SegmentCallbackCommand;

  /// Command handling master representation modified events
  vtkCallbackCommand* MasterRepresentationCallbackCommand;

  /// Modified events of  master representations are observed
  bool MasterRepresentationModifiedEnabled;

  /// Modified events of segments are observed
  bool SegmentModifiedEnabled;

  /// This number is incremented and used for generating the next
  /// segment ID.
  int SegmentIdAutogeneratorIndex;

  /// This contains the segment IDs in display order.
  /// (we could retrieve segment IDs from SegmentMap too, but that always contains segments in
  /// alphabetical order)
  std::deque< std::string > SegmentIds;

  std::set<vtkSmartPointer<vtkDataObject> > MasterRepresentationCache;

  friend class vtkMRMLSegmentationNode;
  friend class vtkSlicerSegmentationsModuleLogic;
  friend class vtkSegmentationModifier;
  friend class qMRMLSegmentEditorWidgetPrivate;

private:
  vtkSegmentation(const vtkSegmentation&) = delete;
  void operator=(const vtkSegmentation&) = delete;
};

#endif // __vtkSegmentation_h
