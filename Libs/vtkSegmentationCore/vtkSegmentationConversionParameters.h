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

==============================================================================*/

#ifndef __vtkSegmentationConversionParameters_h
#define __vtkSegmentationConversionParameters_h

// VTK includes
#include <vtkObject.h>

// STD includes
#include <vector>

// Segmentation includes
#include "vtkSegmentationCoreConfigure.h"

/// \brief Store a list of conversion parameters.
/// \details
/// Stores properties, such as name, description (that may be displayed
/// as tooltip in the GUI) and value (current value or default value)
/// of multiple segmentation conversion parameters.
class vtkSegmentationCore_EXPORT vtkSegmentationConversionParameters : public vtkObject
{
public:
  static vtkSegmentationConversionParameters* New();
  vtkTypeMacro(vtkSegmentationConversionParameters, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// @{
  /// Get/Set parameter name
  std::string GetName(int index) VTK_EXPECTS(0 <= index && index < GetNumberOfParameters());
  void SetName(int index, const std::string& name) VTK_EXPECTS(0 <= index && index < GetNumberOfParameters());
  /// @}

  /// @{
  /// Get/Set parameter description
  std::string GetDescription(int index) VTK_EXPECTS(0 <= index && index < GetNumberOfParameters());
  std::string GetDescription(const std::string& name);
  void SetDescription(int index, const std::string& description)
    VTK_EXPECTS(0 <= index && index < GetNumberOfParameters());
  void SetDescription(const std::string& name, const std::string& description);
  /// @}

  /// @{
  /// Get/Set parameter default value
  std::string GetValue(int index) VTK_EXPECTS(0 <= index && index < GetNumberOfParameters());
  std::string GetValue(const std::string& name);
  double GetValueAsDouble(const std::string& name);
  int GetValueAsInt(const std::string& name);
  void SetValue(int index, const std::string& value) VTK_EXPECTS(0 <= index && index < GetNumberOfParameters());
  void SetValue(const std::string& name, const std::string& value);
  /// @}

  /// Get parameter index from name.
  /// Returns -1 if parameter is not found.
  int GetIndexFromName(const std::string name);

  /// Return number of parameters
  int GetNumberOfParameters();

  /// Delete all parameters
  void RemoveAllParameters();

  /// Delete parameter
  void RemoveParameter(int index) VTK_EXPECTS(0 <= index && index < GetNumberOfParameters());

  /// Set a conversion parameter
  int SetParameter(const std::string& name, const std::string& value, const std::string& description = "");

  /// Replace parameters with content of another parameter list
  void DeepCopy(vtkSegmentationConversionParameters* source);

  /// Replace parameters with content of another parameter list
  void CopyParameter(vtkSegmentationConversionParameters* source, int sourceIndex);

protected:
  struct ConversionParameterType
  {
    std::string Name;
    std::string Description;
    std::string Value;
  };

  std::vector<ConversionParameterType> ParameterList;

protected:
  vtkSegmentationConversionParameters();
  ~vtkSegmentationConversionParameters() override;

private:
  vtkSegmentationConversionParameters(const vtkSegmentationConversionParameters&) = delete;
  void operator=(const vtkSegmentationConversionParameters&) = delete;
};

#endif
