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

#ifndef __vtkMRMLAbstractAnnotationPropertyValueProvider_h
#define __vtkMRMLAbstractAnnotationPropertyValueProvider_h

// MRML includes
class vtkMRMLSliceNode;

// VTK includes
#include <vtkObject.h>

// STD includes
#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <unordered_set>

/// \brief Base class for Annotation property value provider.
///
/// Sub-classes must implement this interface to provide slice view annotations.
class vtkMRMLAbstractAnnotationPropertyValueProvider : public vtkObject
{
public:

  // Typedefs

  typedef std::unordered_map<std::string, std::string> XMLTagAttributes;

  // Display levels

  enum DisplayLevel
  {
    DISPLAY_LEAST = 1,
    DISPLAY_SOMETIMES,
    DISPLAY_ALWAYS
  };

  // Layers

  enum Layer
  {
    LAYER_FOREGROUND = 0,
    LAYER_BACKGROUND,
    LAYER_LABEL
  };

  vtkTypeMacro(vtkMRMLAbstractAnnotationPropertyValueProvider, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual bool
  CanProvideValueForPropertyName(const std::string &propertyName) = 0;
  virtual std::string GetValueForPropertyName(const std::string &propertyName,
                                          const XMLTagAttributes &attributes,
                                          vtkMRMLSliceNode *) = 0;
  virtual std::unordered_set<std::string> GetSupportedProperties() = 0;

  static int GetAttributeValueAsInteger(const std::string &attribute,
                                        const XMLTagAttributes &attributes,
                                        int defaultValue)
  {
    return attributes.count(attribute) ? std::stoi(attributes.at(attribute))
                                     : defaultValue;
  }

  static int GetLayerValueAsInteger(const XMLTagAttributes &attributes,
      int defaultValue = LAYER_FOREGROUND)
  {
    if (attributes.count("layer"))
    {
      std::string layer = attributes.at("layer");

      auto getDigit = [](const std::string &s)
      {
        return std::all_of(s.begin(), s.end(), ::isdigit) ? std::stoi(s) : -1;
      };

      if (layer == "foreground" || getDigit(layer) == LAYER_FOREGROUND)
      {
        return LAYER_FOREGROUND;
      }
      else if (layer == "background" || getDigit(layer) == LAYER_BACKGROUND)
      {
        return LAYER_BACKGROUND;
      }
      else if (layer == "label" || getDigit(layer) == LAYER_LABEL)
      {
        return LAYER_LABEL;
      }
    }
    return defaultValue;
  }

  static int GetDisplayLevelValueAsInteger(const XMLTagAttributes &attributes,
      int defaultValue = DISPLAY_ALWAYS)
  {
      if (attributes.count("display-level"))
      {
          std::string displayLevel = attributes.at("display-level");

          auto getDigit = [](const std::string &s)
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

protected:
  vtkMRMLAbstractAnnotationPropertyValueProvider() = default;
  ~vtkMRMLAbstractAnnotationPropertyValueProvider() override = default;
  vtkMRMLAbstractAnnotationPropertyValueProvider(const vtkMRMLAbstractAnnotationPropertyValueProvider&) = delete;
  void operator=(const vtkMRMLAbstractAnnotationPropertyValueProvider&) = delete;
};

#endif
