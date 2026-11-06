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

// MRML includes
#include "vtkMRMLVectorFieldSamplePointSource.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkMRMLVectorFieldMask.h"
#include "vtkMRMLVectorFieldPipeline.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkVectorFieldSampler.h"

// Segmentation includes
#include <vtkIdList.h>
#include <vtkSegment.h>
#include <vtkSegmentation.h>
#include <vtkSegmentationConverter.h>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkGeneralTransform.h>
#include <vtkGeometryFilter.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkImageData.h>
#include <vtkMaskPoints.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkTransformPolyDataFilter.h>

// STD includes
#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

namespace
{
//----------------------------------------------------------------------------
/// Random positions in the node box (vtkMRMLVectorFieldSamplePointSource::GetNodeBox()) over its first numberOfAxes axes
/// (2: rectangle, 3: box). The same positions every time for the same node.
bool GetRandomPositionsInNodeBox(vtkMRMLNode* node, int numberOfAxes, int numberOfPositions, vtkPoints* positions_RAS)
{
  if (!positions_RAS)
  {
    return false;
  }
  positions_RAS->Initialize();
  vtkNew<vtkMatrix4x4> boxToWorld;
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  if (!vtkMRMLVectorFieldSamplePointSource::GetNodeBox(node, boxToWorld, bounds))
  {
    return false;
  }
  vtkNew<vtkMinimalStandardRandomSequence> random01;
  random01->SetSeed(1);
  positions_RAS->SetNumberOfPoints(numberOfPositions);
  for (int positionIndex = 0; positionIndex < numberOfPositions; ++positionIndex)
  {
    double position_Box[4] = { 0.0, 0.0, 0.0, 1.0 };
    for (int axis = 0; axis < numberOfAxes; ++axis)
    {
      position_Box[axis] = bounds[2 * axis] + random01->GetNextValue() * (bounds[2 * axis + 1] - bounds[2 * axis]);
    }
    double position_World[4] = { 0.0, 0.0, 0.0, 1.0 };
    boxToWorld->MultiplyPoint(position_Box, position_World);
    positions_RAS->SetPoint(positionIndex, position_World);
  }
  return (numberOfPositions > 0);
}
} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLVectorFieldSamplePointSource);

//----------------------------------------------------------------------------
vtkMRMLVectorFieldSamplePointSource::vtkMRMLVectorFieldSamplePointSource()
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkMRMLVectorFieldSamplePointSource::~vtkMRMLVectorFieldSamplePointSource() = default;

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldSamplePointSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "DisplayNode: " << (this->DisplayNode ? this->DisplayNode->GetID() : "(none)") << "\n";
  os << indent << "OutputPoints: " << this->OutputPoints << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldSamplePointSource::SetDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  if (this->DisplayNode == displayNode)
  {
    return;
  }
  this->DisplayNode = displayNode;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode* vtkMRMLVectorFieldSamplePointSource::GetDisplayNode()
{
  return this->DisplayNode;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkMRMLVectorFieldSamplePointSource::GetMTime()
{
  vtkMTimeType mTime = this->Superclass::GetMTime();
  if (!this->DisplayNode)
  {
    return mTime;
  }
  mTime = std::max(mTime, this->DisplayNode->GetMTime());
  // Where the region and the source are, and what defines their points, inside or surface
  mTime = std::max(mTime, vtkMRMLVectorFieldMask::GetRegionMTime(this->DisplayNode->GetSamplingRegionNode()));
  mTime = std::max(mTime, vtkMRMLVectorFieldMask::GetRegionMTime(this->DisplayNode->GetDisplayableNode()));
  return mTime;
}

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldSamplePointSource::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  if (!output)
  {
    return 0;
  }
  vtkNew<vtkPoints> points;
  if (this->DisplayNode)
  {
    if (this->OutputPoints == OutputStreamlineSeeds)
    {
      vtkMRMLVectorFieldSamplePointSource::GetStreamlineSeedPositions(this->DisplayNode, points);
    }
    else
    {
      vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(this->DisplayNode, points);
    }
  }
  output->SetPoints(points);
  return 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::IsPartSupported(vtkMRMLNode* regionNode, int part)
{
  if (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartDefault)
  {
    return true;
  }
  if (vtkMRMLMarkupsROINode::SafeDownCast(regionNode))
  {
    return (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  }
  if (vtkMRMLMarkupsPlaneNode::SafeDownCast(regionNode) || vtkMRMLSliceNode::SafeDownCast(regionNode))
  {
    return (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  }
  if (vtkMRMLMarkupsNode::SafeDownCast(regionNode))
  {
    // A point list, a curve, or any other markups: its points
    return (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints);
  }
  if (vtkMRMLModelNode::SafeDownCast(regionNode))
  {
    return true;
  }
  if (vtkMRMLSegmentationNode::SafeDownCast(regionNode))
  {
    return (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface || part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  }
  if (vtkMRMLVolumeNode::SafeDownCast(regionNode))
  {
    return (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints || part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  }
  // Any other displayable node: its bounding box
  return (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
}

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldSamplePointSource::GetPreferredPart(vtkMRMLNode* regionNode)
{
  if (vtkMRMLMarkupsPlaneNode::SafeDownCast(regionNode) || vtkMRMLSliceNode::SafeDownCast(regionNode))
  {
    return vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface;
  }
  if (vtkMRMLMarkupsROINode::SafeDownCast(regionNode))
  {
    return vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume;
  }
  if (vtkMRMLMarkupsNode::SafeDownCast(regionNode) || vtkMRMLModelNode::SafeDownCast(regionNode))
  {
    return vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints;
  }
  return vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::IsSamplingPartSupported(vtkMRMLVectorFieldDisplayNode* displayNode, int part)
{
  if (!displayNode)
  {
    return false;
  }
  if (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartDefault)
  {
    return true;
  }
  vtkMRMLNode* regionNode = displayNode->GetSamplingRegionNodeOtherThanSource();
  if (!regionNode)
  {
    // The source itself: a model has all three parts, a volume its voxels and its inside, and
    // a transform, which is defined everywhere, has no extent of its own
    if (vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode()))
    {
      // A cell data field is drawn at the cell centers, points without cells: no surface to spread samples over, and an
      // inside only if the field can be evaluated between them
      if (displayNode->IsFieldArrayCellData())
      {
        return (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints //
                || (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume && displayNode->CanSampleAtArbitraryPositions()));
      }
      return true;
    }
    if (vtkMRMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode()))
    {
      return (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints || part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
    }
    return (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  }
  return vtkMRMLVectorFieldSamplePointSource::IsPartSupported(regionNode, part);
}

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldSamplePointSource::GetEffectiveSamplingPart(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume;
  }
  int requestedPart = displayNode->GetSamplingRegionPart();
  if (requestedPart != vtkMRMLVectorFieldDisplayNode::SamplingRegionPartDefault && vtkMRMLVectorFieldSamplePointSource::IsSamplingPartSupported(displayNode, requestedPart))
  {
    return requestedPart;
  }
  // The part that suits the node: the points of a node that is a set of points (a point
  // list, a curve, a model), the surface of a flat one, the inside of the others
  vtkMRMLNode* regionNode = displayNode->GetSamplingRegionNodeOtherThanSource();
  if (!regionNode)
  {
    regionNode = displayNode->GetDisplayableNode();
  }
  int preferredPart = vtkMRMLVectorFieldSamplePointSource::GetPreferredPart(regionNode);
  if (vtkMRMLVectorFieldSamplePointSource::IsSamplingPartSupported(displayNode, preferredPart))
  {
    return preferredPart;
  }
  // A volume has voxels, but sampling each of them is rarely what is wanted, so its points
  // come last
  for (int part : { vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume,
                    vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface,
                    vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints })
  {
    if (vtkMRMLVectorFieldSamplePointSource::IsSamplingPartSupported(displayNode, part))
    {
      return part;
    }
  }
  return vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  if (!displayNode || !vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode()))
  {
    return false;
  }
  // A field that is only known at the points of the mesh can only be drawn there
  if (!displayNode->CanSampleAtArbitraryPositions())
  {
    return true;
  }
  if (displayNode->GetSamplingRegionNodeOtherThanSource())
  {
    return false;
  }
  // The inside of a mesh that can be evaluated between its points is sampled on a lattice or
  // at random; its points and its surface are its own points
  return (vtkMRMLVectorFieldSamplePointSource::GetEffectiveSamplingPart(displayNode) != vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::NeedsLatticeSampling(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return false;
  }
  if (!vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode()))
  {
    return true;
  }
  // A deformed grid is a lattice by definition. Isosurfaces and streamlines use the mesh cells: cheaper, and a shell about
  // one cell thick has almost no lattice points inside, so a lattice would give no isosurface at any resolution.
  if (displayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid)
  {
    return true;
  }
  // A mesh without volumetric cells, or a field fitted to the points (which extends beyond them), needs a lattice too
  return (displayNode->IsPolyDataVisualizationMode() && (!displayNode->HasVolumetricMesh() || displayNode->IsInterpolationUsed()));
}

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldSamplePointSource::GetSamplingKind(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return SamplingKindSource;
  }
  vtkMRMLNode* regionNode = displayNode->GetSamplingRegionNodeOtherThanSource();
  int part = vtkMRMLVectorFieldSamplePointSource::GetEffectiveSamplingPart(displayNode);
  bool random = (displayNode->GetSamplingPattern() == vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  bool flatRegion = (vtkMRMLMarkupsPlaneNode::SafeDownCast(regionNode) || vtkMRMLSliceNode::SafeDownCast(regionNode));
  // Grids and isosurfaces are always built on a lattice, over a region that has an inside
  // or a plane, or over the whole field
  bool latticeOnly = (displayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid //
                      || displayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  if (latticeOnly)
  {
    if (regionNode && (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume || flatRegion))
    {
      return SamplingKindLattice;
    }
    return SamplingKindSource;
  }
  if (!regionNode)
  {
    if (vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(displayNode))
    {
      return SamplingKindSource;
    }
    // A volume, a transform, or the inside of a mesh: a lattice over the field, or random
    // positions in it, or the voxels of a volume
    if (random || part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints)
    {
      return SamplingKindPositions;
    }
    return SamplingKindSource;
  }
  if (!random && (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume || flatRegion))
  {
    return SamplingKindLattice;
  }
  return SamplingKindPositions;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetSamplingLattice(vtkMRMLVectorFieldDisplayNode* displayNode, vtkMatrix4x4* latticeToRAS, int latticeSize[3])
{
  if (!displayNode || !latticeToRAS || !latticeSize || vtkMRMLVectorFieldSamplePointSource::GetSamplingKind(displayNode) != SamplingKindLattice)
  {
    return false;
  }
  return vtkMRMLVectorFieldSamplePointSource::GetLattice(displayNode->GetSamplingRegionNode(),
                                                         displayNode->GetSamplingSpacingForField(), //
                                                         displayNode->GetGridSubdivision(),
                                                         latticeToRAS,
                                                         latticeSize);
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(vtkMRMLVectorFieldDisplayNode* displayNode, vtkPoints* samplePositions_RAS)
{
  if (!displayNode || !samplePositions_RAS)
  {
    return false;
  }
  samplePositions_RAS->Initialize();
  if (vtkMRMLVectorFieldSamplePointSource::GetSamplingKind(displayNode) != SamplingKindPositions)
  {
    return false;
  }
  vtkMRMLNode* regionNode = displayNode->GetSamplingRegionNodeOtherThanSource();
  if (!regionNode)
  {
    // The source itself, sampled where it cannot give a lattice: at random in its extent,
    // or at the voxels of a volume
    regionNode = displayNode->GetDisplayableNode();
  }
  int part = vtkMRMLVectorFieldSamplePointSource::GetEffectiveSamplingPart(displayNode);
  bool random = (displayNode->GetSamplingPattern() == vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  int numberOfSamples = displayNode->GetSamplingPointsNumber();

  if (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume)
  {
    // At random in the inside
    return vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsInBox(regionNode, numberOfSamples, samplePositions_RAS);
  }

  if (part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface)
  {
    // A plane or a slice: at random over its rectangle (a lattice over it is not a list of
    // positions, see GetSamplingLattice())
    if (vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsInRectangle(regionNode, numberOfSamples, samplePositions_RAS))
    {
      return true;
    }
    // The surface of a model or of segments: its points about the spacing apart, or a random
    // subset of them that is spread evenly over the area of the surface
    // The surface is kept by the shared pipeline, so that it is not extracted again on every update
    vtkSmartPointer<vtkMRMLVectorFieldPipeline> pipeline = vtkMRMLVectorFieldPipeline::GetSharedPipeline(displayNode);
    vtkPolyData* surface_World = pipeline->GetSamplingSurfaceInWorld(regionNode);
    if (!surface_World || surface_World->GetNumberOfPoints() == 0)
    {
      return false;
    }
    if (!random)
    {
      vtkNew<vtkIdList> pointIds;
      vtkVectorFieldSampler::GetSpacedPointIds(surface_World->GetPoints(), displayNode->GetSamplingSpacing(), pointIds);
      samplePositions_RAS->SetNumberOfPoints(pointIds->GetNumberOfIds());
      surface_World->GetPoints()->GetPoints(pointIds, samplePositions_RAS);
      return true;
    }
    return vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsOnSurface(surface_World, numberOfSamples, samplePositions_RAS);
  }

  // Points: steps along a curve, or the points of a node
  vtkMRMLMarkupsCurveNode* curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(regionNode);
  if (curveNode && !random)
  {
    return vtkMRMLVectorFieldSamplePointSource::GetCurvePositions(curveNode, displayNode->GetSamplingSpacing(), samplePositions_RAS);
  }
  vtkNew<vtkPoints> nodePositions_RAS;
  if (curveNode)
  {
    nodePositions_RAS->DeepCopy(curveNode->GetCurvePointsWorld());
  }
  else if (!vtkMRMLVectorFieldSamplePointSource::GetNodePointPositions(regionNode, nodePositions_RAS))
  {
    return false;
  }
  if (random)
  {
    vtkMRMLVectorFieldSamplePointSource::SelectRandomSubset(nodePositions_RAS, numberOfSamples, samplePositions_RAS);
  }
  else
  {
    samplePositions_RAS->DeepCopy(nodePositions_RAS);
  }
  return (samplePositions_RAS->GetNumberOfPoints() > 0);
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetStreamlineSeedPositions(vtkMRMLVectorFieldDisplayNode* displayNode, vtkPoints* seedPositions_RAS)
{
  if (!displayNode || !seedPositions_RAS)
  {
    return false;
  }
  seedPositions_RAS->Initialize();
  bool random = (displayNode->GetSamplingPattern() == vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  if (!displayNode->GetSamplingRegionNodeOtherThanSource())
  {
    // The source itself: the points of a model, or some of them at random
    vtkMRMLDisplayableNode* sourceNode = displayNode->GetDisplayableNode();
    if (vtkMRMLModelNode::SafeDownCast(sourceNode))
    {
      vtkNew<vtkPoints> meshPositions_RAS;
      if (!vtkMRMLVectorFieldSamplePointSource::GetNodePointPositions(sourceNode, meshPositions_RAS))
      {
        return false;
      }
      if (random)
      {
        vtkMRMLVectorFieldSamplePointSource::SelectRandomSubset(meshPositions_RAS, displayNode->GetSamplingPointsNumber(), seedPositions_RAS);
      }
      else
      {
        seedPositions_RAS->DeepCopy(meshPositions_RAS);
      }
      return (seedPositions_RAS->GetNumberOfPoints() > 0);
    }
    // A volume: random positions in it, or a lattice over it
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(sourceNode);
    if (!volumeNode)
    {
      // A transform is defined everywhere, so it has nothing to start streamlines from
      return false;
    }
    if (random)
    {
      return vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, seedPositions_RAS);
    }
    return vtkMRMLVectorFieldSamplePointSource::GetVoxelLatticePositions(volumeNode, displayNode->GetSamplingSpacing(), seedPositions_RAS);
  }
  vtkNew<vtkMatrix4x4> latticeToRAS;
  int latticeSize[3] = { 0, 0, 0 };
  if (vtkMRMLVectorFieldSamplePointSource::GetSamplingLattice(displayNode, latticeToRAS, latticeSize))
  {
    vtkVectorFieldSampler::GetLatticePositions(latticeToRAS, latticeSize, seedPositions_RAS);
    return (seedPositions_RAS->GetNumberOfPoints() > 0);
  }
  return vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, seedPositions_RAS);
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetLattice(vtkMRMLNode* regionNode, double spacingMm, int groupSize, vtkMatrix4x4* regionToRAS, int regionSize[3])
{
  if (!regionNode || !regionToRAS || !regionSize)
  {
    return false;
  }
  groupSize = std::max(1, groupSize);

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(regionNode);
  if (sliceNode)
  {
    // A slice defines an oriented plane: sample a flat region in the plane of the slice.
    double fieldOfView[3] = { 0.0, 0.0, 0.0 };
    sliceNode->GetFieldOfView(fieldOfView);
    if (spacingMm <= 0.0 || fieldOfView[0] <= 0.0 || fieldOfView[1] <= 0.0)
    {
      return false;
    }
    regionToRAS->DeepCopy(sliceNode->GetSliceToRAS());
    // Scale the in-plane axes by the spacing and move the origin to the corner; whole groups, so the grid draws complete
    // cells, centered in the field of view
    double origin_RAS[3] = { regionToRAS->GetElement(0, 3), regionToRAS->GetElement(1, 3), regionToRAS->GetElement(2, 3) };
    for (int axis = 0; axis < 2; ++axis)
    {
      int numberOfSteps = std::max(groupSize, static_cast<int>(fieldOfView[axis] / (spacingMm * groupSize)) * groupSize);
      double axisVector[3] = { regionToRAS->GetElement(0, axis), regionToRAS->GetElement(1, axis), regionToRAS->GetElement(2, axis) };
      vtkMath::Normalize(axisVector);
      for (int row = 0; row < 3; ++row)
      {
        origin_RAS[row] -= 0.5 * numberOfSteps * spacingMm * axisVector[row];
        regionToRAS->SetElement(row, axis, axisVector[row] * spacingMm);
      }
      regionSize[axis] = numberOfSteps + 1;
    }
    for (int row = 0; row < 3; ++row)
    {
      regionToRAS->SetElement(row, 3, origin_RAS[row]);
    }
    regionSize[2] = 1;
    return true;
  }

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(regionNode);
  if (roiNode)
  {
    // An oriented box: sample along its own axes, not along the RAS axes.
    double roiSizeMm[3] = { 0.0, 0.0, 0.0 };
    roiNode->GetSize(roiSizeMm);
    return vtkMRMLVectorFieldSamplePointSource::GetOrientedLattice(roiNode->GetObjectToWorldMatrix(), roiSizeMm, spacingMm, groupSize, regionToRAS, regionSize);
  }

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(regionNode);
  if (planeNode)
  {
    // An oriented rectangle: a flat region in the plane.
    double planeSizeMm[2] = { 0.0, 0.0 };
    vtkNew<vtkMatrix4x4> rectangleToWorld;
    vtkMRMLVectorFieldSamplePointSource::GetPlaneRectangle(planeNode, rectangleToWorld, planeSizeMm);
    double regionSizeMm[3] = { planeSizeMm[0], planeSizeMm[1], 0.0 };
    return vtkMRMLVectorFieldSamplePointSource::GetOrientedLattice(rectangleToWorld, regionSizeMm, spacingMm, groupSize, regionToRAS, regionSize);
  }

  // Any other displayable node (a model, a volume) bounds the sampled region by its
  // axis-aligned bounding box in RAS.
  vtkMRMLDisplayableNode* displayableRegionNode = vtkMRMLDisplayableNode::SafeDownCast(regionNode);
  if (!displayableRegionNode)
  {
    vtkGenericWarningMacro("vtkMRMLVectorFieldSamplePointSource::GetLattice: unsupported region node type " << regionNode->GetClassName());
    return false;
  }
  double bounds_RAS[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  displayableRegionNode->GetRASBounds(bounds_RAS);
  if (bounds_RAS[1] < bounds_RAS[0] || bounds_RAS[3] < bounds_RAS[2] || bounds_RAS[5] < bounds_RAS[4])
  {
    return false;
  }
  if (spacingMm <= 0.0)
  {
    // Aim for a reasonable number of glyphs along the largest axis of the region
    double largestSizeMm = std::max(std::max(bounds_RAS[1] - bounds_RAS[0], bounds_RAS[3] - bounds_RAS[2]), bounds_RAS[5] - bounds_RAS[4]);
    spacingMm = (largestSizeMm > 0.0 ? largestSizeMm / 20.0 : 1.0);
  }
  regionToRAS->Identity();
  for (int axis = 0; axis < 3; ++axis)
  {
    regionToRAS->SetElement(axis, axis, spacingMm);
    regionToRAS->SetElement(axis, 3, bounds_RAS[2 * axis]);
    regionSize[axis] = static_cast<int>((bounds_RAS[2 * axis + 1] - bounds_RAS[2 * axis]) / spacingMm) + 1;
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetOrientedLattice(vtkMatrix4x4* objectToWorld,
                                                             const double sizeMm[3],
                                                             double spacingMm,
                                                             int groupSize,
                                                             vtkMatrix4x4* regionToRAS,
                                                             int regionSize[3])
{
  if (!objectToWorld)
  {
    return false;
  }
  if (spacingMm <= 0.0)
  {
    // Aim for a reasonable number of samples along the largest axis of the region
    double largestSizeMm = std::max(std::max(sizeMm[0], sizeMm[1]), sizeMm[2]);
    spacingMm = (largestSizeMm > 0.0 ? largestSizeMm / 20.0 : 1.0);
  }

  // A whole number of groups along each axis, so the grid draws complete cells, centered by splitting the leftover
  // between both sides
  groupSize = std::max(1, groupSize);
  int numberOfSteps[3] = { 0, 0, 0 };
  double corner_Object[4] = { 0.0, 0.0, 0.0, 1.0 };
  for (int axis = 0; axis < 3; ++axis)
  {
    if (sizeMm[axis] <= 0.0)
    {
      // A flat region (a plane): a single layer of samples in it
      regionSize[axis] = 1;
      continue;
    }
    // Rounding, because a region size that is a whole number of steps may be computed as a
    // hair less than that
    numberOfSteps[axis] = std::max(groupSize, static_cast<int>(sizeMm[axis] / (spacingMm * groupSize) + 0.5) * groupSize);
    corner_Object[axis] = -0.5 * numberOfSteps[axis] * spacingMm;
    regionSize[axis] = numberOfSteps[axis] + 1;
  }
  double corner_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
  objectToWorld->MultiplyPoint(corner_Object, corner_RAS);

  regionToRAS->DeepCopy(objectToWorld);
  for (int axis = 0; axis < 3; ++axis)
  {
    double axisVector[3] = { objectToWorld->GetElement(0, axis), objectToWorld->GetElement(1, axis), objectToWorld->GetElement(2, axis) };
    vtkMath::Normalize(axisVector);
    for (int row = 0; row < 3; ++row)
    {
      regionToRAS->SetElement(row, axis, axisVector[row] * spacingMm);
    }
  }
  for (int row = 0; row < 3; ++row)
  {
    regionToRAS->SetElement(row, 3, corner_RAS[row]);
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetNodePointPositions(vtkMRMLNode* pointsNode, vtkPoints* positions_RAS)
{
  if (!positions_RAS)
  {
    return false;
  }
  positions_RAS->Initialize();
  if (!pointsNode)
  {
    return false;
  }

  // A markups node: its control points
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(pointsNode);
  if (markupsNode)
  {
    int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
    positions_RAS->SetNumberOfPoints(numberOfControlPoints);
    for (int controlPointIndex = 0; controlPointIndex < numberOfControlPoints; ++controlPointIndex)
    {
      double position_World[3] = { 0.0, 0.0, 0.0 };
      markupsNode->GetNthControlPointPositionWorld(controlPointIndex, position_World);
      positions_RAS->SetPoint(controlPointIndex, position_World);
    }
    return (numberOfControlPoints > 0);
  }

  vtkNew<vtkGeneralTransform> nodeToWorld;
  vtkMRMLVectorFieldSamplePointSource::GetNodeToWorldTransform(pointsNode, nodeToWorld);

  // A model or any other node with a mesh: the points of that mesh, in world coordinates
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(pointsNode);
  if (modelNode)
  {
    vtkPointSet* mesh = modelNode->GetMesh();
    if (!mesh || !mesh->GetPoints())
    {
      return false;
    }
    vtkPoints* meshPoints = mesh->GetPoints();
    vtkIdType numberOfPoints = meshPoints->GetNumberOfPoints();
    positions_RAS->SetNumberOfPoints(numberOfPoints);
    for (vtkIdType pointIndex = 0; pointIndex < numberOfPoints; ++pointIndex)
    {
      double position_Mesh[3] = { 0.0, 0.0, 0.0 };
      double position_World[3] = { 0.0, 0.0, 0.0 };
      meshPoints->GetPoint(pointIndex, position_Mesh);
      nodeToWorld->TransformPoint(position_Mesh, position_World);
      positions_RAS->SetPoint(pointIndex, position_World);
    }
    return (numberOfPoints > 0);
  }

  // A volume: the center of each of its voxels. This is as many glyphs as there are voxels,
  // so it is only practical for a small volume.
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(pointsNode);
  if (volumeNode)
  {
    vtkImageData* image = volumeNode->GetImageData();
    if (!image)
    {
      return false;
    }
    vtkNew<vtkMatrix4x4> ijkToRAS;
    volumeNode->GetIJKToRASMatrix(ijkToRAS);
    int extent[6] = { 0, -1, 0, -1, 0, -1 };
    image->GetExtent(extent);
    for (int k = extent[4]; k <= extent[5]; ++k)
    {
      for (int j = extent[2]; j <= extent[3]; ++j)
      {
        for (int i = extent[0]; i <= extent[1]; ++i)
        {
          double position_IJK[4] = { static_cast<double>(i), static_cast<double>(j), static_cast<double>(k), 1.0 };
          double position_Node[4] = { 0.0, 0.0, 0.0, 1.0 };
          ijkToRAS->MultiplyPoint(position_IJK, position_Node);
          double position_World[3] = { 0.0, 0.0, 0.0 };
          nodeToWorld->TransformPoint(position_Node, position_World);
          positions_RAS->InsertNextPoint(position_World);
        }
      }
    }
    return (positions_RAS->GetNumberOfPoints() > 0);
  }

  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetCurvePositions(vtkMRMLMarkupsCurveNode* curveNode, double spacingMm, vtkPoints* positions_RAS)
{
  vtkPoints* curvePoints = (curveNode ? curveNode->GetCurvePointsWorld() : nullptr);
  if (!positions_RAS || !curvePoints || curvePoints->GetNumberOfPoints() == 0)
  {
    return false;
  }
  positions_RAS->Initialize();
  // A curve can have thousands of points along it, so they are taken the sampling spacing
  // apart
  double lengthMm = curveNode->GetCurveLengthWorld();
  if (spacingMm <= 0.0)
  {
    spacingMm = (lengthMm > 0.0 ? lengthMm / 10.0 : 0.0);
  }
  int numberOfSteps = 1;
  if (spacingMm > 0.0 && lengthMm > 0.0)
  {
    numberOfSteps = std::max(1, static_cast<int>(lengthMm / spacingMm) + 1);
  }
  numberOfSteps = std::min(numberOfSteps, static_cast<int>(curvePoints->GetNumberOfPoints()));
  for (int stepIndex = 0; stepIndex < numberOfSteps; ++stepIndex)
  {
    vtkIdType curvePointIndex = (numberOfSteps > 1 ? (curvePoints->GetNumberOfPoints() - 1) * stepIndex / (numberOfSteps - 1) : 0);
    positions_RAS->InsertNextPoint(curvePoints->GetPoint(curvePointIndex));
  }
  return (positions_RAS->GetNumberOfPoints() > 0);
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetVoxelLatticePositions(vtkMRMLVolumeNode* volumeNode, double spacingMm, vtkPoints* positions_RAS)
{
  if (!volumeNode || !positions_RAS)
  {
    return false;
  }
  positions_RAS->Initialize();
  vtkNew<vtkMatrix4x4> ijkToRAS;
  double extent[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  if (!vtkMRMLVectorFieldSamplePointSource::GetNodeBox(volumeNode, ijkToRAS, extent))
  {
    return false;
  }
  double* voxelSpacing = volumeNode->GetSpacing();
  double steps_IJK[3] = { 1.0, 1.0, 1.0 };
  for (int axis = 0; axis < 3; ++axis)
  {
    steps_IJK[axis] = (spacingMm > 0.0 && voxelSpacing[axis] > 0.0 ? spacingMm / voxelSpacing[axis] : 1.0);
  }
  for (double k = extent[4]; k <= extent[5]; k += steps_IJK[2])
  {
    for (double j = extent[2]; j <= extent[3]; j += steps_IJK[1])
    {
      for (double i = extent[0]; i <= extent[1]; i += steps_IJK[0])
      {
        double position_IJK[4] = { i, j, k, 1.0 };
        double position_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
        ijkToRAS->MultiplyPoint(position_IJK, position_RAS);
        positions_RAS->InsertNextPoint(position_RAS);
      }
    }
  }
  return (positions_RAS->GetNumberOfPoints() > 0);
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsInBox(vtkMRMLNode* node, int numberOfPositions, vtkPoints* positions_RAS)
{
  // In the box of a region of interest, or in the bounding box of any other node, where the
  // ones outside of a segmentation or a model are dropped later
  return GetRandomPositionsInNodeBox(node, 3, numberOfPositions, positions_RAS);
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsInRectangle(vtkMRMLNode* node, int numberOfPositions, vtkPoints* positions_RAS)
{
  if (!node || !(node->IsA("vtkMRMLMarkupsPlaneNode") || node->IsA("vtkMRMLSliceNode")))
  {
    return false;
  }
  return GetRandomPositionsInNodeBox(node, 2, numberOfPositions, positions_RAS);
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsOnSurface(vtkPolyData* surface, int numberOfPositions, vtkPoints* positions_RAS)
{
  if (!positions_RAS || !surface || surface->GetNumberOfPoints() == 0)
  {
    return false;
  }
  positions_RAS->Initialize();
  vtkNew<vtkMaskPoints> subset;
  subset->SetInputData(surface);
  subset->RandomModeOn();
  subset->SetRandomModeType(vtkMaskPoints::UNIFORM_SPATIAL_SURFACE);
  subset->SetMaximumNumberOfPoints(numberOfPositions);
  subset->SetRandomSeed(1);
  subset->Update();
  if (!subset->GetOutput()->GetPoints())
  {
    return false;
  }
  positions_RAS->DeepCopy(subset->GetOutput()->GetPoints());
  return (positions_RAS->GetNumberOfPoints() > 0);
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldSamplePointSource::SelectRandomSubset(vtkPoints* positions, int numberOfPositions, vtkPoints* subset)
{
  if (positions->GetNumberOfPoints() <= numberOfPositions)
  {
    subset->DeepCopy(positions);
    return;
  }
  std::vector<vtkIdType> pointIds(positions->GetNumberOfPoints());
  std::iota(pointIds.begin(), pointIds.end(), 0);
  vtkNew<vtkMinimalStandardRandomSequence> random01;
  random01->SetSeed(1);
  for (vtkIdType i = static_cast<vtkIdType>(pointIds.size()) - 1; i > 0; --i)
  {
    vtkIdType j = static_cast<vtkIdType>(random01->GetNextValue() * (i + 1)) % (i + 1);
    std::swap(pointIds[i], pointIds[j]);
  }
  subset->SetNumberOfPoints(numberOfPositions);
  for (int sampleIndex = 0; sampleIndex < numberOfPositions; ++sampleIndex)
  {
    subset->SetPoint(sampleIndex, positions->GetPoint(pointIds[sampleIndex]));
  }
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldSamplePointSource::GetNodeBox(vtkMRMLNode* node, vtkMatrix4x4* boxToWorld, double bounds[6])
{
  boxToWorld->Identity();
  if (vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(node))
  {
    double size[3] = { 0.0, 0.0, 0.0 };
    roiNode->GetSize(size);
    boxToWorld->DeepCopy(roiNode->GetObjectToWorldMatrix());
    for (int axis = 0; axis < 3; ++axis)
    {
      bounds[2 * axis] = -0.5 * size[axis];
      bounds[2 * axis + 1] = 0.5 * size[axis];
    }
    return true;
  }
  if (vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(node))
  {
    double size[2] = { 0.0, 0.0 };
    vtkMRMLVectorFieldSamplePointSource::GetPlaneRectangle(planeNode, boxToWorld, size);
    double planeBounds[6] = { -0.5 * size[0], 0.5 * size[0], -0.5 * size[1], 0.5 * size[1], 0.0, 0.0 };
    std::copy(planeBounds, planeBounds + 6, bounds);
    return (size[0] > 0.0 && size[1] > 0.0);
  }
  if (vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node))
  {
    double fieldOfView[3] = { 0.0, 0.0, 0.0 };
    sliceNode->GetFieldOfView(fieldOfView);
    boxToWorld->DeepCopy(sliceNode->GetSliceToRAS());
    double sliceBounds[6] = { -0.5 * fieldOfView[0], 0.5 * fieldOfView[0], -0.5 * fieldOfView[1], 0.5 * fieldOfView[1], 0.0, 0.0 };
    std::copy(sliceBounds, sliceBounds + 6, bounds);
    return (fieldOfView[0] > 0.0 && fieldOfView[1] > 0.0);
  }
  if (vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node))
  {
    // The box of its voxel centers, along its own axes: the field is only known between them
    vtkImageData* image = volumeNode->GetImageData();
    if (!image)
    {
      return false;
    }
    int extent[6] = { 0, -1, 0, -1, 0, -1 };
    image->GetExtent(extent);
    if (extent[0] > extent[1] || extent[2] > extent[3] || extent[4] > extent[5])
    {
      return false;
    }
    vtkMRMLTransformNode* parentTransformNode = volumeNode->GetParentTransformNode();
    if (parentTransformNode && !parentTransformNode->IsTransformToWorldLinear())
    {
      // A box cannot follow a warped volume: the box in world that encloses it
      volumeNode->GetRASBounds(bounds);
      return (bounds[0] <= bounds[1] && bounds[2] <= bounds[3] && bounds[4] <= bounds[5]);
    }
    volumeNode->GetIJKToRASMatrix(boxToWorld);
    if (parentTransformNode)
    {
      vtkNew<vtkMatrix4x4> nodeToWorld;
      parentTransformNode->GetMatrixTransformToWorld(nodeToWorld);
      vtkMatrix4x4::Multiply4x4(nodeToWorld, boxToWorld, boxToWorld);
    }
    for (int axis = 0; axis < 6; ++axis)
    {
      bounds[axis] = extent[axis];
    }
    return true;
  }
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!displayableNode)
  {
    return false;
  }
  displayableNode->GetRASBounds(bounds);
  return (bounds[0] <= bounds[1] && bounds[2] <= bounds[3] && bounds[4] <= bounds[5]);
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldSamplePointSource::GetPlaneRectangle(vtkMRMLMarkupsPlaneNode* planeNode, vtkMatrix4x4* centerToWorld, double size[2])
{
  double planeBounds[4] = { 0.0, 0.0, 0.0, 0.0 };
  planeNode->GetPlaneBounds(planeBounds);
  size[0] = planeBounds[1] - planeBounds[0];
  size[1] = planeBounds[3] - planeBounds[2];
  vtkNew<vtkMatrix4x4> objectToWorld;
  planeNode->GetObjectToWorldMatrix(objectToWorld);
  vtkNew<vtkMatrix4x4> centerToObject;
  centerToObject->SetElement(0, 3, 0.5 * (planeBounds[0] + planeBounds[1]));
  centerToObject->SetElement(1, 3, 0.5 * (planeBounds[2] + planeBounds[3]));
  vtkMatrix4x4::Multiply4x4(objectToWorld, centerToObject, centerToWorld);
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkMRMLVectorFieldSamplePointSource::GetNodeSurfaceInWorld(vtkMRMLNode* node)
{
  vtkNew<vtkAppendPolyData> surfaces;
  if (vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node))
  {
    vtkPointSet* mesh = modelNode->GetMesh();
    if (!mesh || mesh->GetNumberOfPoints() == 0)
    {
      return nullptr;
    }
    vtkNew<vtkGeometryFilter> boundary;
    boundary->SetInputData(mesh);
    surfaces->AddInputConnection(boundary->GetOutputPort());
  }
  else if (vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node))
  {
    // The surfaces the segments are shown with in 3D; if missing, they are made in a temporary copy sharing the source
    // representation, so the segmentation is not changed
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    if (!segmentation)
    {
      return nullptr;
    }
    std::string closedSurfaceName = vtkSegmentationConverter::GetClosedSurfaceRepresentationName();
    vtkSmartPointer<vtkSegmentation> surfaceSegmentation = segmentation;
    if (!segmentation->ContainsRepresentation(closedSurfaceName))
    {
      surfaceSegmentation = vtkSmartPointer<vtkSegmentation>::New();
      std::string sourceName = segmentation->GetSourceRepresentationName();
      surfaceSegmentation->SetSourceRepresentationName(sourceName);
      surfaceSegmentation->CopyConversionParameters(segmentation);
      std::vector<std::string> sourceSegmentIDs;
      segmentation->GetSegmentIDs(sourceSegmentIDs);
      for (const std::string& segmentID : sourceSegmentIDs)
      {
        vtkSegment* segment = segmentation->GetSegment(segmentID);
        vtkDataObject* sourceRepresentation = (segment ? segment->GetRepresentation(sourceName) : nullptr);
        if (!sourceRepresentation)
        {
          continue;
        }
        vtkNew<vtkSegment> segmentCopy;
        segmentCopy->DeepCopyMetadata(segment);
        segmentCopy->AddRepresentation(sourceName, sourceRepresentation);
        surfaceSegmentation->AddSegment(segmentCopy, segmentID);
      }
      if (!surfaceSegmentation->CreateRepresentation(closedSurfaceName))
      {
        return nullptr;
      }
    }
    std::vector<std::string> segmentIDs;
    surfaceSegmentation->GetSegmentIDs(segmentIDs);
    for (const std::string& segmentID : segmentIDs)
    {
      vtkSegment* segment = surfaceSegmentation->GetSegment(segmentID);
      vtkPolyData* segmentSurface = vtkPolyData::SafeDownCast(segment ? segment->GetRepresentation(closedSurfaceName) : nullptr);
      if (segmentSurface && segmentSurface->GetNumberOfPoints() > 0)
      {
        surfaces->AddInputData(segmentSurface);
      }
    }
    if (surfaces->GetNumberOfInputConnections(0) == 0)
    {
      return nullptr;
    }
  }
  else
  {
    return nullptr;
  }
  vtkNew<vtkGeneralTransform> nodeToWorld;
  vtkMRMLVectorFieldSamplePointSource::GetNodeToWorldTransform(node, nodeToWorld);
  vtkNew<vtkTransformPolyDataFilter> toWorld;
  toWorld->SetInputConnection(surfaces->GetOutputPort());
  toWorld->SetTransform(nodeToWorld);
  toWorld->Update();
  return toWorld->GetOutput();
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldSamplePointSource::GetNodeToWorldTransform(vtkMRMLNode* node, vtkGeneralTransform* nodeToWorld)
{
  nodeToWorld->Identity();
  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(node);
  if (transformableNode && transformableNode->GetParentTransformNode())
  {
    vtkMRMLTransformNode::GetTransformBetweenNodes(transformableNode->GetParentTransformNode(), nullptr, nodeToWorld);
  }
}
