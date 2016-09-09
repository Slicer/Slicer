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
#include <vtkDecimatePro.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageConstantPad.h>
#include <vtkImageThreshold.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVersion.h>
#include <vtkWindowedSincPolyDataFilter.h>

//----------------------------------------------------------------------------
vtkSegmentationConverterRuleNewMacro(vtkBinaryLabelmapToClosedSurfaceConversionRule);

//----------------------------------------------------------------------------
vtkBinaryLabelmapToClosedSurfaceConversionRule::vtkBinaryLabelmapToClosedSurfaceConversionRule()
{
  this->ConversionParameters[GetDecimationFactorParameterName()] = std::make_pair("0.0",
    "Desired reduction in the total number of polygons. Range: 0.0 (no decimation) to 1.0 (as much simplification as possible)."
    " Value of 0.8 typically reduces data set size by 80% without losing too much details.");
  this->ConversionParameters[GetSmoothingFactorParameterName()] = std::make_pair("0.5",
    "Smoothing factor. Range: 0.0 (no smoothing) to 1.0 (strong smoothing).");
  this->ConversionParameters[GetComputeSurfaceNormalsParameterName()] = std::make_pair("1",
    "Compute surface normals. 1 (default) = surface normals are computed. "
    "0 = surface normals are not computed (slightly faster but produces less smooth surface display).");
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
  vtkOrientedImageData* orientedBinaryLabelMap = vtkOrientedImageData::SafeDownCast(sourceRepresentation);
  if (!orientedBinaryLabelMap)
    {
    vtkErrorMacro("Convert: Source representation is not oriented image data");
    return false;
    }
  vtkSmartPointer<vtkImageData> binaryLabelMap = vtkImageData::SafeDownCast(sourceRepresentation);
  if (!binaryLabelMap.GetPointer())
    {
    vtkErrorMacro("Convert: Source representation is not image data");
    return false;
    }
  vtkPolyData* closedSurfacePolyData = vtkPolyData::SafeDownCast(targetRepresentation);
  if (!closedSurfacePolyData)
    {
    vtkErrorMacro("Convert: Target representation is not poly data");
    return false;
    }

  // Pad labelmap if it has non-background border voxels
  int *binaryLabelMapExtent = binaryLabelMap->GetExtent();
  if (binaryLabelMapExtent[0] > binaryLabelMapExtent[1]
    || binaryLabelMapExtent[2] > binaryLabelMapExtent[3]
    || binaryLabelMapExtent[4] > binaryLabelMapExtent[5])
    {
    // empty labelmap
    vtkDebugMacro("Convert: No polygons can be created, input image extent is empty");
    closedSurfacePolyData->Reset();
    return true;
    }

  /// If input labelmap has non-background border voxels, then those regions remain open in the output closed surface.
  /// This function adds a 1 voxel padding to the labelmap in these cases.
  bool paddingNecessary = this->IsLabelmapPaddingNecessary(binaryLabelMap);
  if (paddingNecessary)
    {
    vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
    padder->SetInputData(binaryLabelMap);
    int extent[6] = { 0, -1, 0, -1, 0, -1 };
    binaryLabelMap->GetExtent(extent);
    // Set the output extent to the new size
    padder->SetOutputWholeExtent(extent[0] - 1, extent[1] + 1, extent[2] - 1, extent[3] + 1, extent[4] - 1, extent[5] + 1);
    padder->Update();
    binaryLabelMap = padder->GetOutput();
    }
  // Clone labelmap and set identity geometry so that the whole transform can be done in IJK space and then
  // the whole transform can be applied on the poly data to transform it to the world coordinate system
  vtkSmartPointer<vtkImageData> binaryLabelmapWithIdentityGeometry = vtkSmartPointer<vtkImageData>::New();
  binaryLabelmapWithIdentityGeometry->ShallowCopy(binaryLabelMap);
  binaryLabelmapWithIdentityGeometry->SetOrigin(0, 0, 0);
  binaryLabelmapWithIdentityGeometry->SetSpacing(1.0, 1.0, 1.0);

  // Get conversion parameters
  double decimationFactor = vtkVariant(this->ConversionParameters[GetDecimationFactorParameterName()].first).ToDouble();
  double smoothingFactor = vtkVariant(this->ConversionParameters[GetSmoothingFactorParameterName()].first).ToDouble();
  int computeSurfaceNormals = vtkVariant(this->ConversionParameters[GetComputeSurfaceNormalsParameterName()].first).ToInt();

  // Run marching cubes
  vtkSmartPointer<vtkDiscreteMarchingCubes> marchingCubes = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
  marchingCubes->SetInputData(binaryLabelmapWithIdentityGeometry);
  const int labelmapFillValue = binaryLabelmapWithIdentityGeometry->GetScalarRange()[1]; // max value
  marchingCubes->GenerateValues(1, labelmapFillValue, labelmapFillValue);
  marchingCubes->ComputeGradientsOff();
  marchingCubes->ComputeNormalsOff();
  marchingCubes->ComputeScalarsOff();
  marchingCubes->Update();
  vtkSmartPointer<vtkPolyData> processingResult = marchingCubes->GetOutput();
  if (processingResult->GetNumberOfPolys() == 0)
    {
    vtkDebugMacro("Convert: No polygons can be created, probably all voxels are empty");
    closedSurfacePolyData->Reset();
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

  if (smoothingFactor>0)
    {
    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smoother->SetInputData(processingResult);
    smoother->SetNumberOfIterations(20); // based on VTK documentation ("Ten or twenty iterations is all the is usually necessary")
    // This formula maps 0.0 -> 1.0 (almost no smoothing), 0.25 -> 0.01 (average smoothing),
    // 0.5 -> 0.001 (more smoothing), 1.0 -> 0.0001 (very strong smoothing).
    double passBand = pow(10.0, -4.0*smoothingFactor);
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
  orientedBinaryLabelMap->GetImageToWorldMatrix(labelmapImageToWorldMatrix);
  labelmapGeometryTransform->SetMatrix(labelmapImageToWorldMatrix);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformPolyDataFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformPolyDataFilter->SetInputData(processingResult);
  transformPolyDataFilter->SetTransform(labelmapGeometryTransform);

  if (computeSurfaceNormals>0)
    {
    vtkSmartPointer<vtkPolyDataNormals> polyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
    polyDataNormals->SetInputConnection(transformPolyDataFilter->GetOutputPort());
    polyDataNormals->ConsistencyOn(); // discrete marching cubes may generate inconsistent surface
    // We almost always perform smoothing, so splitting would not be able to preserve any sharp features
    // (and sharp edges would look like artifacts in the smooth surface).
    polyDataNormals->SplittingOff();
    polyDataNormals->Update();
    closedSurfacePolyData->ShallowCopy(polyDataNormals->GetOutput());
    }
  else
    {
    transformPolyDataFilter->Update();
    closedSurfacePolyData->ShallowCopy(transformPolyDataFilter->GetOutput());
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkBinaryLabelmapToClosedSurfaceConversionRule::IsLabelmapPaddingNecessary(vtkImageData* binaryLabelMap)
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
