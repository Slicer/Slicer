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

#ifndef __vtkVectorFieldSampler_h
#define __vtkVectorFieldSampler_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkAbstractTransform;
class vtkAlgorithm;
class vtkDataArray;
class vtkIdList;
class vtkImageData;
class vtkMatrix4x4;
class vtkPointSet;
class vtkPoints;
class vtkScatteredBSplineTransform;
class vtkStaticPointLocator;
class vtkUnsignedCharArray;
class vtkUnstructuredGrid;

/// \brief Samples a vector field at positions that do not depend on how the field is stored.
///
/// Field sources (set by SetFieldSource()):
/// - Image: voxels of the input image (at least 3 components), linearly interpolated.
/// - Mesh: point vectors of the input mesh, interpolated in its cells; zero vector outside the cells.
/// - Scattered: B-spline fitted to vectors at scattered points; samples beyond ScatteredMaximumDistance are invalid.
/// - Transform: displacement of a transform; it has no extent, so a region, slice plane or sample positions are needed.
///
/// Outputs are RAS points with a 3-component vector array (GetVectorArrayName()), a magnitude array
/// (GetMagnitudeArrayName()), and optionally a color array sampled from the source (GetColorArrayName()).
///
/// Setters only mark the filter modified; sampling happens in RequestData(). GetMTime() includes the MTime of the
/// sampled object, so a modified field is re-sampled on the next render without observers.
///
/// \sa vtkMRMLVectorFieldDisplayNode
class VTK_MRML_EXPORT vtkVectorFieldSampler : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkVectorFieldSampler* New();
  vtkTypeMacro(vtkVectorFieldSampler, vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// What stores the field that is sampled
  enum FieldSourceType
  {
    FieldSourceNone,      ///< produces empty output, useful to avoid failures when no source is set yet
    FieldSourceImage,     ///< the voxels of the input image
    FieldSourceMesh,      ///< the vectors of the input mesh, inside its cells
    FieldSourceScattered, ///< a B-spline fitted to vectors at scattered points
    FieldSourceTransform, ///< the displacement of a transform
    FieldSource_Last
  };

  ///@{
  /// What stores the field that is sampled. Default is FieldSourceNone.
  /// Switching releases what the previous source used (input, fitted transforms, transform), so set the source first.
  void SetFieldSource(int fieldSource);
  vtkGetMacro(FieldSource, int);
  void SetFieldSourceToNone() { this->SetFieldSource(FieldSourceNone); }
  void SetFieldSourceToImage() { this->SetFieldSource(FieldSourceImage); }
  void SetFieldSourceToMesh() { this->SetFieldSource(FieldSourceMesh); }
  void SetFieldSourceToScattered() { this->SetFieldSource(FieldSourceScattered); }
  void SetFieldSourceToTransform() { this->SetFieldSource(FieldSourceTransform); }
  static const char* GetFieldSourceAsString(int fieldSource);
  ///@}

  ///@{
  /// Image source: the matrix that maps the IJK voxel coordinates of the image to RAS.
  virtual void SetIJKToRAS(vtkMatrix4x4* ijkToRAS);
  vtkMatrix4x4* GetIJKToRAS();
  ///@}

  ///@{
  /// Image source: true if the voxel values are RAS vector components (vtkMRMLVolumeNode::VoxelVectorTypeSpatial),
  /// false if they are IJK components (rotated into RAS by IJKToRAS). Default is true.
  vtkSetMacro(VectorsInRAS, bool);
  vtkGetMacro(VectorsInRAS, bool);
  vtkBooleanMacro(VectorsInRAS, bool);
  ///@}

  ///@{
  /// Mesh source: name of the point data array of the mesh that holds the vectors. If it is
  /// not set, the active vectors of the mesh are used.
  vtkSetStringMacro(MeshVectorArrayName);
  vtkGetStringMacro(MeshVectorArrayName);
  ///@}

  ///@{
  /// Mesh source: point or cell data array to color by, sampled into GetColorArrayName().
  /// If it is not set or the mesh has no such array, the field is colored by its magnitude.
  vtkSetStringMacro(MeshColorArrayName);
  vtkGetStringMacro(MeshColorArrayName);
  ///@}

  ///@{
  /// Mesh source: component of a multi-component color array that the color array gets, -1
  /// for the magnitude. Default is -1.
  vtkSetMacro(MeshColorArrayComponent, int);
  vtkGetMacro(MeshColorArrayComponent, int);
  ///@}

  ///@{
  /// Scattered source: transform fitted to the vectors at the data points (its input point set).
  /// Owned by the display node and shared by all views, so the fit is only recomputed when the data changes.
  virtual void SetScatteredFittedTransform(vtkScatteredBSplineTransform* transform);
  vtkScatteredBSplineTransform* GetScatteredFittedTransform();
  ///@}

  ///@{
  /// Scattered source: optional transform fitted to the color values at the same points (first displacement component).
  /// If set, the values are sampled into GetColorArrayName(), otherwise the field is colored by its magnitude.
  virtual void SetScatteredColorFittedTransform(vtkScatteredBSplineTransform* transform);
  vtkScatteredBSplineTransform* GetScatteredColorFittedTransform();
  ///@}

  ///@{
  /// Scattered source: samples farther than this from the nearest data point are not drawn, in mm.
  /// 0 (default) means a tenth of the diagonal of the bounds of the data points.
  vtkSetClampMacro(ScatteredMaximumDistance, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(ScatteredMaximumDistance, double);
  ///@}

  /// Scattered source: the distance that is actually used: ScatteredMaximumDistance, or the
  /// automatic one when that is 0. Returns 0 if there are no data points.
  double GetScatteredEffectiveMaximumDistance();

  ///@{
  /// Transform source: transform whose displacement (transformed position minus position) is sampled,
  /// in world coordinates. Any transform type works.
  virtual void SetTransform(vtkAbstractTransform* transform);
  vtkAbstractTransform* GetTransform();
  ///@}

  /// Name of the 3-component point data array that stores the sampled vectors, in RAS.
  static const char* GetVectorArrayName();

  /// Name of the point data array that stores the magnitude of the sampled vectors.
  static const char* GetMagnitudeArrayName();

  /// Name of the 1-component point data array with the values to color by (a scalar, a component, or the magnitude
  /// of a source array other than the vectors). Absent when no such array is sampled.
  static const char* GetColorArrayName();

  /// True if the algorithm outputs the vector and magnitude arrays under the sampler's names: a sampler, or the
  /// RAS image of a vector volume (image source of vtkMRMLVectorFieldPipeline).
  static bool ProducesSampledField(vtkAlgorithm* producer);

  /// Name of the field data array with the lattice size of the samples, which downstream filters (grid lines) need
  /// to connect the points. Absent when the samples do not form a lattice.
  static const char* GetLatticeSizeArrayName();

  /// Name of the point data array marking samples where the field has a value (1) or not (0); absent means all are
  /// valid (e.g. transform). Grid lines and lattice cells stop at invalid samples, so they end where the field does.
  static const char* GetValidSampleArrayName();

  /// A component of a tuple of an array, or the magnitude of the tuple if the component is -1
  /// or one that the array does not have (as vtkThresholdPoints and vtkGlyph3DMapper read it).
  /// True if both hold the same points, in the same order (nullptr counts as no points)
  static bool ArePointsEqual(vtkPoints* points, vtkPoints* otherPoints);

  static double GetComponentOrMagnitude(vtkDataArray* array, vtkIdType tupleIndex, int component = -1);

  /// The length of the diagonal of a bounding box (xmin, xmax, ymin, ymax, zmin, zmax).
  static double GetBoundsDiagonal(const double bounds[6]);

  /// Ids of a subset of the points that are about the spacing apart: the point closest to the center of each cube of a
  /// grid with that spacing. In increasing order, and always the same for the same points. All points if the spacing is
  /// not positive.
  static void GetSpacedPointIds(vtkPoints* points, double spacing, vtkIdList* pointIds);

  /// Region that is sampled, in RAS: regionToRAS sets the origin and axis directions, size the number of
  /// sampling steps along each axis. Only marks the filter modified.
  virtual void SetSamplingRegion(vtkMatrix4x4* regionToRAS, const int size[3]);

  ///@{
  /// Distance between sampled points, in mm. 0 means the source decides (for example the
  /// voxel size of the sampled image).
  vtkSetClampMacro(SamplingSpacing, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(SamplingSpacing, double);
  ///@}

  ///@{
  /// Thickness of the sampled slice, in mm: only given sample positions within it are used
  /// (lattices are in the plane anyway). Default is 1.
  vtkSetClampMacro(SliceThickness, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(SliceThickness, double);
  ///@}

  ///@{
  /// Number of sampling steps kept together, so the lattice holds whole groups along each axis and is centered in
  /// the region. Grid visualization sets it to the steps between grid lines to draw complete cells. Default is 1.
  vtkSetClampMacro(LatticeGroupSize, int, 1, VTK_INT_MAX);
  vtkGetMacro(LatticeGroupSize, int);
  ///@}

  /// Restrict sampling to a slice plane: sliceXYToRAS maps slice view coordinates to RAS, fieldOfViewSizeMm is
  /// the size of the visible slice region. A zero field of view lifts the restriction.
  virtual void SetSlicePlane(vtkMatrix4x4* sliceXYToRAS, const double fieldOfViewSizeMm[2]);

  ///@{
  /// Connect the sampled points into cells (for contouring, tracing, deformed grid). Off by default, because
  /// glyphs only need the points and the cells of a large lattice are costly.
  vtkSetMacro(GenerateCells, bool);
  vtkGetMacro(GenerateCells, bool);
  vtkBooleanMacro(GenerateCells, bool);
  ///@}

  /// Number of sampled points along each axis of the lattice, as produced by the last
  /// update. All zero if the samples are not arranged in a lattice.
  vtkGetVector3Macro(OutputLatticeSize, int);

  /// Sampling positions to use instead of a regular lattice, in RAS. Optional. The positions are copied, and setting
  /// the same ones again does not modify the sampler, so they can be set on every update without sampling again.
  virtual void SetSamplePositions(vtkPoints* samplePositions_RAS);
  vtkPoints* GetSamplePositions();

  ///@{
  /// Transform from the coordinates the field is stored in to world (e.g. parent transforms of its node). If set, sample
  /// positions are world positions and vectors are transformed into world directions (as slice views need). Ignored by transforms.
  virtual void SetFieldToWorldTransform(vtkAbstractTransform* fieldToWorld);
  vtkAbstractTransform* GetFieldToWorldTransform();
  ///@}

  /// Modified time, including the modified time of the object that is sampled: a modified
  /// image, mesh, fitted transform or transform is re-sampled on the next render.
  vtkMTimeType GetMTime() override;

  /// Fill samplePositions_RAS with the points of a lattice: latticeToRAS maps the lattice
  /// indices to RAS, latticeSize is the number of points along each of its axes.
  static void GetLatticePositions(vtkMatrix4x4* latticeToRAS, const int latticeSize[3], vtkPoints* samplePositions_RAS);

protected:
  vtkVectorFieldSampler();
  ~vtkVectorFieldSampler() override;
  vtkVectorFieldSampler(const vtkVectorFieldSampler&) = delete;
  void operator=(const vtkVectorFieldSampler&) = delete;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  /// Samples the field with the method of the current source at the computed sample positions; the method fills
  /// the output with FinishOutput(), or leaves it empty if there is no field.
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;
  void ImageRequestData(vtkImageData* image, vtkUnstructuredGrid* output, vtkPoints* samplePositions_RAS, const int latticeSize[3]);
  void MeshRequestData(vtkPointSet* mesh, vtkUnstructuredGrid* output, vtkPoints* samplePositions_RAS, const int latticeSize[3]);
  void ScatteredRequestData(vtkUnstructuredGrid* output, vtkPoints* samplePositions_RAS, const int latticeSize[3]);
  void TransformRequestData(vtkUnstructuredGrid* output, vtkPoints* samplePositions_RAS, const int latticeSize[3]);

  /// Fill the output with the samples: points, vectors (SetOutputVectors()), the valid-sample mask and colors
  /// (AddOutputColors()) if any, and the lattice cells if GenerateCells is on.
  void FinishOutput(vtkUnstructuredGrid* output,
                    vtkPoints* samplePositions_RAS,
                    const int latticeSize[3],
                    vtkDataArray* vectors_RAS,
                    vtkUnsignedCharArray* validSamples = nullptr,
                    vtkDataArray* colorValues = nullptr,
                    int colorComponent = -1);

  /// Modified time of the sampled object (image, mesh, fitted transforms, transform), so
  /// that the sampler re-executes when the field itself changes.
  vtkMTimeType GetSampledObjectMTime();

  /// Compute the sample positions in RAS from, in order of preference: the set sample positions, a lattice in the slice
  /// plane, in the sampling region, or GetDefaultSamplePositions(). latticeSize is zeros if they do not form a lattice.
  void ComputeSamplePositions(vtkPoints* samplePositions_RAS, int latticeSize[3]);

  /// True if a slice plane with a field of view is set (see SetSlicePlane()).
  bool HasSlicePlane();
  /// Half of the slice thickness, in mm: how far from the slice plane a sample still counts as
  /// in the slice. A thickness that is not positive counts as 1 mm.
  double GetHalfSliceThickness();
  /// The sampling spacing, or the default of the source when it is 0.
  double GetEffectiveSamplingSpacing();

  /// Sampling spacing when SamplingSpacing is 0: the largest voxel size of an image; 1/50 of the bounds diagonal of a
  /// mesh or scattered data (sampling them is costly, so keep the count modest); 1 for a transform.
  double GetDefaultSamplingSpacing();

  /// Bounds of the region where the field has values, in RAS, so a slice view does not sample its whole field of view.
  /// Returns false for a field without bounds, such as a transform.
  bool GetFieldBounds(double bounds_RAS[6]);
  /// The bounds of the image, in RAS: the bounds of its eight corners.
  bool ImageGetFieldBounds(double bounds_RAS[6]);
  /// The bounds of the scattered data points themselves (the field bounds are grown by the
  /// distance up to which the field is drawn around them).
  bool ScatteredGetPointBounds(double bounds_RAS[6]);

  /// Positions to sample when no sample positions, slice plane, or region is set: a lattice over the field bounds
  /// at the sampling spacing. Returns false for a transform, which has no bounds.
  bool GetDefaultSamplePositions(vtkPoints* samplePositions_RAS, int latticeSize[3]);
  /// A lattice over the whole image, stepping over voxels so that the samples are
  /// SamplingSpacing apart.
  bool ImageGetDefaultSamplePositions(vtkPoints* samplePositions_RAS, int latticeSize[3]);

  /// The locator that finds the nearest scattered data point. Built once for the data points
  /// and kept until they change.
  vtkStaticPointLocator* ScatteredGetPointLocator();

  /// Fill samplePositions_RAS with an axis-aligned lattice covering bounds_RAS at the given spacing, centered on the
  /// bounds. Returns false if the spacing is not positive.
  static bool GetLatticePositionsInBounds(const double bounds_RAS[6], double spacingMm, vtkPoints* samplePositions_RAS, int latticeSize[3]);

  /// True if a field to world transform is set and it does more than nothing.
  bool HasFieldToWorldTransform();

  /// The positions, in the coordinates of the field, of world positions. A copy of the world
  /// positions if no field to world transform is set.
  void GetFieldPositions(vtkPoints* positions_World, vtkPoints* positions_Field);

  /// Transform vectors sampled at the given field positions into world directions, using the derivative of the field to
  /// world transform at each position (correct for non-linear transforms). Does nothing without that transform.
  void TransformVectorsToWorld(vtkPoints* positions_Field, vtkDataArray* vectors);

  /// Remove the positions outside the field bounds (GetFieldBounds()), where they would only draw empty glyphs.
  /// Not used for lattices, which keep all positions for their cells.
  void RemovePositionsOutsideField(vtkPoints* samplePositions_RAS);

  /// Add the vector array and the magnitude array to the output point set.
  /// vectors must have 3 components and as many tuples as outputPointSet has points.
  static void SetOutputVectors(vtkPointSet* outputPointSet, vtkDataArray* vectors_RAS);

  /// Add the color array (GetColorArrayName()): the values, or for multi-component arrays the given component or the
  /// magnitude (component -1 or out of range). Samples marked invalid in validSamples get 0. One tuple per point.
  static void AddOutputColors(vtkPointSet* outputPointSet, vtkDataArray* values, vtkUnsignedCharArray* validSamples = nullptr, int component = -1);

  /// Connect the points of a lattice (first axis varying fastest) with voxel cells. If validSamples is given, a cell
  /// is only created where all of its corners are valid, so the cells cover the field and not its bounding box.
  static void GenerateLatticeCells(vtkUnstructuredGrid* outputGrid, const int latticeSize[3], vtkUnsignedCharArray* validSamples = nullptr);

  /// Set by RequestData(). Also stores the size in the field data of the output, so that
  /// downstream filters can use it.
  void SetOutputLatticeSize(vtkUnstructuredGrid* output, const int latticeSize[3]);

  int OutputLatticeSize[3]{ 0, 0, 0 };
  bool GenerateCells{ false };

  vtkNew<vtkMatrix4x4> RegionToRAS;
  int RegionSize[3]{ 0, 0, 0 };

  vtkNew<vtkMatrix4x4> SliceXYToRAS;
  double FieldOfViewSize[2]{ 0.0, 0.0 };

  double SamplingSpacing{ 0.0 };
  int LatticeGroupSize{ 1 };
  double SliceThickness{ 1.0 };

  vtkSmartPointer<vtkPoints> SamplePositions;
  vtkSmartPointer<vtkAbstractTransform> FieldToWorldTransform;

  int FieldSource{ FieldSourceNone };

  vtkNew<vtkMatrix4x4> IJKToRAS;
  bool VectorsInRAS{ true };

  char* MeshVectorArrayName{ nullptr };
  char* MeshColorArrayName{ nullptr };
  int MeshColorArrayComponent{ -1 };

  vtkSmartPointer<vtkScatteredBSplineTransform> ScatteredFittedTransform;
  vtkSmartPointer<vtkScatteredBSplineTransform> ScatteredColorFittedTransform;
  double ScatteredMaximumDistance{ 0.0 };
  vtkSmartPointer<vtkStaticPointLocator> ScatteredPointLocator;
  vtkWeakPointer<vtkPointSet> ScatteredPointLocatorPoints;
  vtkMTimeType ScatteredPointLocatorPointsMTime{ 0 };

  vtkSmartPointer<vtkAbstractTransform> Transform;
};

#endif
