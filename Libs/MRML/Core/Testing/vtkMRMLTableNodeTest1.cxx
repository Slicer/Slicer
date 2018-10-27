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

#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTestErrorObserver.h"

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLTableNodeTest1(int , char * [] )
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
  vtkSmartPointer< vtkMRMLTableStorageNode > storageNode = vtkSmartPointer< vtkMRMLTableStorageNode >::Take(vtkMRMLTableStorageNode::SafeDownCast(node2->CreateDefaultStorageNode()));
  CHECK_NOT_NULL(storageNode);

  // Verify basic add/remove column methods

  CHECK_NOT_NULL(node2->AddColumn());
  CHECK_INT(table->GetNumberOfColumns(), 1);

  vtkSmartPointer< vtkStringArray > newEmptyArray = vtkSmartPointer< vtkStringArray >::New();
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

  vtkSmartPointer< vtkStringArray > newShortArray = vtkSmartPointer< vtkStringArray >::New();
  newShortArray->InsertNextValue("something");
  CHECK_NOT_NULL(node2->AddColumn(newShortArray));
  CHECK_INT(table->GetNumberOfRows(), 3);
  CHECK_INT(table->GetNumberOfColumns(), 2);
  CHECK_INT(newShortArray->GetNumberOfTuples(), table->GetNumberOfRows());

  // Verify that arrays that are shorter than the table extend the table

  vtkSmartPointer< vtkStringArray > newLongArray = vtkSmartPointer< vtkStringArray >::New();
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

  // Test GetCellText

  CHECK_INT(newLongArray->GetNumberOfTuples(), table->GetNumberOfRows());

  CHECK_STD_STRING(node2->GetCellText(2,2), "something3");

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_STD_STRING(node2->GetCellText(20,2), "");
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_STD_STRING(node2->GetCellText(20,2), "");
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  CHECK_BOOL(node2->SetCellText(2,2,"ModifiedText"), true);
  CHECK_STD_STRING(node2->GetCellText(2,2), "ModifiedText");

  // Test SetCellText
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(node2->SetCellText(20,2,"invalid"), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(node2->SetCellText(2, 20, "invalid"), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // Verify that Copy method creates a true independent copy
  vtkSmartPointer< vtkMRMLTableNode > node2copy = vtkSmartPointer< vtkMRMLTableNode >::New();
  node2copy->Copy(node2.GetPointer());
  // After copying the contents of the tables should be the same
  CHECK_STD_STRING(node2->GetCellText(0,0), node2copy->GetCellText(0,0));

  // After modifying the copied version, the tables should be different
  // (if there was a shallow copy only, the original table would have been changed, too)
  CHECK_BOOL(node2copy->SetCellText(0,0,"someModifiedText"), true);
  CHECK_STD_STRING_DIFFERENT(node2->GetCellText(0,0), node2copy->GetCellText(0,0));

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
