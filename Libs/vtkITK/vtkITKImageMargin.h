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
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __vtkITKImageMargin_h
#define __vtkITKImageMargin_h

#include "vtkITK.h"
#include "vtkSimpleImageToImageFilter.h"

/// \brief ITK-based utilities for manipulating connected regions in label maps.
/// Limitation: The filter does not work correctly with input volume that has
/// unsigned long scalar type on Linux and MacOSX.
///
class VTK_ITK_EXPORT vtkITKImageMargin : public vtkSimpleImageToImageFilter
{
 public:
  static vtkITKImageMargin *New();
  vtkTypeMacro(vtkITKImageMargin, vtkSimpleImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// The background value that is considered "outside" the image.
  /// Default value is 0.
  vtkGetMacro(BackgroundValue, int);
  vtkSetMacro(BackgroundValue, int);

  /// If the margin calculation should be calculated using image spacing, or using a set number of voxels
  vtkGetMacro(CalculateMarginInMM, bool);
  vtkSetMacro(CalculateMarginInMM, bool);
  vtkBooleanMacro(CalculateMarginInMM, bool);

  /// The distance of the outer margin from the surface.
  /// Positive values are outside the foreground, and negative values are inside the foreground.
  /// Default value is 0.0.
  vtkGetMacro(OuterMarginMM, double);
  vtkSetMacro(OuterMarginMM, double);

  /// The distance of the inner margin from the surface.
  /// Positive values are outside the foreground, and negative values are inside the foreground.
  /// Default value is negative infinity.
  vtkGetMacro(InnerMarginMM, double);
  vtkSetMacro(InnerMarginMM, double);

  /// The number of voxels to the outer margin from the surface.
  /// Positive values are outside the foreground, and negative values are inside the foreground.
  /// Default value is 0.0.
  vtkGetMacro(OuterMarginVoxels, double);
  vtkSetMacro(OuterMarginVoxels, double);

  /// The number of voxels to the inner margin from the surface.
  /// Positive values are outside the foreground, and negative values are inside the foreground.
  /// Default value is negative infinity.
  vtkGetMacro(InnerMarginVoxels, double);
  vtkSetMacro(InnerMarginVoxels, double);

protected:
  int BackgroundValue{0};
  bool CalculateMarginInMM{true};
  double OuterMarginMM{0.0};
  double InnerMarginMM{0.0};
  double OuterMarginVoxels{0.0};
  double InnerMarginVoxels{0.0};

protected:
  vtkITKImageMargin();
  ~vtkITKImageMargin() override;

  void SimpleExecute(vtkImageData* input, vtkImageData* output) override;

private:
  vtkITKImageMargin(const vtkITKImageMargin&) = delete;
  void operator=(const vtkITKImageMargin&) = delete;
};

#endif
