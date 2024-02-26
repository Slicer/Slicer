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
// kWriteNanAndInfFlag = 2,        //!< Allow writing of Infinity, -Infinity and NaN.
#define RAPIDJSON_WRITE_DEFAULT_FLAGS 2
// kParseNanAndInfFlag = 256,      //!< Allow parsing NaN, Inf, Infinity, -Inf and -Infinity as doubles.
#define RAPIDJSON_PARSE_DEFAULT_FLAGS 256

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

#include <vtkMRMLMarkupsJsonElement_Private.h>

// VTK include
#include "vtkCommand.h"
#include "vtkDoubleArray.h"
#include <vtkObjectFactory.h>

// MRML include
#include "vtkCodedEntry.h"

vtkStandardNewMacro(vtkMRMLMarkupsJsonElement);
vtkStandardNewMacro(vtkMRMLMarkupsJsonReader);
vtkStandardNewMacro(vtkMRMLMarkupsJsonWriter);

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLMarkupsJsonElement::vtkInternal::vtkInternal(vtkMRMLMarkupsJsonElement* external)
  : External(external)
{
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsJsonElement::vtkInternal::~vtkInternal() {}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonElement::vtkInternal::ReadVector(rapidjson::Value& item,
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
  for (int i = 0; i < numberOfComponents; i++)
  {
    if (!item[i].IsDouble())
    {
      success = false;
      continue;
    }
    v[i] = item[i].GetDouble();
  }
  return success;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonElement::vtkMRMLMarkupsJsonElement()
{
  this->Internal = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonElement::~vtkMRMLMarkupsJsonElement()
{
  delete this->Internal;
  this->Internal = nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonElement::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonElement::HasMember(const char* propertyName)
{
  return this->Internal->JsonValue.HasMember(propertyName);
}

//----------------------------------------------------------------------------
std::string vtkMRMLMarkupsJsonElement::GetStringProperty(const char* propertyName)
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
bool vtkMRMLMarkupsJsonElement::GetStringProperty(const char* propertyName, std::string& propertyValue)
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
bool vtkMRMLMarkupsJsonElement::GetDoubleProperty(const char* propertyName, double& propertyValue)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  if (!this->Internal->JsonValue[propertyName].IsDouble())
  {
    return false;
  }
  propertyValue = this->Internal->JsonValue[propertyName].GetDouble();
  return true;
}

//----------------------------------------------------------------------------
double vtkMRMLMarkupsJsonElement::GetDoubleProperty(const char* propertyName)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return 0.0;
  }
  if (!this->Internal->JsonValue[propertyName].IsDouble())
  {
    return 0.0;
  }
  return this->Internal->JsonValue[propertyName].GetDouble();
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonElement::GetIntProperty(const char* propertyName, int& propertyValue)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  if (!this->Internal->JsonValue[propertyName].IsInt())
  {
    return false;
  }
  propertyValue = this->Internal->JsonValue[propertyName].GetInt();
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsJsonElement::GetIntProperty(const char* propertyName)
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
bool vtkMRMLMarkupsJsonElement::GetBoolProperty(const char* propertyName)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  return this->Internal->JsonValue[propertyName].GetBool();
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLMarkupsJsonElement::GetCodedEntryProperty(const char* propertyName)
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
    codedEntryVector[0], codedEntryVector[1], codedEntryVector.size() > 2 ? codedEntryVector[2] : "");
  codedEntry->Register(this);
  return codedEntry;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonElement::GetArrayItemsStringProperty(const char* arrayName,
                                                            const char* propertyName,
                                                            std::vector<std::string>& propertyValues)
{
  if (!this->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetArrayItemsStringProperty: invalid JSON object");
    return false;
  }
  if (!this->Internal->JsonValue.HasMember(arrayName))
  {
    vtkErrorMacro("GetArrayItemsStringProperty: " << arrayName << " property is not found");
    return false;
  }
  rapidjson::Value& jsonArray = this->Internal->JsonValue[arrayName];
  if (!jsonArray.IsArray())
  {
    vtkErrorMacro("GetArrayItemsStringProperty: " << arrayName << " property is not an array");
    return false;
  }
  int numberOfItems = jsonArray.GetArray().Size();
  for (int itemIndex = 0; itemIndex < numberOfItems; ++itemIndex)
  {
    rapidjson::Value& item = jsonArray.GetArray()[itemIndex];
    std::string stringProperty = item[propertyName].GetString();
    propertyValues.push_back(stringProperty);
  }
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonElement* vtkMRMLMarkupsJsonElement::GetArrayProperty(const char* arrayName)
{
  if (!this->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetArrayProperty: invalid JSON object");
    return nullptr;
  }
  vtkNew<vtkMRMLMarkupsJsonElement> jsonArray;
  if (!this->Internal->JsonValue.HasMember(arrayName))
  {
    return nullptr;
  }
  jsonArray->Internal->JsonValue = this->Internal->JsonValue[arrayName];
  if (!jsonArray->Internal->JsonValue.IsArray())
  {
    vtkErrorMacro("GetArrayProperty: " << arrayName << " property is not found or not an array");
    return nullptr;
  }
  jsonArray->Internal->JsonRoot = this->Internal->JsonRoot;
  jsonArray->Register(this);
  return jsonArray;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonElement* vtkMRMLMarkupsJsonElement::GetObjectProperty(const char* objectName)
{
  if (!this->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetObjectProperty: invalid JSON object");
    return nullptr;
  }
  if (!this->Internal->JsonValue.HasMember(objectName))
  {
    vtkErrorMacro("GetObjectProperty: " << objectName << " property is not found");
    return nullptr;
  }
  vtkNew<vtkMRMLMarkupsJsonElement> jsonObject;
  jsonObject->Internal->JsonValue = this->Internal->JsonValue[objectName];
  if (!jsonObject->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetObjectProperty: " << objectName << " property is not an array");
    return nullptr;
  }
  jsonObject->Internal->JsonRoot = this->Internal->JsonRoot;
  jsonObject->Register(this);
  return jsonObject;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonElement* vtkMRMLMarkupsJsonElement::GetArrayItem(int childItemIndex)
{
  int arraySize = this->GetArraySize();
  if (childItemIndex >= arraySize)
  {
    vtkErrorMacro("GetArrayItem: failed to child item " << childItemIndex << " from element");
    return nullptr;
  }
  vtkNew<vtkMRMLMarkupsJsonElement> jsonArray;
  jsonArray->Internal->JsonValue = this->Internal->JsonValue.GetArray()[childItemIndex];
  if (!jsonArray->Internal->JsonValue.IsObject())
  {
    vtkErrorMacro("GetArrayItem: failed to child item " << childItemIndex << " is not an object");
    return nullptr;
  }
  jsonArray->Internal->JsonRoot = this->Internal->JsonRoot;
  jsonArray->Register(this);
  return jsonArray;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsJsonElement::GetArraySize()
{
  if (!this->Internal->JsonValue.IsArray())
  {
    return 0;
  }
  return static_cast<int>(this->Internal->JsonValue.GetArray().Size());
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonElement::IsArray()
{
  return this->Internal->JsonValue.IsArray();
}

//----------------------------------------------------------------------------
std::string vtkMRMLMarkupsJsonElement::GetSchema()
{
  if (!this->Internal->JsonValue.HasMember("@schema"))
  {
    return "";
  }
  rapidjson::Value& schema = this->Internal->JsonValue["@schema"];
  return schema.GetString();
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonElement::GetVectorProperty(const char* propertyName, double* v, int numberOfComponents /*=3*/)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  rapidjson::Value& item = this->Internal->JsonValue[propertyName];
  return this->Internal->ReadVector(item, v, numberOfComponents);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonElement::GetMatrix4x4Property(const char* propertyName, double v[16], bool flipRasLps)
{
  if (!this->GetVectorProperty(propertyName, v, 16))
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this,
      this->GetUserMessages(),
      "vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
      "File reading failed: " << propertyName << " 16 - element numeric array.");
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
bool vtkMRMLMarkupsJsonElement::GetStringVectorProperty(const char* propertyName, std::vector<std::string>& arrayValues)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return false;
  }
  rapidjson::Value& item = this->Internal->JsonValue[propertyName];
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
bool vtkMRMLMarkupsJsonElement::HasErrors()
{
  return this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0;
}

//----------------------------------------------------------------------------
vtkDoubleArray* vtkMRMLMarkupsJsonElement::GetDoubleArrayProperty(const char* propertyName)
{
  if (!this->Internal->JsonValue.HasMember(propertyName))
  {
    return nullptr;
  }
  rapidjson::Value& arrayItem = this->Internal->JsonValue[propertyName];
  if (!arrayItem.IsArray())
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this,
      this->GetUserMessages(),
      "vtkMRMLMarkupsJsonElement::GetDoubleArrayProperty",
      "Property " << propertyName << " is expected to contain an array of floating-point numbers");
    return nullptr;
  }
  vtkNew<vtkDoubleArray> values;
  int numberOfTuples = arrayItem.GetArray().Size();
  if (numberOfTuples < 1)
  {
    // no values stored in the array
    return nullptr;
  }
  rapidjson::Value& firstControlPointValue = arrayItem.GetArray()[0];
  if (firstControlPointValue.IsDouble())
  {
    values->SetNumberOfValues(numberOfTuples);
    double* valuesPtr = values->GetPointer(0);
    bool success = this->GetVectorProperty(propertyName, valuesPtr, numberOfTuples);
    if (!success)
    {
      vtkErrorToMessageCollectionWithObjectMacro(
        this,
        this->GetUserMessages(),
        "vtkMRMLMarkupsJsonIO::GetDoubleArrayProperty",
        "Property " << propertyName << " is expected to contain an array of floating-point numbers");
      return nullptr;
    }
  }
  else if (firstControlPointValue.IsArray())
  {
    int numberOfComponents = firstControlPointValue.GetArray().Size();
    values->SetNumberOfComponents(numberOfComponents);
    values->SetNumberOfTuples(numberOfTuples);
    double* valuesPtr = values->GetPointer(0);
    for (auto& itemValue : arrayItem.GetArray())
    {
      bool success = this->Internal->ReadVector(itemValue, valuesPtr, numberOfComponents);
      if (!success)
      {
        vtkErrorToMessageCollectionWithObjectMacro(
          this,
          this->GetUserMessages(),
          "vtkMRMLMarkupsJsonIO::GetDoubleArrayProperty",
          "Property " << propertyName << " is expected to contain an array of floating-point numbers"
                      << " with the same number of components");
        return nullptr;
      }
      valuesPtr += numberOfComponents;
    }
  }
  else
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this,
      this->GetUserMessages(),
      "vtkMRMLMarkupsJsonIO::GetDoubleArrayProperty",
      "Property " << propertyName << " is expected to contain an array of floating-point numbers"
                  << " or array of arrays of floating-point numbers");
    return nullptr;
  }
  values->Register(this);
  return values;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonReader::vtkMRMLMarkupsJsonReader() {}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonReader::~vtkMRMLMarkupsJsonReader() {}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonReader::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonReader::HasErrors()
{
  return this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonElement* vtkMRMLMarkupsJsonReader::ReadFromFile(const char* filePath)
{
  if (!filePath)
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(), "vtkMRMLMarkupsJsonIO::ReadFromFile", "Invalid filename");
    return nullptr;
  }

  // Read document from file
  FILE* fp = fopen(filePath, "r");
  if (!fp)
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonIO::ReadFromFile",
                                               "Error opening the file '" << filePath << "'");
    return nullptr;
  }

  vtkNew<vtkMRMLMarkupsJsonElement> jsonElement;

  jsonElement->Internal->JsonRoot = std::make_shared<vtkMRMLMarkupsJsonElement::vtkInternal::JsonDocumentContainer>();

  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (jsonElement->Internal->JsonRoot->Document->ParseStream(fs).HasParseError())
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonIO::ReadFromFile",
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
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonIO::ReadFromFile",
                                               "Error parsing the file '" << filePath
                                                                          << "' - root item must be array or list");
    return nullptr;
  }

  jsonElement->Register(this);
  return jsonElement;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsJsonWriter::vtkInternal::vtkInternal(vtkMRMLMarkupsJsonWriter* external)
  : External(external)
{
  this->WriteBuffer.resize(65536);
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsJsonWriter::vtkInternal::~vtkInternal() {}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonWriter::vtkMRMLMarkupsJsonWriter()
{
  this->Internal = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonWriter::~vtkMRMLMarkupsJsonWriter() {}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonWriter::HasErrors()
{
  return this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::vtkInternal::WriteVector(double* v, int numberOfComponents /*=3*/)
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
bool vtkMRMLMarkupsJsonWriter::WriteToFileBegin(const char* filePath, const char* schema)
{
  if (!filePath)
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this, this->GetUserMessages(), "vtkMRMLMarkupsJsonIO::WriteToFileBegin", "Invalid filename");
    return false;
  }

  // open the file for writing
  this->Internal->WriteFileHandle = fopen(filePath, "wb");
  if (!this->Internal->WriteFileHandle)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::WriteDataInternal",
                                     "Writing markups node file failed: unable to open file '" << filePath
                                                                                               << "' for writing.");
    return false;
  }

  // Prepare JSON writer and output stream.
  this->Internal->FileWriteStream = std::unique_ptr<rapidjson::FileWriteStream>(new rapidjson::FileWriteStream(
    this->Internal->WriteFileHandle, &this->Internal->WriteBuffer[0], this->Internal->WriteBuffer.size()));

  this->Internal->Writer = std::unique_ptr<rapidjson::PrettyWriter<rapidjson::FileWriteStream>>(
    new rapidjson::PrettyWriter<rapidjson::FileWriteStream>(*this->Internal->FileWriteStream));

  this->Internal->Writer->StartObject();

  if (schema)
  {
    this->WriteStringProperty("@schema", schema);
  }

  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteStringProperty(const std::string& propertyName, const std::string& propertyValue)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->String(propertyValue.c_str());
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteStringVectorProperty(const std::string& propertyName,
                                                         const std::vector<std::string>& arrayValues)
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
void vtkMRMLMarkupsJsonWriter::WriteStringPropertyIfNotEmpty(const std::string& propertyName,
                                                             const std::string& propertyValue)
{
  if (propertyValue.empty())
  {
    return;
  }
  this->WriteStringProperty(propertyName, propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteCodedEntryProperty(const std::string& propertyName, vtkCodedEntry* codedEntry)
{
  if (!codedEntry)
  {
    return;
  }

  // Coded entry is made up of 3 short strings, it is nicer to write them in a single line
  this->Internal->Writer->SetFormatOptions(rapidjson::kFormatSingleLineArray);

  this->WriteStringVectorProperty(propertyName, codedEntry->GetValueSchemeMeaning());

  this->Internal->Writer->SetFormatOptions(rapidjson::kFormatDefault);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteBoolProperty(const std::string& propertyName, bool propertyValue)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->Bool(propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteIntProperty(const std::string& propertyName, int propertyValue)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->Int(propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteDoubleProperty(const std::string& propertyName, double propertyValue)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->Double(propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteVectorProperty(const std::string& propertyName,
                                                   double* v,
                                                   int numberOfComponents /*=3*/)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->WriteVector(v, numberOfComponents);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteMatrix4x4Property(const std::string& propertyName, double v[16], bool flipRasLps)
{
  if (flipRasLps)
  {
    double vFlipped[16] = { 0.0 };
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
void vtkMRMLMarkupsJsonWriter::WriteDoubleArrayProperty(const char* propertyName, vtkDoubleArray* doubleArray)
{
  this->WriteArrayPropertyStart(propertyName);
  int numberOfComponents = doubleArray->GetNumberOfComponents();
  int numberOfTuples = doubleArray->GetNumberOfTuples();
  if (numberOfComponents == 1)
  {
    // write single-component array as single array
    double* values = doubleArray->GetPointer(0);
    // WriteVector() method would write all values in a single line, so we do not use it here
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
void vtkMRMLMarkupsJsonWriter::WriteArrayPropertyStart(const std::string& propertyName)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->StartArray();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteArrayPropertyEnd()
{
  this->Internal->Writer->EndArray();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteObjectPropertyStart(const std::string& propertyName)
{
  this->Internal->Writer->Key(propertyName.c_str());
  this->Internal->Writer->StartObject();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteObjectPropertyEnd()
{
  this->Internal->Writer->EndObject();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteObjectStart()
{
  this->Internal->Writer->StartObject();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonWriter::WriteObjectEnd()
{
  this->Internal->Writer->EndObject();
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonWriter::WriteToFileEnd()
{
  this->Internal->Writer->EndObject();
  int error = fclose(this->Internal->WriteFileHandle);
  this->Internal->WriteFileHandle = 0;
  this->Internal->Writer.reset();
  this->Internal->FileWriteStream.reset();
  return (error == 0);
}
