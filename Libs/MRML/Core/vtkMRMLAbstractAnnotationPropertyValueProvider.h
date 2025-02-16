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
#include <vtkMRMLSliceCompositeNode.h>
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

  // Roles
  enum Role
  {
    ROLE_BACKGROUND = vtkMRMLSliceCompositeNode::LayerBackground,
    ROLE_FOREGROUND = vtkMRMLSliceCompositeNode::LayerForeground,
    ROLE_LABEL = vtkMRMLSliceCompositeNode::LayerLabel,
  };

  vtkTypeMacro(vtkMRMLAbstractAnnotationPropertyValueProvider, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual bool
  CanProvideValueForPropertyName(const std::string &propertyName) = 0;
  virtual std::string GetValueForPropertyName(const std::string& propertyName,
                                              const XMLTagAttributes& attributes,
                                              vtkMRMLSliceNode*) = 0;
  virtual std::unordered_set<std::string> GetSupportedProperties() = 0;

  static int GetAttributeValueAsInteger(const std::string& attribute,
                                        const XMLTagAttributes& attributes,
                                        int defaultValue);

  static int GetRoleValueAsInteger(const XMLTagAttributes& attributes, int defaultValue = ROLE_FOREGROUND);

  static int GetDisplayLevelValueAsInteger(const XMLTagAttributes& attributes, int defaultValue = 1);

protected:
  vtkMRMLAbstractAnnotationPropertyValueProvider() = default;
  ~vtkMRMLAbstractAnnotationPropertyValueProvider() override = default;
  vtkMRMLAbstractAnnotationPropertyValueProvider(const vtkMRMLAbstractAnnotationPropertyValueProvider&) = delete;
  void operator=(const vtkMRMLAbstractAnnotationPropertyValueProvider&) = delete;
};

#endif
