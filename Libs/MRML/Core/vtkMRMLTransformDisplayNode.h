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

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#ifndef __vtkMRMLTransformDisplayNode_h
#define __vtkMRMLTransformDisplayNode_h

#include "vtkMRMLDisplayNode.h"

class vtkColorTransferFunction;
class vtkPointSet;
class vtkMatrix4x4;
class vtkMRMLProceduralColorNode;
class vtkMRMLTransformNode;
class vtkMRMLVolumeNode;


/// \brief MRML node to represent display properties for transforms visualization in the slice and 3D viewers.
///
/// vtkMRMLTransformDisplayNode nodes store display properties of transforms.
class VTK_MRML_EXPORT vtkMRMLTransformDisplayNode : public vtkMRMLDisplayNode
{
 public:
  static vtkMRMLTransformDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLTransformDisplayNode,vtkMRMLDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent ) override;

  enum VisualizationModes
    {
    VIS_MODE_GLYPH,
    VIS_MODE_GRID,
    VIS_MODE_CONTOUR,
    VIS_MODE_LAST // this should be the last mode
    };

  enum GlyphTypes
    {
    GLYPH_TYPE_ARROW,
    GLYPH_TYPE_CONE,
    GLYPH_TYPE_SPHERE,
    GLYPH_TYPE_LAST // this should be the last glyph type
    };

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance () override;

  ///
  /// Read node attributes from XML (MRML) file
  void ReadXMLAttributes ( const char** atts ) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML ( ostream& of, int indent ) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLTransformDisplayNode);

  ///
  /// Get node XML tag name (like Volume, UnstructuredGrid)
  const char* GetNodeTagName ( ) override {return "TransformDisplayNode";}

  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  //--------------------------------------------------------------------------
  /// Display options
  //--------------------------------------------------------------------------

  /// A node that defines the region of interest where the transform should be
  /// displayed.
  vtkMRMLNode* GetRegionNode();
  void SetAndObserveRegionNode(vtkMRMLNode* node);

  /// A node that defines glyph starting point positions.
  /// If not set then glyphs positions are arranged evenly in the full region.
  vtkMRMLNode* GetGlyphPointsNode();
  void SetAndObserveGlyphPointsNode(vtkMRMLNode* node);

  vtkSetMacro(VisualizationMode, int);
  vtkGetMacro(VisualizationMode, int);
  /// Convert visualization mode index to a string for serialization.
  /// Returns an empty string if the index is unknown.
  static const char* ConvertVisualizationModeToString(int modeIndex);
  /// Convert visualization mode string to an index that can be set in VisualizationMode.
  /// Returns -1 if the string is unknown.
  static int ConvertVisualizationModeFromString(const char* modeString);

  // Glyph Parameters
  vtkSetMacro(GlyphSpacingMm, double);
  vtkGetMacro(GlyphSpacingMm, double);
  vtkSetMacro(GlyphScalePercent, double);
  vtkGetMacro(GlyphScalePercent, double);
  vtkSetMacro(GlyphDisplayRangeMaxMm, double);
  vtkGetMacro(GlyphDisplayRangeMaxMm, double);
  vtkSetMacro(GlyphDisplayRangeMinMm, double);
  vtkGetMacro(GlyphDisplayRangeMinMm, double);
  vtkSetMacro(GlyphType, int);
  vtkGetMacro(GlyphType, int);
  /// Convert glyph type index to a string for serialization.
  /// Returns an empty string if the index is unknown.
  static const char* ConvertGlyphTypeToString(int typeIndex);
  /// Convert glyph type string to an index that can be set in GlyphType.
  /// Returns -1 if the string is unknown.
  static int ConvertGlyphTypeFromString(const char* typeString);
  // 3d parameters
  vtkSetMacro(GlyphTipLengthPercent, double);
  vtkGetMacro(GlyphTipLengthPercent, double);
  vtkSetMacro(GlyphDiameterMm, double);
  vtkGetMacro(GlyphDiameterMm, double);
  vtkSetMacro(GlyphShaftDiameterPercent, double);
  vtkGetMacro(GlyphShaftDiameterPercent, double);
  vtkSetMacro(GlyphResolution, int);
  vtkGetMacro(GlyphResolution, int);

  // Grid Parameters
  vtkSetMacro(GridScalePercent, double);
  vtkGetMacro(GridScalePercent, double);
  vtkSetMacro(GridSpacingMm, double);
  vtkGetMacro(GridSpacingMm, double);
  vtkSetMacro(GridLineDiameterMm, double);
  vtkGetMacro(GridLineDiameterMm, double);
  vtkSetMacro(GridResolutionMm, double);
  vtkGetMacro(GridResolutionMm, double);
  vtkSetMacro(GridShowNonWarped, bool);
  vtkGetMacro(GridShowNonWarped, bool);

  // Contour Parameters
  unsigned int GetNumberOfContourLevels();
  void SetContourLevelsMm(double*, int size);
  double* GetContourLevelsMm();
  void GetContourLevelsMm(std::vector<double> &levels);
  std::string GetContourLevelsMmAsString();
  void SetContourLevelsMmFromString(const char* str);
  static std::vector<double> ConvertContourLevelsFromString(const char* str);
  static std::string ConvertContourLevelsToString(const std::vector<double>& levels);
  static bool IsContourLevelEqual(const std::vector<double>& levels1, const std::vector<double>& levels2);

  vtkSetMacro(ContourResolutionMm, double);
  vtkGetMacro(ContourResolutionMm, double);
  vtkSetMacro(ContourOpacity, double);
  vtkGetMacro(ContourOpacity, double);

  // Interaction Parameters
  vtkGetMacro(EditorVisibility, bool);
  vtkSetMacro(EditorVisibility, bool);
  vtkBooleanMacro(EditorVisibility, bool);
  vtkGetMacro(EditorSliceIntersectionVisibility, bool);
  vtkSetMacro(EditorSliceIntersectionVisibility, bool);
  vtkBooleanMacro(EditorSliceIntersectionVisibility, bool);
  vtkGetMacro(EditorTranslationEnabled, bool);
  vtkSetMacro(EditorTranslationEnabled, bool);
  vtkBooleanMacro(EditorTranslationEnabled, bool);
  vtkGetMacro(EditorRotationEnabled, bool);
  vtkSetMacro(EditorRotationEnabled, bool);
  vtkBooleanMacro(EditorRotationEnabled, bool);
  vtkGetMacro(EditorScalingEnabled, bool);
  vtkSetMacro(EditorScalingEnabled, bool);
  vtkBooleanMacro(EditorScalingEnabled, bool);

  /// Ask the editor to recompute its bounds by invoking the
  /// TransformUpdateEditorBoundsEvent event.
  void UpdateEditorBounds();
  enum
    {
    TransformUpdateEditorBoundsEvent = 2750
    };

  /// Set the default color table
  /// Create and a procedural color node with default colors and use it for visualization.
  void SetDefaultColors();

  vtkColorTransferFunction* GetColorMap();
  void SetColorMap(vtkColorTransferFunction* newColorMap);

protected:

  static std::vector<double> StringToDoubleVector(const char* sourceStr);
  static std::string DoubleVectorToString(const double* values, int numberOfValues);

  int VisualizationMode;

  // Glyph Parameters
  double GlyphSpacingMm;
  double GlyphScalePercent;
  double GlyphDisplayRangeMaxMm;
  double GlyphDisplayRangeMinMm;
  int GlyphType;
  // 3d parameters
  double GlyphTipLengthPercent;
  double GlyphDiameterMm;
  double GlyphShaftDiameterPercent;
  int GlyphResolution;

  // Grid Parameters
  double GridScalePercent;
  double GridSpacingMm;
  double GridLineDiameterMm;
  /// Determines how densely the grid is sampled. Higher value results in more faithful representation of the
  /// deformed lines, but needs more computation time.
  double GridResolutionMm;
  /// If true then the non-warped original grid will be also shown. Only used in the slice view, as in the
  /// 3D view it would make the visualization very cluttered.
  bool GridShowNonWarped;

  // Contour Parameters
  double ContourResolutionMm;
  /// Opacity of the 3D contour. Between 0 and 1.
  double ContourOpacity;
  std::vector<double> ContourLevelsMm;

  // Interaction Parameters
  bool EditorVisibility;
  bool EditorSliceIntersectionVisibility;
  bool EditorTranslationEnabled;
  bool EditorRotationEnabled;
  bool EditorScalingEnabled;

 protected:
  vtkMRMLTransformDisplayNode ( );
  ~vtkMRMLTransformDisplayNode ( ) override;
  vtkMRMLTransformDisplayNode ( const vtkMRMLTransformDisplayNode& );
  void operator= ( const vtkMRMLTransformDisplayNode& );

};

#endif
