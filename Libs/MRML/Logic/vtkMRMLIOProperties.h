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

#ifndef __vtkMRMLIOProperties_h
#define __vtkMRMLIOProperties_h

#include "vtkMRMLLogicExport.h"

#include <vtkObject.h>
#include <vtkVariant.h>

#include <map>
#include <string>
#include <vector>

class vtkVariantArray;

/// Named values that specify a file reading or writing request, such as "fileName", "name", "nodeID",
/// and the values of reader or writer options. This is the VTK equivalent of qSlicerIO::IOProperties.
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#file-reading-and-writing
class VTK_MRML_LOGIC_EXPORT vtkMRMLIOProperties : public vtkObject
{
public:
  static vtkMRMLIOProperties* New();
  vtkTypeMacro(vtkMRMLIOProperties, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set a property. A value that is already there is replaced.
  void SetProperty(const std::string& name, const vtkVariant& value);
  void SetStringProperty(const std::string& name, const std::string& value);
  void SetIntProperty(const std::string& name, int value);
  void SetDoubleProperty(const std::string& name, double value);
  void SetBoolProperty(const std::string& name, bool value);

  /// Set a property to a list of strings (such as "fileNames").
  void SetStringListProperty(const std::string& name, const std::vector<std::string>& values);

  /// Set a property to a list of values (strings, numbers, and booleans). The values are copied.
  /// Boolean values are stored as vtkVariant of char type (see vtkVariant(bool)).
  void SetListProperty(const std::string& name, vtkVariantArray* values);

  /// Set a property to a nested set of properties (a map). The properties are copied.
  void SetMapProperty(const std::string& name, vtkMRMLIOProperties* values);

  /// Set a property to an object (such as the "screenShot" image of a scene). The object is referenced.
  void SetObjectProperty(const std::string& name, vtkObject* value);

  /// The value of a property, or an invalid variant if it is not there.
  vtkVariant GetProperty(const std::string& name) const;
  /// The value of a property as a string; *defaultValue* if it is not there.
  std::string GetStringProperty(const std::string& name, const std::string& defaultValue = "") const;
  int GetIntProperty(const std::string& name, int defaultValue = 0) const;
  double GetDoubleProperty(const std::string& name, double defaultValue = 0.0) const;
  bool GetBoolProperty(const std::string& name, bool defaultValue = false) const;
  /// The value of a property as a list of strings. A single string value is returned as a list of one item.
  /// Returns an empty list if the property is not there.
  std::vector<std::string> GetStringListProperty(const std::string& name) const;
  /// The value of a list property. Values of a list of strings property are returned as well.
  /// Returns false if the property is not a list.
  bool GetListProperty(const std::string& name, vtkVariantArray* values) const;
  /// The value of a map property, or nullptr if it is not there or not a map.
  /// The returned object is owned by these properties.
  vtkMRMLIOProperties* GetMapProperty(const std::string& name) const;
  /// The value of an object property, or nullptr if it is not there or not an object.
  vtkObject* GetObjectProperty(const std::string& name) const;

  /// Whether the property was set as a boolean (SetBoolProperty), a list of strings
  /// (SetStringListProperty), a list of values (SetListProperty), a map (SetMapProperty),
  /// or an object (SetObjectProperty).
  /// This allows converting the properties to other representations (such as a Python dictionary) without losing the type.
  bool IsBoolProperty(const std::string& name) const;
  bool IsStringListProperty(const std::string& name) const;
  bool IsListProperty(const std::string& name) const;
  bool IsMapProperty(const std::string& name) const;
  bool IsObjectProperty(const std::string& name) const;

  bool HasProperty(const std::string& name) const;
  void RemoveProperty(const std::string& name);
  void RemoveAllProperties();

  /// The names of the properties that are set, in the order they were first set.
  std::vector<std::string> GetPropertyNames() const;

  /// Replace these properties with the properties of another set.
  void Copy(vtkMRMLIOProperties* source);

  /// Add the properties of another set to these, replacing values that are already set.
  void Update(vtkMRMLIOProperties* source);

protected:
  vtkMRMLIOProperties();
  ~vtkMRMLIOProperties() override;

private:
  vtkMRMLIOProperties(const vtkMRMLIOProperties&) = delete;
  void operator=(const vtkMRMLIOProperties&) = delete;

  std::map<std::string, vtkVariant> Properties;
  std::vector<std::string> Order;
  /// Names of properties that were set as boolean values
  std::vector<std::string> BoolProperties;
};

#endif
