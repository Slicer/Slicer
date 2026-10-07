/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware SAS

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Thibault Pelletier, Kitware SAS,
  and was partially funded by ANR grants ANR-22-CE45-0034 and ANR-18-RHUS-005.

==============================================================================*/

#include "vtkSlicerLayerDMLogic.h"

// Slicer includes
#include <vtkMRMLLayerDMWidgetEventTranslationNode.h>

// VTK includes
#include <vtkObjectFactory.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerLayerDMLogic);

//-----------------------------------------------------------------------------
void vtkSlicerLayerDMLogic::RegisterNodes()
{
  RegisterNodeIfNeeded<vtkMRMLLayerDMWidgetEventTranslationNode>(this->GetMRMLScene());
}

//-----------------------------------------------------------------------------
void vtkSlicerLayerDMLogic::SetWidgetEventTranslationNode(vtkMRMLNode* node, vtkMRMLLayerDMWidgetEventTranslationNode* translationNode)
{
  SetReferenceNode(node, translationNode, EventTranslationRole);
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMWidgetEventTranslationNode* vtkSlicerLayerDMLogic::GetWidgetEventTranslationNode(vtkMRMLNode* node)
{
  return GetReferenceNode<vtkMRMLLayerDMWidgetEventTranslationNode>(node, EventTranslationRole);
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMWidgetEventTranslationNode* vtkSlicerLayerDMLogic::GetWidgetEventTranslationSingleton(vtkMRMLScene* scene, const std::string& singletonId)
{
  if (!scene)
  {
    return {};
  }

  return vtkMRMLLayerDMWidgetEventTranslationNode::SafeDownCast(scene->GetNodeByID(singletonId));
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
std::string vtkSlicerLayerDMLogic::GetEventTranslationRole()
{
  return EventTranslationRole;
}

//-----------------------------------------------------------------------------
std::string vtkSlicerLayerDMLogic::GetDisplayRole()
{
  return DisplayRole;
}

//-----------------------------------------------------------------------------
void vtkSlicerLayerDMLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
