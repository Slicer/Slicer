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
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"

#include "vtkOrientedImageData.h"
#include "vtkCalculateOversamplingFactor.h"

// Slicer includes
#include "vtkLoggingMacros.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkImageCast.h>
#include <vtkImageStencil.h>
#include <vtkPolyDataNormals.h>
#include <vtkStripper.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyDataToImageStencil.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkSegmentationConverterRuleNewMacro(vtkClosedSurfaceToBinaryLabelmapConversionRule);

//----------------------------------------------------------------------------
vtkClosedSurfaceToBinaryLabelmapConversionRule::vtkClosedSurfaceToBinaryLabelmapConversionRule()
  : UseOutputImageDataGeometry(false)
{
  // Reference image geometry parameter
  this->ConversionParameters[vtkSegmentationConverter::GetReferenceImageGeometryParameterName()] = std::make_pair("", "Image geometry description string determining the geometry of the labelmap that is created in course of conversion. Can be copied from a volume, using the button.");
  // Oversampling factor parameter
  this->ConversionParameters[GetOversamplingFactorParameterName()] = std::make_pair("1", "Determines the oversampling of the reference image geometry. If it's a number, then all segments are oversampled with the same value (value of 1 means no oversampling). If it has the value \"A\", then automatic oversampling is calculated.");
}

//----------------------------------------------------------------------------
vtkClosedSurfaceToBinaryLabelmapConversionRule::~vtkClosedSurfaceToBinaryLabelmapConversionRule()
{
}

//----------------------------------------------------------------------------
unsigned int vtkClosedSurfaceToBinaryLabelmapConversionRule::GetConversionCost(
  vtkDataObject* vtkNotUsed(sourceRepresentation)/*=NULL*/,
  vtkDataObject* vtkNotUsed(targetRepresentation)/*=NULL*/)
{
  // Rough input-independent guess (ms)
  return 500;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkClosedSurfaceToBinaryLabelmapConversionRule::ConstructRepresentationObjectByRepresentation(std::string representationName)
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
    return NULL;
    }
}

//----------------------------------------------------------------------------
vtkDataObject* vtkClosedSurfaceToBinaryLabelmapConversionRule::ConstructRepresentationObjectByClass(std::string className)
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
    return NULL;
    }
}

//----------------------------------------------------------------------------
bool vtkClosedSurfaceToBinaryLabelmapConversionRule::Convert(vtkDataObject* sourceRepresentation, vtkDataObject* targetRepresentation)
{
  // Check validity of source and target representation objects
  vtkPolyData* closedSurfacePolyData = vtkPolyData::SafeDownCast(sourceRepresentation);
  if (!closedSurfacePolyData)
    {
    vtkErrorMacro("Convert: Source representation is not a poly data!");
    return false;
    }
  vtkOrientedImageData* binaryLabelMap = vtkOrientedImageData::SafeDownCast(targetRepresentation);
  if (!binaryLabelMap)
    {
    vtkErrorMacro("Convert: Target representation is not an oriented image data!");
    return false;
    }
  if (closedSurfacePolyData->GetNumberOfPoints() < 2 || closedSurfacePolyData->GetNumberOfCells() < 2)
    {
    vtkDebugMacro("Convert: Cannot create binary labelmap from surface with number of points: "
      << closedSurfacePolyData->GetNumberOfPoints() << " and number of cells: " << closedSurfacePolyData->GetNumberOfCells());
    return false;
    }

  // Setup output labelmap

  // Compute output labelmap geometry based on poly data, an reference image
  // geometry, and store the calculated geometry in output labelmap image data
  if (!this->UseOutputImageDataGeometry)
    {
    if (!this->CalculateOutputGeometry(closedSurfacePolyData, binaryLabelMap))
      {
      vtkErrorMacro("Convert: Failed to calculate output image geometry!");
      return false;
      }
    }

  // Allocate output image data
  binaryLabelMap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  void* binaryLabelMapVoxelsPointer = binaryLabelMap->GetScalarPointerForExtent(binaryLabelMap->GetExtent());
  if (!binaryLabelMapVoxelsPointer)
    {
    vtkErrorMacro("Convert: Failed to allocate memory for output labelmap image!");
    return false;
    }
  else
    {
    // Set voxel values to 0
    int extent[6] = {0,-1,0,-1,0,-1};
    binaryLabelMap->GetExtent(extent);
    memset(binaryLabelMapVoxelsPointer, 0, ((extent[1]-extent[0]+1)*(extent[3]-extent[2]+1)*(extent[5]-extent[4]+1) * binaryLabelMap->GetScalarSize() * binaryLabelMap->GetNumberOfScalarComponents()));
    }

  // Perform conversion

  // Now the output labelmap image data contains the right geometry.
  // We need to apply inverse of geometry matrix to the input poly data so that we can perform
  // the conversion in IJK space, because the filters do not support oriented image data.
  vtkSmartPointer<vtkMatrix4x4> outputLabelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  binaryLabelMap->GetImageToWorldMatrix(outputLabelmapImageToWorldMatrix);
  vtkSmartPointer<vtkTransform> inverseOutputLabelmapGeometryTransform = vtkSmartPointer<vtkTransform>::New();
  inverseOutputLabelmapGeometryTransform->SetMatrix(outputLabelmapImageToWorldMatrix);
  inverseOutputLabelmapGeometryTransform->Inverse();

  // Set geometry to identity for the volume so that we can perform the stencil operation in IJK space
  vtkSmartPointer<vtkMatrix4x4> identityMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  identityMatrix->Identity();
  binaryLabelMap->SetGeometryFromImageToWorldMatrix(identityMatrix);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformPolyDataFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformPolyDataFilter->SetInputData(closedSurfacePolyData);
  transformPolyDataFilter->SetTransform(inverseOutputLabelmapGeometryTransform);

  // Compute polydata normals
  vtkNew<vtkPolyDataNormals> normalFilter;
  normalFilter->SetInputConnection(transformPolyDataFilter->GetOutputPort());
  normalFilter->ConsistencyOn();

  // Make sure that we have a clean triangle polydata
  vtkNew<vtkTriangleFilter> triangle;
  triangle->SetInputConnection(normalFilter->GetOutputPort());

  // Convert to triangle strip
  vtkSmartPointer<vtkStripper> stripper=vtkSmartPointer<vtkStripper>::New();
  stripper->SetInputConnection(triangle->GetOutputPort());

  // Convert polydata to stencil
  vtkNew<vtkPolyDataToImageStencil> polyDataToImageStencil;
  polyDataToImageStencil->SetInputConnection(stripper->GetOutputPort());
  polyDataToImageStencil->SetOutputSpacing(binaryLabelMap->GetSpacing());
  polyDataToImageStencil->SetOutputOrigin(binaryLabelMap->GetOrigin());
  polyDataToImageStencil->SetOutputWholeExtent(binaryLabelMap->GetExtent());

  // Convert stencil to image
  vtkNew<vtkImageStencil> stencil;
  stencil->SetInputData(binaryLabelMap);
  stencil->SetStencilConnection(polyDataToImageStencil->GetOutputPort());
  stencil->ReverseStencilOn();
  stencil->SetBackgroundValue(1); // General foreground value is 1 (background value because of reverse stencil)

  // Save result to output
  vtkNew<vtkImageCast> imageCast;
  imageCast->SetInputConnection(stencil->GetOutputPort());
  imageCast->SetOutputScalarTypeToUnsignedChar();
  imageCast->Update();
  binaryLabelMap->ShallowCopy(imageCast->GetOutput());

  // Restore geometry of the labelmap that we set to identity before conversion
  // (so that we can perform the stencil operations in IJK space)
  binaryLabelMap->SetGeometryFromImageToWorldMatrix(outputLabelmapImageToWorldMatrix);

  return true;
}

//----------------------------------------------------------------------------
bool vtkClosedSurfaceToBinaryLabelmapConversionRule::CalculateOutputGeometry(vtkPolyData* closedSurfacePolyData, vtkOrientedImageData* geometryImageData)
{
  if (!closedSurfacePolyData)
    {
    vtkErrorMacro("CalculateOutputGeometry: Invalid input closed surface poly data!");
    return false;
    }
  if (!geometryImageData)
    {
    vtkErrorMacro("CalculateOutputGeometry: Invalid output geometry image Data!");
    return false;
    }

  // Get reference image geometry from parameters
  std::string geometryString = this->ConversionParameters[vtkSegmentationConverter::GetReferenceImageGeometryParameterName()].first;
  if (geometryString.empty() || !vtkSegmentationConverter::DeserializeImageGeometry(geometryString, geometryImageData))
    {
    vtkInfoMacro("CalculateOutputGeometry: No image geometry specified, default geometry is calculated with 1 mm spacing");
    geometryString = this->GetDefaultImageGeometryStringForPolyData(closedSurfacePolyData);

    // If still not valid then return with error
    if (!vtkSegmentationConverter::DeserializeImageGeometry(geometryString, geometryImageData))
      {
      vtkErrorMacro("CalculateOutputGeometry: Failed to get reference image geometry");
      return false;
      }
    }

  // Return reference geometry if polydata is empty
  if (!closedSurfacePolyData->GetPoints())
    {
    return true;
    }

  // Get oversampling factor
  std::string oversamplingString = this->ConversionParameters[GetOversamplingFactorParameterName()].first;
  double oversamplingFactor = 1.0;
  if (!oversamplingString.compare("A"))
    {
    // Automatic oversampling factor is used
    vtkSmartPointer<vtkCalculateOversamplingFactor> oversamplingCalculator = vtkSmartPointer<vtkCalculateOversamplingFactor>::New();
    oversamplingCalculator->SetInputPolyData(closedSurfacePolyData);
    oversamplingCalculator->SetReferenceGeometryImageData(geometryImageData);
    if (oversamplingCalculator->CalculateOversamplingFactor())
      {
      oversamplingFactor = oversamplingCalculator->GetOutputOversamplingFactor();
      }
    else
      {
      vtkWarningMacro("CalculateOutputGeometry: Failed to automatically calculate oversampling factor! Using default value of 1");
      oversamplingFactor = 1.0;
      }
    }
  else
    {
    // Static oversampling factor
    std::stringstream ss;
    ss << oversamplingString;
    ss >> oversamplingFactor;
    if (ss.fail())
      {
      oversamplingFactor = 1.0;
      }
    }

  // Apply oversampling if needed
  vtkCalculateOversamplingFactor::ApplyOversamplingOnImageGeometry(geometryImageData, oversamplingFactor);

  // We need to apply inverse of direction matrix to the input poly data
  // so that we can expand the image in its IJK directions
  vtkSmartPointer<vtkMatrix4x4> geometryImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  geometryImageData->GetImageToWorldMatrix(geometryImageToWorldMatrix);
  vtkSmartPointer<vtkTransform> inverseImageGeometryTransform = vtkSmartPointer<vtkTransform>::New();
  inverseImageGeometryTransform->SetMatrix(geometryImageToWorldMatrix);
  inverseImageGeometryTransform->Inverse();

  vtkSmartPointer<vtkTransformPolyDataFilter> transformPolyDataFilter =
  vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformPolyDataFilter->SetInputData(closedSurfacePolyData);
  transformPolyDataFilter->SetTransform(inverseImageGeometryTransform);
  transformPolyDataFilter->Update();
  vtkPolyData* transformedClosedSurfacePolyData = transformPolyDataFilter->GetOutput();

  // Compute input closed surface poly data bounds
  double surfaceBounds[6] = {0,0,0,0,0,0};
  transformedClosedSurfacePolyData->GetPoints()->GetBounds(surfaceBounds);

  // Expand floating point bounds to extent integers
  int surfaceExtent[6] = {0,-1,0,-1,0,-1};
  surfaceExtent[0] = (int)floor(surfaceBounds[0]);
  surfaceExtent[1] = (int)ceil(surfaceBounds[1]);
  surfaceExtent[2] = (int)floor(surfaceBounds[2]);
  surfaceExtent[3] = (int)ceil(surfaceBounds[3]);
  surfaceExtent[4] = (int)floor(surfaceBounds[4]);
  surfaceExtent[5] = (int)ceil(surfaceBounds[5]);

  // Set effective extent back to the image (less memory needed if the extent only covers the non-zero region)
  geometryImageData->SetExtent(surfaceExtent);
  geometryImageData->AllocateScalars(geometryImageData->GetScalarType(), geometryImageData->GetNumberOfScalarComponents());

  return true;
}

//----------------------------------------------------------------------------
std::string vtkClosedSurfaceToBinaryLabelmapConversionRule::GetDefaultImageGeometryStringForPolyData(vtkPolyData* polyData)
{
  if (!polyData)
    {
    vtkErrorMacro("GetDefaultImageGeometryStringForPolyData: Invalid input poly data!");
    return "";
    }

  // Get poly data bounds
  double bounds[6] = {0,0,0,0,0,0};
  polyData->GetBounds(bounds);

  // Set origin
  vtkSmartPointer<vtkMatrix4x4> geometryMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  geometryMatrix->Identity(); // Default directions and scaling
  geometryMatrix->SetElement(0,3,bounds[0]);
  geometryMatrix->SetElement(1,3,bounds[2]);
  geometryMatrix->SetElement(2,3,bounds[4]);

  // Set extent
  int extent[6] = { 0, (int)(bounds[1]-bounds[0]+1),
                    0, (int)(bounds[3]-bounds[2]+1),
                    0, (int)(bounds[5]-bounds[4]+1) };

  // Serialize geometry
  std::string serializedGeometry = vtkSegmentationConverter::SerializeImageGeometry(geometryMatrix, extent);
  this->ConversionParameters[vtkSegmentationConverter::GetReferenceImageGeometryParameterName()].first = serializedGeometry;
  return serializedGeometry;
}
