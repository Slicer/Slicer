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
int vtkMRMLAbstractAnnotationPropertyValueProvider::GetLayerValueAsInteger(
    const XMLTagAttributes& attributes, int defaultValue)
{
  if (attributes.count("layer"))
  {
    std::string layer = attributes.at("layer");

    auto getDigit = [](const std::string& s)
    {
      return std::all_of(s.begin(), s.end(), ::isdigit) ? std::stoi(s) : -1;
    };

    if (layer == "background" || getDigit(layer) == LAYER_BACKGROUND)
    {
      return LAYER_BACKGROUND;
    }
    else if (layer == "foreground" || getDigit(layer) == LAYER_FOREGROUND)
    {
      return LAYER_FOREGROUND;
    }
    else if (layer == "label" || getDigit(layer) == LAYER_LABEL)
    {
      return LAYER_LABEL;
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
    std::string displayLevel = attributes.at("display-level");

    auto getDigit = [](const std::string& s)
    {
      return std::all_of(s.begin(), s.end(), ::isdigit) ? std::stoi(s) : -1;
    };

    if (displayLevel == "always" || getDigit(displayLevel) == DISPLAY_ALWAYS)
    {
      return DISPLAY_ALWAYS;
    }
    else if (displayLevel == "sometimes" || getDigit(displayLevel) == DISPLAY_SOMETIMES)
    {
       return DISPLAY_SOMETIMES;
    }
    else if (displayLevel == "least" || getDigit(displayLevel) == DISPLAY_LEAST)
    {
      return DISPLAY_LEAST;
    }
  }
  return defaultValue;
}
