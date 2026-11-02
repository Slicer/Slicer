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
#include "vtkVectorFieldSampler.h"

// vtkAddon includes
#include <vtkAddonMathUtilities.h>
#include <vtkIdList.h>
#include <vtkScatteredBSplineTransform.h>

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkBoundingBox.h>
#include <vtkCellArray.h>
#include <vtkCellLocatorStrategy.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkImageInterpolator.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProbeFilter.h>
#include <vtkStaticPointLocator.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

// STD includes
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <vector>

namespace
{
const char VectorArrayName[] = "Vector";
const char MagnitudeArrayName[] = "Magnitude";
const char ColorArrayName[] = "Color";
const char LatticeSizeArrayName[] = "LatticeSize";
const char ValidSampleArrayName[] = "ValidSample";

//----------------------------------------------------------------------------
/// Copy a matrix into another if they differ. Returns whether it did.
bool CopyMatrixIfChanged(vtkMatrix4x4* destination, vtkMatrix4x4* source)
{
  if (vtkAddonMathUtilities::MatrixAreEqual(destination, source, 0.0))
  {
    return false;
  }
  destination->DeepCopy(source);
  return true;
}

//----------------------------------------------------------------------------
/// The bounds of a box after a transform: the bounds of its eight transformed corners (as
/// vtkOrientedImageDataResample::TransformBounds() computes them)
void TransformBounds(const double inputBounds[6], vtkAbstractTransform* inputToOutputTransform, double outputBounds[6])
{
  vtkBoundingBox outputBoundingBox;
  for (int corner = 0; corner < 8; ++corner)
  {
    double inputCorner[3] = { inputBounds[corner & 1], inputBounds[2 + ((corner >> 1) & 1)], inputBounds[4 + ((corner >> 2) & 1)] };
    double outputCorner[3] = { 0.0, 0.0, 0.0 };
    inputToOutputTransform->TransformPoint(inputCorner, outputCorner);
    outputBoundingBox.AddPoint(outputCorner);
  }
  outputBoundingBox.GetBounds(outputBounds);
}

//----------------------------------------------------------------------------
/// Length of the third axis of a slice view's XY coordinate system, in mm. It is the slice spacing (not 1 mm), so
/// a distance from the slice plane measured along it has to be multiplied by this to get mm.
double GetSliceNormalLengthMm(vtkMatrix4x4* sliceXYToRAS)
{
  double zAxis_RAS[3] = { sliceXYToRAS->GetElement(0, 2), sliceXYToRAS->GetElement(1, 2), sliceXYToRAS->GetElement(2, 2) };
  return vtkMath::Norm(zAxis_RAS);
}
} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVectorFieldSampler);

//----------------------------------------------------------------------------
vtkVectorFieldSampler::vtkVectorFieldSampler()
{
  // The image or the mesh that carries the field, for the sources that have one
  this->SetNumberOfInputPorts(1);
}

//----------------------------------------------------------------------------
vtkVectorFieldSampler::~vtkVectorFieldSampler()
{
  this->SetMeshVectorArrayName(nullptr);
  this->SetMeshColorArrayName(nullptr);
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "RegionSize: " << this->RegionSize[0] << " " << this->RegionSize[1] << " " << this->RegionSize[2] << "\n";
  os << indent << "SamplingSpacing: " << this->SamplingSpacing << "\n";
  os << indent << "LatticeGroupSize: " << this->LatticeGroupSize << "\n";
  os << indent << "SliceThickness: " << this->SliceThickness << "\n";
  os << indent << "FieldOfViewSize: " << this->FieldOfViewSize[0] << " " << this->FieldOfViewSize[1] << "\n";
  os << indent << "SamplePositions: " << this->SamplePositions.GetPointer() << "\n";
  os << indent << "FieldToWorldTransform: " << this->FieldToWorldTransform.GetPointer() << "\n";
  os << indent << "GenerateCells: " << (this->GenerateCells ? "On" : "Off") << "\n";
  os << indent << "OutputLatticeSize: " << this->OutputLatticeSize[0] << " " << this->OutputLatticeSize[1] << " " << this->OutputLatticeSize[2] << "\n";
  os << indent << "FieldSource: " << vtkVectorFieldSampler::GetFieldSourceAsString(this->FieldSource) << "\n";
  os << indent << "VectorsInRAS: " << (this->VectorsInRAS ? "On" : "Off") << "\n";
  os << indent << "IJKToRAS:\n";
  this->IJKToRAS->PrintSelf(os, indent.GetNextIndent());
  os << indent << "MeshVectorArrayName: " << (this->MeshVectorArrayName ? this->MeshVectorArrayName : "(none)") << "\n";
  os << indent << "MeshColorArrayName: " << (this->MeshColorArrayName ? this->MeshColorArrayName : "(none)") << "\n";
  os << indent << "MeshColorArrayComponent: " << this->MeshColorArrayComponent << "\n";
  os << indent << "ScatteredFittedTransform: " << this->ScatteredFittedTransform.GetPointer() << "\n";
  os << indent << "ScatteredColorFittedTransform: " << this->ScatteredColorFittedTransform.GetPointer() << "\n";
  os << indent << "ScatteredMaximumDistance: " << this->ScatteredMaximumDistance << "\n";
  os << indent << "Transform: " << this->Transform.GetPointer() << "\n";
}

//----------------------------------------------------------------------------
const char* vtkVectorFieldSampler::GetVectorArrayName()
{
  return VectorArrayName;
}

//----------------------------------------------------------------------------
const char* vtkVectorFieldSampler::GetMagnitudeArrayName()
{
  return MagnitudeArrayName;
}

//----------------------------------------------------------------------------
const char* vtkVectorFieldSampler::GetColorArrayName()
{
  return ColorArrayName;
}

//----------------------------------------------------------------------------
const char* vtkVectorFieldSampler::GetLatticeSizeArrayName()
{
  return LatticeSizeArrayName;
}

//----------------------------------------------------------------------------
const char* vtkVectorFieldSampler::GetValidSampleArrayName()
{
  return ValidSampleArrayName;
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetOutputLatticeSize(vtkUnstructuredGrid* output, const int latticeSize[3])
{
  std::copy(latticeSize, latticeSize + 3, this->OutputLatticeSize);
  if (!output)
  {
    return;
  }
  vtkNew<vtkIntArray> latticeSizeArray;
  latticeSizeArray->SetName(LatticeSizeArrayName);
  latticeSizeArray->SetNumberOfValues(3);
  for (int axis = 0; axis < 3; ++axis)
  {
    latticeSizeArray->SetValue(axis, latticeSize[axis]);
  }
  output->GetFieldData()->AddArray(latticeSizeArray);
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetSamplingRegion(vtkMatrix4x4* regionToRAS, const int size[3])
{
  bool modified = (regionToRAS && CopyMatrixIfChanged(this->RegionToRAS, regionToRAS));
  if (size)
  {
    for (int i = 0; i < 3; ++i)
    {
      if (this->RegionSize[i] != size[i])
      {
        this->RegionSize[i] = size[i];
        modified = true;
      }
    }
  }
  if (modified)
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetSlicePlane(vtkMatrix4x4* sliceXYToRAS, const double fieldOfViewSizeMm[2])
{
  bool modified = (sliceXYToRAS && CopyMatrixIfChanged(this->SliceXYToRAS, sliceXYToRAS));
  if (fieldOfViewSizeMm)
  {
    for (int i = 0; i < 2; ++i)
    {
      if (this->FieldOfViewSize[i] != fieldOfViewSizeMm[i])
      {
        this->FieldOfViewSize[i] = fieldOfViewSizeMm[i];
        modified = true;
      }
    }
  }
  if (modified)
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetSamplePositions(vtkPoints* samplePositions_RAS)
{
  if (vtkVectorFieldSampler::ArePointsEqual(this->SamplePositions, samplePositions_RAS))
  {
    return;
  }
  this->SamplePositions = nullptr;
  if (samplePositions_RAS)
  {
    this->SamplePositions = vtkSmartPointer<vtkPoints>::New();
    this->SamplePositions->DeepCopy(samplePositions_RAS);
  }
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::ArePointsEqual(vtkPoints* points, vtkPoints* otherPoints)
{
  vtkIdType numberOfPoints = (points ? points->GetNumberOfPoints() : 0);
  if (numberOfPoints != (otherPoints ? otherPoints->GetNumberOfPoints() : 0))
  {
    return false;
  }
  double point[3] = { 0.0, 0.0, 0.0 };
  double otherPoint[3] = { 0.0, 0.0, 0.0 };
  for (vtkIdType pointIndex = 0; pointIndex < numberOfPoints; ++pointIndex)
  {
    points->GetPoint(pointIndex, point);
    otherPoints->GetPoint(pointIndex, otherPoint);
    if (point[0] != otherPoint[0] || point[1] != otherPoint[1] || point[2] != otherPoint[2])
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
vtkPoints* vtkVectorFieldSampler::GetSamplePositions()
{
  return this->SamplePositions;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkVectorFieldSampler::GetMTime()
{
  vtkMTimeType mTime = this->Superclass::GetMTime();
  vtkMTimeType sampledObjectMTime = this->GetSampledObjectMTime();
  if (sampledObjectMTime > mTime)
  {
    mTime = sampledObjectMTime;
  }
  if (this->SamplePositions && this->SamplePositions->GetMTime() > mTime)
  {
    mTime = this->SamplePositions->GetMTime();
  }
  // Moving the field moves where each world position falls in it
  if (this->FieldToWorldTransform && this->FieldToWorldTransform->GetMTime() > mTime)
  {
    mTime = this->FieldToWorldTransform->GetMTime();
  }
  return mTime;
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::ProducesSampledField(vtkAlgorithm* producer)
{
  // The image source is a filter local to vtkMRMLVectorFieldPipeline, known by its name
  return (producer && (vtkVectorFieldSampler::SafeDownCast(producer) != nullptr || producer->IsA("vtkImageFieldSource")));
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetFieldToWorldTransform(vtkAbstractTransform* fieldToWorld)
{
  if (this->FieldToWorldTransform == fieldToWorld)
  {
    return;
  }
  this->FieldToWorldTransform = fieldToWorld;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkVectorFieldSampler::GetFieldToWorldTransform()
{
  return this->FieldToWorldTransform;
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::HasFieldToWorldTransform()
{
  if (!this->FieldToWorldTransform)
  {
    return false;
  }
  // A node that is not under any transform gets an empty general transform
  vtkGeneralTransform* generalTransform = vtkGeneralTransform::SafeDownCast(this->FieldToWorldTransform);
  if (generalTransform && generalTransform->GetNumberOfConcatenatedTransforms() == 0)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::GetFieldPositions(vtkPoints* positions_World, vtkPoints* positions_Field)
{
  if (!positions_World || !positions_Field)
  {
    return;
  }
  if (!this->HasFieldToWorldTransform())
  {
    positions_Field->DeepCopy(positions_World);
    return;
  }
  positions_Field->SetDataTypeToDouble();
  this->FieldToWorldTransform->GetInverse()->TransformPoints(positions_World, positions_Field);
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::TransformVectorsToWorld(vtkPoints* positions_Field, vtkDataArray* vectors)
{
  if (!positions_Field || !vectors || !this->HasFieldToWorldTransform())
  {
    return;
  }
  vtkIdType numberOfVectors = std::min(positions_Field->GetNumberOfPoints(), vectors->GetNumberOfTuples());
  for (vtkIdType vectorIndex = 0; vectorIndex < numberOfVectors; ++vectorIndex)
  {
    double position_Field[3] = { 0.0, 0.0, 0.0 };
    positions_Field->GetPoint(vectorIndex, position_Field);
    double vector_Field[3] = { 0.0, 0.0, 0.0 };
    vectors->GetTuple(vectorIndex, vector_Field);
    double vector_World[3] = { 0.0, 0.0, 0.0 };
    this->FieldToWorldTransform->TransformVectorAtPoint(position_Field, vector_Field, vector_World);
    vectors->SetTuple(vectorIndex, vector_World);
  }
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::GetLatticePositions(vtkMatrix4x4* latticeToRAS, const int latticeSize[3], vtkPoints* samplePositions_RAS)
{
  if (!latticeToRAS || !latticeSize || !samplePositions_RAS)
  {
    return;
  }
  samplePositions_RAS->SetNumberOfPoints(static_cast<vtkIdType>(latticeSize[0]) * latticeSize[1] * latticeSize[2]);
  vtkIdType pointIndex = 0;
  for (int k = 0; k < latticeSize[2]; ++k)
  {
    for (int j = 0; j < latticeSize[1]; ++j)
    {
      for (int i = 0; i < latticeSize[0]; ++i)
      {
        double point_Lattice[4] = { static_cast<double>(i), static_cast<double>(j), static_cast<double>(k), 1.0 };
        double point_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
        latticeToRAS->MultiplyPoint(point_Lattice, point_RAS);
        samplePositions_RAS->SetPoint(pointIndex++, point_RAS);
      }
    }
  }
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::GetLatticePositionsInBounds(const double bounds_RAS[6], double spacingMm, vtkPoints* samplePositions_RAS, int latticeSize[3])
{
  if (!bounds_RAS || !samplePositions_RAS || !latticeSize || spacingMm <= 0.0)
  {
    return false;
  }
  vtkNew<vtkMatrix4x4> latticeToRAS;
  latticeToRAS->Identity();
  for (int axis = 0; axis < 3; ++axis)
  {
    double sizeMm = bounds_RAS[2 * axis + 1] - bounds_RAS[2 * axis];
    int numberOfSteps = std::max(0, static_cast<int>(sizeMm / spacingMm));
    latticeSize[axis] = numberOfSteps + 1;
    latticeToRAS->SetElement(axis, axis, spacingMm);
    latticeToRAS->SetElement(axis, 3, bounds_RAS[2 * axis] + 0.5 * (sizeMm - numberOfSteps * spacingMm));
  }
  vtkVectorFieldSampler::GetLatticePositions(latticeToRAS, latticeSize, samplePositions_RAS);
  return true;
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::HasSlicePlane()
{
  // A slice plane is only set for a sampler that serves a slice view; the matrix itself
  // always exists, so it is the field of view that says whether one was given.
  return (this->FieldOfViewSize[0] > 0.0 && this->FieldOfViewSize[1] > 0.0);
}

//----------------------------------------------------------------------------
double vtkVectorFieldSampler::GetHalfSliceThickness()
{
  return 0.5 * (this->SliceThickness > 0.0 ? this->SliceThickness : 1.0);
}

//----------------------------------------------------------------------------
double vtkVectorFieldSampler::GetEffectiveSamplingSpacing()
{
  return (this->SamplingSpacing > 0.0 ? this->SamplingSpacing : this->GetDefaultSamplingSpacing());
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::ComputeSamplePositions(vtkPoints* samplePositions_RAS, int latticeSize[3])
{
  samplePositions_RAS->Initialize();
  latticeSize[0] = 0;
  latticeSize[1] = 0;
  latticeSize[2] = 0;

  // Explicitly requested positions (for example the control points of a markups node)
  if (this->SamplePositions && this->SamplePositions->GetNumberOfPoints() > 0)
  {
    if (!this->HasSlicePlane())
    {
      samplePositions_RAS->DeepCopy(this->SamplePositions);
      this->RemovePositionsOutsideField(samplePositions_RAS);
      return;
    }
    // In a slice view only the positions that the slice shows are sampled
    vtkNew<vtkMatrix4x4> rasToSliceXY;
    vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToSliceXY);
    double halfThicknessMm = this->GetHalfSliceThickness();
    double sliceNormalLengthMm = GetSliceNormalLengthMm(this->SliceXYToRAS);
    vtkIdType numberOfPositions = this->SamplePositions->GetNumberOfPoints();
    for (vtkIdType positionIndex = 0; positionIndex < numberOfPositions; ++positionIndex)
    {
      double position_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
      this->SamplePositions->GetPoint(positionIndex, position_RAS);
      double position_SliceXY[4] = { 0.0, 0.0, 0.0, 1.0 };
      rasToSliceXY->MultiplyPoint(position_RAS, position_SliceXY);
      // The XY axes carry the zoom of the view, the third one the slice spacing. The slab is half-open, so that of
      // layers of positions that are a slice thickness apart (the voxels of a volume) always exactly one is in it.
      double distanceFromSliceMm = position_SliceXY[2] * sliceNormalLengthMm;
      if (distanceFromSliceMm >= -halfThicknessMm && distanceFromSliceMm < halfThicknessMm)
      {
        samplePositions_RAS->InsertNextPoint(position_RAS);
      }
    }
    this->RemovePositionsOutsideField(samplePositions_RAS);
    return;
  }

  double spacingMm = this->GetEffectiveSamplingSpacing();
  if (this->HasSlicePlane() && spacingMm > 0.0)
  {
    // Slice view: an evenly spaced lattice in the plane of the slice, over its field of view.
    // The slice XY coordinate system is in screen units, so the mm spacing is converted into it.
    double xAxis_RAS[3] = { this->SliceXYToRAS->GetElement(0, 0), this->SliceXYToRAS->GetElement(1, 0), this->SliceXYToRAS->GetElement(2, 0) };
    double yAxis_RAS[3] = { this->SliceXYToRAS->GetElement(0, 1), this->SliceXYToRAS->GetElement(1, 1), this->SliceXYToRAS->GetElement(2, 1) };
    double xAxisLengthMm = vtkMath::Norm(xAxis_RAS);
    double yAxisLengthMm = vtkMath::Norm(yAxis_RAS);
    if (xAxisLengthMm <= 0.0 || yAxisLengthMm <= 0.0)
    {
      return;
    }
    // The origin of a slice view's XY coordinate system is a corner of the view, not its
    // center, so the lattice is laid out from that corner.
    double viewSizeX_XY = this->FieldOfViewSize[0] / xAxisLengthMm;
    double viewSizeY_XY = this->FieldOfViewSize[1] / yAxisLengthMm;
    double latticeMin_XY[2] = { 0.0, 0.0 };
    double latticeMax_XY[2] = { viewSizeX_XY, viewSizeY_XY };

    // Cover only the part of the view where the field is: a small mesh in a wide view would otherwise need millions
    // of probes for a few hundred glyphs, and never finishing looks exactly like drawing nothing.
    double fieldBounds_RAS[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    if (this->GetFieldBounds(fieldBounds_RAS) && fieldBounds_RAS[0] <= fieldBounds_RAS[1])
    {
      // The bounds are in the coordinates of the field, the slice is in the world
      vtkNew<vtkMatrix4x4> rasToSliceXY;
      vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToSliceXY);
      vtkNew<vtkGeneralTransform> fieldToSliceXY;
      fieldToSliceXY->PostMultiply();
      if (this->HasFieldToWorldTransform())
      {
        fieldToSliceXY->Concatenate(this->FieldToWorldTransform);
      }
      fieldToSliceXY->Concatenate(rasToSliceXY);
      double fieldBounds_XY[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
      TransformBounds(fieldBounds_RAS, fieldToSliceXY, fieldBounds_XY);
      // A slice that does not reach the field has nothing in it to sample. Along the third axis of
      // the slice coordinate system the distance from the plane is in slice spacings.
      double halfThicknessMm = this->GetHalfSliceThickness();
      double sliceNormalLengthMm = GetSliceNormalLengthMm(this->SliceXYToRAS);
      if (fieldBounds_XY[5] * sliceNormalLengthMm < -halfThicknessMm || fieldBounds_XY[4] * sliceNormalLengthMm > halfThicknessMm)
      {
        return;
      }
      // One step of margin, so that the glyphs at the edge of the object are not left out
      latticeMin_XY[0] = std::max(latticeMin_XY[0], fieldBounds_XY[0] - spacingMm / xAxisLengthMm);
      latticeMin_XY[1] = std::max(latticeMin_XY[1], fieldBounds_XY[2] - spacingMm / yAxisLengthMm);
      latticeMax_XY[0] = std::min(latticeMax_XY[0], fieldBounds_XY[1] + spacingMm / xAxisLengthMm);
      latticeMax_XY[1] = std::min(latticeMax_XY[1], fieldBounds_XY[3] + spacingMm / yAxisLengthMm);
      if (latticeMin_XY[0] >= latticeMax_XY[0] || latticeMin_XY[1] >= latticeMax_XY[1])
      {
        // The field is not within this view at all
        return;
      }
    }

    // The lattice holds a whole number of groups, so that grid visualization draws complete
    // grid cells and never a partial one at the edge.
    int groupSize = std::max(1, this->LatticeGroupSize);
    double latticeSizeX_Mm = (latticeMax_XY[0] - latticeMin_XY[0]) * xAxisLengthMm;
    double latticeSizeY_Mm = (latticeMax_XY[1] - latticeMin_XY[1]) * yAxisLengthMm;
    auto numberOfStepsForSpacing = [groupSize](double sizeMm, double spacing) //
    { return std::max(groupSize, static_cast<int>(sizeMm / (spacing * groupSize)) * groupSize); };
    int numberOfStepsX = numberOfStepsForSpacing(latticeSizeX_Mm, spacingMm);
    int numberOfStepsY = numberOfStepsForSpacing(latticeSizeY_Mm, spacingMm);

    // Coarsen a spacing much finer than the view can show: otherwise the number of samples is unbounded and the
    // sampler never returns, which is indistinguishable from drawing nothing.
    const double maximumNumberOfSamples = 100000.0;
    double numberOfSamples = static_cast<double>(numberOfStepsX + 1) * (numberOfStepsY + 1);
    if (numberOfSamples > maximumNumberOfSamples)
    {
      spacingMm *= std::sqrt(numberOfSamples / maximumNumberOfSamples);
      numberOfStepsX = numberOfStepsForSpacing(latticeSizeX_Mm, spacingMm);
      numberOfStepsY = numberOfStepsForSpacing(latticeSizeY_Mm, spacingMm);
    }

    int numberOfPointsX = numberOfStepsX + 1;
    int numberOfPointsY = numberOfStepsY + 1;
    double stepX_XY = spacingMm / xAxisLengthMm;
    double stepY_XY = spacingMm / yAxisLengthMm;
    // The leftover of the last step is split between the two sides, to center the lattice
    double startX_XY = latticeMin_XY[0] + 0.5 * ((latticeMax_XY[0] - latticeMin_XY[0]) - (numberOfPointsX - 1) * stepX_XY);
    double startY_XY = latticeMin_XY[1] + 0.5 * ((latticeMax_XY[1] - latticeMin_XY[1]) - (numberOfPointsY - 1) * stepY_XY);

    vtkNew<vtkMatrix4x4> latticeToRAS;
    latticeToRAS->DeepCopy(this->SliceXYToRAS);
    double origin_XY[4] = { startX_XY, startY_XY, 0.0, 1.0 };
    double origin_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
    this->SliceXYToRAS->MultiplyPoint(origin_XY, origin_RAS);
    for (int row = 0; row < 3; ++row)
    {
      latticeToRAS->SetElement(row, 0, this->SliceXYToRAS->GetElement(row, 0) * stepX_XY);
      latticeToRAS->SetElement(row, 1, this->SliceXYToRAS->GetElement(row, 1) * stepY_XY);
      latticeToRAS->SetElement(row, 3, origin_RAS[row]);
    }
    latticeSize[0] = numberOfPointsX;
    latticeSize[1] = numberOfPointsY;
    latticeSize[2] = 1;
    vtkVectorFieldSampler::GetLatticePositions(latticeToRAS, latticeSize, samplePositions_RAS);
    return;
  }

  if (this->RegionSize[0] > 0 && this->RegionSize[1] > 0 && this->RegionSize[2] > 0)
  {
    for (int axis = 0; axis < 3; ++axis)
    {
      latticeSize[axis] = this->RegionSize[axis];
    }
    vtkVectorFieldSampler::GetLatticePositions(this->RegionToRAS, latticeSize, samplePositions_RAS);
    return;
  }

  // Leaves the positions empty and the size zero when the source has no default
  this->GetDefaultSamplePositions(samplePositions_RAS, latticeSize);
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::RemovePositionsOutsideField(vtkPoints* samplePositions_RAS)
{
  double fieldBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  if (!samplePositions_RAS || !this->GetFieldBounds(fieldBounds) || fieldBounds[0] > fieldBounds[1])
  {
    // A field without bounds (a transform) has a value everywhere
    return;
  }
  vtkNew<vtkPoints> positions_Field;
  this->GetFieldPositions(samplePositions_RAS, positions_Field);
  vtkNew<vtkPoints> keptPositions_RAS;
  const double tolerance = 1e-6;
  for (vtkIdType pointIndex = 0; pointIndex < samplePositions_RAS->GetNumberOfPoints(); ++pointIndex)
  {
    double position_Field[3] = { 0.0, 0.0, 0.0 };
    positions_Field->GetPoint(pointIndex, position_Field);
    bool inside = true;
    for (int axis = 0; axis < 3 && inside; ++axis)
    {
      inside = (position_Field[axis] >= fieldBounds[2 * axis] - tolerance && position_Field[axis] <= fieldBounds[2 * axis + 1] + tolerance);
    }
    if (inside)
    {
      keptPositions_RAS->InsertNextPoint(samplePositions_RAS->GetPoint(pointIndex));
    }
  }
  if (keptPositions_RAS->GetNumberOfPoints() < samplePositions_RAS->GetNumberOfPoints())
  {
    samplePositions_RAS->DeepCopy(keptPositions_RAS);
  }
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetOutputVectors(vtkPointSet* outputPointSet, vtkDataArray* vectors_RAS)
{
  if (!outputPointSet || !vectors_RAS)
  {
    return;
  }
  vectors_RAS->SetName(VectorArrayName);
  outputPointSet->GetPointData()->SetVectors(vectors_RAS);

  vtkIdType numberOfTuples = vectors_RAS->GetNumberOfTuples();
  vtkNew<vtkDoubleArray> magnitudes;
  magnitudes->SetName(MagnitudeArrayName);
  magnitudes->SetNumberOfValues(numberOfTuples);
  for (vtkIdType i = 0; i < numberOfTuples; ++i)
  {
    double vector[3] = { 0.0, 0.0, 0.0 };
    vectors_RAS->GetTuple(i, vector);
    magnitudes->SetValue(i, vtkMath::Norm(vector));
  }
  outputPointSet->GetPointData()->SetScalars(magnitudes);
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::GetSpacedPointIds(vtkPoints* points, double spacing, vtkIdList* pointIds)
{
  if (!pointIds)
  {
    return;
  }
  pointIds->Reset();
  vtkIdType numberOfPoints = (points ? points->GetNumberOfPoints() : 0);
  if (spacing <= 0.0)
  {
    pointIds->SetNumberOfIds(numberOfPoints);
    for (vtkIdType pointId = 0; pointId < numberOfPoints; ++pointId)
    {
      pointIds->SetId(pointId, pointId);
    }
    return;
  }
  // Closest to the center, so that points of neighboring cubes do not tend to end up side by side at the cube face
  std::map<std::array<long long, 3>, std::pair<double, vtkIdType>> closestPointOfCube;
  for (vtkIdType pointId = 0; pointId < numberOfPoints; ++pointId)
  {
    double point[3] = { 0.0, 0.0, 0.0 };
    points->GetPoint(pointId, point);
    std::array<long long, 3> cube;
    double squaredDistance = 0.0;
    for (int axis = 0; axis < 3; ++axis)
    {
      cube[axis] = static_cast<long long>(std::floor(point[axis] / spacing));
      double fromCenter = point[axis] - (cube[axis] + 0.5) * spacing;
      squaredDistance += fromCenter * fromCenter;
    }
    auto inserted = closestPointOfCube.insert({ cube, { squaredDistance, pointId } });
    if (!inserted.second && squaredDistance < inserted.first->second.first)
    {
      inserted.first->second = { squaredDistance, pointId };
    }
  }
  std::vector<vtkIdType> ids;
  ids.reserve(closestPointOfCube.size());
  for (const auto& cubeAndPoint : closestPointOfCube)
  {
    ids.push_back(cubeAndPoint.second.second);
  }
  std::sort(ids.begin(), ids.end());
  pointIds->SetNumberOfIds(static_cast<vtkIdType>(ids.size()));
  for (size_t index = 0; index < ids.size(); ++index)
  {
    pointIds->SetId(static_cast<vtkIdType>(index), ids[index]);
  }
}

//----------------------------------------------------------------------------
double vtkVectorFieldSampler::GetBoundsDiagonal(const double bounds[6])
{
  return std::sqrt((bounds[1] - bounds[0]) * (bounds[1] - bounds[0]) //
                   + (bounds[3] - bounds[2]) * (bounds[3] - bounds[2]) + (bounds[5] - bounds[4]) * (bounds[5] - bounds[4]));
}

//----------------------------------------------------------------------------
double vtkVectorFieldSampler::GetComponentOrMagnitude(vtkDataArray* array, vtkIdType tupleIndex, int component /*=-1*/)
{
  int numberOfComponents = array->GetNumberOfComponents();
  if (component >= 0 && component < numberOfComponents)
  {
    return array->GetComponent(tupleIndex, component);
  }
  double squaredMagnitude = 0.0;
  for (int c = 0; c < numberOfComponents; ++c)
  {
    double value = array->GetComponent(tupleIndex, c);
    squaredMagnitude += value * value;
  }
  return std::sqrt(squaredMagnitude);
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::AddOutputColors(vtkPointSet* outputPointSet, vtkDataArray* values, vtkUnsignedCharArray* validSamples /*=nullptr*/, int component /*=-1*/)
{
  if (!outputPointSet || !values)
  {
    return;
  }
  vtkIdType numberOfTuples = outputPointSet->GetNumberOfPoints();
  int numberOfComponents = values->GetNumberOfComponents();
  vtkNew<vtkDoubleArray> colors;
  colors->SetName(ColorArrayName);
  colors->SetNumberOfValues(numberOfTuples);
  for (vtkIdType i = 0; i < numberOfTuples; ++i)
  {
    if (i >= values->GetNumberOfTuples() || (validSamples && i < validSamples->GetNumberOfValues() && validSamples->GetValue(i) == 0))
    {
      colors->SetValue(i, 0.0);
      continue;
    }
    // A multi-component array is colored by its magnitude, unless a component is chosen; a
    // one-component array by its value, sign and all
    colors->SetValue(i, vtkVectorFieldSampler::GetComponentOrMagnitude(values, i, numberOfComponents == 1 ? 0 : component));
  }
  outputPointSet->GetPointData()->AddArray(colors);
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::GenerateLatticeCells(vtkUnstructuredGrid* outputGrid, const int latticeSize[3], vtkUnsignedCharArray* validSamples /*=nullptr*/)
{
  if (!outputGrid || !latticeSize)
  {
    return;
  }
  // A lattice that is flat along an axis (a slice) has quads instead of voxels, and a
  // lattice that is a single line or point has no cells at all.
  int numberOfCells[3] = { 0, 0, 0 };
  int numberOfAxesWithCells = 0;
  for (int axis = 0; axis < 3; ++axis)
  {
    numberOfCells[axis] = std::max(0, latticeSize[axis] - 1);
    if (numberOfCells[axis] > 0)
    {
      ++numberOfAxesWithCells;
    }
  }
  if (numberOfAxesWithCells < 2)
  {
    return;
  }

  auto pointId = [&latticeSize](int i, int j, int k)
  { return static_cast<vtkIdType>(i) + static_cast<vtkIdType>(j) * latticeSize[0] + static_cast<vtkIdType>(k) * latticeSize[0] * latticeSize[1]; };

  // A cell whose corners are not all inside the field would interpolate between a real value
  // and nothing, which draws grid lines and isosurfaces through empty space.
  auto allSamplesValid = [validSamples](const vtkIdType* cornerIds, int numberOfCorners)
  {
    if (!validSamples)
    {
      return true;
    }
    for (int corner = 0; corner < numberOfCorners; ++corner)
    {
      if (cornerIds[corner] >= validSamples->GetNumberOfValues() || validSamples->GetValue(cornerIds[corner]) == 0)
      {
        return false;
      }
    }
    return true;
  };

  vtkNew<vtkCellArray> cells;
  if (numberOfAxesWithCells == 3)
  {
    // Hexahedra, not voxels: lattices are often not axis-aligned, and mismatched voxel cells give a stream tracer no start
    // cell. Corners go counterclockwise around the bottom face seen from the top, reversed in a left-handed (LPS) lattice.
    bool rightHanded = true;
    vtkPoints* points = outputGrid->GetPoints();
    if (points && points->GetNumberOfPoints() >= pointId(1, 1, 1) + 1)
    {
      double origin[3] = { 0.0, 0.0, 0.0 };
      double axes[3][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };
      points->GetPoint(pointId(0, 0, 0), origin);
      points->GetPoint(pointId(1, 0, 0), axes[0]);
      points->GetPoint(pointId(0, 1, 0), axes[1]);
      points->GetPoint(pointId(0, 0, 1), axes[2]);
      for (int axis = 0; axis < 3; ++axis)
      {
        vtkMath::Subtract(axes[axis], origin, axes[axis]);
      }
      rightHanded = (vtkMath::Determinant3x3(axes[0], axes[1], axes[2]) >= 0.0);
    }
    int bottom = (rightHanded ? 0 : 1);
    int top = 1 - bottom;
    cells->AllocateEstimate(static_cast<vtkIdType>(numberOfCells[0]) * numberOfCells[1] * numberOfCells[2], 8);
    for (int k = 0; k < numberOfCells[2]; ++k)
    {
      for (int j = 0; j < numberOfCells[1]; ++j)
      {
        for (int i = 0; i < numberOfCells[0]; ++i)
        {
          // VTK_HEXAHEDRON point order
          vtkIdType hexahedronPointIds[8] = { pointId(i, j, k + bottom), pointId(i + 1, j, k + bottom), pointId(i + 1, j + 1, k + bottom), pointId(i, j + 1, k + bottom),
                                              pointId(i, j, k + top),    pointId(i + 1, j, k + top),    pointId(i + 1, j + 1, k + top),    pointId(i, j + 1, k + top) };
          if (!allSamplesValid(hexahedronPointIds, 8))
          {
            continue;
          }
          cells->InsertNextCell(8, hexahedronPointIds);
        }
      }
    }
    outputGrid->SetCells(VTK_HEXAHEDRON, cells);
    return;
  }

  // Flat lattice (a single slice of samples): quads in the plane of the two axes that have
  // more than one sample.
  int firstAxis = -1;
  int secondAxis = -1;
  for (int axis = 0; axis < 3; ++axis)
  {
    if (numberOfCells[axis] > 0)
    {
      (firstAxis < 0 ? firstAxis : secondAxis) = axis;
    }
  }
  cells->AllocateEstimate(static_cast<vtkIdType>(numberOfCells[firstAxis]) * numberOfCells[secondAxis], 4);
  for (int second = 0; second < numberOfCells[secondAxis]; ++second)
  {
    for (int first = 0; first < numberOfCells[firstAxis]; ++first)
    {
      int corner00[3] = { 0, 0, 0 };
      corner00[firstAxis] = first;
      corner00[secondAxis] = second;
      int corner10[3] = { corner00[0], corner00[1], corner00[2] };
      corner10[firstAxis] += 1;
      int corner11[3] = { corner10[0], corner10[1], corner10[2] };
      corner11[secondAxis] += 1;
      int corner01[3] = { corner00[0], corner00[1], corner00[2] };
      corner01[secondAxis] += 1;
      vtkIdType quadPointIds[4] = { pointId(corner00[0], corner00[1], corner00[2]),
                                    pointId(corner10[0], corner10[1], corner10[2]),
                                    pointId(corner11[0], corner11[1], corner11[2]),
                                    pointId(corner01[0], corner01[1], corner01[2]) };
      if (!allSamplesValid(quadPointIds, 4))
      {
        continue;
      }
      cells->InsertNextCell(4, quadPointIds);
    }
  }
  outputGrid->SetCells(VTK_QUAD, cells);
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetFieldSource(int fieldSource)
{
  if (fieldSource < FieldSourceNone || fieldSource >= FieldSource_Last)
  {
    vtkErrorMacro("SetFieldSource: invalid field source " << fieldSource);
    return;
  }
  if (this->FieldSource == fieldSource)
  {
    return;
  }
  this->FieldSource = fieldSource;
  // Release what only the previous source used, so it is not kept alive and its changes do not re-execute the sampler.
  // The input of one source (image, mesh) is never the input of another.
  this->SetInputConnection(nullptr);
  if (fieldSource != FieldSourceScattered)
  {
    this->ScatteredFittedTransform = nullptr;
    this->ScatteredColorFittedTransform = nullptr;
    this->ScatteredPointLocator = nullptr;
    this->ScatteredPointLocatorPoints = nullptr;
  }
  if (fieldSource != FieldSourceTransform)
  {
    this->Transform = nullptr;
  }
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkVectorFieldSampler::GetFieldSourceAsString(int fieldSource)
{
  switch (fieldSource)
  {
    case FieldSourceNone: return "None";
    case FieldSourceImage: return "Image";
    case FieldSourceMesh: return "Mesh";
    case FieldSourceScattered: return "Scattered";
    case FieldSourceTransform: return "Transform";
    default: return "";
  }
}

//----------------------------------------------------------------------------
int vtkVectorFieldSampler::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  // An image for the image source, a mesh for the mesh source, nothing for the others
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkVectorFieldSampler::GetSampledObjectMTime()
{
  switch (this->FieldSource)
  {
    case FieldSourceImage:
    case FieldSourceMesh:
    {
      vtkDataObject* input = this->GetInputDataObject(0, 0);
      return (input ? input->GetMTime() : 0);
    }
    case FieldSourceScattered:
    {
      // The fitted transforms include the modified time of their input points, so editing
      // the data re-samples the field on the next render
      vtkMTimeType mTime = (this->ScatteredFittedTransform ? this->ScatteredFittedTransform->GetMTime() : 0);
      if (this->ScatteredColorFittedTransform)
      {
        mTime = std::max(mTime, this->ScatteredColorFittedTransform->GetMTime());
      }
      return mTime;
    }
    case FieldSourceTransform:
      // Includes the transforms that it concatenates, so that dragging a transform
      // re-samples the field on the next render instead of on every mouse move
      return (this->Transform ? this->Transform->GetMTime() : 0);
    default: return 0;
  }
}

//----------------------------------------------------------------------------
double vtkVectorFieldSampler::GetDefaultSamplingSpacing()
{
  if (this->FieldSource == FieldSourceImage)
  {
    // Follow the resolution of the image
    double voxelSizeMm[3] = { 0.0, 0.0, 0.0 };
    for (int axis = 0; axis < 3; ++axis)
    {
      double axisVector[3] = { this->IJKToRAS->GetElement(0, axis), this->IJKToRAS->GetElement(1, axis), this->IJKToRAS->GetElement(2, axis) };
      voxelSizeMm[axis] = vtkMath::Norm(axisVector);
    }
    return std::max(std::max(voxelSizeMm[0], voxelSizeMm[1]), voxelSizeMm[2]);
  }
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  bool hasBounds = false;
  if (this->FieldSource == FieldSourceMesh)
  {
    vtkPointSet* mesh = vtkPointSet::SafeDownCast(this->GetInputDataObject(0, 0));
    if (mesh && mesh->GetNumberOfPoints() > 0)
    {
      mesh->GetBounds(bounds);
      hasBounds = true;
    }
  }
  else if (this->FieldSource == FieldSourceScattered)
  {
    hasBounds = this->ScatteredGetPointBounds(bounds);
  }
  if (!hasBounds)
  {
    return 1.0;
  }
  // Probing a mesh or evaluating a fit costs far more per sample than evaluating a
  // transform, so the default is a lattice that is readable rather than fine
  double diagonal = vtkVectorFieldSampler::GetBoundsDiagonal(bounds);
  return (diagonal > 0.0 ? diagonal / 50.0 : 1.0);
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::GetFieldBounds(double bounds_RAS[6])
{
  switch (this->FieldSource)
  {
    case FieldSourceImage: return this->ImageGetFieldBounds(bounds_RAS);
    case FieldSourceMesh:
    {
      vtkPointSet* mesh = vtkPointSet::SafeDownCast(this->GetInputDataObject(0, 0));
      if (!mesh || mesh->GetNumberOfPoints() == 0)
      {
        return false;
      }
      mesh->GetBounds(bounds_RAS);
      return (bounds_RAS[0] <= bounds_RAS[1]);
    }
    case FieldSourceScattered:
    {
      if (!this->ScatteredGetPointBounds(bounds_RAS))
      {
        return false;
      }
      // Grow the bounds by the maximum distance, so the field is also drawn on the outer side of the outermost points.
      double maximumDistanceMm = this->GetScatteredEffectiveMaximumDistance();
      for (int axis = 0; axis < 3; ++axis)
      {
        bounds_RAS[2 * axis] -= maximumDistanceMm;
        bounds_RAS[2 * axis + 1] += maximumDistanceMm;
      }
      return true;
    }
    default:
      // A transform is defined everywhere
      return false;
  }
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::GetDefaultSamplePositions(vtkPoints* samplePositions_RAS, int latticeSize[3])
{
  if (!samplePositions_RAS)
  {
    return false;
  }
  if (this->FieldSource == FieldSourceImage)
  {
    return this->ImageGetDefaultSamplePositions(samplePositions_RAS, latticeSize);
  }
  // A lattice over the bounds of the mesh or of the fitted field. A transform has no bounds,
  // so nothing is drawn until a region is given.
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  if (!this->GetFieldBounds(bounds))
  {
    return false;
  }
  return vtkVectorFieldSampler::GetLatticePositionsInBounds(bounds, this->GetEffectiveSamplingSpacing(), samplePositions_RAS, latticeSize);
}

//----------------------------------------------------------------------------
int vtkVectorFieldSampler::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkUnstructuredGrid* output = vtkUnstructuredGrid::GetData(outputVector);
  if (!output)
  {
    return 0;
  }
  // The output is empty here (the pipeline empties it before every execution), and stays so
  // unless the source has a field at the sample positions
  vtkNew<vtkPoints> samplePositions_RAS;
  int latticeSize[3] = { 0, 0, 0 };
  if (this->FieldSource != FieldSourceNone)
  {
    this->ComputeSamplePositions(samplePositions_RAS, latticeSize);
  }
  this->SetOutputLatticeSize(output, latticeSize);
  if (samplePositions_RAS->GetNumberOfPoints() == 0)
  {
    return 1;
  }
  switch (this->FieldSource)
  {
    case FieldSourceImage: this->ImageRequestData(vtkImageData::GetData(inputVector[0]), output, samplePositions_RAS, latticeSize); break;
    case FieldSourceMesh: this->MeshRequestData(vtkPointSet::GetData(inputVector[0]), output, samplePositions_RAS, latticeSize); break;
    case FieldSourceScattered: this->ScatteredRequestData(output, samplePositions_RAS, latticeSize); break;
    case FieldSourceTransform: this->TransformRequestData(output, samplePositions_RAS, latticeSize); break;
    default: break;
  }
  return 1;
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::FinishOutput(vtkUnstructuredGrid* output,
                                         vtkPoints* samplePositions_RAS,
                                         const int latticeSize[3],
                                         vtkDataArray* vectors_RAS,
                                         vtkUnsignedCharArray* validSamples /*=nullptr*/,
                                         vtkDataArray* colorValues /*=nullptr*/,
                                         int colorComponent /*=-1*/)
{
  output->SetPoints(samplePositions_RAS);
  vtkVectorFieldSampler::SetOutputVectors(output, vectors_RAS);
  if (validSamples)
  {
    output->GetPointData()->AddArray(validSamples);
  }
  if (colorValues)
  {
    vtkVectorFieldSampler::AddOutputColors(output, colorValues, validSamples, colorComponent);
  }
  if (this->GenerateCells)
  {
    vtkVectorFieldSampler::GenerateLatticeCells(output, latticeSize, validSamples);
  }
}

//----------------------------------------------------------------------------
// Image source

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetIJKToRAS(vtkMatrix4x4* ijkToRAS)
{
  if (ijkToRAS && CopyMatrixIfChanged(this->IJKToRAS, ijkToRAS))
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkVectorFieldSampler::GetIJKToRAS()
{
  return this->IJKToRAS;
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::ImageGetFieldBounds(double bounds_RAS[6])
{
  vtkImageData* image = vtkImageData::SafeDownCast(this->GetInput());
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
  // The image is a box in IJK, but an arbitrarily oriented one in RAS, so its RAS bounds are
  // the bounds of its eight corners.
  double bounds_IJK[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  std::copy(extent, extent + 6, bounds_IJK);
  vtkNew<vtkGeneralTransform> ijkToRAS;
  ijkToRAS->Concatenate(this->IJKToRAS);
  TransformBounds(bounds_IJK, ijkToRAS, bounds_RAS);
  return true;
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::ImageGetDefaultSamplePositions(vtkPoints* samplePositions_RAS, int latticeSize[3])
{
  vtkImageData* image = vtkImageData::SafeDownCast(this->GetInput());
  if (!image)
  {
    return false;
  }
  // The whole image: step over the voxels so that the samples are SamplingSpacing apart.
  int extent[6] = { 0, -1, 0, -1, 0, -1 };
  image->GetExtent(extent);
  vtkNew<vtkMatrix4x4> latticeToRAS;
  latticeToRAS->DeepCopy(this->IJKToRAS);
  for (int axis = 0; axis < 3; ++axis)
  {
    int numberOfVoxels = extent[2 * axis + 1] - extent[2 * axis] + 1;
    double axisVector[3] = { this->IJKToRAS->GetElement(0, axis), this->IJKToRAS->GetElement(1, axis), this->IJKToRAS->GetElement(2, axis) };
    double voxelSizeMm = vtkMath::Norm(axisVector);
    int stride = 1;
    if (this->SamplingSpacing > 0.0 && voxelSizeMm > 0.0)
    {
      stride = std::max(1, static_cast<int>(this->SamplingSpacing / voxelSizeMm + 0.5));
    }
    // Every stride-th voxel, including the last one when the stride divides the extent (dividing the number of voxels
    // instead would stop up to a stride short of the far side).
    latticeSize[axis] = std::max(1, (numberOfVoxels - 1) / stride + 1);
    for (int row = 0; row < 3; ++row)
    {
      latticeToRAS->SetElement(row, axis, this->IJKToRAS->GetElement(row, axis) * stride);
    }
  }
  // Start at the first voxel of the extent
  double firstVoxel_IJK[4] = { static_cast<double>(extent[0]), static_cast<double>(extent[2]), static_cast<double>(extent[4]), 1.0 };
  double firstVoxel_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
  this->IJKToRAS->MultiplyPoint(firstVoxel_IJK, firstVoxel_RAS);
  for (int row = 0; row < 3; ++row)
  {
    latticeToRAS->SetElement(row, 3, firstVoxel_RAS[row]);
  }
  vtkVectorFieldSampler::GetLatticePositions(latticeToRAS, latticeSize, samplePositions_RAS);
  return true;
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::ImageRequestData(vtkImageData* image, vtkUnstructuredGrid* output, vtkPoints* samplePositions_RAS, const int latticeSize[3])
{
  if (!image || image->GetNumberOfScalarComponents() < 3)
  {
    return;
  }
  vtkIdType numberOfSamples = samplePositions_RAS->GetNumberOfPoints();

  vtkNew<vtkMatrix4x4> rasToIJK;
  vtkMatrix4x4::Invert(this->IJKToRAS, rasToIJK);

  double imageOrigin[3] = { 0.0, 0.0, 0.0 };
  double imageSpacing[3] = { 1.0, 1.0, 1.0 };
  image->GetOrigin(imageOrigin);
  image->GetSpacing(imageSpacing);

  vtkNew<vtkImageInterpolator> interpolator;
  interpolator->SetInterpolationModeToLinear();
  // Sampling positions can fall outside the image (a slice view is usually larger than the
  // volume); those samples must be zero, not the value of the closest voxel.
  interpolator->SetOutValue(0.0);
  interpolator->Initialize(image);
  interpolator->Update();

  // Keep positions outside the image, so the samples stay a regular lattice whose cells allow contours, streamlines
  // and deformed grids. Vectors outside the image are zero, which draws no glyph.
  vtkNew<vtkDoubleArray> vectors_RAS;
  vectors_RAS->SetNumberOfComponents(3);
  vectors_RAS->SetNumberOfTuples(numberOfSamples);

  int imageExtent[6] = { 0, -1, 0, -1, 0, -1 };
  image->GetExtent(imageExtent);

  // The positions where the image is looked up, in the coordinates of the volume
  vtkNew<vtkPoints> samplePositions_Field;
  this->GetFieldPositions(samplePositions_RAS, samplePositions_Field);

  int numberOfComponents = image->GetNumberOfScalarComponents();
  std::vector<double> voxelValue(numberOfComponents, 0.0);
  for (vtkIdType sampleIndex = 0; sampleIndex < numberOfSamples; ++sampleIndex)
  {
    double point_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
    samplePositions_Field->GetPoint(sampleIndex, point_RAS);
    double point_IJK[4] = { 0.0, 0.0, 0.0, 1.0 };
    rasToIJK->MultiplyPoint(point_RAS, point_IJK);
    // vtkImageInterpolator works in the data coordinate system of the image
    double point_Data[3] = { imageOrigin[0] + point_IJK[0] * imageSpacing[0], //
                             imageOrigin[1] + point_IJK[1] * imageSpacing[1],
                             imageOrigin[2] + point_IJK[2] * imageSpacing[2] };
    bool insideImage = (point_IJK[0] >= imageExtent[0] && point_IJK[0] <= imageExtent[1] //
                        && point_IJK[1] >= imageExtent[2] && point_IJK[1] <= imageExtent[3] && point_IJK[2] >= imageExtent[4] && point_IJK[2] <= imageExtent[5]);
    if (insideImage)
    {
      interpolator->Interpolate(point_Data, voxelValue.data());
    }
    else
    {
      std::fill(voxelValue.begin(), voxelValue.end(), 0.0);
    }

    double vector_RAS[3] = { voxelValue[0], voxelValue[1], voxelValue[2] };
    if (!this->VectorsInRAS)
    {
      // The components are along the IJK axes, rotate them into RAS
      double vector_IJK[4] = { vector_RAS[0], vector_RAS[1], vector_RAS[2], 0.0 };
      double rotated_RAS[4] = { 0.0, 0.0, 0.0, 0.0 };
      this->IJKToRAS->MultiplyPoint(vector_IJK, rotated_RAS);
      vector_RAS[0] = rotated_RAS[0];
      vector_RAS[1] = rotated_RAS[1];
      vector_RAS[2] = rotated_RAS[2];
    }
    vectors_RAS->SetTuple(sampleIndex, vector_RAS);
  }
  this->TransformVectorsToWorld(samplePositions_Field, vectors_RAS);
  this->FinishOutput(output, samplePositions_RAS, latticeSize, vectors_RAS);
}

//----------------------------------------------------------------------------
// Mesh source

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::MeshRequestData(vtkPointSet* mesh, vtkUnstructuredGrid* output, vtkPoints* samplePositions_RAS, const int latticeSize[3])
{
  if (!mesh || mesh->GetNumberOfPoints() == 0)
  {
    return;
  }
  vtkIdType numberOfSamples = samplePositions_RAS->GetNumberOfPoints();

  // The positions where the mesh is probed, in the coordinates of the mesh
  vtkNew<vtkPoints> samplePositions_Field;
  this->GetFieldPositions(samplePositions_RAS, samplePositions_Field);
  vtkNew<vtkPolyData> probePoints;
  probePoints->SetPoints(samplePositions_Field);

  vtkNew<vtkProbeFilter> probe;
  probe->SetInputData(probePoints);
  probe->SetSourceData(mesh);
  // Use the cell locator of the mesh itself (built once, rebuilt only when the mesh changes): a locator kept here
  // would leak the mesh once a stream tracer has also built a point locator on it.
  vtkNew<vtkCellLocatorStrategy> strategy;
  probe->SetFindCellStrategy(strategy);
  probe->Update();

  vtkPointData* probedPointData = probe->GetOutput()->GetPointData();
  vtkDataArray* probedVectors = nullptr;
  if (this->MeshVectorArrayName && this->MeshVectorArrayName[0] != '\0')
  {
    probedVectors = probedPointData->GetArray(this->MeshVectorArrayName);
  }
  if (!probedVectors)
  {
    probedVectors = probedPointData->GetVectors();
  }
  if (!probedVectors || probedVectors->GetNumberOfComponents() < 3)
  {
    return;
  }

  vtkNew<vtkDoubleArray> vectors_RAS;
  vectors_RAS->SetNumberOfComponents(3);
  vectors_RAS->SetNumberOfTuples(numberOfSamples);
  // Read as vtkDataArray: vtkProbeFilter stores the mask as vtkCharArray, and a wrong concrete type would give a null
  // array and a mask that is silently never applied.
  vtkDataArray* validMask = probedPointData->GetArray(probe->GetValidPointMaskArrayName());
  if (!validMask)
  {
    vtkWarningMacro("MeshRequestData: the probe reported no valid-point mask, "
                    "samples outside the mesh cannot be told apart from samples with a zero vector");
  }
  // Samples outside the mesh cells get a zero vector (no glyph) and are marked invalid, so grid lines and lattice
  // cells stop there instead of spanning the empty space in the bounding box.
  vtkNew<vtkUnsignedCharArray> validSamples;
  validSamples->SetName(vtkVectorFieldSampler::GetValidSampleArrayName());
  validSamples->SetNumberOfValues(numberOfSamples);
  for (vtkIdType sampleIndex = 0; sampleIndex < numberOfSamples; ++sampleIndex)
  {
    if (validMask && validMask->GetComponent(sampleIndex, 0) == 0)
    {
      vectors_RAS->SetTuple3(sampleIndex, 0.0, 0.0, 0.0);
      validSamples->SetValue(sampleIndex, 0);
      continue;
    }
    double vector[3] = { 0.0, 0.0, 0.0 };
    probedVectors->GetTuple(sampleIndex, vector);
    vectors_RAS->SetTuple(sampleIndex, vector);
    validSamples->SetValue(sampleIndex, 1);
  }
  this->TransformVectorsToWorld(samplePositions_Field, vectors_RAS);

  // The probe interpolates every array of the mesh, point and cell data alike, so the array
  // the field is colored by is read from the same probe as the vectors
  vtkDataArray* probedColors = nullptr;
  if (this->MeshColorArrayName && this->MeshColorArrayName[0] != '\0')
  {
    probedColors = probedPointData->GetArray(this->MeshColorArrayName);
  }
  this->FinishOutput(output, samplePositions_RAS, latticeSize, vectors_RAS, validSamples, probedColors, this->MeshColorArrayComponent);
}

//----------------------------------------------------------------------------
// Scattered source

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetScatteredFittedTransform(vtkScatteredBSplineTransform* transform)
{
  if (this->ScatteredFittedTransform == transform)
  {
    return;
  }
  this->ScatteredFittedTransform = transform;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkScatteredBSplineTransform* vtkVectorFieldSampler::GetScatteredFittedTransform()
{
  return this->ScatteredFittedTransform;
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetScatteredColorFittedTransform(vtkScatteredBSplineTransform* transform)
{
  if (this->ScatteredColorFittedTransform == transform)
  {
    return;
  }
  this->ScatteredColorFittedTransform = transform;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkScatteredBSplineTransform* vtkVectorFieldSampler::GetScatteredColorFittedTransform()
{
  return this->ScatteredColorFittedTransform;
}

//----------------------------------------------------------------------------
bool vtkVectorFieldSampler::ScatteredGetPointBounds(double bounds_RAS[6])
{
  vtkPointSet* points = (this->ScatteredFittedTransform ? this->ScatteredFittedTransform->GetInputPointSet() : nullptr);
  if (!points || points->GetNumberOfPoints() == 0)
  {
    return false;
  }
  points->GetBounds(bounds_RAS);
  return (bounds_RAS[0] <= bounds_RAS[1]);
}

//----------------------------------------------------------------------------
double vtkVectorFieldSampler::GetScatteredEffectiveMaximumDistance()
{
  if (this->ScatteredMaximumDistance > 0.0)
  {
    return this->ScatteredMaximumDistance;
  }
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  if (!this->ScatteredGetPointBounds(bounds))
  {
    return 0.0;
  }
  return 0.1 * vtkVectorFieldSampler::GetBoundsDiagonal(bounds);
}

//----------------------------------------------------------------------------
vtkStaticPointLocator* vtkVectorFieldSampler::ScatteredGetPointLocator()
{
  vtkPointSet* points = (this->ScatteredFittedTransform ? this->ScatteredFittedTransform->GetInputPointSet() : nullptr);
  if (!points || points->GetNumberOfPoints() == 0)
  {
    return nullptr;
  }
  if (this->ScatteredPointLocator && this->ScatteredPointLocatorPoints == points && this->ScatteredPointLocatorPointsMTime == points->GetMTime())
  {
    return this->ScatteredPointLocator;
  }
  this->ScatteredPointLocator = vtkSmartPointer<vtkStaticPointLocator>::New();
  this->ScatteredPointLocator->SetDataSet(points);
  this->ScatteredPointLocator->BuildLocator();
  this->ScatteredPointLocatorPoints = points;
  this->ScatteredPointLocatorPointsMTime = points->GetMTime();
  return this->ScatteredPointLocator;
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::ScatteredRequestData(vtkUnstructuredGrid* output, vtkPoints* samplePositions_RAS, const int latticeSize[3])
{
  vtkStaticPointLocator* locator = this->ScatteredGetPointLocator();
  if (!this->ScatteredFittedTransform || !locator)
  {
    return;
  }
  vtkIdType numberOfSamples = samplePositions_RAS->GetNumberOfPoints();
  vtkPointSet* points = this->ScatteredFittedTransform->GetInputPointSet();
  double maximumDistanceMm = this->GetScatteredEffectiveMaximumDistance();
  double maximumDistance2 = maximumDistanceMm * maximumDistanceMm;

  vtkNew<vtkDoubleArray> vectors_RAS;
  vectors_RAS->SetNumberOfComponents(3);
  vectors_RAS->SetNumberOfTuples(numberOfSamples);
  vtkNew<vtkUnsignedCharArray> validSamples;
  validSamples->SetName(vtkVectorFieldSampler::GetValidSampleArrayName());
  validSamples->SetNumberOfValues(numberOfSamples);
  // The values the field is colored by, from a fit of their own to the same data points
  vtkSmartPointer<vtkDoubleArray> colorValues;
  if (this->ScatteredColorFittedTransform)
  {
    colorValues = vtkSmartPointer<vtkDoubleArray>::New();
    colorValues->SetNumberOfValues(numberOfSamples);
  }
  // The positions where the fit is evaluated, in the coordinates of the data points
  vtkNew<vtkPoints> samplePositions_Field;
  this->GetFieldPositions(samplePositions_RAS, samplePositions_Field);
  for (vtkIdType sampleIndex = 0; sampleIndex < numberOfSamples; ++sampleIndex)
  {
    double position_Field[3] = { 0.0, 0.0, 0.0 };
    samplePositions_Field->GetPoint(sampleIndex, position_Field);
    // Far from the data the fit is an extrapolation: the sample gets a zero vector and is marked invalid, so grid
    // lines and lattice cells stop where the data does.
    vtkIdType closestPointId = locator->FindClosestPoint(position_Field);
    double closestPoint[3] = { 0.0, 0.0, 0.0 };
    if (closestPointId >= 0)
    {
      points->GetPoint(closestPointId, closestPoint);
    }
    if (closestPointId < 0 || vtkMath::Distance2BetweenPoints(position_Field, closestPoint) > maximumDistance2)
    {
      vectors_RAS->SetTuple3(sampleIndex, 0.0, 0.0, 0.0);
      validSamples->SetValue(sampleIndex, 0);
      continue;
    }
    // The fitted transform maps a position to that position plus the vector there
    double transformedPosition_Field[3] = { 0.0, 0.0, 0.0 };
    this->ScatteredFittedTransform->TransformPoint(position_Field, transformedPosition_Field);
    vectors_RAS->SetTuple3(sampleIndex, //
                           transformedPosition_Field[0] - position_Field[0],
                           transformedPosition_Field[1] - position_Field[1],
                           transformedPosition_Field[2] - position_Field[2]);
    validSamples->SetValue(sampleIndex, 1);
    if (colorValues)
    {
      // The value is fitted as the first component of a displacement
      double colorPosition[3] = { 0.0, 0.0, 0.0 };
      this->ScatteredColorFittedTransform->TransformPoint(position_Field, colorPosition);
      colorValues->SetValue(sampleIndex, colorPosition[0] - position_Field[0]);
    }
  }
  this->TransformVectorsToWorld(samplePositions_Field, vectors_RAS);
  this->FinishOutput(output, samplePositions_RAS, latticeSize, vectors_RAS, validSamples, colorValues);
}

//----------------------------------------------------------------------------
// Transform source

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::SetTransform(vtkAbstractTransform* transform)
{
  if (this->Transform == transform)
  {
    return;
  }
  this->Transform = transform;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkVectorFieldSampler::GetTransform()
{
  return this->Transform;
}

//----------------------------------------------------------------------------
void vtkVectorFieldSampler::TransformRequestData(vtkUnstructuredGrid* output, vtkPoints* samplePositions_RAS, const int latticeSize[3])
{
  if (!this->Transform)
  {
    return;
  }
  vtkIdType numberOfSamples = samplePositions_RAS->GetNumberOfPoints();

  // The transform is evaluated in world coordinates, where the samples are, so the field to
  // world transform is not used
  vtkNew<vtkDoubleArray> vectors_RAS;
  vectors_RAS->SetNumberOfComponents(3);
  vectors_RAS->SetNumberOfTuples(numberOfSamples);
  for (vtkIdType sampleIndex = 0; sampleIndex < numberOfSamples; ++sampleIndex)
  {
    double point_RAS[3] = { 0.0, 0.0, 0.0 };
    samplePositions_RAS->GetPoint(sampleIndex, point_RAS);
    double transformedPoint_RAS[3] = { 0.0, 0.0, 0.0 };
    this->Transform->TransformPoint(point_RAS, transformedPoint_RAS);
    double displacement_RAS[3] = { transformedPoint_RAS[0] - point_RAS[0], //
                                   transformedPoint_RAS[1] - point_RAS[1],
                                   transformedPoint_RAS[2] - point_RAS[2] };
    vectors_RAS->SetTuple(sampleIndex, displacement_RAS);
  }
  this->FinishOutput(output, samplePositions_RAS, latticeSize, vectors_RAS);
}
