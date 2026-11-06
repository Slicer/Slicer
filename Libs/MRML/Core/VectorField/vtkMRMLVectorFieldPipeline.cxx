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
#include "vtkMRMLVectorFieldPipeline.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkMRMLVectorFieldMask.h"
#include "vtkMRMLVectorFieldSamplePointSource.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkVectorFieldSampler.h"

// vtkAddon includes
#include <vtkScatteredBSplineTransform.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCellCenters.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStreamTracer.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>

// STD includes
#include <algorithm>
#include <cmath>

namespace
{

/// Name of the array that the values the field is colored by are fitted as
const char ColorValueArrayName[] = "ColorValue";

//----------------------------------------------------------------------------
/// \brief A vector volume as an oriented RAS image with the sampler array names, so streamlines need no cell locator.
/// Voxel vectors are taken as RAS. vtkVectorFieldSampler::ProducesSampledField() recognizes this class by name.
class vtkImageFieldSource : public vtkImageAlgorithm
{
public:
  static vtkImageFieldSource* New();
  vtkTypeMacro(vtkImageFieldSource, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Geometry of the volume: maps voxel indices to RAS.
  void SetIJKToRAS(vtkMatrix4x4* ijkToRAS);

protected:
  vtkImageFieldSource();
  ~vtkImageFieldSource() override = default;
  vtkImageFieldSource(const vtkImageFieldSource&) = delete;
  void operator=(const vtkImageFieldSource&) = delete;

  int RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  /// The origin, spacing and direction of the image in RAS, from IJKToRAS.
  void GetImageGeometry(double origin[3], double spacing[3], double direction[9]);

  vtkSmartPointer<vtkMatrix4x4> IJKToRAS;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkImageFieldSource);

//----------------------------------------------------------------------------
vtkImageFieldSource::vtkImageFieldSource()
{
  this->IJKToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
}

//----------------------------------------------------------------------------
void vtkImageFieldSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "IJKToRAS:\n";
  this->IJKToRAS->PrintSelf(os, indent.GetNextIndent());
}

//----------------------------------------------------------------------------
void vtkImageFieldSource::SetIJKToRAS(vtkMatrix4x4* ijkToRAS)
{
  if (!ijkToRAS)
  {
    return;
  }
  bool changed = false;
  for (int row = 0; row < 4 && !changed; ++row)
  {
    for (int column = 0; column < 4 && !changed; ++column)
    {
      changed = (this->IJKToRAS->GetElement(row, column) != ijkToRAS->GetElement(row, column));
    }
  }
  if (!changed)
  {
    return;
  }
  this->IJKToRAS->DeepCopy(ijkToRAS);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkImageFieldSource::GetImageGeometry(double origin[3], double spacing[3], double direction[9])
{
  // The columns of IJKToRAS are the voxel axes scaled by the voxel size, and its last
  // column is the position of the first voxel
  for (int column = 0; column < 3; ++column)
  {
    double axis[3] = { this->IJKToRAS->GetElement(0, column), this->IJKToRAS->GetElement(1, column), this->IJKToRAS->GetElement(2, column) };
    spacing[column] = vtkMath::Norm(axis);
    for (int row = 0; row < 3; ++row)
    {
      direction[row * 3 + column] = (spacing[column] > 0.0 ? axis[row] / spacing[column] : (row == column ? 1.0 : 0.0));
    }
    origin[column] = this->IJKToRAS->GetElement(column, 3);
  }
}

//----------------------------------------------------------------------------
int vtkImageFieldSource::RequestInformation(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  int wholeExtent[6] = { 0, -1, 0, -1, 0, -1 };
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent, 6);
  double origin[3] = { 0.0, 0.0, 0.0 };
  double spacing[3] = { 1.0, 1.0, 1.0 };
  double direction[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  this->GetImageGeometry(origin, spacing, direction);
  outInfo->Set(vtkDataObject::ORIGIN(), origin, 3);
  outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
  outInfo->Set(vtkDataObject::DIRECTION(), direction, 9);
  return 1;
}

//----------------------------------------------------------------------------
int vtkImageFieldSource::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkImageData* input = vtkImageData::GetData(inputVector[0]);
  vtkImageData* output = vtkImageData::GetData(outputVector);
  if (!output)
  {
    return 0;
  }
  output->Initialize();
  vtkDataArray* voxelVectors = (input ? input->GetPointData()->GetScalars() : nullptr);
  if (!voxelVectors || voxelVectors->GetNumberOfComponents() < 3)
  {
    return 1;
  }
  output->SetExtent(input->GetExtent());
  double origin[3] = { 0.0, 0.0, 0.0 };
  double spacing[3] = { 1.0, 1.0, 1.0 };
  double direction[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  this->GetImageGeometry(origin, spacing, direction);
  output->SetOrigin(origin);
  output->SetSpacing(spacing);
  output->SetDirectionMatrix(direction);

  vtkIdType numberOfVoxels = voxelVectors->GetNumberOfTuples();
  vtkSmartPointer<vtkDataArray> vectors;
  if (voxelVectors->GetNumberOfComponents() == 3)
  {
    // The voxels themselves, under the name that the samplers give their vectors
    vectors.TakeReference(voxelVectors->NewInstance());
    vectors->ShallowCopy(voxelVectors);
  }
  else
  {
    // The first three components are the vector
    vtkNew<vtkDoubleArray> firstComponents;
    firstComponents->SetNumberOfComponents(3);
    firstComponents->SetNumberOfTuples(numberOfVoxels);
    for (vtkIdType voxelIndex = 0; voxelIndex < numberOfVoxels; ++voxelIndex)
    {
      for (int component = 0; component < 3; ++component)
      {
        firstComponents->SetComponent(voxelIndex, component, voxelVectors->GetComponent(voxelIndex, component));
      }
    }
    vectors = firstComponents;
  }
  vectors->SetName(vtkVectorFieldSampler::GetVectorArrayName());

  vtkNew<vtkDoubleArray> magnitudes;
  magnitudes->SetName(vtkVectorFieldSampler::GetMagnitudeArrayName());
  magnitudes->SetNumberOfTuples(numberOfVoxels);
  for (vtkIdType voxelIndex = 0; voxelIndex < numberOfVoxels; ++voxelIndex)
  {
    double vector[3] = { 0.0, 0.0, 0.0 };
    vectors->GetTuple(voxelIndex, vector);
    magnitudes->SetValue(voxelIndex, vtkMath::Norm(vector));
  }

  output->GetPointData()->AddArray(vectors);
  output->GetPointData()->SetActiveVectors(vectors->GetName());
  output->GetPointData()->AddArray(magnitudes);
  output->GetPointData()->SetActiveScalars(magnitudes->GetName());
  return 1;
}

} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLVectorFieldPipeline);

//----------------------------------------------------------------------------
vtkMRMLVectorFieldPipeline::vtkMRMLVectorFieldPipeline()
{
  this->SeedPointSource->SetOutputPoints(vtkMRMLVectorFieldSamplePointSource::OutputStreamlineSeeds);
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldPipeline::SetDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  this->DisplayNode = displayNode;
  this->SamplePointSource->SetDisplayNode(displayNode);
  this->SeedPointSource->SetDisplayNode(displayNode);
}

//----------------------------------------------------------------------------
vtkPoints* vtkMRMLVectorFieldPipeline::GetSamplePositions()
{
  this->SamplePointSource->Update();
  vtkPoints* points = this->SamplePointSource->GetOutput()->GetPoints();
  return (points && points->GetNumberOfPoints() > 0 ? points : nullptr);
}

//----------------------------------------------------------------------------
vtkPoints* vtkMRMLVectorFieldPipeline::GetStreamlineSeedPositions()
{
  this->SeedPointSource->Update();
  vtkPoints* points = this->SeedPointSource->GetOutput()->GetPoints();
  return (points && points->GetNumberOfPoints() > 0 ? points : nullptr);
}

//----------------------------------------------------------------------------
vtkMRMLVectorFieldPipeline::~vtkMRMLVectorFieldPipeline() = default;

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FieldSampler: " << this->FieldSampler.GetPointer() << "\n";
  os << indent << "Mask: " << this->Mask.GetPointer() << "\n";
  os << indent << "VectorFit: " << (this->VectorFitValid ? "set" : "(none)") << "\n";
  os << indent << "FittedMaximumDistance: " << this->FittedMaximumDistance << "\n";
  os << indent << "ColorFit: " << (this->ColorFit ? "set" : "(none)") << "\n";
}

//----------------------------------------------------------------------------
vtkDataObject* vtkMRMLVectorFieldPipeline::GetConnectionOutput(vtkAlgorithmOutput* connection, bool update)
{
  vtkAlgorithm* producer = (connection ? connection->GetProducer() : nullptr);
  if (!producer)
  {
    return nullptr;
  }
  if (update)
  {
    producer->Update(connection->GetIndex());
  }
  return producer->GetOutputDataObject(connection->GetIndex());
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLVectorFieldPipeline::GetParentTransformNode()
{
  vtkMRMLDisplayableNode* displayableNode = this->DisplayNode->GetDisplayableNode();
  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(displayableNode);
  if (!transformableNode || vtkMRMLTransformNode::SafeDownCast(displayableNode))
  {
    return nullptr;
  }
  return transformableNode->GetParentTransformNode();
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldPipeline::UpdateSamplerRegion(vtkVectorFieldSampler* sampler)
{
  if (!sampler)
  {
    return;
  }
  sampler->SetLatticeGroupSize(this->DisplayNode->GetGridSubdivision());
  // Grid, contour and streamline modes are built on the cells that connect the samples;
  // glyphs only need the points
  sampler->SetGenerateCells(this->DisplayNode->IsPolyDataVisualizationMode());
  if (this->DisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline)
  {
    // Streamlines are traced through the whole field, at the resolution the source decides;
    // the sampling region says where they start (GetStreamlineSeedPositions())
    sampler->SetSamplingSpacing(0.0);
    sampler->SetSamplePositions(nullptr);
    const int noRegion[3] = { 0, 0, 0 };
    sampler->SetSamplingRegion(nullptr, noRegion);
    return;
  }
  sampler->SetSamplingSpacing(this->DisplayNode->GetSamplingSpacingForField());

  // Samples (positions, region lattice, or the source lattice) are in the world; a sampler without a field transform
  // (the one of the 3D views) samples in field coordinates, so they are moved there.
  bool toField = (!sampler->GetFieldToWorldTransform() && this->GetFieldToWorldTransform());
  vtkPoints* samplePositions_RAS = this->GetSamplePositions();
  if (samplePositions_RAS)
  {
    if (toField)
    {
      vtkNew<vtkPoints> samplePositions_Field;
      this->GetFieldPositions(samplePositions_RAS, samplePositions_Field);
      sampler->SetSamplePositions(samplePositions_Field);
    }
    else
    {
      sampler->SetSamplePositions(samplePositions_RAS);
    }
  }
  else
  {
    sampler->SetSamplePositions(nullptr);
  }

  vtkNew<vtkMatrix4x4> regionToRAS;
  int regionSize[3] = { 0, 0, 0 };
  if (!vtkMRMLVectorFieldSamplePointSource::GetSamplingLattice(this->DisplayNode, regionToRAS, regionSize))
  {
    // No lattice over a region: the source samples the whole field that it has
    regionSize[0] = 0;
    regionSize[1] = 0;
    regionSize[2] = 0;
  }
  else if (toField)
  {
    // A lattice stays a lattice under a linear transform. It cannot follow a warping one, so
    // then the lattice of the region is sampled in the coordinates of the field as it is.
    vtkMRMLTransformNode* parentTransformNode = this->GetParentTransformNode();
    if (parentTransformNode && parentTransformNode->IsTransformToWorldLinear())
    {
      vtkNew<vtkMatrix4x4> worldToField;
      parentTransformNode->GetMatrixTransformFromWorld(worldToField);
      vtkMatrix4x4::Multiply4x4(worldToField, regionToRAS, regionToRAS);
    }
  }
  sampler->SetSamplingRegion(regionToRAS, regionSize);
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldPipeline::UpdateMeshFieldSampler(vtkMRMLModelNode* modelNode, vtkVectorFieldSampler* sampler)
{
  if (!modelNode || !sampler)
  {
    return false;
  }
  sampler->SetFieldSourceToMesh();
  sampler->SetInputConnection(modelNode->GetMeshConnection());
  // The array the user chose to read the field from is the one that is interpolated, and
  // the array the field is colored by comes along
  sampler->SetMeshVectorArrayName(this->DisplayNode->GetModeVectorArrayName());
  sampler->SetMeshColorArrayName(this->DisplayNode->GetSampledColorArrayName());
  sampler->SetMeshColorArrayComponent(this->DisplayNode->GetActiveScalarComponent());
  this->UpdateSamplerRegion(sampler);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldPipeline::UpdateVolumeFieldSampler(vtkMRMLVolumeNode* volumeNode, vtkVectorFieldSampler* sampler)
{
  if (!volumeNode || !sampler)
  {
    return false;
  }
  if (sampler->GetFieldSource() != vtkVectorFieldSampler::FieldSourceImage)
  {
    sampler->SetFieldSourceToImage();
    this->DisplayNode->SetDefaultSampledFieldArrayNames();
  }
  sampler->SetInputConnection(volumeNode->GetImageDataConnection());
  vtkNew<vtkMatrix4x4> ijkToRAS;
  volumeNode->GetIJKToRASMatrix(ijkToRAS);
  sampler->SetIJKToRAS(ijkToRAS);
  // Vector components are RAS directions in Slicer whatever the voxel axes are, also when the vector type is not
  // specified (as many importers leave it)
  sampler->SetVectorsInRAS(true);
  this->UpdateSamplerRegion(sampler);
  // The volume does not report image changes (e.g. sequence browsing), so the color range is updated on every call:
  // cheap, as the array range is cached, and the first call may come before the node is attached.
  this->DisplayNode->UpdateScalarRange();
  return true;
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldPipeline::GetFieldConnection()
{
  // A model is sampled (fitted, or interpolated in its volumetric cells) when something other than its own points is
  // drawn; otherwise the mesh is glyphed as it is.
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->DisplayNode->GetDisplayableNode());
  if (modelNode)
  {
    // A mesh has no field until an array of it is chosen to be the field. Without one the
    // glyphs would be drawn at every point, neither oriented nor scaled by anything.
    const char* vectorArrayName = this->DisplayNode->GetModeVectorArrayName();
    vtkPointSet* mesh = modelNode->GetMesh();
    if (!vectorArrayName || vectorArrayName[0] == '\0' || !mesh //
        || (!mesh->GetPointData()->GetArray(vectorArrayName) && !mesh->GetCellData()->GetArray(vectorArrayName)))
    {
      return nullptr;
    }
    // The display node is read before the mesh is loaded and is not told when it arrives, so the color range is
    // updated here (as in UpdateVolumeFieldSampler)
    this->DisplayNode->UpdateScalarRange();
    bool glyphMode = (this->DisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
    bool glyphsOnOwnPoints = (glyphMode && vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(this->DisplayNode));
    bool needsSampling = (vtkMRMLVectorFieldSamplePointSource::NeedsLatticeSampling(this->DisplayNode) || (glyphMode && !glyphsOnOwnPoints));
    // A field fitted to the points of the mesh, which extends beyond it. A surface mesh or a
    // point cloud can only be sampled this way.
    if (this->DisplayNode->IsInterpolationUsed() && needsSampling)
    {
      if (this->UpdateScatteredFieldSampler(modelNode, this->FieldSampler))
      {
        return this->FieldSampler->GetOutputPort();
      }
    }
    if (this->DisplayNode->HasVolumetricMesh() && needsSampling)
    {
      if (this->UpdateMeshFieldSampler(modelNode, this->FieldSampler))
      {
        return this->FieldSampler->GetOutputPort();
      }
    }
    // Isosurfaces and streamlines of a volumetric mesh are built on its cells, from point data
    if (this->DisplayNode->IsPolyDataVisualizationMode())
    {
      return this->GetMeshFieldCellsConnection(modelNode);
    }
    // Glyphs on the points of the mesh, or on the centers of its cells for a cell data field
    return this->GetMeshFieldPointsConnection(modelNode);
  }

  // Vector volume: the field is sampled from the voxels.
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(this->DisplayNode->GetDisplayableNode());
  if (volumeNode && volumeNode->GetImageData() && volumeNode->GetImageData()->GetNumberOfScalarComponents() >= 3 //
      && this->DisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline)
  {
    // Streamlines are traced through the voxels directly: a sampled unstructured grid would copy every voxel and need a
    // cell locator per view and per time point. The masking region is applied when tracing (vtkVectorFieldGeometry).
    vtkImageFieldSource* imageSource = vtkImageFieldSource::SafeDownCast(this->StreamlineImageSource);
    if (!imageSource)
    {
      this->StreamlineImageSource = vtkSmartPointer<vtkImageFieldSource>::New();
      imageSource = vtkImageFieldSource::SafeDownCast(this->StreamlineImageSource);
    }
    imageSource->SetInputConnection(volumeNode->GetImageDataConnection());
    vtkNew<vtkMatrix4x4> ijkToRAS;
    volumeNode->GetIJKToRASMatrix(ijkToRAS);
    imageSource->SetIJKToRAS(ijkToRAS);
    this->DisplayNode->SetDefaultSampledFieldArrayNames();
    this->DisplayNode->UpdateScalarRange();
    return this->StreamlineImageSource->GetOutputPort();
  }
  if (volumeNode && volumeNode->GetImageData() && volumeNode->GetImageData()->GetNumberOfScalarComponents() >= 3)
  {
    return (this->UpdateVolumeFieldSampler(volumeNode, this->FieldSampler) ? this->FieldSampler->GetOutputPort() : nullptr);
  }

  return nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldPipeline::UpdateMask(vtkMRMLVectorFieldMask* mask)
{
  if (!mask)
  {
    return;
  }
  int numberOfRegions = 0;
  vtkMRMLNode* samplingRegionNode = this->DisplayNode->GetSamplingRegionNodeOtherThanSource();
  bool samplingInside = (samplingRegionNode && this->DisplayNode->GetEffectiveSamplingRegionPart() == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume //
                         && vtkMRMLVectorFieldMask::CanBeRegion(samplingRegionNode));
  vtkMRMLNode* maskingRegionNode = this->DisplayNode->GetMaskingRegionNode();
  bool masking = vtkMRMLVectorFieldMask::CanBeRegion(maskingRegionNode);
  mask->SetNumberOfRegions((samplingInside ? 1 : 0) + (masking ? 1 : 0));
  if (samplingInside)
  {
    mask->SetRegion(numberOfRegions++, samplingRegionNode, std::vector<std::string>(), true);
  }
  if (masking)
  {
    bool showInside = this->DisplayNode->GetMaskingShowInside();
    mask->SetRegion(numberOfRegions++, maskingRegionNode, this->DisplayNode->GetMaskingSegmentIDs(), showInside);
  }
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldPipeline::UpdateMaskingRegion(vtkMRMLVectorFieldMask* mask)
{
  if (!mask)
  {
    return false;
  }
  vtkMRMLNode* maskingRegionNode = this->DisplayNode->GetMaskingRegionNode();
  if (!vtkMRMLVectorFieldMask::CanBeRegion(maskingRegionNode))
  {
    mask->SetNumberOfRegions(0);
    return false;
  }
  mask->SetNumberOfRegions(1);
  bool showInside = this->DisplayNode->GetMaskingShowInside();
  mask->SetRegion(0, maskingRegionNode, this->DisplayNode->GetMaskingSegmentIDs(), showInside);
  return true;
}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLVectorFieldPipeline::GetSamplingSurfaceInWorld(vtkMRMLNode* node)
{
  // Slice views ask for the samples on every update, and the surface is the same until the
  // node, its mesh or segments, or its transforms change
  vtkMTimeType nodeMTime = vtkMRMLVectorFieldMask::GetRegionMTime(node);
  if (this->SamplingSurfaceNode != node || this->SamplingSurfaceMTime != nodeMTime || !this->SamplingSurfaceValid)
  {
    this->SamplingSurface = vtkMRMLVectorFieldSamplePointSource::GetNodeSurfaceInWorld(node);
    this->SamplingSurfaceNode = node;
    this->SamplingSurfaceMTime = nodeMTime;
    this->SamplingSurfaceValid = true;
  }
  return this->SamplingSurface;
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLVectorFieldPipeline::GetFieldToWorldTransform()
{
  vtkMRMLTransformNode* parentTransformNode = this->GetParentTransformNode();
  if (!parentTransformNode)
  {
    return nullptr;
  }
  // Rebuilt only when a node or transform in the parent chain changes, because a new transform makes the samplers and
  // the region filter recompute everything
  vtkMTimeType chainMTime = 0;
  for (vtkMRMLTransformNode* transformNode = parentTransformNode; transformNode; transformNode = transformNode->GetParentTransformNode())
  {
    chainMTime = std::max(chainMTime, transformNode->GetMTime());
    if (transformNode->GetTransformToParent())
    {
      chainMTime = std::max(chainMTime, transformNode->GetTransformToParent()->GetMTime());
    }
  }
  if (!this->FieldToWorldTransform || this->FieldToWorldTransformParent != parentTransformNode || this->FieldToWorldTransformMTime < chainMTime)
  {
    this->FieldToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    vtkMRMLVectorFieldSamplePointSource::GetNodeToWorldTransform(this->DisplayNode->GetDisplayableNode(), this->FieldToWorldTransform);
    this->FieldToWorldTransformParent = parentTransformNode;
    this->FieldToWorldTransformMTime = chainMTime;
  }
  return this->FieldToWorldTransform;
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldPipeline::GetFieldPositions(vtkPoints* positions_World, vtkPoints* positions_Field)
{
  if (!positions_World || !positions_Field)
  {
    return;
  }
  vtkAbstractTransform* fieldToWorld = this->GetFieldToWorldTransform();
  if (!fieldToWorld)
  {
    positions_Field->DeepCopy(positions_World);
    return;
  }
  positions_Field->Initialize();
  fieldToWorld->GetInverse()->TransformPoints(positions_World, positions_Field);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldPipeline::GetMaskedFieldConnection()
{
  vtkAlgorithmOutput* fieldConnection = this->DisplayNode->GetFieldConnection();
  if (!fieldConnection || !this->DisplayNode->HasRegionTest())
  {
    return fieldConnection;
  }
  if (!this->Mask)
  {
    this->Mask = vtkSmartPointer<vtkMRMLVectorFieldMask>::New();
  }
  this->Mask->SetInputConnection(fieldConnection);
  this->UpdateMask(this->Mask);
  // The field is in the coordinates of the node that carries it, the regions are in the
  // world. A transform is evaluated in the world already.
  this->Mask->SetPointsToWorldTransform(this->GetFieldToWorldTransform());
  return this->Mask->GetOutputPort();
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLVectorFieldPipeline::GetNonLinearFieldToWorldTransform()
{
  vtkAbstractTransform* fieldToWorld = this->GetFieldToWorldTransform();
  vtkMRMLTransformNode* parentTransformNode = this->GetParentTransformNode();
  if (!fieldToWorld || !parentTransformNode || parentTransformNode->IsTransformToWorldLinear())
  {
    return nullptr;
  }
  return fieldToWorld;
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldPipeline::GetWorldFieldConnection()
{
  vtkAlgorithmOutput* fieldConnection = this->GetMaskedFieldConnection();
  vtkAbstractTransform* fieldToWorld = this->GetNonLinearFieldToWorldTransform();
  if (!fieldConnection || !fieldToWorld)
  {
    if (this->FieldToWorldWarper)
    {
      this->FieldToWorldWarper->SetInputConnection(nullptr);
    }
    return fieldConnection;
  }
  // A non-linear transform has to be applied to the points, before the glyphs are placed and
  // the grid is warped, so that they are placed and warped in the world
  if (!this->FieldToWorldWarper)
  {
    this->FieldToWorldWarper = vtkSmartPointer<vtkTransformFilter>::New();
  }
  this->FieldToWorldWarper->SetTransform(fieldToWorld);
  this->FieldToWorldWarper->SetInputConnection(fieldConnection);
  return this->FieldToWorldWarper->GetOutputPort();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldPipeline::GetOutputPolyDataConnection()
{
  if (!this->DisplayNode->IsPolyDataVisualizationMode())
  {
    return nullptr;
  }
  // Streamlines are traced once in field coordinates, shared with the slice views, and moved into the world afterwards;
  // the grid is warped and the contours are computed in the world
  bool streamlines = (this->DisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  vtkAlgorithmOutput* fieldConnection = (streamlines ? this->GetMaskedFieldConnection() : this->GetWorldFieldConnection());
  if (!fieldConnection)
  {
    return nullptr;
  }
  vtkAlgorithmOutput* geometryConnection = (this->UpdateGeometryFilter(this->Geometry, fieldConnection, false) ? this->Geometry->GetOutputPort() : nullptr);
  vtkAbstractTransform* fieldToWorld = (streamlines ? this->GetNonLinearFieldToWorldTransform() : nullptr);
  if (!geometryConnection || !fieldToWorld)
  {
    if (this->StreamlineToWorldWarper)
    {
      this->StreamlineToWorldWarper->SetInputConnection(nullptr);
    }
    return geometryConnection;
  }
  if (!this->StreamlineToWorldWarper)
  {
    this->StreamlineToWorldWarper = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  }
  this->StreamlineToWorldWarper->SetTransform(fieldToWorld);
  this->StreamlineToWorldWarper->SetInputConnection(geometryConnection);
  return this->StreamlineToWorldWarper->GetOutputPort();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldPipeline::GetNonWarpedGridConnection()
{
  if (this->DisplayNode->GetVisualizationMode() != vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid || !this->DisplayNode->GetGridShowNonWarped())
  {
    return nullptr;
  }
  if (!this->GetOutputPolyDataConnection())
  {
    return nullptr;
  }
  return this->Geometry->GetOutputPort(vtkVectorFieldGeometry::GridUndeformedLinesPort);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldPipeline::GetStreamlineConnection()
{
  if (this->DisplayNode->GetVisualizationMode() != vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline)
  {
    return nullptr;
  }
  // The geometry filter of the 3D views, so that every view uses the streamlines of one stream tracer
  if (!this->GetOutputPolyDataConnection())
  {
    return nullptr;
  }
  return this->Geometry->GetOutputPort(vtkVectorFieldGeometry::StreamlineLinesPort);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldPipeline::GetMeshFieldPointsConnection(vtkMRMLModelNode* modelNode)
{
  if (!modelNode)
  {
    return nullptr;
  }
  if (!this->DisplayNode->IsFieldArrayCellData())
  {
    // The glyphs are on the points of the mesh. An array that scales or colors them but is
    // cell data is averaged onto the points, so that it is there to be read.
    if (this->DisplayNode->IsMeshCellDataArray(this->DisplayNode->GetActiveScalarName()) //
        || this->DisplayNode->IsMeshCellDataArray(this->DisplayNode->GetEffectiveGlyphScaleArrayName()))
    {
      return this->GetMeshCellDataOnPointsConnection(modelNode);
    }
    return modelNode->GetMeshConnection();
  }
  if (!this->CellCenters)
  {
    this->PointDataToCellData = vtkSmartPointer<vtkPointDataToCellData>::New();
    this->CellCenters = vtkSmartPointer<vtkCellCenters>::New();
    this->CellCenters->SetInputConnection(this->PointDataToCellData->GetOutputPort());
    this->CellCenters->VertexCellsOff();
    this->CellCenters->CopyArraysOn();
  }
  this->PointDataToCellData->SetInputConnection(modelNode->GetMeshConnection());
  return this->CellCenters->GetOutputPort();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldPipeline::GetMeshFieldCellsConnection(vtkMRMLModelNode* modelNode)
{
  if (!modelNode)
  {
    return nullptr;
  }
  // The field and the array that colors it are both needed as point data
  if (!this->DisplayNode->IsFieldArrayCellData() && !this->DisplayNode->IsMeshCellDataArray(this->DisplayNode->GetActiveScalarName()))
  {
    return modelNode->GetMeshConnection();
  }
  return this->GetMeshCellDataOnPointsConnection(modelNode);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldPipeline::GetMeshCellDataOnPointsConnection(vtkMRMLModelNode* modelNode)
{
  if (!this->CellDataToPointData)
  {
    this->CellDataToPointData = vtkSmartPointer<vtkCellDataToPointData>::New();
    this->CellDataToPointData->PassCellDataOn();
  }
  this->CellDataToPointData->SetInputConnection(modelNode->GetMeshConnection());
  return this->CellDataToPointData->GetOutputPort();
}

//----------------------------------------------------------------------------
vtkPointSet* vtkMRMLVectorFieldPipeline::GetInterpolationPoints()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->DisplayNode->GetDisplayableNode());
  if (!modelNode)
  {
    return nullptr;
  }
  if (this->DisplayNode->IsFieldArrayCellData())
  {
    // The fit needs the cell centers themselves, not a connection to them. Updating the
    // filters only re-executes them when the mesh has changed.
    return vtkPointSet::SafeDownCast(vtkMRMLVectorFieldPipeline::GetConnectionOutput(this->GetMeshFieldPointsConnection(modelNode), true));
  }
  return modelNode->GetMesh();
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldPipeline::UpdateScatteredFieldSampler(vtkMRMLModelNode* modelNode, vtkVectorFieldSampler* sampler)
{
  vtkScatteredBSplineTransform* transform = this->UpdateInterpolationTransform();
  if (!modelNode || !sampler || !transform)
  {
    return false;
  }
  sampler->SetFieldSourceToScattered();
  sampler->SetScatteredFittedTransform(transform);
  sampler->SetScatteredColorFittedTransform(this->UpdateColorInterpolationTransform());
  // The same distance as the domain of the fit was grown by
  sampler->SetScatteredMaximumDistance(this->FittedMaximumDistance);
  this->UpdateSamplerRegion(sampler);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldPipeline::UpdateSliceFieldSampler(vtkVectorFieldSampler* sampler)
{
  if (!sampler)
  {
    return false;
  }
  bool glyphsOnMeshPoints = (this->DisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph //
                             && vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(this->DisplayNode));
  if (!this->DisplayNode->CanSampleAtArbitraryPositions() || glyphsOnMeshPoints)
  {
    // Slice views select the points near the plane instead, also when glyphs belong on the mesh points, so that they
    // draw the field at the same places as the 3D view
    return false;
  }
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->DisplayNode->GetDisplayableNode());
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(this->DisplayNode->GetDisplayableNode());
  if (!modelNode && !volumeNode)
  {
    return false;
  }
  // A slice view samples where its plane is in the world. Set on every update: the cached transform is replaced when
  // the parent transform chain changes, and setting the same one again does not modify the sampler.
  sampler->SetFieldToWorldTransform(this->GetFieldToWorldTransform());
  if (modelNode && this->DisplayNode->IsInterpolationUsed())
  {
    // Interpolated between the points of a surface mesh or a point cloud. The fit itself is
    // shared with every other view; the sampler only evaluates it in this slice.
    return this->UpdateScatteredFieldSampler(modelNode, sampler);
  }
  return (modelNode ? this->UpdateMeshFieldSampler(modelNode, sampler) : this->UpdateVolumeFieldSampler(volumeNode, sampler));
}

//----------------------------------------------------------------------------
vtkScatteredBSplineTransform* vtkMRMLVectorFieldPipeline::UpdateInterpolationTransform()
{
  if (!this->DisplayNode->IsInterpolationUsed())
  {
    return nullptr;
  }
  return this->UpdateVectorFit(this->GetInterpolationPoints(), this->DisplayNode->GetModeVectorArrayName(), this->DisplayNode->GetInterpolationMaximumDistance());
}

//----------------------------------------------------------------------------
vtkScatteredBSplineTransform* vtkMRMLVectorFieldPipeline::UpdateColorInterpolationTransform()
{
  const char* colorArrayName = this->DisplayNode->GetSampledColorArrayName();
  // The values are fitted on the same points and lattice as the vectors
  vtkScatteredBSplineTransform* vectorTransform = this->UpdateInterpolationTransform();
  vtkPointSet* points = this->GetInterpolationPoints();
  if (!colorArrayName || !vectorTransform || !points)
  {
    return nullptr;
  }
  vtkDataArray* colors = points->GetPointData()->GetArray(colorArrayName);
  if (!colors && !this->DisplayNode->IsFieldArrayCellData())
  {
    // A cell data array, colored at the points of the mesh, where the vectors are
    vtkAlgorithmOutput* cellDataOnPoints = this->GetMeshCellDataOnPointsConnection(vtkMRMLModelNode::SafeDownCast(this->DisplayNode->GetDisplayableNode()));
    vtkDataSet* meshWithPointColors = vtkDataSet::SafeDownCast(vtkMRMLVectorFieldPipeline::GetConnectionOutput(cellDataOnPoints, true));
    colors = (meshWithPointColors ? meshWithPointColors->GetPointData()->GetArray(colorArrayName) : nullptr);
  }
  if (!colors)
  {
    return nullptr;
  }
  return this->UpdateColorFit(points, colors, this->DisplayNode->GetEffectiveActiveScalarComponent(colors));
}

//----------------------------------------------------------------------------
double vtkMRMLVectorFieldPipeline::GetEffectiveInterpolationMaximumDistance()
{
  return (this->UpdateInterpolationTransform() ? this->FittedMaximumDistance : 0.0);
}

//----------------------------------------------------------------------------
vtkScatteredBSplineTransform* vtkMRMLVectorFieldPipeline::UpdateVectorFit(vtkPointSet* points, const char* vectorArrayName, double maximumDistanceMm)
{
  this->VectorFitValid = false;
  if (!points || points->GetNumberOfPoints() == 0 || !vectorArrayName || vectorArrayName[0] == '\0')
  {
    return nullptr;
  }
  vtkDataArray* vectors = points->GetPointData()->GetArray(vectorArrayName);
  if (!vectors || vectors->GetNumberOfComponents() != 3)
  {
    return nullptr;
  }

  if (!this->VectorFit)
  {
    this->VectorFit = vtkSmartPointer<vtkScatteredBSplineTransform>::New();
    // Away from the data the field follows its overall linear trend instead of flattening
    // out to zero, which is what a displacement field is expected to do
    this->VectorFit->UseLinearApproximationOn();
  }
  this->VectorFit->SetInputPointSet(points);
  this->VectorFit->SetDisplacementArrayName(vectorArrayName);

  // Fit everywhere the field is drawn, up to the maximum distance beyond the outermost points: outside its domain a
  // B-spline keeps its border value
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  points->GetBounds(bounds);
  if (maximumDistanceMm <= 0.0)
  {
    maximumDistanceMm = 0.1 * vtkVectorFieldSampler::GetBoundsDiagonal(bounds);
  }
  for (int axis = 0; axis < 3; ++axis)
  {
    bounds[2 * axis] -= maximumDistanceMm;
    bounds[2 * axis + 1] += maximumDistanceMm;
  }
  this->VectorFit->AutomaticGridBoundsOff();
  this->VectorFit->SetGridBounds(bounds);
  this->FittedMaximumDistance = maximumDistanceMm;
  double largestSizeMm = std::max(std::max(bounds[1] - bounds[0], bounds[3] - bounds[2]), bounds[5] - bounds[4]);
  // Each refinement level has 8x the control points. A hundredth of the data size is finer than a readable glyph
  // density shows, and keeps the lattice within about a million control points.
  this->VectorFit->SetMinimumControlPointSpacing(largestSizeMm > 0.0 ? largestSizeMm / 100.0 : 0.1);
  // The fit is refined until it reproduces the vectors to a thousandth of the longest one,
  // whatever their units are
  double magnitudeRange[2] = { 0.0, -1.0 };
  vectors->GetRange(magnitudeRange, -1); // -1: L2 norm over all components
  this->VectorFit->SetTolerance(magnitudeRange[1] > 0.0 ? 0.001 * magnitudeRange[1] : 1e-6);
  this->VectorFitValid = true;
  return this->VectorFit;
}

//----------------------------------------------------------------------------
vtkScatteredBSplineTransform* vtkMRMLVectorFieldPipeline::UpdateColorFit(vtkPointSet* points, vtkDataArray* values, int component)
{
  if (!this->VectorFitValid || !points || !points->GetPoints() || !values || values->GetNumberOfTuples() != points->GetNumberOfPoints())
  {
    return nullptr;
  }

  // The values become the first component of a displacement, which is what the transform fits. Copied again only when
  // they or the points change, as a new input makes the transform refit.
  vtkMTimeType pointsMTime = points->GetPoints()->GetMTime();
  if (!this->ColorFitPoints || this->ColorFitSourceArray != values                                   //
      || this->ColorFitSourceMTime != values->GetMTime() || this->ColorFitPointsMTime != pointsMTime //
      || this->ColorFitComponent != component)
  {
    vtkNew<vtkDoubleArray> displacements;
    displacements->SetName(ColorValueArrayName);
    displacements->SetNumberOfComponents(3);
    displacements->SetNumberOfTuples(values->GetNumberOfTuples());
    int numberOfComponents = values->GetNumberOfComponents();
    for (vtkIdType pointIndex = 0; pointIndex < values->GetNumberOfTuples(); ++pointIndex)
    {
      double value = 0.0;
      if (numberOfComponents == 1)
      {
        value = values->GetComponent(pointIndex, 0);
      }
      else if (component >= 0)
      {
        value = values->GetComponent(pointIndex, component);
      }
      else
      {
        // A multi-component array is colored by its magnitude, unless a component is chosen
        for (int c = 0; c < numberOfComponents; ++c)
        {
          double componentValue = values->GetComponent(pointIndex, c);
          value += componentValue * componentValue;
        }
        value = std::sqrt(value);
      }
      displacements->SetTuple3(pointIndex, value, 0.0, 0.0);
    }
    this->ColorFitPoints = vtkSmartPointer<vtkPolyData>::New();
    this->ColorFitPoints->SetPoints(points->GetPoints());
    this->ColorFitPoints->GetPointData()->AddArray(displacements);
    this->ColorFitSourceArray = values;
    this->ColorFitSourceMTime = values->GetMTime();
    this->ColorFitPointsMTime = pointsMTime;
    this->ColorFitComponent = component;
  }

  // On the same lattice as the vectors
  if (!this->ColorFit)
  {
    this->ColorFit = vtkSmartPointer<vtkScatteredBSplineTransform>::New();
    this->ColorFit->UseLinearApproximationOn();
    this->ColorFit->SetDisplacementArrayName(ColorValueArrayName);
  }
  this->ColorFit->SetInputPointSet(this->ColorFitPoints);
  this->ColorFit->AutomaticGridBoundsOff();
  this->ColorFit->SetGridBounds(this->VectorFit->GetGridBounds());
  this->ColorFit->SetMinimumControlPointSpacing(this->VectorFit->GetMinimumControlPointSpacing());
  // Refined until it reproduces the values to a thousandth of the largest one
  double valueRange[2] = { 0.0, -1.0 };
  this->ColorFitPoints->GetPointData()->GetArray(ColorValueArrayName)->GetRange(valueRange, 0);
  double largestValue = std::max(std::abs(valueRange[0]), std::abs(valueRange[1]));
  this->ColorFit->SetTolerance(largestValue > 0.0 ? 0.001 * largestValue : 1e-6);
  return this->ColorFit;
}

//----------------------------------------------------------------------------
double vtkMRMLVectorFieldPipeline::GetInterpolationResidual()
{
  vtkScatteredBSplineTransform* transform = this->UpdateInterpolationTransform();
  if (!transform)
  {
    return -1.0;
  }
  transform->Update();
  return transform->GetResidual();
}

//----------------------------------------------------------------------------
vtkAlgorithm* vtkMRMLVectorFieldPipeline::GetFieldProducer(vtkAlgorithmOutput* fieldConnection)
{
  // A transform filter (that moves a node under a transform into the world) and the region
  // filter keep the names of the arrays, so they are looked through
  vtkAlgorithm* producer = (fieldConnection ? fieldConnection->GetProducer() : nullptr);
  while ((vtkTransformFilter::SafeDownCast(producer) || vtkMRMLVectorFieldMask::SafeDownCast(producer)) //
         && producer->GetNumberOfInputConnections(0) > 0)
  {
    producer = producer->GetInputConnection(0, 0)->GetProducer();
  }
  return producer;
}

//----------------------------------------------------------------------------
const char* vtkMRMLVectorFieldPipeline::GetFieldVectorArrayName(vtkAlgorithmOutput* fieldConnection)
{
  // The array name depends on the connection, not the display node: a sampler renames its arrays, and the same node may
  // feed 3D views from the mesh and slice views from samplers
  if (vtkVectorFieldSampler::ProducesSampledField(vtkMRMLVectorFieldPipeline::GetFieldProducer(fieldConnection)))
  {
    return vtkVectorFieldSampler::GetVectorArrayName();
  }
  return this->DisplayNode->GetModeVectorArrayName();
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldPipeline::UpdateGeometryFilter(vtkVectorFieldGeometry* geometry, vtkAlgorithmOutput* fieldConnection, bool flat)
{
  if (!geometry || !fieldConnection)
  {
    return false;
  }
  vtkMRMLVectorFieldDisplayNode* displayNode = this->DisplayNode;
  int mode = vtkVectorFieldGeometry::ModeNone;
  switch (displayNode->GetVisualizationMode())
  {
    case vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid: mode = vtkVectorFieldGeometry::ModeGrid; break;
    case vtkMRMLVectorFieldDisplayNode::VisualizationModeContour: mode = vtkVectorFieldGeometry::ModeContour; break;
    case vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline: mode = vtkVectorFieldGeometry::ModeStreamline; break;
    default: return false;
  }
  const char* vectorArrayName = this->GetFieldVectorArrayName(fieldConnection);
  if (!vectorArrayName || vectorArrayName[0] == '\0')
  {
    return false;
  }
  geometry->SetMode(mode);
  geometry->SetFlat(flat);
  geometry->SetVectorArrayName(vectorArrayName);
  // A sampled field has its magnitude; mesh point data may not, so the filter computes it from the vectors
  bool sampled = vtkVectorFieldSampler::ProducesSampledField(vtkMRMLVectorFieldPipeline::GetFieldProducer(fieldConnection));
  geometry->SetMagnitudeArrayName(sampled ? vtkVectorFieldSampler::GetMagnitudeArrayName() : "");
  // Grid lines and isosurfaces carry the array that says where the field is shown from the samples they are built from
  geometry->SetRegionArrayName(displayNode->HasRegionTest() ? vtkMRMLVectorFieldMask::GetInsideArrayName() : "");
  vtkAlgorithmOutput* inputConnection = fieldConnection;

  if (mode == vtkVectorFieldGeometry::ModeGrid)
  {
    if (!displayNode->GetGridLinesVisibility() && !displayNode->GetGridShowNonWarped() && !displayNode->GetGridFill())
    {
      return false;
    }
    geometry->SetGridSubdivision(displayNode->GetGridSubdivision());
    geometry->SetGridScaleFactor(displayNode->GetGridScaleFactor());
    geometry->SetGridDeformedLinesVisibility(displayNode->GetGridLinesVisibility());
    geometry->SetGridUndeformedLinesVisibility(displayNode->GetGridShowNonWarped());
    geometry->SetGridFill(displayNode->GetGridFill());
    geometry->SetGridLineDiameter(displayNode->GetGridLineDiameter());
  }
  else if (mode == vtkVectorFieldGeometry::ModeContour)
  {
    std::vector<double> levels;
    displayNode->GetEffectiveContourLevels(levels);
    geometry->SetContourLevels(levels);
  }
  else
  {
    // Seeds are in the world, streamlines are traced in the coordinates of the field
    vtkPoints* allSeedPositions_RAS = this->GetStreamlineSeedPositions();
    if (!allSeedPositions_RAS)
    {
      return false;
    }
    vtkNew<vtkPoints> seedPositions_RAS;
    vtkMRMLVectorFieldSamplePointSource::SelectRandomSubset(allSeedPositions_RAS, this->MaximumNumberOfStreamlineSeeds, seedPositions_RAS);
    vtkNew<vtkPoints> seedPositions_Field;
    this->GetFieldPositions(seedPositions_RAS, seedPositions_Field);
    geometry->SetStreamlineSeeds(seedPositions_Field);
    // Not cut by the regions: the masking region stops them where it hides the field, the sampling region only says
    // where they start
    geometry->SetRegionArrayName("");
    if (!this->StreamlineMask)
    {
      this->StreamlineMask = vtkSmartPointer<vtkMRMLVectorFieldMask>::New();
    }
    if (this->UpdateMaskingRegion(this->StreamlineMask))
    {
      this->StreamlineMask->SetInputConnection(fieldConnection);
      this->StreamlineMask->SetPointsToWorldTransform(this->GetFieldToWorldTransform());
      inputConnection = this->StreamlineMask->GetOutputPort();
      geometry->SetStreamlineDomainArrayName(vtkMRMLVectorFieldMask::GetInsideArrayName());
    }
    else
    {
      this->StreamlineMask->SetInputConnection(nullptr);
      geometry->SetStreamlineDomainArrayName("");
    }
    geometry->SetStreamlineBidirectional(displayNode->GetStreamlineBidirectional());
    geometry->SetStreamlineMaximumPropagation(displayNode->GetStreamlineMaximumPropagation());
    geometry->SetStreamlineInitialIntegrationStep(displayNode->GetStreamlineInitialIntegrationStep());
    geometry->SetStreamlineTubeDiameter(displayNode->GetStreamlineTubeDiameter());
    // Only traced where the color value is in the threshold range, where glyphs are shown
    const char* thresholdArrayName = displayNode->GetRenderedActiveScalarName();
    bool hasThresholdArray = (thresholdArrayName && thresholdArrayName[0] != '\0');
    geometry->SetThresholdEnabled(displayNode->GetThresholdEnabled() && hasThresholdArray);
    geometry->SetThresholdArrayName(hasThresholdArray ? thresholdArrayName : "");
    geometry->SetThresholdComponent(displayNode->GetRenderedActiveScalarComponent());
    geometry->SetThresholdRange(displayNode->GetThresholdRange());
  }
  geometry->SetInputConnection(inputConnection);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldPipeline::IsDirectionalScaleUsed(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return false;
  }
  // A thickness in mm stays in mm only if not scaled with the glyph, and only a glyph with an axis and a cross section
  // (not a sphere or a box) can be stretched along it
  int glyphType = displayNode->GetGlyphType();
  bool stretchable = (glyphType == vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow   //
                      || glyphType == vtkMRMLVectorFieldDisplayNode::GlyphTypeCone //
                      || glyphType == vtkMRMLVectorFieldDisplayNode::GlyphTypeCylinder);
  if (!displayNode->GetGlyphDiameterAbsolute() || !displayNode->GetGlyphScaleDirectional() || !stretchable)
  {
    return false;
  }
  const char* orientationArrayName = displayNode->GetRenderedGlyphOrientationArrayName();
  const char* scaleArrayName = displayNode->GetRenderedGlyphScaleArrayName();
  return (orientationArrayName && orientationArrayName[0] != '\0' && scaleArrayName && scaleArrayName[0] != '\0');
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldPipeline::UpdateGlyphShape(vtkVectorFieldGlyph* glyph)
{
  if (!glyph)
  {
    return;
  }
  vtkMRMLVectorFieldDisplayNode* displayNode = this->DisplayNode;
  glyph->SetFlat(false);
  switch (displayNode->GetGlyphType())
  {
    case vtkMRMLVectorFieldDisplayNode::GlyphTypeCone: glyph->SetGlyphType(vtkVectorFieldGlyph::GlyphCone); break;
    case vtkMRMLVectorFieldDisplayNode::GlyphTypeBox: glyph->SetGlyphType(vtkVectorFieldGlyph::GlyphBox); break;
    case vtkMRMLVectorFieldDisplayNode::GlyphTypeCylinder: glyph->SetGlyphType(vtkVectorFieldGlyph::GlyphCylinder); break;
    case vtkMRMLVectorFieldDisplayNode::GlyphTypeLine: glyph->SetGlyphType(vtkVectorFieldGlyph::GlyphLine); break;
    case vtkMRMLVectorFieldDisplayNode::GlyphTypeSphere: glyph->SetGlyphType(vtkVectorFieldGlyph::GlyphSphere); break;
    case vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow:
    default: glyph->SetGlyphType(vtkVectorFieldGlyph::GlyphArrow); break;
  }
  glyph->SetResolution(displayNode->GetGlyphResolution());
  // Radius of the unit-length shape: a fraction for a relative thickness, or mm for an absolute one, which holds
  // because such glyphs are stretched along their axis alone
  double tipRadius = 0.5 * (displayNode->GetGlyphDiameterAbsolute() ? displayNode->GetGlyphDiameter() : 0.01 * displayNode->GetGlyphDiameterPercent());
  glyph->SetTipLength(displayNode->GetGlyphTipLengthPercent() * 0.01);
  glyph->SetTipRadius(tipRadius);
  glyph->SetShaftRadius(tipRadius * 0.01 * displayNode->GetGlyphShaftDiameterPercent());
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldPipeline::UpdateGlyphFilter(vtkVectorFieldGlyph* glyph, vtkAlgorithmOutput* fieldConnection, vtkMatrix4x4* slicePlaneToRAS /*=nullptr*/)
{
  if (!glyph || !fieldConnection)
  {
    return false;
  }
  vtkMRMLVectorFieldDisplayNode* displayNode = this->DisplayNode;
  bool flat = (slicePlaneToRAS != nullptr);
  glyph->SetInputConnection(fieldConnection);

  // The settings that decide what the outputs hold go first, so that the settings that only
  // the mapper uses mark the filter modified when the outputs depend on them
  glyph->SetGenerateGeometry(flat);
  glyph->SetDirectionalScale(!flat && vtkMRMLVectorFieldPipeline::IsDirectionalScaleUsed(displayNode));
  glyph->SetProjectToPlane(flat && displayNode->GetGlyphSliceProjectionEnabled());
  glyph->SetGlyphPlane(slicePlaneToRAS);

  // Shape: flat outlines in slice views, which stay crisp at any zoom level and do not hide the image underneath
  this->UpdateGlyphShape(glyph);
  if (flat)
  {
    glyph->SetFlat(true);
    glyph->SetResolution(displayNode->GetGlyphResolution2D());
    glyph->SetTipLength(displayNode->GetGlyphTipLengthPercent2D() * 0.01);
  }

  // Thresholding: only keep points where the active scalar is within the threshold range.
  // Multi-component arrays are thresholded by the chosen component or by their magnitude.
  const char* activeScalarName = displayNode->GetRenderedActiveScalarName();
  bool hasActiveScalar = (activeScalarName && activeScalarName[0] != '\0');
  glyph->SetThresholdEnabled(displayNode->GetThresholdEnabled() && hasActiveScalar);
  glyph->SetThresholdArrayName(hasActiveScalar ? activeScalarName : "");
  glyph->SetThresholdComponent(displayNode->GetRenderedActiveScalarComponent());
  double thresholdRange[2] = { 0.0, -1.0 };
  displayNode->GetThresholdRange(thresholdRange);
  glyph->SetThresholdRange(thresholdRange);

  // The regions limit where glyphs are drawn: points outside the inside of the sampling
  // region or the masking region are dropped
  glyph->SetRegionArrayName(displayNode->HasRegionTest() ? vtkMRMLVectorFieldMask::GetInsideArrayName() : "");

  // Glyphs on all or a random subset of the mesh points. A slice view selects points near the slice without their
  // cells, so it has no surface or volume to spread the subset over.
  bool randomPattern = (displayNode->GetSamplingPattern() == vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  bool randomSubset = (vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(displayNode) && randomPattern);
  int part = displayNode->GetEffectiveSamplingRegionPart();
  int subsetMode = vtkVectorFieldGlyph::SubsetAll;
  if (randomSubset)
  {
    subsetMode = (flat                                                               ? vtkVectorFieldGlyph::SubsetRandomBounds
                  : part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface ? vtkVectorFieldGlyph::SubsetRandomSurface
                  : part == vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume  ? vtkVectorFieldGlyph::SubsetRandomVolume
                                                                                     : vtkVectorFieldGlyph::SubsetRandomBounds);
  }
  // Regularly on the surface of the mesh. As above, a slice view has only the points near the slice to take them from.
  if (!randomPattern && displayNode->IsSamplingOwnSurfaceSpaced())
  {
    subsetMode = (flat ? vtkVectorFieldGlyph::SubsetSpaced : vtkVectorFieldGlyph::SubsetSpacedSurface);
  }
  glyph->SetSubsetMode(subsetMode);
  glyph->SetMaximumNumberOfPoints(displayNode->GetSamplingPointsNumber());
  glyph->SetSubsetSpacing(displayNode->GetSamplingSpacing());

  // Orientation and scale
  const char* orientationArrayName = displayNode->GetRenderedGlyphOrientationArrayName();
  glyph->SetOrientationArrayName(orientationArrayName ? orientationArrayName : "");
  const char* scaleArrayName = displayNode->GetRenderedGlyphScaleArrayName();
  bool hasScaleArray = (scaleArrayName && scaleArrayName[0] != '\0');
  glyph->SetScaleArrayName(hasScaleArray ? scaleArrayName : "");
  vtkDataArray* scaleArray = displayNode->GetRenderedGlyphScaleArray();
  bool scaleByComponents = (scaleArray && scaleArray->GetNumberOfComponents() == 3 && displayNode->GetGlyphScaleByComponents());
  glyph->SetScaleMode(!hasScaleArray ? vtkVectorFieldGlyph::ScaleNone : scaleByComponents ? vtkVectorFieldGlyph::ScaleByComponents : vtkVectorFieldGlyph::ScaleByMagnitude);
  glyph->SetScaleFactor(displayNode->GetGlyphScaleFactor());
  return true;
}
