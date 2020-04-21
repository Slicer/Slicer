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

// Cameras Logic includes
#include "vtkSlicerCamerasModuleLogic.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCamerasModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerCamerasModuleLogic::vtkSlicerCamerasModuleLogic()
{
  this->CopyImportedCameras = true;
}

//----------------------------------------------------------------------------
vtkSlicerCamerasModuleLogic::~vtkSlicerCamerasModuleLogic() = default;

//----------------------------------------------------------------------------
void vtkSlicerCamerasModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkMRMLCameraNode* vtkSlicerCamerasModuleLogic
::GetViewActiveCameraNode(vtkMRMLViewNode* viewNode)
{
  vtkCollection* nodes = this->GetMRMLScene() ? this->GetMRMLScene()->GetNodes() : nullptr;
  if (nodes == nullptr || viewNode == nullptr)
    {
    return nullptr;
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
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkSlicerCamerasModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAboutToBeAddedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerCamerasModuleLogic
::ProcessMRMLSceneEvents(vtkObject *caller, unsigned long event, void *callData)
{
  this->Superclass::ProcessMRMLSceneEvents(caller, event, callData);
  if (event == vtkMRMLScene::NodeAboutToBeAddedEvent &&
      this->GetMRMLScene()->IsImporting() &&
      this->CopyImportedCameras)
    {
    vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(
      reinterpret_cast<vtkObject*>(callData));
    if (cameraNode)
      {
      vtkSmartPointer<vtkCollection> existingCamerasWithSameName;
      existingCamerasWithSameName.TakeReference(
        this->GetMRMLScene()->GetNodesByClassByName(cameraNode->GetClassName(),
                                                    cameraNode->GetName()));
      vtkMRMLCameraNode* existingCameraWithSameName =
        vtkMRMLCameraNode::SafeDownCast(
          existingCamerasWithSameName->GetItemAsObject(0));
      if (existingCameraWithSameName)
        {
        int wasModifying = existingCameraWithSameName->StartModify();
        existingCameraWithSameName->SetParallelProjection(
          cameraNode->GetParallelProjection());
        existingCameraWithSameName->SetParallelScale(
          cameraNode->GetParallelScale());
        existingCameraWithSameName->SetPosition(
          cameraNode->GetPosition());
        existingCameraWithSameName->SetFocalPoint(
          cameraNode->GetFocalPoint());
        existingCameraWithSameName->SetViewUp(
          cameraNode->GetViewUp());
        // \tbd: Copy AppliedTransform ?
        existingCameraWithSameName->ResetClippingRange();
        existingCameraWithSameName->EndModify(wasModifying);
        }
      }
    }
}
