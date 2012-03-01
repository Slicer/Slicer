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

// Models logic
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

bool testAddEmptyFile(const char* filePath);
bool testAddFile(const char* filePath);

//-----------------------------------------------------------------------------
int vtkSlicerModelsLogicTest( int argc, char * argv[] )
{
  bool res = true;
  std::cout << ">>>>>>>>>>>>>>>>>> "
            << "The following can print errors and warnings"
            << " <<<<<<<<<<<<<<<<<<" << std::endl;
  res = testAddEmptyFile(0) && res;
  res = testAddEmptyFile("") && res;
  res = testAddEmptyFile("non existing file.badextension") && res;
  res = testAddEmptyFile("non existing file.vtk") && res;
  std::cout << ">>>>>>>>>>>>>>>>>> "
            << "end of normal errors and warnings"
            << " <<<<<<<<<<<<<<<<<<" << std::endl;
  if (argc > 1)
    {
    res = testAddFile(argv[1]) && res;
    }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//-----------------------------------------------------------------------------
bool testAddEmptyFile(const char * filePath)
{
  vtkNew<vtkSlicerModelsLogic> modelsLogic;
  vtkMRMLModelNode* model = modelsLogic->AddModel(filePath);
  if (model != 0)
    {
    std::cerr << "Line " << __LINE__
              << ": Adding an invalid file shall not return a valid model"
              << std::endl;
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  modelsLogic->SetMRMLScene(scene.GetPointer());
  int nodeCount = scene->GetNumberOfNodes();

  modelsLogic->AddModel(filePath);

  if (model != 0 ||
      scene->GetNumberOfNodes() != nodeCount)
    {
    std::cerr << "Line " << __LINE__
              << ": Adding an invalid file shall not add nodes in scene. "
              << scene->GetNumberOfNodes() << " vs " << nodeCount
              << std::endl;
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool testAddFile(const char * filePath)
{
  vtkNew<vtkSlicerModelsLogic> modelsLogic;
  vtkMRMLModelNode* model = modelsLogic->AddModel(filePath);
  if (model != 0)
    {
    std::cerr << "Line " << __LINE__
              << ": File can't be loaded if no scene is set."
              << std::endl;
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  modelsLogic->SetMRMLScene(scene.GetPointer());
  int nodeCount = scene->GetNumberOfNodes();
  model = modelsLogic->AddModel(filePath);

  if (model == 0 ||
      scene->GetNumberOfNodes() != nodeCount + 3)
    {
    std::cerr << "Adding an model should create 3 nodes" << std::endl;
    return false;
    }

  return true;
}
