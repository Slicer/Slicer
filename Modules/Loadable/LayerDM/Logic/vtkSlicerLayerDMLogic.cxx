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

#include "vtkSlicerLayerDMLogic.h"

// Slicer includes
#include <vtkMRMLLayerDMWidgetEventTranslationNode.h>

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSlicerLayerDMLogic);

void vtkSlicerLayerDMLogic::RegisterNodes()
{
  RegisterNodeIfNeeded<vtkMRMLLayerDMWidgetEventTranslationNode>(GetMRMLScene());
}

void vtkSlicerLayerDMLogic::SetWidgetEventTranslationNode(vtkMRMLNode* node, vtkMRMLLayerDMWidgetEventTranslationNode* translationNode)
{
  SetReferenceNode(node, translationNode, EventTranslationRole);
}

vtkMRMLLayerDMWidgetEventTranslationNode* vtkSlicerLayerDMLogic::GetWidgetEventTranslationNode(vtkMRMLNode* node)
{
  return GetReferenceNode<vtkMRMLLayerDMWidgetEventTranslationNode>(node, EventTranslationRole);
}

vtkMRMLLayerDMWidgetEventTranslationNode* vtkSlicerLayerDMLogic::GetWidgetEventTranslationSingleton(vtkMRMLScene* scene, const std::string& singletonId)
{
  if (!scene)
  {
    return {};
  }

  return vtkMRMLLayerDMWidgetEventTranslationNode::SafeDownCast(scene->GetNodeByID(singletonId));
}

vtkMRMLLayerDMWidgetEventTranslationNode* vtkSlicerLayerDMLogic::GetWidgetEventTranslationSingleton(
  vtkMRMLScene* scene,
  const std::string& singletonId,
  const std::function<void(vtkMRMLLayerDMWidgetEventTranslationNode*)>& configureF)
{
  auto node = GetWidgetEventTranslationSingleton(scene, singletonId);
  if (!node)
  {
    node = CreateWidgetEventTranslationSingleton(scene, singletonId);
    configureF(node);
  }
  return node;
}

vtkMRMLLayerDMWidgetEventTranslationNode* vtkSlicerLayerDMLogic::CreateWidgetEventTranslationSingleton(vtkMRMLScene* scene, const std::string& singletonId)
{
  if (auto node = GetWidgetEventTranslationSingleton(scene, singletonId))
  {
    return node;
  }

  if (!scene)
  {
    return {};
  }

  vtkNew<vtkMRMLLayerDMWidgetEventTranslationNode> newNode;
  newNode->SetSingletonTag(singletonId.c_str());
  newNode->SetSaveWithScene(false);
  return vtkMRMLLayerDMWidgetEventTranslationNode::SafeDownCast(scene->AddNode(newNode));
}

void vtkSlicerLayerDMLogic::CreateDefaultEventTranslation(vtkMRMLNode* node,
                                                          const std::string& singletonId,
                                                          const std::function<void(vtkMRMLLayerDMWidgetEventTranslationNode*)>& configureF)
{
  if (GetWidgetEventTranslationNode(node))
  {
    return;
  }
  SetWidgetEventTranslationNode(node, GetWidgetEventTranslationSingleton(node ? node->GetScene() : nullptr, singletonId, configureF));
}

std::string vtkSlicerLayerDMLogic::GetEventTranslationRole()
{
  return EventTranslationRole;
}

std::string vtkSlicerLayerDMLogic::GetDisplayRole()
{
  return DisplayRole;
}
