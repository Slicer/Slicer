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

#include "vtkSlicerSceneReader.h"

// Logic includes
#include <vtkMRMLFileIOManager.h>
#include <vtkMRMLIOOptionsDescription.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>
#include <vtkWeakPointer.h>

// STD includes
#include <set>
#include <sstream>
#include <tuple>
#include <vector>

namespace
{

//----------------------------------------------------------------------------
std::set<std::string> splitNonEmpty(const std::string& text, char separator)
{
  std::set<std::string> items;
  std::stringstream stream(text);
  std::string item;
  while (std::getline(stream, item, separator))
  {
    if (!item.empty())
    {
      items.insert(item);
    }
  }
  return items;
}
} // namespace

vtkStandardNewMacro(vtkSlicerSceneReader);

//----------------------------------------------------------------------------
vtkSlicerSceneReader::vtkSlicerSceneReader()
{
  this->SetFileType("SceneFile");
  this->SetDescription(vtkMRMLTr("qSlicerSceneReader", "MRML Scene"));
  this->SetNameFilters(std::vector<std::string>{ "*.mrml" });
}

//----------------------------------------------------------------------------
vtkSlicerSceneReader::~vtkSlicerSceneReader() = default;

//----------------------------------------------------------------------------
bool vtkSlicerSceneReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  vtkMRMLScene* scene = this->GetScene();
  if (!properties || !scene)
  {
    vtkErrorMacro("Load failed: invalid properties or scene");
    return false;
  }
  std::string file = properties->GetStringProperty("fileName");
  scene->SetURL(file.c_str());
  bool clear = properties->GetBoolProperty("clear", false);

  // Nodes that are in the scene before loading. Weak pointers are used, because nodes may be deleted
  // while loading (when the scene is cleared) and new nodes may then be created at the same address.
  std::vector<vtkWeakPointer<vtkMRMLNode>> nodesBeforeLoading;
  vtkCollection* nodes = scene->GetNodes();
  for (int index = 0; index < nodes->GetNumberOfItems(); ++index)
  {
    nodesBeforeLoading.emplace_back(vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(index)));
  }

  bool success = false;
  if (clear)
  {
    vtkDebugMacro("Clear and import into main MRML scene");
    success = scene->Connect(this->GetUserMessages());
    if (success && this->GetFileIOManager())
    {
      // Set default scene file format to .mrml
      this->GetFileIOManager()->SetDefaultSceneFileType(vtkMRMLTr("qSlicerSceneReader", "MRML Scene") + " (.mrml)");
    }
  }
  else
  {
    if (!properties->GetBoolProperty("copyCameras", true))
    {
      vtkWarningMacro("Load: copyCameras=false property is ignored, cameras are now always replaced in the scene");
    }
    success = scene->Import(this->GetUserMessages());
  }

  // Report the nodes that were added to the scene
  std::set<vtkMRMLNode*> nodesStillPresentFromBeforeLoading;
  for (const vtkWeakPointer<vtkMRMLNode>& node : nodesBeforeLoading)
  {
    if (node)
    {
      nodesStillPresentFromBeforeLoading.insert(node);
    }
  }
  nodes = scene->GetNodes();
  for (int index = 0; index < nodes->GetNumberOfItems(); ++index)
  {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(index));
    if (!node || !node->GetID() || nodesStillPresentFromBeforeLoading.count(node))
    {
      continue;
    }
    this->AddLoadedNodeID(node->GetID());
  }

  // Display warning message if scene file was created with a different application or with a future application version
  std::string currentApplication;
  int currentMajor = 0;
  int currentMinor = 0;
  int currentPatch = 0;
  int currentRevision = 0;
  std::string loadedApplication;
  int loadedMajor = 0;
  int loadedMinor = 0;
  int loadedPatch = 0;
  int loadedRevision = 0;
  if (vtkMRMLScene::ParseVersion(scene->GetVersion(), currentApplication, currentMajor, currentMinor, currentPatch, currentRevision) //
      && vtkMRMLScene::ParseVersion(scene->GetLastLoadedVersion(), loadedApplication, loadedMajor, loadedMinor, loadedPatch, loadedRevision))
  {
    std::vector<std::string> sceneVersionWarningMessages;
    if (loadedApplication != currentApplication)
    {
      sceneVersionWarningMessages.push_back(vtkMRMLI18N::Format(
        vtkMRMLTr("qSlicerSceneReader", "The scene file was saved with %1 application (this application is %2)."), loadedApplication.c_str(), currentApplication.c_str()));
    }
    if (std::make_tuple(loadedMajor, loadedMinor, loadedPatch) > std::make_tuple(currentMajor, currentMinor, currentPatch))
    {
      std::string loadedVersionStr = std::to_string(loadedMajor) + "." + std::to_string(loadedMinor) + "." + std::to_string(loadedPatch);
      std::string currentVersionStr = std::to_string(currentMajor) + "." + std::to_string(currentMinor) + "." + std::to_string(currentPatch);
      sceneVersionWarningMessages.push_back(
        vtkMRMLI18N::Format(vtkMRMLTr("qSlicerSceneReader", "The scene file was created with a newer version of the application (%1) than the current version (%2)."),
                            loadedVersionStr.c_str(),
                            currentVersionStr.c_str()));
    }
    if (!sceneVersionWarningMessages.empty())
    {
      sceneVersionWarningMessages.push_back(vtkMRMLTr("qSlicerSceneReader", "The scene may not load correctly."));
      std::string message;
      for (const std::string& warningMessage : sceneVersionWarningMessages)
      {
        message += (message.empty() ? "" : " ") + warningMessage;
      }
      this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent, message);
    }
  }

  // If there were scene loading errors then log the list of extensions that were installed when the scene was saved.
  // It may provide useful hints for why the extension load failed.
  if (!success                                                                          //
      || this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0 //
      || this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::WarningEvent) > 0)
  {
    std::string extensions = scene->GetExtensions() ? scene->GetExtensions() : "";
    std::string lastLoadedExtensions = scene->GetLastLoadedExtensions() ? scene->GetLastLoadedExtensions() : "";
    if (extensions != lastLoadedExtensions)
    {
      std::set<std::string> installedExtensions = splitNonEmpty(extensions, ';');
      std::string notInstalledExtensions;
      for (const std::string& extension : splitNonEmpty(lastLoadedExtensions, ';'))
      {
        if (installedExtensions.count(extension) == 0)
        {
          notInstalledExtensions += (notInstalledExtensions.empty() ? "" : ", ") + extension;
        }
      }
      if (!notInstalledExtensions.empty())
      {
        std::string extensionsInformation = vtkMRMLI18N::Format(vtkMRMLTr("qSlicerSceneReader",
                                                                          "These extensions were installed when the scene was saved but not installed now: %1."
                                                                          " These extensions may be required for successful loading of the scene."),
                                                                notInstalledExtensions.c_str());
        this->GetUserMessages()->AddMessage(vtkCommand::MessageEvent, extensionsInformation);
      }
    }
  }

  return success;
}

//----------------------------------------------------------------------------
void vtkSlicerSceneReader::GetOptionsDescription(vtkMRMLIOOptionsDescription* description)
{
  if (!description)
  {
    return;
  }
  description->AddBoolOption("clear", vtkMRMLTr("qSlicerSceneIOOptionsWidget", "Clear existing scene"), "", false);
}
