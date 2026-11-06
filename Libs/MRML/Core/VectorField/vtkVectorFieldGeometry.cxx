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
#include "vtkVectorFieldGeometry.h"
#include "vtkVectorFieldGrid.h"
#include "vtkVectorFieldSampler.h"

// vtkAddon includes
#include <vtkAddonMathUtilities.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkArrayCalculator.h>
#include <vtkAssignAttribute.h>
#include <vtkCellData.h>
#include <vtkClipPolyData.h>
#include <vtkContourFilter.h>
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPassInputTypeAlgorithm.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkStreamTracer.h>
#include <vtkThreshold.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTrivialProducer.h>
#include <vtkTubeFilter.h>
#include <vtkUnsignedCharArray.h>

// STD includes
#include <algorithm>
#include <vector>

namespace
{
/// The value that streamlines are thresholded by
const char StreamlineThresholdValueArrayName[] = "StreamlineThresholdValue";

//----------------------------------------------------------------------------
/// Traces nothing in an empty field (e.g. all masked out), where vtkStreamTracer searches with a never-built locator in
/// each thread and crashes at random.
class vtkEmptyFieldSafeStreamTracer : public vtkStreamTracer
{
public:
  static vtkEmptyFieldSafeStreamTracer* New();
  vtkTypeMacro(vtkEmptyFieldSafeStreamTracer, vtkStreamTracer);

protected:
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override
  {
    vtkDataSet* field = vtkDataSet::GetData(inputVector[0]);
    if (field && (field->GetNumberOfPoints() == 0 || field->GetNumberOfCells() == 0))
    {
      vtkPolyData::GetData(outputVector)->Initialize();
      return 1;
    }
    return this->Superclass::RequestData(request, inputVector, outputVector);
  }
};
vtkStandardNewMacro(vtkEmptyFieldSafeStreamTracer);
} // namespace

//----------------------------------------------------------------------------
/// The image that streamlines are traced through, made from the image of the field.
///
/// A non-axis-aligned image (its axes are not along the coordinate axes) is given in its own axis-aligned space: its
/// axes are the coordinate axes there and its first voxel is at the origin. vtkStreamTracer tests positions against the
/// cell that it found last as a voxel, which is only valid along the coordinate axes: in a non-axis-aligned image
/// streamlines went astray and stopped early, and finding the cell of each position instead takes a few times longer.
/// The vectors are rotated into that space too, and they are the active vectors, so that vtkTransformPolyDataFilter
/// rotates them back with the streamlines.
///
/// The cells that streamlines are not traced through are hidden: those that are not in the domain at all of their
/// points, and those that have no point in the threshold range. vtkStreamTracer finds no position in a hidden cell, so
/// the cells do not have to be extracted into an unstructured grid.
class vtkVectorFieldStreamlineImage : public vtkPassInputTypeAlgorithm
{
public:
  static vtkVectorFieldStreamlineImage* New();
  vtkTypeMacro(vtkVectorFieldStreamlineImage, vtkPassInputTypeAlgorithm);

  /// True if each axis of the image is along the same coordinate axis, in either direction.
  static bool IsAxisAligned(vtkImageData* image)
  {
    vtkMatrix3x3* direction = image->GetDirectionMatrix();
    for (int row = 0; row < 3; ++row)
    {
      for (int column = 0; column < 3; ++column)
      {
        if (row != column && direction->GetElement(row, column) != 0.0)
        {
          return false;
        }
      }
    }
    return true;
  }

  /// Transform from the axis-aligned space of a non-axis-aligned image to the space that the image is in.
  static void GetAxisAlignedToImageMatrix(vtkImageData* image, vtkMatrix4x4* axisAlignedToImage)
  {
    axisAlignedToImage->Identity();
    for (int row = 0; row < 3; ++row)
    {
      for (int column = 0; column < 3; ++column)
      {
        axisAlignedToImage->SetElement(row, column, image->GetDirectionMatrix()->GetElement(row, column));
      }
      axisAlignedToImage->SetElement(row, 3, image->GetOrigin()[row]);
    }
  }

  /// Point array of the vectors that are integrated.
  vtkSetMacro(VectorArrayName, std::string);
  /// Point array that is at least 0.5 in the domain. Empty: the whole image.
  vtkSetMacro(DomainArrayName, std::string);
  ///@{
  /// Point array, its component (-1: magnitude) and the range of its values. Empty: no threshold.
  vtkSetMacro(ThresholdArrayName, std::string);
  vtkSetMacro(ThresholdComponent, int);
  vtkSetVector2Macro(ThresholdRange, double);
  ///@}

protected:
  int RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector) override
  {
    vtkImageData* input = vtkImageData::GetData(inputVector[0]);
    vtkImageData* output = vtkImageData::GetData(outputVector);
    if (!input || !output)
    {
      return 0;
    }
    output->ShallowCopy(input);
    if (input->GetNumberOfPoints() == 0)
    {
      return 1;
    }
    // By the arrays of the input: the output has the vectors under the same name, rotated
    this->HideCells(input, output);
    if (!vtkVectorFieldStreamlineImage::IsAxisAligned(input))
    {
      this->AlignWithCoordinateAxes(input, output);
    }
    return 1;
  }

  void HideCells(vtkImageData* input, vtkImageData* output)
  {
    vtkDataArray* domainArray = (this->DomainArrayName.empty() ? nullptr : input->GetPointData()->GetArray(this->DomainArrayName.c_str()));
    vtkDataArray* thresholdArray = (this->ThresholdArrayName.empty() ? nullptr : input->GetPointData()->GetArray(this->ThresholdArrayName.c_str()));
    if (!domainArray && !thresholdArray)
    {
      return;
    }
    vtkIdType numberOfPoints = input->GetNumberOfPoints();
    std::vector<unsigned char> inDomain(numberOfPoints, 1);
    std::vector<unsigned char> inThresholdRange(numberOfPoints, 1);
    for (vtkIdType pointId = 0; pointId < numberOfPoints; ++pointId)
    {
      if (domainArray)
      {
        inDomain[pointId] = (domainArray->GetComponent(pointId, 0) >= 0.5);
      }
      if (thresholdArray)
      {
        double value = vtkVectorFieldSampler::GetComponentOrMagnitude(thresholdArray, pointId, this->ThresholdComponent);
        inThresholdRange[pointId] = (value >= this->ThresholdRange[0] && value <= this->ThresholdRange[1]);
      }
    }

    // A flat image has a single layer of cells, between a single layer of points
    int dimensions[3] = { 0, 0, 0 };
    input->GetDimensions(dimensions);
    int numberOfCells[3] = { 1, 1, 1 };
    int numberOfCorners[3] = { 1, 1, 1 };
    for (int axis = 0; axis < 3; ++axis)
    {
      numberOfCells[axis] = std::max(dimensions[axis] - 1, 1);
      numberOfCorners[axis] = (dimensions[axis] > 1 ? 2 : 1);
    }
    vtkNew<vtkUnsignedCharArray> ghosts;
    ghosts->SetName(vtkDataSetAttributes::GhostArrayName());
    ghosts->SetNumberOfValues(output->GetNumberOfCells());
    vtkIdType cellId = 0;
    for (int k = 0; k < numberOfCells[2]; ++k)
    {
      for (int j = 0; j < numberOfCells[1]; ++j)
      {
        for (int i = 0; i < numberOfCells[0]; ++i, ++cellId)
        {
          // In the domain at all points, as the cells of a mesh are; a point in the threshold range is enough, so that
          // the streamlines reach the position where the value leaves the range
          bool allInDomain = true;
          bool anyInThresholdRange = false;
          for (int cornerK = 0; cornerK < numberOfCorners[2]; ++cornerK)
          {
            for (int cornerJ = 0; cornerJ < numberOfCorners[1]; ++cornerJ)
            {
              for (int cornerI = 0; cornerI < numberOfCorners[0]; ++cornerI)
              {
                vtkIdType pointId = (i + cornerI) + static_cast<vtkIdType>(dimensions[0]) * ((j + cornerJ) + static_cast<vtkIdType>(dimensions[1]) * (k + cornerK));
                allInDomain = allInDomain && inDomain[pointId];
                anyInThresholdRange = anyInThresholdRange || inThresholdRange[pointId];
              }
            }
          }
          ghosts->SetValue(cellId, (allInDomain && anyInThresholdRange) ? 0 : vtkDataSetAttributes::HIDDENCELL);
        }
      }
    }
    output->GetCellData()->AddArray(ghosts);
  }

  void AlignWithCoordinateAxes(vtkImageData* input, vtkImageData* output)
  {
    vtkDataArray* vectors = input->GetPointData()->GetArray(this->VectorArrayName.c_str());
    if (!vectors || vectors->GetNumberOfComponents() != 3)
    {
      return;
    }
    // The rotated vectors are kept until the vectors or the direction of the image change: the domain and the threshold
    // change more often
    vtkMatrix3x3* direction = input->GetDirectionMatrix();
    bool axisAlignedVectorsValid = (this->AxisAlignedVectors && this->AxisAlignedVectorsSource == vectors && this->AxisAlignedVectorsSourceMTime == vectors->GetMTime());
    for (int element = 0; element < 9 && axisAlignedVectorsValid; ++element)
    {
      axisAlignedVectorsValid = (this->AxisAlignedVectorsDirection[element] == direction->GetData()[element]);
    }
    if (!axisAlignedVectorsValid)
    {
      vtkNew<vtkMatrix3x3> imageToAxisAligned;
      vtkMatrix3x3::Invert(direction, imageToAxisAligned);
      vtkIdType numberOfPoints = vectors->GetNumberOfTuples();
      this->AxisAlignedVectors.TakeReference(vectors->NewInstance());
      this->AxisAlignedVectors->SetName(vectors->GetName());
      this->AxisAlignedVectors->SetNumberOfComponents(3);
      this->AxisAlignedVectors->SetNumberOfTuples(numberOfPoints);
      for (vtkIdType pointId = 0; pointId < numberOfPoints; ++pointId)
      {
        double vector[3] = { 0.0, 0.0, 0.0 };
        double axisAlignedVector[3] = { 0.0, 0.0, 0.0 };
        vectors->GetTuple(pointId, vector);
        imageToAxisAligned->MultiplyPoint(vector, axisAlignedVector);
        this->AxisAlignedVectors->SetTuple(pointId, axisAlignedVector);
      }
      this->AxisAlignedVectorsSource = vectors;
      this->AxisAlignedVectorsSourceMTime = vectors->GetMTime();
      std::copy_n(direction->GetData(), 9, this->AxisAlignedVectorsDirection);
    }
    vtkNew<vtkMatrix3x3> identity;
    output->SetDirectionMatrix(identity);
    output->SetOrigin(0.0, 0.0, 0.0);
    // In place of the vectors of the input
    output->GetPointData()->AddArray(this->AxisAlignedVectors);
    output->GetPointData()->SetActiveVectors(this->AxisAlignedVectors->GetName());
  }

  std::string VectorArrayName;
  std::string DomainArrayName;
  std::string ThresholdArrayName;
  int ThresholdComponent{ -1 };
  double ThresholdRange[2]{ 0.0, -1.0 };

  vtkSmartPointer<vtkDataArray> AxisAlignedVectors;
  vtkDataArray* AxisAlignedVectorsSource{ nullptr };
  vtkMTimeType AxisAlignedVectorsSourceMTime{ 0 };
  double AxisAlignedVectorsDirection[9]{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
};
vtkStandardNewMacro(vtkVectorFieldStreamlineImage);

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVectorFieldGeometry);

//----------------------------------------------------------------------------
vtkVectorFieldGeometry::vtkVectorFieldGeometry()
{
  this->SetNumberOfOutputPorts(3);

  this->RegionScalars->SetInputConnection(nullptr);
  this->RegionClip->SetInputConnection(this->RegionScalars->GetOutputPort());
  this->RegionClip->SetValue(0.5);

  this->FieldMagnitude->SetAttributeTypeToPointData();
  this->FieldMagnitude->SetResultArrayName(vtkVectorFieldSampler::GetMagnitudeArrayName());

  // The cells whose points are all in the domain
  this->StreamlineDomain->SetThresholdFunction(vtkThreshold::THRESHOLD_UPPER);
  this->StreamlineDomain->SetUpperThreshold(0.5);
  this->StreamlineDomain->SetAllScalars(true);
  // The cells that have a point in the threshold range
  this->StreamlineThresholdDomain->SetThresholdFunction(vtkThreshold::THRESHOLD_BETWEEN);
  this->StreamlineThresholdDomain->SetAllScalars(false);
  this->StreamTracer = vtkSmartPointer<vtkEmptyFieldSafeStreamTracer>::New();
  // Set once: setting the integrator type makes a new integrator, which modifies the tracer
  this->StreamTracer->SetIntegratorTypeToRungeKutta45();
  // The vorticity is not used, and it would stay in the space that the streamlines are traced in. It also makes the
  // tracer set normals as the active vectors, in place of the vectors that are rotated back with the streamlines.
  this->StreamTracer->SetComputeVorticity(false);
  this->StreamlineSeedsToTraced->SetInputData(this->StreamlineSeeds);
  this->StreamlineSeedsToTraced->SetTransform(this->StreamlineTracedToField->GetInverse());
  this->StreamTracer->SetSourceConnection(this->StreamlineSeedsToTraced->GetOutputPort());
  this->StreamlinesToField->SetTransform(this->StreamlineTracedToField);

  this->StreamlineThresholdValue->SetAttributeTypeToPointData();
  this->StreamlineThresholdValue->SetResultArrayName(StreamlineThresholdValueArrayName);
  this->StreamlineThresholdScalars->SetInputConnection(this->StreamlineThresholdValue->GetOutputPort());
  this->StreamlineThresholdScalars->Assign(StreamlineThresholdValueArrayName, vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
  this->StreamlineClipBelow->SetInputConnection(this->StreamlineThresholdScalars->GetOutputPort());
  this->StreamlineClipAbove->SetInputConnection(this->StreamlineClipBelow->GetOutputPort());
  this->StreamlineClipAbove->InsideOutOn();
  this->StreamlineTuber->SetNumberOfSides(8);
  this->StreamlineTuber->CappingOn();
}

//----------------------------------------------------------------------------
vtkVectorFieldGeometry::~vtkVectorFieldGeometry() = default;

//----------------------------------------------------------------------------
void vtkVectorFieldGeometry::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Mode: " << this->Mode << "\n";
  os << indent << "Flat: " << (this->Flat ? "On" : "Off") << "\n";
  os << indent << "VectorArrayName: " << this->VectorArrayName << "\n";
  os << indent << "MagnitudeArrayName: " << this->MagnitudeArrayName << "\n";
  os << indent << "RegionArrayName: " << this->RegionArrayName << "\n";
  os << indent << "GridSubdivision: " << this->GridSubdivision << "\n";
  os << indent << "GridScaleFactor: " << this->GridScaleFactor << "\n";
  os << indent << "GridDeformedLinesVisibility: " << (this->GridDeformedLinesVisibility ? "On" : "Off") << "\n";
  os << indent << "GridUndeformedLinesVisibility: " << (this->GridUndeformedLinesVisibility ? "On" : "Off") << "\n";
  os << indent << "GridFill: " << (this->GridFill ? "On" : "Off") << "\n";
  os << indent << "GridLineDiameter: " << this->GridLineDiameter << "\n";
  os << indent << "NumberOfContourLevels: " << this->ContourLevels.size() << "\n";
  os << indent << "NumberOfStreamlineSeeds: " << this->StreamlineSeeds->GetNumberOfPoints() << "\n";
  os << indent << "StreamlineDomainArrayName: " << this->StreamlineDomainArrayName << "\n";
  os << indent << "StreamlineBidirectional: " << (this->StreamlineBidirectional ? "On" : "Off") << "\n";
  os << indent << "StreamlineMaximumPropagation: " << this->StreamlineMaximumPropagation << "\n";
  os << indent << "StreamlineInitialIntegrationStep: " << this->StreamlineInitialIntegrationStep << "\n";
  os << indent << "StreamlineTubeDiameter: " << this->StreamlineTubeDiameter << "\n";
  os << indent << "ThresholdEnabled: " << (this->ThresholdEnabled ? "On" : "Off") << "\n";
  os << indent << "ThresholdArrayName: " << this->ThresholdArrayName << "\n";
  os << indent << "ThresholdComponent: " << this->ThresholdComponent << "\n";
  os << indent << "ThresholdRange: " << this->ThresholdRange[0] << " " << this->ThresholdRange[1] << "\n";
}

//----------------------------------------------------------------------------
void vtkVectorFieldGeometry::SetContourLevels(const std::vector<double>& levels)
{
  if (this->ContourLevels == levels)
  {
    return;
  }
  this->ContourLevels = levels;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkVectorFieldGeometry::SetStreamlineSeeds(vtkPoints* seeds)
{
  if (vtkVectorFieldSampler::ArePointsEqual(this->StreamlineSeeds->GetPoints(), seeds))
  {
    return;
  }
  // Copied, so that the caller can reuse its points; the tracer re-runs only when they differ
  this->StreamlineSeeds->Initialize();
  if (seeds)
  {
    vtkNew<vtkPoints> seedsCopy;
    seedsCopy->DeepCopy(seeds);
    this->StreamlineSeeds->SetPoints(seedsCopy);
  }
  this->Modified();
}

//----------------------------------------------------------------------------
vtkPoints* vtkVectorFieldGeometry::GetStreamlineSeeds()
{
  return this->StreamlineSeeds->GetPoints();
}

//----------------------------------------------------------------------------
int vtkVectorFieldGeometry::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkVectorFieldGeometry::UpdateFieldCopy(vtkDataSet* input)
{
  if (this->FieldCopy && this->FieldCopySource == input && this->FieldCopySourceMTime == input->GetMTime())
  {
    return;
  }
  if (!this->FieldCopy || !this->FieldCopy->IsA(input->GetClassName()))
  {
    this->FieldCopy = vtkSmartPointer<vtkDataSet>::Take(input->NewInstance());
    this->FieldProducer->SetOutput(this->FieldCopy);
  }
  this->FieldCopy->ShallowCopy(input);
  this->FieldCopySource = input;
  this->FieldCopySourceMTime = input->GetMTime();
}

//----------------------------------------------------------------------------
int vtkVectorFieldGeometry::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkDataSet* input = vtkDataSet::GetData(inputVector[0]);
  vtkPolyData* output = vtkPolyData::GetData(outputVector, 0);
  vtkPolyData* streamlinesOutput = vtkPolyData::GetData(outputVector, StreamlineLinesPort);
  if (!input || !output || !streamlinesOutput)
  {
    return 0;
  }
  if (this->Mode == ModeNone || this->VectorArrayName.empty())
  {
    return 1;
  }
  this->UpdateFieldCopy(input);

  vtkAlgorithmOutput* geometryConnection = nullptr;
  vtkAlgorithmOutput* streamlinesConnection = nullptr;
  switch (this->Mode)
  {
    case ModeGrid: geometryConnection = this->ConfigureGrid(); break;
    case ModeContour: geometryConnection = this->ConfigureContour(); break;
    case ModeStreamline: geometryConnection = this->ConfigureStreamlines(streamlinesConnection); break;
    default: break;
  }
  if (geometryConnection)
  {
    vtkAlgorithm* producer = geometryConnection->GetProducer();
    producer->Update(geometryConnection->GetIndex());
    output->ShallowCopy(producer->GetOutputDataObject(geometryConnection->GetIndex()));
    if (this->Mode == ModeGrid)
    {
      vtkPolyData::GetData(outputVector, GridUndeformedLinesPort)->ShallowCopy(this->Grid->GetOutputDataObject(vtkVectorFieldGrid::UndeformedLinesPort));
    }
  }
  if (streamlinesConnection)
  {
    vtkAlgorithm* producer = streamlinesConnection->GetProducer();
    producer->Update(streamlinesConnection->GetIndex());
    streamlinesOutput->ShallowCopy(producer->GetOutputDataObject(streamlinesConnection->GetIndex()));
  }
  return 1;
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkVectorFieldGeometry::ConfigureGrid()
{
  // Lines are drawn every Subdivision-th sample but follow every sample, so a finer sampling shows how the grid curves
  if (!this->GridDeformedLinesVisibility && !this->GridUndeformedLinesVisibility && !this->GridFill)
  {
    return nullptr;
  }
  this->Grid->SetInputConnection(this->FieldProducer->GetOutputPort());
  this->Grid->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, this->VectorArrayName.c_str());
  this->Grid->SetSubdivision(this->GridSubdivision);
  this->Grid->SetScaleFactor(this->GridScaleFactor);
  this->Grid->SetDeformedLinesVisibility(this->GridDeformedLinesVisibility);
  this->Grid->SetUndeformedLinesVisibility(this->GridUndeformedLinesVisibility);
  this->Grid->SetFill(this->GridFill);
  // In a slice view a tube would be seen edge-on and only clutter the image
  this->Grid->SetLineDiameter(this->Flat ? 0.0 : this->GridLineDiameter);
  this->Grid->SetClipArrayName(this->RegionArrayName);
  return this->Grid->GetOutputPort();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkVectorFieldGeometry::ConfigureContour()
{
  // Isosurfaces of the magnitude; a single layer of cells (a slice) gives isolines
  vtkAlgorithmOutput* magnitudeConnection = this->FieldProducer->GetOutputPort();
  if (this->MagnitudeArrayName.empty())
  {
    // Reconfigured only when the vectors change: resetting the variables modifies the calculator
    if (this->FieldMagnitudeVectorArrayName != this->VectorArrayName)
    {
      this->FieldMagnitude->RemoveAllVariables();
      this->FieldMagnitude->AddVectorArrayName(this->VectorArrayName.c_str());
      this->FieldMagnitude->SetFunction(("mag(" + this->VectorArrayName + ")").c_str());
      this->FieldMagnitudeVectorArrayName = this->VectorArrayName;
    }
    this->FieldMagnitude->SetInputConnection(this->FieldProducer->GetOutputPort());
    magnitudeConnection = this->FieldMagnitude->GetOutputPort();
  }
  const char* magnitudeArrayName = (this->MagnitudeArrayName.empty() ? vtkVectorFieldSampler::GetMagnitudeArrayName() : this->MagnitudeArrayName.c_str());
  this->Contour->SetInputConnection(magnitudeConnection);
  this->Contour->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, magnitudeArrayName);
  this->Contour->SetNumberOfContours(static_cast<int>(this->ContourLevels.size()));
  for (size_t levelIndex = 0; levelIndex < this->ContourLevels.size(); ++levelIndex)
  {
    this->Contour->SetValue(static_cast<int>(levelIndex), this->ContourLevels[levelIndex]);
  }
  return this->ClipToRegion(this->Contour->GetOutputPort());
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkVectorFieldGeometry::ClipToRegion(vtkAlgorithmOutput* connection)
{
  if (this->RegionArrayName.empty())
  {
    return connection;
  }
  this->RegionScalars->Assign(this->RegionArrayName.c_str(), vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
  this->RegionScalars->SetInputConnection(connection);
  return this->RegionClip->GetOutputPort();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkVectorFieldGeometry::ConfigureStreamlines(vtkAlgorithmOutput*& streamlinesConnection)
{
  if (this->StreamlineSeeds->GetNumberOfPoints() == 0)
  {
    return nullptr;
  }
  // The domain and the threshold limit integration, so streamlines stop where they leave them instead of being cut
  // afterwards (which would show parts that come back in). Seeds outside start nothing.
  // They are traced through every cell that has a point in the threshold range, so that the lines reach the position
  // where the value leaves the range; they are cut there below.
  vtkDataArray* thresholdArray = nullptr;
  if (this->ThresholdEnabled && this->ThresholdRange[0] <= this->ThresholdRange[1])
  {
    thresholdArray = this->FieldCopy->GetPointData()->GetArray(this->ThresholdArrayName.c_str());
  }
  vtkAlgorithmOutput* fieldConnection = this->FieldProducer->GetOutputPort();
  // A non-axis-aligned image is traced in its own axis-aligned space, where the seeds are moved to; the streamlines
  // are moved back below
  vtkNew<vtkMatrix4x4> tracedToField;
  vtkImageData* image = vtkImageData::SafeDownCast(this->FieldCopy);
  bool nonAxisAlignedImage = (image && !vtkVectorFieldStreamlineImage::IsAxisAligned(image));
  if (nonAxisAlignedImage)
  {
    vtkVectorFieldStreamlineImage::GetAxisAlignedToImageMatrix(image, tracedToField);
  }
  // Setting a matrix modifies the transform, which would trace the streamlines again
  if (!vtkAddonMathUtilities::MatrixAreEqual(this->StreamlineTracedToField->GetMatrix(), tracedToField, 0.0))
  {
    this->StreamlineTracedToField->SetMatrix(tracedToField);
  }
  if (image)
  {
    // An image is traced as an image, with the cells that are left out hidden in it
    if (nonAxisAlignedImage || !this->StreamlineDomainArrayName.empty() || thresholdArray)
    {
      this->StreamlineImage->SetVectorArrayName(this->VectorArrayName);
      this->StreamlineImage->SetDomainArrayName(this->StreamlineDomainArrayName);
      this->StreamlineImage->SetThresholdArrayName(thresholdArray ? this->ThresholdArrayName : "");
      this->StreamlineImage->SetThresholdComponent(this->ThresholdComponent);
      this->StreamlineImage->SetThresholdRange(this->ThresholdRange);
      this->StreamlineImage->SetInputConnection(fieldConnection);
      fieldConnection = this->StreamlineImage->GetOutputPort();
    }
  }
  else
  {
    // The cells of a mesh are extracted
    if (!this->StreamlineDomainArrayName.empty())
    {
      this->StreamlineDomain->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, this->StreamlineDomainArrayName.c_str());
      this->StreamlineDomain->SetInputConnection(fieldConnection);
      fieldConnection = this->StreamlineDomain->GetOutputPort();
    }
    if (thresholdArray)
    {
      int numberOfComponents = thresholdArray->GetNumberOfComponents();
      bool componentUsed = (this->ThresholdComponent >= 0 && this->ThresholdComponent < numberOfComponents);
      this->StreamlineThresholdDomain->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, this->ThresholdArrayName.c_str());
      // vtkThreshold uses the magnitude if the number of components is selected
      this->StreamlineThresholdDomain->SetSelectedComponent(componentUsed ? this->ThresholdComponent : numberOfComponents);
      this->StreamlineThresholdDomain->SetLowerThreshold(this->ThresholdRange[0]);
      this->StreamlineThresholdDomain->SetUpperThreshold(this->ThresholdRange[1]);
      this->StreamlineThresholdDomain->SetInputConnection(fieldConnection);
      fieldConnection = this->StreamlineThresholdDomain->GetOutputPort();
    }
  }
  this->StreamTracer->SetInputConnection(fieldConnection);
  this->StreamTracer->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, this->VectorArrayName.c_str());
  if (this->StreamlineBidirectional)
  {
    this->StreamTracer->SetIntegrationDirectionToBoth();
  }
  else
  {
    this->StreamTracer->SetIntegrationDirectionToForward();
  }
  this->StreamTracer->SetMaximumPropagation(this->StreamlineMaximumPropagation);
  if (this->StreamlineInitialIntegrationStep > 0.0)
  {
    this->StreamTracer->SetIntegrationStepUnit(vtkStreamTracer::LENGTH_UNIT);
    this->StreamTracer->SetInitialIntegrationStep(this->StreamlineInitialIntegrationStep);
  }

  streamlinesConnection = this->StreamTracer->GetOutputPort();
  if (nonAxisAlignedImage)
  {
    this->StreamlinesToField->SetInputConnection(streamlinesConnection);
    streamlinesConnection = this->StreamlinesToField->GetOutputPort();
  }
  // Cut where the threshold value leaves the range, in the outermost cells that the streamlines are traced through
  if (thresholdArray)
  {
    // Reconfigured only when the array changes: resetting the variables modifies the calculator
    if (this->StreamlineThresholdArrayName != this->ThresholdArrayName || this->StreamlineThresholdComponent != this->ThresholdComponent)
    {
      this->StreamlineThresholdValue->RemoveAllVariables();
      if (this->ThresholdComponent < 0)
      {
        this->StreamlineThresholdValue->AddVectorVariable("value", this->ThresholdArrayName.c_str());
        this->StreamlineThresholdValue->SetFunction("mag(value)");
      }
      else
      {
        this->StreamlineThresholdValue->AddScalarVariable("value", this->ThresholdArrayName.c_str(), this->ThresholdComponent);
        this->StreamlineThresholdValue->SetFunction("value");
      }
      this->StreamlineThresholdArrayName = this->ThresholdArrayName;
      this->StreamlineThresholdComponent = this->ThresholdComponent;
    }
    this->StreamlineThresholdValue->SetInputConnection(streamlinesConnection);
    this->StreamlineClipBelow->SetValue(this->ThresholdRange[0]);
    this->StreamlineClipAbove->SetValue(this->ThresholdRange[1]);
    streamlinesConnection = this->StreamlineClipAbove->GetOutputPort();
  }
  if (this->Flat || this->StreamlineTubeDiameter <= 0.0)
  {
    return streamlinesConnection;
  }
  this->StreamlineTuber->SetInputConnection(streamlinesConnection);
  this->StreamlineTuber->SetRadius(0.5 * this->StreamlineTubeDiameter);
  return this->StreamlineTuber->GetOutputPort();
}
