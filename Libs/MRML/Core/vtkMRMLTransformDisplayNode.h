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

#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkMRMLTransformInteractionDisplayNode.h"

// VTK includes
#include <vtkWeakPointer.h>

// STD includes
#include <map>
#include <string>

class vtkAbstractTransform;
class vtkColorTransferFunction;
class vtkGeneralTransform;
class vtkMRMLTransformNode;

/// \brief MRML node to represent display properties for transforms visualization in the slice and 3D viewers.
///
/// A transform is a vector field: the displacement it applies at each point. This node is
/// therefore a vtkMRMLVectorFieldDisplayNode whose source is the transform, and everything
/// it shows - glyphs, a deformed grid, contours of the displacement magnitude - is drawn by
/// the shared vector field display pipeline.
///
/// The properties that this class adds on top of the shared ones are the ones that are
/// specific to a transform: a separate sampling spacing per visualization mode, and the
/// color map that maps a displacement in mm to a color. Its older accessors are kept as
/// forwarders to the shared properties, so that existing code and saved scenes keep working;
/// see the note on each of them for the property it now sets.
///
/// The interactive editing handles are not part of this node any more: they are stored in a
/// vtkMRMLTransformInteractionDisplayNode of the same transform node, so that a transform
/// has one display node per concern. The editor accessors below forward to it.
///
/// \sa vtkMRMLVectorFieldDisplayNode, vtkVectorFieldSampler, vtkMRMLTransformInteractionDisplayNode
class VTK_MRML_EXPORT vtkMRMLTransformDisplayNode : public vtkMRMLVectorFieldDisplayNode
{
public:
  static vtkMRMLTransformDisplayNode* New();
  vtkTypeMacro(vtkMRMLTransformDisplayNode, vtkMRMLVectorFieldDisplayNode);

  /// Visualization modes, kept for backward compatibility. The values are the same as the
  /// corresponding vtkMRMLVectorFieldDisplayNode::VisualizationModeType values.
  enum VisualizationModes
  {
    VIS_MODE_GLYPH = vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph,
    VIS_MODE_GRID = vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid,
    VIS_MODE_CONTOUR = vtkMRMLVectorFieldDisplayNode::VisualizationModeContour,
    VIS_MODE_LAST // this should be the last mode
  };

  /// Glyph types kept for backward compatibility, with the vtkMRMLVectorFieldDisplayNode::GlyphTypeType values that
  /// GetGlyphType() returns. GLYPH_TYPE_SPHERE is no longer 2, as the shared node has more glyph types in between.
  enum GlyphTypes
  {
    GLYPH_TYPE_ARROW = vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow,
    GLYPH_TYPE_CONE = vtkMRMLVectorFieldDisplayNode::GlyphTypeCone,
    GLYPH_TYPE_SPHERE = vtkMRMLVectorFieldDisplayNode::GlyphTypeSphere,
    GLYPH_TYPE_LAST = vtkMRMLVectorFieldDisplayNode::GlyphType_Last // this should be the last glyph type
  };

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML (MRML) file
  void ReadXMLAttributes(const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Get node XML tag name (like Volume, UnstructuredGrid)
  const char* GetNodeTagName() override { return "TransformDisplayNode"; }

  //--------------------------------------------------------------------------
  /// Vector field source
  //--------------------------------------------------------------------------

  /// The displacement field of the transform, sampled in the region node. Returns nullptr if no region node is set,
  /// as a transform is defined everywhere.
  vtkAlgorithmOutput* GetFieldConnection() override;

  /// Configure the sampler of a slice view to evaluate the transform in its plane.
  bool UpdateSliceFieldSampler(vtkVectorFieldSampler* sampler) override;

  /// A transform can be evaluated at any position.
  bool CanSampleAtArbitraryPositions() override;

  /// A transform is sampled into the same two arrays every time, so there is nothing to
  /// choose between.
  bool HasFixedFieldArrays() override;

  /// Glyph, grid and contour modes; streamlines are not meaningful for a displacement (not velocity) field.
  bool IsVisualizationModeSupported(int visualizationMode) override;

  /// The color map of a transform maps a displacement in mm to a color, so its range is the
  /// scalar range: the colors are used exactly as the map defines them.
  void UpdateScalarRange() override;

  /// Keeps the scalar range in step with the color map, which is what defines it.
  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  //--------------------------------------------------------------------------
  /// Display options
  //--------------------------------------------------------------------------

  /// A node that defines the region of interest where the transform should be
  /// displayed. Same as vtkMRMLVectorFieldDisplayNode::GetSamplingRegionNode().
  vtkMRMLNode* GetRegionNode();
  void SetAndObserveRegionNode(vtkMRMLNode* node);

  /// Node of the glyph starting points: the sampling region (GetSamplingRegionNode()), so setting it replaces the region
  /// node. Get returns the sampling region node only when its points are sampled.
  vtkMRMLNode* GetGlyphPointsNode();
  void SetAndObserveGlyphPointsNode(vtkMRMLNode* node);

  /// Convert visualization mode index to a string for serialization.
  /// Returns an empty string if the index is unknown.
  static const char* ConvertVisualizationModeToString(int modeIndex);
  /// Convert visualization mode string to an index that can be set in VisualizationMode.
  /// Returns -1 if the string is unknown.
  static int ConvertVisualizationModeFromString(const char* modeString);

  ///@{
  /// Distance between the glyphs in mm: the sampling spacing in glyph mode.
  /// \deprecated Use SetSamplingSpacing / GetSamplingSpacing, which every mode uses.
  void SetGlyphSpacingMm(double spacing) { this->SetSamplingSpacing(spacing); }
  double GetGlyphSpacingMm() { return this->GetSamplingSpacing(); }
  ///@}

  ///@{
  /// Size of the glyphs, in percent. Forwards to GlyphScaleFactor (100% is a scale factor of 1).
  void SetGlyphScalePercent(double scalePercent);
  double GetGlyphScalePercent();
  ///@}

  ///@{
  /// Only displacements within this range are shown as glyphs. Forwards to the shared
  /// threshold on the displacement magnitude.
  void SetGlyphDisplayRangeMaxMm(double maxMm);
  double GetGlyphDisplayRangeMaxMm();
  void SetGlyphDisplayRangeMinMm(double minMm);
  double GetGlyphDisplayRangeMinMm();
  ///@}

  /// Convert glyph type index to a string for serialization.
  /// Returns an empty string if the index is unknown.
  static const char* ConvertGlyphTypeToString(int typeIndex);
  /// Convert glyph type string to an index that can be set in GlyphType.
  /// Returns -1 if the string is unknown.
  static int ConvertGlyphTypeFromString(const char* typeString);

  ///@{
  /// Contour levels, in mm. Forwards to the shared contour levels.
  void SetContourLevelsMm(double* levels, int size);
  double* GetContourLevelsMm();
  void GetContourLevelsMm(std::vector<double>& levels);
  std::string GetContourLevelsMmAsString();
  void SetContourLevelsMmFromString(const char* str);
  static std::vector<double> ConvertContourLevelsFromString(const char* str);
  static std::string ConvertContourLevelsToString(const std::vector<double>& levels);
  static bool IsContourLevelEqual(const std::vector<double>& levels1, const std::vector<double>& levels2);
  ///@}

  ///@{
  /// \deprecated Use GlyphDiameter, GridSpacing, GridLineDiameter, and GridScaleFactor (a factor, not percent) of
  /// vtkMRMLVectorFieldDisplayNode. Grid and contour resolutions are the sampling spacing, which every mode uses.
  double GetGlyphDiameterMm() { return this->GetGlyphDiameter(); }
  void SetGlyphDiameterMm(double diameter) { this->SetGlyphDiameter(diameter); }
  double GetGridSpacingMm() { return this->GetGridSpacing(); }
  void SetGridSpacingMm(double spacing) { this->SetGridSpacing(spacing); }
  double GetGridLineDiameterMm() { return this->GetGridLineDiameter(); }
  void SetGridLineDiameterMm(double diameter) { this->SetGridLineDiameter(diameter); }
  double GetGridResolutionMm() { return this->GetSamplingSpacing(); }
  void SetGridResolutionMm(double resolution) { this->SetSamplingSpacing(resolution); }
  double GetContourResolutionMm() { return this->GetSamplingSpacing(); }
  void SetContourResolutionMm(double resolution) { this->SetSamplingSpacing(resolution); }
  double GetGridScalePercent() { return this->GetGridScaleFactor() * 100.0; }
  void SetGridScalePercent(double scalePercent) { this->SetGridScaleFactor(scalePercent * 0.01); }
  ///@}

  //--------------------------------------------------------------------------
  /// Interaction parameters: forwarded to the transform node's vtkMRMLTransformInteractionDisplayNode,
  /// which is created if it does not exist yet.
  //--------------------------------------------------------------------------

  /// Interaction display node of the same transform node, or nullptr if there is none and
  /// createIfMissing is false.
  vtkMRMLTransformInteractionDisplayNode* GetInteractionDisplayNode(bool createIfMissing = false);

  bool GetEditorVisibility();
  void SetEditorVisibility(bool enabled);
  void EditorVisibilityOn() { this->SetEditorVisibility(true); }
  void EditorVisibilityOff() { this->SetEditorVisibility(false); }
  bool GetEditorVisibility3D();
  void SetEditorVisibility3D(bool enabled);
  void EditorVisibility3DOn() { this->SetEditorVisibility3D(true); }
  void EditorVisibility3DOff() { this->SetEditorVisibility3D(false); }
  bool GetEditorSliceIntersectionVisibility();
  void SetEditorSliceIntersectionVisibility(bool enabled);
  void EditorSliceIntersectionVisibilityOn() { this->SetEditorSliceIntersectionVisibility(true); }
  void EditorSliceIntersectionVisibilityOff() { this->SetEditorSliceIntersectionVisibility(false); }
  bool GetEditorTranslationEnabled();
  void SetEditorTranslationEnabled(bool enabled);
  void EditorTranslationEnabledOn() { this->SetEditorTranslationEnabled(true); }
  void EditorTranslationEnabledOff() { this->SetEditorTranslationEnabled(false); }
  bool GetEditorTranslationSliceEnabled();
  void SetEditorTranslationSliceEnabled(bool enabled);
  void EditorTranslationSliceEnabledOn() { this->SetEditorTranslationSliceEnabled(true); }
  void EditorTranslationSliceEnabledOff() { this->SetEditorTranslationSliceEnabled(false); }
  bool GetEditorTranslationSliceAnywhereEnabled();
  void SetEditorTranslationSliceAnywhereEnabled(bool enabled);
  void EditorTranslationSliceAnywhereEnabledOn() { this->SetEditorTranslationSliceAnywhereEnabled(true); }
  void EditorTranslationSliceAnywhereEnabledOff() { this->SetEditorTranslationSliceAnywhereEnabled(false); }
  double GetEditorTranslationSliceAnywhereSensitivity();
  void SetEditorTranslationSliceAnywhereSensitivity(double sensitivity);
  bool GetEditorRotationEnabled();
  void SetEditorRotationEnabled(bool enabled);
  void EditorRotationEnabledOn() { this->SetEditorRotationEnabled(true); }
  void EditorRotationEnabledOff() { this->SetEditorRotationEnabled(false); }
  bool GetEditorRotationSliceEnabled();
  void SetEditorRotationSliceEnabled(bool enabled);
  void EditorRotationSliceEnabledOn() { this->SetEditorRotationSliceEnabled(true); }
  void EditorRotationSliceEnabledOff() { this->SetEditorRotationSliceEnabled(false); }
  bool GetEditorScalingEnabled();
  void SetEditorScalingEnabled(bool enabled);
  void EditorScalingEnabledOn() { this->SetEditorScalingEnabled(true); }
  void EditorScalingEnabledOff() { this->SetEditorScalingEnabled(false); }
  bool GetEditorScalingSliceEnabled();
  void SetEditorScalingSliceEnabled(bool enabled);
  void EditorScalingSliceEnabledOn() { this->SetEditorScalingSliceEnabled(true); }
  void EditorScalingSliceEnabledOff() { this->SetEditorScalingSliceEnabled(false); }

  double GetInteractionSizeMm();
  void SetInteractionSizeMm(double sizeMm);
  double GetInteractionScalePercent();
  void SetInteractionScalePercent(double scalePercent);
  bool GetInteractionSizeAbsolute();
  void SetInteractionSizeAbsolute(bool absolute);
  void InteractionSizeAbsoluteOn() { this->SetInteractionSizeAbsolute(true); }
  void InteractionSizeAbsoluteOff() { this->SetInteractionSizeAbsolute(false); }
  double GetInteractionHandleOpacity();
  void SetInteractionHandleOpacity(double opacity);
  int GetActiveInteractionType();
  void SetActiveInteractionType(int interactionType);
  int GetActiveInteractionIndex();
  void SetActiveInteractionIndex(int index);

  void GetRotationHandleComponentVisibility3D(bool visibility[4]);
  void SetRotationHandleComponentVisibility3D(bool visibility[4]);
  void GetScaleHandleComponentVisibility3D(bool visibility[4]);
  void SetScaleHandleComponentVisibility3D(bool visibility[4]);
  void GetTranslationHandleComponentVisibility3D(bool visibility[4]);
  void SetTranslationHandleComponentVisibility3D(bool visibility[4]);
  void GetRotationHandleComponentVisibilitySlice(bool visibility[4]);
  void SetRotationHandleComponentVisibilitySlice(bool visibility[4]);
  void GetScaleHandleComponentVisibilitySlice(bool visibility[4]);
  void SetScaleHandleComponentVisibilitySlice(bool visibility[4]);
  void GetTranslationHandleComponentVisibilitySlice(bool visibility[4]);
  void SetTranslationHandleComponentVisibilitySlice(bool visibility[4]);

  /// Ask the editor to recompute its bounds by invoking the
  /// TransformUpdateEditorBoundsEvent event.
  void UpdateEditorBounds();
  enum
  {
    TransformUpdateEditorBoundsEvent = vtkMRMLTransformInteractionDisplayNode::TransformUpdateEditorBoundsEvent
  };

  /// Set the default color table
  /// Create and a procedural color node with default colors and use it for visualization.
  void SetDefaultColors();

  vtkColorTransferFunction* GetColorMap();
  void SetColorMap(vtkColorTransferFunction* newColorMap);

  /// Applies the interactive editing settings of a scene that was saved when they were
  /// still stored in this node (see ReadXMLAttributes).
  void UpdateScene(vtkMRMLScene* scene) override;

protected:
  static std::vector<double> StringToDoubleVector(const char* sourceStr);
  static std::string DoubleVectorToString(const double* values, int numberOfValues);

  /// Point a sampler at the transform, in the sampling region of this node.
  void ConfigureTransformSampler(vtkVectorFieldSampler* sampler);

  /// The transform node's transform to world, sampled by the 3D and slice view samplers. Rebuilt only when nodes change
  /// (inversion, reparenting); transform edits reach the samplers through the transforms it concatenates.
  vtkAbstractTransform* GetSampledTransform();
  vtkSmartPointer<vtkGeneralTransform> SampledTransform;
  vtkWeakPointer<vtkMRMLTransformNode> SampledTransformNode;
  vtkMTimeType SampledTransformBuildTime{ 0 };

  /// Move the interaction settings read from a legacy scene to the interaction display node. Does nothing until the
  /// transform node is known, as that is where the interaction display node is found or added.
  void ApplyLegacyInteractionAttributes();

  /// Interaction settings by attribute name, read from a legacy scene. Kept until the interaction display node can be
  /// found: while a scene is read, this node does not know its transform node yet.
  std::map<std::string, std::string> LegacyInteractionAttributes;

protected:
  vtkMRMLTransformDisplayNode();
  ~vtkMRMLTransformDisplayNode() override;
  vtkMRMLTransformDisplayNode(const vtkMRMLTransformDisplayNode&);
  void operator=(const vtkMRMLTransformDisplayNode&);
};

#endif
