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

#ifndef __vtkMRMLVectorFieldSamplePointSource_h
#define __vtkMRMLVectorFieldSamplePointSource_h

// MRML includes
#include "vtkSlicerVectorFieldRenderingModuleMRMLExport.h"

// VTK includes
#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkGeneralTransform;
class vtkMatrix4x4;
class vtkMRMLMarkupsCurveNode;
class vtkMRMLMarkupsPlaneNode;
class vtkMRMLNode;
class vtkMRMLVectorFieldDisplayNode;
class vtkMRMLVolumeNode;
class vtkPoints;
class vtkPolyData;

/// \brief Computes where a vector field is sampled, from the sampling region node of a vector field display node.
///
/// Input is the vector field display node (from SetDisplayNode()), whose sampling region node (ROI, plane, slice, point list,
/// curve, model, segmentation, volume) and settings choose the region part and the kind of sampling.
///
/// The filter takes the points of the region or steps along a curve, or draw random positions in its box, rectangle
/// or surface (with a fixed seed, so the same node gives the same ones).
///
/// Outputs the sample positions or the streamline seeds (by SetOutputPoints()), as the points of a poly data, in world
/// coordinates; no points when the samples are not a list of positions. Computed again only when the display node,
/// the region node, the displayable node or what defines them (mesh, image, segments, transforms) changes.
///
/// \sa vtkMRMLVectorFieldDisplayNode, vtkVectorFieldSampler
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_MRML_EXPORT vtkMRMLVectorFieldSamplePointSource : public vtkPolyDataAlgorithm
{
public:
  static vtkMRMLVectorFieldSamplePointSource* New();
  vtkTypeMacro(vtkMRMLVectorFieldSamplePointSource, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /// The display node whose sample points are computed. Not owned.
  void SetDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode);
  vtkMRMLVectorFieldDisplayNode* GetDisplayNode();
  ///@}

  enum OutputPointsTypes
  {
    OutputSamplePositions, ///< GetSamplePositions()
    OutputStreamlineSeeds, ///< GetStreamlineSeedPositions()
    OutputPoints_Last
  };

  ///@{
  /// Which points are output. Default is OutputSamplePositions.
  vtkSetClampMacro(OutputPoints, int, OutputSamplePositions, OutputPoints_Last - 1);
  vtkGetMacro(OutputPoints, int);
  ///@}

  /// Includes the modified time of the display node, its sampling region node and displayable node, and what defines
  /// them (mesh, image, segments, parent transforms).
  vtkMTimeType GetMTime() override;

  /// True if a node has the given part (points, surface, or inside) to sample, when it is the
  /// sampling region of a field that is stored in another node.
  static bool IsPartSupported(vtkMRMLNode* regionNode, int part);

  /// The part that suits a node best: points of a point list, curve or model, surface of a plane or slice, else inside.
  static int GetPreferredPart(vtkMRMLNode* regionNode);

  /// True if the part can be sampled on the sampling region node or, if none is set, on the source: a model has all
  /// three parts, a volume its voxels and inside, a transform only the inside.
  static bool IsSamplingPartSupported(vtkMRMLVectorFieldDisplayNode* displayNode, int part);

  /// The part a display node samples: its SamplingRegionPart if supported, else the GetPreferredPart() of the node.
  static int GetEffectiveSamplingPart(vtkMRMLVectorFieldDisplayNode* displayNode);

  /// True if glyphs go at (all or a random subset of) the points of the mesh carrying the field: always if the field
  /// cannot be evaluated between them, else if no other region node is set and the points or surface are sampled.
  static bool IsSamplingOwnPoints(vtkMRMLVectorFieldDisplayNode* displayNode);

  /// True if the geometry must be built on a lattice: for any non-model source and the deformed grid; for model isosurfaces
  /// and streamlines only without volumetric cells or when interpolated (mesh cells are exact and work for thin shells).
  static bool NeedsLatticeSampling(vtkMRMLVectorFieldDisplayNode* displayNode);

  /// Samples of the current mode: the source's own (mesh points, or a lattice over the whole field), a lattice over the
  /// sampling region (GetSamplingLattice()), or a list of positions (GetSamplePositions()).
  enum SamplingKind
  {
    SamplingKindSource = 0,
    SamplingKindLattice,
    SamplingKindPositions
  };
  static int GetSamplingKind(vtkMRMLVectorFieldDisplayNode* displayNode);

  /// Lattice over the sampling region (ROI along its axes, plane or slice in its plane, else the bounding box); latticeToRAS
  /// axes are scaled by the spacing. False if no region node is set, or its points or random samples are used.
  static bool GetSamplingLattice(vtkMRMLVectorFieldDisplayNode* displayNode, vtkMatrix4x4* latticeToRAS, int latticeSize[3]);

  /// Samples as a list of world positions (node points, curve steps, random or surface samples). False when they are a
  /// lattice (GetSamplingLattice()), the mesh points of the source (IsSamplingOwnPoints()), or the source lattice.
  static bool GetSamplePositions(vtkMRMLVectorFieldDisplayNode* displayNode, vtkPoints* samplePositions_RAS);

  /// Streamline start positions (world): the region samples, else model points or a volume lattice (or random); a transform
  /// needs a region node. All are returned (see vtkMRMLVectorFieldPipeline::SetMaximumNumberOfStreamlineSeeds()).
  static bool GetStreamlineSeedPositions(vtkMRMLVectorFieldDisplayNode* displayNode, vtkPoints* seedPositions_RAS);

  /// Centered lattice spacingMm apart (0: ~20 samples on the longest axis; not for slices) in a slice or plane, an ROI box,
  /// or another node's bounding box, holding whole groups of groupSize steps per axis (complete grid cells).
  static bool GetLattice(vtkMRMLNode* regionNode, double spacingMm, int groupSize, vtkMatrix4x4* latticeToRAS, int latticeSize[3]);

  /// A lattice in an oriented box (sizeMm, centered on the origin of objectToWorld, along its
  /// axes), as GetLattice() makes it. A size of 0 along an axis makes it flat.
  static bool GetOrientedLattice(vtkMatrix4x4* objectToWorld, const double sizeMm[3], double spacingMm, int groupSize, vtkMatrix4x4* latticeToRAS, int latticeSize[3]);

  /// The points of any node that has some: the control points of a markups node, the mesh
  /// points of a model, or the voxel centers of a volume.
  static bool GetNodePointPositions(vtkMRMLNode* pointsNode, vtkPoints* positions_RAS);

  /// Positions along a curve, spacingMm apart (a tenth of its length if 0).
  static bool GetCurvePositions(vtkMRMLMarkupsCurveNode* curveNode, double spacingMm, vtkPoints* positions_RAS);

  /// Voxel centers of a volume, every spacingMm along each axis (every voxel if 0).
  static bool GetVoxelLatticePositions(vtkMRMLVolumeNode* volumeNode, double spacingMm, vtkPoints* positions_RAS);

  /// Random positions in the box of a node (see GetNodeBox()).
  static bool GetRandomPositionsInBox(vtkMRMLNode* node, int numberOfPositions, vtkPoints* positions_RAS);

  /// Random positions on the rectangle of a plane or of what a slice view shows. Returns false
  /// for other nodes.
  static bool GetRandomPositionsInRectangle(vtkMRMLNode* node, int numberOfPositions, vtkPoints* positions_RAS);

  /// A random subset of the points of a surface, spread evenly over its area.
  static bool GetRandomPositionsOnSurface(vtkPolyData* surface, int numberOfPositions, vtkPoints* positions_RAS);

  /// A random subset of numberOfPositions of the positions, each one at most once, or all of
  /// them if there are not more than that. Always the same subset for the same positions.
  static void SelectRandomSubset(vtkPoints* positions, int numberOfPositions, vtkPoints* subset);

  /// Box enclosing a node: ROI box, plane or slice view rectangle (flat), box of volume voxel centers, else bounding box.
  /// boxToWorld maps the coordinates of bounds to world.
  static bool GetNodeBox(vtkMRMLNode* node, vtkMatrix4x4* boxToWorld, double bounds[6]);

  /// Plane rectangle: centerToWorld maps a frame centered on it (plane axes) to world, size is its extent. Not always
  /// centered on the plane origin: a plane sized by its control points keeps its origin at the first point.
  static void GetPlaneRectangle(vtkMRMLMarkupsPlaneNode* planeNode, vtkMatrix4x4* centerToWorld, double size[2]);

  /// The surface of a model (the boundary of a volumetric mesh), or of the segments of a
  /// segmentation, in world coordinates. nullptr if the node has none.
  static vtkSmartPointer<vtkPolyData> GetNodeSurfaceInWorld(vtkMRMLNode* node);

  /// The transform from the coordinates of a node to world, through its parent transforms.
  static void GetNodeToWorldTransform(vtkMRMLNode* node, vtkGeneralTransform* nodeToWorld);

protected:
  vtkMRMLVectorFieldSamplePointSource();
  ~vtkMRMLVectorFieldSamplePointSource() override;
  vtkMRMLVectorFieldSamplePointSource(const vtkMRMLVectorFieldSamplePointSource&) = delete;
  void operator=(const vtkMRMLVectorFieldSamplePointSource&) = delete;

  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  vtkWeakPointer<vtkMRMLVectorFieldDisplayNode> DisplayNode;
  int OutputPoints{ OutputSamplePositions };
};

#endif
