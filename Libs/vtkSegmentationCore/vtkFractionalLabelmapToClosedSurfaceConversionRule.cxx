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

// SegmentationCore includes
#include <vtkOrientedImageData.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkVersion.h>
#include <vtkMarchingCubes.h>
#include <vtkDecimatePro.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkImageConstantPad.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageReslice.h>
#include <vtkImageResize.h>
#include <vtkFieldData.h>
#include <vtkDoubleArray.h>


#include "vtkFractionalLabelmapToClosedSurfaceConversionRule.h"

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
vtkFractionalLabelmapToClosedSurfaceConversionRule::~vtkFractionalLabelmapToClosedSurfaceConversionRule()
{
}

//----------------------------------------------------------------------------
unsigned int vtkFractionalLabelmapToClosedSurfaceConversionRule::GetConversionCost(
    vtkDataObject* vtkNotUsed(sourceRepresentation)/*=NULL*/,
    vtkDataObject* vtkNotUsed(targetRepresentation)/*=NULL*/)
{
  // Rough input-independent guess (ms)
  return 500;
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
    return NULL;
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
    return NULL;
    }
}

//----------------------------------------------------------------------------
bool vtkFractionalLabelmapToClosedSurfaceConversionRule::Convert(vtkDataObject* sourceRepresentation, vtkDataObject* targetRepresentation)
{
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
  vtkSmartPointer<vtkMarchingCubes> marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
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
  if (marchingCubes->GetOutput()->GetNumberOfPolys() == 0)
    {
    vtkErrorMacro("Convert: No polygons can be created!");
    return false;
    }

  // Decimate if necessary
  vtkSmartPointer<vtkDecimatePro> decimator = vtkSmartPointer<vtkDecimatePro>::New();
  decimator->SetInputConnection(marchingCubes->GetOutputPort());
  if (decimationFactor > 0.0)
    {
    decimator->SetFeatureAngle(60);
    decimator->SplittingOff();
    decimator->PreserveTopologyOn();
    decimator->SetMaximumError(1);
    decimator->SetTargetReduction(decimationFactor);
    try
      {
      decimator->Update();
      }
    catch(...)
      {
      vtkErrorMacro("Error decimating model");
      return false;
      }
    }

  // Perform smoothing using specified factor
  vtkSmartPointer<vtkSmoothPolyDataFilter> smoothFilter = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
  if (decimationFactor > 0.0)
    {
    smoothFilter->SetInputConnection(decimator->GetOutputPort());
    }
  else
    {
    smoothFilter->SetInputConnection(marchingCubes->GetOutputPort());
    }
  smoothFilter->SetRelaxationFactor(smoothingFactor);
  smoothFilter->Update();

  // Transform the result surface from labelmap IJK to world coordinate system
  vtkSmartPointer<vtkTransform> labelmapGeometryTransform = vtkSmartPointer<vtkTransform>::New();
  labelmapGeometryTransform->SetMatrix(labelmapImageToWorldMatrix);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformPolyDataFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformPolyDataFilter->SetInputConnection(smoothFilter->GetOutputPort());
  transformPolyDataFilter->SetTransform(labelmapGeometryTransform);
  transformPolyDataFilter->Update();

  // Set output
  closedSurfacePolyData->ShallowCopy(transformPolyDataFilter->GetOutput());

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
