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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DoubleArrays logic
#include "vtkSlicerDoubleArraysLogic.h"

// MRML includes
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTestingOutputWindow.h>

//-----------------------------------------------------------------------------
bool testAddEmptyFile(const char* filePath);
bool testAddFile(const char* filePath);

//-----------------------------------------------------------------------------
int vtkSlicerDoubleArraysLogicAddFileTest( int argc, char * argv[] )
{
  bool res = true;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  res = testAddEmptyFile(nullptr) && res;
  res = testAddEmptyFile("") && res;
  res = testAddEmptyFile("non existing file.badextension") && res;
  res = testAddEmptyFile("non existing file.vtk") && res;
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  if (argc > 1)
    {
    res = testAddFile(argv[1]) && res;
    }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//-----------------------------------------------------------------------------
bool testAddEmptyFile(const char * filePath)
{
  vtkNew<vtkSlicerDoubleArraysLogic> doubleArraysLogic;
  vtkMRMLDoubleArrayNode* doubleArray = doubleArraysLogic->AddDoubleArray(filePath);
  if (doubleArray != nullptr)
    {
    std::cerr << "Line " << __LINE__
              << ": Adding an invalid file (" << (filePath ? filePath : nullptr)
              << ") shall not return a valid doubleArray"
              << std::endl;
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  doubleArraysLogic->SetMRMLScene(scene.GetPointer());
  int nodeCount = scene->GetNumberOfNodes();

  doubleArray = doubleArraysLogic->AddDoubleArray(filePath);

  if (doubleArray != nullptr ||
      scene->GetNumberOfNodes() != nodeCount)
    {
    std::cerr << "Line " << __LINE__
              << ": Adding an invalid file ("<< (filePath ? filePath : nullptr)
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
  vtkNew<vtkSlicerDoubleArraysLogic> doubleArraysLogic;
  vtkMRMLDoubleArrayNode* doubleArray = doubleArraysLogic->AddDoubleArray(filePath);
  if (doubleArray != nullptr)
    {
    std::cerr << "Line " << __LINE__
              << ": File can't be loaded if no scene is set."
              << std::endl;
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  doubleArraysLogic->SetMRMLScene(scene.GetPointer());
  int nodeCount = scene->GetNumberOfNodes();
  doubleArray = doubleArraysLogic->AddDoubleArray(filePath);

  if (doubleArray == nullptr ||
      scene->GetNumberOfNodes() != nodeCount + 2)
    {
    std::cerr << "Line " << __LINE__
              << ": Adding an doubleArray should create 2 nodes. "
              << scene->GetNumberOfNodes() << " vs " << nodeCount + 2
              << std::endl;
    return false;
    }

  return true;
}

