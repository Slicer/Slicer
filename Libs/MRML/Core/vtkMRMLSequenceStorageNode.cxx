/*==============================================================================

Program: 3D Slicer

Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

==============================================================================*/

#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>

// Qt includes
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QPixmap>

#include <vtksys/SystemTools.hxx>

static const char NODE_BASE_NAME_SEPARATOR[] = "-";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSequenceStorageNode);

//----------------------------------------------------------------------------
vtkMRMLSequenceStorageNode::vtkMRMLSequenceStorageNode() = default;

//----------------------------------------------------------------------------
vtkMRMLSequenceStorageNode::~vtkMRMLSequenceStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLSequenceStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLSequenceStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLSequenceNode");
}

//----------------------------------------------------------------------------
int vtkMRMLSequenceStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLSequenceNode *sequenceNode = dynamic_cast <vtkMRMLSequenceNode *> (refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("ReadDataInternal: File name not specified");
    return 0;
    }

  // check that the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()) == false)
    {
    vtkErrorMacro("ReadDataInternal: model file '" << fullName.c_str() << "' not found.");
    return 0;
    }

  // compute file prefix
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if( extension.empty() )
    {
    vtkErrorMacro("ReadData: no file extension specified: " << fullName.c_str());
    return 0;
    }

  vtkDebugMacro("ReadDataInternal: extension = " << extension.c_str());

  // Custom nodes (such as vtkMRMLSceneView node) must be registered in the sequence scene,
  // otherwise the XML parser cannot instantiate them
  if (this->GetScene() && sequenceNode->GetSequenceScene())
    {
    this->GetScene()->CopyRegisteredNodesToScene(sequenceNode->GetSequenceScene());
    }
  else
    {
    vtkErrorMacro("Cannot register nodes in the sequence node");
    }

  int success = false;
  if (extension == std::string(".mrb"))
    {
    vtkMRMLScene* sequenceScene = sequenceNode->GetSequenceScene();
    success = sequenceScene->ReadFromMRB(fullName.c_str());
    if (success)
      {
      // Remove scene view nodes, as they would interfere with re-saving of the embedded scene
      // It would be better to disable automatic adding of scene view nodes altogether, but there is no API for that in the Slicer core.
      std::vector<vtkMRMLNode*> sceneViewNodes;
      sequenceScene->GetNodesByClass("vtkMRMLSceneViewNode", sceneViewNodes);
      for (std::vector<vtkMRMLNode*>::iterator sceneViewNodeIt = sceneViewNodes.begin(); sceneViewNodeIt != sceneViewNodes.end(); ++sceneViewNodeIt)
        {
        sequenceScene->RemoveNode(*sceneViewNodeIt);
        }

      // Read sequence index information from node embedded in the internal scene
      vtkMRMLSequenceNode* embeddedSequenceNode = vtkMRMLSequenceNode::SafeDownCast(
        sequenceScene->GetSingletonNode("SequenceIndex", "vtkMRMLSequenceNode"));
      if (embeddedSequenceNode)
        {
        sequenceNode->CopySequenceIndex(embeddedSequenceNode);
        // Convert node IDs to node pointers
        sequenceNode->UpdateSequenceIndex();
        sequenceScene->RemoveNode(embeddedSequenceNode);
        }
      }
    }
  else
    {
    vtkErrorMacro("Cannot read sequence file '" << fullName.c_str() << "' (extension = " << extension.c_str() << ")");
    }

  return success ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkMRMLSequenceStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLSequenceNode *sequenceNode = vtkMRMLSequenceNode::SafeDownCast(refNode);

  // Custom nodes (such as vtkMRMLSceneView node) must be registered in the sequence scene,
  // otherwise we could not create default storage nodes.
  if (this->GetScene() && sequenceNode->GetSequenceScene())
    {
    this->GetScene()->CopyRegisteredNodesToScene(sequenceNode->GetSequenceScene());
    }
  else
    {
    vtkErrorMacro("Cannot register nodes in the sequence node");
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLSequenceNode: File name not specified");
    return 0;
    }

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);

  bool success = false;
  if (extension == ".mrb")
    {
    this->ForceUniqueDataNodeFileNames(sequenceNode); // Prevents storable nodes' files from being overwritten due to the same node name
    vtkMRMLScene *sequenceScene=sequenceNode->GetSequenceScene();

    // Save sequence index information in the bundle file so that users can load
    // a sequence just from a .seq.mrb file
    vtkNew<vtkMRMLSequenceNode> embeddedSequenceNode;
    embeddedSequenceNode->CopySequenceIndex(sequenceNode);
    embeddedSequenceNode->SetSingletonTag("SequenceIndex");

    // We add a singleton node to the scene.
    // If there was a SequenceIndex node in the scene already then it will be overwritten.
    sequenceScene->AddNode(embeddedSequenceNode.GetPointer());

    success = sequenceScene->WriteToMRB(fullName.c_str());
    }
  else
    {
    vtkErrorMacro( << "No file extension recognized: " << fullName.c_str() );
    }

  return success ? 1 : 0;
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Sequence Bundle (.seq.mrb)");
  this->SupportedReadFileTypes->InsertNextValue("Medical Reality Bundle (.mrb)");
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Sequence Bundle (.seq.mrb)");
  this->SupportedWriteFileTypes->InsertNextValue("Medical Reality Bundle (.mrb)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLSequenceStorageNode::GetDefaultWriteFileExtension()
{
  return "seq.mrb";
}



//----------------------------------------------------------------------------
std::string vtkMRMLSequenceStorageNode::GetSequenceBaseName(const std::string& fileNameName, const std::string& itemName)
{
  std::string baseNodeName = fileNameName;
  std::string fileNameNameLowercase = vtksys::SystemTools::LowerCase(fileNameName);

  // strip known file extensions from filename to get base name
  std::vector<std::string> recognizedExtensions;
  if (!itemName.empty())
    {
    recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + itemName + NODE_BASE_NAME_SEPARATOR + "Seq.seq.mrb");
    recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + itemName + NODE_BASE_NAME_SEPARATOR + "Seq.seq.mha");
    recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + itemName + NODE_BASE_NAME_SEPARATOR + "Seq.seq.mhd");
    recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + itemName + NODE_BASE_NAME_SEPARATOR + "Seq.seq.nrrd");
    recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + itemName + NODE_BASE_NAME_SEPARATOR + "Seq.seq.nhdr");
    }
  recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + "Seq.seq.mrb");
  recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + "Seq.seq.mha");
  recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + "Seq.seq.mhd");
  recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + "Seq.seq.nrrd");
  recognizedExtensions.push_back(std::string(NODE_BASE_NAME_SEPARATOR) + "Seq.seq.nhdr");
  recognizedExtensions.push_back(".seq.mrb");
  recognizedExtensions.push_back(".seq.mha");
  recognizedExtensions.push_back(".seq.mhd");
  recognizedExtensions.push_back(".seq.nrrd");
  recognizedExtensions.push_back(".seq.nhdr");
  recognizedExtensions.push_back(".mrb");
  recognizedExtensions.push_back(".mhd");
  recognizedExtensions.push_back(".mha");
  recognizedExtensions.push_back(".nrrd");
  recognizedExtensions.push_back(".nhdr");
  for (std::vector<std::string>::iterator recognizedExtensionIt = recognizedExtensions.begin();
    recognizedExtensionIt != recognizedExtensions.end(); ++recognizedExtensionIt)
    {
    std::string recognizedExtensionLowercase = vtksys::SystemTools::LowerCase(*recognizedExtensionIt);
    if (fileNameNameLowercase.length() > recognizedExtensionLowercase.length() &&
      fileNameNameLowercase.compare(fileNameNameLowercase.length() - recognizedExtensionLowercase.length(),
      recognizedExtensionLowercase.length(), recognizedExtensionLowercase) == 0)
      {
      baseNodeName.erase(baseNodeName.size() - recognizedExtensionLowercase.length(), recognizedExtensionLowercase.length());
      break;
      }
    }

  return baseNodeName;
}

//----------------------------------------------------------------------------
std::string vtkMRMLSequenceStorageNode::GetSequenceNodeName(const std::string& baseName, const std::string& itemName)
{
  std::string fullName = baseName
    + NODE_BASE_NAME_SEPARATOR + itemName
    + NODE_BASE_NAME_SEPARATOR + "Seq";
  return fullName;
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceStorageNode::ForceUniqueDataNodeFileNames(vtkMRMLSequenceNode* sequenceNode)
{
  if (!sequenceNode)
    {
    return;
    }

  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); i++)
    {
    vtkMRMLStorableNode* currStorableNode = vtkMRMLStorableNode::SafeDownCast(sequenceNode->GetNthDataNode(i));
    if (!currStorableNode)
      {
      continue;
      }
    vtkMRMLStorageNode* currStorageNode = currStorableNode->GetStorageNode();
    if (!currStorageNode)
      {
      currStorableNode->AddDefaultStorageNode();
      currStorageNode = currStorableNode->GetStorageNode();
      }
    if (!currStorageNode)
      {
      // no need for storage node
      continue;
      }
    std::stringstream uniqueFileNameStream;
    uniqueFileNameStream << currStorableNode->GetName(); // Note that special characters are dealt with by the application logic when writing scene
    uniqueFileNameStream << "_" << i; // All file names are suffixed by the item number, ensuring uniqueness
    std::string uniqueFileName = uniqueFileNameStream.str();
    currStorageNode->SetFileName(uniqueFileName.c_str());
    }
}
