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

// MRMLLogic includes
#include "vtkMRMLAbstractAnnotationPropertyValueProvider.h"

//----------------------------------------------------------------------------
void vtkMRMLAbstractAnnotationPropertyValueProvider::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SupportedProperties: ";
  if (this->GetSupportedProperties().empty())
  {
    os << "(none)\n";
  }
  else
  {
    for(const std::string& propertyName: this->GetSupportedProperties())
    {
      os << indent << indent.GetNextIndent() << propertyName << "\n";
    }
  }
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractAnnotationPropertyValueProvider::GetAttributeValueAsInteger(
    const std::string& attribute, const XMLTagAttributes& attributes, int defaultValue)
{
  return attributes.count(attribute) ? std::stoi(attributes.at(attribute)) : defaultValue;
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractAnnotationPropertyValueProvider::GetRoleValueAsInteger(
    const XMLTagAttributes& attributes, int defaultValue)
{
  if (attributes.count("role"))
  {
    std::string role = attributes.at("role");

    auto getDigit = [](const std::string& s)
    {
      return std::all_of(s.begin(), s.end(), ::isdigit) ? std::stoi(s) : -1;
    };

    if (role == "background" || getDigit(role) == ROLE_BACKGROUND)
    {
      return ROLE_BACKGROUND;
    }
    else if (role == "foreground" || getDigit(role) == ROLE_FOREGROUND)
    {
      return ROLE_FOREGROUND;
    }
    else if (role == "label" || getDigit(role) == ROLE_LABEL)
    {
      return ROLE_LABEL;
    }
  }
  return defaultValue;
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractAnnotationPropertyValueProvider::GetDisplayLevelValueAsInteger(
    const XMLTagAttributes& attributes, int defaultValue)
{
  if (attributes.count("display-level"))
  {
    return std::stoi(attributes.at("display-level"));
  }
  return defaultValue;
}
