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
#include "vtkOrientedImageDataResample.h"

#include "vtkOrientedImageData.h"

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkGeneralTransform.h>
#include <vtkImageReslice.h>
#include <vtkImageConstantPad.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVersionMacros.h>
#include <vtkVector.h>

// STD includes
#include <algorithm>

vtkStandardNewMacro(vtkOrientedImageDataResample);

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class BaseImageScalarType, class ModifierImageScalarType>
void MergeImageGeneric2(
    vtkImageData *baseImage,
    vtkImageData *modifierImage,
    int operation,
    const int extent[6],
    int maskThreshold,
    int fillValue)
{
  // Compute update extent as intersection of base and modifier image extents (extent can be further reduced by specifying a smaller extent)
  int updateExt[6] = { 0, -1, 0, -1, 0, -1 };
  baseImage->GetExtent(updateExt);
  int* modifierExt = modifierImage->GetExtent();
  for (int idx = 0; idx < 3; ++idx)
    {
    if (modifierExt[idx * 2] > updateExt[idx * 2])
      {
      updateExt[idx * 2] = modifierExt[idx * 2];
      }
    if (extent && extent[idx * 2] > updateExt[idx * 2])
      {
      updateExt[idx * 2] = extent[idx * 2];
      }
    if (modifierExt[idx * 2 + 1] < updateExt[idx * 2 + 1])
      {
      updateExt[idx * 2 + 1] = modifierExt[idx * 2 + 1];
      }
    if (extent && extent[idx * 2 + 1] < updateExt[idx * 2 + 1])
      {
      updateExt[idx * 2 + 1] = extent[idx * 2 + 1];
      }
    }
  if (updateExt[0] > updateExt[1] || updateExt[2] > updateExt[3] || updateExt[4] > updateExt[5])
    {
    // base and modifier images don't intersect, nothing need to be done
    return;
    }

  // Get increments to march through data
  vtkIdType baseIncX = 0;
  vtkIdType baseIncY = 0;
  vtkIdType baseIncZ = 0;
  vtkIdType modifierIncX = 0;
  vtkIdType modifierIncY = 0;
  vtkIdType modifierIncZ = 0;
  baseImage->GetContinuousIncrements(updateExt, baseIncX, baseIncY, baseIncZ);
  modifierImage->GetContinuousIncrements(updateExt, modifierIncX, modifierIncY, modifierIncZ);
  int maxX = (updateExt[1] - updateExt[0]) * baseImage->GetNumberOfScalarComponents();
  int maxY = updateExt[3] - updateExt[2];
  int maxZ = updateExt[5] - updateExt[4];
  BaseImageScalarType* baseImagePtr = static_cast<BaseImageScalarType*>(baseImage->GetScalarPointerForExtent(updateExt));
  ModifierImageScalarType* modifierImagePtr = static_cast<ModifierImageScalarType*>(modifierImage->GetScalarPointerForExtent(updateExt));

  if (baseImagePtr == NULL)
    {
    vtkGenericWarningMacro("vtkOrientedImageDataResample::MergeImageGeneric: Base image pointer is invalid");
    return;
    }
  if (modifierImagePtr == NULL)
    {
    vtkGenericWarningMacro("vtkOrientedImageDataResample::MergeImageGeneric: Modifier image pointer is invalid");
    return;
    }

  bool baseImageModified = false;

  // Loop through output pixels
  // There is difference in only one line between min/max computation but the comparison
  // is performed for each pixel, so it is faster to make the conditional expression in the outer loop.
  // Looping is performed in two step: first we just check if any of the pixels have to be changed,
  // if we find any, then we set baseImageModified flag and to the second loop without need to set
  // baseImageModified flag again (setting a flag in a hot loop may impact speed).
  if (operation == vtkOrientedImageDataResample::OPERATION_MAXIMUM)
    {
    for (vtkIdType idxZ = 0; idxZ <= maxZ; idxZ++)
      {
      for (vtkIdType idxY = 0; idxY <= maxY; idxY++)
        {
        for (vtkIdType idxX = 0; idxX <= maxX; idxX++)
          {
          if (*modifierImagePtr > *baseImagePtr)
            {
            *baseImagePtr = *modifierImagePtr;
            baseImageModified = true;
            }
          baseImagePtr++;
          modifierImagePtr++;
          }
        baseImagePtr += baseIncY;
        modifierImagePtr += modifierIncY;
        }
      baseImagePtr += baseIncZ;
      modifierImagePtr += modifierIncZ;
      }
    }
  else if (operation == vtkOrientedImageDataResample::OPERATION_MINIMUM)
    {
    for (vtkIdType idxZ = 0; idxZ <= maxZ; idxZ++)
      {
      for (vtkIdType idxY = 0; idxY <= maxY; idxY++)
        {
        for (vtkIdType idxX = 0; idxX <= maxX; idxX++)
          {
          if (*modifierImagePtr < *baseImagePtr)
            {
            *baseImagePtr = *modifierImagePtr;
            baseImageModified = true;
            }
          baseImagePtr++;
          modifierImagePtr++;
          }
        baseImagePtr += baseIncY;
        modifierImagePtr += modifierIncY;
        }
      baseImagePtr += baseIncZ;
      modifierImagePtr += modifierIncZ;
      }
    }
  else if (operation == vtkOrientedImageDataResample::OPERATION_MASKING)
    {
    BaseImageScalarType fillValueBaseImageType = static_cast<BaseImageScalarType>(fillValue);
    for (vtkIdType idxZ = 0; idxZ <= maxZ; idxZ++)
      {
      for (vtkIdType idxY = 0; idxY <= maxY; idxY++)
        {
        for (vtkIdType idxX = 0; idxX <= maxX; idxX++)
          {
          if (*modifierImagePtr > maskThreshold)
            {
            *baseImagePtr = fillValueBaseImageType;
            baseImageModified = true;
            }
          baseImagePtr++;
          modifierImagePtr++;
        }
        baseImagePtr += baseIncY;
        modifierImagePtr += modifierIncY;
        }
      baseImagePtr += baseIncZ;
      modifierImagePtr += modifierIncZ;
      }
    }
  if (baseImageModified)
    {
    baseImage->Modified();
    }
}

//----------------------------------------------------------------------------
template <class BaseImageScalarType>
void MergeImageGeneric(
    vtkImageData *baseImage,
    vtkImageData *modifierImage,
    int operation,
    const int extent[6],
    int maskThreshold,
    int fillValue)
{
  switch (modifierImage->GetScalarType())
    {
    vtkTemplateMacro((MergeImageGeneric2<BaseImageScalarType, VTK_TT>(
                        baseImage,
                        modifierImage,
                        operation,
                        extent,
                        maskThreshold,
                        fillValue)));
  default:
    vtkGenericWarningMacro("vtkOrientedImageDataResample::MergeImage: Unknown ScalarType");
    }
}

//----------------------------------------------------------------------------
vtkOrientedImageDataResample::vtkOrientedImageDataResample()
{
}

//----------------------------------------------------------------------------
vtkOrientedImageDataResample::~vtkOrientedImageDataResample()
{
}

//-----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* referenceImage, vtkOrientedImageData* outputImage, bool linearInterpolation/*=false*/, bool padImage/*=false*/, vtkAbstractTransform* inputImageTransform/*=NULL*/)
{
  if (!inputImage || !referenceImage || !outputImage)
    {
    return false;
    }

  vtkNew<vtkMatrix4x4> referenceImageToWorldMatrix;
  referenceImage->GetImageToWorldMatrix(referenceImageToWorldMatrix.GetPointer());

  // Simply copy input into output if the reference has the same geometry as the input, so no resampling is necessary
  bool isInputImageTransformIdentity = false;
  if (inputImageTransform == NULL)
    {
    // TODO: this could be improved by checking if inputImageTransform is identity
    isInputImageTransformIdentity = true;
    }
  if ( isInputImageTransformIdentity
    && vtkOrientedImageDataResample::DoGeometriesMatch(inputImage, referenceImage))
    {
    if (vtkOrientedImageDataResample::DoExtentsMatch(inputImage, referenceImage))
      {
      // Input and output are exactly the same
      if (inputImage != outputImage)
        {
        outputImage->DeepCopy(inputImage);
        }
      }
    else
      {
      // Only extent is different
      vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
      padder->SetInputData(inputImage);
      padder->SetOutputWholeExtent(referenceImage->GetExtent());
      padder->Update();
      outputImage->ShallowCopy(padder->GetOutput());
      outputImage->SetImageToWorldMatrix(referenceImageToWorldMatrix.GetPointer());
      }
    return true;
    }

  // Get transform between input and reference
  vtkNew<vtkGeneralTransform> inputImageToReferenceImageTransform;
  inputImageToReferenceImageTransform->PostMultiply();
  inputImageToReferenceImageTransform->Identity();

  // input IJK to RAS
  vtkNew<vtkMatrix4x4> inputImageToWorldMatrix;
  inputImage->GetImageToWorldMatrix(inputImageToWorldMatrix.GetPointer());
  inputImageToReferenceImageTransform->Concatenate(inputImageToWorldMatrix.GetPointer());

  // input warp
  if (inputImageTransform)
    {
    inputImageToReferenceImageTransform->Concatenate(inputImageTransform);
    }

  // output RAS to IJK
  vtkNew<vtkMatrix4x4> worldToReferenceImageMatrix;
  vtkMatrix4x4::Invert(referenceImageToWorldMatrix.GetPointer(), worldToReferenceImageMatrix.GetPointer());
  inputImageToReferenceImageTransform->Concatenate(worldToReferenceImageMatrix.GetPointer());

  // Calculate output extent in reference frame for padding if requested. Use all bounding box corners
  int inputExtentInReferenceFrame[6] = {0,-1,0,-1,0,-1};
  if (padImage)
    {
    vtkOrientedImageDataResample::TransformExtent(inputImage->GetExtent(), inputImageToReferenceImageTransform.GetPointer(), inputExtentInReferenceFrame);
    }
  else
    {
    referenceImage->GetExtent(inputExtentInReferenceFrame);
    }

  // Return with failure if output extent is empty
  if ( inputExtentInReferenceFrame[0] > inputExtentInReferenceFrame[1] || inputExtentInReferenceFrame[2] > inputExtentInReferenceFrame[3] || inputExtentInReferenceFrame[4] > inputExtentInReferenceFrame[5] )
    {
    return false;
    }

  // Make sure input image data fits into the extent. If padding is disabled, then union extent is the reference extent
  int referenceExtent[6] = {0,-1,0,-1,0,-1};
  referenceImage->GetExtent(referenceExtent);
  int unionExtent[6] = { std::min(inputExtentInReferenceFrame[0],referenceExtent[0]), std::max(inputExtentInReferenceFrame[1],referenceExtent[1]),
                         std::min(inputExtentInReferenceFrame[2],referenceExtent[2]), std::max(inputExtentInReferenceFrame[3],referenceExtent[3]),
                         std::min(inputExtentInReferenceFrame[4],referenceExtent[4]), std::max(inputExtentInReferenceFrame[5],referenceExtent[5]) };

  // Invert transform for the resampling
  vtkAbstractTransform* referenceImageToInputImageTransform = inputImageToReferenceImageTransform->GetInverse();
  referenceImageToInputImageTransform->Update();

  // Create clone for input image that has an identity geometry
  //TODO: Creating a new vtkOrientedImageReslice class would be a better solution on the long run
  vtkSmartPointer<vtkMatrix4x4> identityMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  identityMatrix->Identity();
  vtkSmartPointer<vtkOrientedImageData> identityInputImage = vtkSmartPointer<vtkOrientedImageData>::New();
  identityInputImage->ShallowCopy(inputImage);
  identityInputImage->SetGeometryFromImageToWorldMatrix(identityMatrix);

  // Perform resampling
  vtkSmartPointer<vtkImageReslice> resliceFilter = vtkSmartPointer<vtkImageReslice>::New();
  resliceFilter->SetInputData(identityInputImage);
  resliceFilter->SetOutputOrigin(0, 0, 0);
  resliceFilter->SetOutputSpacing(1, 1, 1);
  resliceFilter->SetOutputExtent(unionExtent);
  resliceFilter->SetOutputScalarType(inputImage->GetScalarType());

  resliceFilter->SetResliceTransform(referenceImageToInputImageTransform);

  // Set interpolation mode
  if (linearInterpolation)
    {
    resliceFilter->SetInterpolationModeToLinear();
    }
  else
    {
    resliceFilter->SetInterpolationModeToNearestNeighbor();
    }
  resliceFilter->Update();

  // Set output
  outputImage->ShallowCopy(resliceFilter->GetOutput());
  outputImage->SetGeometryFromImageToWorldMatrix(referenceImageToWorldMatrix.GetPointer());

  return true;
}

//-----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::ResampleOrientedImageToReferenceGeometry(vtkOrientedImageData* inputImage, vtkMatrix4x4* referenceToWorldMatrix, vtkOrientedImageData* outputImage, bool linearInterpolation/*=false*/)
{
  if (!inputImage || !referenceToWorldMatrix || !outputImage)
    {
    return false;
    }

  // Only support the following scalar types
  int inputImageScalarType = inputImage->GetScalarType();
  if ( inputImageScalarType != VTK_UNSIGNED_CHAR
    && inputImageScalarType != VTK_UNSIGNED_SHORT
    && inputImageScalarType != VTK_SHORT )
    {
    vtkErrorWithObjectMacro(inputImage, "ResampleOrientedImageToReferenceGeometry: Input image scalar type must be unsigned char, unsighed short, or short!");
    return false;
    }

  // Determine IJK extent of contained data (non-zero voxels) in the input image
  int inputExtent[6] = {0,-1,0,-1,0,-1};
  inputImage->GetExtent(inputExtent);
  int effectiveInputExtent[6] = {0,-1,0,-1,0,-1};
  if (!vtkOrientedImageDataResample::CalculateEffectiveExtent(inputImage, effectiveInputExtent))
    {
    // Return if effective extent is empty
    return false;
    }

  // Assemble transform
  vtkNew<vtkTransform> referenceImageToInputImageTransform;
  referenceImageToInputImageTransform->Identity();
  referenceImageToInputImageTransform->PostMultiply();

  vtkSmartPointer<vtkMatrix4x4> inputImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  inputImage->GetImageToWorldMatrix(inputImageToWorldMatrix);
  referenceImageToInputImageTransform->Concatenate(inputImageToWorldMatrix);

  vtkSmartPointer<vtkMatrix4x4> worldToReferenceMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  worldToReferenceMatrix->DeepCopy(referenceToWorldMatrix);
  worldToReferenceMatrix->Invert();
  referenceImageToInputImageTransform->Concatenate(worldToReferenceMatrix);

  vtkAbstractTransform* inputImageToReferenceImageTransform = referenceImageToInputImageTransform->GetInverse();
  inputImageToReferenceImageTransform->Update();

  // Determine output origin and spacing using vtkOrientedImageData function
  vtkSmartPointer<vtkOrientedImageData> utilityImageData = vtkSmartPointer<vtkOrientedImageData>::New();
  utilityImageData->SetGeometryFromImageToWorldMatrix(referenceToWorldMatrix);
  double outputOrigin[3] = {0.0, 0.0, 0.0};
  utilityImageData->GetOrigin(outputOrigin);
  double outputSpacing[3] = {0.0, 0.0, 0.0};
  utilityImageData->GetSpacing(outputSpacing);

  // Calculate output extent in reference frame. Use all bounding box corners
  int outputExtent[6] = {0,-1,0,-1,0,-1};
  vtkOrientedImageDataResample::TransformExtent(effectiveInputExtent, referenceImageToInputImageTransform.GetPointer(), outputExtent);

  // Return with failure if effective output extent is empty
  if ( outputExtent[0] == outputExtent[1]
    || outputExtent[2] == outputExtent[3]
    || outputExtent[4] == outputExtent[5] )
    {
    return false;
    }

  // Create clone for input image that has an identity geometry
  //TODO: Creating a new vtkOrientedImageReslice class would be a better solution on the long run
  vtkSmartPointer<vtkMatrix4x4> identityMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  identityMatrix->Identity();
  vtkSmartPointer<vtkOrientedImageData> identityInputImage = vtkSmartPointer<vtkOrientedImageData>::New();
  identityInputImage->ShallowCopy(inputImage);
  identityInputImage->SetGeometryFromImageToWorldMatrix(identityMatrix);

  // Perform resampling
  vtkSmartPointer<vtkImageReslice> resliceFilter = vtkSmartPointer<vtkImageReslice>::New();
  resliceFilter->SetInputData(identityInputImage);
  resliceFilter->SetOutputOrigin(0, 0, 0);
  resliceFilter->SetOutputSpacing(1, 1, 1);
  resliceFilter->SetOutputExtent(outputExtent);

  resliceFilter->SetResliceTransform(inputImageToReferenceImageTransform);

  // Set interpolation mode
  if (linearInterpolation)
    {
    resliceFilter->SetInterpolationModeToLinear();
    }
  else
    {
    resliceFilter->SetInterpolationModeToNearestNeighbor();
    }
  resliceFilter->Update();

  // Set output
  outputImage->DeepCopy(resliceFilter->GetOutput());
  outputImage->SetGeometryFromImageToWorldMatrix(referenceToWorldMatrix);

  return true;
}

//---------------------------------------------------------------------------
bool vtkOrientedImageDataResample::IsEqual(vtkMatrix4x4* lhs, vtkMatrix4x4* rhs)
{
  if (!lhs || !rhs)
    {
    return false;
    }
  return  AreEqualWithTolerance(lhs->GetElement(0,0), rhs->GetElement(0,0)) &&
          AreEqualWithTolerance(lhs->GetElement(0,1), rhs->GetElement(0,1)) &&
          AreEqualWithTolerance(lhs->GetElement(0,2), rhs->GetElement(0,2)) &&
          AreEqualWithTolerance(lhs->GetElement(0,3), rhs->GetElement(0,3)) &&
          AreEqualWithTolerance(lhs->GetElement(1,0), rhs->GetElement(1,0)) &&
          AreEqualWithTolerance(lhs->GetElement(1,1), rhs->GetElement(1,1)) &&
          AreEqualWithTolerance(lhs->GetElement(1,2), rhs->GetElement(1,2)) &&
          AreEqualWithTolerance(lhs->GetElement(1,3), rhs->GetElement(1,3)) &&
          AreEqualWithTolerance(lhs->GetElement(2,0), rhs->GetElement(2,0)) &&
          AreEqualWithTolerance(lhs->GetElement(2,1), rhs->GetElement(2,1)) &&
          AreEqualWithTolerance(lhs->GetElement(2,2), rhs->GetElement(2,2)) &&
          AreEqualWithTolerance(lhs->GetElement(2,3), rhs->GetElement(2,3)) &&
          AreEqualWithTolerance(lhs->GetElement(3,0), rhs->GetElement(3,0)) &&
          AreEqualWithTolerance(lhs->GetElement(3,1), rhs->GetElement(3,1)) &&
          AreEqualWithTolerance(lhs->GetElement(3,2), rhs->GetElement(3,2)) &&
          AreEqualWithTolerance(lhs->GetElement(3,3), rhs->GetElement(3,3));
}

//----------------------------------------------------------------------------
template <typename T> void CalculateEffectiveExtentGeneric(vtkOrientedImageData* image, int effectiveExtent[6], T threshold)
{
  // Get increments to march through image
  int *wholeExt = image->GetExtent();

  effectiveExtent[0] = wholeExt[1]+1;
  effectiveExtent[1] = wholeExt[0]-1;
  effectiveExtent[2] = wholeExt[3]+1;
  effectiveExtent[3] = wholeExt[2]-1;
  effectiveExtent[4] = wholeExt[5]+1;
  effectiveExtent[5] = wholeExt[4]-1;

  if (image->GetScalarPointer() == NULL)
    {
    // no image data is allocated, return with empty extent
    return;
    }

  // Loop through output pixels
  for (int k = wholeExt[4]; k <= wholeExt[5]; k++)
    {
    for (int j = wholeExt[2]; j <= wholeExt[3]; j++)
      {
      bool currentLineInEffectiveExtent = (k >= effectiveExtent[4] && k <= effectiveExtent[5] && j >= effectiveExtent[2] && j <= effectiveExtent[3]);
      int i = wholeExt[0];
      T* imagePtr = static_cast<T*>(image->GetScalarPointer(i,j,k));
      int firstSegmentEnd = currentLineInEffectiveExtent ? effectiveExtent[0] : wholeExt[1];
      for (; i <= firstSegmentEnd; i++)
        {
        if (*(imagePtr++) > threshold)
          {
          if (i < effectiveExtent[0]) { effectiveExtent[0] = i; }
          if (i > effectiveExtent[1]) { effectiveExtent[1] = i; }
          if (j < effectiveExtent[2]) { effectiveExtent[2] = j; }
          if (j > effectiveExtent[3]) { effectiveExtent[3] = j; }
          if (k < effectiveExtent[4]) { effectiveExtent[4] = k; }
          if (k > effectiveExtent[5]) { effectiveExtent[5] = k; }
          currentLineInEffectiveExtent = true;
          break;
          }
        }
      if (!currentLineInEffectiveExtent)
        {
        // We haven't found any non-empty voxel in this line
        continue;
        }
      // Now we need to find the other end of the extent: the last non-empty voxel in the line.
      // The fastest way to find it is to start backward search from the end of the line.
      i = wholeExt[1];
      imagePtr = static_cast<T*>(image->GetScalarPointer(i,j,k));
      for (; i > effectiveExtent[1]; i--)
        {
        if (*(imagePtr--)>threshold)
          {
          if (i < effectiveExtent[0]) { effectiveExtent[0] = i; }
          if (i > effectiveExtent[1]) { effectiveExtent[1] = i; }
          if (j < effectiveExtent[2]) { effectiveExtent[2] = j; }
          if (j > effectiveExtent[3]) { effectiveExtent[3] = j; }
          if (k < effectiveExtent[4]) { effectiveExtent[4] = k; }
          if (k > effectiveExtent[5]) { effectiveExtent[5] = k; }
          break;
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::CalculateEffectiveExtent(vtkOrientedImageData* image, int effectiveExtent[6])
{
  if (!image)
    {
    return false;
    }

  switch (image->GetScalarType())
    {
    vtkTemplateMacro(CalculateEffectiveExtentGeneric<VTK_TT>(image, effectiveExtent, 0));
  default:
    vtkGenericWarningMacro("vtkOrientedImageDataResample::CalculateEffectiveExtent: Unknown ScalarType");
    return false;
    }

  // Return with failure if effective input extent is empty
  if ( effectiveExtent[0] > effectiveExtent[1] || effectiveExtent[2] > effectiveExtent[3] || effectiveExtent[4] > effectiveExtent[5] )
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::DoGeometriesMatch(vtkOrientedImageData* image1, vtkOrientedImageData* image2)
{
  if (!image1 || !image2)
    {
    return false;
    }

  vtkSmartPointer<vtkMatrix4x4> image1ToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  image1->GetImageToWorldMatrix(image1ToWorldMatrix);

  vtkSmartPointer<vtkMatrix4x4> image2ToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  image2->GetImageToWorldMatrix(image2ToWorldMatrix);

  return vtkOrientedImageDataResample::IsEqual(image1ToWorldMatrix, image2ToWorldMatrix);
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::DoExtentsMatch(vtkOrientedImageData* image1, vtkOrientedImageData* image2)
{
  if (!image1 || !image2)
    {
    return false;
    }

  int image1Extent[6] = {0,-1,0,-1,0,-1};
  image1->GetExtent(image1Extent);
  int image2Extent[6] = {0,-1,0,-1,0,-1};
  image2->GetExtent(image2Extent);
  if ( image1Extent[0] != image2Extent[0] || image1Extent[1] != image2Extent[1] || image1Extent[2] != image2Extent[2]
    || image1Extent[3] != image2Extent[3] || image1Extent[4] != image2Extent[4] || image1Extent[5] != image2Extent[5] )
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::DoGeometriesMatchIgnoreOrigin(vtkOrientedImageData* image1, vtkOrientedImageData* image2)
{
  if (!image1 || !image2)
    {
    return false;
    }

  // Create geometry matrices with no origin so that comparison for only directions and spacing is possible
  vtkSmartPointer<vtkMatrix4x4> image1ToWorldMatrixWithoutOrigin = vtkSmartPointer<vtkMatrix4x4>::New();
  image1->GetImageToWorldMatrix(image1ToWorldMatrixWithoutOrigin);
  image1ToWorldMatrixWithoutOrigin->SetElement(0,3,0.0);
  image1ToWorldMatrixWithoutOrigin->SetElement(1,3,0.0);
  image1ToWorldMatrixWithoutOrigin->SetElement(2,3,0.0);

  vtkSmartPointer<vtkMatrix4x4> image2ToWorldMatrixWithoutOrigin = vtkSmartPointer<vtkMatrix4x4>::New();
  image2->GetImageToWorldMatrix(image2ToWorldMatrixWithoutOrigin);
  image2ToWorldMatrixWithoutOrigin->SetElement(0,3,0.0);
  image2ToWorldMatrixWithoutOrigin->SetElement(1,3,0.0);
  image2ToWorldMatrixWithoutOrigin->SetElement(2,3,0.0);

  return vtkOrientedImageDataResample::IsEqual(image1ToWorldMatrixWithoutOrigin, image2ToWorldMatrixWithoutOrigin);
}

//----------------------------------------------------------------------------
void vtkOrientedImageDataResample::TransformExtent(const int inputExtent[6], vtkAbstractTransform* inputToOutputTransform, int outputExtent[6])
{
  if (!inputToOutputTransform)
    {
    return;
    }

  double inputCorners[6] =
    {
    double(inputExtent[0]) - 0.5, double(inputExtent[1]) + 0.5,
    double(inputExtent[2]) - 0.5, double(inputExtent[3]) + 0.5,
    double(inputExtent[4]) - 0.5, double(inputExtent[5]) + 0.5
    };

  // Apply transform on all eight corners and determine output extent based on these transformed corners
  double outputIjkExtentCorner[3] = {0.0, 0.0, 0.0};
  double outputExtentDouble[6] = {VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN};
  for (int i=0; i<2; ++i)
    {
    for (int j=0; j<2; ++j)
      {
      for (int k=0; k<2; ++k)
        {
        double inputBoxCorner[3] = { inputCorners[i], inputCorners[2 + j], inputCorners[4 + k] };
        inputToOutputTransform->TransformPoint(inputBoxCorner, outputIjkExtentCorner);
        if (outputIjkExtentCorner[0] + 0.5 < outputExtentDouble[0])
          {
          outputExtentDouble[0] = outputIjkExtentCorner[0] + 0.5;
          }
        if (outputIjkExtentCorner[0] - 0.5 > outputExtentDouble[1])
          {
          outputExtentDouble[1] = outputIjkExtentCorner[0] - 0.5;
          }
        if (outputIjkExtentCorner[1] + 0.5 < outputExtentDouble[2])
          {
          outputExtentDouble[2] = outputIjkExtentCorner[1] + 0.5;
          }
        if (outputIjkExtentCorner[1] - 0.5 > outputExtentDouble[3])
          {
          outputExtentDouble[3] = outputIjkExtentCorner[1] - 0.5;
          }
        if (outputIjkExtentCorner[2] + 0.5 < outputExtentDouble[4])
          {
          outputExtentDouble[4] = outputIjkExtentCorner[2] + 0.5;
          }
        if (outputIjkExtentCorner[2] - 0.5 > outputExtentDouble[5])
          {
          outputExtentDouble[5] = outputIjkExtentCorner[2] - 0.5;
          }
        }
      }
    }

  // Round to the 6th decimal so that these small values do not shift the extent by a whole voxel (especially in case of zeroes)
  for (int index=0; index<6; ++index)
    {
    long long multiplier = 1000000;
    double roundedExtentElement = (long long)(outputExtentDouble[index] * multiplier + 0.5);
    outputExtentDouble[index] = roundedExtentElement / multiplier;
    }

  // Extend precise extent to integer numbers
  outputExtent[0] = (int)floor(outputExtentDouble[0]);
  outputExtent[1] = (int)ceil(outputExtentDouble[1]);
  outputExtent[2] = (int)floor(outputExtentDouble[2]);
  outputExtent[3] = (int)ceil(outputExtentDouble[3]);
  outputExtent[4] = (int)floor(outputExtentDouble[4]);
  outputExtent[5] = (int)ceil(outputExtentDouble[5]);
}

//----------------------------------------------------------------------------
void vtkOrientedImageDataResample::TransformBounds(const double inputBounds[6], vtkAbstractTransform* inputToOutputTransform, double outputBounds[6])
{
  if (!inputToOutputTransform)
    {
    return;
    }

  // Apply transform on all eight corners and determine output extent based on these transformed corners
  outputBounds[0] = VTK_DOUBLE_MAX;
  outputBounds[1] = VTK_DOUBLE_MIN;
  outputBounds[2] = VTK_DOUBLE_MAX;
  outputBounds[3] = VTK_DOUBLE_MIN;
  outputBounds[4] = VTK_DOUBLE_MAX;
  outputBounds[5] = VTK_DOUBLE_MIN;
  double outputBoxCorner[3];
  for (int i = 0; i<2; ++i)
    {
    for (int j = 0; j<2; ++j)
      {
      for (int k = 0; k<2; ++k)
        {
        double inputBoxCorner[3] = { inputBounds[i], inputBounds[2 + j], inputBounds[4 + k] };
        inputToOutputTransform->TransformPoint(inputBoxCorner, outputBoxCorner);
        if (outputBoxCorner[0] < outputBounds[0])
          {
          outputBounds[0] = outputBoxCorner[0];
          }
        if (outputBoxCorner[0] > outputBounds[1])
          {
          outputBounds[1] = outputBoxCorner[0];
          }
        if (outputBoxCorner[1] < outputBounds[2])
          {
          outputBounds[2] = outputBoxCorner[1];
          }
        if (outputBoxCorner[1] > outputBounds[3])
          {
          outputBounds[3] = outputBoxCorner[1];
          }
        if (outputBoxCorner[2] < outputBounds[4])
          {
          outputBounds[4] = outputBoxCorner[2];
          }
        if (outputBoxCorner[2] > outputBounds[5])
          {
          outputBounds[5] = outputBoxCorner[2];
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkOrientedImageDataResample::TransformOrientedImageDataBounds(vtkOrientedImageData* image, vtkAbstractTransform* transform, double transformedBounds[6])
{
  if (!image || !transform)
    {
    return;
    }

  // Get input image properties
  vtkSmartPointer<vtkMatrix4x4> imageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  image->GetImageToWorldMatrix(imageToWorldMatrix);
  int* imageExtent = image->GetExtent();

  // Append transformed side planes poly data to one model and get bounds
  vtkNew<vtkAppendPolyData> appendPolyData;
  for (int i=0; i<6; i++)
    {
    int normalAxis = i/2; // Axis along which the plane is constant
    double currentPlaneOriginImage[4] = {
      static_cast<double>(imageExtent[0]),
      static_cast<double>(imageExtent[2]),
      static_cast<double>(imageExtent[4]),
      1.0};
    currentPlaneOriginImage[normalAxis] += (imageExtent[i] - imageExtent[normalAxis*2]);
    double currentPlaneOriginWorld[4] = {0.0, 0.0, 0.0, 1.0};
    imageToWorldMatrix->MultiplyPoint(currentPlaneOriginImage, currentPlaneOriginWorld);

    double currentPlanePoint1Image[4] = {currentPlaneOriginImage[0], currentPlaneOriginImage[1], currentPlaneOriginImage[2], 1.0};
    int point1Axis = (normalAxis + 1) % 3; // Axis different from normal axis
    currentPlanePoint1Image[point1Axis] = imageExtent[point1Axis * 2 + 1];
    double currentPlanePoint1World[4] = {0.0, 0.0, 0.0, 1.0};
    imageToWorldMatrix->MultiplyPoint(currentPlanePoint1Image, currentPlanePoint1World);

    double currentPlanePoint2Image[4] = {currentPlaneOriginImage[0], currentPlaneOriginImage[1], currentPlaneOriginImage[2], 1.0};
    int point2Axis = 3 - normalAxis - point1Axis; // Axis different from both normal axis and point 1 axis
    currentPlanePoint2Image[point2Axis] = imageExtent[point2Axis * 2 + 1];
    double currentPlanePoint2World[4] = {0.0, 0.0, 0.0, 1.0};
    imageToWorldMatrix->MultiplyPoint(currentPlanePoint2Image, currentPlanePoint2World);

    vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();
    planeSource->SetOrigin(currentPlaneOriginWorld);
    planeSource->SetPoint1(currentPlanePoint1World);
    planeSource->SetPoint2(currentPlanePoint2World);
    planeSource->SetResolution(5,5); // Use only three subdivision points along each axis
    planeSource->Update();

    appendPolyData->AddInputData(planeSource->GetOutput());
    }

  // Transform boundary poly data
  vtkNew<vtkTransformPolyDataFilter> transformFilter;
  transformFilter->SetInputConnection(appendPolyData->GetOutputPort());
  transformFilter->SetTransform(transform);
  transformFilter->Update();

  // Get bounds of transformed boundary poly data
  transformFilter->GetOutput()->ComputeBounds();
  transformFilter->GetOutput()->GetBounds(transformedBounds);
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::GetTransformBetweenOrientedImages(vtkOrientedImageData* image1, vtkOrientedImageData* image2, vtkTransform* image1ToImage2Transform)
{
  if (!image1 || !image2 || !image1ToImage2Transform)
    {
    return false;
    }

  // Assemble inputTransform
  image1ToImage2Transform->Identity();
  image1ToImage2Transform->PostMultiply();

  vtkSmartPointer<vtkMatrix4x4> image1ToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  image1->GetImageToWorldMatrix(image1ToWorldMatrix);
  image1ToImage2Transform->Concatenate(image1ToWorldMatrix);

  vtkSmartPointer<vtkMatrix4x4> image2ToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  image2->GetImageToWorldMatrix(image2ToWorldMatrix);

  vtkSmartPointer<vtkMatrix4x4> worldToImage2Matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  worldToImage2Matrix->DeepCopy(image2ToWorldMatrix);
  worldToImage2Matrix->Invert();
  image1ToImage2Transform->Concatenate(worldToImage2Matrix);

  return true;
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::IsTransformLinear(vtkAbstractTransform* inputTransform, vtkTransform* outputLinearTransform)
{
  if (!inputTransform || !outputLinearTransform)
    {
    return false;
    }

  // Reset output transform
  outputLinearTransform->Identity();

  // Simply concatenate if type is homogeneous transform
  vtkLinearTransform* linearTransform = vtkLinearTransform::SafeDownCast(inputTransform);
  if (linearTransform)
    {
    outputLinearTransform->Concatenate(linearTransform);
    return true;
    }

  // If general transform then check included concatenated transforms and concatenate them to output transform if all linear
  vtkGeneralTransform* generalTransform = vtkGeneralTransform::SafeDownCast(inputTransform);
  if (generalTransform)
    {
    for (int transformIndex=0; transformIndex<generalTransform->GetNumberOfConcatenatedTransforms(); ++transformIndex)
      {
      vtkLinearTransform* currentLinearTransform = vtkLinearTransform::SafeDownCast(
        generalTransform->GetConcatenatedTransform(transformIndex) );
      if (currentLinearTransform)
        {
        outputLinearTransform->Concatenate(currentLinearTransform);
        }
      else
        {
        outputLinearTransform->Identity();
        return false;
        }
      }

    // All concatenated transforms were linear
    return true;
    }

  vtkErrorWithObjectMacro(inputTransform, "vtkOrientedImageDataResample::IsTransformLinear: Unsupported input transform with type " << inputTransform->GetClassName());
  return false;
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::DoesTransformMatrixContainShear(vtkMatrix4x4* matrix)
{
  if (!matrix)
    {
    return false;
    }
  vtkVector3d xAxis = vtkVector3d(matrix->GetElement(0,0), matrix->GetElement(1,0), matrix->GetElement(2,0));
  vtkVector3d yAxis = vtkVector3d(matrix->GetElement(0,1), matrix->GetElement(1,1), matrix->GetElement(2,1));
  vtkVector3d zAxis = vtkVector3d(matrix->GetElement(0,2), matrix->GetElement(1,2), matrix->GetElement(2,2));
  return !AreEqualWithTolerance(xAxis.Dot(yAxis), 0.0)
      || !AreEqualWithTolerance(xAxis.Dot(zAxis), 0.0)
      || !AreEqualWithTolerance(yAxis.Dot(zAxis), 0.0);
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::PadImageToContainImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* containedImage, vtkOrientedImageData* outputImage)
{
  return vtkOrientedImageDataResample::PadImageToContainImage(inputImage, containedImage, outputImage, NULL);
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::PadImageToContainImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* containedImage, vtkOrientedImageData* outputImage, const int extent[6])
{
  if (!inputImage || !containedImage || !outputImage)
    {
    return false;
    }

  // Get transform between input and contained
  vtkSmartPointer<vtkTransform> containedImageToInputImageTransform = vtkSmartPointer<vtkTransform>::New();
  vtkOrientedImageDataResample::GetTransformBetweenOrientedImages(containedImage, inputImage, containedImageToInputImageTransform);

  // Calculate output extent in reference frame for padding if requested. Use all bounding box corners
  int containedImageExtentInInputImageFrame[6] = {0,-1,0,-1,0,-1};
  const int* containedExtent = extent ? extent : containedImage->GetExtent();
  vtkOrientedImageDataResample::TransformExtent(containedExtent, containedImageToInputImageTransform, containedImageExtentInInputImageFrame);

  // Return with failure if output extent is invalid
  if ( containedImageExtentInInputImageFrame[0] > containedImageExtentInInputImageFrame[1] || containedImageExtentInInputImageFrame[2] > containedImageExtentInInputImageFrame[3] || containedImageExtentInInputImageFrame[4] > containedImageExtentInInputImageFrame[5] )
    {
    return false;
    }

  // Make sure input image data fits into the extent. If padding is disabled, then output extent is the reference extent
  int inputImageExtent[6] = {0,-1,0,-1,0,-1};
  inputImage->GetExtent(inputImageExtent);
  int unionExtent[6] = { std::min(containedImageExtentInInputImageFrame[0],inputImageExtent[0]), std::max(containedImageExtentInInputImageFrame[1],inputImageExtent[1]),
                         std::min(containedImageExtentInInputImageFrame[2],inputImageExtent[2]), std::max(containedImageExtentInInputImageFrame[3],inputImageExtent[3]),
                         std::min(containedImageExtentInInputImageFrame[4],inputImageExtent[4]), std::max(containedImageExtentInInputImageFrame[5],inputImageExtent[5]) };

  // Pad image by expansion extent (extents are fitted to the structure, dilate will reach the edge of the image)
  vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
  padder->SetInputData(inputImage);
  padder->SetOutputWholeExtent(unionExtent);
  padder->Update();

  // Output may be same as input, so save the geometry information before overwriting it
  vtkSmartPointer<vtkMatrix4x4> inputImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  inputImage->GetImageToWorldMatrix(inputImageToWorldMatrix);

  // Set output
  outputImage->DeepCopy(padder->GetOutput());
  outputImage->SetGeometryFromImageToWorldMatrix(inputImageToWorldMatrix);

  return true;
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::MergeImage(
    vtkOrientedImageData* inputImage,
    vtkOrientedImageData* imageToAppend,
    vtkOrientedImageData* outputImage,
    int operation,
    const int extent[6]/*=0*/,
    int maskThreshold /*=0*/,
    int fillValue /*=1*/,
    bool *outputModified /*=NULL*/)
{
  if (outputModified != NULL)
    {
    (*outputModified) = false;
    }
  if (!inputImage || !imageToAppend || !outputImage)
    {
    return false;
    }

  if (!vtkOrientedImageDataResample::DoGeometriesMatch(inputImage, imageToAppend))
    {
    vtkGenericWarningMacro("vtkOrientedImageDataResample::MergeImage failed: geometry mismatch between inputImage and imageToAppend");
    return false;
    }
  if (!vtkOrientedImageDataResample::PadImageToContainImage(inputImage, imageToAppend, outputImage, extent))
    {
    vtkGenericWarningMacro("vtkOrientedImageDataResample::MergeImage: Failed to pad segment labelmap");
    return false;
    }
  vtkMTimeType outputImageMTimeBefore = outputImage->GetMTime();
  switch (inputImage->GetScalarType())
    {
    vtkTemplateMacro(MergeImageGeneric<VTK_TT>(
                       outputImage,
                       imageToAppend,
                       operation,
                       extent,
                       maskThreshold,
                       fillValue));
  default:
    vtkGenericWarningMacro("vtkOrientedImageDataResample::MergeImage: Unknown ScalarType");
    return false;
    }
  vtkMTimeType outputImageMTimeAfter = outputImage->GetMTime();
  if (outputModified != NULL)
    {
    (*outputModified) = (outputImageMTimeBefore<outputImageMTimeAfter);
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::ModifyImage(
    vtkOrientedImageData* inputImage,
    vtkOrientedImageData* modifierImage,
    int operation,
    const int extent[6]/*=0*/,
    int maskThreshold /*=0*/,
    int fillValue /*=1*/)
{
  if (!inputImage || !modifierImage)
    {
    return false;
    }
  if (!vtkOrientedImageDataResample::DoGeometriesMatch(inputImage, modifierImage))
    {
    vtkGenericWarningMacro("vtkOrientedImageDataResample::ModifyImage failed: geometry mismatch between inputImage and modifierImage");
    return false;
    }
  switch (inputImage->GetScalarType())
    {
    vtkTemplateMacro(MergeImageGeneric<VTK_TT>(
                       inputImage,
                       modifierImage,
                       operation,
                       extent,
                       maskThreshold,
                       fillValue));
  default:
    vtkGenericWarningMacro("vtkOrientedImageDataResample::ModifyImage failed: unknown ScalarType");
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::CopyImage(vtkOrientedImageData* imageToCopy, vtkOrientedImageData* outputImage, const int extent[6]/*=0*/)
{
  if (!imageToCopy || !outputImage)
    {
    return false;
    }

  // Copy with clipping to specified extent
  vtkNew<vtkImageConstantPad> padder;
  padder->SetInputData(imageToCopy);
  padder->SetOutputWholeExtent(extent ? const_cast<int*>(extent) : imageToCopy->GetExtent());
  padder->Update();
  outputImage->ShallowCopy(padder->GetOutput());
  outputImage->CopyDirections(imageToCopy);

  return true;
}

//----------------------------------------------------------------------------
void vtkOrientedImageDataResample::TransformOrientedImage(
    vtkOrientedImageData* image,
    vtkAbstractTransform* transform,
    bool geometryOnly/*=false*/,
    bool alwaysResample/*=false*/)
{
  if (!image || !transform)
    {
    return;
    }

  // Linear: simply multiply the geometry matrix with the applied matrix, extent stays the same
  vtkSmartPointer<vtkTransform> worldToTransformedWorldLinearTransform = vtkSmartPointer<vtkTransform>::New();
  if (!alwaysResample && vtkOrientedImageDataResample::IsTransformLinear(transform, worldToTransformedWorldLinearTransform))
    {
    vtkSmartPointer<vtkMatrix4x4> imageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    image->GetImageToWorldMatrix(imageToWorldMatrix);

    vtkSmartPointer<vtkTransform> imageToTransformedWorldTransform = vtkSmartPointer<vtkTransform>::New();
    imageToTransformedWorldTransform->Concatenate(worldToTransformedWorldLinearTransform);
    imageToTransformedWorldTransform->Concatenate(imageToWorldMatrix);

    image->SetGeometryFromImageToWorldMatrix(imageToTransformedWorldTransform->GetMatrix());
    }
  // Non-linear: calculate new extents and change only the extents when applying deformable transform
  else
    {
    // Get geometry transform and its inverse
    vtkSmartPointer<vtkMatrix4x4> imageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    image->GetImageToWorldMatrix(imageToWorldMatrix);

    vtkSmartPointer<vtkMatrix4x4> worldToImageMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    worldToImageMatrix->DeepCopy(imageToWorldMatrix);
    worldToImageMatrix->Invert();

    // Calculate output extent
    double transformedBoundsWorld[6] = {0.0, -1.0, 0.0, -1.0, 0.0, -1.0};
    vtkOrientedImageDataResample::TransformOrientedImageDataBounds(image, transform, transformedBoundsWorld);
    double transformedBoundsWorldCorner1[4] = {transformedBoundsWorld[0], transformedBoundsWorld[2], transformedBoundsWorld[4], 1.0};
    double transformedBoundsWorldCorner2[4] = {transformedBoundsWorld[1], transformedBoundsWorld[3], transformedBoundsWorld[5], 1.0};
    double transformedBoundsImageCorner1[4] = {0.0, 0.0, 0.0, 1.0};
    double transformedBoundsImageCorner2[4] = {0.0, 0.0, 0.0, 1.0};
    worldToImageMatrix->MultiplyPoint(transformedBoundsWorldCorner1, transformedBoundsImageCorner1);
    worldToImageMatrix->MultiplyPoint(transformedBoundsWorldCorner2, transformedBoundsImageCorner2);
    int outputExtent[6] = { // Bounds and extent might be in different order if transform also mirrors (it usually does due to LPS->RAS mapping)
      (int)floor( std::min(transformedBoundsImageCorner1[0], transformedBoundsImageCorner2[0]) ),
      (int)ceil( std::max(transformedBoundsImageCorner1[0], transformedBoundsImageCorner2[0]) ),
      (int)floor( std::min(transformedBoundsImageCorner1[1], transformedBoundsImageCorner2[1]) ),
      (int)ceil( std::max(transformedBoundsImageCorner1[1], transformedBoundsImageCorner2[1]) ),
      (int)floor( std::min(transformedBoundsImageCorner1[2], transformedBoundsImageCorner2[2]) ),
      (int)ceil( std::max(transformedBoundsImageCorner1[2], transformedBoundsImageCorner2[2]) )
      };

    // If only transform of the image's geometry was requested, then set the newly calculated extent and return
    if (geometryOnly)
      {
      image->SetExtent(outputExtent);
      return;
      }

    // Create clone for input image that has an identity geometry
    vtkSmartPointer<vtkMatrix4x4> identityMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    identityMatrix->Identity();
    vtkSmartPointer<vtkOrientedImageData> identityInputImage = vtkSmartPointer<vtkOrientedImageData>::New();
    identityInputImage->ShallowCopy(image);
    identityInputImage->SetGeometryFromImageToWorldMatrix(identityMatrix);

    // Invert input transform, so it becomes transformedWorldToWorld transform
    vtkAbstractTransform* transformedWorldToWorldTransform = transform->GetInverse();
    transformedWorldToWorldTransform->Update();

    // Create reslice transform
    vtkSmartPointer<vtkGeneralTransform> resliceTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    resliceTransform->Identity();
    resliceTransform->PostMultiply();
    resliceTransform->Concatenate(imageToWorldMatrix);
    resliceTransform->Concatenate(transformedWorldToWorldTransform);
    resliceTransform->Concatenate(worldToImageMatrix);

    // Perform resampling
    vtkNew<vtkImageReslice> reslice;
    reslice->SetInputData(identityInputImage);
    // reslice->SetInterpolationModeToLinear(); //TODO: Use this option for fractional labelmaps
    reslice->SetInterpolationModeToNearestNeighbor();
    reslice->SetBackgroundColor(0, 0, 0, 0);
    reslice->AutoCropOutputOff();
    reslice->SetOptimization(1);
    reslice->SetOutputOrigin(0, 0, 0);
    reslice->SetOutputSpacing(1, 1, 1);
    reslice->SetOutputExtent(outputExtent);
    reslice->SetResliceTransform(resliceTransform);
    reslice->Update();

    image->DeepCopy(reslice->GetOutput());
    image->SetGeometryFromImageToWorldMatrix(imageToWorldMatrix);
    }
}

//----------------------------------------------------------------------------
void vtkOrientedImageDataResample::PrintImageInformation(vtkImageData* imageData, ostream& os, vtkIndent indent)
{
  if (!imageData)
    {
    os << indent << "(none)\n";
    return;
    }
  if (imageData)
    {
    os << indent << "Origin: "
      << imageData->GetOrigin()[0] << " "
      << imageData->GetOrigin()[1] << " "
      << imageData->GetOrigin()[2] << "\n";
    os << indent << "Spacing: "
      << imageData->GetSpacing()[0] << " "
      << imageData->GetSpacing()[1] << " "
      << imageData->GetSpacing()[2] << "\n";
    os << indent << "Extent:";
    for(int j=0; j<6; j++)
      {
      os << " " << imageData->GetExtent()[j];
      }
    os << "\n";
    os << indent << "Scalar type: " << imageData->GetScalarTypeAsString() << "\n";
    os << indent << "Number of components: " << imageData->GetNumberOfScalarComponents() << "\n";
    }
  vtkOrientedImageData* orientedImageData = vtkOrientedImageData::SafeDownCast(imageData);
  if (orientedImageData)
    {
    os << indent << "IJKToRASDirections:\n";
    vtkNew<vtkMatrix4x4> directions;
    orientedImageData->GetDirectionMatrix(directions.GetPointer());
    for(int i=0; i<3; i++)
      {
      os << indent.GetNextIndent();
      for(int j=0; j<3; j++)
        {
        os << directions->GetElement(i,j) << " ";
        }
      os << "\n";
      }
    }
}

//----------------------------------------------------------------------------
template <typename T> void FillImageGeneric(vtkImageData* image, T fillValue, const int extent[6])
{
  if (image->GetScalarPointer() == NULL)
    {
    // no image data is allocated, return with empty extent
    return;
    }
  // Use the whole extent if extent is not specified
  const int *wholeExt = extent ? extent : image->GetExtent();
  if (wholeExt[0]>wholeExt[1] || wholeExt[2]>wholeExt[3] || wholeExt[4]>wholeExt[5])
    {
    // empty image
    return;
    }

  // Pre-allocate a filled row so that we can just memcpy it into the image
  unsigned int numberOfScalarsPerRow = image->GetNumberOfScalarComponents() * (wholeExt[1]-wholeExt[0]+1);
  std::vector<T> filledRow(numberOfScalarsPerRow, fillValue);
  T* filledRowPtr = &(filledRow[0]);
  unsigned int rowSizeInBytes = image->GetScalarSize() * numberOfScalarsPerRow;

  // Loop through output pixels
  for (int k = wholeExt[4]; k <= wholeExt[5]; k++)
    {
    for (int j = wholeExt[2]; j <= wholeExt[3]; j++)
      {
      T* imagePtr = static_cast<T*>(image->GetScalarPointer(wholeExt[0],j,k));
      memcpy(imagePtr, filledRowPtr, rowSizeInBytes);
      }
    }
  image->Modified();
}

//----------------------------------------------------------------------------
void vtkOrientedImageDataResample::FillImage(vtkImageData* image, double fillValue, const int extent[6]/*=NULL*/)
{
  if (!image)
    {
    return;
    }
  if (image->GetPointData() == NULL || image->GetPointData()->GetScalars() == NULL)
    {
    return;
    }
  switch (image->GetScalarType())
    {
    vtkTemplateMacro(FillImageGeneric<VTK_TT>(image, fillValue, extent));
  default:
    vtkGenericWarningMacro("vtkOrientedImageDataResample::FillImage: Unknown ScalarType");
    }
}
