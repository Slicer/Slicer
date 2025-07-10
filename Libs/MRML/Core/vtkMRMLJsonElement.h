/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef vtkMRMLJsonElement_h
#define vtkMRMLJsonElement_h

#include "vtkMRMLMessageCollection.h"
#include "vtkSmartPointer.h"
#include "vtkNew.h"

#include <vector>

class vtkCodedEntry;
class vtkXMLDataElement;

/// \brief Represents a json object or list.
///
class VTK_MRML_EXPORT vtkMRMLJsonElement : public vtkObject
{
public:
  static vtkMRMLJsonElement* New();
  vtkTypeMacro(vtkMRMLJsonElement, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Separator characters for (de)serializing the UID and the attributes map
  static const std::string XML_SEPARATOR;
  static const std::string XML_NAME_VALUE_SEPARATOR;

  /// Get the JSON schema name
  std::string GetSchema();

  /// Returns true if the JSON object contains a member by this name.
  bool HasMember(const char* propertyName);

  enum Type
  {
    UNKNOWN = 0,
    OBJECT = 1,
    ARRAY = 2,
    STRING = 3,
    BOOL = 4,
    INT = 5,
    DOUBLE = 6
  };
  Type GetMemberType(const char* propertyName);

  /// Get string property value.
  /// Returns empty string if no such string property is found.
  std::string GetStringProperty(const char* propertyName);
  /// Get string property value.
  /// If a string property by the specified name is found then set it in propertyValue and return true.
  /// If a string property by the specified name is not found then return false.
  bool GetStringProperty(const char* propertyName, std::string& propertyValue);

  /// Get Boolean property value
  bool GetBoolProperty(const char* propertyName);

  /// Get double-precision floating point property value.
  /// If a double property by the specified name is found then set it in propertyValue and return true.
  /// If a double property by the specified name is not found then return false.
  bool GetDoubleProperty(const char* propertyName, double& propertyValue);

  /// Get double-precision floating point property value.
  /// If no such property is found or it is not double then 0.0 is returned.
  double GetDoubleProperty(const char* propertyName);

  /// Get integer property value.
  /// If an integer property by the specified name is found then set it in propertyValue and return true.
  /// If an integer double property by the specified name is not found then return false.
  bool GetIntProperty(const char* propertyName, int& propertyValue);

  /// Get integer property value.
  /// If no such property is found or it is not integer then 0 is returned.
  int GetIntProperty(const char* propertyName);

  /// Get a fixed-size vector of floating point values from a property.
  /// If no such property is found or it is not the right type then false is returned.
  bool GetVectorProperty(const char* propertyName, double* v, int numberOfComponents = 3);

  /// Get 4x4 matrix from a property.
  /// If no such property is found or it is not the right type then false is returned.
  /// If flipRasLps is set to true then the matrix is converted between LPS/RAS coordinate system
  /// (multiplied by diag(-1,-1,1,1) matrix).
  bool GetMatrix4x4Property(const char* propertyName, double v[16], bool flipRasLps);

  /// Get a vector of string values from a property.
  /// If no such property is found or it is not the right type then false is returned.
  bool GetStringVectorProperty(const char* propertyName, std::vector<std::string>& arrayValues);

  /// Get a coded entry object from a property.
  /// If no such property is found or it is not the right type then nullptr is returned.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkCodedEntry* GetCodedEntryProperty(const char* propertyName);

  /// Get a variable-size, potentially multi-component floating-point vector from a property.
  /// If no such property is found or it is not the right type then nullptr is returned.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkDoubleArray* GetDoubleArrayProperty(const char* propertyName);

  /// Get property values from each item of an array.
  /// If no such property is found or it is not the right type then false is returned.
  bool GetArrayItemsStringProperty(const char* arrayName,
                                   const char* propertyName,
                                   std::vector<std::string>& propertyValues);

  /// Get an array element from a property.
  /// If no such property is found or it is not the right type then nullptr is returned.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLJsonElement* GetArrayProperty(const char* arrayName);

  /// Get an object element from a property.
  /// If no such property is found or it is not the right type then nullptr is returned.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLJsonElement* GetObjectProperty(const char* objectName);

  /// Returns true if this element is an array.
  bool IsArray();

  /// Returns the number of elements in this array.
  int GetArraySize();

  /// Returns the n-th object elements in this array.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLJsonElement* GetArrayItem(int childItemIndex);

  /// Returns true if this element is an object.
  bool IsObject();

  /// Returns the number of elements in this object.
  int GetObjectSize();

  /// Get object member name by index.
  std::string GetObjectPropertyNameByIndex(int childItemIndex);

  /// Returns the n-th elements in this object as object.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLJsonElement* GetObjectItem(int childItemIndex);

  /// Returns this element type.
  Type GetType();

  /// Similar to GetObjectItem, but it returns the n-th elements for any type.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLJsonElement* GetChildMemberItem(int childItemIndex);

  /// Returns user-displayable messages that may contain details about any failed operation.
  vtkGetObjectMacro(UserMessages, vtkMRMLMessageCollection);

  /// Returns true if user messages contain error messages.
  bool HasErrors();

protected:
  vtkMRMLJsonElement();
  ~vtkMRMLJsonElement() override;
  vtkMRMLJsonElement(const vtkMRMLJsonElement&);
  void operator=(const vtkMRMLJsonElement&);

  vtkNew<vtkMRMLMessageCollection> UserMessages;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
  friend class vtkMRMLJsonReader;
};

class VTK_MRML_EXPORT vtkMRMLJsonReader : public vtkObject
{
public:
  static vtkMRMLJsonReader* New();
  vtkTypeMacro(vtkMRMLJsonReader, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Read JSON document from file.
  /// \return JSON element on success and nullptr on failure.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLJsonElement* ReadFromFile(const char* filePath);

  /// Read JSON document from file.
  /// \return JSON element on success and nullptr on failure.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLJsonElement* ReadFromString(const std::string& jsonString);

  /// Convert JSON to XML string
  /// return string
  std::string ConvertJsonToXML(const std::string& jsonString, const std::string& nodeTagName);

  /// Returns user-displayable messages that may contain details about any failed operation.
  vtkGetObjectMacro(UserMessages, vtkMRMLMessageCollection);

  /// Returns true if user messages contain error messages.
  bool HasErrors();

protected:
  vtkMRMLJsonReader();
  ~vtkMRMLJsonReader() override;
  vtkMRMLJsonReader(const vtkMRMLJsonReader&);
  void operator=(const vtkMRMLJsonReader&);

  std::string processJsonElement(vtkMRMLJsonElement* jsonElement, const std::string& elementKey = "");

  vtkNew<vtkMRMLMessageCollection> UserMessages;
};

/// \brief Writes properties into a JSON stream
///
class VTK_MRML_EXPORT vtkMRMLJsonWriter : public vtkObject
{
public:
  static vtkMRMLJsonWriter* New();
  vtkTypeMacro(vtkMRMLJsonWriter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// This method must be called before writing any properties to the output file.
  /// Returns true on success.
  bool WriteToFileBegin(const char* filePath, const char* schema);

  /// This method must be called after writing all properties to the output file.
  /// Returns true on success.
  bool WriteToFileEnd();

  /// This method must be called before writing any properties to the output file.
  /// Returns true on success.
  bool WriteToStringBegin(const char* nodeTagName);

  /// This method must be called after writing all properties to the output file.
  /// Returns string representation of the JSON document.
  std::string WriteToStringEnd();

  /// This method creates a new array as a property.
  void WriteArrayPropertyStart(const std::string& propertyName);
  /// This method must be called when all array items are written.
  void WriteArrayPropertyEnd();

  /// This method creates a new object in an array.
  void WriteObjectStart();
  /// This method must be called when all properties of the object are written.
  void WriteObjectEnd();

  /// This method creates a new object as a property.
  void WriteObjectPropertyStart(const std::string& propertyName);
  /// This method must be called when all properties of the object are written.
  void WriteObjectPropertyEnd();

  /// @{
  /// Write simple value as an object property.
  void WriteStringProperty(const std::string& propertyName, const std::string& propertyValue);
  void WriteStringVectorProperty(const std::string& propertyName, const std::vector<std::string>& arrayValues);
  void WriteCodedEntryProperty(const std::string& propertyName, vtkCodedEntry* codedEntry);
  void WriteStringPropertyIfNotEmpty(const std::string& propertyName, const std::string& propertyValue);
  void WriteBoolProperty(const std::string& propertyName, bool propertyValue);
  void WriteIntProperty(const std::string& propertyName, int propertyValue);
  void WriteDoubleProperty(const std::string& propertyName, double propertyValue);
  void WriteVectorProperty(const std::string& propertyName, double* v, int numberOfComponents = 3);
  void WriteMatrix4x4Property(const std::string& propertyName, double v[16], bool flipRasLps);
  void WriteDoubleArrayProperty(const char* propertyName, vtkDoubleArray* doubleArray);
  /// @}

  /// Utility function to convert string to lower case
  std::string toLower(const std::string& str);
  /// Utility functions to check string type
  bool isBool(const std::string& str);
  bool isInt(const std::string& str);
  bool isDouble(const std::string& str);

  /// Convert XML to JSON
  /// return string
  std::string ConvertXMLToJson(vtkXMLDataElement* xmlElement, const std::string& nodeTagName);

  /// Returns user-displayable messages that may contain details about any failed operation.
  vtkGetObjectMacro(UserMessages, vtkMRMLMessageCollection);

  /// Returns true if user messages contain error messages.
  bool HasErrors();

protected:
  vtkMRMLJsonWriter();
  ~vtkMRMLJsonWriter() override;
  vtkMRMLJsonWriter(const vtkMRMLJsonWriter&);
  void operator=(const vtkMRMLJsonWriter&);

  void processXMLElement(vtkXMLDataElement* xmlElement);

  vtkNew<vtkMRMLMessageCollection> UserMessages;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
