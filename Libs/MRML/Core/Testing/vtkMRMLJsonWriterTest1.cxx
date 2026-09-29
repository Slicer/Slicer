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

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLJsonElement.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkVariant.h>

// STD includes
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
int vtkMRMLJsonWriterTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  const std::string specialString = "quote\" backslash\\ newline\n end";
  const int64_t largeInteger = static_cast<int64_t>(std::numeric_limits<int>::max()) * 4;

  // Write a document without wrapper object
  vtkNew<vtkMRMLJsonWriter> writer;
  CHECK_BOOL(writer->WriteToStringBegin(), true);
  writer->WriteArrayPropertyStart("items");
  for (int i = 0; i < 2; ++i)
  {
    writer->WriteObjectStart();
    writer->WriteIntProperty("index", i);
    writer->WriteObjectEnd();
  }
  writer->WriteArrayPropertyEnd();
  writer->WriteStringProperty("string", specialString);
  writer->WriteBoolProperty("bool", true);
  writer->WriteIntProperty("int", -12);
  writer->WriteDoubleProperty("double", 1.25);
  writer->WriteStringVectorProperty("stringVector", { "a", "b" });
  writer->WriteNullProperty("null");
  writer->WriteVariantProperty("variantInvalid", vtkVariant());
  writer->WriteVariantProperty("variantString", vtkVariant(specialString));
  writer->WriteVariantProperty("variantInt", vtkVariant(7));
  writer->WriteVariantProperty("variantInt64", vtkVariant(static_cast<long long>(largeInteger)));
  writer->WriteVariantProperty("variantDouble", vtkVariant(2.5));
  writer->WriteVariantProperty("variantNaN", vtkVariant(std::numeric_limits<double>::quiet_NaN()));
  writer->WriteVariantProperty("variantInfinity", vtkVariant(-std::numeric_limits<double>::infinity()));
  std::string json = writer->WriteToStringEnd();
  CHECK_BOOL(writer->HasErrors(), false);

  // Read it back
  vtkNew<vtkMRMLJsonReader> reader;
  vtkSmartPointer<vtkMRMLJsonElement> root = vtkSmartPointer<vtkMRMLJsonElement>::Take(reader->ReadFromString(json));
  CHECK_NOT_NULL(root);
  CHECK_BOOL(reader->HasErrors(), false);
  CHECK_BOOL(root->IsObject(), true);

  vtkSmartPointer<vtkMRMLJsonElement> items = vtkSmartPointer<vtkMRMLJsonElement>::Take(root->GetArrayProperty("items"));
  CHECK_NOT_NULL(items);
  CHECK_INT(items->GetArraySize(), 2);
  for (int i = 0; i < 2; ++i)
  {
    vtkSmartPointer<vtkMRMLJsonElement> item = vtkSmartPointer<vtkMRMLJsonElement>::Take(items->GetArrayItem(i));
    CHECK_NOT_NULL(item);
    CHECK_INT(item->GetIntProperty("index"), i);
  }

  CHECK_STD_STRING(root->GetStringProperty("string"), specialString);
  CHECK_BOOL(root->GetBoolProperty("bool"), true);
  CHECK_INT(root->GetIntProperty("int"), -12);
  CHECK_DOUBLE(root->GetDoubleProperty("double"), 1.25);
  std::vector<std::string> stringVector;
  CHECK_BOOL(root->GetStringVectorProperty("stringVector", stringVector), true);
  CHECK_INT(static_cast<int>(stringVector.size()), 2);
  CHECK_STD_STRING(stringVector[0], "a");
  CHECK_STD_STRING(stringVector[1], "b");

  // Null values are present, but they are not of any known type
  CHECK_BOOL(root->HasMember("null"), true);
  CHECK_INT(root->GetMemberType("null"), vtkMRMLJsonElement::UNKNOWN);
  CHECK_BOOL(root->HasMember("variantInvalid"), true);
  CHECK_INT(root->GetMemberType("variantInvalid"), vtkMRMLJsonElement::UNKNOWN);

  // Variant values keep their type
  CHECK_INT(root->GetMemberType("variantString"), vtkMRMLJsonElement::STRING);
  CHECK_STD_STRING(root->GetStringProperty("variantString"), specialString);
  CHECK_INT(root->GetMemberType("variantInt"), vtkMRMLJsonElement::INT);
  CHECK_INT(root->GetIntProperty("variantInt"), 7);
  CHECK_INT(root->GetMemberType("variantInt64"), vtkMRMLJsonElement::INT);
  CHECK_DOUBLE(root->GetDoubleProperty("variantInt64"), static_cast<double>(largeInteger));
  CHECK_INT(root->GetMemberType("variantDouble"), vtkMRMLJsonElement::DOUBLE);
  CHECK_DOUBLE(root->GetDoubleProperty("variantDouble"), 2.5);
  // Non-finite numbers are written as null
  CHECK_BOOL(root->HasMember("variantNaN"), true);
  CHECK_INT(root->GetMemberType("variantNaN"), vtkMRMLJsonElement::UNKNOWN);
  CHECK_BOOL(root->HasMember("variantInfinity"), true);
  CHECK_INT(root->GetMemberType("variantInfinity"), vtkMRMLJsonElement::UNKNOWN);

  // Write a document with wrapper object
  vtkNew<vtkMRMLJsonWriter> taggedWriter;
  CHECK_BOOL(taggedWriter->WriteToStringBegin("tag"), true);
  taggedWriter->WriteStringProperty("name", "value");
  std::string taggedJson = taggedWriter->WriteToStringEnd();
  vtkSmartPointer<vtkMRMLJsonElement> taggedRoot = vtkSmartPointer<vtkMRMLJsonElement>::Take(reader->ReadFromString(taggedJson));
  CHECK_NOT_NULL(taggedRoot);
  CHECK_INT(taggedRoot->GetMemberType("tag"), vtkMRMLJsonElement::OBJECT);
  CHECK_INT(taggedRoot->GetObjectSize(), 1);
  CHECK_STD_STRING(taggedRoot->GetObjectPropertyNameByIndex(0), "tag");
  vtkSmartPointer<vtkMRMLJsonElement> tagObject = vtkSmartPointer<vtkMRMLJsonElement>::Take(taggedRoot->GetObjectProperty("tag"));
  CHECK_NOT_NULL(tagObject);
  CHECK_STD_STRING(tagObject->GetStringProperty("name"), "value");

  // Tag name is required for the tagged version
  vtkNew<vtkMRMLJsonWriter> invalidWriter;
  CHECK_BOOL(invalidWriter->WriteToStringBegin(nullptr), false);

  std::cout << "vtkMRMLJsonWriterTest1 passed" << std::endl;
  return EXIT_SUCCESS;
}
