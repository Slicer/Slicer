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

#ifndef __vtkVectorFieldGeometry_h
#define __vtkVectorFieldGeometry_h

// MRML includes
#include "vtkSlicerVectorFieldRenderingModuleMRMLExport.h"

// VTK includes
#include <vtkNew.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>

// STD includes
#include <string>
#include <vector>

class vtkArrayCalculator;
class vtkAssignAttribute;
class vtkClipPolyData;
class vtkContourFilter;
class vtkDataSet;
class vtkPoints;
class vtkPolyData;
class vtkStreamTracer;
class vtkThreshold;
class vtkTransform;
class vtkTransformPolyDataFilter;
class vtkTrivialProducer;
class vtkTubeFilter;
class vtkVectorFieldGrid;
class vtkVectorFieldStreamlineImage;

/// \brief Turns a vector field into the geometry of a visualization mode: deformed grid, magnitude contours, or streamlines.
///
/// Input: a point set or image data with a 3-component vector array in point data (VectorArrayName).
/// Operations: warp a grid by the vectors (vtkVectorFieldGrid), contour the magnitude, or trace streamlines from seed
/// points, until they leave the domain or a value leaves the threshold range, optionally made into tubes; clip to a
/// region array. An image is traced as an image: in its own axis-aligned space if it is not axis-aligned (its axes are
/// not along the coordinate axes), which the stream tracer needs to trace it right and fast.
/// Outputs: port 0 the geometry, port 1 the streamlines as lines (thresholded, without tubes), port 2 the undeformed
/// grid lines.
/// The internal filters are kept between executions, and the input is copied only when it changed, so changing a
/// setting re-runs only the steps after it (e.g. a new tube diameter does not trace the streamlines again).
///
/// \sa vtkVectorFieldSampler, vtkVectorFieldGrid, vtkVectorFieldGlyph
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_MRML_EXPORT vtkVectorFieldGeometry : public vtkPolyDataAlgorithm
{
public:
  static vtkVectorFieldGeometry* New();
  vtkTypeMacro(vtkVectorFieldGeometry, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum Modes
  {
    ModeNone,       ///< no geometry
    ModeGrid,       ///< grid lines and fill of the sampling lattice, deformed by the vectors
    ModeContour,    ///< isosurfaces of the vector magnitude (isolines in a single layer of cells)
    ModeStreamline, ///< streamlines traced from the seeds
    Mode_Last
  };

  /// Output port of the streamlines as lines (port 0 holds tubes when the tube diameter is positive)
  static const int StreamlineLinesPort = 1;
  /// Output port of the undeformed grid lines (port 0 holds the deformed lines and the fill)
  static const int GridUndeformedLinesPort = 2;

  ///@{
  /// Visualization mode (Modes). Default is ModeNone.
  vtkSetClampMacro(Mode, int, ModeNone, Mode_Last - 1);
  vtkGetMacro(Mode, int);
  ///@}

  ///@{
  /// Draw for a slice view: grid lines and streamlines as lines, not tubes. Default is off.
  vtkSetMacro(Flat, bool);
  vtkGetMacro(Flat, bool);
  vtkBooleanMacro(Flat, bool);
  ///@}

  ///@{
  /// Point array of the vectors that deform the grid and are integrated along streamlines.
  vtkSetMacro(VectorArrayName, std::string);
  vtkGetMacro(VectorArrayName, std::string);
  ///@}

  ///@{
  /// Point array of the vector magnitude, which is contoured. Empty: computed from the vectors.
  vtkSetMacro(MagnitudeArrayName, std::string);
  vtkGetMacro(MagnitudeArrayName, std::string);
  ///@}

  ///@{
  /// Point array that says where the field is shown: grid and contours are cut where it is below 0.5. Empty: no cut.
  vtkSetMacro(RegionArrayName, std::string);
  vtkGetMacro(RegionArrayName, std::string);
  ///@}

  ///@{
  /// Grid settings, see vtkVectorFieldGrid. The line diameter is not used for flat output.
  vtkSetClampMacro(GridSubdivision, int, 1, VTK_INT_MAX);
  vtkGetMacro(GridSubdivision, int);
  vtkSetMacro(GridScaleFactor, double);
  vtkGetMacro(GridScaleFactor, double);
  vtkSetMacro(GridDeformedLinesVisibility, bool);
  vtkGetMacro(GridDeformedLinesVisibility, bool);
  vtkSetMacro(GridUndeformedLinesVisibility, bool);
  vtkGetMacro(GridUndeformedLinesVisibility, bool);
  vtkSetMacro(GridFill, bool);
  vtkGetMacro(GridFill, bool);
  vtkSetClampMacro(GridLineDiameter, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(GridLineDiameter, double);
  ///@}

  ///@{
  /// Magnitude values of the contours
  void SetContourLevels(const std::vector<double>& levels);
  std::vector<double> GetContourLevels() const { return this->ContourLevels; }
  ///@}

  ///@{
  /// Streamline seed positions, in the coordinates of the input. Setting the same positions again does not modify the
  /// filter, so they can be set on every update without tracing again.
  void SetStreamlineSeeds(vtkPoints* seeds);
  vtkPoints* GetStreamlineSeeds();
  ///@}

  ///@{
  /// Point array that limits where streamlines are traced: only through cells where it is at least 0.5 at all points,
  /// so they stop where they leave the region. Empty: the whole field. The other cells of an image are hidden in it,
  /// so it is traced as an image; the cells of a mesh are extracted.
  vtkSetMacro(StreamlineDomainArrayName, std::string);
  vtkGetMacro(StreamlineDomainArrayName, std::string);
  ///@}

  ///@{
  /// Streamline integration: both directions from the seeds or forward only, length limit in mm, initial step in mm
  /// (0: the stream tracer default).
  vtkSetMacro(StreamlineBidirectional, bool);
  vtkGetMacro(StreamlineBidirectional, bool);
  vtkSetClampMacro(StreamlineMaximumPropagation, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(StreamlineMaximumPropagation, double);
  vtkSetClampMacro(StreamlineInitialIntegrationStep, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(StreamlineInitialIntegrationStep, double);
  ///@}

  ///@{
  /// Diameter of streamline tubes in mm. 0 or flat output: lines.
  vtkSetClampMacro(StreamlineTubeDiameter, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(StreamlineTubeDiameter, double);
  ///@}

  ///@{
  /// Trace streamlines only while a point array is in the threshold range: as with the domain, they stop where the
  /// value leaves the range, and seeds where it is out of the range start nothing. They are traced through the cells
  /// that have a point in the range, and cut where the value crosses a limit. The component is used for a
  /// multi-component array; -1 uses the magnitude.
  vtkSetMacro(ThresholdEnabled, bool);
  vtkGetMacro(ThresholdEnabled, bool);
  vtkSetMacro(ThresholdArrayName, std::string);
  vtkGetMacro(ThresholdArrayName, std::string);
  vtkSetMacro(ThresholdComponent, int);
  vtkGetMacro(ThresholdComponent, int);
  vtkSetVector2Macro(ThresholdRange, double);
  vtkGetVector2Macro(ThresholdRange, double);
  ///@}

protected:
  vtkVectorFieldGeometry();
  ~vtkVectorFieldGeometry() override;
  vtkVectorFieldGeometry(const vtkVectorFieldGeometry&) = delete;
  void operator=(const vtkVectorFieldGeometry&) = delete;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  /// Copy the input into the head of the internal pipeline if it changed since the last execution
  void UpdateFieldCopy(vtkDataSet* input);
  vtkAlgorithmOutput* ConfigureGrid();
  vtkAlgorithmOutput* ConfigureContour();
  /// Returns the tubes or lines; streamlinesConnection is set to the lines
  vtkAlgorithmOutput* ConfigureStreamlines(vtkAlgorithmOutput*& streamlinesConnection);
  /// Cut by the region array, if set
  vtkAlgorithmOutput* ClipToRegion(vtkAlgorithmOutput* connection);

  int Mode{ ModeNone };
  bool Flat{ false };
  std::string VectorArrayName;
  std::string MagnitudeArrayName;
  std::string RegionArrayName;

  int GridSubdivision{ 1 };
  double GridScaleFactor{ 1.0 };
  bool GridDeformedLinesVisibility{ true };
  bool GridUndeformedLinesVisibility{ false };
  bool GridFill{ false };
  double GridLineDiameter{ 0.0 };

  std::vector<double> ContourLevels;

  std::string StreamlineDomainArrayName;
  bool StreamlineBidirectional{ true };
  double StreamlineMaximumPropagation{ 100.0 };
  double StreamlineInitialIntegrationStep{ 0.0 };
  double StreamlineTubeDiameter{ 0.0 };

  bool ThresholdEnabled{ false };
  std::string ThresholdArrayName;
  int ThresholdComponent{ -1 };
  double ThresholdRange[2]{ 0.0, -1.0 };

  /// Head of the internal pipeline: a shallow copy of the input, renewed only when the input changes
  vtkSmartPointer<vtkDataSet> FieldCopy;
  vtkDataSet* FieldCopySource{ nullptr };
  vtkMTimeType FieldCopySourceMTime{ 0 };
  vtkNew<vtkTrivialProducer> FieldProducer;

  vtkNew<vtkVectorFieldGrid> Grid;
  /// Magnitude of the vectors, when the input has no magnitude array
  vtkNew<vtkArrayCalculator> FieldMagnitude;
  std::string FieldMagnitudeVectorArrayName;
  vtkNew<vtkContourFilter> Contour;
  vtkNew<vtkAssignAttribute> RegionScalars;
  vtkNew<vtkClipPolyData> RegionClip;

  vtkNew<vtkPolyData> StreamlineSeeds;
  /// From the space that the streamlines are traced in to that of the field: the axis-aligned space of a
  /// non-axis-aligned image, where the seeds are moved to and the streamlines are moved back from. Identity for any
  /// other field.
  vtkNew<vtkTransform> StreamlineTracedToField;
  vtkNew<vtkTransformPolyDataFilter> StreamlineSeedsToTraced;
  vtkNew<vtkTransformPolyDataFilter> StreamlinesToField;
  /// The image that streamlines are traced through: axis-aligned, with the cells hidden that are out of the domain or
  /// of the threshold range
  vtkNew<vtkVectorFieldStreamlineImage> StreamlineImage;
  /// Streamline domain of a mesh: the cells that are in the domain at all of their points
  vtkNew<vtkThreshold> StreamlineDomain;
  /// Streamline threshold of a mesh: the cells that have a point in the range
  vtkNew<vtkThreshold> StreamlineThresholdDomain;
  vtkSmartPointer<vtkStreamTracer> StreamTracer;
  /// Streamline threshold: the value as active scalars, clipped from below and from above, for the ends of the lines
  vtkNew<vtkArrayCalculator> StreamlineThresholdValue;
  std::string StreamlineThresholdArrayName;
  int StreamlineThresholdComponent{ -2 };
  vtkNew<vtkAssignAttribute> StreamlineThresholdScalars;
  vtkNew<vtkClipPolyData> StreamlineClipBelow;
  vtkNew<vtkClipPolyData> StreamlineClipAbove;
  vtkNew<vtkTubeFilter> StreamlineTuber;
};

#endif
