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

#include "vtkOrientedImageData.h"

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

//----------------------------------------------------------------------------
vtkSegmentationConverterRuleNewMacro(vtkBinaryLabelmapToClosedSurfaceConversionRule);

//----------------------------------------------------------------------------
vtkBinaryLabelmapToClosedSurfaceConversionRule::vtkBinaryLabelmapToClosedSurfaceConversionRule()
{
  this->ConversionParameters[GetDecimationFactorParameterName()] = std::make_pair("0.0", "Desired reduction in the total number of polygons (e.g., if set to 0.9, then reduce the data set to 10% of its original size)");
  this->ConversionParameters[GetSmoothingFactorParameterName()] = std::make_pair("0.1", "Relaxation factor for Laplacian smoothing. Value of 0 results in no smoothing, while 1 means significant smoothing.");
}

//----------------------------------------------------------------------------
vtkBinaryLabelmapToClosedSurfaceConversionRule::~vtkBinaryLabelmapToClosedSurfaceConversionRule()
{
}

//----------------------------------------------------------------------------
unsigned int vtkBinaryLabelmapToClosedSurfaceConversionRule::GetConversionCost(
    vtkDataObject* vtkNotUsed(sourceRepresentation)/*=NULL*/,
    vtkDataObject* vtkNotUsed(targetRepresentation)/*=NULL*/)
{
  // Rough input-independent guess (ms)
  return 500;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkBinaryLabelmapToClosedSurfaceConversionRule::ConstructRepresentationObjectByRepresentation(std::string representationName)
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
vtkDataObject* vtkBinaryLabelmapToClosedSurfaceConversionRule::ConstructRepresentationObjectByClass(std::string className)
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
bool vtkBinaryLabelmapToClosedSurfaceConversionRule::Convert(vtkDataObject* sourceRepresentation, vtkDataObject* targetRepresentation)
{
  // Check validity of source and target representation objects
  vtkOrientedImageData* binaryLabelMap = vtkOrientedImageData::SafeDownCast(sourceRepresentation);
  if (!binaryLabelMap)
    {
    vtkErrorMacro("Convert: Source representation is not an oriented image data");
    return false;
    }
  vtkPolyData* closedSurfacePolyData = vtkPolyData::SafeDownCast(targetRepresentation);
  if (!closedSurfacePolyData)
    {
    vtkErrorMacro("Convert: Target representation is not a poly data");
    return false;
    }

  // Pad labelmap if it has non-background border voxels
  int *binaryLabelMapExtent = binaryLabelMap->GetExtent();
  if (binaryLabelMapExtent[0] > binaryLabelMapExtent[1]
    || binaryLabelMapExtent[2] > binaryLabelMapExtent[3]
    || binaryLabelMapExtent[4] > binaryLabelMapExtent[5])
    {
    // empty labelmap
    vtkErrorMacro("Convert: No polygons can be created, input image extent is empty");
    return false;
    }
  bool paddingNecessary = this->IsLabelmapPaddingNecessary(binaryLabelMap);
  if (paddingNecessary)
    {
    vtkOrientedImageData* paddedLabelmap = vtkOrientedImageData::New();
    paddedLabelmap->DeepCopy(binaryLabelMap);
    this->PadLabelmap(paddedLabelmap);
    binaryLabelMap = paddedLabelmap;
    }

  // Get conversion parameters
  double decimationFactor = vtkVariant(this->ConversionParameters[GetDecimationFactorParameterName()].first).ToDouble();
  double smoothingFactor = vtkVariant(this->ConversionParameters[GetSmoothingFactorParameterName()].first).ToDouble();

  // Save geometry of oriented image data before conversion so that it can be applied on the poly data afterwards
  vtkSmartPointer<vtkMatrix4x4> labelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  binaryLabelMap->GetImageToWorldMatrix(labelmapImageToWorldMatrix);

  // Clone labelmap and set identity geometry so that the whole transform can be done in IJK space and then
  // the whole transform can be applied on the poly data to transform it to the world coordinate system
  vtkSmartPointer<vtkOrientedImageData> binaryLabelmapWithIdentityGeometry = vtkSmartPointer<vtkOrientedImageData>::New();
  binaryLabelmapWithIdentityGeometry->ShallowCopy(binaryLabelMap);
  vtkSmartPointer<vtkMatrix4x4> identityMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  identityMatrix->Identity();
  binaryLabelmapWithIdentityGeometry->SetGeometryFromImageToWorldMatrix(identityMatrix);

  // Run marching cubes
  vtkSmartPointer<vtkMarchingCubes> marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
  marchingCubes->SetInputData(binaryLabelmapWithIdentityGeometry);
  marchingCubes->SetNumberOfContours(1);
  marchingCubes->SetValue(0, 0.5); //TODO: In the vtkLabelmapToModelFilter class this is LabelValue/2.0. If we know why, it would make sense to explain it here.
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
    vtkErrorMacro("Convert: No polygons can be created");
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
    binaryLabelMap->Delete();
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkBinaryLabelmapToClosedSurfaceConversionRule::IsLabelmapPaddingNecessary(vtkOrientedImageData* binaryLabelMap)
{
  if (!binaryLabelMap)
    {
    return false;
    }

  // Only support the following scalar types
  int inputImageScalarType = binaryLabelMap->GetScalarType();
  if ( inputImageScalarType != VTK_UNSIGNED_CHAR
    && inputImageScalarType != VTK_UNSIGNED_SHORT
    && inputImageScalarType != VTK_SHORT )
    {
    vtkErrorWithObjectMacro(binaryLabelMap, "IsLabelmapPaddingNecessary: Image scalar type must be unsigned char, unsighed short, or short!");
    return false;
    }

  // Check if there are non-zero voxels in the labelmap
  int extent[6] = {0,-1,0,-1,0,-1};
  binaryLabelMap->GetExtent(extent);
  int dimensions[3] = {0, 0, 0};
  binaryLabelMap->GetDimensions(dimensions);
  // Handle three scalar types
  unsigned char* imagePtrUChar = (unsigned char*)binaryLabelMap->GetScalarPointerForExtent(extent);
  unsigned short* imagePtrUShort = (unsigned short*)binaryLabelMap->GetScalarPointerForExtent(extent);
  short* imagePtrShort = (short*)binaryLabelMap->GetScalarPointerForExtent(extent);

  for (int i=0; i<dimensions[0]; ++i)
    {
    for (int j=0; j<dimensions[1]; ++j)
      {
      for (int k=0; k<dimensions[2]; ++k)
        {
        if (i!=0 && i!=dimensions[0]-1 && j!=0 && j!=dimensions[1]-1 && k!=0 && k!=dimensions[2]-1)
          {
          // Skip non-border voxels
          continue;
          }
        int voxelValue = 0;
        if (inputImageScalarType == VTK_UNSIGNED_CHAR)
          {
          voxelValue = (*(imagePtrUChar + i + j*dimensions[0] + k*dimensions[0]*dimensions[1]));
          }
        else if (inputImageScalarType == VTK_UNSIGNED_SHORT)
          {
          voxelValue = (*(imagePtrUShort + i + j*dimensions[0] + k*dimensions[0]*dimensions[1]));
          }
        else if (inputImageScalarType == VTK_SHORT)
          {
          voxelValue = (*(imagePtrShort + i + j*dimensions[0] + k*dimensions[0]*dimensions[1]));
          }

        if (voxelValue != 0)
          {
          return true;
          }
        }
      }
    }

  return false;
}

//----------------------------------------------------------------------------
void vtkBinaryLabelmapToClosedSurfaceConversionRule::PadLabelmap(vtkOrientedImageData* binaryLabelMap)
{
  vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
  padder->SetInputData(binaryLabelMap);
  int extent[6] = {0,-1,0,-1,0,-1};
  binaryLabelMap->GetExtent(extent);
  // Set the output extent to the new size
  padder->SetOutputWholeExtent(extent[0]-1, extent[1]+1, extent[2]-1, extent[3]+1, extent[4]-1, extent[5]+1);
  padder->Update();
  binaryLabelMap->vtkImageData::DeepCopy(padder->GetOutput());
}
