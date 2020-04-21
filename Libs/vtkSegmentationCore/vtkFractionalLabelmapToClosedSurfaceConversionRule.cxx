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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// VTK includes
#include <vtkVersion.h> // must precede reference to VTK_MAJOR_VERSION
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  #include <vtkFlyingEdges3D.h>
#else
  #include <vtkMarchingCubes.h>
#endif
#include <vtkDecimatePro.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkImageConstantPad.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageReslice.h>
#include <vtkImageResize.h>
#include <vtkFieldData.h>
#include <vtkDoubleArray.h>
#include <vtkPolyDataNormals.h>

// SegmentationCore includes
#include "vtkFractionalLabelmapToClosedSurfaceConversionRule.h"
#include <vtkOrientedImageData.h>
#include <vtkSegment.h>

//----------------------------------------------------------------------------
vtkSegmentationConverterRuleNewMacro(vtkFractionalLabelmapToClosedSurfaceConversionRule);

//----------------------------------------------------------------------------
vtkFractionalLabelmapToClosedSurfaceConversionRule::vtkFractionalLabelmapToClosedSurfaceConversionRule()
  : vtkBinaryLabelmapToClosedSurfaceConversionRule()
{
  this->ConversionParameters[this->GetFractionalLabelMapOversamplingFactorParameterName()] = std::make_pair("1", "Determines the oversampling of the reference image geometry. All segments are oversampled with the same value (value of 1 means no oversampling).");
  this->ConversionParameters[this->GetThresholdFractionParameterName()] = std::make_pair("0.5", "Determines the threshold that the closed surface is created at as a fractional value between 0 and 1.");
}

//----------------------------------------------------------------------------
vtkFractionalLabelmapToClosedSurfaceConversionRule::~vtkFractionalLabelmapToClosedSurfaceConversionRule() = default;

//----------------------------------------------------------------------------
unsigned int vtkFractionalLabelmapToClosedSurfaceConversionRule::GetConversionCost(
    vtkDataObject* vtkNotUsed(sourceRepresentation)/*=nullptr*/,
    vtkDataObject* vtkNotUsed(targetRepresentation)/*=nullptr*/)
{
  // Rough input-independent guess (ms)
  return 600;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkFractionalLabelmapToClosedSurfaceConversionRule::ConstructRepresentationObjectByRepresentation(std::string representationName)
{
  if ( !representationName.compare(this->GetSourceRepresentationName()) )
    {
    return (vtkDataObject*)vtkOrientedImageData::New();
    }
  else if ( !representationName.compare(this->GetTargetRepresentationName()) )
    {
    return (vtkDataObject*)vtkPolyData::New();
    }
  else
    {
    return nullptr;
    }
}

//----------------------------------------------------------------------------
vtkDataObject* vtkFractionalLabelmapToClosedSurfaceConversionRule::ConstructRepresentationObjectByClass(std::string className)
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
bool vtkFractionalLabelmapToClosedSurfaceConversionRule::Convert(vtkSegment* segment)
{
  this->CreateTargetRepresentation(segment);

  vtkDataObject* sourceRepresentation = segment->GetRepresentation(this->GetSourceRepresentationName());
  vtkDataObject* targetRepresentation = segment->GetRepresentation(this->GetTargetRepresentationName());

  // Check validity of source and target representation objects
  vtkOrientedImageData* fractionalLabelMap = vtkOrientedImageData::SafeDownCast(sourceRepresentation);
  if (!fractionalLabelMap)
    {
    vtkErrorMacro("Convert: Source representation is not an oriented image data!");
    return false;
    }
  vtkPolyData* closedSurfacePolyData = vtkPolyData::SafeDownCast(targetRepresentation);
  if (!closedSurfacePolyData)
    {
    vtkErrorMacro("Convert: Target representation is not a poly data!");
    return false;
    }

  // Get the range of the scalars in the image data from the ScalarRange field if it exists
  // Default to the scalar range of 0.0 to 1.0 otherwise
  double minimumValue = 0.0;
  double maximumValue = 1.0;
  fractionalLabelMap->GetFieldData();
  vtkDoubleArray* scalarRange = vtkDoubleArray::SafeDownCast(
    fractionalLabelMap->GetFieldData()->GetAbstractArray( vtkSegmentationConverter::GetScalarRangeFieldName() )
    );
  if (scalarRange && scalarRange->GetNumberOfValues() == 2)
    {
    minimumValue = scalarRange->GetValue(0);
    maximumValue = scalarRange->GetValue(1);
    }

  // Pad labelmap if it has non-background border voxels
  bool paddingNecessary = this->IsLabelmapPaddingNecessary(fractionalLabelMap);
  if (paddingNecessary)
    {
    vtkOrientedImageData* paddedLabelmap = vtkOrientedImageData::New();
    paddedLabelmap->DeepCopy(fractionalLabelMap);
    this->PadLabelmap(paddedLabelmap, minimumValue);
    fractionalLabelMap = paddedLabelmap;
    }

  // Get conversion parameters
  double decimationFactor = vtkVariant(this->ConversionParameters[this->GetDecimationFactorParameterName()].first).ToDouble();
  double smoothingFactor = vtkVariant(this->ConversionParameters[this->GetSmoothingFactorParameterName()].first).ToDouble();
  int computeSurfaceNormals = vtkVariant(this->ConversionParameters[GetComputeSurfaceNormalsParameterName()].first).ToInt();
  double fractionalOversamplingFactor = vtkVariant(this->ConversionParameters[this->GetFractionalLabelMapOversamplingFactorParameterName()].first).ToDouble();
  double fractionalThreshold = vtkVariant(this->ConversionParameters[this->GetThresholdFractionParameterName()].first).ToDouble();

  if (fractionalThreshold < 0 || fractionalThreshold > 1)
    {
    vtkErrorMacro("Convert: Fractional threshold must be between 0.0 and 1.0!");
    return false;
    }

  // Save geometry of oriented image data before conversion so that it can be applied on the poly data afterwards
  vtkSmartPointer<vtkMatrix4x4> labelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  fractionalLabelMap->GetImageToWorldMatrix(labelmapImageToWorldMatrix);

  // Clone labelmap and set identity geometry so that the whole transform can be done in IJK space and then
  // the whole transform can be applied on the poly data to transform it to the world coordinate system
  vtkSmartPointer<vtkOrientedImageData> fractionalLabelmapWithIdentityGeometry = vtkSmartPointer<vtkOrientedImageData>::New();
  fractionalLabelmapWithIdentityGeometry->ShallowCopy(fractionalLabelMap);
  vtkSmartPointer<vtkMatrix4x4> identityMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  identityMatrix->Identity();
  fractionalLabelmapWithIdentityGeometry->SetGeometryFromImageToWorldMatrix(identityMatrix);

  // Resize the image with interpolation, this helps the conversion for structures with small labelmaps
  vtkSmartPointer<vtkImageResize> imageResize = vtkSmartPointer<vtkImageResize>::New();
  imageResize->SetInputData(fractionalLabelmapWithIdentityGeometry);
  imageResize->BorderOn();
  imageResize->SetResizeMethodToMagnificationFactors();
  imageResize->SetMagnificationFactors(fractionalOversamplingFactor, fractionalOversamplingFactor, fractionalOversamplingFactor);
  imageResize->InterpolateOn();

  // Run marching cubes
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  vtkSmartPointer<vtkFlyingEdges3D> marchingCubes = vtkSmartPointer<vtkFlyingEdges3D>::New();
#else
  vtkSmartPointer<vtkMarchingCubes> marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
#endif
  marchingCubes->SetInputConnection(imageResize->GetOutputPort());
  marchingCubes->SetNumberOfContours(1);
  marchingCubes->SetValue(0, (fractionalThreshold * (maximumValue - minimumValue)) + minimumValue);
  marchingCubes->ComputeScalarsOff();
  marchingCubes->ComputeGradientsOff();
  marchingCubes->ComputeNormalsOff();
  try
    {
    marchingCubes->Update();
    }
  catch(...)
    {
    vtkErrorMacro("Convert: Error while running marching cubes!");
    return false;
    }

    vtkSmartPointer<vtkPolyData> convertedSegment = vtkSmartPointer<vtkPolyData>::New();

  // Run marching cubes
  vtkSmartPointer<vtkPolyData> processingResult = marchingCubes->GetOutput();
  if (processingResult->GetNumberOfPolys() == 0)
    {
    vtkDebugMacro("Convert: No polygons can be created, probably all voxels are empty");
    convertedSegment = nullptr;
    closedSurfacePolyData->Reset();
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

  if (smoothingFactor > 0)
    {
    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smoother->SetInputData(processingResult);
    smoother->SetNumberOfIterations(20); // based on VTK documentation ("Ten or twenty iterations is all the is usually necessary")
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
  labelmapGeometryTransform->SetMatrix(labelmapImageToWorldMatrix);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformPolyDataFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformPolyDataFilter->SetInputData(processingResult);
  transformPolyDataFilter->SetTransform(labelmapGeometryTransform);
  transformPolyDataFilter->Update();

  if (computeSurfaceNormals > 0)
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

  // Set output
  closedSurfacePolyData->ShallowCopy(convertedSegment);

  // Delete temporary padded labelmap if it was created
  if (paddingNecessary)
    {
    fractionalLabelMap->Delete();
    }

  return true;
}

//----------------------------------------------------------------------------
void vtkFractionalLabelmapToClosedSurfaceConversionRule::PadLabelmap(vtkOrientedImageData* fractionalLabelMap, double paddingConstant)
{
  vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
  padder->SetInputData(fractionalLabelMap);
  padder->SetConstant(paddingConstant);
  int extent[6] = {0,-1,0,-1,0,-1};
  fractionalLabelMap->GetExtent(extent);
  // Set the output extent to the new size
  padder->SetOutputWholeExtent(extent[0]-1, extent[1]+1, extent[2]-1, extent[3]+1, extent[4]-1, extent[5]+1);
  padder->Update();
  fractionalLabelMap->vtkImageData::DeepCopy(padder->GetOutput());
}
