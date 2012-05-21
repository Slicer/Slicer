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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// ModuleTemplate includes
#include "vtkSlicerCamerasModuleLogic.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

// STD includes
#include <cassert> 

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCamerasModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerCamerasModuleLogic::vtkSlicerCamerasModuleLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerCamerasModuleLogic::~vtkSlicerCamerasModuleLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerCamerasModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkMRMLCameraNode* vtkSlicerCamerasModuleLogic
::GetViewActiveCameraNode(vtkMRMLViewNode* viewNode)
{
  vtkCollection* nodes = this->GetMRMLScene() ? this->GetMRMLScene()->GetNodes() : 0;
  if (nodes == 0 || viewNode == 0)
    {
    return 0;
    }
  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(nodes->GetNextItemAsObject(it))) ;)
    {
    vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(node);
    if (cameraNode &&
        cameraNode->GetActiveTag() &&
        !strcmp(cameraNode->GetActiveTag(), viewNode->GetID()))
      {
      return cameraNode;
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
//void vtkSlicerCamerasModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
//{
//  vtkNew<vtkIntArray> events;
//  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
//  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
//  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
//  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
//  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
//}
//
////---------------------------------------------------------------------------
//void vtkSlicerCamerasModuleLogic::UpdateFromMRMLScene()
//{
//}
