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

// MRML includes
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneViewNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

bool connect(const char* sceneFilePath);
bool connectTwice(const char* sceneFilePath);
bool import(const char* sceneFilePath);
bool importTwice(const char* sceneFilePath);

//---------------------------------------------------------------------------
int vtkMRMLSceneImportTest(int argc, char * argv[] )
{
  if (argc < 2)
    {
    std::cout << "Usage: vtkMRMLSceneImportTest scene_file_path.mrml"
              << std::endl;
    return EXIT_FAILURE;
    }
  const char* sceneFilePath = argv[1];
  bool res = true;
  res = connect("") && res;
  res = connect(sceneFilePath) && res;
  res = connectTwice(sceneFilePath) && res;
  res = import("") && res;
  res = import(sceneFilePath) && res;
  res = importTwice(sceneFilePath) && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
bool connect(const char * sceneFilePath)
{
  vtkNew<vtkMRMLScene> scene;
  scene->SetURL(sceneFilePath);
  scene->Connect();
  return true;
}

//---------------------------------------------------------------------------
bool connectTwice(const char * sceneFilePath)
{
  vtkNew<vtkMRMLScene> scene;
  scene->SetURL(sceneFilePath);
  scene->Connect();
  int numberOfNodes = scene->GetNumberOfNodes();
  scene->Connect();
  int newNumberOfNodes = scene->GetNumberOfNodes();
  return numberOfNodes == newNumberOfNodes;
}

//---------------------------------------------------------------------------
bool import(const char * sceneFilePath)
{
  vtkNew<vtkMRMLScene> scene;
  scene->SetURL(sceneFilePath);
  scene->Import();
  return true;
}

//---------------------------------------------------------------------------
bool importTwice(const char * sceneFilePath)
{
  vtkNew<vtkMRMLScene> scene;
  scene->SetURL(sceneFilePath);
  scene->Import();
  scene->Import();
  return true;
}

