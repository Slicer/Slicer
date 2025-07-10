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

// SegmentationCore includes
#include "vtkSegmentationConversionParameters.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkVariant.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSegmentationConversionParameters);

//----------------------------------------------------------------------------
vtkSegmentationConversionParameters::vtkSegmentationConversionParameters() {}

//----------------------------------------------------------------------------
vtkSegmentationConversionParameters::~vtkSegmentationConversionParameters() {}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::PrintSelf(ostream& os, vtkIndent indent)
{
  for (const ConversionParameterType& parameter : this->ParameterList)
  {
    os << indent << parameter.Name << ": " << parameter.Value << " [" << parameter.Description << "]\n";
  }
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConversionParameters::GetName(int index)
{
  if (index < 0 || index >= this->GetNumberOfParameters())
  {
    return "";
  }
  return this->ParameterList[index].Name;
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::SetName(int index, const std::string& name)
  VTK_EXPECTS(0 <= index && index < GetNumberOfParameters())
{
  if (index < 0 || index >= this->GetNumberOfParameters())
  {
    vtkErrorMacro("SetName failed: invalid index");
    return;
  }
  this->ParameterList[index].Name = name;
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConversionParameters::GetDescription(int index)
{
  if (index < 0 || index >= this->GetNumberOfParameters())
  {
    return "";
  }
  return this->ParameterList[index].Description;
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConversionParameters::GetDescription(const std::string& name)
{
  int index = this->GetIndexFromName(name);
  if (index < 0)
  {
    return "";
  }
  else
  {
    return this->ParameterList[index].Description;
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::SetDescription(int index, const std::string& description)
{
  if (index < 0 || index >= this->GetNumberOfParameters())
  {
    vtkErrorMacro("SetDescription failed: invalid index");
    return;
  }
  this->ParameterList[index].Description = description;
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::SetDescription(const std::string& name, const std::string& description)
{
  int index = this->GetIndexFromName(name);
  if (index < 0)
  {
    ConversionParameterType parameter;
    parameter.Name = name;
    parameter.Description = description;
    this->ParameterList.push_back(parameter);
  }
  else
  {
    this->ParameterList[index].Description = description;
  }
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConversionParameters::GetValue(int index)
{
  if (index < 0 || index >= this->GetNumberOfParameters())
  {
    return "";
  }
  return this->ParameterList[index].Value;
}

//----------------------------------------------------------------------------
double vtkSegmentationConversionParameters::GetValueAsDouble(const std::string& name)
{
  double value = vtkVariant(this->GetValue(name)).ToDouble();
  return value;
}

//----------------------------------------------------------------------------
int vtkSegmentationConversionParameters::GetValueAsInt(const std::string& name)
{
  double value = vtkVariant(this->GetValue(name)).ToInt();
  return value;
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConversionParameters::GetValue(const std::string& name)
{
  int index = this->GetIndexFromName(name);
  if (index < 0)
  {
    return "";
  }
  else
  {
    return this->ParameterList[index].Value;
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::SetValue(int index, const std::string& value)
{
  if (index < 0 || index >= this->GetNumberOfParameters())
  {
    vtkErrorMacro("SetValue failed: invalid index");
    return;
  }
  this->ParameterList[index].Value = value;
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::SetValue(const std::string& name, const std::string& value)
{
  int index = this->GetIndexFromName(name);
  if (index < 0)
  {
    ConversionParameterType parameter;
    parameter.Name = name;
    parameter.Value = value;
    this->ParameterList.push_back(parameter);
  }
  else
  {
    this->ParameterList[index].Value = value;
  }
}

//----------------------------------------------------------------------------
int vtkSegmentationConversionParameters::GetIndexFromName(const std::string name)
{
  int index = 0;
  for (const ConversionParameterType& parameter : ParameterList)
  {
    if (parameter.Name == name)
    {
      return index;
    }
    index += 1;
  }
  return -1;
}

//----------------------------------------------------------------------------
int vtkSegmentationConversionParameters::GetNumberOfParameters()
{
  return this->ParameterList.size();
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::RemoveAllParameters()
{
  this->ParameterList.clear();
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::RemoveParameter(int index)
{
  if (index < 0 || index >= this->GetNumberOfParameters())
  {
    vtkErrorMacro("RemoveParameter failed: invalid index");
    return;
  }
  this->ParameterList.erase(this->ParameterList.begin() + index);
}

//----------------------------------------------------------------------------
int vtkSegmentationConversionParameters::SetParameter(const std::string& name,
                                                      const std::string& value,
                                                      const std::string& description /*=""*/)
{
  int parameterIndex = this->GetIndexFromName(name);
  if (parameterIndex < 0)
  {
    ConversionParameterType newParameter;
    this->ParameterList.push_back(newParameter);
    parameterIndex = this->ParameterList.size() - 1;
  }
  ConversionParameterType parameter;
  parameter.Name = name;
  parameter.Value = value;
  parameter.Description = description;
  this->ParameterList[parameterIndex] = parameter;
  return parameterIndex;
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::DeepCopy(vtkSegmentationConversionParameters* source)
{
  if (!source)
  {
    vtkErrorMacro("DeepCopy failed: invalid source object");
    return;
  }
  this->ParameterList = source->ParameterList;
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionParameters::CopyParameter(vtkSegmentationConversionParameters* source, int sourceIndex)
{
  if (!source || sourceIndex < 0 || sourceIndex >= source->GetNumberOfParameters())
  {
    vtkErrorMacro("DeepCopy failed: invalid source object or index");
    return;
  }
  this->SetParameter(source->ParameterList[sourceIndex].Name,
                     source->ParameterList[sourceIndex].Value,
                     source->ParameterList[sourceIndex].Description);
}
