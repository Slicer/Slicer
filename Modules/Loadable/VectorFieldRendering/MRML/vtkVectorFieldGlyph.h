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

#ifndef __vtkVectorFieldGlyph_h
#define __vtkVectorFieldGlyph_h

// MRML includes
#include "vtkSlicerVectorFieldRenderingModuleMRMLExport.h"

// VTK includes
#include <vtkNew.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>

// STD includes
#include <string>

class vtkArrowSource;
class vtkConeSource;
class vtkCubeSource;
class vtkCylinderSource;
class vtkDataSet;
class vtkGeometryFilter;
class vtkGlyph3DMapper;
class vtkGlyphSource2D;
class vtkLineSource;
class vtkMaskPoints;
class vtkMatrix4x4;
class vtkSphereSource;
class vtkTransformPolyDataFilter;

/// \brief Glyphs that show a vector field: which points get one, their shape, and their geometry.
///
/// Input: vtkVectorFieldSampler samples or mesh points, with the arrays that orient, scale, and color the glyphs.
/// Outputs:
/// - 0, glyph points: the shown input points with their arrays plus the directional scale and projected orientation
///   arrays. 3D views draw them with a vtkGlyph3DMapper set up by ConfigureMapper().
/// - 1, glyph shape: one unit long along +X.
/// - 2, glyph geometry (if GenerateGeometry): every glyph as polygons or lines with its point data, placed as
///   vtkGlyph3DMapper places them. Drawn by slice views and used for export.
///
/// Shown points pass the threshold and region array; a surface or volume random subset is taken from the input
/// cells before thresholding, other subsets after it. Mapper-only settings (orientation, scale) do not re-execute
/// the filter unless the outputs depend on them, so dragging the scale slider does not reselect the points.
///
/// \sa vtkVectorFieldSampler, vtkVectorFieldGeometry
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_MRML_EXPORT vtkVectorFieldGlyph : public vtkPolyDataAlgorithm
{
public:
  static vtkVectorFieldGlyph* New();
  vtkTypeMacro(vtkVectorFieldGlyph, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum OutputPorts
  {
    GlyphPointsPort = 0,
    GlyphShapePort = 1,
    GlyphGeometryPort = 2
  };

  enum SubsetModes
  {
    SubsetAll,           ///< every point that is shown
    SubsetRandomBounds,  ///< a random subset spread evenly over the bounds of the points
    SubsetRandomSurface, ///< a random subset spread evenly over the surface of the input
    SubsetRandomVolume,  ///< a random subset spread evenly over the volume of the input
    SubsetSpaced,        ///< a subset of the points that are shown, about SubsetSpacing apart
    SubsetSpacedSurface, ///< a subset of the points that are shown on the surface of the input, about SubsetSpacing apart
    Subset_Last
  };

  enum ScaleModes
  {
    ScaleNone,         ///< all glyphs ScaleFactor long
    ScaleByMagnitude,  ///< by the magnitude of the scale array (the value of a scalar array)
    ScaleByComponents, ///< along each axis of the glyph by a component of the scale array
    Scale_Last
  };

  enum GlyphTypes
  {
    GlyphArrow,
    GlyphCone,
    GlyphBox,
    GlyphCylinder,
    GlyphLine,
    GlyphSphere,
    Glyph_Last
  };

  /// Name of the per-axis scale array of directionally scaled glyphs (DirectionalScale):
  /// (length times ScaleFactor, 1, 1).
  static const char* GetDirectionalScaleArrayName();

  /// Suffix of the name of the projected orientation array (ProjectToPlane)
  static const char* GetProjectedArrayNameSuffix();

  ///@{
  /// Only show the points where the threshold array is within ThresholdRange.
  /// Default is off.
  vtkSetMacro(ThresholdEnabled, bool);
  vtkGetMacro(ThresholdEnabled, bool);
  vtkBooleanMacro(ThresholdEnabled, bool);
  ///@}

  ///@{
  /// Point array that the threshold is applied to.
  void SetThresholdArrayName(const std::string& name);
  std::string GetThresholdArrayName() const { return this->ThresholdArrayName; }
  ///@}

  ///@{
  /// Component of a multi-component threshold array that is compared to the range: -1 (or a
  /// component that the array does not have) for its magnitude. Default is -1.
  vtkSetMacro(ThresholdComponent, int);
  vtkGetMacro(ThresholdComponent, int);
  ///@}

  ///@{
  /// Range of values that are shown, inclusive.
  vtkSetVector2Macro(ThresholdRange, double);
  vtkGetVector2Macro(ThresholdRange, double);
  ///@}

  ///@{
  /// Point array that says where the field is shown: points where it is below 0.5 get no
  /// glyph. Empty, or an array that the input does not have, shows every point.
  void SetRegionArrayName(const std::string& name);
  std::string GetRegionArrayName() const { return this->RegionArrayName; }
  ///@}

  ///@{
  /// Which of the points that are shown get a glyph. Default is SubsetAll.
  vtkSetClampMacro(SubsetMode, int, SubsetAll, Subset_Last - 1);
  vtkGetMacro(SubsetMode, int);
  ///@}

  ///@{
  /// Number of points in a random subset. Default is 1000.
  vtkSetClampMacro(MaximumNumberOfPoints, vtkIdType, 0, VTK_ID_MAX);
  vtkGetMacro(MaximumNumberOfPoints, vtkIdType);
  /// Distance between the points of the spaced subset modes. Not positive: all points. Default is 0.
  vtkSetClampMacro(SubsetSpacing, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(SubsetSpacing, double);
  ///@}

  ///@{
  /// Seed of the random subset. The same seed gives the same subset of the same points, so
  /// that the glyphs do not move when the filter computes again. Default is 1.
  vtkSetMacro(RandomSeed, int);
  vtkGetMacro(RandomSeed, int);
  ///@}

  ///@{
  /// Point array that the glyphs point along. Empty leaves them pointing along +X.
  void SetOrientationArrayName(const std::string& name);
  std::string GetOrientationArrayName() const { return this->OrientationArrayName; }
  ///@}

  ///@{
  /// Point array that the glyphs are scaled by. Empty scales them by ScaleFactor only.
  void SetScaleArrayName(const std::string& name);
  std::string GetScaleArrayName() const { return this->ScaleArrayName; }
  ///@}

  ///@{
  /// How the scale array scales the glyphs. Default is ScaleByMagnitude.
  void SetScaleMode(int scaleMode);
  vtkGetMacro(ScaleMode, int);
  ///@}

  ///@{
  /// Factor that the glyph size is multiplied with. A negative factor reverses the direction of the glyphs.
  /// Default is 1.
  void SetScaleFactor(double scaleFactor);
  vtkGetMacro(ScaleFactor, double);
  ///@}

  ///@{
  /// Stretch the glyphs only along their axis (thickness stays that of the shape) via the per-axis scale array
  /// GetDirectionalScaleArrayName(). Needs a scale array; ignored for flat glyphs. Default is off.
  vtkSetMacro(DirectionalScale, bool);
  vtkGetMacro(DirectionalScale, bool);
  vtkBooleanMacro(DirectionalScale, bool);
  ///@}

  ///@{
  /// Shape of the glyphs. Default is GlyphArrow.
  vtkSetClampMacro(GlyphType, int, GlyphArrow, Glyph_Last - 1);
  vtkGetMacro(GlyphType, int);
  ///@}

  ///@{
  /// Number of sides of round shapes (arrow, cone, cylinder, sphere; circle of flat glyphs).
  /// Default is 16.
  vtkSetClampMacro(Resolution, int, 3, 1024);
  vtkGetMacro(Resolution, int);
  ///@}

  ///@{
  /// Length of the tip of an arrow, relative to its length. Default is 0.35.
  vtkSetClampMacro(TipLength, double, 0.0, 1.0);
  vtkGetMacro(TipLength, double);
  ///@}

  ///@{
  /// Radius of an arrow tip and of a cone, relative to the length. Default is 0.1.
  vtkSetClampMacro(TipRadius, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(TipRadius, double);
  ///@}

  ///@{
  /// Radius of the shaft of an arrow, relative to the length. Default is 0.03.
  vtkSetClampMacro(ShaftRadius, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(ShaftRadius, double);
  ///@}

  ///@{
  /// Draw flat glyphs: outlines of 2D shapes (arrow, edge arrow, square, dash, circle) in GlyphPlane (XY plane if none),
  /// always scaled uniformly so glyphs of small vectors shrink away instead of leaving full-width heads. Default is off.
  vtkSetMacro(Flat, bool);
  vtkGetMacro(Flat, bool);
  vtkBooleanMacro(Flat, bool);
  ///@}

  ///@{
  /// Plane of flat glyphs and of ProjectToPlane: maps its X and Y axes to input coordinates. Only its directions are
  /// used, so it may scale; nullptr for none.
  void SetGlyphPlane(vtkMatrix4x4* planeToInput);
  vtkMatrix4x4* GetGlyphPlane();
  ///@}

  ///@{
  /// Orient glyphs along the in-plane part of the vectors (GlyphPlane), stored in an array named with GetProjectedArrayNameSuffix(),
  /// so coloring still uses the original vectors. Needs a glyph plane and an orientation array. Default is off.
  vtkSetMacro(ProjectToPlane, bool);
  vtkGetMacro(ProjectToPlane, bool);
  vtkBooleanMacro(ProjectToPlane, bool);
  ///@}

  ///@{
  /// Compute the glyph geometry (output 2). Default is off: the geometry of many glyphs is
  /// large, and 3D views place the glyphs on the GPU instead.
  vtkSetMacro(GenerateGeometry, bool);
  vtkGetMacro(GenerateGeometry, bool);
  vtkBooleanMacro(GenerateGeometry, bool);
  ///@}

  /// Set up a glyph mapper with the shape (output 1), orientation, and scale of the glyphs.
  /// The caller connects the glyph points (output 0) as mapper input.
  void ConfigureMapper(vtkGlyph3DMapper* mapper);

  /// Copy every setting of another glyph filter, not its input.
  void CopySettings(vtkVectorFieldGlyph* source);

  /// The array that orients the glyphs: the projected orientation array when the vectors are
  /// projected to the glyph plane, otherwise the orientation array. Empty if there is none.
  std::string GetRenderedOrientationArrayName();

  /// The array that scales the glyphs: the scale array, or the rendered orientation array when the glyphs are scaled
  /// by the vectors that orient them. Empty if there is none.
  std::string GetRenderedScaleArrayName();

  /// How the glyphs are scaled (as ConfigureMapper() and BuildGlyphGeometry() do it): the scaling point array (empty if
  /// none), the scale mode (ScaleNone without an array), and the scale factor.
  void GetRenderedScale(std::string& scaleArrayName, int& scaleMode, double& scaleFactor);

protected:
  vtkVectorFieldGlyph();
  ~vtkVectorFieldGlyph() override;
  vtkVectorFieldGlyph(const vtkVectorFieldGlyph&) = delete;
  void operator=(const vtkVectorFieldGlyph&) = delete;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  /// The points of the input that get a glyph, with their point data
  void SelectGlyphPoints(vtkDataSet* input, vtkPolyData* glyphPoints);
  /// The points of dataSet within the threshold range and the region, with their point data
  void ExtractShownPoints(vtkDataSet* dataSet, vtkPolyData* shownPoints);
  /// Random subset of the points of dataSet, spread as SubsetMode says
  void TakeRandomSubset(vtkDataSet* dataSet, int spread, vtkPolyData* subset);
  /// Subset of the points of pointSet that are about SubsetSpacing apart
  void TakeSpacedSubset(vtkPolyData* points, vtkPolyData* subset);
  /// Add the arrays that the glyphs need: the directional scale, the projected orientation
  void AddGlyphArrays(vtkPolyData* glyphPoints);
  /// The shape that is placed at each point, one unit long along +X
  void BuildGlyphShape(vtkPolyData* shape);
  /// Every glyph, placed as vtkGlyph3DMapper places it
  void BuildGlyphGeometry(vtkPolyData* glyphPoints, vtkPolyData* shape, vtkPolyData* geometry);
  /// True if the outputs depend on the settings that the mapper uses
  bool OutputUsesMapperSettings();
  /// Unit vector along the normal of the glyph plane. Returns false if there is no plane.
  bool GetGlyphPlaneNormal(double normal[3]);

  bool ThresholdEnabled{ false };
  std::string ThresholdArrayName;
  int ThresholdComponent{ -1 };
  double ThresholdRange[2]{ 0.0, 1.0 };
  std::string RegionArrayName;
  int SubsetMode{ SubsetAll };
  vtkIdType MaximumNumberOfPoints{ 1000 };
  double SubsetSpacing{ 0.0 };
  int RandomSeed{ 1 };
  std::string OrientationArrayName;
  std::string ScaleArrayName;
  int ScaleMode{ ScaleByMagnitude };
  double ScaleFactor{ 1.0 };
  bool DirectionalScale{ false };
  int GlyphType{ GlyphArrow };
  int Resolution{ 16 };
  double TipLength{ 0.35 };
  double TipRadius{ 0.1 };
  double ShaftRadius{ 0.03 };
  bool Flat{ false };
  vtkSmartPointer<vtkMatrix4x4> GlyphPlane;
  bool ProjectToPlane{ false };
  bool GenerateGeometry{ false };

  vtkNew<vtkGeometryFilter> SurfaceFilter;
  vtkNew<vtkMaskPoints> MaskPoints;
  vtkNew<vtkArrowSource> ArrowSource;
  vtkNew<vtkConeSource> ConeSource;
  vtkNew<vtkCubeSource> CubeSource;
  vtkNew<vtkCylinderSource> CylinderSource;
  vtkNew<vtkTransformPolyDataFilter> CylinderRotation;
  vtkNew<vtkLineSource> LineSource;
  vtkNew<vtkSphereSource> SphereSource;
  vtkNew<vtkGlyphSource2D> FlatSource;
  vtkNew<vtkTransformPolyDataFilter> FlatSourceShift;
};

#endif
