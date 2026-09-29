/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLIOOptionsDescription_h
#define __vtkMRMLIOOptionsDescription_h

#include "vtkMRMLLogicExport.h"

#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkVariant.h>

#include <string>
#include <vector>

class vtkMRMLIOProperties;

/// Description of the options of a file reader or writer (properties that the user can set, such as "labelmap"),
/// which user interfaces use to display widgets for setting the options.
/// Readers and writers add options in vtkMRMLFileIOHandler::GetOptionsDescription() using the Add...Option() methods.
/// User interfaces read the options using the GetNthOption...() methods, or as JSON (ToJSON()).
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#reader-and-writer-options
class VTK_MRML_LOGIC_EXPORT vtkMRMLIOOptionsDescription : public vtkObject
{
public:
  static vtkMRMLIOOptionsDescription* New();
  vtkTypeMacro(vtkMRMLIOOptionsDescription, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Context and current values of options (for example, edited by the user). These values take precedence over defaults.
  void SetProperties(vtkMRMLIOProperties* properties);
  vtkMRMLIOProperties* GetProperties();

  /// Default values specified by the application (for example, from user settings).
  void SetDefaults(vtkMRMLIOProperties* defaults);
  vtkMRMLIOProperties* GetDefaults();

  /// Get the file name from the properties. If multiple file names are specified then the first one is returned.
  std::string GetFileName();
  /// Get all file names from the properties.
  std::vector<std::string> GetFileNames();

  //@{
  /// Add an option. If an option already exists for the property then it is replaced.
  void AddBoolOption(const std::string& property, const std::string& label, const std::string& toolTip, bool defaultValue);
  void AddIntOption(const std::string& property, const std::string& label, const std::string& toolTip, int defaultValue, int minimum, int maximum);
  void AddDoubleOption(const std::string& property, //
                       const std::string& label,
                       const std::string& toolTip,
                       double defaultValue,
                       double minimum,
                       double maximum,
                       int decimals);
  void AddStringOption(const std::string& property, const std::string& label, const std::string& toolTip, const std::string& defaultValue);
  void AddStringListOption(const std::string& property,
                           const std::string& label,
                           const std::string& toolTip,
                           const std::vector<std::string>& defaultValue,
                           const std::string& separator = ";");
  /// Add an option that can have one of the values added by AddEnumChoice().
  void AddEnumOption(const std::string& property, const std::string& label, const std::string& toolTip, const vtkVariant& defaultValue);
  void AddEnumChoice(const std::string& property, const vtkVariant& value, const std::string& label);
  /// Add an option for selecting a MRML node. The value is the node ID.
  void AddNodeOption(const std::string& property,
                     const std::string& label,
                     const std::string& toolTip,
                     const std::string& defaultNodeID,
                     const std::vector<std::string>& nodeClasses,
                     bool noneEnabled);
  //@}

  //@{
  /// Set additional attributes of an option.
  void SetOptionEnabled(const std::string& property, bool enabled);
  void SetOptionVisible(const std::string& property, bool visible);
  /// Hint for choosing a specific widget (for example "colorTable").
  void SetOptionWidget(const std::string& property, const std::string& widget);
  /// Show nodes that are hidden from editors in node selectors.
  void SetOptionShowHidden(const std::string& property, bool showHidden);
  //@}

  /// Number of options.
  int GetNumberOfOptions();
  bool HasOption(const std::string& property);
  /// Remove all options.
  void RemoveAllOptions();

  /// Effective value of an option (from properties, defaults, or the default specified for the option).
  /// Returns invalid variant if the option is not found. For string list options, items are joined using the separator.
  vtkVariant GetOptionValue(const std::string& property);
  /// Effective value of a boolean option. Returns defaultValue if the option is not found.
  bool GetBoolOptionValue(const std::string& property, bool defaultValue = false);

  /// Get the effective values of all options. Options that have empty string or string list values are not included.
  void GetOptionValues(vtkMRMLIOProperties* values);

  /// Get index of the option that sets the specified property. Returns -1 if not found.
  int GetOptionIndex(const std::string& property);

  //@{
  /// Get attributes of the option at the specified index (0 <= index < GetNumberOfOptions()).
  /// User interfaces use these methods to display widgets for the options.
  /// If the index is out of range then an error is logged and an empty value is returned.
  std::string GetNthOptionProperty(int index);
  /// Option type: "bool", "int", "double", "string", "stringList", "enum", or "node".
  std::string GetNthOptionType(int index);
  std::string GetNthOptionLabel(int index);
  std::string GetNthOptionToolTip(int index);
  /// Hint for choosing a specific widget (for example "colorTable"). Empty if not specified.
  std::string GetNthOptionWidget(int index);
  bool GetNthOptionEnabled(int index);
  bool GetNthOptionVisible(int index);
  /// Effective value of the option (from properties, defaults, or the default specified for the option).
  /// Boolean values are returned as integer (0 or 1), enum values keep the type of the choice values.
  /// For string list options, use GetNthOptionStringListValue() instead.
  vtkVariant GetNthOptionValue(int index);
  /// Effective value of a string list option.
  std::vector<std::string> GetNthOptionStringListValue(int index);
  /// Separator of items of a string list option.
  std::string GetNthOptionSeparator(int index);
  /// Returns true if the option has a range (int and double options).
  bool GetNthOptionHasRange(int index);
  double GetNthOptionMinimum(int index);
  double GetNthOptionMaximum(int index);
  /// Number of decimals of a double option. Returns -1 if not specified.
  int GetNthOptionDecimals(int index);
  /// Choices of an enum option.
  int GetNthOptionNumberOfChoices(int index);
  vtkVariant GetNthOptionChoiceValue(int index, int choiceIndex);
  std::string GetNthOptionChoiceLabel(int index, int choiceIndex);
  /// Node classes, whether "None" can be selected, and whether hidden nodes are shown (node options).
  std::vector<std::string> GetNthOptionNodeClasses(int index);
  bool GetNthOptionNoneEnabled(int index);
  bool GetNthOptionShowHidden(int index);
  //@}

  /// Get the description of all options as JSON.
  std::string ToJSON();

protected:
  vtkMRMLIOOptionsDescription();
  ~vtkMRMLIOOptionsDescription() override;

private:
  vtkMRMLIOOptionsDescription(const vtkMRMLIOOptionsDescription&) = delete;
  void operator=(const vtkMRMLIOOptionsDescription&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
