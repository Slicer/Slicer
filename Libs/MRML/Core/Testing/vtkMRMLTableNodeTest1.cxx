/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#include "vtkMRMLScene.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableStorageNode.h"

#include "vtkBitArray.h"
#include "vtkCharArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkLongLongArray.h"
#include "vtkShortArray.h"
#include "vtkSignedCharArray.h"
#include "vtkStringArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkUnsignedLongLongArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkTable.h"
#include "vtkTestErrorObserver.h"

#include "vtkMRMLCoreTestingMacros.h"

namespace
{
template <class ArrayType, typename ValueType>
void checkDefaultArrayValue(vtkMRMLTableNode* tableNode,
                            std::string columnType,
                            std::string defaultValueString,
                            ValueType value)
{
  tableNode->SetDefaultColumnType(columnType, defaultValueString);
  ArrayType* newArray = ArrayType::SafeDownCast(tableNode->AddColumn());
  if (!newArray)
  {
    std::cerr << "checkDefaultArrayValue failed for '" << columnType << "'. Failed to create array" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (newArray->GetValue(0) != value)
  {
    std::cerr << "checkDefaultArrayValue failed for '" << columnType << "' with '" << defaultValueString
              << "'. Expected value: '" << value << "', got: '" << newArray->GetValue(0) << "'" << std::endl;
    exit(EXIT_FAILURE);
  }
  tableNode->RemoveColumn(tableNode->GetNumberOfColumns() - 1);
}
} // namespace

int vtkMRMLTableNodeTest1(int, char*[])
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLTableNode> node1;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  vtkNew<vtkMRMLTableNode> node2;
  scene->AddNode(node2.GetPointer());

  vtkTable* table = node2->GetTable();
  CHECK_NOT_NULL(table);

  // Verify if a proper storage node is created
  vtkSmartPointer<vtkMRMLTableStorageNode> storageNode = vtkSmartPointer<vtkMRMLTableStorageNode>::Take(
    vtkMRMLTableStorageNode::SafeDownCast(node2->CreateDefaultStorageNode()));
  CHECK_NOT_NULL(storageNode);

  // Verify basic add/remove column methods

  CHECK_NOT_NULL(node2->AddColumn());
  CHECK_INT(table->GetNumberOfColumns(), 1);

  vtkSmartPointer<vtkStringArray> newEmptyArray = vtkSmartPointer<vtkStringArray>::New();
  CHECK_NOT_NULL(node2->AddColumn(newEmptyArray));
  CHECK_INT(table->GetNumberOfColumns(), 2);

  CHECK_BOOL(node2->RemoveColumn(1), true);
  CHECK_INT(table->GetNumberOfColumns(), 1);

  CHECK_NOT_NULL(node2->AddColumn());
  CHECK_NOT_NULL(node2->AddColumn());
  CHECK_NOT_NULL(node2->AddColumn());
  CHECK_BOOL(node2->RemoveAllColumns(), true);
  CHECK_INT(table->GetNumberOfColumns(), 0);

  CHECK_NOT_NULL(node2->AddColumn());
  CHECK_INT(node2->AddEmptyRow(), 0);
  CHECK_INT(table->GetNumberOfRows(), 1);

  CHECK_INT(node2->AddEmptyRow(), 1);
  CHECK_INT(node2->AddEmptyRow(), 2);
  CHECK_INT(node2->AddEmptyRow(), 3);
  CHECK_INT(table->GetNumberOfRows(), 4);

  CHECK_BOOL(node2->RemoveRow(1), true);
  CHECK_INT(table->GetNumberOfRows(), 3);

  // Verify that arrays that are shorter than the table size are extended to match the current table size

  vtkSmartPointer<vtkStringArray> newShortArray = vtkSmartPointer<vtkStringArray>::New();
  newShortArray->InsertNextValue("something");
  CHECK_NOT_NULL(node2->AddColumn(newShortArray));
  CHECK_INT(table->GetNumberOfRows(), 3);
  CHECK_INT(table->GetNumberOfColumns(), 2);
  CHECK_INT(newShortArray->GetNumberOfTuples(), table->GetNumberOfRows());

  // Verify that arrays that are shorter than the table extend the table

  vtkSmartPointer<vtkStringArray> newLongArray = vtkSmartPointer<vtkStringArray>::New();
  newLongArray->InsertNextValue("something1");
  newLongArray->InsertNextValue("something2");
  newLongArray->InsertNextValue("something3");
  newLongArray->InsertNextValue("something4");
  newLongArray->InsertNextValue("something5");
  newLongArray->InsertNextValue("something6");
  newLongArray->InsertNextValue("something7");
  newLongArray->InsertNextValue("something8");
  newLongArray->InsertNextValue("something9");
  newLongArray->InsertNextValue("something10");

  CHECK_NOT_NULL(node2->AddColumn(newLongArray));
  CHECK_INT(table->GetNumberOfRows(), 10);
  CHECK_INT(table->GetNumberOfColumns(), 3);

  // Test if new arrays are initialized correctly

  vtkNew<vtkMRMLTableNode> node3;
  node3->AddColumn();
  node3->AddEmptyRow();
  node3->AddEmptyRow();
  node3->AddEmptyRow();

  checkDefaultArrayValue<vtkStringArray, std::string>(node3, "string", "test", "test");
  checkDefaultArrayValue<vtkStringArray, std::string>(node3, "string", "", "");

  checkDefaultArrayValue<vtkDoubleArray, double>(node3, "double", "3.5", 3.5);
  checkDefaultArrayValue<vtkDoubleArray, double>(node3, "double", "", 0.0);
  checkDefaultArrayValue<vtkDoubleArray, double>(node3, "double", "some", 0.0);

  checkDefaultArrayValue<vtkFloatArray, double>(node3, "float", "3.5", 3.5);
  checkDefaultArrayValue<vtkFloatArray, double>(node3, "float", "", 0.0);
  checkDefaultArrayValue<vtkFloatArray, double>(node3, "float", "some", 0.0);

  checkDefaultArrayValue<vtkIntArray, int>(node3, "int", "3", 3);
  checkDefaultArrayValue<vtkIntArray, int>(node3, "int", "-5", -5);
  checkDefaultArrayValue<vtkIntArray, int>(node3, "int", "3.3", 0);
  checkDefaultArrayValue<vtkIntArray, int>(node3, "int", "", 0);
  checkDefaultArrayValue<vtkIntArray, int>(node3, "int", "some", 0);

  checkDefaultArrayValue<vtkUnsignedIntArray, unsigned int>(node3, "unsigned int", "3", 3);
  // this is a bit odd (since an unsigned value accepts a negative value), but this is vtkVariant behavior:
  checkDefaultArrayValue<vtkUnsignedIntArray, unsigned int>(node3, "unsigned int", "-5", -5);
  checkDefaultArrayValue<vtkUnsignedIntArray, unsigned int>(node3, "unsigned int", "3.3", 0);
  checkDefaultArrayValue<vtkUnsignedIntArray, unsigned int>(node3, "unsigned int", "", 0);
  checkDefaultArrayValue<vtkUnsignedIntArray, unsigned int>(node3, "unsigned int", "some", 0);

  checkDefaultArrayValue<vtkShortArray, short>(node3, "short", "3", 3);
  checkDefaultArrayValue<vtkShortArray, short>(node3, "short", "-5", -5);
  checkDefaultArrayValue<vtkShortArray, short>(node3, "short", "3.3", 0);
  checkDefaultArrayValue<vtkShortArray, short>(node3, "short", "", 0);
  checkDefaultArrayValue<vtkShortArray, short>(node3, "short", "some", 0);

  checkDefaultArrayValue<vtkUnsignedShortArray, unsigned short>(node3, "unsigned short", "3", 3);
  // this is a bit odd (since an unsigned value accepts a negative value), but this is vtkVariant behavior:
  checkDefaultArrayValue<vtkUnsignedShortArray, unsigned short>(node3, "unsigned short", "-5", -5);
  checkDefaultArrayValue<vtkUnsignedShortArray, unsigned short>(node3, "unsigned short", "3.3", 0);
  checkDefaultArrayValue<vtkUnsignedShortArray, unsigned short>(node3, "unsigned short", "", 0);
  checkDefaultArrayValue<vtkUnsignedShortArray, unsigned short>(node3, "unsigned short", "some", 0);

  checkDefaultArrayValue<vtkLongArray, long>(node3, "long", "3", 3);
  checkDefaultArrayValue<vtkLongArray, long>(node3, "long", "-5", -5);
  checkDefaultArrayValue<vtkLongArray, long>(node3, "long", "3.3", 0);
  checkDefaultArrayValue<vtkLongArray, long>(node3, "long", "", 0);
  checkDefaultArrayValue<vtkLongArray, long>(node3, "long", "some", 0);

  checkDefaultArrayValue<vtkUnsignedLongArray, unsigned long>(node3, "unsigned long", "3", 3);
  // this is a bit odd (since an unsigned value accepts a negative value), but this is vtkVariant behavior:
  checkDefaultArrayValue<vtkUnsignedLongArray, unsigned long>(node3, "unsigned long", "-5", -5);
  checkDefaultArrayValue<vtkUnsignedLongArray, unsigned long>(node3, "unsigned long", "3.3", 0);
  checkDefaultArrayValue<vtkUnsignedLongArray, unsigned long>(node3, "unsigned long", "", 0);
  checkDefaultArrayValue<vtkUnsignedLongArray, unsigned long>(node3, "unsigned long", "some", 0);

  checkDefaultArrayValue<vtkLongLongArray, long long>(node3, "long long", "3", 3);
  checkDefaultArrayValue<vtkLongLongArray, long long>(node3, "long long", "-5", -5);
  checkDefaultArrayValue<vtkLongLongArray, long long>(node3, "long long", "3.3", 0);
  checkDefaultArrayValue<vtkLongLongArray, long long>(node3, "long long", "", 0);
  checkDefaultArrayValue<vtkLongLongArray, long long>(node3, "long long", "some", 0);

  checkDefaultArrayValue<vtkUnsignedLongLongArray, unsigned long long>(node3, "unsigned long long", "3", 3);
  // this is a bit odd (since an unsigned value accepts a negative value), but this is vtkVariant behavior:
  checkDefaultArrayValue<vtkUnsignedLongLongArray, unsigned long long>(node3, "unsigned long long", "-5", -5);
  checkDefaultArrayValue<vtkUnsignedLongLongArray, unsigned long long>(node3, "unsigned long long", "3.3", 0);
  checkDefaultArrayValue<vtkUnsignedLongLongArray, unsigned long long>(node3, "unsigned long long", "", 0);
  checkDefaultArrayValue<vtkUnsignedLongLongArray, unsigned long long>(node3, "unsigned long long", "some", 0);

  checkDefaultArrayValue<vtkIdTypeArray, vtkIdType>(node3, "idtype", "3", 3);
  checkDefaultArrayValue<vtkIdTypeArray, vtkIdType>(node3, "idtype", "-5", -5);
  checkDefaultArrayValue<vtkIdTypeArray, vtkIdType>(node3, "idtype", "3.3", 0);
  checkDefaultArrayValue<vtkIdTypeArray, vtkIdType>(node3, "idtype", "", 0);
  checkDefaultArrayValue<vtkIdTypeArray, vtkIdType>(node3, "idtype", "some", 0);

  checkDefaultArrayValue<vtkBitArray, int>(node3, "bit", "0", false);
  checkDefaultArrayValue<vtkBitArray, int>(node3, "bit", "1", true);
  checkDefaultArrayValue<vtkBitArray, int>(node3, "bit", "3", true);
  checkDefaultArrayValue<vtkBitArray, int>(node3, "bit", "-5", true);
  checkDefaultArrayValue<vtkBitArray, int>(node3, "bit", "3.3", false);
  checkDefaultArrayValue<vtkBitArray, int>(node3, "bit", "", false);
  checkDefaultArrayValue<vtkBitArray, int>(node3, "bit", "some", false);

  checkDefaultArrayValue<vtkCharArray, char>(node3, "char", "0", '0');
  checkDefaultArrayValue<vtkCharArray, char>(node3, "char", "1", '1');
  checkDefaultArrayValue<vtkCharArray, char>(node3, "char", "3", '3');
  checkDefaultArrayValue<vtkCharArray, char>(node3, "char", "-5", 0);  // invalid because not a single character
  checkDefaultArrayValue<vtkCharArray, char>(node3, "char", "3.3", 0); // invalid because not a single character
  checkDefaultArrayValue<vtkCharArray, char>(node3, "char", "", 0);
  checkDefaultArrayValue<vtkCharArray, char>(node3, "char", "some", false); // invalid because not a single character

  checkDefaultArrayValue<vtkSignedCharArray, signed char>(node3, "signed char", "0", '0');
  checkDefaultArrayValue<vtkSignedCharArray, signed char>(node3, "signed char", "1", '1');
  checkDefaultArrayValue<vtkSignedCharArray, signed char>(node3, "signed char", "3", '3');
  checkDefaultArrayValue<vtkSignedCharArray, signed char>(node3, "signed char", "-5", 0);
  checkDefaultArrayValue<vtkSignedCharArray, signed char>(node3, "signed char", "3.3", 0);
  checkDefaultArrayValue<vtkSignedCharArray, signed char>(node3, "signed char", "", 0);
  checkDefaultArrayValue<vtkSignedCharArray, signed char>(node3, "signed char", "some", 0);

  // it is a bit odd that "0" get converted to '0' instead of 0 ('0x0'), but this is vtkVariant behavior:
  checkDefaultArrayValue<vtkUnsignedCharArray, unsigned char>(node3, "unsigned char", "0", '0');
  checkDefaultArrayValue<vtkUnsignedCharArray, unsigned char>(node3, "unsigned char", "1", '1');
  checkDefaultArrayValue<vtkUnsignedCharArray, unsigned char>(node3, "unsigned char", "3", '3');
  checkDefaultArrayValue<vtkUnsignedCharArray, unsigned char>(node3, "unsigned char", "-5", 0);
  checkDefaultArrayValue<vtkUnsignedCharArray, unsigned char>(node3, "unsigned char", "3.3", 0);
  checkDefaultArrayValue<vtkUnsignedCharArray, unsigned char>(node3, "unsigned char", "", 0);
  checkDefaultArrayValue<vtkUnsignedCharArray, unsigned char>(node3, "unsigned char", "some", 0);

  // Test GetCellText

  CHECK_INT(newLongArray->GetNumberOfTuples(), table->GetNumberOfRows());

  CHECK_STD_STRING(node2->GetCellText(2, 2), "something3");

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_STD_STRING(node2->GetCellText(20, 2), "");
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_STD_STRING(node2->GetCellText(20, 2), "");
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  CHECK_BOOL(node2->SetCellText(2, 2, "ModifiedText"), true);
  CHECK_STD_STRING(node2->GetCellText(2, 2), "ModifiedText");

  // Test SetCellText
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(node2->SetCellText(20, 2, "invalid"), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(node2->SetCellText(2, 20, "invalid"), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // Verify that Copy method creates a true independent copy
  vtkSmartPointer<vtkMRMLTableNode> node2copy = vtkSmartPointer<vtkMRMLTableNode>::New();
  node2copy->Copy(node2.GetPointer());
  // After copying the contents of the tables should be the same
  CHECK_STD_STRING(node2->GetCellText(0, 0), node2copy->GetCellText(0, 0));

  // After modifying the copied version, the tables should be different
  // (if there was a shallow copy only, the original table would have been changed, too)
  CHECK_BOOL(node2copy->SetCellText(0, 0, "someModifiedText"), true);
  CHECK_STD_STRING_DIFFERENT(node2->GetCellText(0, 0), node2copy->GetCellText(0, 0));

  // Test table column properties

  // Set properties
  node2->SetColumnProperty(0, "MyProp", "MyPropValue");
  CHECK_STD_STRING(node2->GetColumnProperty(0, "MyProp"), "MyPropValue");
  node2->SetColumnProperty("Column 2", "MyProp", "MyPropValue2");
  CHECK_STD_STRING(node2->GetColumnProperty("Column 2", "MyProp"), "MyPropValue2");
  node2->SetColumnProperty("Column 2", "MyProp B", "MyPropValue222");
  CHECK_STD_STRING(node2->GetColumnProperty("Column 2", "MyProp B"), "MyPropValue222");

  vtkNew<vtkStringArray> propertyNames;
  node2->GetAllColumnPropertyNames(propertyNames.GetPointer());
  CHECK_INT(propertyNames->GetNumberOfValues(), 2);

  // Test if table properties are preserved if column is renamed
  node2->RenameColumn(node2->GetColumnIndex("Column 2"), "Column 2 renamed");
  CHECK_STD_STRING(node2->GetColumnProperty("Column 2", "MyProp"), "");
  CHECK_STD_STRING(node2->GetColumnProperty("Column 2 renamed", "MyProp"), "MyPropValue2");

  // Test if table properties are deleted if column i s deleted
  node2->RemoveColumn(node2->GetColumnIndex("Column 2 renamed"));
  CHECK_STD_STRING(node2->GetColumnProperty("Column 2 renamed", "MyProp"), "");

  // Get properties
  // non-existing property name
  CHECK_STD_STRING(node2->GetColumnProperty(0, "nonexisting"), "");
  CHECK_STD_STRING(node2->GetColumnProperty(0, ""), "");
  // reserved property name
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_STD_STRING(node2->GetColumnProperty(0, "columnName"), "");
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_STD_STRING(node2->GetColumnProperty(0, "type"), "string");

  std::cout << "vtkMRMLTableNodeTest1 completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
