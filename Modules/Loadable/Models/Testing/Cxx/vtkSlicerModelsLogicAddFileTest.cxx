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
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkPolyData.h>

//-----------------------------------------------------------------------------
bool testAddEmptyFile(const char* filePath);
bool testAddFile(const char* filePath);
bool testAddModelWithPolyData(bool withPolyData);

//-----------------------------------------------------------------------------
int vtkSlicerModelsLogicAddFileTest( int argc, char * argv[] )
{
  bool res = true;
  /*
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
  */
  if (argc > 1)
    {
    res = testAddFile(argv[1]) && res;
    }
  res = testAddModelWithPolyData(false) && res;
  res = testAddModelWithPolyData(true) && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//-----------------------------------------------------------------------------
bool testAddEmptyFile(const char * filePath)
{
  vtkNew<vtkSlicerModelsLogic> modelsLogic;
  vtkMRMLModelNode* model = modelsLogic->AddModel(filePath);
  if (model != nullptr)
    {
    std::cerr << "Error line " << __LINE__
              << ": Adding an invalid file shall not return a valid model"
              << std::endl;
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  modelsLogic->SetMRMLScene(scene.GetPointer());
  int nodeCount = scene->GetNumberOfNodes();

  model = modelsLogic->AddModel(filePath);

  if (model != nullptr ||
      scene->GetNumberOfNodes() != nodeCount)
    {
    std::cerr << "Error line " << __LINE__
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
  if (model != nullptr)
    {
    std::cerr << "Error line " << __LINE__
              << ": File can't be loaded if no scene is set."
              << std::endl;
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  modelsLogic->SetMRMLScene(scene.GetPointer());
  int nodeCount = scene->GetNumberOfNodes();
  model = modelsLogic->AddModel(filePath);

  if (model == nullptr ||
      scene->GetNumberOfNodes() != nodeCount + 3)
    {
    std::cerr << "Adding an model should create 3 nodes" << std::endl;
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool testAddModelWithPolyData(bool withPolyData)
{
  vtkNew<vtkPolyData> polyData;
  vtkPolyData* poly = (withPolyData ? polyData.GetPointer() : nullptr);
  vtkNew<vtkSlicerModelsLogic> modelsLogic;
  if (modelsLogic->AddModel(poly) != nullptr)
    {
    std::cout << "Error line " << __LINE__
      <<": vtkSlicerModelsLogic::AddModel(vtkPolyData*) failed."
      << std::endl;
    return false;
    }
  vtkNew<vtkMRMLScene> scene;
  modelsLogic->SetMRMLScene(scene.GetPointer());
  vtkMRMLModelNode* model = modelsLogic->AddModel(poly);
  if (model->GetPolyData() != poly ||
      model->GetModelDisplayNode() == nullptr)
    {
    std::cout << "Error line " << __LINE__
      <<": vtkSlicerModelsLogic::AddModel(vtkPolyData*) failed."
      << std::endl;
    return false;
    }
  return true;
}

