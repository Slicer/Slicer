/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SegmentationCore includes
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"
#include "vtkSegmentation.h"

#include "vtkOrientedImageData.h"

// VTK includes
#include <vtkCompositeDataIterator.h>
#include <vtkDecimatePro.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkExtractSelectedThresholds.h>
#include <vtkGeometryFilter.h>
#include <vtkImageAccumulate.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageConstantPad.h>
#include <vtkImageThreshold.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkMultiThreshold.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkThreshold.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkMatrix3x3.h>
#include <vtkReverseSense.h>
#include <vtkStringToNumeric.h>
#include <vtkStringArray.h>
#include <vtkSurfaceNets3D.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkFloatArray.h>
#include <vtkExtractSelectedIds.h>
#include <vtkInformation.h>
#include <vtkExtractSelection.h>
#include <vtkSelectionSource.h>

//----------------------------------------------------------------------------
const std::string vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_FLYING_EDGES = std::string("0");
const std::string vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_SURFACE_NETS = std::string("1");

//----------------------------------------------------------------------------
vtkSegmentationConverterRuleNewMacro(vtkBinaryLabelmapToClosedSurfaceConversionRule);

//----------------------------------------------------------------------------
vtkBinaryLabelmapToClosedSurfaceConversionRule::vtkBinaryLabelmapToClosedSurfaceConversionRule()
{
  this->ConversionParameters->SetParameter(
    GetDecimationFactorParameterName(),
    "0.0",
    "Desired reduction in the total number of polygons. Range: 0.0 (no decimation) to 1.0 (as much simplification as "
    "possible)."
    " Value of 0.8 typically reduces data set size by 80% without losing too much details.");
  this->ConversionParameters->SetParameter(
    GetSmoothingFactorParameterName(), "0.5", "Smoothing factor. Range: 0.0 (no smoothing) to 1.0 (strong smoothing).");
  this->ConversionParameters->SetParameter(GetComputeSurfaceNormalsParameterName(),
                                           "1",
                                           "Compute surface normals. 1 (default) = surface normals are computed. "
                                           "0 = surface normals are not computed (slightly faster but produces less "
                                           "smooth surface display, not used if vtkSurfaceNets3D is used).");
  this->ConversionParameters->SetParameter(
    GetConversionMethodParameterName(),
    CONVERSION_METHOD_FLYING_EDGES,
    "Conversion method. 0 (default) = vtkDiscreteFlyingEdges3D is used to generate closed surface."
    "1 = vtkSurfaceNets3D (more performant than flying edges).");
  this->ConversionParameters->SetParameter(
    GetSurfaceNetInternalSmoothingParameterName(),
    "0",
    "SurfaceNets smoothing. 0 (default) = Smoothing done by vtkWindowedSincPolyDataFilter"
    "1 = Smoothing done in surface nets filter.");
  this->ConversionParameters->SetParameter(GetJointSmoothingParameterName(), "0", "Perform joint smoothing.");
}

//----------------------------------------------------------------------------
vtkBinaryLabelmapToClosedSurfaceConversionRule::~vtkBinaryLabelmapToClosedSurfaceConversionRule() = default;

//----------------------------------------------------------------------------
unsigned int vtkBinaryLabelmapToClosedSurfaceConversionRule::GetConversionCost(
  vtkDataObject* vtkNotUsed(sourceRepresentation) /*=nullptr*/,
  vtkDataObject* vtkNotUsed(targetRepresentation) /*=nullptr*/)
{
  // Rough input-independent guess (ms)
  return 500;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkBinaryLabelmapToClosedSurfaceConversionRule::ConstructRepresentationObjectByRepresentation(
  std::string representationName)
{
  if (!representationName.compare(this->GetSourceRepresentationName()))
  {
    return (vtkDataObject*)vtkOrientedImageData::New();
  }
  else if (!representationName.compare(this->GetTargetRepresentationName()))
  {
    return (vtkDataObject*)vtkPolyData::New();
  }
  else
  {
    return nullptr;
  }
}

//----------------------------------------------------------------------------
vtkDataObject* vtkBinaryLabelmapToClosedSurfaceConversionRule::ConstructRepresentationObjectByClass(
  std::string className)
{
  if (!className.compare("vtkOrientedImageData"))
  {
    return (vtkDataObject*)vtkOrientedImageData::New();
  }
  else if (!className.compare("vtkPolyData"))
  {
    return (vtkDataObject*)vtkPolyData::New();
  }
  else
  {
    return nullptr;
  }
}

//----------------------------------------------------------------------------
bool vtkBinaryLabelmapToClosedSurfaceConversionRule::Convert(vtkSegment* segment)
{
  this->CreateTargetRepresentation(segment);

  vtkDataObject* sourceRepresentation = segment->GetRepresentation(this->GetSourceRepresentationName());
  vtkDataObject* targetRepresentation = segment->GetRepresentation(this->GetTargetRepresentationName());

  vtkPolyData* closedSurfacePolyData = vtkPolyData::SafeDownCast(targetRepresentation);
  if (!closedSurfacePolyData)
  {
    vtkErrorMacro("Convert: Target representation is not poly data");
    return false;
  }

  vtkOrientedImageData* orientedBinaryLabelmap = vtkOrientedImageData::SafeDownCast(sourceRepresentation);
  // Check validity of source and target representation objects
  if (!orientedBinaryLabelmap)
  {
    vtkErrorMacro("Convert: Source representation is not oriented image data");
    return false;
  }

  double smoothingFactor = this->ConversionParameters->GetValueAsDouble(GetSmoothingFactorParameterName());
  int jointSmoothing = this->ConversionParameters->GetValueAsInt(GetJointSmoothingParameterName());

  if (jointSmoothing > 0 && smoothingFactor > 0)
  {
    if (this->JointSmoothCache.find(orientedBinaryLabelmap) == this->JointSmoothCache.end())
    {
      double* scalarRange = orientedBinaryLabelmap->GetScalarRange();
      int lowLabel = (int)(floor(scalarRange[0]));
      int highLabel = (int)(ceil(scalarRange[1]));

      vtkNew<vtkImageAccumulate> imageAccumulate;
      imageAccumulate->SetInputData(orientedBinaryLabelmap);
      imageAccumulate->IgnoreZeroOn();
      imageAccumulate->SetComponentOrigin(0, 0, 0);
      imageAccumulate->SetComponentSpacing(1, 1, 1);
      imageAccumulate->SetComponentExtent(lowLabel, highLabel, 0, 0, 0, 0);
      imageAccumulate->Update();

      std::vector<int> labelValues;
      for (int labelValue = lowLabel; labelValue <= highLabel; ++labelValue)
      {
        // Add a new threshold for every level in the labelmap
        double numberOfVoxels =
          imageAccumulate->GetOutput()->GetPointData()->GetScalars()->GetTuple1((int)labelValue - lowLabel);
        if (numberOfVoxels > 0.0)
        {
          labelValues.push_back(labelValue);
        }
      }

      vtkSmartPointer<vtkPolyData> jointSmoothedSurface = vtkSmartPointer<vtkPolyData>::New();
      this->CreateClosedSurface(orientedBinaryLabelmap, jointSmoothedSurface, labelValues);
      this->JointSmoothCache[orientedBinaryLabelmap] = jointSmoothedSurface;
    }

    vtkDataObject* sharedSurface = this->JointSmoothCache[orientedBinaryLabelmap];
    if (!sharedSurface)
    {
      vtkErrorMacro("Convert: Could not find cached surface");
      return false;
    }

    vtkNew<vtkSelectionSource> selection;
    selection->SetContentType(vtkSelectionNode::THRESHOLDS);
    selection->SetFieldType(vtkSelectionNode::POINT);
    selection->GetContainingCells();
    selection->AddThreshold(segment->GetLabelValue(), segment->GetLabelValue());

    vtkNew<vtkExtractSelection> threshold;
    threshold->SetInputData(sharedSurface);
    threshold->SetSelectionConnection(selection->GetOutputPort());

    vtkNew<vtkGeometryFilter> geometry;
    geometry->SetInputConnection(threshold->GetOutputPort());
    geometry->Update();

    vtkPolyData* thresholdedSurface = geometry->GetOutput();
    closedSurfacePolyData->ShallowCopy(thresholdedSurface);
  }
  else
  {
    std::vector<int> labelValue = { segment->GetLabelValue() };
    this->CreateClosedSurface(orientedBinaryLabelmap, closedSurfacePolyData, labelValue);
  }

  // Remove "ImageScalars" array because having a scalar in a model would get that
  // scalar array displayed automatically (instead of model node color) when the mesh is loaded.
  vtkPointData* pointData = closedSurfacePolyData->GetPointData();
  if (pointData != nullptr)
  {
    pointData->RemoveArray("ImageScalars");
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkBinaryLabelmapToClosedSurfaceConversionRule::CreateClosedSurface(vtkOrientedImageData* orientedBinaryLabelmap,
                                                                         vtkPolyData* closedSurfacePolyData,
                                                                         std::vector<int> labelValues)
{
  if (!closedSurfacePolyData)
  {
    vtkErrorMacro("Convert: Target representation is not poly data");
    return false;
  }

  // Check validity of source and target representation objects
  if (!orientedBinaryLabelmap)
  {
    vtkErrorMacro("Convert: Source representation is not oriented image data");
    return false;
  }

  vtkSmartPointer<vtkImageData> binaryLabelmap = orientedBinaryLabelmap;
  if (!binaryLabelmap)
  {
    vtkErrorMacro("Convert: Source representation is not data");
    return false;
  }

  // Pad labelmap if it has non-background border voxels
  int* binaryLabelmapExtent = binaryLabelmap->GetExtent();
  if (binaryLabelmapExtent[0] > binaryLabelmapExtent[1] || binaryLabelmapExtent[2] > binaryLabelmapExtent[3]
      || binaryLabelmapExtent[4] > binaryLabelmapExtent[5])
  {
    // empty labelmap
    vtkDebugMacro("Convert: No polygons can be created, input image extent is empty");
    closedSurfacePolyData->Initialize();
    return true;
  }

  /// If input labelmap has non-background border voxels, then those regions remain open in the output closed surface.
  /// This function adds a 1 voxel padding to the labelmap in these cases.
  bool paddingNecessary = this->IsLabelmapPaddingNecessary(binaryLabelmap);
  if (paddingNecessary)
  {
    vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
    padder->SetInputData(binaryLabelmap);
    int extent[6] = { 0, -1, 0, -1, 0, -1 };
    binaryLabelmap->GetExtent(extent);
    // Set the output extent to the new size
    padder->SetOutputWholeExtent(
      extent[0] - 1, extent[1] + 1, extent[2] - 1, extent[3] + 1, extent[4] - 1, extent[5] + 1);
    padder->Update();
    binaryLabelmap = padder->GetOutput();
  }

  // Clone labelmap and set identity geometry so that the whole transform can be done in IJK space and then
  // the whole transform can be applied on the poly data to transform it to the world coordinate system
  vtkSmartPointer<vtkImageData> binaryLabelmapWithIdentityGeometry = vtkSmartPointer<vtkImageData>::New();
  binaryLabelmapWithIdentityGeometry->ShallowCopy(binaryLabelmap);
  binaryLabelmapWithIdentityGeometry->SetOrigin(0, 0, 0);
  binaryLabelmapWithIdentityGeometry->SetSpacing(1.0, 1.0, 1.0);

  // Get conversion parameters
  double decimationFactor = this->ConversionParameters->GetValueAsDouble(GetDecimationFactorParameterName());
  double smoothingFactor = this->ConversionParameters->GetValueAsDouble(GetSmoothingFactorParameterName());
  int computeSurfaceNormals = this->ConversionParameters->GetValueAsInt(GetComputeSurfaceNormalsParameterName());

  // Conversion method
  std::string conversionMethod = this->ConversionParameters->GetValue(GetConversionMethodParameterName());

  // SurfaceNetInternalSmoothing
  // 0 = use vtkWindowedSincPolyDataFilter
  // 1 = use surface nets internal smoothing filter (vtkConstrainedSmoothingFilter)
  int surfaceNetsSmoothing = this->ConversionParameters->GetValueAsInt(GetSurfaceNetInternalSmoothingParameterName());

  vtkSmartPointer<vtkPolyData> processingResult = vtkSmartPointer<vtkPolyData>::New();

  if (conversionMethod == vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_FLYING_EDGES)
  {
    vtkNew<vtkDiscreteFlyingEdges3D> flyingEdges;
    flyingEdges->SetInputData(binaryLabelmapWithIdentityGeometry);
    flyingEdges->ComputeGradientsOff();
    flyingEdges->ComputeNormalsOff(); // While computing normals is faster using the flying edges filter,
    // it results in incorrect normals in meshes from shared labelmaps marchingCubes->ComputeScalarsOn();

    int valueIndex = 0;
    for (vtkIdType labelValue : labelValues)
    {
      flyingEdges->SetValue(valueIndex, labelValue);
      ++valueIndex;
    }
    try
    {
      flyingEdges->Update();
    }
    catch (...)
    {
      vtkErrorMacro("Convert: Error while running flying edges!");
      return false;
    }
    processingResult = flyingEdges->GetOutput();
  }
  else if (conversionMethod == vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_SURFACE_NETS)
  {
    vtkNew<vtkSurfaceNets3D> surfaceNets;
    surfaceNets->SetInputData(binaryLabelmapWithIdentityGeometry);

    // Disable internal smoothing, and use vtkWindowedSincPolyDataFilter for smoothing as needed
    surfaceNets->SmoothingOff();

    if (surfaceNetsSmoothing == 1)
    {
      surfaceNets->SmoothingOn();

      // This formula maps (input) -> (iteration count)
      // 0.0  ->  0   (almost no smoothing)
      // 0.2  ->  2   (little smoothing)
      // 0.5  ->  8   (average smoothing)
      // 0.7  ->  14  (strong smoothing)
      // 1.0  ->  24  (very strong smoothing)
      double fCount = 15.0 * smoothingFactor * smoothingFactor + 9.0 * smoothingFactor;
      int iterationCount = floor(fCount);
      surfaceNets->SetNumberOfIterations(iterationCount);
    }

    int valueIndex = 0;
    for (vtkIdType labelValue : labelValues)
    {
      surfaceNets->SetValue(valueIndex, labelValue);
      ++valueIndex;
    }

    try
    {
      surfaceNets->Update();
    }
    catch (...)
    {
      vtkErrorMacro("Convert: Error while running surface nets!");
      return false;
    }
    processingResult = surfaceNets->GetOutput();
  }
  else
  {
    vtkErrorMacro("Conversion Rule: Unknown surface generation method");
  }

  vtkSmartPointer<vtkPolyData> convertedSegment = vtkSmartPointer<vtkPolyData>::New();

  if (processingResult->GetNumberOfPolys() == 0)
  {
    vtkDebugMacro("Convert: No polygons can be created, probably all voxels are empty");
    convertedSegment = nullptr;
    closedSurfacePolyData->Initialize();
  }

  if (!convertedSegment)
  {
    return true;
  }

  // Decimate
  if (decimationFactor > 0.0)
  {
    vtkSmartPointer<vtkDecimatePro> decimator = vtkSmartPointer<vtkDecimatePro>::New();
    decimator->SetInputData(processingResult);
    decimator->SetFeatureAngle(60);
    decimator->SplittingOff();
    decimator->PreserveTopologyOn();
    decimator->SetMaximumError(1);
    decimator->SetTargetReduction(decimationFactor);
    decimator->Update();
    processingResult = decimator->GetOutput();
  }

  if (smoothingFactor > 0 && surfaceNetsSmoothing == 0)
  {
    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smoother->SetInputData(processingResult);
    smoother->SetNumberOfIterations(
      20); // based on VTK documentation ("Ten or twenty iterations is all the is usually necessary")
    // This formula maps:
    // 0.0  -> 1.0   (almost no smoothing)
    // 0.25 -> 0.1   (average smoothing)
    // 0.5  -> 0.01  (more smoothing)
    // 1.0  -> 0.001 (very strong smoothing)
    double passBand = pow(10.0, -4.0 * smoothingFactor);
    smoother->SetPassBand(passBand);
    smoother->BoundarySmoothingOff();
    smoother->FeatureEdgeSmoothingOff();
    smoother->NonManifoldSmoothingOn();
    smoother->NormalizeCoordinatesOn();
    smoother->Update();
    processingResult = smoother->GetOutput();
  }

  // Transform the result surface from labelmap IJK to world coordinate system
  vtkSmartPointer<vtkTransform> labelmapGeometryTransform = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkMatrix4x4> labelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  orientedBinaryLabelmap->GetImageToWorldMatrix(labelmapImageToWorldMatrix);
  labelmapGeometryTransform->SetMatrix(labelmapImageToWorldMatrix);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformPolyDataFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformPolyDataFilter->SetInputData(processingResult);
  transformPolyDataFilter->SetTransform(labelmapGeometryTransform);

  if (computeSurfaceNormals > 0
      && conversionMethod == vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_FLYING_EDGES)
  {
    vtkSmartPointer<vtkPolyDataNormals> polyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
    polyDataNormals->SetInputConnection(transformPolyDataFilter->GetOutputPort());
    polyDataNormals->ConsistencyOn(); // discrete marching cubes may generate inconsistent surface
    // We almost always perform smoothing, so splitting would not be able to preserve any sharp features
    // (and sharp edges would look like artifacts in the smooth surface).
    polyDataNormals->SplittingOff();
    polyDataNormals->Update();
    convertedSegment->ShallowCopy(polyDataNormals->GetOutput());
  }
  else
  {
    transformPolyDataFilter->Update();
    convertedSegment->ShallowCopy(transformPolyDataFilter->GetOutput());
  }

  closedSurfacePolyData->ShallowCopy(convertedSegment);
  return true;
}

//----------------------------------------------------------------------------
bool vtkBinaryLabelmapToClosedSurfaceConversionRule::PostConvert(vtkSegmentation* vtkNotUsed(segmentation))
{
  this->JointSmoothCache.clear();
  return true;
}

//----------------------------------------------------------------------------
template <class ImageScalarType>
void IsLabelmapPaddingNecessaryGeneric(vtkImageData* binaryLabelmap, bool& paddingNecessary)
{
  if (!binaryLabelmap)
  {
    paddingNecessary = false;
    return;
  }

  // Check if there are non-zero voxels in the labelmap
  int extent[6] = { 0, -1, 0, -1, 0, -1 };
  binaryLabelmap->GetExtent(extent);
  int dimensions[3] = { 0, 0, 0 };
  binaryLabelmap->GetDimensions(dimensions);

  ImageScalarType* imagePtr = (ImageScalarType*)binaryLabelmap->GetScalarPointerForExtent(extent);

  for (long k = 0; k < dimensions[2]; ++k)
  {
    long offset2 = k * dimensions[0] * dimensions[1];
    for (long j = 0; j < dimensions[1]; ++j)
    {
      long offset1 = j * dimensions[0] + offset2;
      for (long i = 0; i < dimensions[0]; ++i)
      {
        if (i != 0 && i != dimensions[0] - 1 && j != 0 && j != dimensions[1] - 1 && k != 0 && k != dimensions[2] - 1)
        {
          // Skip non-border voxels
          continue;
        }
        int voxelValue = 0;
        voxelValue = (*(imagePtr + i + offset1));

        if (voxelValue != 0)
        {
          paddingNecessary = true;
          return;
        }
      }
    }
  }

  paddingNecessary = false;
  return;
}

//----------------------------------------------------------------------------
bool vtkBinaryLabelmapToClosedSurfaceConversionRule::IsLabelmapPaddingNecessary(vtkImageData* binaryLabelmap)
{
  if (!binaryLabelmap)
  {
    return false;
  }

  bool paddingNecessary = false;

  switch (binaryLabelmap->GetScalarType())
  {
    vtkTemplateMacro(IsLabelmapPaddingNecessaryGeneric<VTK_TT>(binaryLabelmap, paddingNecessary));
    default:
      vtkErrorWithObjectMacro(binaryLabelmap, "IsLabelmapPaddingNecessary: Unknown image scalar type!");
      return false;
  }

  return paddingNecessary;
}
