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
#include "vtkSegmentationConverter.h"
#include "vtkOrientedImageData.h"

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkBoundingBox.h>
#include <vtkGeneralTransform.h>
#include <vtkImageCast.h>
#include <vtkImageConstantPad.h>
#include <vtkImageMask.h>
#include <vtkImageReslice.h>
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
#include <vector>

vtkStandardNewMacro(vtkOrientedImageDataResample);

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class BaseImageScalarType, class ModifierImageScalarType>
void MergeImageGeneric2(
    vtkImageData *baseImage,
    vtkImageData *modifierImage,
    int operation,
    const int extent[6]/*=nullptr*/,
    double maskThreshold,
    double fillValue)
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

  if (baseImagePtr == nullptr)
    {
    vtkGenericWarningMacro("vtkOrientedImageDataResample::MergeImageGeneric: Base image pointer is invalid");
    return;
    }
  if (modifierImagePtr == nullptr)
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
          if (static_cast<BaseImageScalarType>(*modifierImagePtr) > *baseImagePtr)
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
          if (static_cast<BaseImageScalarType>(*modifierImagePtr) < *baseImagePtr)
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

    // Make sure the fill value is valid for the base image scalar range
    BaseImageScalarType fillValueBaseImageType = 0;
    if (fillValue < baseImage->GetScalarTypeMin())
      {
      fillValueBaseImageType = static_cast<BaseImageScalarType>(baseImage->GetScalarTypeMin());
      }
    else if (fillValue > baseImage->GetScalarTypeMax())
      {
      fillValueBaseImageType = static_cast<BaseImageScalarType>(baseImage->GetScalarTypeMax());
      }
    else
      {
      fillValueBaseImageType = static_cast<BaseImageScalarType>(fillValue);
      }

    // Make sure the threshold is valid for the modifier scalar range
    ModifierImageScalarType maskThresholdModifierType = 0;
    if (maskThreshold < modifierImage->GetScalarTypeMin())
      {
      maskThresholdModifierType = static_cast<ModifierImageScalarType>(modifierImage->GetScalarTypeMin());
      }
    else if (maskThreshold > modifierImage->GetScalarTypeMax())
      {
      maskThresholdModifierType = static_cast<ModifierImageScalarType>(modifierImage->GetScalarTypeMax());
      }
    else
      {
      maskThresholdModifierType = static_cast<ModifierImageScalarType>(maskThreshold);
      }

    for (vtkIdType idxZ = 0; idxZ <= maxZ; idxZ++)
      {
      for (vtkIdType idxY = 0; idxY <= maxY; idxY++)
        {
        for (vtkIdType idxX = 0; idxX <= maxX; idxX++)
          {
          if ((*modifierImagePtr) > maskThresholdModifierType)
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
    const int extent[6]/*=nullptr*/,
    double maskThreshold,
    double fillValue)
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
vtkOrientedImageDataResample::vtkOrientedImageDataResample() = default;

//----------------------------------------------------------------------------
vtkOrientedImageDataResample::~vtkOrientedImageDataResample() = default;

//-----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* referenceImage, vtkOrientedImageData* outputImage, bool linearInterpolation/*=false*/, bool padImage/*=false*/, vtkAbstractTransform* inputImageTransform/*=nullptr*/, double backgroundValue/*=0*/)
{
  if (!inputImage || !referenceImage || !outputImage)
    {
    return false;
    }

  vtkNew<vtkMatrix4x4> referenceImageToWorldMatrix;
  referenceImage->GetImageToWorldMatrix(referenceImageToWorldMatrix.GetPointer());

  int* inputExtent = inputImage->GetExtent();
  if (inputExtent[0] > inputExtent[1] || inputExtent[2] > inputExtent[3] || inputExtent[4] > inputExtent[5])
    {
    // empty input image, fill with background value
    outputImage->SetExtent(referenceImage->GetExtent());

    // if data is not allocated then GetScalarType() would always return VTK_DOUBLE,
    // which is not a good default image type
    if (backgroundValue >= 0 && backgroundValue <= 255)
      {
      outputImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
      }
    else if (backgroundValue >= -32768 && backgroundValue <= 32767)
      {
      outputImage->AllocateScalars(VTK_SHORT, 1);
      }
    else if (backgroundValue >= 0 && backgroundValue <= 65535)
      {
      outputImage->AllocateScalars(VTK_UNSIGNED_SHORT, 1);
      }
    else
      {
      outputImage->AllocateScalars(VTK_DOUBLE, 1);
      }

    outputImage->GetPointData()->GetScalars()->Fill(backgroundValue);
    outputImage->SetImageToWorldMatrix(referenceImageToWorldMatrix.GetPointer());
    return true;
    }

  // Simply copy input into output if the reference has the same geometry as the input, so no resampling is necessary
  bool isInputImageTransformIdentity = false;
  if (inputImageTransform == nullptr)
    {
    isInputImageTransformIdentity = true;
    }
  else
    {
    // TODO: this could be further improved to detect if inputImageTransform is identity
    vtkGeneralTransform* inputImageTransformGeneral = vtkGeneralTransform::SafeDownCast(inputImageTransform);
    if (inputImageTransformGeneral && inputImageTransformGeneral->GetNumberOfConcatenatedTransforms() == 0)
      {
      isInputImageTransformIdentity = true;
      }
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
      int* referenceExtent = referenceImage->GetExtent();
      int unionExtent[6] = { 0, -1, 0, -1, 0, -1 };
      if (padImage)
        {
        bool referenceExtentValid = true;
        for (int i = 0; i < 3; ++i)
          {
          if (referenceExtent[i * 2 + 1] < referenceExtent[i * 2])
            {
            referenceExtentValid = false;
            }
          }

        // Make sure input image data fits into the extent.
        for (int i = 0; i < 3; i++)
          {
          if (referenceExtentValid)
            {
            unionExtent[i * 2] = std::min(inputExtent[i * 2], referenceExtent[i * 2]);
            unionExtent[i * 2 + 1] = std::max(inputExtent[i * 2 + 1], referenceExtent[i * 2 + 1]);
            }
          else
            {
            // Output extent is empty
            unionExtent[i * 2] = inputExtent[i * 2];
            unionExtent[i * 2 + 1] = inputExtent[i * 2 + 1];
            }
          }
        }
      else
        {
        referenceImage->GetExtent(unionExtent);
        }
      vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
      padder->SetInputData(inputImage);
      padder->SetConstant(backgroundValue);
      padder->SetOutputWholeExtent(unionExtent);
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
  int inputExtentInReferenceFrame[6] = { 0, -1, 0, -1, 0, -1 };
  vtkOrientedImageDataResample::TransformExtent(inputImage->GetExtent(), inputImageToReferenceImageTransform.GetPointer(), inputExtentInReferenceFrame);
  int referenceExtent[6] = { 0, -1, 0, -1, 0, -1 };
  referenceImage->GetExtent(referenceExtent);
  int unionExtent[6] = { 0, -1, 0, -1, 0, -1 };
  if (padImage)
    {
    bool referenceExtentValid = true;
    for (int i = 0; i < 3; ++i)
      {
      if (referenceExtent[i * 2 + 1] < referenceExtent[i * 2])
        {
        referenceExtentValid = false;
        }
      }

    // Make sure input image data fits into the extent.
    for (int i = 0; i < 3; i++)
      {
      if (referenceExtentValid)
        {
        unionExtent[i * 2] = std::min(inputExtentInReferenceFrame[i * 2], referenceExtent[i * 2]);
        unionExtent[i * 2 + 1] = std::max(inputExtentInReferenceFrame[i * 2 + 1], referenceExtent[i * 2 + 1]);
        }
      else
        {
        // Output extent is empty
        unionExtent[i * 2] = inputExtentInReferenceFrame[i * 2];
        unionExtent[i * 2 + 1] = inputExtentInReferenceFrame[i * 2 + 1];
        }
      }
    }
  else
    {
    referenceImage->GetExtent(unionExtent);
    }

  // Return with failure if output extent is empty
  if (unionExtent[0] > unionExtent[1] || unionExtent[2] > unionExtent[3] || unionExtent[4] > unionExtent[5])
    {
    return false;
    }

  // Invert transform for the resampling
  vtkAbstractTransform* referenceImageToInputImageTransform = inputImageToReferenceImageTransform->GetInverse();
  referenceImageToInputImageTransform->Update();

  // Create clone for input image that has an identity geometry
  // TODO: it would be better to use direction matrix that is now available in vtkImageData in latest VTK
  vtkSmartPointer<vtkMatrix4x4> identityMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  identityMatrix->Identity();
  vtkSmartPointer<vtkOrientedImageData> identityInputImage = vtkSmartPointer<vtkOrientedImageData>::New();
  identityInputImage->ShallowCopy(inputImage);
  identityInputImage->SetImageToWorldMatrix(identityMatrix);

  // Perform resampling
  vtkSmartPointer<vtkImageReslice> resliceFilter = vtkSmartPointer<vtkImageReslice>::New();
  resliceFilter->SetInputData(identityInputImage);
  resliceFilter->SetOutputOrigin(0, 0, 0);
  resliceFilter->SetOutputSpacing(1, 1, 1);
  resliceFilter->SetOutputExtent(unionExtent);
  resliceFilter->SetOutputScalarType(inputImage->GetScalarType());
  resliceFilter->SetBackgroundLevel(backgroundValue);
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
  outputImage->SetImageToWorldMatrix(referenceImageToWorldMatrix.GetPointer());

  return true;
}

//-----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::ResampleOrientedImageToReferenceGeometry(vtkOrientedImageData* inputImage, vtkMatrix4x4* referenceToWorldMatrix, vtkOrientedImageData* outputImage, bool linearInterpolation/*=false*/)
{
  if (!inputImage || !referenceToWorldMatrix || !outputImage)
    {
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

  // Calculate output extent in reference frame. Use all bounding box corners
  int outputExtent[6] = {0,-1,0,-1,0,-1};
  vtkOrientedImageDataResample::TransformExtent(effectiveInputExtent, referenceImageToInputImageTransform.GetPointer(), outputExtent);

  // Return with failure if effective output extent is empty
  if ( outputExtent[0] > outputExtent[1]
    || outputExtent[2] > outputExtent[3]
    || outputExtent[4] > outputExtent[5] )
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

  if (image->GetScalarPointer() == nullptr)
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
bool vtkOrientedImageDataResample::CalculateEffectiveExtent(vtkOrientedImageData* image, int effectiveExtent[6], double threshold /*=0.0*/)
{
  if (!image)
    {
    return false;
    }

  switch (image->GetScalarType())
    {
    vtkTemplateMacro(CalculateEffectiveExtentGeneric<VTK_TT>(image, effectiveExtent, threshold));
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
  if (!inputToOutputTransform
    || inputExtent[0] > inputExtent[1] || inputExtent[2] > inputExtent[3] || inputExtent[4] > inputExtent[5])
    {
    outputExtent[0] = 0;
    outputExtent[1] = -1;
    outputExtent[2] = 0;
    outputExtent[3] = -1;
    outputExtent[4] = 0;
    outputExtent[5] = -1;
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
  vtkMath::UninitializeBounds(outputBounds);
  if (!inputToOutputTransform)
    {
    return;
    }
  if (inputBounds[0] > inputBounds[1] || inputBounds[2] > inputBounds[3] || inputBounds[4] > inputBounds[5])
    {
    // invalid input bounds, do not attempt to transform it
    return;
    }

  // Apply transform on all eight corners and determine output extent based on these transformed corners
  vtkBoundingBox outputBoundingBox;
  double outputBoxCorner[3];
  for (int i = 0; i<2; ++i)
    {
    for (int j = 0; j<2; ++j)
      {
      for (int k = 0; k<2; ++k)
        {
        double inputBoxCorner[3] = { inputBounds[i], inputBounds[2 + j], inputBounds[4 + k] };
        inputToOutputTransform->TransformPoint(inputBoxCorner, outputBoxCorner);
        outputBoundingBox.AddPoint(outputBoxCorner);
        }
      }
    }
  outputBoundingBox.GetBounds(outputBounds);
}

//----------------------------------------------------------------------------
void vtkOrientedImageDataResample::TransformOrientedImageDataBounds(vtkOrientedImageData* image, vtkAbstractTransform* transform, double transformedBounds[6])
{
  vtkMath::UninitializeBounds(transformedBounds);
  if (!image || !transform)
    {
    return;
    }

  int* imageExtentCenter = image->GetExtent();
  if (imageExtentCenter[0] > imageExtentCenter[1] || imageExtentCenter[2] > imageExtentCenter[3] || imageExtentCenter[4] > imageExtentCenter[5])
    {
    // empty image, return invalid bounds
    return;
    }

  // Get input image properties
  vtkSmartPointer<vtkMatrix4x4> imageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  image->GetImageToWorldMatrix(imageToWorldMatrix);
  // Add 0.5 to image extents to contain voxel corners
  double imageExtent[6] =
    {
    static_cast<double>(imageExtentCenter[0]) - 0.5,
    static_cast<double>(imageExtentCenter[1]) + 0.5,
    static_cast<double>(imageExtentCenter[2]) - 0.5,
    static_cast<double>(imageExtentCenter[3]) + 0.5,
    static_cast<double>(imageExtentCenter[4]) - 0.5,
    static_cast<double>(imageExtentCenter[5]) + 0.5
    };

  // Append transformed side planes poly data to one model and get bounds
  vtkNew<vtkAppendPolyData> appendPolyData;
  for (int i=0; i<6; i++)
    {
    int normalAxis = i/2; // Axis along which the plane is constant
    double currentPlaneOriginImage[4] = {
      imageExtent[0],
      imageExtent[2],
      imageExtent[4],
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
  return vtkOrientedImageDataResample::PadImageToContainImage(inputImage, containedImage, outputImage, nullptr);
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::PadImageToContainImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* containedImage, vtkOrientedImageData* outputImage, const int extent[6])
{
  if (!inputImage || !containedImage || !outputImage)
    {
    return false;
    }

  // Get transform between input and contained
  vtkNew<vtkTransform> containedImageToInputImageTransform;
  vtkOrientedImageDataResample::GetTransformBetweenOrientedImages(containedImage, inputImage, containedImageToInputImageTransform);

  // Calculate output extent in reference frame for padding if requested. Use all bounding box corners
  int containedImageExtentInInputImageFrame[6] = {0,-1,0,-1,0,-1};
  const int* containedExtent = extent ? extent : containedImage->GetExtent();
  vtkOrientedImageDataResample::TransformExtent(containedExtent, containedImageToInputImageTransform, containedImageExtentInInputImageFrame);

  // Return with failure if output extent is invalid
  if ( containedImageExtentInInputImageFrame[0] > containedImageExtentInInputImageFrame[1]
    || containedImageExtentInInputImageFrame[2] > containedImageExtentInInputImageFrame[3]
    || containedImageExtentInInputImageFrame[4] > containedImageExtentInInputImageFrame[5] )
    {
    return false;
    }

  // Make sure input image data fits into the extent. If padding is disabled, then output extent is the reference extent
  int inputImageExtent[6] = {0,-1,0,-1,0,-1};
  inputImage->GetExtent(inputImageExtent);

  if (inputImageExtent[0] > inputImageExtent[1] || inputImageExtent[2] > inputImageExtent[3] || inputImageExtent[4] > inputImageExtent[5])
    {
    // input image is empty, set it to the required output extent
    outputImage->SetExtent(containedImageExtentInInputImageFrame);
    outputImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    outputImage->GetPointData()->GetScalars()->Fill(0);
    return true;
    }

  // input image is not empty
  int unionExtent[6] = { std::min(containedImageExtentInInputImageFrame[0],inputImageExtent[0]), std::max(containedImageExtentInInputImageFrame[1],inputImageExtent[1]),
                         std::min(containedImageExtentInInputImageFrame[2],inputImageExtent[2]), std::max(containedImageExtentInInputImageFrame[3],inputImageExtent[3]),
                         std::min(containedImageExtentInInputImageFrame[4],inputImageExtent[4]), std::max(containedImageExtentInInputImageFrame[5],inputImageExtent[5]) };

  int outputImageExtent[6] = { 0,-1,0,-1,0,-1 };
  outputImage->GetExtent(outputImageExtent);

  bool paddingRequired = false;
  for (int i = 0; i < 3; ++i)
    {
    if (unionExtent[2 * i] < outputImageExtent[2 * i] || unionExtent[2 * i + 1] > outputImageExtent[2 * i + 1])
      {
      paddingRequired = true;
      break;
      }
    }

  if (paddingRequired)
    {
    // Output may be same as input, so save the geometry information before overwriting it
    vtkNew<vtkMatrix4x4> inputImageToWorldMatrix;
    inputImage->GetImageToWorldMatrix(inputImageToWorldMatrix);

    // Pad image by expansion extent (extents are fitted to the structure, dilate will reach the edge of the image)
    vtkNew<vtkImageConstantPad> padder;
    padder->SetInputData(inputImage);
    padder->SetOutputWholeExtent(unionExtent);
    padder->Update();
    outputImage->ShallowCopy(padder->GetOutput());

    outputImage->SetGeometryFromImageToWorldMatrix(inputImageToWorldMatrix);
    }
  else
    {
    if (inputImage != outputImage)
      {
      outputImage->DeepCopy(inputImage);
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::MergeImage(
    vtkOrientedImageData* inputImage,
    vtkOrientedImageData* imageToAppend,
    vtkOrientedImageData* outputImage,
    int operation,
    const int extent[6]/*=nullptr*/,
    double maskThreshold /*=0*/,
    double fillValue /*=1*/,
    bool *outputModified /*=nullptr*/)
{
  if (outputModified != nullptr)
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
  if (outputModified != nullptr)
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
    double maskThreshold /*=0*/,
    double fillValue /*=1*/)
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
    bool alwaysResample/*=false*/,
    bool linearInterpolation/*=false*/,
    double backgroundColor[4]/*=nullptr*/)
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
    if (linearInterpolation)
      {
      reslice->SetInterpolationModeToLinear();
      }
    else
      {
      reslice->SetInterpolationModeToNearestNeighbor();
      }
    if (backgroundColor)
      {
        reslice->SetBackgroundColor(backgroundColor);
      }
    else
      {
        reslice->SetBackgroundColor(0, 0, 0, 0);
      }
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
  if (image->GetScalarPointer() == nullptr)
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
void vtkOrientedImageDataResample::FillImage(vtkImageData* image, double fillValue, const int extent[6]/*=nullptr*/)
{
  if (!image)
    {
    return;
    }
  if (image->GetPointData() == nullptr || image->GetPointData()->GetScalars() == nullptr)
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

//-----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::ApplyImageMask(vtkOrientedImageData* input, vtkOrientedImageData* mask, double fillValue,
  bool notMask/*=false*/)
{
  if (!input || !mask)
    {
    vtkGenericWarningMacro("vtkOrientedImageDataResample::ApplyImageMask failed: Invalid inputs");
    return false;
    }

  // Make sure mask has the same lattice as the input labelmap
  if (!vtkOrientedImageDataResample::DoGeometriesMatch(input, mask))
    {
    vtkGenericWarningMacro("vtkOrientedImageDataResample::ApplyImageMask failed: input and mask image geometry mismatch");
    return false;
    }

  // Make sure mask has the same extent as the input labelmap
  vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
  padder->SetInputData(mask);
  padder->SetOutputWholeExtent(input->GetExtent());
  padder->Update();
  //mask->DeepCopy(padder->GetOutput());

  // Apply mask
  vtkNew<vtkImageMask> masker;
  masker->SetImageInputData(input);
  //masker->SetMaskInputData(resampledMask);
  masker->SetMaskInputData(padder->GetOutput());
  //masker->SetMaskInputData(mask);
  masker->SetNotMask(notMask);
  masker->SetMaskedOutputValue(fillValue);
  masker->Update();

  // Copy masked input to input
  vtkNew<vtkMatrix4x4> inputImageToWorldMatrix;
  input->GetImageToWorldMatrix(inputImageToWorldMatrix.GetPointer());
  input->ShallowCopy(masker->GetOutput());
  input->SetGeometryFromImageToWorldMatrix(inputImageToWorldMatrix.GetPointer());

  return true;
}

//----------------------------------------------------------------------------
template <class ImageScalarType, class MaskScalarType>
void GetLabelValuesInMaskGeneric2(
  std::vector<int>& foundValues,
  vtkOrientedImageData* binaryLabelmap,
  vtkOrientedImageData* mask,
  const int extent[6]/*=nullptr*/,
  int maskThreshold)
{
  // Compute update extent as intersection of base and mask image extents (extent can be further reduced by specifying a smaller extent)
  int updateExt[6] = { 0, -1, 0, -1, 0, -1 };
  binaryLabelmap->GetExtent(updateExt);
  int* maskExt = mask->GetExtent();
  for (int idx = 0; idx < 3; ++idx)
    {
    if (maskExt[idx * 2] > updateExt[idx * 2])
      {
      updateExt[idx * 2] = maskExt[idx * 2];
      }
    if (extent && extent[idx * 2] > updateExt[idx * 2])
      {
      updateExt[idx * 2] = extent[idx * 2];
      }
    if (maskExt[idx * 2 + 1] < updateExt[idx * 2 + 1])
      {
      updateExt[idx * 2 + 1] = maskExt[idx * 2 + 1];
      }
    if (extent && extent[idx * 2 + 1] < updateExt[idx * 2 + 1])
      {
      updateExt[idx * 2 + 1] = extent[idx * 2 + 1];
      }
    }
  if (updateExt[0] > updateExt[1] || updateExt[2] > updateExt[3] || updateExt[4] > updateExt[5])
    {
    // base and mask images don't intersect, nothing need to be done
    return;
    }

  // Get increments to march through data
  vtkIdType baseIncX = 0;
  vtkIdType baseIncY = 0;
  vtkIdType baseIncZ = 0;
  vtkIdType maskIncX = 0;
  vtkIdType maskIncY = 0;
  vtkIdType maskIncZ = 0;
  binaryLabelmap->GetContinuousIncrements(updateExt, baseIncX, baseIncY, baseIncZ);
  mask->GetContinuousIncrements(updateExt, maskIncX, maskIncY, maskIncZ);
  int maxX = (updateExt[1] - updateExt[0]) * binaryLabelmap->GetNumberOfScalarComponents();
  int maxY = updateExt[3] - updateExt[2];
  int maxZ = updateExt[5] - updateExt[4];
  ImageScalarType* binaryLabelmapPointer = static_cast<ImageScalarType*>(binaryLabelmap->GetScalarPointerForExtent(updateExt));
  MaskScalarType* maskPointer = static_cast<MaskScalarType*>(mask->GetScalarPointerForExtent(updateExt));

  // Make sure the threshold is valid for the modifier scalar range
  MaskScalarType maskThresholdMaskType = 0;
  if (maskThreshold < mask->GetScalarTypeMin())
    {
    maskThresholdMaskType = static_cast<MaskScalarType>(mask->GetScalarTypeMin());
    }
  else if (maskThreshold > mask->GetScalarTypeMax())
    {
    maskThresholdMaskType = static_cast<MaskScalarType>(mask->GetScalarTypeMax());
    }
  else
    {
    maskThresholdMaskType = static_cast<MaskScalarType>(maskThreshold);
    }

  int minimumValue = (int)std::numeric_limits<ImageScalarType>::min();
  int maximumValue = (int)std::numeric_limits<ImageScalarType>::max();
  int rangeSize = maximumValue - minimumValue;

  // Faster to preallocate a vector of the potential values between the minimum and maximum than to generate unique values using std::set
  // Not scalable to any scalar range, so the preallocated array method is only used up to the maximum below.
  size_t maximumSize = 1024 * 1024;
  if (rangeSize * sizeof(int) <= maximumSize)
    {
    std::vector<int> arrayValues(rangeSize, 0);
    for (vtkIdType idxZ = 0; idxZ <= maxZ; idxZ++)
      {
      for (vtkIdType idxY = 0; idxY <= maxY; idxY++)
        {
        for (vtkIdType idxX = 0; idxX <= maxX; idxX++)
          {
          if ((*maskPointer) > maskThresholdMaskType)
            {
            int value = static_cast<int>(*binaryLabelmapPointer);
            int index = value - minimumValue;
            arrayValues[index] = value;
            }
          binaryLabelmapPointer++;
          maskPointer++;
          }
        binaryLabelmapPointer += baseIncY;
        maskPointer += maskIncY;
        }
      binaryLabelmapPointer += baseIncZ;
      maskPointer += maskIncZ;
      }
    for (int value : arrayValues)
      {
      if (value != 0)
        {
        foundValues.push_back(value);
        }
      }
    }
  else
    {
    std::set<int> setValues;
    for (vtkIdType idxZ = 0; idxZ <= maxZ; idxZ++)
      {
      for (vtkIdType idxY = 0; idxY <= maxY; idxY++)
        {
        for (vtkIdType idxX = 0; idxX <= maxX; idxX++)
          {
          if ((*maskPointer) > maskThresholdMaskType)
            {
            int value = static_cast<int>(*binaryLabelmapPointer);
            setValues.insert(value);
            }
          binaryLabelmapPointer++;
          maskPointer++;
          }
        binaryLabelmapPointer += baseIncY;
        maskPointer += maskIncY;
        }
      binaryLabelmapPointer += baseIncZ;
      maskPointer += maskIncZ;
      }
    for (int value : setValues)
      {
      if (value != 0)
        {
        foundValues.push_back(value);
        }
      }
    }
}

//----------------------------------------------------------------------------
template <class ImageScalarType>
void GetLabelValuesInMaskGeneric(
  std::vector<int>& foundValues,
  vtkOrientedImageData* binaryLabelmap,
  vtkOrientedImageData* resampledMask,
  const int extent[6]/*=nullptr*/,
  int maskThreshold)
{
  switch (resampledMask->GetScalarType())
    {
    vtkTemplateMacro((GetLabelValuesInMaskGeneric2<ImageScalarType, VTK_TT>(
      foundValues,
      binaryLabelmap,
      resampledMask,
      extent,
      maskThreshold)));
    default:
      vtkGenericWarningMacro("vtkOrientedImageDataResample::GetLabelValuesInMaskGeneric: Unknown ScalarType");
    }
}

//-----------------------------------------------------------------------------
void vtkOrientedImageDataResample::GetLabelValuesInMask(std::vector<int>& labelValues,
  vtkOrientedImageData* binaryLabelmap, vtkOrientedImageData* mask, const int extent[6]/*=nullptr*/, int maskThreshold/*=0*/)
{
  labelValues.clear();

  int binaryExtent[6] = { 0 };
  binaryLabelmap->GetExtent(binaryExtent);

  int maskExtent[6] = { 0 };
  mask->GetExtent(maskExtent);

  int effectiveExtent[6] = { 0 };
  for (int i = 0; i < 3; ++i)
    {
    effectiveExtent[2 * i] = std::max(binaryExtent[2 * i], maskExtent[2 * i]);
    effectiveExtent[2 * i + 1] = std::min(binaryExtent[2 * i + 1], maskExtent[2 * i + 1]);
    }

  // No labels in mask if effective extent is emptyy
  if (effectiveExtent[0] > effectiveExtent[1] || effectiveExtent[2] > effectiveExtent[3] || effectiveExtent[4] > effectiveExtent[5])
    {
    return;
    }

  vtkNew<vtkOrientedImageData> referenceImage;
  referenceImage->ShallowCopy(mask);
  referenceImage->SetExtent(effectiveExtent);

  vtkSmartPointer<vtkOrientedImageData> resampledBinaryLabelmap;
  if (vtkOrientedImageDataResample::DoGeometriesMatch(binaryLabelmap, referenceImage))
    {
    resampledBinaryLabelmap = binaryLabelmap;
    }
  else
    {
    resampledBinaryLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
    vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(binaryLabelmap, referenceImage, resampledBinaryLabelmap);
    }

  vtkSmartPointer<vtkOrientedImageData> resampledMask;
  if (vtkOrientedImageDataResample::DoGeometriesMatch(mask, referenceImage))
    {
    resampledMask = mask;
    }
  else
    {
    resampledMask = vtkSmartPointer<vtkOrientedImageData>::New();
    vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(mask, referenceImage, resampledMask);
    }

  std::vector<int> foundValues;
  switch (binaryLabelmap->GetScalarType())
    {
    vtkTemplateMacro((GetLabelValuesInMaskGeneric<VTK_TT>(
      foundValues,
      resampledBinaryLabelmap,
      resampledMask,
      extent,
      maskThreshold)));
    default:
      vtkGenericWarningMacro("vtkOrientedImageDataResample::GetLabelValuesInMask: Unknown ScalarType");
    }

  for (int foundValue : foundValues)
    {
    labelValues.push_back(foundValue);
    }
  return;
}

//----------------------------------------------------------------------------
template <class ImageScalarType, class MaskScalarType>
void IsLabelInMaskGeneric2(vtkOrientedImageData* binaryLabelmap, vtkOrientedImageData* mask,
  int extent[6]/*=nullptr*/, int maskThreshold, bool &inMask)
{
  // Compute update extent as intersection of base and mask image extents (extent can be further reduced by specifying a smaller extent)
  int updateExt[6] = { 0, -1, 0, -1, 0, -1 };
  binaryLabelmap->GetExtent(updateExt);
  int* maskExt = mask->GetExtent();
  for (int idx = 0; idx < 3; ++idx)
    {
    if (maskExt[idx * 2] > updateExt[idx * 2])
      {
      updateExt[idx * 2] = maskExt[idx * 2];
      }
    if (extent && extent[idx * 2] > updateExt[idx * 2])
      {
      updateExt[idx * 2] = extent[idx * 2];
      }
    if (maskExt[idx * 2 + 1] < updateExt[idx * 2 + 1])
      {
      updateExt[idx * 2 + 1] = maskExt[idx * 2 + 1];
      }
    if (extent && extent[idx * 2 + 1] < updateExt[idx * 2 + 1])
      {
      updateExt[idx * 2 + 1] = extent[idx * 2 + 1];
      }
    }
  if (updateExt[0] > updateExt[1] || updateExt[2] > updateExt[3] || updateExt[4] > updateExt[5])
    {
    // base and mask images don't intersect, nothing need to be done
    return;
    }

  // Get increments to march through data
  vtkIdType baseIncX = 0;
  vtkIdType baseIncY = 0;
  vtkIdType baseIncZ = 0;
  vtkIdType maskIncX = 0;
  vtkIdType maskIncY = 0;
  vtkIdType maskIncZ = 0;
  binaryLabelmap->GetContinuousIncrements(updateExt, baseIncX, baseIncY, baseIncZ);
  mask->GetContinuousIncrements(updateExt, maskIncX, maskIncY, maskIncZ);
  int maxX = (updateExt[1] - updateExt[0]) * binaryLabelmap->GetNumberOfScalarComponents();
  int maxY = updateExt[3] - updateExt[2];
  int maxZ = updateExt[5] - updateExt[4];
  ImageScalarType* binaryLabelmapPointer = static_cast<ImageScalarType*>(binaryLabelmap->GetScalarPointerForExtent(updateExt));
  MaskScalarType* maskPointer = static_cast<MaskScalarType*>(mask->GetScalarPointerForExtent(updateExt));

  inMask = false;
  for (vtkIdType idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    for (vtkIdType idxY = 0; idxY <= maxY; idxY++)
      {
      for (vtkIdType idxX = 0; idxX <= maxX; idxX++)
        {
        if (*maskPointer > static_cast<MaskScalarType>(maskThreshold) && *binaryLabelmapPointer != (ImageScalarType)0)
          {
          inMask = true;
          return;
          }
        binaryLabelmapPointer++;
        maskPointer++;
        }
      binaryLabelmapPointer += baseIncY;
      maskPointer += maskIncY;
      }
    binaryLabelmapPointer += baseIncZ;
    maskPointer += maskIncZ;
    }
}

//----------------------------------------------------------------------------
template <class ImageScalarType>
void IsLabelInMaskGeneric(vtkOrientedImageData* binaryLabelmap, vtkOrientedImageData* mask,
  int extent[6]/*=nullptr*/, int maskThreshold, bool &inMask)
{
  switch (mask->GetScalarType())
    {
    vtkTemplateMacro((IsLabelInMaskGeneric2<ImageScalarType, VTK_TT>(
      binaryLabelmap,
      mask,
      extent,
      maskThreshold,
      inMask)));
    default:
      vtkGenericWarningMacro("vtkOrientedImageDataResample::IsLabelInMaskGeneric: Unknown ScalarType");
    }
}

//-----------------------------------------------------------------------------
bool vtkOrientedImageDataResample::IsLabelInMask(
  vtkOrientedImageData* binaryLabelmap, vtkOrientedImageData* mask, int extent[6]/*=nullptr*/, int maskThreshold/*=0*/)
{
  vtkNew<vtkTransform> binaryToMaskTransform;
  vtkOrientedImageDataResample::GetTransformBetweenOrientedImages(binaryLabelmap, mask, binaryToMaskTransform);

  int binaryExtent[6] = { 0 };
  binaryLabelmap->GetExtent(binaryExtent);
  vtkOrientedImageDataResample::TransformExtent(binaryExtent, binaryToMaskTransform, binaryExtent);

  int maskExtent[6] = { 0 };
  mask->GetExtent(maskExtent);

  int effectiveExtent[6] = { 0 };
  for (int i = 0; i < 3; ++i)
    {
    effectiveExtent[2 * i] = std::max(binaryExtent[2 * i], maskExtent[2 * i]);
    effectiveExtent[2 * i + 1] = std::min(binaryExtent[2 * i + 1], maskExtent[2 * i + 1]);
    }

  // No labels in mask if effective extent is emptyy
  if (effectiveExtent[0] > effectiveExtent[1] || effectiveExtent[2] > effectiveExtent[3] || effectiveExtent[4] > effectiveExtent[5])
    {
    return false;
    }

  vtkNew<vtkOrientedImageData> referenceImage;
  referenceImage->ShallowCopy(mask);
  referenceImage->SetExtent(effectiveExtent);

  vtkNew<vtkOrientedImageData> resampledBinaryLabelmap;
  vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(binaryLabelmap, referenceImage, resampledBinaryLabelmap);
  vtkNew<vtkOrientedImageData> resampledMask;
  vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(mask, referenceImage, resampledMask);

  bool valueFound = false;
  switch (binaryLabelmap->GetScalarType())
    {
    vtkTemplateMacro((IsLabelInMaskGeneric<VTK_TT>(
      resampledBinaryLabelmap,
      resampledMask,
      extent,
      maskThreshold,
      valueFound)));
    default:
      vtkGenericWarningMacro("vtkOrientedImageDataResample::IsLabelInMask: Unknown ScalarType");
    }
  return valueFound;
}


//----------------------------------------------------------------------------
void vtkOrientedImageDataResample::CastImageForValue(vtkOrientedImageData* image, double value)
{
  if (!image)
    {
    return;
    }

  if (value >= image->GetScalarTypeMin() && value <= image->GetScalarTypeMax())
    {
    // Scalar range can already contain value
    return;
    }

  vtkNew<vtkImageCast> imageCast;
  imageCast->SetInputData(image);
  int scalarType = image->GetScalarType();
  bool typeIsSigned = false;
  switch (scalarType)
    {
    case VTK_CHAR:
      typeIsSigned = true;
      break;
    case VTK_SIGNED_CHAR:
    case VTK_SHORT:
    case VTK_INT:
    case VTK_LONG:
    case VTK_FLOAT:
    case VTK_DOUBLE:
      typeIsSigned = true;
      break;
    case VTK_UNSIGNED_CHAR:
    case VTK_UNSIGNED_INT:
    case VTK_UNSIGNED_SHORT:
    case VTK_UNSIGNED_LONG:
      typeIsSigned = false;
      break;
    }

  if (typeIsSigned)
    {
    if (value > VTK_FLOAT_MAX || value < VTK_FLOAT_MIN)
      {
      scalarType = VTK_DOUBLE;
      }
    else if (value > VTK_LONG_MAX || value < VTK_LONG_MIN)
      {
      scalarType = VTK_FLOAT;
      }
    else if (value > VTK_INT_MAX || value < VTK_INT_MIN)
      {
      scalarType = VTK_LONG;
      }
    else if (value > VTK_SHORT_MAX || value < VTK_SHORT_MIN)
      {
      scalarType = VTK_SHORT;
      }
    }
  else
    {
    if (value > VTK_FLOAT_MAX)
      {
      scalarType = VTK_DOUBLE;
      }
    else if (value > static_cast<double>(VTK_UNSIGNED_LONG_MAX))
      {
      scalarType = VTK_FLOAT;
      }
    else if (value > VTK_UNSIGNED_INT_MAX)
      {
      scalarType = VTK_UNSIGNED_LONG;
      }
    else if (value > VTK_UNSIGNED_SHORT_MAX)
      {
      scalarType = VTK_UNSIGNED_INT;
      }
    else if (value > VTK_UNSIGNED_CHAR_MAX)
      {
      scalarType = VTK_UNSIGNED_SHORT;
      }
    }
  imageCast->SetOutputScalarType(scalarType);
  imageCast->Update();
  image->vtkImageData::ShallowCopy(imageCast->GetOutput());
}
