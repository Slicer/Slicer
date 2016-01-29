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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SliceViewControllers Logic includes
#include "vtkSlicerViewControllersLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerViewControllersLogic);

//----------------------------------------------------------------------------
vtkSlicerViewControllersLogic::vtkSlicerViewControllersLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerViewControllersLogic::~vtkSlicerViewControllersLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerViewControllersLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
 }

//-----------------------------------------------------------------------------
void vtkSlicerViewControllersLogic::RegisterNodes()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkSlicerViewControllersLogic::RegisterNodes failed: invalid scene");
    return;
    }
}

//-----------------------------------------------------------------------------
vtkMRMLSliceNode* vtkSlicerViewControllersLogic::GetDefaultSliceViewNode()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkSlicerViewControllersLogic::GetDefaultSliceViewNode failed: invalid scene");
    return NULL;
    }
  vtkMRMLNode* defaultNode = scene->GetDefaultNodeByClass("vtkMRMLSliceNode");
  if (!defaultNode)
    {
    defaultNode = scene->CreateNodeByClass("vtkMRMLSliceNode");
    scene->AddDefaultNode(defaultNode);
    defaultNode->Delete(); // scene owns it now
    }
  return vtkMRMLSliceNode::SafeDownCast(defaultNode);
}

//-----------------------------------------------------------------------------
vtkMRMLViewNode* vtkSlicerViewControllersLogic::GetDefaultThreeDViewNode()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkSlicerViewControllersLogic::GetDefaultThreeDViewNode failed: invalid scene");
    return NULL;
    }
  vtkMRMLNode* defaultNode = scene->GetDefaultNodeByClass("vtkMRMLViewNode");
  if (!defaultNode)
    {
    defaultNode = scene->CreateNodeByClass("vtkMRMLViewNode");
    scene->AddDefaultNode(defaultNode);
    defaultNode->Delete(); // scene owns it now
    }
  return vtkMRMLViewNode::SafeDownCast(defaultNode);
}

//-----------------------------------------------------------------------------
void vtkSlicerViewControllersLogic::ResetAllViewNodesToDefault()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkSlicerViewControllersLogic::ResetAllViewNodesToDefault failed: invalid scene");
    return;
    }
  scene->StartState(vtkMRMLScene::BatchProcessState);
  vtkMRMLSliceNode* defaultSliceViewNode = GetDefaultSliceViewNode();
  std::vector< vtkMRMLNode* > viewNodes;
  scene->GetNodesByClass("vtkMRMLSliceNode", viewNodes);
  for (std::vector< vtkMRMLNode* >::iterator it = viewNodes.begin(); it != viewNodes.end(); ++it)
    {
    (*it)->Reset(defaultSliceViewNode);
    }
  viewNodes.clear();
  vtkMRMLViewNode* defaultThreeDViewNode = GetDefaultThreeDViewNode();
  scene->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector< vtkMRMLNode* >::iterator it = viewNodes.begin(); it != viewNodes.end(); ++it)
    {
    (*it)->Reset(defaultThreeDViewNode);
    }
  scene->EndState(vtkMRMLScene::BatchProcessState);
}
