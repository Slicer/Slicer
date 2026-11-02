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

#ifndef __vtkMRMLVectorFieldPipeline_h
#define __vtkMRMLVectorFieldPipeline_h

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLVectorFieldSamplePointSource.h"
#include "vtkVectorFieldGeometry.h"
#include "vtkVectorFieldGlyph.h"
#include "vtkVectorFieldSampler.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkAbstractTransform;
class vtkAlgorithm;
class vtkAlgorithmOutput;
class vtkCellCenters;
class vtkCellDataToPointData;
class vtkDataArray;
class vtkDataObject;
class vtkGeneralTransform;
class vtkImageAlgorithm;
class vtkMatrix4x4;
class vtkMRMLModelNode;
class vtkMRMLNode;
class vtkMRMLTransformNode;
class vtkMRMLVectorFieldDisplayNode;
class vtkMRMLVectorFieldMask;
class vtkMRMLVolumeNode;
class vtkPointDataToCellData;
class vtkPointSet;
class vtkPoints;
class vtkPolyData;
class vtkScatteredBSplineTransform;
class vtkTransformFilter;
class vtkTransformPolyDataFilter;

/// \brief Builds and owns the VTK filters that display the field of a vector field display node.
///
/// This object creates and manages the VTK filters needed to visualize the vector field.
/// Its input is the display node, which provides the settings for the visualization and
/// access to the underlying displayable node (vector volume, model with a vector array, or transform).
/// Each Get...Connection() method reads the display node settings, configures the underlying filters,
/// and returns their output.
///
/// 3D views share all the filters, which are updated when the display node changes.
/// Each slice view has its own sampler, geometry, glyph and mask filters,
/// and configures them with the Update...() methods.
///
/// \sa vtkMRMLVectorFieldDisplayNode, vtkVectorFieldGeometry, vtkVectorFieldGlyph
class VTK_MRML_EXPORT vtkMRMLVectorFieldPipeline : public vtkObject
{
public:
  static vtkMRMLVectorFieldPipeline* New();
  vtkTypeMacro(vtkMRMLVectorFieldPipeline, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// The display node whose field this is, which owns this object
  void SetDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode);

  /// The sample positions of the display node, in world coordinates, computed again only when what defines them
  /// changes (see vtkMRMLVectorFieldSamplePointSource). nullptr when the samples are not a list of positions.
  vtkPoints* GetSamplePositions();
  /// The streamline seeds of the display node, in world coordinates, all of them. nullptr if there are none.
  vtkPoints* GetStreamlineSeedPositions();

  /// The connection that produces the field of a model or of a vector volume, in the
  /// coordinates of the node that stores it (vtkMRMLVectorFieldDisplayNode::GetFieldConnection()).
  vtkAlgorithmOutput* GetFieldConnection();

  /// GetFieldConnection() plus the shown-region array if HasRegionTest(): glyphs are placed where it is 1, grid lines and
  /// isosurfaces are cut where it changes. Streamlines are not cut: they stop where they leave the masking region.
  vtkAlgorithmOutput* GetMaskedFieldConnection();

  /// The field moved into the world when the parent transform is not linear, the masked
  /// field otherwise
  vtkAlgorithmOutput* GetWorldFieldConnection();

  /// Grid, contour or streamline geometry for the 3D views (shared filters), in the world if the parent transform is not
  /// linear. nullptr in glyph mode or if the source lacks the needed cells. Slice views build their own.
  vtkAlgorithmOutput* GetOutputPolyDataConnection();

  /// Undeformed grid lines for the 3D views, from the same filter as GetOutputPolyDataConnection().
  /// nullptr if they are not shown.
  vtkAlgorithmOutput* GetNonWarpedGridConnection();

  /// Thresholded streamline lines (no tubes) in field coordinates, traced once and shared by all 3D and slice views.
  /// nullptr if not in streamline mode or there is nothing to trace.
  vtkAlgorithmOutput* GetStreamlineConnection();

  /// See vtkMRMLVectorFieldDisplayNode::UpdateSliceFieldSampler()
  bool UpdateSliceFieldSampler(vtkVectorFieldSampler* sampler);

  /// Configure a geometry filter for the visualization mode, on the given field; flat (slice views) draws lines instead
  /// of tubes. Returns false in glyph mode or when there is nothing to draw (e.g. no streamline seeds).
  bool UpdateGeometryFilter(vtkVectorFieldGeometry* geometry, vtkAlgorithmOutput* fieldConnection, bool flat);

  /// The glyph filter of the 3D views
  vtkVectorFieldGlyph* GetGlyphFilter() { return this->Glyph; }
  /// Configure which points of the field get a glyph (threshold, region, random subset) and the glyph shape, orientation
  /// and scale. Slice views pass their plane to get flat outlines. Returns false if there is no field.
  bool UpdateGlyphFilter(vtkVectorFieldGlyph* glyph, vtkAlgorithmOutput* fieldConnection, vtkMatrix4x4* slicePlaneToRAS = nullptr);
  /// Configure only the 3D glyph shape from the glyph type and geometry settings
  void UpdateGlyphShape(vtkVectorFieldGlyph* glyph);
  /// True if the glyph points carry the directional scale array: the glyphs are directionally
  /// scaled, and there are arrays to orient and to scale them by.
  static bool IsDirectionalScaleUsed(vtkMRMLVectorFieldDisplayNode* displayNode);

  ///@{
  /// Largest number of streamlines traced; more seeds than this are reduced to an evenly spread random subset.
  /// Default is 5000.
  vtkSetClampMacro(MaximumNumberOfStreamlineSeeds, int, 1, VTK_INT_MAX);
  vtkGetMacro(MaximumNumberOfStreamlineSeeds, int);
  ///@}

  /// Push the region, the sampling spacing and the sample positions into a sampler.
  void UpdateSamplerRegion(vtkVectorFieldSampler* sampler);

  /// Set the regions that the points have to be in, or out of, on a region filter. Slice
  /// views use it on the samples they take in their plane, which are in world coordinates.
  void UpdateMask(vtkMRMLVectorFieldMask* mask);
  /// Set only the masking region on a region filter, to mark the part of the field that streamlines are traced through.
  /// Returns false if there is no masking region.
  bool UpdateMaskingRegion(vtkMRMLVectorFieldMask* mask);

  /// Field to world: the parent transforms of the volume or model carrying the field; nullptr if none or the node is a
  /// transform. Samples, sampling region and seeds are in the world, GetFieldConnection() in field coordinates.
  vtkAbstractTransform* GetFieldToWorldTransform();
  /// GetFieldToWorldTransform() if not linear, else nullptr: views apply a linear one to the actor, much cheaper than
  /// moving every point. Used by GetWorldFieldConnection() and GetOutputPolyDataConnection().
  vtkAbstractTransform* GetNonLinearFieldToWorldTransform();
  /// Positions in the coordinates of the field (see GetFieldToWorldTransform()) of positions
  /// in the world. A copy if the field is in the world.
  void GetFieldPositions(vtkPoints* positions_World, vtkPoints* positions_Field);

  /// The surface of a model or of the segments of a segmentation, in world coordinates, kept
  /// until the node changes. nullptr if the node has none.
  vtkPolyData* GetSamplingSurfaceInWorld(vtkMRMLNode* node);

  /// Distance from the data points up to which the interpolated field is drawn: InterpolationMaximumDistance, or the
  /// automatic one when that is 0. Returns 0 if the field is not interpolated.
  double GetEffectiveInterpolationMaximumDistance();
  /// Largest difference between the fitted field and the vectors at the data points (vector units); computes the fit if
  /// needed. Returns -1 if the field is not interpolated or the fit failed.
  double GetInterpolationResidual();

  ///@{
  /// B-spline fits (vtkScatteredBSplineTransform) of vectors and color values at scattered points, for vtkVectorFieldSampler.
  /// Only configured here: the fit is computed when first evaluated, shared by all views. No display node needed.
  /// UpdateVectorFit: fit of the 3-component array over the point bounds grown by maximumDistanceMm (0: a tenth of their
  /// diagonal), how far from the points the field is drawn. nullptr if the array is missing.
  /// GetFittedMaximumDistance: the distance the last UpdateVectorFit() grew the bounds by.
  /// UpdateColorFit: fit of a component (>= 0) or the magnitude (-1) of the values on the vector fit's lattice. nullptr if
  /// there is no vector fit or the values do not match the points.
  vtkScatteredBSplineTransform* UpdateVectorFit(vtkPointSet* points, const char* vectorArrayName, double maximumDistanceMm);
  vtkGetMacro(FittedMaximumDistance, double);
  vtkScatteredBSplineTransform* UpdateColorFit(vtkPointSet* points, vtkDataArray* values, int component);
  ///@}

  /// Samples the field for the 3D views; not used for a mesh drawn at its own points.
  vtkVectorFieldSampler* GetFieldSampler() { return this->FieldSampler; }

  /// Output data object of a connection, updated first if update is true (else as last executed, nullptr before the
  /// first run). nullptr if there is no connection.
  static vtkDataObject* GetConnectionOutput(vtkAlgorithmOutput* connection, bool update);

protected:
  vtkMRMLVectorFieldPipeline();
  ~vtkMRMLVectorFieldPipeline() override;
  vtkMRMLVectorFieldPipeline(const vtkMRMLVectorFieldPipeline&) = delete;
  void operator=(const vtkMRMLVectorFieldPipeline&) = delete;

  /// The transform node that the node carrying the field is under, or nullptr if there is
  /// none, or the node is a transform, which is evaluated in the world whatever it is under.
  vtkMRMLTransformNode* GetParentTransformNode();

  ///@{
  /// Configure a sampler to read the field of a vector volume, to interpolate it in the cells of a mesh, or to evaluate
  /// the fit of the vectors at the points of a mesh. Returns false if the sampler cannot be used.
  bool UpdateVolumeFieldSampler(vtkMRMLVolumeNode* volumeNode, vtkVectorFieldSampler* sampler);
  bool UpdateMeshFieldSampler(vtkMRMLModelNode* modelNode, vtkVectorFieldSampler* sampler);
  bool UpdateScatteredFieldSampler(vtkMRMLModelNode* modelNode, vtkVectorFieldSampler* sampler);
  ///@}

  /// Vector fit of the mesh points with input and parameters up to date, shared by the samplers of all views so it is
  /// computed once. Does not compute the fit; nullptr if there is nothing to fit.
  vtkScatteredBSplineTransform* UpdateInterpolationTransform();

  /// Fit of the color values (vtkMRMLVectorFieldDisplayNode::GetSampledColorArrayName()) on the points and lattice of the
  /// vector fit. nullptr when colored by its own magnitude or there is nothing to fit.
  vtkScatteredBSplineTransform* UpdateColorInterpolationTransform();

  /// The points that the field is fitted to: the mesh, or the centers of its cells for a
  /// cell data field.
  vtkPointSet* GetInterpolationPoints();

  /// The points that carry the field as point data: the mesh itself for a point data array,
  /// and the centers of its cells for a cell data array.
  vtkAlgorithmOutput* GetMeshFieldPointsConnection(vtkMRMLModelNode* modelNode);

  /// The mesh, keeping its cells, with the field and color arrays averaged onto the points if they are cell data:
  /// isosurfaces and streamlines are built from point data.
  vtkAlgorithmOutput* GetMeshFieldCellsConnection(vtkMRMLModelNode* modelNode);

  /// The mesh with every cell data array averaged onto its points too, keeping its cells and
  /// its own point data.
  vtkAlgorithmOutput* GetMeshCellDataOnPointsConnection(vtkMRMLModelNode* modelNode);

  /// What produces the field, looking through the filters that move it into the world or mark where it is shown, which
  /// keep the names of its arrays.
  static vtkAlgorithm* GetFieldProducer(vtkAlgorithmOutput* fieldConnection);
  /// Name of the vector array of a field: the sampler's own name for a sampled field, the chosen array otherwise
  const char* GetFieldVectorArrayName(vtkAlgorithmOutput* fieldConnection);

  /// The display node that owns this object
  vtkMRMLVectorFieldDisplayNode* DisplayNode{ nullptr };

  vtkNew<vtkVectorFieldSampler> FieldSampler;
  /// Sample positions and streamline seeds, shared by all views
  vtkNew<vtkMRMLVectorFieldSamplePointSource> SamplePointSource;
  vtkNew<vtkMRMLVectorFieldSamplePointSource> SeedPointSource;
  /// Grid, contour and streamline geometry of the 3D views; its streamlines are shared with the slice views
  vtkNew<vtkVectorFieldGeometry> Geometry;
  /// Glyphs of the 3D views
  vtkNew<vtkVectorFieldGlyph> Glyph;
  /// Marks the part of the field that the masking region shows, which streamlines are traced through
  vtkSmartPointer<vtkMRMLVectorFieldMask> StreamlineMask;
  int MaximumNumberOfStreamlineSeeds{ 5000 };

  /// The fit of the vectors at the points of a mesh (see UpdateVectorFit())
  vtkSmartPointer<vtkScatteredBSplineTransform> VectorFit;
  bool VectorFitValid{ false };
  double FittedMaximumDistance{ 0.0 };
  /// The fit of the values the field is colored by, and the points with those values that it
  /// is fitted to, kept until the values or the points change (see UpdateColorFit())
  vtkSmartPointer<vtkScatteredBSplineTransform> ColorFit;
  vtkSmartPointer<vtkPolyData> ColorFitPoints;
  vtkWeakPointer<vtkDataArray> ColorFitSourceArray;
  vtkMTimeType ColorFitSourceMTime{ 0 };
  vtkMTimeType ColorFitPointsMTime{ 0 };
  int ColorFitComponent{ -1 };

  /// Adds the array that says where the field is shown to the field of the 3D views
  vtkSmartPointer<vtkMRMLVectorFieldMask> Mask;
  /// The surface of the sampling region node, kept until the node changes
  vtkSmartPointer<vtkPolyData> SamplingSurface;
  vtkWeakPointer<vtkMRMLNode> SamplingSurfaceNode;
  vtkMTimeType SamplingSurfaceMTime{ 0 };
  bool SamplingSurfaceValid{ false };
  /// The transform from the coordinates of the field to world that the region filter uses
  vtkSmartPointer<vtkGeneralTransform> FieldToWorldTransform;
  vtkWeakPointer<vtkMRMLTransformNode> FieldToWorldTransformParent;
  vtkMTimeType FieldToWorldTransformMTime{ 0 };
  /// Moves the field into the world for the 3D views when the parent transform is not
  /// linear (see GetNonLinearFieldToWorldTransform())
  vtkSmartPointer<vtkTransformFilter> FieldToWorldWarper;
  /// Moves streamline geometry into the world for 3D views when the parent transform is not linear (streamlines are
  /// traced in field coordinates because slice views share them)
  vtkSmartPointer<vtkTransformPolyDataFilter> StreamlineToWorldWarper;

  /// RAS image of a vector volume that streamlines are traced through directly (vtkImageFieldSource, local to the .cxx)
  vtkSmartPointer<vtkImageAlgorithm> StreamlineImageSource;

  /// Moves the cell data of a mesh to the centers of its cells. The point data is averaged
  /// onto the cells first, so that arrays used for scaling or coloring come along.
  vtkSmartPointer<vtkPointDataToCellData> PointDataToCellData;
  vtkSmartPointer<vtkCellCenters> CellCenters;
  /// Averages the cell data of a mesh onto its points, keeping the cells.
  vtkSmartPointer<vtkCellDataToPointData> CellDataToPointData;
};

#endif
