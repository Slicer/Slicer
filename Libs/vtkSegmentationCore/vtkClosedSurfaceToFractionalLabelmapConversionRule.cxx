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
#include "vtkClosedSurfaceToFractionalLabelmapConversionRule.h"

// SegmentationCore includes
#include "vtkOrientedImageData.h"
#include "vtkPolyDataToFractionalLabelmapFilter.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkFieldData.h>

// SegmentationCore includes
#include "vtkSegment.h"

//----------------------------------------------------------------------------
vtkSegmentationConverterRuleNewMacro(vtkClosedSurfaceToFractionalLabelmapConversionRule);

//----------------------------------------------------------------------------
vtkClosedSurfaceToFractionalLabelmapConversionRule::vtkClosedSurfaceToFractionalLabelmapConversionRule()
{
  this->NumberOfOffsets = 6;
  this->UseOutputImageDataGeometry = true;
}

//----------------------------------------------------------------------------
vtkClosedSurfaceToFractionalLabelmapConversionRule::~vtkClosedSurfaceToFractionalLabelmapConversionRule() = default;

//----------------------------------------------------------------------------
unsigned int vtkClosedSurfaceToFractionalLabelmapConversionRule::GetConversionCost(
  vtkDataObject* vtkNotUsed(sourceRepresentation)/*=nullptr*/,
  vtkDataObject* vtkNotUsed(targetRepresentation)/*=nullptr*/)
{
  // Rough input-independent guess (ms)
  return 7000;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkClosedSurfaceToFractionalLabelmapConversionRule::ConstructRepresentationObjectByRepresentation(std::string representationName)
{
  if ( !representationName.compare(this->GetSourceRepresentationName()) )
    {
    return (vtkDataObject*)vtkPolyData::New();
    }
  else if ( !representationName.compare(this->GetTargetRepresentationName()) )
    {
    return (vtkDataObject*)vtkOrientedImageData::New();
    }
  else
    {
    return nullptr;
    }
}

//----------------------------------------------------------------------------
vtkDataObject* vtkClosedSurfaceToFractionalLabelmapConversionRule::ConstructRepresentationObjectByClass(std::string className)
{
  if (!className.compare("vtkPolyData"))
    {
    return (vtkDataObject*)vtkPolyData::New();
    }
  else if (!className.compare("vtkOrientedImageData"))
    {
    return (vtkDataObject*)vtkOrientedImageData::New();
    }
  else
    {
    return nullptr;
    }
}

//----------------------------------------------------------------------------
bool vtkClosedSurfaceToFractionalLabelmapConversionRule::Convert(vtkSegment* segment)
{
  this->CreateTargetRepresentation(segment);

  vtkDataObject* sourceRepresentation = segment->GetRepresentation(this->GetSourceRepresentationName());
  vtkDataObject* targetRepresentation = segment->GetRepresentation(this->GetTargetRepresentationName());

  // Check validity of source and target representation objects
  vtkPolyData* closedSurfacePolyData = vtkPolyData::SafeDownCast(sourceRepresentation);
  if (!closedSurfacePolyData)
    {
    vtkErrorMacro("Convert: Source representation is not a poly data!");
    return false;
    }
  vtkOrientedImageData* fractionalLabelMap = vtkOrientedImageData::SafeDownCast(targetRepresentation);
  if (!fractionalLabelMap)
    {
    vtkErrorMacro("Convert: Target representation is not an oriented image data!");
    return false;
    }
  if (closedSurfacePolyData->GetNumberOfPoints() < 2 || closedSurfacePolyData->GetNumberOfCells() < 2)
    {
    vtkErrorMacro("Convert: Cannot create binary labelmap from surface with number of points: " << closedSurfacePolyData->GetNumberOfPoints() << " and number of cells: " << closedSurfacePolyData->GetNumberOfCells());
    return false;
    }

  // Compute output labelmap geometry based on poly data, an reference image
  // geometry, and store the calculated geometry in output labelmap image data
  if (!this->CalculateOutputGeometry(closedSurfacePolyData, fractionalLabelMap))
    {
    vtkErrorMacro("Convert: Failed to calculate output image geometry!");
    return false;
    }

  // Pad the extent of the fractional labelmap
  int extent[6] = {0,-1,0,-1,0,-1};
  fractionalLabelMap->GetExtent(extent);
  for (int i=0; i<2; ++i)
    {
    --extent[2*i];
    ++extent[2*i+1];
    }
  fractionalLabelMap->SetExtent(extent);

  vtkSmartPointer<vtkMatrix4x4> imageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  fractionalLabelMap->GetImageToWorldMatrix(imageToWorldMatrix);

  // Create a fractional labelmap from the closed surface
  vtkSmartPointer<vtkPolyDataToFractionalLabelmapFilter> polyDataToLabelmapFilter = vtkSmartPointer<vtkPolyDataToFractionalLabelmapFilter>::New();
  polyDataToLabelmapFilter->SetInputData(closedSurfacePolyData);
  polyDataToLabelmapFilter->SetOutputImageToWorldMatrix(imageToWorldMatrix);
  polyDataToLabelmapFilter->SetNumberOfOffsets(this->NumberOfOffsets);
  polyDataToLabelmapFilter->SetOutputWholeExtent(fractionalLabelMap->GetExtent());
  polyDataToLabelmapFilter->Update();
  fractionalLabelMap->DeepCopy(polyDataToLabelmapFilter->GetOutput());

  // Specify the scalar range of values in the labelmap
  vtkSmartPointer<vtkDoubleArray> scalarRange = vtkSmartPointer<vtkDoubleArray>::New();
  scalarRange->SetName(vtkSegmentationConverter::GetScalarRangeFieldName());
  scalarRange->InsertNextValue(FRACTIONAL_MIN);
  scalarRange->InsertNextValue(FRACTIONAL_MAX);
  fractionalLabelMap->GetFieldData()->AddArray(scalarRange);

  // Specify the surface threshold value for visualization
  vtkSmartPointer<vtkDoubleArray> thresholdValue = vtkSmartPointer<vtkDoubleArray>::New();
  thresholdValue->SetName(vtkSegmentationConverter::GetThresholdValueFieldName());
  thresholdValue->InsertNextValue((FRACTIONAL_MIN+FRACTIONAL_MAX)/2.0);
  fractionalLabelMap->GetFieldData()->AddArray(thresholdValue);

  // Specify the interpolation type for visualization
  vtkSmartPointer<vtkIntArray> interpolationType = vtkSmartPointer<vtkIntArray>::New();
  interpolationType->SetName(vtkSegmentationConverter::GetInterpolationTypeFieldName());
  interpolationType->InsertNextValue(VTK_LINEAR_INTERPOLATION);
  fractionalLabelMap->GetFieldData()->AddArray(interpolationType);

  return true;
}
