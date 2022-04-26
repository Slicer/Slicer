/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// MRML includes
#include <vtkCacheManager.h>
#include <vtkDataIOManager.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLLinearTransformSequenceStorageNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSequenceNode.h>
#include <vtkMRMLSequenceStorageNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVolumeSequenceStorageNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPolyData.h>

#include "vtkMRMLCoreTestingMacros.h"

//-----------------------------------------------------------------------------
int TestWriteReadSequence(const std::string& tempDir, vtkMRMLSequenceNode* sequenceNode, vtkMRMLStorageNode* storageNode, std::string fileName)
{
  std::stringstream fullFilePathSS;
  fullFilePathSS << tempDir << "/" << fileName << "." << storageNode->GetDefaultWriteFileExtension();
  std::string fullFilePath = fullFilePathSS.str();
  if (vtksys::SystemTools::FileExists(fullFilePath.c_str(), true))
    {
    vtksys::SystemTools::RemoveFile(fullFilePath.c_str());
    }

  std::cout << "Testing sequence write: " << fullFilePath << std::endl;
  storageNode->SetFileName(fullFilePath.c_str());
  CHECK_BOOL(storageNode->WriteData(sequenceNode), true);

  vtkSmartPointer<vtkMRMLScene> scene = sequenceNode->GetScene();

  vtkSmartPointer<vtkMRMLSequenceNode> readSequenceNode = vtkSmartPointer<vtkMRMLSequenceNode>::Take(
    vtkMRMLSequenceNode::SafeDownCast(sequenceNode->CreateNodeInstance()));
  scene->AddNode(readSequenceNode);
  vtkSmartPointer<vtkMRMLStorageNode> readStorageNode = vtkSmartPointer<vtkMRMLStorageNode>::Take(
    vtkMRMLStorageNode::SafeDownCast(storageNode->CreateNodeInstance()));
  scene->AddNode(readStorageNode);

  std::cout << "Testing sequence read: " << fullFilePath << std::endl;
  readStorageNode->SetFileName(fullFilePath.c_str());
  CHECK_BOOL(readStorageNode->ReadData(readSequenceNode), true);
  CHECK_INT(readSequenceNode->GetNumberOfDataNodes(), sequenceNode->GetNumberOfDataNodes());
  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int vtkMRMLSequenceStorageNodeTest1( int argc, char * argv[] )
{
  std::string tempDir = ".";
  if (argc > 1)
    {
    tempDir = argv[1];
    }

  vtkNew<vtkMRMLScene> scene;
  scene->SetDataIOManager(vtkNew<vtkDataIOManager>());
  scene->GetDataIOManager()->SetCacheManager(vtkNew<vtkCacheManager>());
  scene->GetDataIOManager()->GetCacheManager()->SetRemoteCacheDirectory(tempDir.c_str());

  // Add generic node sequence
  {
    vtkSmartPointer<vtkMRMLSequenceNode> genericSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
    vtkSmartPointer<vtkMRMLModelNode> modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
    genericSequenceNode->SetDataNodeAtValue(modelNode.GetPointer(), "0");
    genericSequenceNode->AddDefaultStorageNode();
    vtkSmartPointer<vtkMRMLSequenceStorageNode> addedGenericStorageNode = vtkMRMLSequenceStorageNode::SafeDownCast(genericSequenceNode->GetStorageNode());
    CHECK_NOT_NULL(addedGenericStorageNode);
    CHECK_EXIT_SUCCESS(TestWriteReadSequence(tempDir, genericSequenceNode, addedGenericStorageNode, "TestGenericSequence"));
  }

  // Add volume node sequence
  {
    vtkSmartPointer<vtkMRMLSequenceNode> imageSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
    vtkNew<vtkImageData> image;
    image->SetDimensions(10, 10, 1);
    image->AllocateScalars(VTK_CHAR, 1);
    vtkSmartPointer<vtkMRMLScalarVolumeNode> volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode"));
    volumeNode->SetAndObserveImageData(image);
    imageSequenceNode->SetDataNodeAtValue(volumeNode.GetPointer(), "0");
    imageSequenceNode->AddDefaultStorageNode();
    vtkSmartPointer<vtkMRMLVolumeSequenceStorageNode> addedVolumeStorageNode
      = vtkMRMLVolumeSequenceStorageNode::SafeDownCast(imageSequenceNode->GetStorageNode());
    CHECK_EXIT_SUCCESS(TestWriteReadSequence(tempDir, imageSequenceNode, addedVolumeStorageNode, "TestImageSequence"));
  }

  // Add transform node sequence
  {
    vtkSmartPointer<vtkMRMLSequenceNode> transformSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
    vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode
      = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
    transformSequenceNode->SetDataNodeAtValue(transformNode.GetPointer(), "0");
    transformSequenceNode->AddDefaultStorageNode();
    vtkSmartPointer<vtkMRMLLinearTransformSequenceStorageNode> addedTransformStorageNode
      = vtkMRMLLinearTransformSequenceStorageNode::SafeDownCast(transformSequenceNode->GetStorageNode());
    CHECK_NOT_NULL(addedTransformStorageNode);
    CHECK_EXIT_SUCCESS(TestWriteReadSequence(tempDir, transformSequenceNode, addedTransformStorageNode, "TestTransformSequence"));
  }

  // Create generic node sequence
  {
    vtkSmartPointer<vtkMRMLSequenceNode> genericSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
    vtkSmartPointer<vtkMRMLModelNode> modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
    genericSequenceNode->SetDataNodeAtValue(modelNode.GetPointer(), "0");
    vtkSmartPointer<vtkMRMLSequenceStorageNode> createdGenericStorageNode = vtkSmartPointer<vtkMRMLSequenceStorageNode>::Take(
      vtkMRMLSequenceStorageNode::SafeDownCast(genericSequenceNode->CreateDefaultStorageNode()));
    CHECK_NOT_NULL(createdGenericStorageNode);
  }

  // Create volume node sequence
  {
    vtkSmartPointer<vtkMRMLSequenceNode> imageSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
    vtkSmartPointer<vtkMRMLScalarVolumeNode> volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode"));
    imageSequenceNode->SetDataNodeAtValue(volumeNode.GetPointer(), "0");
    vtkSmartPointer<vtkMRMLVolumeSequenceStorageNode> createdVolumeStorageNode = vtkSmartPointer<vtkMRMLVolumeSequenceStorageNode>::Take(
      vtkMRMLVolumeSequenceStorageNode::SafeDownCast(imageSequenceNode->CreateDefaultStorageNode()));
    CHECK_NOT_NULL(createdVolumeStorageNode);
  }

  // Create transform node sequence
  {
    vtkSmartPointer<vtkMRMLSequenceNode> transformSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
    vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode
      = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
    transformSequenceNode->SetDataNodeAtValue(transformNode.GetPointer(), "0");
    vtkSmartPointer<vtkMRMLLinearTransformSequenceStorageNode> createdTransformStorageNode
      = vtkSmartPointer<vtkMRMLLinearTransformSequenceStorageNode>::Take(
      vtkMRMLLinearTransformSequenceStorageNode::SafeDownCast(transformSequenceNode->CreateDefaultStorageNode()));
    CHECK_NOT_NULL(createdTransformStorageNode);
  }

  return EXIT_SUCCESS;
}
