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

#include "vtkMRMLLabelMapVolumeDisplayNode.h"

#include <set>

class vtkMRMLColorTableNode;
class vtkSegmentation;
class vtkStringArray;
class vtkVector3d;


/// \ingroup Segmentations
/// \brief MRML node for representing segmentation display attributes.
///
/// vtkMRMLSegmentationDisplayNode nodes describe how volume is displayed.
class VTK_MRML_EXPORT vtkMRMLSegmentationDisplayNode : public vtkMRMLLabelMapVolumeDisplayNode
{
public:
  // Define constants
  static const std::string GetColorTableNodeNamePostfix() { return "_ColorTable"; };
  static const std::string GetColorIndexTag() { return "ColorIndex"; };
  static const char* GetSegmentationColorNameBackground() { return "Background"; };
  static const char* GetSegmentationColorNameRemoved() { return "Removed"; };
  static unsigned short GetSegmentationColorIndexBackground() { return 0; };

  /// Display properties per segment
  struct SegmentDisplayProperties
    {
    /// Displayed segment color (may be different than default color stored in segment)
    double Color[3];
    /// Visibility
    bool Visible; // Turns visibility on/off in general. Useful for allowing the user to show/hide a segment without changing any detailed visibility options.
    bool Visible3D;
    bool Visible2DFill; // This one is used for labelmap volume related operations (color table, merged labelmap)
    bool Visible2DOutline;
    /// Opacity
    double Opacity3D;
    double Opacity2DFill; // This one is used for labelmap volume related operations (color table, merged labelmap)
    double Opacity2DOutline;

    // Initialize with default values
    SegmentDisplayProperties()
    : Visible(true)
    , Visible3D(true)
    , Visible2DFill(true)
    , Visible2DOutline(true)
    , Opacity3D(1.0)
    , Opacity2DFill(1.0) // Default is 1, because these are relative values. Half transparency is default for the whole segmentation
    , Opacity2DOutline(1.0)
      {
      Color[0] = 1.0;
      Color[1] = 0.0;
      Color[2] = 0.0;
      }

    // Automatically generated operator= and copy constructor work
    // correctly for these members, so there is no need to define them.
    };

  typedef std::map<std::string, SegmentDisplayProperties> SegmentDisplayPropertiesMap;

public:
  static vtkMRMLSegmentationDisplayNode *New();
  vtkTypeMacro(vtkMRMLSegmentationDisplayNode,vtkMRMLLabelMapVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() { return "SegmentationDisplay"; };

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

  /// Get/Set 3D visibility
  vtkGetMacro(Visibility3D, bool);
  vtkSetMacro(Visibility3D, bool);
  vtkBooleanMacro(Visibility3D, bool);
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

  /// Generate new color for an added segment. Uses \sa NumberOfAddedSegments to get the color
  /// for the new segment from default label color table
  void GenerateSegmentColor(double color[3]);
  /// Python compatibility function for \sa GenerateSegmentColor
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
  /// \return Segment color if segment found, otherwise the pre-defined invalid color
  vtkVector3d GetSegmentColor(std::string segmentID);
  /// Set segment color by segment ID
  void SetSegmentColor(std::string segmentID, double r, double g, double b);
  /// Set segment color by segment ID
  void SetSegmentColor(std::string segmentID, vtkVector3d color);

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

protected:
  /// Convenience function for getting all segment IDs.
  void GetAllSegmentIDs(std::vector<std::string>& segmentIDs, bool visibleSegmentsOnly);

  /// Update list of segment display properties.
  /// Remove entries for missing segments and add missing entries for existing segments
  void UpdateSegmentList();

protected:
  vtkMRMLSegmentationDisplayNode();
  virtual ~vtkMRMLSegmentationDisplayNode();
  vtkMRMLSegmentationDisplayNode(const vtkMRMLSegmentationDisplayNode&);
  void operator=(const vtkMRMLSegmentationDisplayNode&);

protected:
  /// Name of representation that is displayed in 2D views as outline or filled area
  /// if exists. If does not exist, then master representation is displayed.
  char* PreferredDisplayRepresentationName2D;

  /// Name of representation that is displayed as poly data in the 3D view.
  /// If does not exist, then master representation is displayed if poly data,
  /// otherwise the first poly data representation if any.
  char* PreferredDisplayRepresentationName3D;

  /// List of segment display properties for all segments in associated segmentation.
  /// Maps segment identifier string (segment name by default) to properties.
  SegmentDisplayPropertiesMap SegmentationDisplayProperties;

  /// Number of segments ever added to the segmentation belonging to this display node.
  /// Used to generate new color for new segments, taken into account removed segments too.
  unsigned int NumberOfAddedSegments;

  /// For checking if cached segment list in SegmentationDisplayProperties has to be updated
  vtkMTimeType SegmentListUpdateTime;
  vtkSegmentation* SegmentListUpdateSource;

  /// 3D visibility for the whole segmentation
  bool Visibility3D;
  /// 2D fill visibility for the whole segmentation
  bool Visibility2DFill;
  /// 2D outline visibility for the whole segmentation
  bool Visibility2DOutline;

  /// 3D opacity for the whole segmentation
  double Opacity3D;
  /// 2D fill opacity for the whole segmentation
  double Opacity2DFill;
  /// 2D outline opacity for the whole segmentation
  double Opacity2DOutline;
};

#endif
