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

#include "vtkMRMLIOProperties.h"

#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkVariantArray.h>

#include <algorithm>
#include <cctype>

vtkStandardNewMacro(vtkMRMLIOProperties);

//----------------------------------------------------------------------------
vtkMRMLIOProperties::vtkMRMLIOProperties() = default;

//----------------------------------------------------------------------------
vtkMRMLIOProperties::~vtkMRMLIOProperties() = default;

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  for (const std::string& name : this->Order)
  {
    os << indent << name << ": ";
    if (this->IsStringListProperty(name))
    {
      for (const std::string& value : this->GetStringListProperty(name))
      {
        os << "[" << value << "]";
      }
    }
    else if (this->IsListProperty(name))
    {
      vtkVariantArray* values = vtkVariantArray::SafeDownCast(this->Properties.at(name).ToVTKObject());
      for (vtkIdType i = 0; values && i < values->GetNumberOfValues(); ++i)
      {
        os << "[" << values->GetValue(i).ToString() << "]";
      }
    }
    else if (this->IsMapProperty(name))
    {
      os << "\n";
      this->GetMapProperty(name)->PrintSelf(os, indent.GetNextIndent());
      continue;
    }
    else if (this->IsObjectProperty(name))
    {
      vtkObject* object = this->GetObjectProperty(name);
      os << (object ? object->GetClassName() : "(none)");
    }
    else
    {
      os << this->Properties.at(name).ToString();
    }
    os << "\n";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::SetProperty(const std::string& name, const vtkVariant& value)
{
  if (name.empty())
  {
    vtkErrorMacro("SetProperty: a property needs a name");
    return;
  }
  if (this->Properties.find(name) == this->Properties.end())
  {
    this->Order.push_back(name);
  }
  this->Properties[name] = value;
  this->BoolProperties.erase(std::remove(this->BoolProperties.begin(), this->BoolProperties.end(), name), this->BoolProperties.end());
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::SetStringProperty(const std::string& name, const std::string& value)
{
  this->SetProperty(name, vtkVariant(value));
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::SetIntProperty(const std::string& name, int value)
{
  this->SetProperty(name, vtkVariant(value));
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::SetDoubleProperty(const std::string& name, double value)
{
  this->SetProperty(name, vtkVariant(value));
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::SetBoolProperty(const std::string& name, bool value)
{
  this->SetProperty(name, vtkVariant(value ? 1 : 0));
  if (!name.empty())
  {
    this->BoolProperties.push_back(name);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::SetStringListProperty(const std::string& name, const std::vector<std::string>& values)
{
  // Stored as vtkStringArray, because vtkVariant cannot store a list of strings
  vtkNew<vtkStringArray> valuesArray;
  for (const std::string& value : values)
  {
    valuesArray->InsertNextValue(value);
  }
  this->SetProperty(name, vtkVariant(valuesArray.GetPointer()));
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::SetListProperty(const std::string& name, vtkVariantArray* values)
{
  vtkNew<vtkVariantArray> copiedValues;
  if (values)
  {
    copiedValues->DeepCopy(values);
  }
  this->SetProperty(name, vtkVariant(copiedValues.GetPointer()));
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::SetMapProperty(const std::string& name, vtkMRMLIOProperties* values)
{
  vtkNew<vtkMRMLIOProperties> copiedValues;
  if (values)
  {
    copiedValues->Copy(values);
  }
  this->SetProperty(name, vtkVariant(copiedValues.GetPointer()));
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::SetObjectProperty(const std::string& name, vtkObject* value)
{
  this->SetProperty(name, vtkVariant(value));
}

//----------------------------------------------------------------------------
vtkVariant vtkMRMLIOProperties::GetProperty(const std::string& name) const
{
  std::map<std::string, vtkVariant>::const_iterator found = this->Properties.find(name);
  return found == this->Properties.end() ? vtkVariant() : found->second;
}

//----------------------------------------------------------------------------
std::string vtkMRMLIOProperties::GetStringProperty(const std::string& name, const std::string& defaultValue) const
{
  vtkVariant value = this->GetProperty(name);
  if (!value.IsValid())
  {
    return defaultValue;
  }
  if (value.IsVTKObject())
  {
    // A list of one string is the same as that string
    vtkStringArray* values = vtkStringArray::SafeDownCast(value.ToVTKObject());
    if (values)
    {
      return values->GetNumberOfValues() == 1 ? values->GetValue(0) : std::string();
    }
    vtkVariantArray* variantValues = vtkVariantArray::SafeDownCast(value.ToVTKObject());
    if (variantValues)
    {
      return variantValues->GetNumberOfValues() == 1 ? variantValues->GetValue(0).ToString() : std::string();
    }
    return std::string();
  }
  return value.ToString();
}

//----------------------------------------------------------------------------
int vtkMRMLIOProperties::GetIntProperty(const std::string& name, int defaultValue) const
{
  vtkVariant value = this->GetProperty(name);
  if (!value.IsValid() || value.IsVTKObject())
  {
    return defaultValue;
  }
  bool valid = false;
  int intValue = value.ToInt(&valid);
  if (!valid)
  {
    // floating-point value given as string
    double doubleValue = value.ToDouble(&valid);
    intValue = static_cast<int>(doubleValue);
  }
  return valid ? intValue : defaultValue;
}

//----------------------------------------------------------------------------
double vtkMRMLIOProperties::GetDoubleProperty(const std::string& name, double defaultValue) const
{
  vtkVariant value = this->GetProperty(name);
  if (!value.IsValid() || value.IsVTKObject())
  {
    return defaultValue;
  }
  bool valid = false;
  double doubleValue = value.ToDouble(&valid);
  return valid ? doubleValue : defaultValue;
}

//----------------------------------------------------------------------------
bool vtkMRMLIOProperties::GetBoolProperty(const std::string& name, bool defaultValue) const
{
  vtkVariant value = this->GetProperty(name);
  if (!value.IsValid() || value.IsVTKObject())
  {
    return defaultValue;
  }
  if (value.IsString())
  {
    std::string text = value.ToString();
    std::transform(text.begin(), text.end(), text.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
    return !(text == "" || text == "0" || text == "false" || text == "no");
  }
  return value.ToDouble() != 0.0;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLIOProperties::GetStringListProperty(const std::string& name) const
{
  std::vector<std::string> result;
  vtkVariant value = this->GetProperty(name);
  if (!value.IsValid())
  {
    return result;
  }
  if (value.IsVTKObject())
  {
    vtkStringArray* values = vtkStringArray::SafeDownCast(value.ToVTKObject());
    for (vtkIdType i = 0; values && i < values->GetNumberOfValues(); ++i)
    {
      result.push_back(values->GetValue(i));
    }
    vtkVariantArray* variantValues = vtkVariantArray::SafeDownCast(value.ToVTKObject());
    for (vtkIdType i = 0; variantValues && i < variantValues->GetNumberOfValues(); ++i)
    {
      result.push_back(variantValues->GetValue(i).ToString());
    }
    return result;
  }
  result.push_back(value.ToString());
  return result;
}

//----------------------------------------------------------------------------
bool vtkMRMLIOProperties::GetListProperty(const std::string& name, vtkVariantArray* values) const
{
  if (!values)
  {
    return false;
  }
  values->Reset();
  vtkVariant value = this->GetProperty(name);
  if (!value.IsVTKObject())
  {
    return false;
  }
  vtkVariantArray* variantValues = vtkVariantArray::SafeDownCast(value.ToVTKObject());
  if (variantValues)
  {
    values->DeepCopy(variantValues);
    return true;
  }
  vtkStringArray* stringValues = vtkStringArray::SafeDownCast(value.ToVTKObject());
  if (stringValues)
  {
    for (vtkIdType i = 0; i < stringValues->GetNumberOfValues(); ++i)
    {
      values->InsertNextValue(vtkVariant(stringValues->GetValue(i)));
    }
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
vtkMRMLIOProperties* vtkMRMLIOProperties::GetMapProperty(const std::string& name) const
{
  vtkVariant value = this->GetProperty(name);
  if (!value.IsVTKObject())
  {
    return nullptr;
  }
  return vtkMRMLIOProperties::SafeDownCast(value.ToVTKObject());
}

//----------------------------------------------------------------------------
vtkObject* vtkMRMLIOProperties::GetObjectProperty(const std::string& name) const
{
  if (!this->IsObjectProperty(name))
  {
    return nullptr;
  }
  return vtkObject::SafeDownCast(this->GetProperty(name).ToVTKObject());
}

//----------------------------------------------------------------------------
bool vtkMRMLIOProperties::IsBoolProperty(const std::string& name) const
{
  return std::find(this->BoolProperties.begin(), this->BoolProperties.end(), name) != this->BoolProperties.end();
}

//----------------------------------------------------------------------------
bool vtkMRMLIOProperties::IsStringListProperty(const std::string& name) const
{
  vtkVariant value = this->GetProperty(name);
  return value.IsVTKObject() && vtkStringArray::SafeDownCast(value.ToVTKObject()) != nullptr;
}

//----------------------------------------------------------------------------
bool vtkMRMLIOProperties::IsListProperty(const std::string& name) const
{
  vtkVariant value = this->GetProperty(name);
  return value.IsVTKObject() && vtkVariantArray::SafeDownCast(value.ToVTKObject()) != nullptr;
}

//----------------------------------------------------------------------------
bool vtkMRMLIOProperties::IsMapProperty(const std::string& name) const
{
  vtkVariant value = this->GetProperty(name);
  return value.IsVTKObject() && vtkMRMLIOProperties::SafeDownCast(value.ToVTKObject()) != nullptr;
}

//----------------------------------------------------------------------------
bool vtkMRMLIOProperties::IsObjectProperty(const std::string& name) const
{
  vtkVariant value = this->GetProperty(name);
  return value.IsVTKObject() //
         && !this->IsStringListProperty(name) && !this->IsListProperty(name) && !this->IsMapProperty(name);
}

//----------------------------------------------------------------------------
bool vtkMRMLIOProperties::HasProperty(const std::string& name) const
{
  return this->Properties.find(name) != this->Properties.end();
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::RemoveProperty(const std::string& name)
{
  if (!this->HasProperty(name))
  {
    return;
  }
  this->Properties.erase(name);
  this->Order.erase(std::remove(this->Order.begin(), this->Order.end(), name), this->Order.end());
  this->BoolProperties.erase(std::remove(this->BoolProperties.begin(), this->BoolProperties.end(), name), this->BoolProperties.end());
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::RemoveAllProperties()
{
  if (this->Properties.empty())
  {
    return;
  }
  this->Properties.clear();
  this->Order.clear();
  this->BoolProperties.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLIOProperties::GetPropertyNames() const
{
  return this->Order;
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::Copy(vtkMRMLIOProperties* source)
{
  if (!source || source == this)
  {
    return;
  }
  this->RemoveAllProperties();
  this->Update(source);
}

//----------------------------------------------------------------------------
void vtkMRMLIOProperties::Update(vtkMRMLIOProperties* source)
{
  if (!source || source == this)
  {
    return;
  }
  for (const std::string& name : source->Order)
  {
    if (source->IsStringListProperty(name))
    {
      this->SetStringListProperty(name, source->GetStringListProperty(name));
    }
    else if (source->IsListProperty(name))
    {
      this->SetListProperty(name, vtkVariantArray::SafeDownCast(source->Properties.at(name).ToVTKObject()));
    }
    else if (source->IsMapProperty(name))
    {
      this->SetMapProperty(name, source->GetMapProperty(name));
    }
    else if (source->IsBoolProperty(name))
    {
      this->SetBoolProperty(name, source->GetBoolProperty(name));
    }
    else
    {
      this->SetProperty(name, source->Properties.at(name));
    }
  }
}
