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
#include <vtkMatrix4x4.h>
#include <vtkNew.h>

#include "vtkMRMLCoreTestingMacros.h"

//-----------------------------------------------------------------------------
int vtkMRMLSequenceStorageNodeTest1( int, char * [] )
{
  vtkNew<vtkMRMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLLinearTransformSequenceStorageNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLSequenceNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLSequenceStorageNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLVolumeSequenceStorageNode>::New());

  // Add generic node sequence
  {
    vtkSmartPointer<vtkMRMLSequenceNode> genericSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
    vtkSmartPointer<vtkMRMLModelNode> modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
    genericSequenceNode->SetDataNodeAtValue(modelNode.GetPointer(), "0");
    genericSequenceNode->AddDefaultStorageNode();
    vtkSmartPointer<vtkMRMLSequenceStorageNode> addedGenericStorageNode = vtkMRMLSequenceStorageNode::SafeDownCast(genericSequenceNode->GetStorageNode());
    CHECK_NOT_NULL(addedGenericStorageNode);
  }

  // Add volume node sequence
  {
    vtkSmartPointer<vtkMRMLSequenceNode> imageSequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
    vtkSmartPointer<vtkMRMLScalarVolumeNode> volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode"));
    imageSequenceNode->SetDataNodeAtValue(volumeNode.GetPointer(), "0");
    imageSequenceNode->AddDefaultStorageNode();
    vtkSmartPointer<vtkMRMLVolumeSequenceStorageNode> addedVolumeStorageNode
      = vtkMRMLVolumeSequenceStorageNode::SafeDownCast(imageSequenceNode->GetStorageNode());
    CHECK_NOT_NULL(addedVolumeStorageNode);
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
