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

// SegmentationCore includes
#include "vtkSegment.h"
#include "vtkSegmentationConverter.h"
#include "vtkSegmentationConverterRule.h"

#include "vtkSegmentationCoreConfigure.h"

class vtkAbstractTransform;
class vtkCallbackCommand;
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
///     * Additional representations can be defined (SlicerRT adds three: Planar contour, Fractional labelmap, Ribbon model)
///   * Conversion between representations are driven by a conversion graph in which the nodes are the representations and the edges
///     are conversion rules
///     * When converting with the default method (\sa CreateRepresentation without specifying a path), then the path with the lowest
///       cost is used (rules have a cost field that gives a ballpark value for the conversion cost)
///     * Representation types can be defined by registering conversion algorithms (rules) that specify their source and target
///       representations, and an estimated cost metric
///   * Master representation
///     * Privileged representation type. Can be any of the available representations, but usually it's the original representation
///       of the data (binary labelmap for editing, planar contour for RT)
///     * Properties:
///       a) all conversions use it as source,
///       b) when changed all other representations are invalidated, and
///       c) it is the representation that is saved to disk
///     * Using the proper master representation ensures that no information is lost, which is crucial to avoid discrepancies that can
///       never be solved when data is permanently lost in conversion
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
    ContainedRepresentationNamesModified
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
  void PrintSelf(ostream& os, vtkIndent indent);

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
//BTX
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
//ETX
#endif // __VTK_WRAP__

// Segment related methods
public:
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
  /// \return Success flag
  bool AddSegment(vtkSegment* segment, std::string segmentId="");

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

  /// Return all contained segments
  SegmentMap GetSegments() { return this->Segments; };

  /// Get IDs for all contained segments
  void GetSegmentIDs(std::vector<std::string> &segmentIds);

  /// Get IDs for all contained segments, for python compatibility
  void GetSegmentIDs(vtkStringArray* segmentIds);

  /// Request the total number of segments, primarily used for iterating over all segments
  int GetNumberOfSegments() const;

  /// Find segment ID by segment instance
  std::string GetSegmentIdBySegment(vtkSegment* segment);

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
public:
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

// Conversion related methods
public:
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
  bool CreateRepresentation(const std::string& targetRepresentationName,
                            vtkSegmentationConverter::ConversionPathType path,
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
  /// \param defaultColor of added segment. If not specified then empty then vtkSegment::SEGMENT_COLOR_VALUE_INVALID is used.
  /// \return ID of the added segment. Empty on failure
  std::string AddEmptySegment(std::string segmentId="", std::string segmentName="", double defaultColor[3]=NULL);

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
public:
  /// Get master representation name
  vtkGetMacro(MasterRepresentationName, std::string);
  /// Set master representation name.
  /// Need to make sure before setting the name that the newly set master representation exists in
  /// the segmentation! Use \sa CreateRepresentation for that.
  virtual void SetMasterRepresentationName(const std::string& representationName);

protected:
  /// Convert given segment along a specified path
  /// \param segment Segment to convert
  /// \param path Path to do the conversion along
  /// \param overwriteExisting If true then do each conversion step regardless the target representation
  ///   exists. If false then skip those conversion steps that would overwrite existing representation
  /// \return Success flag
  bool ConvertSegmentUsingPath(vtkSegment* segment, vtkSegmentationConverter::ConversionPathType path, bool overwriteExisting=false);

  /// Converts a single segment to a representation.
  bool ConvertSingleSegment(std::string segmentId, std::string targetRepresentationName);

  /// Remove segment by iterator. The two \sa RemoveSegment methods call this function after
  /// finding the iterator based on their different input arguments.
  void RemoveSegment(SegmentMap::iterator segmentIt);

  /// Generate unique segment ID. If argument is empty then a new ID will be generated in the form "SegmentN",
  /// where N is the number of segments. If argument is unique it is returned unchanged. If there is a segment
  /// with the given name, then it is postfixed by "_1"
  std::string GenerateUniqueSegmentId(std::string id);

  /// Temporarily enable/disable master representation modified event.
  /// \return Old value of MasterRepresentationModifiedEnabled.
  /// In general, the old value should be restored after modified is temporarily disabled to ensure proper
  /// state when calling SetMasterRepresentationModifiedEnabled in nested functions.
  bool SetMasterRepresentationModifiedEnabled(bool enabled);

protected:
  /// Callback function invoked when segment is modified.
  /// It calls Modified on the segmentation and rebuilds observations on the master representation of each segment
  static void OnSegmentModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Callback function observing the master representation of each segment
  /// It fires a \sa MasterRepresentationModifiedEvent if master representation is changed in ANY segment
  static void OnMasterRepresentationModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

protected:
  vtkSegmentation();
  ~vtkSegmentation();
  void operator=(const vtkSegmentation&);

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

  friend class vtkSlicerSegmentationsModuleLogic;
  friend class qMRMLSegmentEditorWidgetPrivate;
};

#endif // __vtkSegmentation_h
