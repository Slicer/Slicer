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

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableStorageNode.h"
#include "vtkDoubleArray.h"
#include "vtkStringArray.h"
#include "vtkTable.h"

#include <vtksys/SystemTools.hxx>

//---------------------------------------------------------------------------
int TestReadWriteWithoutSchema(vtkMRMLScene* scene);
int TestReadWriteWithSchema(vtkMRMLScene* scene);
int TestReadWriteData(vtkMRMLScene* scene, const char *extension, vtkTable* table, bool schemaExpected);

int vtkMRMLTableStorageNodeTest1(int argc, char * argv[])
{
  if (argc != 2)
    {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLTableStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  vtkNew<vtkMRMLScene> scene;
  const char* tempDir = argv[1];
  scene->SetRootDirectory(tempDir);

  CHECK_EXIT_SUCCESS(TestReadWriteWithoutSchema(scene.GetPointer()));
  CHECK_EXIT_SUCCESS(TestReadWriteWithSchema(scene.GetPointer()));

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadWriteWithoutSchema(vtkMRMLScene* scene)
{
  // Create a scene with string columns
  vtkNew<vtkStringArray> col1;
  col1->SetName("col1");
  col1->InsertNextValue("aa");
  col1->InsertNextValue("bb");
  vtkNew<vtkStringArray> col2;
  col2->SetName("col2");
  col2->InsertNextValue("cc");
  col2->InsertNextValue("dd");
  vtkNew<vtkTable> table;
  table->AddColumn(col1.GetPointer());
  table->AddColumn(col2.GetPointer());

  CHECK_EXIT_SUCCESS(TestReadWriteData(scene, ".csv", table.GetPointer(), false));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene, ".tsv", table.GetPointer(), false));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene, ".txt", table.GetPointer(), false));

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadWriteWithSchema(vtkMRMLScene* scene)
{
  // Create a scene with various column types
  // (it will require using a schema to save column types)
  vtkNew<vtkStringArray> col1;
  col1->SetName("col1");
  col1->InsertNextValue("aa");
  col1->InsertNextValue("bb");
  vtkNew<vtkDoubleArray> col2;
  col2->SetName("col2");
  col2->InsertNextValue(123);
  col2->InsertNextValue(456.7);
  vtkNew<vtkTable> table;
  table->AddColumn(col1.GetPointer());
  table->AddColumn(col2.GetPointer());

  CHECK_EXIT_SUCCESS(TestReadWriteData(scene, ".csv", table.GetPointer(), true));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene, ".tsv", table.GetPointer(), true));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene, ".txt", table.GetPointer(), true));

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadWriteData(vtkMRMLScene* scene, const char *extension, vtkTable* table, bool schemaExpected)
{
  std::string fileName = std::string(scene->GetRootDirectory()) +
    std::string("/vtkMRMLTableStorageNodeTest1") +
    std::string(extension);

  std::string expectedSchemaFileName = std::string(scene->GetRootDirectory()) +
    std::string("/vtkMRMLTableStorageNodeTest1.schema") +
    std::string(extension);

  vtksys::SystemTools::RemoveFile(fileName);
  vtksys::SystemTools::RemoveFile(expectedSchemaFileName);

  int numberOfColumns = table->GetNumberOfColumns();
  CHECK_BOOL(numberOfColumns > 0, true);

  // Add model node
  vtkNew<vtkMRMLTableNode> tableNode;
  tableNode->SetAndObserveTable(table);
  CHECK_NOT_NULL(tableNode->GetTable());
  CHECK_NOT_NULL(scene->AddNode(tableNode.GetPointer()));

  // Add storage node
  tableNode->AddDefaultStorageNode();
  vtkMRMLStorageNode* storageNode = tableNode->GetStorageNode();
  CHECK_NOT_NULL(storageNode);
  storageNode->SetFileName(fileName.c_str());

  // Test writing
  CHECK_BOOL(storageNode->WriteData(tableNode.GetPointer()), true);

  bool schemaExists = vtksys::SystemTools::FileExists(expectedSchemaFileName);
  CHECK_BOOL(schemaExists, schemaExpected);

  // Clear data from model node
  tableNode->SetAndObserveTable(nullptr);
  CHECK_NULL(tableNode->GetTable());

  // Test reading
  CHECK_BOOL(storageNode->ReadData(tableNode.GetPointer()), true);
  vtkTable* table2 = tableNode->GetTable();
  CHECK_NOT_NULL(table2);
  CHECK_INT(table2->GetNumberOfColumns(), numberOfColumns);

  return EXIT_SUCCESS;
}
