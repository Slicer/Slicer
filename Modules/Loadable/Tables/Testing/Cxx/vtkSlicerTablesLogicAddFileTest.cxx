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

// DoubleArrays logic
#include "vtkSlicerTablesLogic.h"

// MRML includes
#include <vtkMRMLTableNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTable.h>

#include "vtkTestingOutputWindow.h"
#include "vtkMRMLCoreTestingMacros.h"

//-----------------------------------------------------------------------------
int testAddInvalidFile(const char* filePath);
int testAddFile(const char* filePath);

//-----------------------------------------------------------------------------
int vtkSlicerTablesLogicAddFileTest( int argc, char * argv[] )
{
  bool res = true;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(testAddInvalidFile(nullptr));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(testAddInvalidFile(""));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(testAddInvalidFile("non existing file.badextension"));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(testAddInvalidFile("non existing file.vtk"));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  if (argc > 1)
    {
    CHECK_EXIT_SUCCESS(testAddFile(argv[1]));
    }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//-----------------------------------------------------------------------------
int testAddInvalidFile(const char * filePath)
{
  std::cout << "Test loading of invalid file: " << (filePath ? filePath : "(none)") << ". Errors are expected.";

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerTablesLogic> tablesLogic;
  tablesLogic->SetMRMLScene(scene.GetPointer());

  int nodeCount = scene->GetNumberOfNodes();
  vtkMRMLTableNode* table = tablesLogic->AddTable(filePath);

  CHECK_NULL(table);
  CHECK_INT(scene->GetNumberOfNodes(), nodeCount);

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int testAddFile(const char * filePath)
{
  vtkNew<vtkSlicerTablesLogic> tablesLogic;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkMRMLTableNode* tableNode = tablesLogic->AddTable(filePath);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // File can't be loaded if no scene is set
  CHECK_NULL(tableNode);

  vtkNew<vtkMRMLScene> scene;
  tablesLogic->SetMRMLScene(scene.GetPointer());

  int nodeCount = scene->GetNumberOfNodes();
  tableNode = tablesLogic->AddTable(filePath);

  CHECK_NOT_NULL(tableNode);
  // Adding an table should create a table node and a storage node
  CHECK_INT(scene->GetNumberOfNodes(), nodeCount + 2);
  CHECK_BOOL(tableNode->GetNumberOfRows() > 0, true);
  CHECK_BOOL(tableNode->GetNumberOfColumns() > 0, true);

  vtkTable* table = tableNode->GetTable();

  CHECK_NOT_NULL(table->GetColumnByName("TestBool"));
  CHECK_NOT_NULL(table->GetColumnByName("TestString"));
  CHECK_NOT_NULL(table->GetColumnByName("TestInt"));
  CHECK_NOT_NULL(table->GetColumnByName("TestDouble"));
  CHECK_NOT_NULL(table->GetColumnByName("TestFloat"));

  CHECK_INT(table->GetColumnByName("TestBool")->GetDataType(), VTK_BIT);
  CHECK_INT(table->GetColumnByName("TestString")->GetDataType(), VTK_STRING);
  CHECK_INT(table->GetColumnByName("TestInt")->GetDataType(), VTK_INT);
  CHECK_INT(table->GetColumnByName("TestDouble")->GetDataType(), VTK_DOUBLE);
  CHECK_INT(table->GetColumnByName("TestFloat")->GetDataType(), VTK_FLOAT);

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}
