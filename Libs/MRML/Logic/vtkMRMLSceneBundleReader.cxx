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

#include "vtkMRMLSceneBundleReader.h"

#include "vtkMRMLFileIOManager.h"
#include "vtkMRMLIOProperties.h"

// MRML includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>
#include <vtkWeakPointer.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <set>
#include <vector>

vtkStandardNewMacro(vtkMRMLSceneBundleReader);

//----------------------------------------------------------------------------
vtkMRMLSceneBundleReader::vtkMRMLSceneBundleReader()
{
  this->SetFileType("SceneFile");
  this->SetDescription(vtkMRMLTr("qSlicerSceneBundleReader", "MRB Slicer Data Bundle"));
  this->SetNameFilters(std::vector<std::string>{ "*.mrb", "*.zip", "*.xar" });
}

//----------------------------------------------------------------------------
vtkMRMLSceneBundleReader::~vtkMRMLSceneBundleReader() = default;

//----------------------------------------------------------------------------
bool vtkMRMLSceneBundleReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  vtkMRMLScene* scene = this->GetScene();
  if (!properties || !scene)
  {
    vtkErrorMacro("Load failed: invalid properties or scene");
    return false;
  }
  // unzipping needs an absolute path
  std::string file = vtksys::SystemTools::CollapseFullPath(properties->GetStringProperty("fileName"));
  bool clear = properties->GetBoolProperty("clear", false);

  // Get all the nodes that have been around before loading. Weak pointers are used, because nodes may be deleted
  // while loading (when the scene is cleared) and new nodes may then be created at the same address.
  std::vector<vtkWeakPointer<vtkMRMLNode>> nodesBeforeLoading;
  vtkCollection* nodes = scene->GetNodes();
  for (int index = 0; index < nodes->GetNumberOfItems(); ++index)
  {
    nodesBeforeLoading.emplace_back(vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(index)));
  }

  bool success = scene->ReadFromMRB(file.c_str(), clear, this->GetUserMessages());

  // Get all the new nodes
  std::set<vtkMRMLNode*> nodesPresentBeforeLoading;
  for (const vtkWeakPointer<vtkMRMLNode>& node : nodesBeforeLoading)
  {
    if (node)
    {
      nodesPresentBeforeLoading.insert(node);
    }
  }
  nodes = scene->GetNodes();
  for (int index = 0; index < nodes->GetNumberOfItems(); ++index)
  {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(index));
    if (!node || !node->GetID() || nodesPresentBeforeLoading.count(node))
    {
      continue;
    }
    this->AddLoadedNodeID(node->GetID());
  }

  if (success && this->GetFileIOManager())
  {
    // Set default scene file format to mrb
    this->GetFileIOManager()->SetDefaultSceneFileType(vtkMRMLTr("qSlicerSceneBundleReader", "Medical Reality Bundle") + " (.mrb)");
  }
  return success;
}
