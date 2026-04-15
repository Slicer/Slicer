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

// Standard includes
#include <algorithm> // For clamp
#include <cmath>     // For log10

// VTK includes
#include "vtkImageMapToWindowLevelColors.h"

// MRML includes
#include "vtkMRML.h"

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
    Logarithmic,        // emphasize low values
    InverseLogarithmic, // emphasize high values
  };

  /// Method for mapping/compressing scalar values to the window range
  /// \sa mappingMode, SetMappingMode
  WindowMappingMode GetMappingMode() const;

  /// Set the method for mapping/compressing scalar values to the window range
  /// \sa GetMappingMode
  void SetMappingMode(vtkImageMapToWindowLevelAddon::WindowMappingMode mappingMode);

  /// Maps inputValue within [rangeMin, rangeMax] to [0, 1].
  /// A negative window (rangeMin > rangeMax) inverts the output direction.
  template <typename T>
  static inline double mapScalarToWindow(T inputValue, double rangeMin, double rangeMax, WindowMappingMode mappingMode) noexcept
  {
    static_assert(std::is_arithmetic_v<T>, "mapScalarToWindow requires numeric input");
    if (rangeMin == rangeMax)
    {
      return 0.0;
    }
    const bool invertWindow = (rangeMin > rangeMax);
    if (invertWindow)
    {
      std::swap(rangeMin, rangeMax);
    }
    const double rangeWidth = rangeMax - rangeMin;
    return computeWindowFactor(static_cast<double>(inputValue), rangeMin, rangeMax, rangeWidth, invertWindow, mappingMode);
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
  template <class T>
  friend void vtkImageMapToWindowLevelAddonExecute(vtkImageMapToWindowLevelAddon*, vtkImageData*, T*, vtkImageData*, unsigned char*, int[6], int);

  /// Core per-pixel mapping. Pre-conditions: lower <= upper, rangeWidth == upper - lower.
  /// Returns a value in [0, 1]; invertWindow flips the direction to [1, 0].
  static inline double computeWindowFactor( //
    double inVal,
    double lower,
    double upper,
    double rangeWidth,
    bool invertWindow,
    WindowMappingMode mappingMode) noexcept
  {
    double factor;
    if (inVal <= lower)
    {
      factor = 0.0;
    }
    else if (inVal >= upper)
    {
      factor = 1.0;
    }
    else
    {
      // lower < inVal < upper, so 0 < t < 1, therefore no clamping needed
      const double t = (inVal - lower) / rangeWidth;
      switch (mappingMode)
      {
        case WindowMappingMode::Logarithmic: factor = std::log10(1.0 + 9.0 * t); break;
        case WindowMappingMode::InverseLogarithmic: factor = 1.0 - std::log10(1.0 + 9.0 * (1.0 - t)); break;
        default: factor = t; break;
      }
    }
    return invertWindow ? 1.0 - factor : factor;
  }

  vtkImageMapToWindowLevelAddon(const vtkImageMapToWindowLevelAddon&) = delete;
  void operator=(const vtkImageMapToWindowLevelAddon&) = delete;
};
#endif
