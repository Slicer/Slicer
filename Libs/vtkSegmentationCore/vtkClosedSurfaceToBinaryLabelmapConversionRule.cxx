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
#include "vtkSegmentation.h"

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

int DEFAULT_LABEL_VALUE = 1;

//----------------------------------------------------------------------------
vtkSegmentationConverterRuleNewMacro(vtkClosedSurfaceToBinaryLabelmapConversionRule);

//----------------------------------------------------------------------------
vtkClosedSurfaceToBinaryLabelmapConversionRule::vtkClosedSurfaceToBinaryLabelmapConversionRule()
{
  this->ReplaceTargetRepresentation = true;

  // Reference image geometry parameter
  this->ConversionParameters[vtkSegmentationConverter::GetReferenceImageGeometryParameterName()] = std::make_pair("",
    "Image geometry description string determining the geometry of the labelmap that is created in course of conversion."
    " Can be copied from a volume, using the button.");
  // Oversampling factor parameter
  this->ConversionParameters[GetOversamplingFactorParameterName()] = std::make_pair("1",
    "Determines the oversampling of the reference image geometry. If it's a number, then all segments are oversampled"
    " with the same value (value of 1 means no oversampling). If it has the value \"A\", then automatic oversampling is calculated.");
  // Crop to reference geometry parameter
  this->ConversionParameters[GetCropToReferenceImageGeometryParameterName()] = std::make_pair("0",
    "Crop the model to the extent of reference geometry. 0 (default) = created labelmap will contain the entire model."
    " 1 = created labelmap extent will be within reference image extent.");
  // Collapse labelmaps parameter
  this->ConversionParameters[GetCollapseLabelmapsParameterName()] = std::make_pair("1",
    "Merge the labelmaps into as few shared labelmaps as possible"
    " 1 = created labelmaps will be shared if possible without overwriting each other.");
}

//----------------------------------------------------------------------------
vtkClosedSurfaceToBinaryLabelmapConversionRule::~vtkClosedSurfaceToBinaryLabelmapConversionRule() = default;

//----------------------------------------------------------------------------
unsigned int vtkClosedSurfaceToBinaryLabelmapConversionRule::GetConversionCost(
  vtkDataObject* vtkNotUsed(sourceRepresentation)/*=nullptr*/,
  vtkDataObject* vtkNotUsed(targetRepresentation)/*=nullptr*/)
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
    return nullptr;
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
    return nullptr;
    }
}

//----------------------------------------------------------------------------
bool vtkClosedSurfaceToBinaryLabelmapConversionRule::Convert(vtkSegment* segment)
{
  vtkSmartPointer<vtkOrientedImageData> outputGeometryLabelmap = vtkOrientedImageData::SafeDownCast(
    segment->GetRepresentation(this->GetTargetRepresentationName()));
  this->CreateTargetRepresentation(segment);

  // Check validity of source and target representation objects
  vtkPolyData* closedSurfacePolyData = vtkPolyData::SafeDownCast(segment->GetRepresentation(this->GetSourceRepresentationName()));
  if (!closedSurfacePolyData)
    {
    vtkErrorMacro("Convert: Source representation is not a poly data!");
    return false;
    }

  if (closedSurfacePolyData->GetNumberOfPoints() < 2 || closedSurfacePolyData->GetNumberOfCells() < 2)
    {
    vtkDebugMacro("Convert: Cannot create binary labelmap from surface with number of points: "
      << closedSurfacePolyData->GetNumberOfPoints() << " and number of cells: " << closedSurfacePolyData->GetNumberOfCells());
    return false;
    }

  vtkOrientedImageData* binaryLabelmap = vtkOrientedImageData::SafeDownCast(segment->GetRepresentation(this->GetTargetRepresentationName()));
  if (!binaryLabelmap)
    {
    vtkErrorMacro("Convert: Target representation is not an oriented image data!");
    return false;
    }

  // Setup output labelmap

  // Compute output labelmap geometry based on poly data, an reference image
  // geometry, and store the calculated geometry in output labelmap image data
  if (!this->UseOutputImageDataGeometry)
    {
    if (!this->CalculateOutputGeometry(closedSurfacePolyData, binaryLabelmap))
      {
      vtkErrorMacro("Convert: Failed to calculate output image geometry!");
      return false;
      }
    }
  else if (outputGeometryLabelmap)
    {
    std::string geometryString = vtkSegmentationConverter::SerializeImageGeometry(outputGeometryLabelmap);
    vtkSegmentationConverter::DeserializeImageGeometry(geometryString, binaryLabelmap, false);
    }

  // Allocate output image data
  binaryLabelmap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  void* binaryLabelmapVoxelsPointer = binaryLabelmap->GetScalarPointerForExtent(binaryLabelmap->GetExtent());
  if (!binaryLabelmapVoxelsPointer)
    {
    vtkErrorMacro("Convert: Failed to allocate memory for output labelmap image!");
    return false;
    }
  else
    {
    // Set voxel values to 0
    int extent[6] = {0,-1,0,-1,0,-1};
    binaryLabelmap->GetExtent(extent);
    memset(binaryLabelmapVoxelsPointer, 0, ((extent[1]-extent[0]+1)*(extent[3]-extent[2]+1)*(extent[5]-extent[4]+1) *
      binaryLabelmap->GetScalarSize() * binaryLabelmap->GetNumberOfScalarComponents()));
    }

  // Perform conversion

  // Now the output labelmap image data contains the right geometry.
  // We need to apply inverse of geometry matrix to the input poly data so that we can perform
  // the conversion in IJK space, because the filters do not support oriented image data.
  vtkSmartPointer<vtkMatrix4x4> outputLabelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  binaryLabelmap->GetImageToWorldMatrix(outputLabelmapImageToWorldMatrix);
  vtkSmartPointer<vtkTransform> inverseOutputLabelmapGeometryTransform = vtkSmartPointer<vtkTransform>::New();
  inverseOutputLabelmapGeometryTransform->SetMatrix(outputLabelmapImageToWorldMatrix);
  inverseOutputLabelmapGeometryTransform->Inverse();

  // Set geometry to identity for the volume so that we can perform the stencil operation in IJK space
  vtkSmartPointer<vtkMatrix4x4> identityMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  identityMatrix->Identity();
  binaryLabelmap->SetGeometryFromImageToWorldMatrix(identityMatrix);

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
  polyDataToImageStencil->SetOutputSpacing(binaryLabelmap->GetSpacing());
  polyDataToImageStencil->SetOutputOrigin(binaryLabelmap->GetOrigin());
  polyDataToImageStencil->SetOutputWholeExtent(binaryLabelmap->GetExtent());

  // Convert stencil to image
  vtkNew<vtkImageStencil> stencil;
  stencil->SetInputData(binaryLabelmap);
  stencil->SetStencilConnection(polyDataToImageStencil->GetOutputPort());
  stencil->ReverseStencilOn();
  // If the output labelmap was to required to be unsigned char, we could use the segment label value.
  // To ensure that the label value is < 255, we set it to 1. Collapsing the labelmaps during post-conversion may assign new a value regardless.
  stencil->SetBackgroundValue(DEFAULT_LABEL_VALUE); // General foreground value is 1 (background value because of reverse stencil)

  // Save result to output
  vtkNew<vtkImageCast> imageCast;
  imageCast->SetInputConnection(stencil->GetOutputPort());
  imageCast->SetOutputScalarTypeToUnsignedChar();
  imageCast->Update();
  binaryLabelmap->ShallowCopy(imageCast->GetOutput());

  // Restore geometry of the labelmap that we set to identity before conversion
  // (so that we can perform the stencil operations in IJK space)
  binaryLabelmap->SetGeometryFromImageToWorldMatrix(outputLabelmapImageToWorldMatrix);

  // Set segment value to 1
  segment->SetLabelValue(DEFAULT_LABEL_VALUE);

  return true;
}

//----------------------------------------------------------------------------
bool vtkClosedSurfaceToBinaryLabelmapConversionRule::PostConvert(vtkSegmentation* segmentation)
{
  int collapseLabelmaps = vtkVariant(this->ConversionParameters[GetCollapseLabelmapsParameterName()].first).ToInt();
  if (collapseLabelmaps > 0)
    {
    segmentation->CollapseBinaryLabelmaps(false);
    }
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
    geometryString = this->GetDefaultImageGeometryStringForPolyData(closedSurfacePolyData);
    vtkInfoMacro("CalculateOutputGeometry: No image geometry specified, default geometry is calculated (" << geometryString << ")");

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

  int cropToReferenceImageGeometry = 0;
    {
    std::string cropToReferenceImageGeometryString = this->ConversionParameters[GetCropToReferenceImageGeometryParameterName()].first;
    std::stringstream ss;
    ss << cropToReferenceImageGeometryString;
    ss >> cropToReferenceImageGeometry;
    if (ss.fail())
      {
      cropToReferenceImageGeometry = 0;
      }
    }

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
  double surfaceBounds[6] = { 0, 0, 0, 0, 0, 0 };
  transformedClosedSurfacePolyData->GetPoints()->GetBounds(surfaceBounds);

  // Expand floating point bounds to extent integers
  int surfaceExtent[6] = { 0, -1, 0, -1, 0, -1 };
  surfaceExtent[0] = (int)floor(surfaceBounds[0]);
  surfaceExtent[1] = (int)ceil(surfaceBounds[1]);
  surfaceExtent[2] = (int)floor(surfaceBounds[2]);
  surfaceExtent[3] = (int)ceil(surfaceBounds[3]);
  surfaceExtent[4] = (int)floor(surfaceBounds[4]);
  surfaceExtent[5] = (int)ceil(surfaceBounds[5]);

  if (cropToReferenceImageGeometry)
    {
    // Set effective extent to be maximum as large as the reference extent (less memory needed if the extent only covers the non-zero region)
    int referenceExtent[6] = { 0, -1, 0, -1, 0, -1 };
    geometryImageData->GetExtent(referenceExtent);
    if (surfaceExtent[0] > referenceExtent[0]) { referenceExtent[0] = surfaceExtent[0]; }
    if (surfaceExtent[1] < referenceExtent[1]) { referenceExtent[1] = surfaceExtent[1]; }
    if (surfaceExtent[2] > referenceExtent[2]) { referenceExtent[2] = surfaceExtent[2]; }
    if (surfaceExtent[3] < referenceExtent[3]) { referenceExtent[3] = surfaceExtent[3]; }
    if (surfaceExtent[4] > referenceExtent[4]) { referenceExtent[4] = surfaceExtent[4]; }
    if (surfaceExtent[5] < referenceExtent[5]) { referenceExtent[5] = surfaceExtent[5]; }
    geometryImageData->SetExtent(referenceExtent);
    }
  else
    {
    // Set effective extent to be just large enough to contain the full surface
    geometryImageData->SetExtent(surfaceExtent);
    }

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

  // set spacing to have an approxmately 250^3 volume
  // this size is not too large for average computing hardware yet
  // it is sufficiently detailed for many applications
  const double preferredVolumeSizeInVoxels = 250 * 250 * 250;
  double volumeSizeInMm3 = (bounds[1] - bounds[0]) * (bounds[3] - bounds[2]) * (bounds[5] - bounds[4]);
  double spacing = std::pow(volumeSizeInMm3 / preferredVolumeSizeInVoxels, 1 / 3.);
  geometryMatrix->SetElement(0, 0, spacing);
  geometryMatrix->SetElement(1, 1, spacing);
  geometryMatrix->SetElement(2, 2, spacing);

  // Set extent
  int extent[6] =
    {
    0, (int)((bounds[1] - bounds[0]) / spacing),
    0, (int)((bounds[3] - bounds[2]) / spacing),
    0, (int)((bounds[5] - bounds[4]) / spacing)
    };

  // Serialize geometry
  std::string serializedGeometry = vtkSegmentationConverter::SerializeImageGeometry(geometryMatrix, extent);
  return serializedGeometry;
}
