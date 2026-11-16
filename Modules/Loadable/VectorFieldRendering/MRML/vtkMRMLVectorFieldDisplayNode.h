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

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __vtkMRMLVectorFieldDisplayNode_h
#define __vtkMRMLVectorFieldDisplayNode_h

// MRML includes
#include "vtkMRMLDisplayNode.h"
#include "vtkSlicerVectorFieldRenderingModuleMRMLExport.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <string>
#include <vector>

class vtkDataArray;
class vtkDataSet;
class vtkIntArray;

/// \brief Display properties of a vector field: glyphs, deformed grid, magnitude contours or streamlines.
///
/// Can be added to a model (3-component point or cell data array; cell data is drawn at cell centers), a vector volume,
/// or a transform (its displacement). A volumetric mesh is interpolated inside its cells; a surface mesh or point cloud
/// only has values at its points, unless InterpolateBetweenPoints fits a smooth B-spline that can be sampled anywhere
/// near them. A transform is defined everywhere: 3D views draw it in the sampling region, slice views in their plane.
/// A node can have any number of vector field display nodes, each drawn on its own.
///
/// The filters that draw the field are in vtkMRMLVectorFieldPipeline, one for each display node, which the views and
/// the GUI share (vtkMRMLVectorFieldPipeline::GetSharedPipeline()); this node only holds the settings, and the output
/// of the field source of the pipeline (GetSampledFieldDataSet()). The field is sampled when the renderer pulls the
/// data, not when a node changes.
///
/// \sa vtkVectorFieldSampler
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_MRML_EXPORT vtkMRMLVectorFieldDisplayNode : public vtkMRMLDisplayNode
{
public:
  static vtkMRMLVectorFieldDisplayNode* New();
  vtkTypeMacro(vtkMRMLVectorFieldDisplayNode, vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Shape of the glyph geometry used to represent each point.
  enum GlyphTypeType
  {
    GlyphTypeArrow = 0,
    GlyphTypeCone,
    GlyphTypeBox,
    GlyphTypeCylinder,
    GlyphTypeLine,
    GlyphTypeSphere,
    GlyphType_Last // placeholder after the last valid value, this must be the last in the list of modes
  };

  /// How the vector field is drawn.
  enum VisualizationModeType
  {
    VisualizationModeGlyph = 0,  ///< a glyph at each sampled point, oriented and scaled by the vector
    VisualizationModeGrid,       ///< a lattice of lines, deformed by the vectors
    VisualizationModeContour,    ///< isosurfaces (isolines in slice views) of the vector magnitude
    VisualizationModeStreamline, ///< curves that follow the direction of the field
    VisualizationMode_Last       // placeholder after the last valid value, this must be the last in the list of modes
  };

  /// Which part of the sampling region the samples are taken from.
  enum SamplingRegionPartType
  {
    SamplingRegionPartDefault = 0, ///< the part that suits the node, see GetEffectiveSamplingRegionPart()
    SamplingRegionPartPoints,      ///< its points: control points, steps along a curve, mesh points, voxel centers
    SamplingRegionPartSurface,     ///< its surface: a plane, a slice, the surface of a model or of segments
    SamplingRegionPartVolume,      ///< its inside: a box, a closed model, segments, the extent of a volume
    SamplingRegionPart_Last        // placeholder after the last valid value, this must be the last in the list
  };

  /// How the samples are spread over the part of the sampling region.
  enum SamplingPatternType
  {
    SamplingPatternRegular = 0, ///< every point; points of a mesh surface about, a lattice over a plane or volume or steps along a curve exactly, SamplingSpacing apart
    SamplingPatternRandom,      ///< SamplingPointsNumber samples, spread uniformly at random
    SamplingPattern_Last        // placeholder after the last valid value, this must be the last in the list
  };

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "VectorFieldDisplay"; }

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLVectorFieldDisplayNode);

  /// Alternative method to propagate events generated in the referenced nodes
  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  ///@{
  /// Glyph mode: shape of the glyphs in 3D views; slice views draw a flat outline of it.
  /// Default is GlyphTypeArrow.
  vtkGetMacro(GlyphType, int);
  vtkSetMacro(GlyphType, int);
  const char* GetGlyphTypeAsString();
  void SetGlyphTypeFromString(const char* modeString);
  static const char* GetGlyphTypeAsString(int id);
  static int GetGlyphTypeFromString(const char* modeString);
  ///@}

  ///@{
  /// Glyph mode: 3-component array that orients the glyphs, and scales them if no scale array is set.
  /// Follows FieldArrayName unless set otherwise; nullptr means the field array (see GetModeVectorArrayName()).
  vtkGetStringMacro(GlyphOrientationArrayName);
  vtkSetStringMacro(GlyphOrientationArrayName);
  ///@}

  ///@{
  /// Glyph mode: array that scales the glyphs; follows FieldArrayName unless set otherwise, nullptr means the
  /// orientation array. Only used for glyphs at mesh points; a sampled field is scaled by its vectors.
  vtkGetStringMacro(GlyphScaleArrayName);
  vtkSetStringMacro(GlyphScaleArrayName);
  ///@}

  ///@{
  /// Glyph mode: scale each glyph axis by a component of a 3-component scale array (true), or the whole glyph
  /// by the vector magnitude (false). Default is false.
  vtkGetMacro(GlyphScaleByComponents, bool);
  vtkSetMacro(GlyphScaleByComponents, bool);
  vtkBooleanMacro(GlyphScaleByComponents, bool);
  ///@}

  ///@{
  /// Glyph mode: glyph length multiplier; negative flips the glyphs. Grid mode uses GridScaleFactor instead.
  /// Default is 10; SetFieldArrayName() sets it to a readable size (see ComputeDefaultGlyphScaleFactor()).
  vtkGetMacro(GlyphScaleFactor, double);
  vtkSetMacro(GlyphScaleFactor, double);
  ///@}

  ///@{
  /// Array the field is read from; grid, contour and streamline modes use it. The glyph orientation, scale and
  /// color arrays follow it unless set otherwise. Empty by default, which means the source decides.
  vtkGetStringMacro(FieldArrayName);
  void SetFieldArrayName(const char* arrayName);
  ///@}

  /// Array the vectors are read from in the current mode: the glyph orientation array in glyph mode, the field
  /// array otherwise. Falls back to the other one if the one the mode uses is not set.
  const char* GetModeVectorArrayName();

  /// Mesh array the field is colored by: the active scalar array if it is a mesh array other than the vectors.
  /// Samplers carry it as vtkVectorFieldSampler::GetColorArrayName(). nullptr if colored by the vectors.
  const char* GetSampledColorArrayName();

  /// Scale factor at which the longest vector spans about a twentieth of the sampled region.
  /// Returns 0 if it cannot be computed, such as before anything has been sampled.
  double ComputeDefaultGlyphScaleFactor();

  ///@{
  /// Scale of the field in the current mode: glyph length in glyph mode, grid deformation in grid mode
  /// (1 is the field's own size). Other modes keep the glyph scale factor.
  double GetEffectiveScaleFactor();
  void SetEffectiveScaleFactor(double scaleFactor);
  /// True if the visualization mode has something to scale.
  bool IsScaleFactorUsed();
  ///@}

  /// True if the visualization mode can hide parts of the field by their magnitude.
  bool IsThresholdUsed();

  /// Magnitudes where contour mode draws isosurfaces: ContourLevels, or if empty, levels spread over the
  /// scalar range, so that a field of unknown magnitudes still shows something.
  void GetEffectiveContourLevels(std::vector<double>& levels);

  /// True if what is drawn comes from a sampler rather than directly from the source; then the arrays
  /// are the sampler's own, whatever the source arrays are called. Decided from the settings and the source.
  bool IsFieldSampled();

  ///@{
  /// Names of the arrays in what is drawn: the chosen arrays when the source is drawn directly, the sampler's
  /// arrays when sampled (also for a mesh read through a sampler, whose own arrays are not in the output).
  const char* GetRenderedGlyphOrientationArrayName();
  const char* GetRenderedGlyphScaleArrayName();
  const char* GetRenderedActiveScalarName();
  /// What the field source of the pipeline produces (see vtkMRMLVectorFieldPipeline::GetFieldConnection()), as a
  /// render has last filled it, or nullptr before a pipeline has been set up. Kept after the pipeline is deleted.
  vtkDataSet* GetSampledFieldDataSet();
  /// The array that GetRenderedActiveScalarName() names, in what is drawn.
  vtkDataArray* GetRenderedActiveScalarArray();
  /// Component of the rendered active scalar array that colors and threshold use, -1 for the magnitude
  /// (see vtkMRMLDisplayNode::GetEffectiveScalarComponent()). Known before anything is sampled.
  int GetRenderedActiveScalarComponent();
  /// True if the rendered active scalar array has multiple components, so colors and threshold use
  /// GetRenderedActiveScalarComponent(). Known before anything is sampled.
  bool IsRenderedActiveScalarMultiComponent();

  /// A field that is sampled and colored by its own vectors offers their 3 components,
  /// whichever array names them (the magnitude, by default).
  int GetActiveScalarNumberOfComponents() override;
  /// The array that GetRenderedGlyphScaleArrayName() names, in what is drawn.
  vtkDataArray* GetRenderedGlyphScaleArray();
  ///@}

  /// True if the part can be sampled in the sampling region node, or in the source if no node is set
  /// (see vtkMRMLVectorFieldSamplePointSource::IsSamplingPartSupported()).
  bool IsSamplingRegionPartSupported(int part);

  /// True if the sampling region, part and pattern affect the current mode. Not for isosurfaces of a volumetric
  /// mesh, built on its cells (see vtkMRMLVectorFieldSamplePointSource::NeedsLatticeSampling()); masking still applies.
  bool IsSamplingRegionUsed();

  /// The sampled part: SamplingRegionPart if supported, otherwise the part that suits the node
  /// (see vtkMRMLVectorFieldSamplePointSource::GetEffectiveSamplingPart()).
  int GetEffectiveSamplingRegionPart();

  /// True if the source can be drawn the way the visualization mode asks for. Reimplemented
  /// by nodes whose source does not offer every mode (a transform has no streamlines).
  bool IsVisualizationModeSupported(int visualizationMode);

  ///@{
  /// Number of samples with the random pattern. Inside a segmentation or model, samples are spread over its
  /// bounding box and only those inside are kept, so fewer are drawn. Default is 5000.
  vtkGetMacro(SamplingPointsNumber, int);
  vtkSetClampMacro(SamplingPointsNumber, int, 1, VTK_INT_MAX);
  ///@}

  ///@{
  /// How the vector field is drawn.
  /// Default is VisualizationModeGlyph.
  vtkGetMacro(VisualizationMode, int);
  vtkSetMacro(VisualizationMode, int);
  const char* GetVisualizationModeAsString();
  void SetVisualizationModeFromString(const char* modeString);
  static const char* GetVisualizationModeAsString(int id);
  static int GetVisualizationModeFromString(const char* modeString);
  ///@}

  ///@{
  /// Scale glyphs along their own axis only, so their thickness (GlyphDiameter) does not grow with length.
  /// Used in 3D views for arrow, cone and cylinder with GlyphDiameterAbsolute. Default is true.
  vtkGetMacro(GlyphScaleDirectional, bool);
  vtkSetMacro(GlyphScaleDirectional, bool);
  vtkBooleanMacro(GlyphScaleDirectional, bool);
  ///@}

  ///@{
  /// Glyph thickness is in mm, the same for every glyph (true), or a percentage of the glyph length (false).
  /// Default is false (true for transforms).
  vtkGetMacro(GlyphDiameterAbsolute, bool);
  vtkSetMacro(GlyphDiameterAbsolute, bool);
  vtkBooleanMacro(GlyphDiameterAbsolute, bool);
  ///@}

  ///@{
  /// Glyph geometry: the thickness of the glyphs, in percent of their length.
  /// Only used when GlyphDiameterAbsolute is false. Default is 20.
  vtkGetMacro(GlyphDiameterPercent, double);
  vtkSetMacro(GlyphDiameterPercent, double);
  ///@}

  ///@{
  /// Thickness of the glyphs, in mm. Only used when GlyphDiameterAbsolute is true.
  /// Default is 5.
  vtkGetMacro(GlyphDiameter, double);
  vtkSetClampMacro(GlyphDiameter, double, 0.0, VTK_DOUBLE_MAX);
  ///@}

  ///@{
  /// Grid mode: distance between grid lines, in mm; 0 draws a line through every sampled point. A finer
  /// sampling spacing shows how the lines curve between the sampled points. Default is 3.
  vtkGetMacro(GridSpacing, double);
  vtkSetClampMacro(GridSpacing, double, 0.0, VTK_DOUBLE_MAX);
  ///@}

  ///@{
  /// Grid mode: also draw the undeformed grid, in GridNonWarpedColor.
  /// Default is false.
  vtkGetMacro(GridShowNonWarped, bool);
  vtkSetMacro(GridShowNonWarped, bool);
  vtkBooleanMacro(GridShowNonWarped, bool);
  ///@}

  ///@{
  /// Grid mode: color of the undeformed grid, whichever way the field is colored.
  /// Default is dark gray, the color of zero displacement in the default color map of transforms.
  vtkGetVector3Macro(GridNonWarpedColor, double);
  vtkSetVector3Macro(GridNonWarpedColor, double);
  ///@}

  ///@{
  /// Grid mode: draw the deformed grid lines; turn off to show only GridFill or GridShowNonWarped.
  /// Default is true.
  vtkGetMacro(GridLinesVisibility, bool);
  vtkSetMacro(GridLinesVisibility, bool);
  vtkBooleanMacro(GridLinesVisibility, bool);
  ///@}

  ///@{
  /// Grid mode: also fill the deformed, colored surface of the sampled lattice: a color map over a plane or slice,
  /// the outer surface over a volume region. In slice views it fills the slice plane. Default is false.
  vtkGetMacro(GridFill, bool);
  vtkSetMacro(GridFill, bool);
  vtkBooleanMacro(GridFill, bool);
  ///@}

  ///@{
  /// Contour mode: isosurface opacity in 3D views (0-1), applied on top of Opacity so nested isosurfaces are
  /// visible by default. Slice views draw isolines with SliceIntersectionOpacity. Default is 0.8.
  vtkGetMacro(ContourOpacity3D, double);
  vtkSetClampMacro(ContourOpacity3D, double, 0.0, 1.0);
  ///@}

  /// True if the sampling spacing affects the current mode: not where node points are used as they are,
  /// for random samples, or where nothing is sampled (see IsSamplingRegionUsed()).
  bool IsSamplingSpacingUsed();

  /// True if glyphs go on the surface of the mesh that carries the field, which regularly means about SamplingSpacing
  /// apart, instead of on another sampling region or on all the points of the mesh.
  bool IsSamplingOwnSurfaceSpaced();

  /// Spacing the field is actually sampled at. In grid mode it is refined to the next spacing that evenly divides
  /// GridSpacing, so the drawn grid spacing does not jump as the requested spacing changes.
  double GetSamplingSpacingForField();

  /// Number of sampling steps between two grid lines at GetSamplingSpacingForField(),
  /// so that grid lines land exactly GridSpacing apart.
  int GetGridSubdivision();

  ///@{
  /// Grid mode: grid deformation relative to the vectors; negative deforms the opposite way (approximately the
  /// inverse displacement). GlyphScaleFactor is not used in grid mode. Default is 1.
  vtkGetMacro(GridScaleFactor, double);
  vtkSetMacro(GridScaleFactor, double);
  ///@}

  ///@{
  /// Grid mode: diameter of the grid line tubes in 3D views, in mm; 0 draws thin lines, as slice views always do.
  /// Default is 0.5.
  vtkGetMacro(GridLineDiameter, double);
  vtkSetClampMacro(GridLineDiameter, double, 0.0, VTK_DOUBLE_MAX);
  ///@}

  ///@{
  /// Contour mode: magnitudes of the isosurfaces (isolines in slice views), in field units (mm for displacements).
  /// Empty by default: 10, 25, 50, 75, 90% of the scalar range are used. Transforms default to 2, 4, ..., 18 mm.
  unsigned int GetNumberOfContourLevels();
  void SetContourLevels(double* levels, int size);
  double* GetContourLevels() VTK_SIZEHINT(GetNumberOfContourLevels());
  void GetContourLevels(std::vector<double>& levels);
  std::string GetContourLevelsAsString();
  void SetContourLevelsAsString(const std::string& levelsString);
  ///@}

  ///@{
  /// Streamline mode: follow streamlines both ways from the seed, or only along the field direction.
  /// Default is true.
  vtkGetMacro(StreamlineBidirectional, bool);
  vtkSetMacro(StreamlineBidirectional, bool);
  vtkBooleanMacro(StreamlineBidirectional, bool);
  ///@}

  ///@{
  /// Streamline mode: initial integration step, in mm; smaller follows sharp turns more closely but is slower.
  /// Default is 0.1.
  vtkGetMacro(StreamlineInitialIntegrationStep, double);
  vtkSetClampMacro(StreamlineInitialIntegrationStep, double, 0.0, VTK_DOUBLE_MAX);
  ///@}

  ///@{
  /// Streamline mode: maximum length of a streamline, in mm.
  /// Default is 100.
  vtkGetMacro(StreamlineMaximumPropagation, double);
  vtkSetClampMacro(StreamlineMaximumPropagation, double, 0.0, VTK_DOUBLE_MAX);
  ///@}

  ///@{
  /// Streamline mode: slice views draw whole streamlines that cross the slice, fading out with distance; this is
  /// the distance in mm where they fully fade. 0 shows only the part within the slice thickness. Default is 20.
  vtkGetMacro(StreamlineSliceFadeDistance, double);
  vtkSetClampMacro(StreamlineSliceFadeDistance, double, 0.0, VTK_DOUBLE_MAX);
  ///@}

  ///@{
  /// Streamline mode: diameter of the streamline tubes, in mm. 0 draws them as thin lines.
  /// Default is 0.5.
  vtkGetMacro(StreamlineTubeDiameter, double);
  vtkSetClampMacro(StreamlineTubeDiameter, double, 0.0, VTK_DOUBLE_MAX);
  ///@}

  ///@{
  /// Distance between samples in mm, in every mode: glyph/grid/isosurface lattice, curve steps, streamline seeds.
  /// 0 means the source decides (e.g. voxel size). Refined in grid mode (GetSamplingSpacingForField()). Default is 5.
  vtkGetMacro(SamplingSpacing, double);
  vtkSetClampMacro(SamplingSpacing, double, 0.0, VTK_DOUBLE_MAX);
  ///@}

  ///@{
  /// Slice views: project vectors onto the slice plane, so glyph length shows the in-plane component.
  /// Coloring still uses the original magnitude. Default is true.
  vtkGetMacro(GlyphSliceProjectionEnabled, bool);
  vtkSetMacro(GlyphSliceProjectionEnabled, bool);
  vtkBooleanMacro(GlyphSliceProjectionEnabled, bool);
  ///@}

  ///@{
  /// Number of subdivisions of the 3D glyph geometry (e.g. sides of an arrow's shaft and tip); higher is smoother.
  /// Default is 16.
  vtkGetMacro(GlyphResolution, int);
  vtkSetClampMacro(GlyphResolution, int, 3, VTK_INT_MAX);
  ///@}

  ///@{
  /// Length of the tip of arrow glyphs in 3D views, in percent of the glyph length.
  /// Default is 35.
  vtkGetMacro(GlyphTipLengthPercent, double);
  vtkSetClampMacro(GlyphTipLengthPercent, double, 0.0, 100.0);
  ///@}

  ///@{
  /// Diameter of an arrow glyph's shaft, in percent of the diameter of its tip.
  /// Default is 30.
  vtkGetMacro(GlyphShaftDiameterPercent, double);
  vtkSetClampMacro(GlyphShaftDiameterPercent, double, 0.0, 100.0);
  ///@}

  ///@{
  /// Number of subdivisions of the flat glyphs in slice views (e.g. sides of a circle).
  /// Default is 12.
  vtkGetMacro(GlyphResolution2D, int);
  vtkSetClampMacro(GlyphResolution2D, int, 3, VTK_INT_MAX);
  ///@}

  ///@{
  /// Length of the tip of arrow glyphs in slice views, in percent of the glyph length.
  /// Default is 30.
  vtkGetMacro(GlyphTipLengthPercent2D, double);
  vtkSetClampMacro(GlyphTipLengthPercent2D, double, 0.0, 100.0);
  ///@}

  ///@{
  /// Only show the field where the active scalar value is in ThresholdRange: glyphs are hidden elsewhere, and
  /// streamlines are only integrated there, as in the masking region (they stop where the value leaves the range).
  /// Default is off.
  vtkGetMacro(ThresholdEnabled, bool);
  vtkSetMacro(ThresholdEnabled, bool);
  vtkBooleanMacro(ThresholdEnabled, bool);
  ///@}

  ///@{
  /// Range of active scalar values for which glyphs are shown and streamlines are integrated.
  /// Only used if ThresholdEnabled is true.
  vtkSetVector2Macro(ThresholdRange, double);
  vtkGetVector2Macro(ThresholdRange, double);
  ///@}

  //--------------------------------------------------------------------------
  /// Vector field source: hides from displayable managers and the GUI where the field is stored. Methods only
  /// return pipeline objects and metadata; VTK samples the field when the renderer pulls the data.
  //--------------------------------------------------------------------------

  ///@{
  /// Node the samples are taken from (glyph positions, streamline seeds, grid/isosurface lattice): markups, slice,
  /// model, segmentation or volume. If not set or the field's node, the source is sampled; transforms then show nothing in 3D.
  vtkMRMLNode* GetSamplingRegionNode();
  void SetAndObserveSamplingRegionNode(vtkMRMLNode* node);
  void SetAndObserveSamplingRegionNodeID(const char* nodeID);
  ///@}

  /// The sampling region node, or nullptr if not set or it is the field's node (the source itself is sampled).
  vtkMRMLNode* GetSamplingRegionNodeOtherThanSource();

  ///@{
  /// Which part of the sampling region the samples are taken from.
  /// Default is SamplingRegionPartDefault, which takes the part that suits the node.
  vtkGetMacro(SamplingRegionPart, int);
  vtkSetMacro(SamplingRegionPart, int);
  const char* GetSamplingRegionPartAsString();
  void SetSamplingRegionPartFromString(const char* partString);
  static const char* GetSamplingRegionPartAsString(int id);
  static int GetSamplingRegionPartFromString(const char* partString);
  ///@}

  ///@{
  /// How samples are spread over the sampling region part: regularly at SamplingSpacing, or SamplingPointsNumber
  /// at random. Grid and contour modes always use a lattice. Default is SamplingPatternRandom.
  vtkGetMacro(SamplingPattern, int);
  vtkSetMacro(SamplingPattern, int);
  const char* GetSamplingPatternAsString();
  void SetSamplingPatternFromString(const char* patternString);
  static const char* GetSamplingPatternAsString(int id);
  static int GetSamplingPatternFromString(const char* patternString);
  ///@}

  ///@{
  /// Node that limits where the field is shown in every mode and view: ROI, closed model, segmentation or volume
  /// extent. Glyphs outside are hidden; grid lines, isosurfaces and streamlines are cut. Not set by default.
  vtkMRMLNode* GetMaskingRegionNode();
  void SetAndObserveMaskingRegionNode(vtkMRMLNode* node);
  void SetAndObserveMaskingRegionNodeID(const char* nodeID);
  ///@}

  ///@{
  /// Segments of a segmentation masking region that the field is shown in (or outside of).
  /// Default is empty, which means all segments.
  std::vector<std::string> GetMaskingSegmentIDs();
  void SetMaskingSegmentIDs(const std::vector<std::string>& segmentIDs);
  ///@}

  ///@{
  /// Show the field inside of the masking region (true) or outside of it (false).
  /// Default is true.
  vtkGetMacro(MaskingShowInside, bool);
  vtkSetMacro(MaskingShowInside, bool);
  vtkBooleanMacro(MaskingShowInside, bool);
  ///@}

  /// True if a masking region is set, or the inside of a segmentation or model is sampled. The field connection
  /// then has the vtkMRMLVectorFieldMask::GetInsideArrayName() array that says where the field is shown.
  bool HasRegionTest();

  ///@{
  /// Display node whose coloring (scalar visibility, array, color table, range) is copied into this node when linked
  /// and whenever it changes, overriding local edits. Removing the link keeps the last copied values. Not set by default.
  vtkMRMLDisplayNode* GetColoringSourceDisplayNode();
  void SetAndObserveColoringSourceDisplayNode(vtkMRMLDisplayNode* node);
  ///@}

  /// True if the current visualization mode draws poly data instead of glyphs.
  bool IsPolyDataVisualizationMode();

  /// True if the field can be sampled at any position, so sampling spacing and region apply.
  /// False for sources with values only at fixed locations, such as mesh point data.
  bool CanSampleAtArbitraryPositions();

  /// True if the source always provides the same arrays (a vector volume or transform: vectors and magnitude),
  /// so there is nothing for the user to pick. False for a mesh.
  bool HasFixedFieldArrays();

  /// True if the model's mesh has cells that enclose a volume, so the field at its points can be interpolated
  /// inside them. A surface mesh has values only at its points.
  bool HasVolumetricMesh();

  /// True if GetModeVectorArrayName() is a cell data array of the model's mesh (drawn at cell centers).
  /// A point data array of the same name takes precedence.
  bool IsFieldArrayCellData();

  ///@{
  /// Fit a smooth, approximating field (vtkMRMLVectorFieldPipeline::GetInterpolationResidual()) to mesh point vectors so it
  /// can be sampled within InterpolationMaximumDistance, also beyond the cells of a volumetric mesh. Default is false.
  vtkGetMacro(InterpolateBetweenPoints, bool);
  vtkSetMacro(InterpolateBetweenPoints, bool);
  vtkBooleanMacro(InterpolateBetweenPoints, bool);
  ///@}

  ///@{
  /// Interpolation between points: samples farther than this (mm) from the nearest data point are not drawn.
  /// 0 means automatic: a tenth of the diagonal of the data point bounds. Default is 0.
  vtkGetMacro(InterpolationMaximumDistance, double);
  vtkSetClampMacro(InterpolationMaximumDistance, double, 0.0, VTK_DOUBLE_MAX);
  ///@}

  /// True if the source can be interpolated between its points (a model whose mesh has points), so
  /// InterpolateBetweenPoints applies and the GUI offers it.
  bool CanInterpolateBetweenPoints();

  /// True if the field is drawn from a fit to the vectors at the points: interpolation is
  /// enabled and the source can be interpolated between its points.
  bool IsInterpolationUsed();

  /// Get the name and number of components of each array usable for glyph orientation, scaling and coloring.
  /// Does not update the pipeline, so it is safe to call from the GUI.
  void GetFieldArrayInfo(std::vector<std::string>& arrayNames, std::vector<int>& numberOfComponents);

  /// Data set with the arrays usable for glyph orientation, scaling and coloring: the mesh of a model, otherwise
  /// the last sampled field (GetSampledFieldDataSet()), nullptr before the first render. Does not update.
  vtkDataSet* GetScalarDataSet() override;

  /// Return the active scalar array (used for coloring and thresholding the glyphs)
  /// from the associated model node's mesh. Returns nullptr if not found or not set.
  vtkDataArray* GetActiveScalarArray() override;

  ///@{
  /// The arrays that GlyphOrientationArrayName and GlyphScaleArrayName name, in what the
  /// field is read from (see GetFieldDataSetArray()). nullptr if not found or not set.
  vtkDataArray* GetGlyphOrientationArray();
  vtkDataArray* GetGlyphScaleArray();
  ///@}

  /// Recomputes the automatic scalar range for the new active scalar array (glyphs are colored by the array
  /// directly, there is no assign-attribute filter to update).
  void UpdateAssignedAttribute() override;

  /// Compute the scalar range per ScalarRangeFlag, of the magnitude or chosen component for multi-component arrays
  /// (from the voxel vectors for a vector volume, before sampling). Kept as it is if there is nothing to compute it from.
  void UpdateScalarRange() override;

  /// Updates the scalar range as well, which is taken from the color node if ScalarRangeFlag is UseColorNodeScalarRange
  void SetAndObserveColorNodeID(const char* colorNodeID) override;
  using Superclass::SetAndObserveColorNodeID;

protected:
  /// Does the work of UpdateScalarRange(), which only guards it against being started again
  /// while it runs.
  void UpdateScalarRangeInternal();

  /// Copy the coloring settings of the coloring source display node into this node.
  void UpdateColoringFromSourceDisplayNode();

  /// Takes the coloring of a coloring source display node as soon as it is linked, also
  /// when the link is read from a scene.
  void OnNodeReferenceAdded(vtkMRMLNodeReference* reference) override;
  void OnNodeReferenceModified(vtkMRMLNodeReference* reference) override;

  vtkMRMLVectorFieldDisplayNode();
  ~vtkMRMLVectorFieldDisplayNode() override;
  vtkMRMLVectorFieldDisplayNode(const vtkMRMLVectorFieldDisplayNode&);
  void operator=(const vtkMRMLVectorFieldDisplayNode&);

  /// The events of a sampling or masking region node that change where the field is drawn.
  static void AddSamplingEvents(vtkIntArray* events);

  /// True if the named array is a cell data array of the mesh of the model node, and there
  /// is no point data array of the same name.
  bool IsMeshCellDataArray(const char* arrayName);

  ///@{
  /// Name of the array that scales the glyphs, and the array: the scale array if set, else the orientation array.
  const char* GetEffectiveGlyphScaleArrayName();
  vtkDataArray* GetEffectiveGlyphScaleArray();
  ///@}

  /// True if a sampled field is colored by one component of its sampled vectors
  bool IsSampledVectorComponentColored();

  /// True if a mesh is drawn somewhere other than at its own points (a lattice, or glyphs in a sampling region),
  /// so that it is sampled if it can be
  bool IsMeshSamplingNeeded();

  /// Set by the pipeline (see GetSampledFieldDataSet())
  void SetSampledFieldDataSet(vtkDataSet* dataSet);

  /// The named point data array of what a render has produced from the sampler
  /// (GetSampledFieldDataSet()), or nullptr before the first one.
  vtkDataArray* GetSampledFieldArray(const char* arrayName);

  int GlyphType{ GlyphTypeArrow };
  char* GlyphOrientationArrayName{ nullptr };
  char* GlyphScaleArrayName{ nullptr };
  char* FieldArrayName{ nullptr };
  bool GlyphScaleByComponents{ false };
  double GlyphScaleFactor{ 10.0 };
  int SamplingRegionPart{ SamplingRegionPartDefault };
  int SamplingPattern{ SamplingPatternRandom };
  int SamplingPointsNumber{ 5000 };
  bool MaskingShowInside{ true };
  std::vector<std::string> MaskingSegmentIDs;

  /// The array of the given name in what the field is read from. For a mesh, a point data
  /// array is preferred, and a cell data array is used if there is no such point data array.
  vtkDataArray* GetFieldDataSetArray(const char* arrayName);

  /// Set the array names to the arrays that a sampler produces, if they are not set yet.
  /// Sampled fields always provide the same two arrays, so the user does not have to pick.
  void SetDefaultSampledFieldArrayNames();

  /// What the field source of the pipeline produces
  vtkSmartPointer<vtkDataSet> SampledFieldDataSet;
  /// The pipeline asks this node what the settings mean, and sets SampledFieldDataSet
  friend class vtkMRMLVectorFieldPipeline;

  int VisualizationMode{ VisualizationModeGlyph };
  bool GlyphScaleDirectional{ true };
  double GlyphDiameter{ 5.0 };
  bool GlyphDiameterAbsolute{ false };
  double GlyphDiameterPercent{ 20.0 };
  double GridSpacing{ 3.0 };
  bool GridShowNonWarped{ false };
  double GridNonWarpedColor[3]{ 0.2, 0.2, 0.2 };
  bool GridFill{ false };
  bool GridLinesVisibility{ true };
  double ContourOpacity3D{ 0.8 };
  double GridScaleFactor{ 1.0 };
  double GridLineDiameter{ 0.5 };
  std::vector<double> ContourLevels;
  double StreamlineMaximumPropagation{ 100.0 };
  bool StreamlineBidirectional{ true };
  double StreamlineInitialIntegrationStep{ 0.1 };
  double StreamlineTubeDiameter{ 0.5 };
  double StreamlineSliceFadeDistance{ 20.0 };

  double SamplingSpacing{ 5.0 };
  bool GlyphSliceProjectionEnabled{ true };
  /// The glyph geometry defaults reproduce the geometry of the VTK glyph sources with their
  /// own default parameters (vtkArrowSource: tip length 0.35, shaft radius 0.3 * tip radius)
  int GlyphResolution{ 16 };
  double GlyphTipLengthPercent{ 35.0 };
  double GlyphShaftDiameterPercent{ 30.0 };
  int GlyphResolution2D{ 12 };
  double GlyphTipLengthPercent2D{ 30.0 };
  bool ThresholdEnabled{ false };
  /// Invalid by default, as in vtkMRMLModelDisplayNode
  double ThresholdRange[2]{ 0.0, -1.0 };

  bool InterpolateBetweenPoints{ false };
  double InterpolationMaximumDistance{ 0.0 };

  /// True while UpdateScalarRange() runs: computing it can prepare the field of a volume, which updates the range
  /// again, and that must not recurse.
  bool IsUpdatingScalarRange{ false };
};

#endif
