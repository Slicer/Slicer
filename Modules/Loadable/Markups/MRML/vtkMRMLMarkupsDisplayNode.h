/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkMRMLMarkupsDisplayNode - MRML node to represent display properties for markups
// .SECTION Description
// vtkMRMLMarkupsDisplayNode nodes store display properties of markups,
// keeping elements that are applicable to all parts of the markups in this superclass
//

#ifndef __vtkMRMLMarkupsDisplayNode_h
#define __vtkMRMLMarkupsDisplayNode_h

#include "vtkSlicerMarkupsModuleMRMLExport.h"

#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

// vtkAddon includes
#include <vtkAddonSetGet.h>

// STD includes
#include <map>

// VTK include
#include <vtkTextProperty.h>

class vtkMRMLInteractionEventData;
class vtkMRMLProceduralColorNode;

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsDisplayNode : public vtkMRMLDisplayNode
{
public:
  static vtkMRMLMarkupsDisplayNode *New();
  vtkTypeMacro(vtkMRMLMarkupsDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  /// Read node attributes from XML (MRML) file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLMarkupsDisplayNode);

  /// Get node XML tag name (like Volume, Markups)
  const char* GetNodeTagName() override { return "MarkupsDisplay"; };

  /// Finds the storage node and read the data
  void UpdateScene(vtkMRMLScene* scene) override;

  /// Alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents(vtkObject* /*caller*/, unsigned long /*event*/, void* /*callData*/ ) override;

  /// Convenience function for getting the displayable markups node
  vtkMRMLMarkupsNode* GetMarkupsNode();

  /// Get name of the default interaction context (typically the mouse)
  static const std::string GetDefaultContextName() { return ""; };

  /// Active component (that the mouse or other interaction context is hovered over).
  /// This property is computed on-the-fly and saved to file.
  /// \param context Name of the interaction context. By default it is empty string, meaning mouse.
  ///   Additional devices, such as virtual reality controllers can specify additional context names.
  ///   This mechanism allows interacting with multiple markups at the same time (user can grab
  ///   different markup points with each controller at the same time).
  int GetActiveComponentType(std::string context=vtkMRMLMarkupsDisplayNode::GetDefaultContextName());
  enum ComponentType
    {
    ComponentNone = 0,
    ComponentControlPoint,
    ComponentCenterPoint,
    ComponentLine,
    ComponentPlane,
    ComponentRotationHandle,
    ComponentTranslationHandle,
    };
  struct ComponentInfo
    {
    ComponentInfo()
      {
      this->Type = ComponentNone;
      this->Index = -1;
      }
    int Type;
    int Index;
    };

  /// Index of active component (that the mouse or other interaction context is hovered over).
  /// This property is computed on-the-fly and saved to file.
  /// \param context Name of the interaction context. By default it is empty string, meaning mouse
  int GetActiveComponentIndex(std::string context=vtkMRMLMarkupsDisplayNode::GetDefaultContextName());

  /// Set active component type and index for interaction context (empty by default, meaning mouse)
  void SetActiveComponent(int componentType, int componentIndex,
                          std::string context=vtkMRMLMarkupsDisplayNode::GetDefaultContextName());

  /// Query if there is an active component for any interaction context
  bool HasActiveComponent();

  /// Get list of interaction context names that have active components
  /// \return List of interaction context names that have active components
  std::vector<std::string> GetActiveComponentInteractionContexts();

  /// Set active component index to the provided value and component type to ComponentControlPoint.
  void SetActiveControlPoint(int controlPointIndex);

  /// Convenience method to perform several update operations at once, with minimum number of modified events.
  /// It updates the active control point index (if controlPointIndex<0 then it creates a new point) and
  /// updates its position and orientation.
  /// Returns the control point index (different from the input if the input was < 0).
  int UpdateActiveControlPointWorld(int controlPointIndex, vtkMRMLInteractionEventData* eventData,
    double accurateWorldOrientationMatrix[9], const char* viewNodeID,
    const char* associatedNodeID, int positionStatus);

  /// Returns index of active control point for all interaction contexts if active component type is
  /// ComponentControlPoint.
  void GetActiveControlPoints(std::vector<int>& controlPointIndices);
  /// Returns index of active control point for interaction context if active component type is
  /// ComponentControlPoint, -1 otherwise.
  /// \param context Name of the interaction context. By default it is empty string, meaning mouse
  int GetActiveControlPoint(std::string context=vtkMRMLMarkupsDisplayNode::GetDefaultContextName());

  /// Set the text scale of the associated text.
  vtkGetMacro(TextScale,double);
  vtkSetMacro(TextScale,double);

  //@{
  /**
   * Control visibility of control point labels.
   */
  vtkSetMacro(PointLabelsVisibility, bool);
  vtkGetMacro(PointLabelsVisibility, bool);
  vtkBooleanMacro(PointLabelsVisibility, bool);
  //@}

  //@{
  /**
   * Control visibility of information box.
   */
  vtkSetMacro(PropertiesLabelVisibility, bool);
  vtkGetMacro(PropertiesLabelVisibility, bool);
  vtkBooleanMacro(PropertiesLabelVisibility, bool);
  //@}

  //@{
  /**
   * Control visibility of representation fill.
   */
  vtkSetMacro(FillVisibility, bool);
  vtkGetMacro(FillVisibility, bool);
  vtkBooleanMacro(FillVisibility, bool);

  //@{
  /**
   * Control visibility of representation outline.
   */
  vtkSetMacro(OutlineVisibility, bool);
  vtkGetMacro(OutlineVisibility, bool);
  vtkBooleanMacro(OutlineVisibility, bool);

  //@{
  /**
   * Control opacity of representation fill.
   */
  vtkSetMacro(FillOpacity, double);
  vtkGetMacro(FillOpacity, double);
  vtkBooleanMacro(FillOpacity, double);

  //@{
  /**
   * Control opacity of representation edges.
   */
  vtkSetMacro(OutlineOpacity, double);
  vtkGetMacro(OutlineOpacity, double);
  vtkBooleanMacro(OutlineOpacity, double);

  /// Define how points are placed and moved in views
  enum SnapModes
    {
    SnapModeUnconstrained, //< point is moved independently from displayed objects in 3D views (e.g., in parallel with camera plane)
    SnapModeToVisibleSurface, //< point is snapped to any visible surface in 3D views
    //SnapModeToNode //< point is snapped to a specific node, not implemented yet
    SnapMode_Last // insert new items above this line
    };

  /// Which kind of glyph should be used to display this markup?
  /// Vertex2D is supposed to start at 1
  enum GlyphShapes
    {
    GlyphTypeInvalid = 0,
    StarBurst2D,
    Cross2D,
    CrossDot2D,
    ThickCross2D,
    Dash2D,
    Sphere3D,
    Vertex2D,
    Circle2D,
    Triangle2D,
    Square2D,
    Diamond2D,
    Arrow2D,
    ThickArrow2D,
    HookedArrow2D,
    GlyphType_Last // insert new types above this line
    };
  /// Return the min/max glyph types, for iterating over them in tcl
  static int GetMinimumGlyphType() { return 1; };
  static int GetMaximumGlyphType() { return vtkMRMLMarkupsDisplayNode::GlyphType_Last-1; };

  /// The glyph type used to display this fiducial
  vtkSetMacro(GlyphType, int);
  vtkGetMacro(GlyphType, int);
  /// Returns 1 if the type is a 3d one, 0 else
  int GlyphTypeIs3D(int glyphType);
  int GlyphTypeIs3D() { return this->GlyphTypeIs3D(this->GlyphType); };

  /// Return a string representing the glyph type, set it from a string
  const char* GetGlyphTypeAsString();
  void SetGlyphTypeFromString(const char* glyphString);

  static const char* GetGlyphTypeAsString(int g);
  static int GetGlyphTypeFromString(const char*);

  /// Specifies if position of control points are snapped to selected surfaces
  /// \sa SnapModes
  vtkSetMacro(SnapMode, int);
  vtkGetMacro(SnapMode, int);

  static const char* GetSnapModeAsString(int g);
  static int GetSnapModeFromString(const char*);

  /// Get/Set markup point size relative to the window size.
  /// This value is only used in slice views and only if SliceUseGlyphScale is set to true.
  /// Diameter of the point is defined as "scale" percentage of diagonal size of the window.
  vtkSetMacro(GlyphScale,double);
  vtkGetMacro(GlyphScale,double);

  /// Get/Set absolute markup point size.
  /// This value is used in 3D views. This value is used in slice views if SliceUseGlyphScale is set to false.
  /// Diameter of the point is defined as "scale" percentage of diagonal size of the window.
  vtkSetMacro(GlyphSize,double);
  vtkGetMacro(GlyphSize,double);

  /// This flag controls if GlyphScale relative or GlyphSize absolute size is used
  /// to determine size of point glyphs.
  /// On by default (GlyphScale is used for point sizing in 2D views).
  /// \sa SetGlyphScale, SetGlyphSize
  vtkSetMacro(UseGlyphScale, bool);
  vtkGetMacro(UseGlyphScale, bool);
  vtkBooleanMacro(UseGlyphScale, bool);

  enum
    {
    ResetToDefaultsEvent = 19001, //< reset this node to the default values, request completed by markups logic
    JumpToPointEvent, /**< request jump to a selected control point, request completed by markups logic,
                      event data is vtkMRMLInteractionEventData*/
    ActionEvent, /**< default action on the point is requested (mapped to double-click by default),
                 event data is vtkMRMLInteractionEventData */
    };

  /// Set SliceProjection flag that controls if the projection of markups
  /// is visible or not in 2D viewers on slices on which it is normally
  /// not visible.
  /// Off by default
  /// \sa SliceIntersectionVisibilty, SliceProjectionColor
  vtkSetMacro(SliceProjection, bool);
  vtkGetMacro(SliceProjection, bool);
  vtkBooleanMacro(SliceProjection, bool);

  /// Set projection color to be the same as the fiducial color
  /// On by default
  vtkSetMacro(SliceProjectionUseFiducialColor, bool);
  vtkGetMacro(SliceProjectionUseFiducialColor, bool);
  vtkBooleanMacro(SliceProjectionUseFiducialColor, bool);

  /// Set projection's view different if under/over/in the plane
  /// Off by default
  vtkSetMacro(SliceProjectionOutlinedBehindSlicePlane, bool);
  vtkGetMacro(SliceProjectionOutlinedBehindSlicePlane, bool);
  vtkBooleanMacro(SliceProjectionOutlinedBehindSlicePlane, bool);

  /// Set color of the projection on the 2D viewers
  /// White (1.0, 1.0, 1.0) by default.
  vtkSetVector3Macro(SliceProjectionColor, double);
  vtkGetVector3Macro(SliceProjectionColor, double);

  /// Set opacity of projection on the 2D viewers
  /// 0.6 by default
  vtkSetClampMacro(SliceProjectionOpacity, double, 0.0, 1.0);
  vtkGetMacro(SliceProjectionOpacity, double);

  /// Way of determining line radius of markup curves. Default is relative thickness
  /// Current mode is stored in \sa CurveLineSizeMode
  enum CurveLineSizeModes
    {
    UseLineThickness = 0,
    UseLineDiameter,
    CurveLineSizeMode_Last // insert new types above this line
    };

  /// Configure mode of determining line radius of markup curves.
  /// Default is relative thickness. Available modes in \sa CurveLineSizeModes
  vtkSetMacro(CurveLineSizeMode, int);
  vtkGetMacro(CurveLineSizeMode, int);
  const char* GetCurveLineSizeModeAsString();
  void SetCurveLineSizeModeFromString(const char* modeString);
  static const char* GetCurveLineSizeModeAsString(int mode);
  static int GetCurveLineSizeModeFromString(const char*);

  /// Configure line thickness
  /// Thickness is specified relative to markup point size
  /// (1.0 means line diameter is the same as diameter of point glyphs).
  /// This relative value is used if \sa CurveLineSizeMode is UseLineThickness
  /// For absolute control of thickness, \sa LineDiameter should be used.
  vtkGetMacro(LineThickness, double);
  vtkSetMacro(LineThickness, double);

  /// Configure line diameter
  /// Diameter is specified in absolute mm value
  /// This absolute value is used if \sa CurveLineSizeMode is UseLineDiameter
  /// For relative control of diameter, \sa LineThickness should be used.
  vtkGetMacro(LineDiameter, double);
  vtkSetMacro(LineDiameter, double);

  /// Configure the line color fading appearance
  /// Default value = 1.0
  vtkGetMacro(LineColorFadingStart, double);
  vtkSetMacro(LineColorFadingStart, double);

  /// Configure the line color fading appearance
  /// Default value = 10.0
  vtkGetMacro(LineColorFadingEnd, double);
  vtkSetMacro(LineColorFadingEnd, double);

  /// Configures the line color fading appearance
  /// Default value = 1.0
  vtkSetClampMacro(LineColorFadingSaturation, double, 0.0, 1.0);
  vtkGetMacro(LineColorFadingSaturation, double);

  /// Configures the line color fading appearance
  /// Default value = 0.0
  vtkSetClampMacro(LineColorFadingHueOffset, double, 0.0, 1.0);
  vtkGetMacro(LineColorFadingHueOffset, double);

  /// Set the line color node ID used for the projection on the line actors on the 2D viewers.
  /// Setting a line color node allows to define any arbitrary color mapping.
  /// Setting a line color node will overwrite the settings given by the
  /// color, opacity and LineColorFading variables of the displayNode.
  virtual void SetLineColorNodeID(const char *lineColorNodeID);

  /// Get the line color node ID used for the projection on the line actors on the 2D viewers.
  const char* GetLineColorNodeID();

  /// Get the line color node used for the projection on the line actors on the 2D viewers.
  vtkMRMLProceduralColorNode* GetLineColorNode();
  virtual const char* GetLineColorNodeReferenceRole();

  /// Displays the occluded regions of the markup on top of other objects.
  /// Opacity can be adjusted with OccludedOpacity
  /// \sa SetOccludedOpacity, GetOccludedOpacity
  vtkGetMacro(OccludedVisibility, bool);
  vtkSetMacro(OccludedVisibility, bool);
  vtkBooleanMacro(OccludedVisibility, bool);

  /// Opacity of the occluded parts of the markup.
  /// 0.0 results in the markup being fully transparent, while 1.0 is fully opaque.
  /// \sa SetOccludedVisibility, GetOccludedVisibility
  vtkGetMacro(OccludedOpacity, double);
  vtkSetMacro(OccludedOpacity, double);

  /// Text property object that controls the display properties of text actors in 2D and 3D.
  /// The text object property controls background color/opacity, frame size/color, font, etc.
  /// This function should always return a valid vtkTextProperty pointer.
  vtkGetObjectMacro(TextProperty, vtkTextProperty);

  /// The visibility and interactability of the interaction handles
  vtkGetMacro(HandlesInteractive, bool);
  vtkSetMacro(HandlesInteractive, bool);
  vtkBooleanMacro(HandlesInteractive, bool);

protected:
  vtkMRMLMarkupsDisplayNode();
  ~vtkMRMLMarkupsDisplayNode() override;
  vtkMRMLMarkupsDisplayNode( const vtkMRMLMarkupsDisplayNode& );
  void operator= ( const vtkMRMLMarkupsDisplayNode& );

  // Set the text style from a string
  // String format follows html-style conventions
  void SetTextPropertyFromString(std::string textPropertyString);

  // Return a string representing the text style
  // String format follows html-style conventions
  std::string GetTextPropertyAsString();

  // Returns a string containing the text style of the vtkTextProperty
  // String format follows html-style conventions
  static std::string GetTextPropertyAsString(vtkTextProperty* property);

  // Update the style of a vtkTextProperty from a string
  // String format follows html-style conventions
  static void UpdateTextPropertyFromString(std::string inputString, vtkTextProperty* property);

  // Get the color from a string of the form: rgba(0,0,0,0)
  static void GetColorFromString(const std::string& colorString, double color[4]);

  /// Current active point or widget component type and index (hovered by the mouse or other interaction context)
  /// Map interaction context identifier (empty string for mouse) to component type enum
  std::map<std::string, ComponentInfo> ActiveComponents;

  int SnapMode;

  bool PropertiesLabelVisibility;
  bool PointLabelsVisibility;
  bool FillVisibility;
  bool OutlineVisibility;
  double FillOpacity;
  double OutlineOpacity;
  double TextScale;
  int GlyphType;
  double GlyphScale;
  double GlyphSize;
  bool UseGlyphScale;

  bool SliceProjection;
  bool SliceProjectionUseFiducialColor;
  bool SliceProjectionOutlinedBehindSlicePlane;
  double SliceProjectionColor[3];
  double SliceProjectionOpacity;

  virtual const char* GetLineColorNodeReferenceMRMLAttributeName();

  static const char* LineColorNodeReferenceRole;
  static const char* LineColorNodeReferenceMRMLAttributeName;

  int CurveLineSizeMode;
  double LineThickness;
  double LineDiameter;

  double LineColorFadingStart;
  double LineColorFadingEnd;
  double LineColorFadingSaturation;
  double LineColorFadingHueOffset;

  bool OccludedVisibility;
  double OccludedOpacity;

  vtkTextProperty* TextProperty;

  bool HandlesInteractive;
};
#endif
