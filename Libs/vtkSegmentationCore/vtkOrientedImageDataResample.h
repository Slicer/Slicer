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

#ifndef __vtkOrientedImageDataResample_h
#define __vtkOrientedImageDataResample_h

// Segmentation includes
#include "vtkSegmentationCoreConfigure.h"

// VTK includes
#include "vtkObject.h"

// std includes
#include <vector>

class vtkImageData;
class vtkMatrix4x4;
class vtkOrientedImageData;
class vtkTransform;
class vtkAbstractTransform;

/// \ingroup SegmentationCore
/// \brief Utility functions for resampling oriented image data
class vtkSegmentationCore_EXPORT vtkOrientedImageDataResample : public vtkObject
{
public:
  static vtkOrientedImageDataResample *New();
  vtkTypeMacro(vtkOrientedImageDataResample,vtkObject);

  enum
    {
    OPERATION_MINIMUM,
    OPERATION_MAXIMUM,
    OPERATION_MASKING
    };

  /// Resample an oriented image data to match the geometry of a reference geometry matrix.
  /// Origin and dimensions are determined from the contents of the input image.
  /// \param inputImage Oriented image to resample
  /// \param referenceGeometryMatrix Matrix containing the desired geometry
  /// \param outputImage Output image
  /// \param linearInterpolation True if linear interpolation is requested (fractional labelmap), or false for nearest neighbor (binary labelmap). Default is false.
  /// \return Success flag
  static bool ResampleOrientedImageToReferenceGeometry(vtkOrientedImageData* inputImage, vtkMatrix4x4* referenceGeometryMatrix, vtkOrientedImageData* outputImage, bool linearInterpolation=false);

  /// Resample an oriented image data to match the geometry of a reference oriented image data
  /// \param inputImage Oriented image to resample
  /// \param referenceImage Oriented image containing the desired geometry
  /// \param outputImage Output image
  /// \param linearInterpolation True if linear interpolation is requested (fractional labelmap), or false for nearest neighbor (binary labelmap). Default is false.
  /// \param padImage If enabled then it is made sure that the input image's extent fits into the resampled reference image, so if part of the extent is transformed
  ///          to be outside the reference extent, then it is padded. Disabled by default.
  /// \param inputImageTransform If specified then inputImage will be transformed with inputImageTransform before resampled into referenceImage.
  /// \return Success flag
  static bool ResampleOrientedImageToReferenceOrientedImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* referenceImage, vtkOrientedImageData* outputImage, bool linearInterpolation=false, bool padImage=false, vtkAbstractTransform* inputImageTransform=nullptr, double backgroundValue=0);

  /// Transform an oriented image data using a transform that can be linear or non-linear.
  /// Linear: simply multiply the geometry matrix with the applied matrix, extent stays the same
  /// Non-linear: calculate new extents and change only the extents when applying deformable transform
  /// \param image Oriented image to transform
  /// \param transform Input transform
  /// \param geometryOnly Only the geometry of the image is changed according to the transform if this flag is turned on.
  ///          This flag only has an effect if the transform is non-linear, in which case only the extent is changed. Off by default
  /// \param alwaysResample If on, then image data will be resampled even if the applied transform is linear
  static void TransformOrientedImage(vtkOrientedImageData* image, vtkAbstractTransform* transform, bool geometryOnly=false, bool alwaysResample=false, bool linearInterpolation=false, double backgroundColor[4]=nullptr);

  /// Combines the inputImage and imageToAppend into a new image by max/min operation. The extent will be the union of the two images.
  /// Extent can be specified to restrict imageToAppend's extent to a smaller region.
  /// inputImage and imageToAppend must have the same geometry, but they may have different extents.
  static bool MergeImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* imageToAppend, vtkOrientedImageData* outputImage, int operation,
    const int extent[6]=nullptr, double maskThreshold = 0, double fillValue = 1, bool *outputModified=nullptr);

  /// Modifies inputImage in-place by combining with modifierImage using max/min operation.
  /// The extent will remain unchanged.
  /// Extent can be specified to restrict modifierImage's extent to a smaller region.
  /// inputImage and modifierImage must have the same geometry (origin, spacing, directions) and scalar type, but they may have different extents.
  static bool ModifyImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* modifierImage, int operation,
    const int extent[6] = nullptr, double maskThreshold = 0, double fillValue = 1);

  /// Copy image with clipping to the specified extent
  static bool CopyImage(vtkOrientedImageData* imageToCopy, vtkOrientedImageData* outputImage, const int extent[6]=nullptr);

  /// Prints image information. Does not print lots of irrelevant information that default PrintSelf would print.
  static void PrintImageInformation(vtkImageData* imageData, ostream& os, vtkIndent indent);

  /// Fills an image with the specified value
  /// \param extent The whole extent is filled if extent is not specified
  static void FillImage(vtkImageData* image, double fillValue, const int extent[6]=nullptr);

public:
  /// Calculate effective extent of an image: the IJK extent where non-zero voxels are located
  static bool CalculateEffectiveExtent(vtkOrientedImageData* image, int effectiveExtent[6], double threshold = 0.0);

  /// Determine if geometries of two oriented image data objects match.
  /// Origin, spacing and direction are considered, extent is not.
  static bool DoGeometriesMatch(vtkOrientedImageData* image1, vtkOrientedImageData* image2);

  /// Determine if extents of two oriented image data objects match.
  static bool DoExtentsMatch(vtkOrientedImageData* image1, vtkOrientedImageData* image2);

  /// Determine if geometries of two oriented image data objects match.
  /// Only considers spacing and orientation, origin and extent may be different!
  static bool DoGeometriesMatchIgnoreOrigin(vtkOrientedImageData* image1, vtkOrientedImageData* image2);

  /// Transform input extent to determine output extent of an image. Use all bounding box corners,
  /// may miss part of the extent in case of non-linear transforms are used.
  static void TransformExtent(const int inputExtent[6], vtkAbstractTransform* inputToOutputTransform, int outputExtent[6]);

  /// Transform input bounds to determine output bounds. Use all bounding box corners,
  /// may miss part of the extent in case of non-linear transforms are used.
  static void TransformBounds(const double inputBounds[6], vtkAbstractTransform* inputToOutputTransform, double outputBounds[6]);

  /// Transform bounds of oriented image data using a linear or non-linear transform
  static void TransformOrientedImageDataBounds(vtkOrientedImageData* image, vtkAbstractTransform* transform, double transformedBounds[6]);

  /// Compare the values (with tolerance) between two 4x4 matrices
  /// \param lhs Left-hand side matrix to compare
  /// \param rhs Right-hand side matrix to compare
  static bool IsEqual(vtkMatrix4x4* lhs, vtkMatrix4x4* rhs);

  /// Compare two floating point numbers within tolerance
  static bool AreEqualWithTolerance(double a, double b) { return fabs(a - b) < 0.0001; };

  /// Calculate transform between two oriented image data
  static bool GetTransformBetweenOrientedImages(vtkOrientedImageData* image1, vtkOrientedImageData* image2, vtkTransform* image1ToImage2Transform);

  /// Pad an image to entirely contain another image using custom extent to contain
  static bool PadImageToContainImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* containedImage, vtkOrientedImageData* outputImage, const int extent[6]);
  /// Pad an image to entirely contain another image
  static bool PadImageToContainImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* containedImage, vtkOrientedImageData* outputImage);

  /// Determine if a transform is linear and return it if it is. A simple downcast is not enough, as the transform may be
  /// a general transform, which can be linear if the concatenation it contains consist of all linear transforms.
  /// \param transform Input transform to assess
  /// \param linearTransform Output transform in case transform is linear
  /// \return True if input is linear, false otherwise.
  static bool IsTransformLinear(vtkAbstractTransform* transform, vtkTransform* linearTransform);

  /// Determine if a transform matrix contains shear
  static bool DoesTransformMatrixContainShear(vtkMatrix4x4* matrix);

  /// Apply mask image on an input image
  /// \param input Input image to apply the mask on
  /// \param mask Mask to apply
  /// \param notMask If on, the mask is passed through a boolean not before it is used to mask the image.
  ///   The effect is to pass the input pixels where the mask is zero, and replace the pixels where the
  ///   mask is non zero
  static bool ApplyImageMask(vtkOrientedImageData* input, vtkOrientedImageData* mask, double fillValue, bool notMask = false);

  /// Get the values contained in the labelmap under the mask
  /// \param binaryLabelmap Input image to get values from
  /// \param mask Mask image to get values under
  /// \param extent Can be set to restrict the examined extent to a smaller region.
  ///  If nullptr, the extent will be the overlapping extent between the label and mask.
  /// \param labelValues The values found in the binary labelmap underneath the mask
  /// \param maskThreshold Threshold value for the mask. Values above this threshold are considered to be under the mask
  static void GetLabelValuesInMask(std::vector<int>& labelValues, vtkOrientedImageData* binaryLabelmap, vtkOrientedImageData* mask,
    const int extent[6]=nullptr, int maskThreshold = 0);

  /// Determine if there is a non-zero value in the labelmap under the mask
  /// \param binaryLabelmap Input image to get values from
  /// \param mask Mask image to get values under
  /// \param extent Can be set to restrict the examined extent to a smaller region.
  ///  If nullptr, the extent will be the overlapping extent between the label and mask.
  /// \param maskThreshold Threshold value for the mask. Values above this threshold are considered to be under the mask
  static bool IsLabelInMask(vtkOrientedImageData* binaryLabelmap, vtkOrientedImageData* mask,
    int extent[6]=nullptr, int maskThreshold=0);

  /// Cast the data type of the image to be able to contain the specified value
  /// \param image Image to convert
  /// \param value Value that should be representable by the image data type
  static void CastImageForValue(vtkOrientedImageData* image, double value);

protected:
  vtkOrientedImageDataResample();
  ~vtkOrientedImageDataResample() override;

private:
  vtkOrientedImageDataResample(const vtkOrientedImageDataResample&) = delete;
  void operator=(const vtkOrientedImageDataResample&) = delete;
};

#endif
