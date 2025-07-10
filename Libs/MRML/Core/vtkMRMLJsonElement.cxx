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

// Relax JSON standard and allow reading/writing of nan and inf
// values. Such values should not normally occur, but if they do then
// it is easier to troubleshoot problems if numerical values are the
// same in memory and files.
// kWriteNanAndInfFlag = 2,        //!< Allow writing of Infinity, -Infinity and
// NaN.
#define RAPIDJSON_WRITE_DEFAULT_FLAGS 2
// kParseNanAndInfFlag = 256,      //!< Allow parsing NaN, Inf, Infinity, -Inf
// and -Infinity as doubles.
#define RAPIDJSON_PARSE_DEFAULT_FLAGS 256

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"

#include <vtkMRMLJsonElement_Private.h>

// VTK include
#include "vtkCommand.h"
#include "vtkDoubleArray.h"
#include <vtkObjectFactory.h>
#include <vtkXMLDataElement.h>

// MRML include
#include "vtkCodedEntry.h"
#include "vtkMRMLSubjectHierarchyNode.h"

vtkStandardNewMacro(vtkMRMLJsonElement);
vtkStandardNewMacro(vtkMRMLJsonReader);
vtkStandardNewMacro(vtkMRMLJsonWriter);

//----------------------------------------------------------------------------
const std::string vtkMRMLJsonElement::XML_SEPARATOR = std::string(";");
const std::string vtkMRMLJsonElement::XML_NAME_VALUE_SEPARATOR = std::string(":");

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLJsonElement::vtkInternal::vtkInternal(vtkMRMLJsonElement* external)
    : External(external) {}

//---------------------------------------------------------------------------
vtkMRMLJsonElement::vtkInternal::~vtkInternal() {}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::vtkInternal::ReadVector(rapidjson::Value &item,
                                                 double* v,
                                                 int numberOfComponents /*=3*/)
{
  if (!item.IsArray())
  {
    return false;
  }
  if (static_cast<int>(item.Size()) != numberOfComponents)
  {
    return false;
  }
  bool success = true;
  for (int index = 0; index < numberOfComponents; ++index)
  {
    if (!item[index].IsNumber())
    {
      success = false;
      continue;
    }
    v[index] = item[index].GetDouble();
  }
  return success;
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement::Type vtkMRMLJsonElement::vtkInternal::GetValueType(rapidjson::Value &item)
{
  if (item.IsObject())
  {
    return vtkMRMLJsonElement::Type::OBJECT;
  }
  else if (item.IsArray())
  {
    return vtkMRMLJsonElement::Type::ARRAY;
  }
  else if (item.IsString())
  {
    return vtkMRMLJsonElement::Type::STRING;
  }
  else if (item.IsBool())
  {
    return vtkMRMLJsonElement::Type::BOOL;
  }
  else if (item.IsInt() || item.IsUint() || //
           item.IsInt64() || item.IsUint64())
  {
    return vtkMRMLJsonElement::Type::INT;
  }
  else if (item.IsDouble())
  {
    return vtkMRMLJsonElement::Type::DOUBLE;
  }
  else
  {
    return vtkMRMLJsonElement::Type::UNKNOWN;
  }
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement::vtkMRMLJsonElement()
{
  this->Internal = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement::~vtkMRMLJsonElement()
{
  delete this->Internal;
  this->Internal = nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLJsonElement::PrintSelf(ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::HasMember(const char* propertyName)
{
  return this->Internal->JsonValue.HasMember(propertyName);
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement::Type vtkMRMLJsonElement::GetMemberType(const char* propertyName)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return vtkMRMLJsonElement::Type::UNKNOWN;
  }

  rapidjson::Value& member = this->Internal->JsonValue[propertyName];
  return this->Internal->GetValueType(member);
}

//----------------------------------------------------------------------------
std::string vtkMRMLJsonElement::GetStringProperty(const char* propertyName)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return "";
  }
  if (!this->Internal->JsonValue[propertyName].IsString())
  {
    return "";
  }
  return this->Internal->JsonValue[propertyName].GetString();
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::GetStringProperty(const char* propertyName,
                                           std::string& propertyValue)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  if (!this->Internal->JsonValue[propertyName].IsString())
  {
    return false;
  }
  propertyValue = this->Internal->JsonValue[propertyName].GetString();
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::GetDoubleProperty(const char* propertyName,
                                           double& propertyValue)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  if (!this->Internal->JsonValue[propertyName].IsNumber())
  {
    return false;
  }
  propertyValue = this->Internal->JsonValue[propertyName].GetDouble();
  return true;
}

//----------------------------------------------------------------------------
double vtkMRMLJsonElement::GetDoubleProperty(const char* propertyName)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return 0.0;
  }
  if (!this->Internal->JsonValue[propertyName].IsNumber())
  {
    return 0.0;
  }
  return this->Internal->JsonValue[propertyName].GetDouble();
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::GetIntProperty(const char* propertyName,
                                        int& propertyValue)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  if (!this->Internal->JsonValue[propertyName].IsInt()) {
    return false;
  }
  propertyValue = this->Internal->JsonValue[propertyName].GetInt();
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLJsonElement::GetIntProperty(const char* propertyName)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return 0;
  }
  if (!this->Internal->JsonValue[propertyName].IsInt())
  {
    return 0;
  }
  return this->Internal->JsonValue[propertyName].GetInt();
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::GetBoolProperty(const char* propertyName)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  return this->Internal->JsonValue[propertyName].GetBool();
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLJsonElement::GetCodedEntryProperty(const char* propertyName)
{
  std::vector<std::string> codedEntryVector;
  if (!this->GetStringVectorProperty(propertyName, codedEntryVector))
  {
    return nullptr;
  }
  if (codedEntryVector.size() < 2 || codedEntryVector.size() > 3)
  {
    // Full specification is code value, scheme, meaning.
    // Code meaning is optional.
    return nullptr;
  }
  vtkNew<vtkCodedEntry> codedEntry;
  codedEntry->SetValueSchemeMeaning(
      codedEntryVector[0], codedEntryVector[1],
      codedEntryVector.size() > 2 ? codedEntryVector[2] : "");
  codedEntry->Register(this);
  return codedEntry;
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::GetArrayItemsStringProperty(const char* arrayName,
                                                     const char* propertyName,
                                                     std::vector<std::string> &propertyValues)
{
  if (!this->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetArrayItemsStringProperty: invalid JSON object");
    return false;
  }
  if (!this->Internal->JsonValue.HasMember(arrayName))
  {
    vtkErrorMacro("GetArrayItemsStringProperty: " << arrayName
                                                  << " property is not found");
    return false;
  }
  rapidjson::Value &jsonArray = this->Internal->JsonValue[arrayName];
  if (!jsonArray.IsArray())
  {
    vtkErrorMacro("GetArrayItemsStringProperty: "
                  << arrayName << " property is not an array");
    return false;
  }
  int numberOfItems = jsonArray.GetArray().Size();
  for (int itemIndex = 0; itemIndex < numberOfItems; ++itemIndex)
  {
    rapidjson::Value &item = jsonArray.GetArray()[itemIndex];
    std::string stringProperty = item[propertyName].GetString();
    propertyValues.push_back(stringProperty);
  }
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement* vtkMRMLJsonElement::GetArrayProperty(const char* arrayName)
{
  if (!this->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetArrayProperty: invalid JSON object");
    return nullptr;
  }
  vtkNew<vtkMRMLJsonElement> jsonArray;
  if (!this->Internal->JsonValue.HasMember(arrayName))
  {
    return nullptr;
  }
  jsonArray->Internal->JsonValue = this->Internal->JsonValue[arrayName];
  if (!jsonArray->Internal->JsonValue.IsArray())
  {
    vtkErrorMacro("GetArrayProperty: "
                  << arrayName << " property is not found or not an array");
    return nullptr;
  }
  jsonArray->Internal->JsonRoot = this->Internal->JsonRoot;
  jsonArray->Register(this);
  return jsonArray;
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement* vtkMRMLJsonElement::GetObjectProperty(const char* objectName)
{
  if (!this->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetObjectProperty: invalid JSON object");
    return nullptr;
  }
  if (!this->Internal->JsonValue.HasMember(objectName))
  {
    vtkErrorMacro("GetObjectProperty: " << objectName
                                        << " property is not found");
    return nullptr;
  }
  vtkNew<vtkMRMLJsonElement> jsonObject;
  jsonObject->Internal->JsonValue = this->Internal->JsonValue[objectName];
  if (!jsonObject->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetObjectProperty: " << objectName
                                        << " property is not an array");
    return nullptr;
  }
  jsonObject->Internal->JsonRoot = this->Internal->JsonRoot;
  jsonObject->Register(this);
  return jsonObject;
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement* vtkMRMLJsonElement::GetArrayItem(int childItemIndex)
{
  int arraySize = this->GetArraySize();
  if (childItemIndex >= arraySize)
  {
    vtkErrorMacro("GetArrayItem: failed to child item " << childItemIndex
                                                        << " from element");
    return nullptr;
  }
  vtkNew<vtkMRMLJsonElement> jsonArray;
  jsonArray->Internal->JsonValue = this->Internal->JsonValue.GetArray()[childItemIndex];
  if (!jsonArray->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetArrayItem: failed to child item " << childItemIndex
                                                        << " is not an object");
    return nullptr;
  }
  jsonArray->Internal->JsonRoot = this->Internal->JsonRoot;
  jsonArray->Register(this);
  return jsonArray;
}

//----------------------------------------------------------------------------
int vtkMRMLJsonElement::GetArraySize()
{
  if (!this->Internal->JsonValue.IsArray())
  {
    return 0;
  }
  return static_cast<int>(this->Internal->JsonValue.GetArray().Size());
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::IsArray()
{
  return this->Internal->JsonValue.IsArray();
}

//----------------------------------------------------------------------------
std::string vtkMRMLJsonElement::GetObjectPropertyNameByIndex(int childItemIndex)
{
  if (!this->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetObjectPropertyNameByIndex: JsonValue is not an object");
    return "";
  }
  if (childItemIndex < 0 || childItemIndex >= static_cast<int>(this->Internal->JsonValue.MemberCount()))
  {
    vtkErrorMacro("GetObjectPropertyNameByIndex: index out of range");
    return "";
  }
  rapidjson::Value::MemberIterator itr = this->Internal->JsonValue.MemberBegin();
  itr = itr + childItemIndex;
  return itr->name.GetString();
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement* vtkMRMLJsonElement::GetObjectItem(int childItemIndex)
{
  int obejctSize = this->GetObjectSize();
  if (childItemIndex >= obejctSize)
  {
    vtkErrorMacro("GetObejctItem: failed to child item " << childItemIndex
                                                        << " from element");
    return nullptr;
  }
  vtkNew<vtkMRMLJsonElement> jsonObject;
  rapidjson::Value::MemberIterator itr = this->Internal->JsonValue.MemberBegin();
  itr = itr + childItemIndex;
  jsonObject->Internal->JsonValue = itr->value;
  if (!jsonObject->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetObejctItem: failed to child item " << childItemIndex
                                                        << " is not an object");
    return nullptr;
  }
  jsonObject->Internal->JsonRoot = this->Internal->JsonRoot;
  jsonObject->Register(this);
  return jsonObject;
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement::Type vtkMRMLJsonElement::GetType()
{
  return this->Internal->GetValueType(this->Internal->JsonValue);
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement* vtkMRMLJsonElement::GetChildMemberItem(int childItemIndex)
{
  int obejctSize = this->GetObjectSize();
  if (childItemIndex >= obejctSize)
  {
    vtkErrorMacro("GetObejctItem: failed to child item " << childItemIndex
                                                        << " from element");
    return nullptr;
  }
  vtkNew<vtkMRMLJsonElement> jsonObject;
  rapidjson::Value::MemberIterator itr = this->Internal->JsonValue.MemberBegin();
  itr = itr + childItemIndex;
  jsonObject->Internal->JsonValue = itr->value;
  jsonObject->Internal->JsonRoot = this->Internal->JsonRoot;
  jsonObject->Register(this);
  return jsonObject;
}

//----------------------------------------------------------------------------
int vtkMRMLJsonElement::GetObjectSize()
{
  if (!this->Internal->JsonValue.IsObject())
  {
    return 0;
  }

  return static_cast<int>(this->Internal->JsonValue.GetObject().MemberCount());
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::IsObject()
{
  return this->Internal->JsonValue.IsObject();
}

//----------------------------------------------------------------------------
std::string vtkMRMLJsonElement::GetSchema()
{
  if (!this->Internal->JsonValue.HasMember("@schema"))
  {
    return "";
  }
  rapidjson::Value &schema = this->Internal->JsonValue["@schema"];
  return schema.GetString();
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::GetVectorProperty(const char* propertyName, double* v,
                                           int numberOfComponents /*=3*/)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  rapidjson::Value &item = this->Internal->JsonValue[propertyName];
  return this->Internal->ReadVector(item, v, numberOfComponents);
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::GetMatrix4x4Property(const char* propertyName,
                                              double v[16], bool flipRasLps)
{
  if (!this->GetVectorProperty(propertyName, v, 16))
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(),
      "vtkMRMLPlaneJsonStorageNode::vtkInternal::"
      "UpdateNodeFromJsonValue",
      "File reading failed: " << propertyName
                              << " 16 - element numeric array.");
    return false;
  }

  if (flipRasLps)
  {
    for (int i = 0; i < 8; i++)
    {
      v[i] = -v[i];
    }
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::GetStringVectorProperty(const char* propertyName,
                                                 std::vector<std::string> &arrayValues)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  rapidjson::Value &item = this->Internal->JsonValue[propertyName];
  if (!item.IsArray())
  {
    return false;
  }
  arrayValues.resize(item.Size());
  bool success = true;
  for (unsigned int i = 0; i < item.Size(); i++)
  {
    if (!item[i].IsString())
    {
      success = false;
      continue;
    }
    arrayValues[i] = item[i].GetString();
  }
  return success;
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonElement::HasErrors()
{
  return this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0;
}

//----------------------------------------------------------------------------
vtkDoubleArray* vtkMRMLJsonElement::GetDoubleArrayProperty(const char* propertyName)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return nullptr;
  }
  rapidjson::Value &arrayItem = this->Internal->JsonValue[propertyName];
  if (!arrayItem.IsArray())
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(),
      "vtkMRMLJsonElement::GetDoubleArrayProperty",
      "Property "
        << propertyName
        << " is expected to contain an array of floating-point numbers");
    return nullptr;
  }
  vtkNew<vtkDoubleArray> values;
  int numberOfTuples = arrayItem.GetArray().Size();
  if (numberOfTuples < 1)
  {
    // no values stored in the array
    return nullptr;
  }
  rapidjson::Value &firstControlPointValue = arrayItem.GetArray()[0];
  if (firstControlPointValue.IsNumber())
  {
    values->SetNumberOfValues(numberOfTuples);
    double* valuesPtr = values->GetPointer(0);
    bool success =
        this->GetVectorProperty(propertyName, valuesPtr, numberOfTuples);
    if (!success)
    {
      vtkErrorToMessageCollectionWithObjectMacro(
        this, this->GetUserMessages(),
        "vtkMRMLJsonIO::GetDoubleArrayProperty",
        "Property "
          << propertyName
          << " is expected to contain an array of floating-point numbers");
      return nullptr;
    }
  }
  else if (firstControlPointValue.IsArray())
  {
    int numberOfComponents = firstControlPointValue.GetArray().Size();
    values->SetNumberOfComponents(numberOfComponents);
    values->SetNumberOfTuples(numberOfTuples);
    double* valuesPtr = values->GetPointer(0);
    for (auto &itemValue : arrayItem.GetArray())
    {
      bool success = this->Internal->ReadVector(itemValue, valuesPtr, numberOfComponents);
      if (!success)
      {
        vtkErrorToMessageCollectionWithObjectMacro(
          this, this->GetUserMessages(),
          "vtkMRMLJsonIO::GetDoubleArrayProperty",
          "Property "
            << propertyName
            << " is expected to contain an array of floating-point numbers"
            << " with the same number of components");
        return nullptr;
      }
      valuesPtr += numberOfComponents;
    }
  }
  else
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(),
      "vtkMRMLJsonIO::GetDoubleArrayProperty",
      "Property "
        << propertyName
        << " is expected to contain an array of floating-point numbers"
        << " or array of arrays of floating-point numbers");
    return nullptr;
  }
  values->Register(this);
  return values;
}

//----------------------------------------------------------------------------
vtkMRMLJsonReader::vtkMRMLJsonReader() {}

//----------------------------------------------------------------------------
vtkMRMLJsonReader::~vtkMRMLJsonReader() {}

//----------------------------------------------------------------------------
void vtkMRMLJsonReader::PrintSelf(ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonReader::HasErrors()
{
  return this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0;
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement* vtkMRMLJsonReader::ReadFromFile(const char* filePath)
{
  if (!filePath)
  {
    vtkErrorToMessageCollectionWithObjectMacro(this, this->GetUserMessages(),
      "vtkMRMLJsonIO::ReadFromFile",
      "Invalid filename");
    return nullptr;
  }

  // Read document from file
  FILE* fp = fopen(filePath, "r");
  if (!fp)
  {
    vtkErrorToMessageCollectionWithObjectMacro(this, this->GetUserMessages(),
      "vtkMRMLJsonIO::ReadFromFile",
      "Error opening the file '" << filePath << "'");
    return nullptr;
  }

  vtkNew<vtkMRMLJsonElement> jsonElement;

  jsonElement->Internal->JsonRoot = std::make_shared<vtkMRMLJsonElement::vtkInternal::JsonDocumentContainer>();

  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (jsonElement->Internal->JsonRoot->Document->ParseStream(fs).HasParseError())
  {
    vtkErrorToMessageCollectionWithObjectMacro(this, this->GetUserMessages(),
      "vtkMRMLJsonIO::ReadFromFile",
      "Error parsing the file '" << filePath << "'");
    fclose(fp);
    return nullptr;
  }
  fclose(fp);

  if (jsonElement->Internal->JsonRoot->Document->IsObject())
  {
    jsonElement->Internal->JsonValue = jsonElement->Internal->JsonRoot->Document->GetObject();
  }
  else if (jsonElement->Internal->JsonRoot->Document->IsArray())
  {
    jsonElement->Internal->JsonValue = jsonElement->Internal->JsonRoot->Document->GetArray();
  }
  else
  {
    vtkErrorToMessageCollectionWithObjectMacro(this, this->GetUserMessages(),
      "vtkMRMLJsonIO::ReadFromFile",
      "Error parsing the file '" << filePath << "' - root item must be array or list");
    return nullptr;
  }

  jsonElement->Register(this);
  return jsonElement;
}

//----------------------------------------------------------------------------
vtkMRMLJsonElement* vtkMRMLJsonReader::ReadFromString(const std::string& jsonString)
{
  if (jsonString.empty())
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(), "vtkMRMLJsonIO::ReadFromString",
      "Invalid JSON string");
    return nullptr;
  }

  vtkNew<vtkMRMLJsonElement> jsonElement;

  jsonElement->Internal->JsonRoot = std::make_shared<
      vtkMRMLJsonElement::vtkInternal::JsonDocumentContainer>();

  if (jsonElement->Internal->JsonRoot->Document->Parse(jsonString.c_str()).HasParseError())
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(), "vtkMRMLJsonIO::ReadFromString",
      "Error parsing the JSON string");
    return nullptr;
  }

  if (jsonElement->Internal->JsonRoot->Document->IsObject())
  {
    jsonElement->Internal->JsonValue =
      jsonElement->Internal->JsonRoot->Document->GetObject();
  }
  else if (jsonElement->Internal->JsonRoot->Document->IsArray())
  {
    jsonElement->Internal->JsonValue =
      jsonElement->Internal->JsonRoot->Document->GetArray();
  }
  else
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(), "vtkMRMLJsonIO::ReadFromString",
      "Error parsing the JSON string - root item must be array or list");
    return nullptr;
  }

  jsonElement->Register(this);
  return jsonElement;
}

//---------------------------------------------------------------------------
std::string vtkMRMLJsonReader::ConvertJsonToXML(const std::string& jsonString, const std::string& nodeTagName)
{
  vtkSmartPointer<vtkMRMLJsonElement> jsonElement = vtkSmartPointer<vtkMRMLJsonElement>::Take
    (this->ReadFromString(jsonString));
  if (!jsonElement.GetPointer())
  {
    return "";
  }

  if (jsonElement->GetObjectPropertyNameByIndex(0) != nodeTagName)
  {
    return "";
  }

  return this->processJsonElement(jsonElement);
}

//---------------------------------------------------------------------------
std::string vtkMRMLJsonReader::processJsonElement(vtkMRMLJsonElement* jsonElement, const std::string& elementKey/*=""*/)
{
  if (!jsonElement)
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(), "vtkMRMLJsonIO::processJsonElement",
      "Error parsing the JSON string - vtkMRMLJsonElement is null");
    return "";
  }

  std::ostringstream xmlStream;
  if (!elementKey.empty())
  {
    xmlStream << "<" << elementKey;
  }

  int numberOfMembers = jsonElement->GetObjectSize();
  for (int memberIndex = 0; memberIndex < numberOfMembers; ++memberIndex)
  {
    std::string memberKey = jsonElement->GetObjectPropertyNameByIndex(memberIndex);
    vtkMRMLJsonElement::Type type = jsonElement->GetMemberType(memberKey.c_str());
    if (type == vtkMRMLJsonElement::Type::UNKNOWN)
    {
      continue;
    }
    else if (type == vtkMRMLJsonElement::Type::OBJECT)
    {
      if ((memberKey != "attributes" && memberKey != "uids" && memberKey != "references"))
      {
        continue;
      }

      vtkSmartPointer<vtkMRMLJsonElement> memberElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonElement->GetObjectItem(memberIndex));
      if (!memberElement)
      {
        continue;
      }
      std::ostringstream oss;
      int numberOfChildMembers = memberElement->GetObjectSize();

      std::string delimiter = vtkMRMLJsonElement::XML_SEPARATOR;
      std::string valueDelimiter = vtkMRMLJsonElement::XML_NAME_VALUE_SEPARATOR;
      if (elementKey == "SubjectHierarchyItem")
      {
        delimiter = vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR; // SubjectHierarchyItem uses | as delimiter
        valueDelimiter = vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR; // SubjectHierarchyItem uses ^ as valueDelimiter
      }

      for (int childIndex = 0; childIndex < numberOfChildMembers; ++childIndex)
      {
        if (childIndex != 0)
        {
          oss << delimiter;
        }
        std::string childPropName = memberElement->GetObjectPropertyNameByIndex(childIndex);
        if (memberElement->GetStringProperty(childPropName.c_str()) != "")
        {
          oss << childPropName << valueDelimiter << memberElement->GetStringProperty(childPropName.c_str());
        }
        else
        {
          std::vector<std::string> arrayValues;
          memberElement->GetStringVectorProperty(childPropName.c_str(), arrayValues);
          oss << childPropName << valueDelimiter;
          for (size_t arrayIndex = 0; arrayIndex < arrayValues.size(); ++arrayIndex)
          {
            if (arrayIndex > 0)
            {
              oss << " ";
            }
            oss << arrayValues[arrayIndex];
          }
        }
      }
      xmlStream << " " << memberKey << "=\"" << oss.str() << "\"";
    }
    else if (type == vtkMRMLJsonElement::Type::ARRAY)
    {
      vtkSmartPointer<vtkMRMLJsonElement> arrayElement = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonElement->GetChildMemberItem(memberIndex));
      if (!arrayElement)
      {
        continue;
      }
      if (arrayElement->GetType() != vtkMRMLJsonElement::Type::ARRAY)
      {
        continue;
      }

      std::ostringstream arrayStream;
      rapidjson::Value::ValueIterator itr;
      for (itr = arrayElement->Internal->JsonValue.Begin(); itr != arrayElement->Internal->JsonValue.End(); ++itr)
      {
        if (itr != arrayElement->Internal->JsonValue.Begin())
        {
          arrayStream << " ";
        }

        if (itr->IsDouble())
        {
          arrayStream << itr->GetDouble();
        }
        else if (itr->IsString())
        {
          arrayStream << itr->GetString();
        }
        else if (itr->IsInt())
        {
          arrayStream << itr->GetInt();
        }
        else if (itr->IsUint())
        {
          arrayStream << itr->GetUint();
        }
        else if (itr->IsInt64())
        {
          arrayStream << itr->GetInt64();
        }
        else if (itr->IsUint64())
        {
          arrayStream << itr->GetUint64();
        }
      }
      xmlStream << " " << memberKey << "=\"" << arrayStream.str() << "\"";
    }
    else
    {
      xmlStream << " " << memberKey << "=\"";
      if (type == vtkMRMLJsonElement::Type::STRING)
      {
        xmlStream << jsonElement->GetStringProperty(memberKey.c_str());
      }
      else if (type == vtkMRMLJsonElement::Type::BOOL)
      {
        xmlStream << (jsonElement->GetBoolProperty(memberKey.c_str()) ? "true" : "false");
      }
      else if (type == vtkMRMLJsonElement::Type::INT)
      {
        xmlStream << jsonElement->GetIntProperty(memberKey.c_str());
      }
      else if (type == vtkMRMLJsonElement::Type::DOUBLE)
      {
        xmlStream << jsonElement->GetDoubleProperty(memberKey.c_str());
      }
      xmlStream << "\"";
    }
  }

  if (!elementKey.empty())
  {
    xmlStream << ">";
  }

  for (int memberIndex = 0; memberIndex < numberOfMembers; ++memberIndex)
  {
    vtkSmartPointer<vtkMRMLJsonElement> childMemberElement = vtkSmartPointer<vtkMRMLJsonElement>::Take
      (jsonElement->GetChildMemberItem(memberIndex));
    if (!childMemberElement || childMemberElement->GetType() != vtkMRMLJsonElement::Type::OBJECT)
    {
      continue;
    }

    std::string memberKey = jsonElement->GetObjectPropertyNameByIndex(memberIndex);
    xmlStream << this->processJsonElement(childMemberElement, memberKey);
  }

  if (!elementKey.empty())
  {
    xmlStream << "</" << elementKey << ">";
  }
  return xmlStream.str();
 }

//---------------------------------------------------------------------------
vtkMRMLJsonWriter::vtkInternal::vtkInternal(vtkMRMLJsonWriter* external) : External(external)
{
  this->WriteBuffer.resize(65536);
}

//---------------------------------------------------------------------------
vtkMRMLJsonWriter::vtkInternal::~vtkInternal() {}

vtkMRMLJsonWriter::vtkMRMLJsonWriter()
{
  this->Internal = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkMRMLJsonWriter::~vtkMRMLJsonWriter()
{
  delete this->Internal;
  this->Internal = nullptr;
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonWriter::HasErrors()
{
  return this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0;
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::PrintSelf(ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::vtkInternal::WriteVector(double* v, int numberOfComponents /*=3*/)
{
  this->Writer->SetFormatOptions(rapidjson::kFormatSingleLineArray);
  this->Writer->StartArray();
  for (int i = 0; i < numberOfComponents; i++)
  {
    this->Writer->Double(v[i]);
  }
  this->Writer->EndArray();
  this->Writer->SetFormatOptions(rapidjson::kFormatDefault);
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonWriter::WriteToFileBegin(const char* filePath, const char* schema)
{
  if (!filePath)
  {
    vtkErrorToMessageCollectionWithObjectMacro(this, this->GetUserMessages(),
      "vtkMRMLJsonIO::WriteToFileBegin",
      "Invalid filename");
    return false;
  }

  // open the file for writing
  this->Internal->WriteFileHandle = fopen(filePath, "wb");
  if (!this->Internal->WriteFileHandle)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLJsonStorageNode::WriteDataInternal",
      "Writing  node file failed: unable to open file '" << filePath << "' for writing.");
    return false;
  }

  // Prepare JSON writer and output stream.
  this->Internal->FileWriteStream = std::unique_ptr<rapidjson::FileWriteStream>(
    new rapidjson::FileWriteStream(this->Internal->WriteFileHandle, &this->Internal->WriteBuffer[0], this->Internal->WriteBuffer.size()));

  auto stringWriter = std::unique_ptr<rapidjson::PrettyWriter<rapidjson::FileWriteStream>>(
    new rapidjson::PrettyWriter<rapidjson::FileWriteStream> (*this->Internal->FileWriteStream));
  this->Internal->SetFileWriter(std::move(stringWriter));

  this->Internal->Writer->StartObject();

  if (schema)
  {
    this->WriteStringProperty("@schema", schema);
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonWriter::WriteToFileEnd()
{
  this->Internal->Writer->EndObject();
  int error = fclose(this->Internal->WriteFileHandle);
  this->Internal->WriteFileHandle = 0;
  this->Internal->Writer.reset();
  this->Internal->FileWriteStream.reset();
  return (error == 0);
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonWriter::WriteToStringBegin(const char* nodeTagName)
{
  if (!nodeTagName)
  {
    return false;
  }

  // Prepare JSON writer and output stream.
  this->Internal->StringBuffer = std::make_unique<rapidjson::StringBuffer>();
  auto stringWriter = std::make_unique<rapidjson::PrettyWriter<rapidjson::StringBuffer>>(*this->Internal->StringBuffer);
  this->Internal->SetStringWriter(std::move(stringWriter));

  this->Internal->Writer->StartObject();
  this->Internal->Writer->Key(nodeTagName);
  this->Internal->Writer->StartObject();

  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLJsonWriter::WriteToStringEnd()
{
  this->Internal->Writer->EndObject();
  this->Internal->Writer->EndObject();
  std::string jsonString = this->Internal->StringBuffer->GetString();
  this->Internal->Writer.reset();
  this->Internal->StringBuffer.reset();
  return jsonString;
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteStringProperty(const std::string& propertyName,
                                            const std::string& propertyValue)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->String(propertyValue.c_str());
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteStringVectorProperty(const std::string& propertyName,
                                                  const std::vector<std::string> &arrayValues)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->StartArray();
  for (const std::string& value : arrayValues)
  {
    this->Internal->Writer->String(value.c_str());
  }
  this->Internal->Writer->EndArray();
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteStringPropertyIfNotEmpty(const std::string& propertyName,
                                                      const std::string& propertyValue)
{
  if (propertyValue.empty())
  {
    return;
  }
  this->WriteStringProperty(propertyName, propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteCodedEntryProperty(const std::string& propertyName,
                                                vtkCodedEntry* codedEntry)
{
  if (!codedEntry)
  {
    return;
  }

  // Coded entry is made up of 3 short strings, it is nicer to write them in a
  // single line
  this->Internal->Writer->SetFormatOptions(rapidjson::kFormatSingleLineArray);

  this->WriteStringVectorProperty(propertyName,
                                  codedEntry->GetValueSchemeMeaning());

  this->Internal->Writer->SetFormatOptions(rapidjson::kFormatDefault);
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteBoolProperty(const std::string& propertyName,
                                          bool propertyValue)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->Bool(propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteIntProperty(const std::string& propertyName,
                                         int propertyValue)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->Int(propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteDoubleProperty(const std::string& propertyName,
                                            double propertyValue)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->Double(propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteVectorProperty(const std::string& propertyName,
                                            double* v,
                                            int numberOfComponents /*=3*/)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->WriteVector(v, numberOfComponents);
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteMatrix4x4Property(const std::string& propertyName,
                                               double v[16], bool flipRasLps)
{
  if (flipRasLps)
  {
    double vFlipped[16] = {0.0};
    for (int i = 0; i < 8; ++i)
    {
      vFlipped[i] = -v[i];
    }
    for (int i = 8; i < 16; ++i)
    {
      vFlipped[i] = v[i];
    }
    this->WriteVectorProperty(propertyName, vFlipped, 16);
  }
  else
  {
    this->WriteVectorProperty(propertyName, v, 16);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteDoubleArrayProperty(const char* propertyName,
                                                 vtkDoubleArray* doubleArray)
{
  this->WriteArrayPropertyStart(propertyName);
  int numberOfComponents = doubleArray->GetNumberOfComponents();
  int numberOfTuples = doubleArray->GetNumberOfTuples();
  if (numberOfComponents == 1)
  {
    // write single-component array as single array
    double* values = doubleArray->GetPointer(0);
    // WriteVector() method would write all values in a single line, so we do
    // not use it here
    for (int tupleIndex = 0; tupleIndex < numberOfTuples; ++tupleIndex)
    {
      this->Internal->Writer->Double(values[tupleIndex]);
    }
  }
  else
  {
    // write multi-component array as an array of arrays
    for (int tupleIndex = 0; tupleIndex < numberOfTuples; ++tupleIndex)
    {
      double* tuple = doubleArray->GetTuple(tupleIndex);
      this->Internal->WriteVector(tuple, numberOfComponents);
    }
  }
  this->WriteArrayPropertyEnd();
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteArrayPropertyStart(const std::string& propertyName)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->StartArray();
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteArrayPropertyEnd() {
  this->Internal->Writer->EndArray();
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteObjectPropertyStart(const std::string& propertyName)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->StartObject();
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteObjectPropertyEnd()
{
  this->Internal->Writer->EndObject();
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteObjectStart()
{
  this->Internal->Writer->StartObject();
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::WriteObjectEnd()
{
  this->Internal->Writer->EndObject();
}

//----------------------------------------------------------------------------
std::string vtkMRMLJsonWriter::ConvertXMLToJson(vtkXMLDataElement* xmlElement, const std::string& nodeTagName)
{
  if (!this->WriteToStringBegin(nodeTagName.c_str()))
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(), "vtkMRMLJsonIO::ConvertXMLToJson",
      "Error initialization JSON writer - nodeTagName must be provided");
    return "";
  }

  this->processXMLElement(xmlElement);

  return this->WriteToStringEnd();
}

//----------------------------------------------------------------------------
std::string vtkMRMLJsonWriter::toLower(const std::string& str)
{
  std::string lowerStr;
  for (unsigned char c : str)
  {
    if (std::isalpha(c))
    {
      lowerStr += std::tolower(c); // Convert to lowercase
    }
    else
    {
      lowerStr += c;
    }
  }
  return lowerStr;
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonWriter::isBool(const std::string& str)
{
  std::stringstream ss(str);
  std::string lowerStr = toLower(str);
  if (lowerStr == "true" || lowerStr == "false")
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonWriter::isInt(const std::string& str)
{
  std::stringstream ss(str);
  int intVal;
  char leftover;
  if (ss >> intVal)
  {
    if (!(ss >> leftover))
    {
      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLJsonWriter::isDouble(const std::string& str)
{
  std::stringstream ss(str);
  double doubleVal;
  char leftover;
  if (ss >> doubleVal)
  {
    if (!(ss >> leftover))
    {
      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
void vtkMRMLJsonWriter::processXMLElement(vtkXMLDataElement* xmlElement)
{
  int nAttributes = xmlElement->GetNumberOfAttributes();
  for (int attrIndex = 0; attrIndex < nAttributes; ++attrIndex)
  {
    const char* name = xmlElement->GetAttributeName(attrIndex);
    const char* value = xmlElement->GetAttributeValue(attrIndex);
    std::string valueStr = value ? std::string(value) : "";

    if (std::string(name) == "attributes" || //
        std::string(name) == "uids" || //
        std::string(name) == "references")
    {
      std::map<std::string, std::vector<std::string>> valuesMap;

      std::stringstream ss(valueStr);
      std::string lineValue;
      std::string delimiter = vtkMRMLJsonElement::XML_SEPARATOR;
      std::string valueDelimiter = vtkMRMLJsonElement::XML_NAME_VALUE_SEPARATOR;
      if (valueStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR) != std::string::npos && //
          valueStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR) != std::string::npos)
      {
        delimiter = vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR; // SubjectHierarchyItem uses | as delimiter
        valueDelimiter = vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR; // SubjectHierarchyItem uses ^ as valueDelimiter
      }
      while (std::getline(ss, lineValue, delimiter[0]))
      {
        int colonIndex = lineValue.find(valueDelimiter);
        std::string attributeName = lineValue.substr(0, colonIndex);
        std::string attributeValue = lineValue.substr(colonIndex + 1);
        vtksys::SystemTools::ReplaceString(attributeValue, "%", "%25");
        vtksys::SystemTools::ReplaceString(attributeValue, ";", "%3B");
        std::stringstream valueStream(attributeValue);
        std::string singleValue;

        while (std::getline(valueStream, singleValue, ' '))
        {
          std::string nextValue;
          std::getline(valueStream, nextValue, ' ');
          if (nextValue.front() == '{' && nextValue.back() == '}')
          {
            // for references: merge references properties to the node id
            // e.g. "vtkMRMLSliceNodeRed {clippingState=ClipPositiveSpace} vtkMRMLSliceNodeGreen {clippingState=ClipPositiveSpace}"
            valuesMap[attributeName].push_back(singleValue + " " + nextValue);
          }
          else
          {
            valueStream.seekg(-nextValue.size(), std::ios_base::cur);
            valuesMap[attributeName].push_back(singleValue);
          }

        }
      }
      this->WriteObjectPropertyStart(name);

      std::map<std::string, std::vector<std::string>>::const_iterator entry;
      for (entry = valuesMap.begin(); entry != valuesMap.end(); ++entry)
      {
        if (entry->second.size() == 1)
        {
          this->WriteStringProperty(entry->first, entry->second[0]);
        }
        else
        {
          this->WriteStringVectorProperty(entry->first, entry->second);
        }
      }
      this->WriteObjectPropertyEnd();
    }
    // Check if it a double array
    else if (std::count(valueStr.begin(), valueStr.end(), ' ') > 0 && //
      std::all_of(valueStr.begin(), valueStr.end(), [](char c) { return std::isdigit(c) || c == ' ' || c == '-' || c == '.';}))
    {
      vtkNew<vtkDoubleArray> valueVector;
      std::istringstream iss(valueStr);
      std::string item;
      while (iss >> item)
      {
        valueVector->InsertNextValue(std::strtod(item.c_str(), nullptr));
      }
      this->WriteDoubleArrayProperty(name, valueVector);
    }
    else
    {
      if (this->isInt(value))
      {
        this->WriteIntProperty(name, std::atoi(value));
      }
      else if (this->isDouble(value))
      {
        this->WriteDoubleProperty(name, std::atof(value));
      }
      else if (this->isBool(value))
      {
        this->WriteBoolProperty(name, this->toLower(value) == "true");
      }
      else
      {
        this->WriteStringProperty(name, value ? value : "");
      }
    }
  }

  for (int childIndex = 0; childIndex < xmlElement->GetNumberOfNestedElements(); ++childIndex)
  {
    vtkXMLDataElement* childElement = xmlElement->GetNestedElement(childIndex);
    this->WriteObjectPropertyStart(childElement->GetName());
    this->processXMLElement(childElement);
    this->WriteObjectPropertyEnd();
  }
}
