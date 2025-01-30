/*==============================================================================

  Program: 3D Slicer

  Copyright(c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLAbstractVolumeResampler_h
#define __vtkMRMLAbstractVolumeResampler_h

// MRMLLogic includes
#include "vtkMRMLLogicExport.h"

// MRML includes
class vtkMRMLTransformNode;
class vtkMRMLVectorVolumeNode;
class vtkMRMLVolumeNode;

// VTK includes
#include <vtkObject.h>

/// \brief Base class for volume resampler.
///
/// Sub-classes must implement actual resampling.
class VTK_MRML_LOGIC_EXPORT vtkMRMLAbstractVolumeResampler : public vtkObject
{
public:
  vtkTypeMacro(vtkMRMLAbstractVolumeResampler, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum
  {
    InterpolationTypeUndefined,
    InterpolationTypeNearestNeighbor,
    InterpolationTypeLinear,
    InterpolationTypeWindowedSinc,
    InterpolationTypeBSpline,
    InterpolationType_Last // must be last
  };

  enum
  {
    WindowedSincFunctionUndefined,
    WindowedSincFunctionHamming,
    WindowedSincFunctionCosine,
    WindowedSincFunctionWelch,
    WindowedSincFunctionLanczos,
    WindowedSincFunctionBlackman,
    WindowedSincFunction_Last // must be last
  };

  typedef std::map<std::string, std::string> ResamplingParameters;

  virtual bool Resample(vtkMRMLVolumeNode* inputVolume,
                        vtkMRMLVolumeNode* outputVolume,
                        vtkMRMLTransformNode* resamplingTransform,
                        vtkMRMLVolumeNode* referenceVolume,
                        int interpolationType,
                        int windowedSincFunction,
                        const ResamplingParameters& resamplingParameter) = 0;

  /// @{
  /// Get resampling parameter value
  static std::string GetParameterValue(const ResamplingParameters& parameters, const std::string& name);
  static double GetParameterValueAsDouble(const ResamplingParameters& parameters, const std::string& name);
  static int GetParameterValueAsInt(const ResamplingParameters& parameters, const std::string& name);
  /// }@

  /// @{
  /// Set resampling parameter value
  static void SetParameterValue(ResamplingParameters& parameters, const std::string& name, const std::string& value);
  static void SetParameterValueAsDouble(ResamplingParameters& parameters, const std::string& name, double value);
  static void SetParameterValueAsInt(ResamplingParameters& parameters, const std::string& name, int value);
  /// }@

protected:
  vtkMRMLAbstractVolumeResampler() = default;
  ~vtkMRMLAbstractVolumeResampler() override = default;
  vtkMRMLAbstractVolumeResampler(const vtkMRMLAbstractVolumeResampler&) = delete;
  void operator=(const vtkMRMLAbstractVolumeResampler&) = delete;
};

#endif
