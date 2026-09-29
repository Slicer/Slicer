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

#include "vtkSlicerTextsReader.h"

// Logic includes
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTextNode.h>
#include <vtkMRMLTextStorageNode.h>

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSlicerTextsReader);

//----------------------------------------------------------------------------
vtkSlicerTextsReader::vtkSlicerTextsReader()
{
  this->SetFileType("TextFile");
  this->SetDescription("Text file");
  this->SetNameFilters(std::vector<std::string>{ "Text file (*.txt)", "XML document (*.xml)", "JSON document (*.json)" });
}

//----------------------------------------------------------------------------
vtkSlicerTextsReader::~vtkSlicerTextsReader() = default;

//----------------------------------------------------------------------------
bool vtkSlicerTextsReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  this->GetUserMessages()->ClearMessages();
  vtkMRMLScene* scene = this->GetScene();
  if (!properties || !properties->HasProperty("fileName") || !scene)
  {
    vtkErrorMacro("Load: did not receive fileName property or scene is invalid");
    return false;
  }

  vtkSmartPointer<vtkMRMLTextStorageNode> storageNode = vtkMRMLTextStorageNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextStorageNode"));
  if (!storageNode)
  {
    return false;
  }

  std::string fileName = properties->GetStringProperty("fileName");
  std::string textNodeName = scene->GetUniqueNameByString(storageNode->GetFileNameWithoutExtension(fileName.c_str()).c_str());
  vtkSmartPointer<vtkMRMLTextNode> textNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode", textNodeName));
  if (!textNode)
  {
    scene->RemoveNode(storageNode);
    return false;
  }
  textNode->SetAndObserveStorageNodeID(storageNode->GetID());

  storageNode->SetFileName(fileName.c_str());
  int retval = storageNode->ReadData(textNode);
  this->GetUserMessages()->AddMessages(storageNode->GetUserMessages());
  if (retval != 1)
  {
    vtkErrorMacro("Load: error reading " << fileName);
    scene->RemoveNode(textNode);
    scene->RemoveNode(storageNode);
    return false;
  }
  this->AddLoadedNodeID(textNode->GetID());
  return true;
}
