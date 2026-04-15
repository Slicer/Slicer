/*==============================================================================

  Copyright (c) TBD

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Alex Allphin, PhD at Revvity.

==============================================================================*/
/**
 * @class   vtkImageMapToWindowLevelAddon
 * @brief   Subclass of vtkImageMapToWindowLevelColors that adds the option to compress values within the window logarithmically.
 *
 * This class is a subclass of vtkImageMapToWindowLevelColors that adds optional logarithmic window compression.
 *
 * In the long term, this class may be removed and the functionality added to vtkImageMapToWindowLevelColors, if it is found to be useful.
 */

#ifndef vtkImageMapToWindowLevelAddon_h
#define vtkImageMapToWindowLevelAddon_h

// VTK includes
#include "vtkImageMapToWindowLevelColors.h"

// MRML includes
#include "vtkMRML.h"

#include <algorithm> // For clamp

class VTK_MRML_EXPORT vtkImageMapToWindowLevelAddon : public vtkImageMapToWindowLevelColors
{
public:
  vtkTypeMacro(vtkImageMapToWindowLevelAddon, vtkImageMapToWindowLevelColors);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkImageMapToWindowLevelAddon* New();

  /// Window Scalar Mapping Modes
  enum WindowMappingMode
  {
    Linear = 0,
    LogCompressLowValues,
    LogCompressHighValues,
  };

  /// Method for mapping/compressing scalar values to the window range
  /// \sa mappingMode, SetMappingMode
  WindowMappingMode GetMappingMode() const;

  /// Set the method for mapping/compressing scalar values to the window range
  /// \sa GetMappingMode
  void SetMappingMode(vtkImageMapToWindowLevelAddon::WindowMappingMode mappingMode);

  /// Function for scalar mapping calculations
  /// Returns a value in [0, 1]. Mode and inversion affect how input values are mapped within that range.
  template <typename T>
  static inline double mapScalarToWindow(T inputValue, T rangeMin, T rangeMax, WindowMappingMode mappingMode) noexcept
  {
    static_assert(std::is_arithmetic_v<T>, "mapScalarToWindow requires numeric input");
    double inVal = static_cast<double>(inputValue);
    double minVal = static_cast<double>(rangeMin);
    double maxVal = static_cast<double>(rangeMax);
    const double rangeWidth = maxVal - minVal;
    bool needToCalculate = true;

    if (rangeWidth <= 0)
    {
      return 0.0;
    }

    if (inVal <= minVal)
    {
      return 0.0;
    }

    if (inVal >= maxVal)
    {
      return 1.0;
    }

    switch (mappingMode)
    {
      case WindowMappingMode::Linear: return std::clamp((inVal - minVal) / rangeWidth, 0.0, 1.0);
      case WindowMappingMode::LogCompressLowValues: return std::log10(std::clamp(1.0 + 9.0 * (inVal - minVal) / rangeWidth, 1.0, 10.0));
      case WindowMappingMode::LogCompressHighValues: return 1.0 - std::log10(std::clamp(1.0 + 9.0 * (maxVal - inVal) / rangeWidth, 1.0, 10.0));
    }
    return 0.0;
  }

protected:
  vtkImageMapToWindowLevelAddon();
  ~vtkImageMapToWindowLevelAddon() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  void ThreadedRequestData(vtkInformation* request,
                           vtkInformationVector** inputVector,
                           vtkInformationVector* outputVector,
                           vtkImageData*** inData,
                           vtkImageData** outData,
                           int outExt[6],
                           int id) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  WindowMappingMode MappingMode;

private:
  vtkImageMapToWindowLevelAddon(const vtkImageMapToWindowLevelAddon&) = delete;
  void operator=(const vtkImageMapToWindowLevelAddon&) = delete;
};
#endif
