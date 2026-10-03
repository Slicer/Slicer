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

// Data logic includes
#include "vtkSlicerSceneReader.h"

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLIOProperties.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScriptedModuleNode.h>

// VTK includes
#include <vtkNew.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <iostream>
#include <vector>

namespace
{
//-----------------------------------------------------------------------------
std::vector<std::string> LoadedNodeNames(vtkSlicerSceneReader* reader, vtkMRMLScene* scene)
{
  std::vector<std::string> names;
  for (const std::string& nodeID : reader->GetLoadedNodeIDs())
  {
    vtkMRMLNode* node = scene->GetNodeByID(nodeID);
    names.push_back(node && node->GetName() ? node->GetName() : "(missing)");
  }
  std::sort(names.begin(), names.end());
  return names;
}
} // namespace

//-----------------------------------------------------------------------------
int vtkSlicerSceneReaderTest1(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: vtkSlicerSceneReaderTest1 /path/to/temporary/directory" << std::endl;
    return EXIT_FAILURE;
  }
  const std::string tempDir = argv[1];
  vtksys::SystemTools::MakeDirectory(tempDir);
  const std::string sceneFile = tempDir + "/vtkSlicerSceneReaderTest1.mrml";

  // Write a scene with two nodes
  {
    vtkNew<vtkMRMLScene> savedScene;
    for (const char* name : { "Saved1", "Saved2" })
    {
      vtkNew<vtkMRMLScriptedModuleNode> node;
      node->SetName(name);
      savedScene->AddNode(node);
    }
    savedScene->SetRootDirectory(tempDir.c_str());
    savedScene->SetURL(sceneFile.c_str());
    CHECK_BOOL(savedScene->Commit() != 0, true);
  }

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLScriptedModuleNode> existingNode;
  existingNode->SetName("Existing");
  scene->AddNode(existingNode);

  vtkNew<vtkSlicerSceneReader> reader;
  reader->SetScene(scene);
  vtkNew<vtkMRMLIOProperties> properties;
  properties->SetStringProperty("fileName", sceneFile);

  // Import: only the nodes that were added are reported
  CHECK_BOOL(reader->Load(properties), true);
  std::vector<std::string> expectedNames = { "Saved1", "Saved2" };
  CHECK_BOOL(LoadedNodeNames(reader, scene) == expectedNames, true);

  // Clear and load: the loaded nodes are reported (the nodes that were in the scene are removed)
  properties->SetBoolProperty("clear", true);
  CHECK_BOOL(reader->Load(properties), true);
  CHECK_BOOL(LoadedNodeNames(reader, scene) == expectedNames, true);
  CHECK_NULL(scene->GetFirstNodeByName("Existing"));

  return EXIT_SUCCESS;
}
