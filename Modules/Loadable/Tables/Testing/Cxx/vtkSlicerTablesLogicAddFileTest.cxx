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

#include "vtkTestingOutputWindow.h"

//-----------------------------------------------------------------------------
bool testAddInvalidFile(const char* filePath);
bool testAddFile(const char* filePath);

//-----------------------------------------------------------------------------
int vtkSlicerTablesLogicAddFileTest( int argc, char * argv[] )
{
  bool res = true;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  res = testAddInvalidFile(0) && res;
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  res = testAddInvalidFile("") && res;
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  res = testAddInvalidFile("non existing file.badextension") && res;
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  res = testAddInvalidFile("non existing file.vtk") && res;
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (argc > 1)
    {
    res = testAddFile(argv[1]) && res;
    }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//-----------------------------------------------------------------------------
bool testAddInvalidFile(const char * filePath)
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerTablesLogic> tablesLogic;
  tablesLogic->SetMRMLScene(scene.GetPointer());

  int nodeCount = scene->GetNumberOfNodes();
  vtkMRMLTableNode* table = tablesLogic->AddTable(filePath);

  if (table != 0 ||
      scene->GetNumberOfNodes() != nodeCount)
    {
    std::cerr << "Line " << __LINE__
              << ": Adding an invalid file ("<< (filePath ? filePath : 0)
              << ") shall not add nodes in scene. "
              << scene->GetNumberOfNodes() << " vs " << nodeCount
              << std::endl;
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool testAddFile(const char * filePath)
{
  vtkNew<vtkSlicerTablesLogic> tablesLogic;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkMRMLTableNode* table = tablesLogic->AddTable(filePath);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (table != 0)
    {
    std::cerr << "Line " << __LINE__
              << ": File can't be loaded if no scene is set."
              << std::endl;
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  tablesLogic->SetMRMLScene(scene.GetPointer());
  int nodeCount = scene->GetNumberOfNodes();
  table = tablesLogic->AddTable(filePath);

  if (table == 0 ||
      scene->GetNumberOfNodes() != nodeCount + 2)
    {
    std::cerr << "Adding an table should create 2 nodes" << std::endl;
    return false;
    }

  if (table->GetNumberOfRows()==0 || table->GetNumberOfColumns()==0)
    {
    std::cerr << "Table is empty" << std::endl;
    return false;
    }

  return true;
}
