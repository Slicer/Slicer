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

#include "vtkMRMLIOOptionsDescription.h"

#include "vtkMRMLIOProperties.h"

#include <vtkMRMLJsonElement.h>

#include <vtkNew.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLIOOptionsDescription);

namespace
{
//----------------------------------------------------------------------------
struct Option
{
  std::string Property;
  std::string Type;
  std::string Label;
  std::string ToolTip;
  std::string Widget;
  std::string Separator;
  vtkVariant DefaultValue;
  std::vector<std::string> DefaultListValue;
  bool Enabled{ true };
  bool Visible{ true };
  bool NoneEnabled{ false };
  bool ShowHidden{ false };
  bool HasRange{ false };
  double Minimum{ 0.0 };
  double Maximum{ 0.0 };
  int Decimals{ -1 };
  std::vector<std::pair<vtkVariant, std::string>> Choices;
  std::vector<std::string> NodeClasses;
};

//----------------------------------------------------------------------------
std::string joinStrings(const std::vector<std::string>& values, const std::string& separator)
{
  std::string result;
  for (size_t i = 0; i < values.size(); ++i)
  {
    result += (i > 0 ? separator + " " : "") + values[i];
  }
  return result;
}

} // namespace

//----------------------------------------------------------------------------
class vtkMRMLIOOptionsDescription::vtkInternal
{
public:
  Option* FindOption(const std::string& property)
  {
    for (Option& option : this->Options)
    {
      if (option.Property == property)
      {
        return &option;
      }
    }
    return nullptr;
  }

  /// Get option by index. Logs an error and returns nullptr if the index is out of range.
  const Option* GetNthOption(vtkMRMLIOOptionsDescription* self, int index, const char* methodName)
  {
    if (index < 0 || index >= static_cast<int>(this->Options.size()))
    {
      vtkErrorWithObjectMacro(self, << methodName << " failed: invalid option index " << index);
      return nullptr;
    }
    return &this->Options[index];
  }

  Option& AddOption(const std::string& property, const std::string& type, const std::string& label, const std::string& toolTip)
  {
    Option* existing = this->FindOption(property);
    if (existing)
    {
      *existing = Option();
    }
    else
    {
      this->Options.emplace_back();
      existing = &this->Options.back();
    }
    existing->Property = property;
    existing->Type = type;
    existing->Label = label;
    existing->ToolTip = toolTip;
    return *existing;
  }

  /// Get value specified in properties or defaults. Returns nullptr if not specified.
  vtkMRMLIOProperties* GetSource(const std::string& property)
  {
    if (this->Properties && this->Properties->HasProperty(property))
    {
      return this->Properties;
    }
    if (this->Defaults && this->Defaults->HasProperty(property))
    {
      return this->Defaults;
    }
    return nullptr;
  }

  /// Effective value of an option
  vtkVariant GetValue(const Option& option)
  {
    vtkMRMLIOProperties* source = this->GetSource(option.Property);
    if (option.Type == "stringList")
    {
      std::vector<std::string> values = source ? source->GetStringListProperty(option.Property) : option.DefaultListValue;
      return vtkVariant(joinStrings(values, option.Separator));
    }
    if (!source)
    {
      return option.DefaultValue;
    }
    if (option.Type == "bool")
    {
      return vtkVariant(source->GetBoolProperty(option.Property) ? 1 : 0);
    }
    if (option.Type == "int")
    {
      return vtkVariant(source->GetIntProperty(option.Property));
    }
    if (option.Type == "double")
    {
      return vtkVariant(source->GetDoubleProperty(option.Property));
    }
    if (option.Type == "enum" && option.DefaultValue.IsNumeric())
    {
      // keep the value type of the choices
      if (option.DefaultValue.IsFloat() || option.DefaultValue.IsDouble())
      {
        return vtkVariant(source->GetDoubleProperty(option.Property));
      }
      return vtkVariant(source->GetIntProperty(option.Property));
    }
    return vtkVariant(source->GetStringProperty(option.Property));
  }

  std::vector<std::string> GetListValue(const Option& option)
  {
    vtkMRMLIOProperties* source = this->GetSource(option.Property);
    return source ? source->GetStringListProperty(option.Property) : option.DefaultListValue;
  }

  std::vector<Option> Options;
  vtkSmartPointer<vtkMRMLIOProperties> Properties;
  vtkSmartPointer<vtkMRMLIOProperties> Defaults;
};

//----------------------------------------------------------------------------
vtkMRMLIOOptionsDescription::vtkMRMLIOOptionsDescription()
{
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkMRMLIOOptionsDescription::~vtkMRMLIOOptionsDescription()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Options: " << this->ToJSON() << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::SetProperties(vtkMRMLIOProperties* properties)
{
  this->Internal->Properties = properties;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLIOProperties* vtkMRMLIOOptionsDescription::GetProperties()
{
  return this->Internal->Properties;
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::SetDefaults(vtkMRMLIOProperties* defaults)
{
  this->Internal->Defaults = defaults;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLIOProperties* vtkMRMLIOOptionsDescription::GetDefaults()
{
  return this->Internal->Defaults;
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOOptionsDescription::GetFileName()
{
  std::vector<std::string> fileNames = this->GetFileNames();
  return fileNames.empty() ? std::string() : fileNames[0];
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLIOOptionsDescription::GetFileNames()
{
  if (!this->Internal->Properties)
  {
    return std::vector<std::string>();
  }
  std::vector<std::string> fileNames = this->Internal->Properties->GetStringListProperty("fileName");
  if (fileNames.empty() || (fileNames.size() == 1 && fileNames[0].empty()))
  {
    fileNames = this->Internal->Properties->GetStringListProperty("fileNames");
  }
  return fileNames;
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::AddBoolOption(const std::string& property, const std::string& label, const std::string& toolTip, bool defaultValue)
{
  Option& option = this->Internal->AddOption(property, "bool", label, toolTip);
  option.DefaultValue = vtkVariant(defaultValue ? 1 : 0);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::AddIntOption(const std::string& property, //
                                               const std::string& label,
                                               const std::string& toolTip,
                                               int defaultValue,
                                               int minimum,
                                               int maximum)
{
  Option& option = this->Internal->AddOption(property, "int", label, toolTip);
  option.DefaultValue = vtkVariant(defaultValue);
  option.HasRange = true;
  option.Minimum = minimum;
  option.Maximum = maximum;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::AddDoubleOption(const std::string& property,
                                                  const std::string& label,
                                                  const std::string& toolTip,
                                                  double defaultValue,
                                                  double minimum,
                                                  double maximum,
                                                  int decimals)
{
  Option& option = this->Internal->AddOption(property, "double", label, toolTip);
  option.DefaultValue = vtkVariant(defaultValue);
  option.HasRange = true;
  option.Minimum = minimum;
  option.Maximum = maximum;
  option.Decimals = decimals;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::AddStringOption(const std::string& property, //
                                                  const std::string& label,
                                                  const std::string& toolTip,
                                                  const std::string& defaultValue)
{
  Option& option = this->Internal->AddOption(property, "string", label, toolTip);
  option.DefaultValue = vtkVariant(defaultValue);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::AddStringListOption(const std::string& property,
                                                      const std::string& label,
                                                      const std::string& toolTip,
                                                      const std::vector<std::string>& defaultValue,
                                                      const std::string& separator /*=";"*/)
{
  Option& option = this->Internal->AddOption(property, "stringList", label, toolTip);
  option.DefaultListValue = defaultValue;
  option.Separator = separator;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::AddEnumOption(const std::string& property, //
                                                const std::string& label,
                                                const std::string& toolTip,
                                                const vtkVariant& defaultValue)
{
  Option& option = this->Internal->AddOption(property, "enum", label, toolTip);
  option.DefaultValue = defaultValue;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::AddEnumChoice(const std::string& property, const vtkVariant& value, const std::string& label)
{
  Option* option = this->Internal->FindOption(property);
  if (!option || option->Type != "enum")
  {
    vtkErrorMacro("AddEnumChoice failed: enum option " << property << " is not found");
    return;
  }
  option->Choices.emplace_back(value, label);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::AddNodeOption(const std::string& property,
                                                const std::string& label,
                                                const std::string& toolTip,
                                                const std::string& defaultNodeID,
                                                const std::vector<std::string>& nodeClasses,
                                                bool noneEnabled)
{
  Option& option = this->Internal->AddOption(property, "node", label, toolTip);
  option.DefaultValue = vtkVariant(defaultNodeID);
  option.NodeClasses = nodeClasses;
  option.NoneEnabled = noneEnabled;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::SetOptionEnabled(const std::string& property, bool enabled)
{
  Option* option = this->Internal->FindOption(property);
  if (!option)
  {
    vtkErrorMacro("SetOptionEnabled failed: option " << property << " is not found");
    return;
  }
  option->Enabled = enabled;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::SetOptionVisible(const std::string& property, bool visible)
{
  Option* option = this->Internal->FindOption(property);
  if (!option)
  {
    vtkErrorMacro("SetOptionVisible failed: option " << property << " is not found");
    return;
  }
  option->Visible = visible;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::SetOptionWidget(const std::string& property, const std::string& widget)
{
  Option* option = this->Internal->FindOption(property);
  if (!option)
  {
    vtkErrorMacro("SetOptionWidget failed: option " << property << " is not found");
    return;
  }
  option->Widget = widget;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::SetOptionShowHidden(const std::string& property, bool showHidden)
{
  Option* option = this->Internal->FindOption(property);
  if (!option)
  {
    vtkErrorMacro("SetOptionShowHidden failed: option " << property << " is not found");
    return;
  }
  option->ShowHidden = showHidden;
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkMRMLIOOptionsDescription::GetNumberOfOptions()
{
  return static_cast<int>(this->Internal->Options.size());
}

//----------------------------------------------------------------------------
bool vtkMRMLIOOptionsDescription::HasOption(const std::string& property)
{
  return this->Internal->FindOption(property) != nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::RemoveAllOptions()
{
  this->Internal->Options.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
vtkVariant vtkMRMLIOOptionsDescription::GetOptionValue(const std::string& property)
{
  Option* option = this->Internal->FindOption(property);
  if (!option)
  {
    return vtkVariant();
  }
  return this->Internal->GetValue(*option);
}

//----------------------------------------------------------------------------
bool vtkMRMLIOOptionsDescription::GetBoolOptionValue(const std::string& property, bool defaultValue /*=false*/)
{
  vtkVariant value = this->GetOptionValue(property);
  if (!value.IsValid())
  {
    return defaultValue;
  }
  return value.ToInt() != 0;
}

//----------------------------------------------------------------------------
void vtkMRMLIOOptionsDescription::GetOptionValues(vtkMRMLIOProperties* values)
{
  if (!values)
  {
    return;
  }
  for (const Option& option : this->Internal->Options)
  {
    const std::string& name = option.Property;
    if (option.Type == "stringList")
    {
      std::vector<std::string> listValue = this->Internal->GetListValue(option);
      if (listValue.empty() || (listValue.size() == 1 && listValue[0].empty()))
      {
        continue;
      }
      values->SetStringListProperty(name, listValue);
      continue;
    }
    vtkVariant value = this->Internal->GetValue(option);
    if (option.Type == "bool")
    {
      values->SetBoolProperty(name, value.ToInt() != 0);
    }
    else if ((option.Type == "string" || option.Type == "node") && value.ToString().empty())
    {
      continue;
    }
    else
    {
      values->SetProperty(name, value);
    }
  }
}

//----------------------------------------------------------------------------
int vtkMRMLIOOptionsDescription::GetOptionIndex(const std::string& property)
{
  for (size_t index = 0; index < this->Internal->Options.size(); ++index)
  {
    if (this->Internal->Options[index].Property == property)
    {
      return static_cast<int>(index);
    }
  }
  return -1;
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOOptionsDescription::GetNthOptionProperty(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Property : std::string();
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOOptionsDescription::GetNthOptionType(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Type : std::string();
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOOptionsDescription::GetNthOptionLabel(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Label : std::string();
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOOptionsDescription::GetNthOptionToolTip(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->ToolTip : std::string();
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOOptionsDescription::GetNthOptionWidget(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Widget : std::string();
}

//----------------------------------------------------------------------------
bool vtkMRMLIOOptionsDescription::GetNthOptionEnabled(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Enabled : false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIOOptionsDescription::GetNthOptionVisible(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Visible : false;
}

//----------------------------------------------------------------------------
vtkVariant vtkMRMLIOOptionsDescription::GetNthOptionValue(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? this->Internal->GetValue(*option) : vtkVariant();
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLIOOptionsDescription::GetNthOptionStringListValue(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? this->Internal->GetListValue(*option) : std::vector<std::string>();
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOOptionsDescription::GetNthOptionSeparator(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Separator : std::string();
}

//----------------------------------------------------------------------------
bool vtkMRMLIOOptionsDescription::GetNthOptionHasRange(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->HasRange : false;
}

//----------------------------------------------------------------------------
double vtkMRMLIOOptionsDescription::GetNthOptionMinimum(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Minimum : 0.0;
}

//----------------------------------------------------------------------------
double vtkMRMLIOOptionsDescription::GetNthOptionMaximum(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Maximum : 0.0;
}

//----------------------------------------------------------------------------
int vtkMRMLIOOptionsDescription::GetNthOptionDecimals(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->Decimals : -1;
}

//----------------------------------------------------------------------------
int vtkMRMLIOOptionsDescription::GetNthOptionNumberOfChoices(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? static_cast<int>(option->Choices.size()) : 0;
}

//----------------------------------------------------------------------------
vtkVariant vtkMRMLIOOptionsDescription::GetNthOptionChoiceValue(int index, int choiceIndex)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  if (!option)
  {
    return vtkVariant();
  }
  if (choiceIndex < 0 || choiceIndex >= static_cast<int>(option->Choices.size()))
  {
    vtkErrorMacro(<< __func__ << " failed: invalid choice index " << choiceIndex << " for option " << option->Property);
    return vtkVariant();
  }
  return option->Choices[choiceIndex].first;
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOOptionsDescription::GetNthOptionChoiceLabel(int index, int choiceIndex)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  if (!option)
  {
    return std::string();
  }
  if (choiceIndex < 0 || choiceIndex >= static_cast<int>(option->Choices.size()))
  {
    vtkErrorMacro(<< __func__ << " failed: invalid choice index " << choiceIndex << " for option " << option->Property);
    return std::string();
  }
  return option->Choices[choiceIndex].second;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLIOOptionsDescription::GetNthOptionNodeClasses(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->NodeClasses : std::vector<std::string>();
}

//----------------------------------------------------------------------------
bool vtkMRMLIOOptionsDescription::GetNthOptionNoneEnabled(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->NoneEnabled : false;
}

//----------------------------------------------------------------------------
bool vtkMRMLIOOptionsDescription::GetNthOptionShowHidden(int index)
{
  const Option* option = this->Internal->GetNthOption(this, index, __func__);
  return option ? option->ShowHidden : false;
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOOptionsDescription::ToJSON()
{
  vtkNew<vtkMRMLJsonWriter> writer;
  writer->WriteToStringBegin();
  writer->WriteArrayPropertyStart("options");
  for (const Option& option : this->Internal->Options)
  {
    writer->WriteObjectStart();
    writer->WriteStringProperty("property", option.Property);
    writer->WriteStringProperty("type", option.Type);
    writer->WriteStringProperty("label", option.Label);
    writer->WriteStringProperty("toolTip", option.ToolTip);
    if (option.Type == "stringList")
    {
      writer->WriteStringVectorProperty("value", this->Internal->GetListValue(option));
      writer->WriteStringProperty("separator", option.Separator);
    }
    else if (option.Type == "bool")
    {
      writer->WriteBoolProperty("value", this->Internal->GetValue(option).ToInt() != 0);
    }
    else
    {
      writer->WriteVariantProperty("value", this->Internal->GetValue(option));
    }
    writer->WriteBoolProperty("enabled", option.Enabled);
    writer->WriteBoolProperty("visible", option.Visible);
    writer->WriteStringPropertyIfNotEmpty("widget", option.Widget);
    if (option.HasRange)
    {
      // minimum and maximum may be infinite, which is written as null
      writer->WriteVariantProperty("minimum", vtkVariant(option.Minimum));
      writer->WriteVariantProperty("maximum", vtkVariant(option.Maximum));
      if (option.Decimals >= 0)
      {
        writer->WriteIntProperty("decimals", option.Decimals);
      }
    }
    if (option.Type == "enum")
    {
      writer->WriteArrayPropertyStart("choices");
      for (const auto& choice : option.Choices)
      {
        writer->WriteObjectStart();
        writer->WriteVariantProperty("value", choice.first);
        writer->WriteStringProperty("label", choice.second);
        writer->WriteObjectEnd();
      }
      writer->WriteArrayPropertyEnd();
    }
    if (option.Type == "node")
    {
      writer->WriteStringVectorProperty("nodeClasses", option.NodeClasses);
      writer->WriteBoolProperty("noneEnabled", option.NoneEnabled);
      writer->WriteBoolProperty("showHidden", option.ShowHidden);
    }
    writer->WriteObjectEnd();
  }
  writer->WriteArrayPropertyEnd();
  return writer->WriteToStringEnd();
}
