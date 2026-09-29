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

#include "vtkSlicerMarkupsWriter.h"

// Logic includes
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLMarkupsFiducialStorageNode.h>
#include <vtkMRMLMarkupsJsonStorageNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

vtkStandardNewMacro(vtkSlicerMarkupsWriter);

//----------------------------------------------------------------------------
vtkSlicerMarkupsWriter::vtkSlicerMarkupsWriter()
{
  this->SetDescription("Markups");
  this->SetFileType("MarkupsFile");
  this->SetNodeClassName("vtkMRMLMarkupsNode");
  this->SetSupportUseCompression(true);
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsWriter::~vtkSlicerMarkupsWriter() = default;

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerMarkupsWriter::GetNameFiltersForObject(vtkObject* vtkNotUsed(object))
{
  std::vector<std::string> nameFilters;
  vtkNew<vtkMRMLMarkupsJsonStorageNode> jsonStorageNode;
  vtkStringArray* jsonFormats = jsonStorageNode->GetSupportedWriteFileTypes();
  for (vtkIdType formatIt = 0; formatIt < jsonFormats->GetNumberOfValues(); ++formatIt)
  {
    nameFilters.push_back(jsonFormats->GetValue(formatIt));
  }
  vtkNew<vtkMRMLMarkupsFiducialStorageNode> fcsvStorageNode;
  vtkStringArray* fcsvFormats = fcsvStorageNode->GetSupportedWriteFileTypes();
  for (vtkIdType formatIt = 0; formatIt < fcsvFormats->GetNumberOfValues(); ++formatIt)
  {
    nameFilters.push_back(fcsvFormats->GetValue(formatIt));
  }
  return nameFilters;
}

//----------------------------------------------------------------------------
void vtkSlicerMarkupsWriter::SetStorageNodeClass(vtkMRMLStorableNode* storableNode, const std::string& storageNodeClassName)
{
  if (!storableNode)
  {
    vtkErrorMacro("SetStorageNodeClass failed: invalid storable node");
    return;
  }
  vtkMRMLScene* scene = storableNode->GetScene();
  if (!scene)
  {
    vtkErrorMacro("SetStorageNodeClass failed: invalid scene");
    return;
  }
  vtkMRMLStorageNode* currentStorageNode = storableNode->GetStorageNode();
  if (currentStorageNode != nullptr && currentStorageNode->IsA(storageNodeClassName.c_str()))
  {
    // requested storage node class is the same as current class, there is nothing to do
    return;
  }

  // Create and use new storage node of the correct class
  vtkMRMLStorageNode* newStorageNode = vtkMRMLStorageNode::SafeDownCast(scene->AddNewNodeByClass(storageNodeClassName));
  if (!newStorageNode)
  {
    vtkErrorMacro("SetStorageNodeClass failed: cannot create new storage node of class " << storageNodeClassName);
    return;
  }
  storableNode->SetAndObserveStorageNodeID(newStorageNode->GetID());

  // Remove old storage node
  if (currentStorageNode)
  {
    scene->RemoveNode(currentStorageNode);
  }
}

//----------------------------------------------------------------------------
bool vtkSlicerMarkupsWriter::Write(vtkMRMLIOProperties* properties)
{
  if (!properties)
  {
    return false;
  }
  vtkMRMLStorableNode* node = vtkMRMLStorableNode::SafeDownCast(this->GetNodeByID(properties->GetStringProperty("nodeID")));
  if (!node)
  {
    vtkErrorMacro("Write failed: invalid node");
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");

  vtkNew<vtkMRMLMarkupsFiducialStorageNode> fcsvStorageNode;
  std::string fcsvCompatibleFileExtension = fcsvStorageNode->GetSupportedFileExtension(fileName.c_str(), false, true);
  if (!fcsvCompatibleFileExtension.empty())
  {
    // fcsv file needs to be written
    this->SetStorageNodeClass(node, "vtkMRMLMarkupsFiducialStorageNode");
  }
  else
  {
    // json file needs to be written
    this->SetStorageNodeClass(node, node->GetDefaultStorageNodeClassName());
  }
  return this->Superclass::Write(properties);
}
