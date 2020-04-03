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

#ifndef __vtkMRMLSegmentationDisplayNode_h
#define __vtkMRMLSegmentationDisplayNode_h

#include "vtkMRMLDisplayNode.h"

#include <set>

class vtkMRMLColorTableNode;
class vtkSegmentation;
class vtkStringArray;
class vtkVector3d;


/// \ingroup Segmentations
/// \brief MRML node for representing segmentation display attributes.
///
/// vtkMRMLSegmentationDisplayNode nodes describe how volume is displayed.
class VTK_MRML_EXPORT vtkMRMLSegmentationDisplayNode : public vtkMRMLDisplayNode
{
public:
  // Define constants
  static const double SEGMENT_COLOR_NO_OVERRIDE;

  /// Display properties per segment
  struct SegmentDisplayProperties
    {
    /// Override segment color
    /// By default it's invalid (-1,-1,-1), and only valid if different than color stored in segment. Its purpose is to enable showing per-view colors
    double OverrideColor[3];
    /// Visibility
    bool Visible{true}; // Turns visibility on/off in general. Useful for allowing the user to show/hide a segment without changing any detailed visibility options.
    bool Visible3D{true};
    bool Visible2DFill{true}; // This one is used for labelmap volume related operations (color table, merged labelmap)
    bool Visible2DOutline{true};
    /// Opacity
    double Opacity3D{1.0};
    double Opacity2DFill{1.0}; // This one is used for labelmap volume related operations (color table, merged labelmap)
    double Opacity2DOutline{1.0};

    // Initialize with default values
    SegmentDisplayProperties()
      {
      OverrideColor[0] = SEGMENT_COLOR_NO_OVERRIDE;
      OverrideColor[1] = SEGMENT_COLOR_NO_OVERRIDE;
      OverrideColor[2] = SEGMENT_COLOR_NO_OVERRIDE;
      }

    // Automatically generated operator= and copy constructor work
    // correctly for these members, so there is no need to define them.
    };

  typedef std::map<std::string, SegmentDisplayProperties> SegmentDisplayPropertiesMap;

public:
  static vtkMRMLSegmentationDisplayNode *New();
  vtkTypeMacro(vtkMRMLSegmentationDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLSegmentationDisplayNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "SegmentationDisplay"; }

public:
  /// Get name of representation that is displayed in the 2D view if exists
  /// To get the actually displayed 2D representation call \sa
  vtkGetStringMacro(PreferredDisplayRepresentationName2D);
  /// Set name of representation that is displayed in the 2D view if exists
  vtkSetStringMacro(PreferredDisplayRepresentationName2D);
  /// Get name of representation that is displayed in the 3D view if exists
  vtkGetStringMacro(PreferredDisplayRepresentationName3D);
  /// Set name of representation that is displayed in the 3D view if exists
  vtkSetStringMacro(PreferredDisplayRepresentationName3D);

  /// Get/Set 2D fill visibility
  vtkGetMacro(Visibility2DFill, bool);
  vtkSetMacro(Visibility2DFill, bool);
  vtkBooleanMacro(Visibility2DFill, bool);
  /// Get/Set 2D outline visibility
  vtkGetMacro(Visibility2DOutline, bool);
  vtkSetMacro(Visibility2DOutline, bool);
  vtkBooleanMacro(Visibility2DOutline, bool);

  /// Get/Set 3D opacity
  vtkGetMacro(Opacity3D, double);
  vtkSetMacro(Opacity3D, double);
  /// Get/Set 2D fill opacity
  vtkGetMacro(Opacity2DFill, double);
  vtkSetMacro(Opacity2DFill, double);
  /// Get/Set 2D outline opacity
  vtkGetMacro(Opacity2DOutline, double);
  vtkSetMacro(Opacity2DOutline, double);

public:
  /// Get segment display properties for a specific segment
  /// \param segmentID Identifier of segment of which the properties are queried
  /// \param properties Display properties of the segment are copied into this object. If display properties
  /// are not defined for this segment explicitly then a warning is logged and default display properties are used.
  /// \return True if display properties are specified for the segment.
  bool GetSegmentDisplayProperties(std::string segmentID, SegmentDisplayProperties &properties);

  /// Set segment display properties.
  void SetSegmentDisplayProperties(std::string segmentID, SegmentDisplayProperties &properties);

  /// Set segment display properties to default.
  void SetSegmentDisplayPropertiesToDefault(const std::string& segmentId);

  /// Remove segment display properties
  void RemoveSegmentDisplayProperties(std::string segmentID);

  /// Clear segment display properties
  void ClearSegmentDisplayProperties();

  /// Determine and set automatic opacities for segments using topological hierarchies.
  /// Stores value in opacity component of \sa SegmentDisplayProperties.
  /// \return Success flag
  bool CalculateAutoOpacitiesForSegments();

  /// Generate new color for an added segment. Uses \sa NumberOfGeneratedColors to get the color
  /// for the new segment from default label color table
  /// \param color Output argument for the generated color
  /// \param colorNumber Index of the color in generic anatomy color table that is returned.
  ///                    Default value is 0, meaning that \sa NumberOfGeneratedColors is used.
  ///                    A value of -1 means that a random color is generated.
  void GenerateSegmentColor(double color[3], int colorNumber=0);
  /// Python compatibility function for \sa GenerateSegmentColor.
  /// The color with index \sa NumberOfGeneratedColors from generic anatomy color table is returned.
  void GenerateSegmentColor(double &r, double &g, double &b);

  /// Collect representation names that are stored as poly data
  void GetPolyDataRepresentationNames(std::set<std::string> &representationNames);

  /// Decide which poly data representation to use for 3D display.
  /// If preferred representation exists \sa PreferredDisplayRepresentationName3D, then return that.
  /// Otherwise if master representation is a poly data then return master representation type.
  /// Otherwise return first poly data representation if any.
  /// Otherwise return empty string meaning there is no poly data representation to display.
  std::string GetDisplayRepresentationName3D();

  /// Decide which representation to use for 2D display.
  /// If preferred representation exists \sa PreferredDisplayRepresentationName2D, then return that.
  /// Otherwise return master representation.
  std::string GetDisplayRepresentationName2D();

// Convenience and python compatibility functions
public:
  /// Get segment color by segment ID. Convenience function for python compatibility.
  /// \return Segment color not overridden, otherwise the override color
  vtkVector3d GetSegmentColor(std::string segmentID);
  /// Get segment color by segment ID. If overridden then the override color is returned
  bool GetSegmentColor(std::string segmentID, double* color);
  /// Get segment color by segment ID. If overridden then the override color is returned
  bool GetSegmentColor(std::string segmentID, double &r, double &g, double &b);

  /// Get segment override color by segment ID. Convenience function for python compatibility.
  /// \return Override color if segment found and color overridden, otherwise the invalid override color (-1,-1,-1)
  vtkVector3d GetSegmentOverrideColor(std::string segmentID);
  /// Set segment override color by segment ID
  /// Override color is used for specifying custom color for a segment in selected views.
  /// By default, segment color is invalid (-1,-1,-1), which means that the color stored in vtkSegment object
  /// will be used. If a valid override color is specified then in the views corresponding to this display node,
  /// segment will be colored using the override color.
  void SetSegmentOverrideColor(std::string segmentID, double r, double g, double b);
  /// Set segment override color by segment ID
  void SetSegmentOverrideColor(std::string segmentID, vtkVector3d overrideColor);
  /// Unset segment override color (default segment color will be used for display instead).
  void UnsetSegmentOverrideColor(std::string segmentID);

  /// Get overall segment visibility by segment ID. Convenience function for python compatibility.
  /// \return Segment visibility if segment found, otherwise false
  bool GetSegmentVisibility(std::string segmentID);
  /// Set overall segment visibility by segment ID. Convenience function for python compatibility.
  void SetSegmentVisibility(std::string segmentID, bool visible);
  void SetAllSegmentsVisibility(bool visible);

  /// Get segment 3D visibility by segment ID. Convenience function for python compatibility.
  /// \return Segment 3D visibility if segment found, otherwise false
  bool GetSegmentVisibility3D(std::string segmentID);
  /// Set segment 3D visibility by segment ID. Convenience function for python compatibility.
  void SetSegmentVisibility3D(std::string segmentID, bool visible);
  void SetAllSegmentsVisibility3D(bool visible, bool changeVisibleSegmentsOnly = false);

  /// Get segment 2D fill visibility by segment ID. Convenience function for python compatibility.
  /// \return Segment 2D fill visibility if segment found, otherwise false
  bool GetSegmentVisibility2DFill(std::string segmentID);
  /// Set segment 2D fill visibility by segment ID. Convenience function for python compatibility.
  void SetSegmentVisibility2DFill(std::string segmentID, bool visible);
  void SetAllSegmentsVisibility2DFill(bool visible, bool changeVisibleSegmentsOnly = false);

  /// Get segment 2D outline visibility by segment ID. Convenience function for python compatibility.
  /// \return Segment 2D outline visibility if segment found, otherwise false
  bool GetSegmentVisibility2DOutline(std::string segmentID);
  /// Set segment 2D outline visibility by segment ID. Convenience function for python compatibility.
  void SetSegmentVisibility2DOutline(std::string segmentID, bool visible);
  void SetAllSegmentsVisibility2DOutline(bool visible, bool changeVisibleSegmentsOnly = false);

  /// Get segment 3D opacity by segment ID. Convenience function for python compatibility.
  /// \return Segment 3D opacity if segment found, otherwise false
  double GetSegmentOpacity3D(std::string segmentID);
  /// Set segment 3D opacity by segment ID. Convenience function for python compatibility.
  void SetSegmentOpacity3D(std::string segmentID, double opacity);
  void SetAllSegmentsOpacity3D(double opacity, bool changeVisibleSegmentsOnly = false);

  /// Get segment 2D fill opacity by segment ID. Convenience function for python compatibility.
  /// \return Segment 2D fill opacity if segment found, otherwise false
  double GetSegmentOpacity2DFill(std::string segmentID);
  /// Set segment 2D fill opacity by segment ID. Convenience function for python compatibility.
  void SetSegmentOpacity2DFill(std::string segmentID, double opacity);
  void SetAllSegmentsOpacity2DFill(double opacity, bool changeVisibleSegmentsOnly = false);

  /// Get segment 2D outline opacity by segment ID. Convenience function for python compatibility.
  /// \return Segment 2D outline opacity if segment found, otherwise false
  double GetSegmentOpacity2DOutline(std::string segmentID);
  /// Set segment 2D outline opacity by segment ID. Convenience function for python compatibility.
  void SetSegmentOpacity2DOutline(std::string segmentID, double opacity);
  void SetAllSegmentsOpacity2DOutline(double opacity, bool changeVisibleSegmentsOnly = false);

  /// Set all three types of segment opacity by segment ID
  void SetSegmentOpacity(std::string segmentID, double opacity);
  void SetAllSegmentsOpacity(double opacity, bool changeVisibleSegmentsOnly = false);

  /// Get all visible segment IDs.
  void GetVisibleSegmentIDs(vtkStringArray* segmentIDs);

  /// Get all visible segment IDs.
  void GetVisibleSegmentIDs(std::vector<std::string>& segmentIDs);

protected:
  /// Convenience function for getting all segment IDs.
  void GetSegmentIDs(std::vector<std::string>& segmentIDs, bool visibleSegmentsOnly);

  /// Update list of segment display properties.
  /// Remove entries for missing segments (if removeUnusedDisplayProperties is enabled)
  /// and add missing entries for existing segments.
  void UpdateSegmentList(bool removeUnusedDisplayProperties = true);

protected:
  vtkMRMLSegmentationDisplayNode();
  ~vtkMRMLSegmentationDisplayNode() override;
  vtkMRMLSegmentationDisplayNode(const vtkMRMLSegmentationDisplayNode&);
  void operator=(const vtkMRMLSegmentationDisplayNode&);

  friend class vtkMRMLSegmentationNode; // Access to UpdateSegmentList();

protected:
  /// Name of representation that is displayed in 2D views as outline or filled area
  /// if exists. If does not exist, then master representation is displayed.
  char* PreferredDisplayRepresentationName2D{nullptr};

  /// Name of representation that is displayed as poly data in the 3D view.
  /// If does not exist, then master representation is displayed if poly data,
  /// otherwise the first poly data representation if any.
  char* PreferredDisplayRepresentationName3D{nullptr};

  /// List of segment display properties for all segments in associated segmentation.
  /// Maps segment identifier string (segment name by default) to properties.
  SegmentDisplayPropertiesMap SegmentationDisplayProperties;

  /// Number of segments ever added to the segmentation belonging to this display node.
  /// Used to generate new color for new segments, taken into account removed segments too.
  unsigned int NumberOfGeneratedColors{0};

  /// For checking if cached segment list in SegmentationDisplayProperties has to be updated
  vtkMTimeType SegmentListUpdateTime{0};
  vtkSegmentation* SegmentListUpdateSource{nullptr};

  /// 2D fill visibility for the whole segmentation.
  /// In order for the fill to be visible, \sa Visibility, Visibility2D, and Visibility2DFill
  /// need to be all enabled.
  bool Visibility2DFill{true};
  /// 2D outline visibility for the whole segmentation.
  /// In order for the outline to be visible, \sa Visibility, Visibility2D, and Visibility2DOutline
  /// need to be all enabled.
  bool Visibility2DOutline{true};

  /// 3D opacity for the whole segmentation
  double Opacity3D{1.0};
  /// 2D fill opacity for the whole segmentation
  double Opacity2DFill{0.5};
  /// 2D outline opacity for the whole segmentation
  double Opacity2DOutline{1.0};
};

#endif
