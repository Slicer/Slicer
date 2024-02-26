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

// Markups Module MRML storage nodes
//
// Helper objects for reading/writing markups from/to JSON file
//

#ifndef vtkMRMLMarkupsJsonElement_h
#define vtkMRMLMarkupsJsonElement_h

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMessageCollection.h"

#include "vtkSmartPointer.h"
#include "vtkNew.h"

#include <vector>

class vtkCodedEntry;

/// \brief Represents a json object or list.
///
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsJsonElement : public vtkObject
{
public:
  static vtkMRMLMarkupsJsonElement* New();
  vtkTypeMacro(vtkMRMLMarkupsJsonElement, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Get the JSON schema name
  std::string GetSchema();

  /// Returns true if the JSON object contains a member by this name.
  bool HasMember(const char* propertyName);

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
  vtkMRMLMarkupsJsonElement* GetArrayProperty(const char* arrayName);

  /// Get an object element from a property.
  /// If no such property is found or it is not the right type then nullptr is returned.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLMarkupsJsonElement* GetObjectProperty(const char* objectName);

  /// Returns true if this element is an array.
  bool IsArray();

  /// Returns the number of elements in this array.
  int GetArraySize();

  /// Returns the n-th elements in this array.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLMarkupsJsonElement* GetArrayItem(int childItemIndex);

  /// Returns user-displayable messages that may contain details about any failed operation.
  vtkGetObjectMacro(UserMessages, vtkMRMLMessageCollection);

  /// Returns true if user messages contain error messages.
  bool HasErrors();

protected:
  vtkMRMLMarkupsJsonElement();
  ~vtkMRMLMarkupsJsonElement() override;
  vtkMRMLMarkupsJsonElement(const vtkMRMLMarkupsJsonElement&);
  void operator=(const vtkMRMLMarkupsJsonElement&);

  vtkNew<vtkMRMLMessageCollection> UserMessages;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
  friend class vtkMRMLMarkupsJsonReader;
};

/// \brief Reads a JSON file into a vtkMRMLMarkupsJsonElement
///
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsJsonReader : public vtkObject
{
public:
  static vtkMRMLMarkupsJsonReader* New();
  vtkTypeMacro(vtkMRMLMarkupsJsonReader, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Read JSON document from file.
  /// \return JSON element on success and nullptr on failure.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLMarkupsJsonElement* ReadFromFile(const char* filePath);

  /// Returns user-displayable messages that may contain details about any failed operation.
  vtkGetObjectMacro(UserMessages, vtkMRMLMessageCollection);

  /// Returns true if user messages contain error messages.
  bool HasErrors();

protected:
  vtkMRMLMarkupsJsonReader();
  ~vtkMRMLMarkupsJsonReader() override;
  vtkMRMLMarkupsJsonReader(const vtkMRMLMarkupsJsonReader&);
  void operator=(const vtkMRMLMarkupsJsonReader&);

  vtkNew<vtkMRMLMessageCollection> UserMessages;
};

/// \brief Writes properties into a JSON file
///
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsJsonWriter : public vtkObject
{
public:
  static vtkMRMLMarkupsJsonWriter* New();
  vtkTypeMacro(vtkMRMLMarkupsJsonWriter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// This method must be called before writing any properties to the output file.
  /// Returns true on success.
  bool WriteToFileBegin(const char* filePath, const char* schema);

  /// This method must be called after writing all properties to the output file.
  /// Returns true on success.
  bool WriteToFileEnd();

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

  /// Returns user-displayable messages that may contain details about any failed operation.
  vtkGetObjectMacro(UserMessages, vtkMRMLMessageCollection);

  /// Returns true if user messages contain error messages.
  bool HasErrors();

protected:
  vtkMRMLMarkupsJsonWriter();
  ~vtkMRMLMarkupsJsonWriter() override;
  vtkMRMLMarkupsJsonWriter(const vtkMRMLMarkupsJsonWriter&);
  void operator=(const vtkMRMLMarkupsJsonWriter&);

  vtkNew<vtkMRMLMessageCollection> UserMessages;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
