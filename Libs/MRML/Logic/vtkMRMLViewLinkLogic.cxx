/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// MRMLLogic includes
#include "vtkMRMLViewLinkLogic.h"
#include "vtkMRMLApplicationLogic.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkCollection.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLViewLinkLogic);

//----------------------------------------------------------------------------
vtkMRMLViewLinkLogic::vtkMRMLViewLinkLogic() = default;

//----------------------------------------------------------------------------
vtkMRMLViewLinkLogic::~vtkMRMLViewLinkLogic() = default;

//----------------------------------------------------------------------------
void vtkMRMLViewLinkLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  // List of events the slice logics should listen
  vtkNew<vtkIntArray> events;
  vtkNew<vtkFloatArray> priorities;

  float normalPriority = 0.0;

  // Events that use the default priority.  Don't care the order they
  // are triggered
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  priorities->InsertNextValue(normalPriority);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  priorities->InsertNextValue(normalPriority);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer(), priorities.GetPointer());

  this->ProcessMRMLSceneEvents(newScene, vtkCommand::ModifiedEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLViewLinkLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkMRMLViewNode") || node->IsA("vtkMRMLCameraNode"))
    {
    vtkEventBroker::GetInstance()->AddObservation(
      node, vtkCommand::ModifiedEvent, this, this->GetMRMLNodesCallbackCommand());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLinkLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkMRMLViewNode") || node->IsA("vtkMRMLCameraNode"))
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      node, vtkCommand::ModifiedEvent, this, this->GetMRMLNodesCallbackCommand());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLinkLogic::OnMRMLNodeModified(vtkMRMLNode* node)
{
  // Update from CameraNode
  vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(node);
  if (cameraNode && cameraNode->GetID() &&
      this->GetMRMLScene() && !this->GetMRMLScene()->IsBatchProcessing())
    {

    // if this is not the node that we are interacting with, short circuit
    if (!cameraNode->GetInteracting() || !cameraNode->GetInteractionFlags())
      {
      return;
      }

    // CameraNode was modified. Need to find the corresponding
    // ViewNode to check whether operations are linked
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast
      (this->GetMRMLScene()->GetNodeByID(cameraNode->GetActiveTag()));

    if (viewNode && viewNode->GetLinkedControl())
      {
      this->BroadcastCameraNodeEvent(cameraNode);
      }
    else
      {
      // camera node changed and views are not linked. Do not broadcast.
      return;
      }
    }

  // Update from viewNode
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(node);
  if (viewNode && viewNode->GetID() &&
      this->GetMRMLScene() && !this->GetMRMLScene()->IsBatchProcessing())
    {
    // if this is not the node that we are interacting with, short circuit
    if (!viewNode->GetInteracting()
        || !viewNode->GetInteractionFlags())
      {
      return;
      }

    if (viewNode && viewNode->GetLinkedControl())
      {
      // view node changed and views are linked. Broadcast.
      this->BroadcastViewNodeEvent(viewNode);
      }
    else
      {
      // view node changed and views are not linked. Do not broadcast.
      }
    }
}


//----------------------------------------------------------------------------
void vtkMRMLViewLinkLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();
}

//----------------------------------------------------------------------------
void vtkMRMLViewLinkLogic::BroadcastCameraNodeEvent(vtkMRMLCameraNode* cameraNode)
{
  // only broadcast a camera node event if we are not already actively
  // broadcasting events and we are actively interacting with the node
  if (!cameraNode || !cameraNode->GetInteracting())
    {
    return;
    }

  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast
    (this->GetMRMLScene()->GetNodeByID(cameraNode->GetActiveTag()));

  if (!viewNode)
    {
    return;
    }

  int requiredViewGroup = viewNode->GetViewGroup();
  vtkMRMLCameraNode* sNode = nullptr;
  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->GetMRMLScene()->GetNodesByClass("vtkMRMLCameraNode"));
  for (nodes->InitTraversal(it);
      (sNode = vtkMRMLCameraNode::SafeDownCast(nodes->GetNextItemAsObject(it)));)
    {  
    if (!sNode || sNode == cameraNode)
      {
      continue;
      }

    vtkMRMLViewNode* sViewNode = vtkMRMLViewNode::SafeDownCast
      (this->GetMRMLScene()->GetNodeByID(cameraNode->GetActiveTag()));

    if (!sViewNode || sViewNode->GetViewGroup() != requiredViewGroup)
      {
      continue;
      }

    vtkCamera* sCamera = sNode->GetCamera();
    if (!sCamera)
      {
      continue;
      }

    // Axis selection
    if (cameraNode->GetInteractionFlags() == vtkMRMLCameraNode::LookFromAxis)
      {
      vtkCamera* camera = cameraNode->GetCamera();
      if (camera)
        {
        int wasModifying = sNode->StartModify();

        if(sNode->GetParentTransformNode())
          {
          vtkNew<vtkTransform> cameraTransform;
          vtkNew<vtkMatrix4x4> cameraTransformMatrix;
          // Assumption: mrmlCamera nodes are only linearly transformed
          vtkMRMLTransformNode::GetMatrixTransformBetweenNodes
            (nullptr, sNode->GetParentTransformNode(), cameraTransformMatrix.GetPointer());
          cameraTransformMatrix->Invert(cameraTransformMatrix.GetPointer(), cameraTransformMatrix.GetPointer());
          cameraTransform->Concatenate(cameraTransformMatrix.GetPointer());

          double Position[3] = {0.0};
          cameraTransform->Update();
          cameraTransform->InternalTransformPoint(camera->GetPosition(), Position);
          sCamera->SetPosition(Position);

          double ViewUp[3] = {0.0};
          cameraTransform->TransformNormal(camera->GetViewUp(), ViewUp);
          sCamera->SetViewUp(ViewUp);
          }
        else
          {
          sCamera->SetPosition(camera->GetPosition());
          sCamera->SetViewUp(camera->GetViewUp());
          }

        sNode->EndModify(wasModifying);
        sNode->InvokeCustomModifiedEvent(vtkMRMLCameraNode::ResetCameraClippingEvent);
        }
      }
    // ZoomIn
    else if (cameraNode->GetInteractionFlags() == vtkMRMLCameraNode::ZoomInFlag)
      {
      // The zoom factor value is defined in the constructor of ctkVTKRenderView
      double zoomFactor = 0.05;
      if (sCamera)
        {
        int wasModifying = sNode->StartModify();

        if (sCamera->GetParallelProjection())
          {
          sCamera->SetParallelScale(sCamera->GetParallelScale() / (1 + zoomFactor));
          }
        else
          {
          sCamera->Dolly(1 + zoomFactor);
          }

        sNode->EndModify(wasModifying);
        sNode->InvokeCustomModifiedEvent(vtkMRMLCameraNode::ResetCameraClippingEvent);
        }
      }
    // ZoomOut
    else if (cameraNode->GetInteractionFlags() == vtkMRMLCameraNode::ZoomOutFlag)
      {
      // The zoom factor value is defined in the constructor of ctkVTKRenderView
      double zoomFactor = -0.05;
      if (sCamera)
        {
        int wasModifying = sNode->StartModify();

        if (sCamera->GetParallelProjection())
          {
          sCamera->SetParallelScale(sCamera->GetParallelScale() / (1 + zoomFactor));
          }
        else
          {
          sCamera->Dolly(1 + zoomFactor);
          }

        sNode->EndModify(wasModifying);
        sNode->InvokeCustomModifiedEvent(vtkMRMLCameraNode::ResetCameraClippingEvent);
        }
      }
    // Reset Focal Point
    else if (cameraNode->GetInteractionFlags() == vtkMRMLCameraNode::CenterFlag)
      {
      vtkCamera* camera = cameraNode->GetCamera();
      if (camera)
        {
        int wasModifying = sNode->StartModify();

        if(sNode->GetParentTransformNode())
          {
          vtkNew<vtkTransform> cameraTransform;
          vtkNew<vtkMatrix4x4> cameraTransformMatrix;
          // Assumption: mrmlCamera nodes are only linearly transformed
          vtkMRMLTransformNode::GetMatrixTransformBetweenNodes
            (nullptr, sNode->GetParentTransformNode(), cameraTransformMatrix.GetPointer());
          cameraTransformMatrix->Invert(cameraTransformMatrix.GetPointer(), cameraTransformMatrix.GetPointer());
          cameraTransform->Concatenate(cameraTransformMatrix.GetPointer());

          double FocalPoint[3] = {0.};
          cameraTransform->Update();
          cameraTransform->InternalTransformPoint(camera->GetFocalPoint(), FocalPoint);
          sCamera->SetFocalPoint(FocalPoint);
          }
        else
          {
          sCamera->SetFocalPoint(camera->GetFocalPoint());
          }

        sNode->EndModify(wasModifying);
        sNode->InvokeCustomModifiedEvent(vtkMRMLCameraNode::ResetCameraClippingEvent);
        }
      }
    // Update vtkCameras (i.e., mouse interactions)
    else if (cameraNode->GetInteractionFlags() == vtkMRMLCameraNode::CameraInteractionFlag)
      {
      vtkCamera* camera = cameraNode->GetCamera();
      if (camera)
        {
        int wasModifying = sNode->StartModify();

        if(sNode->GetParentTransformNode())
          {
          vtkNew<vtkTransform> cameraTransform;
          vtkNew<vtkMatrix4x4> cameraTransformMatrix;
          // Assumption: mrmlCamera nodes are only linearly transformed
          vtkMRMLTransformNode::GetMatrixTransformBetweenNodes
            (nullptr, sNode->GetParentTransformNode(), cameraTransformMatrix.GetPointer());
          cameraTransformMatrix->Invert(cameraTransformMatrix.GetPointer(), cameraTransformMatrix.GetPointer());
          cameraTransform->Concatenate(cameraTransformMatrix.GetPointer());

          double Position[3] = {0.0};
          cameraTransform->Update();
          cameraTransform->InternalTransformPoint(camera->GetPosition(), Position);
          sCamera->SetPosition(Position);

          double FocalPoint[3] = {0.0};
          cameraTransform->Update();
          cameraTransform->InternalTransformPoint(camera->GetFocalPoint(), FocalPoint);
          sCamera->SetFocalPoint(FocalPoint);

          double ViewUp[3] = {0.0};
          cameraTransform->TransformNormal(camera->GetViewUp(), ViewUp);
          sCamera->SetViewUp(ViewUp);
          }
        else
          {
          sCamera->SetPosition(camera->GetPosition());
          sCamera->SetFocalPoint(camera->GetFocalPoint());
          sCamera->SetViewUp(camera->GetViewUp());
          }
        if (camera->GetParallelProjection())
          {
          sCamera->SetParallelScale(camera->GetParallelScale());
          }

        sNode->EndModify(wasModifying);
        sNode->InvokeCustomModifiedEvent(vtkMRMLCameraNode::ResetCameraClippingEvent);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLinkLogic::BroadcastViewNodeEvent(vtkMRMLViewNode* viewNode)
{
  // only broadcast a view node event if we are not already actively
  // broadcasting events and we actively interacting with the node
  if (!viewNode || !viewNode->GetInteracting())
    {
    return;
    }

  int requiredViewGroup = viewNode->GetViewGroup();
  vtkMRMLViewNode* vNode = nullptr;
  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->GetMRMLScene()->GetNodesByClass("vtkMRMLViewNode"));

  for (nodes->InitTraversal(it);
      (vNode = vtkMRMLViewNode::SafeDownCast(nodes->GetNextItemAsObject(it)));)
    {
    if (!vNode || vNode == viewNode || vNode->GetViewGroup() != requiredViewGroup)
      {
      continue;
      }

    // RenderMode selection
    if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::RenderModeFlag)
      {
      vNode->SetRenderMode(viewNode->GetRenderMode());
      }
    // AnimationMode selection
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::AnimationModeFlag)
      {
      vNode->SetAnimationMode(viewNode->GetAnimationMode());
      }
    // Box visibility
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::BoxVisibleFlag)
      {
      vNode->SetBoxVisible(viewNode->GetBoxVisible());
      }
    // Box labels visibility
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::BoxLabelVisibileFlag)
      {
      vNode->SetAxisLabelsVisible(viewNode->GetAxisLabelsVisible());
      }
    // Background color
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::BackgroundColorFlag)
      {
      int wasModifying = vNode->StartModify();
      // The ThreeDView displayable manager will change the background color of
      // the renderer.

      vNode->SetBackgroundColor(viewNode->GetBackgroundColor());
      vNode->SetBackgroundColor2(viewNode->GetBackgroundColor2());
      vNode->EndModify(wasModifying);
      }
    // Stereo type
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::StereoTypeFlag)
      {
      vNode->SetStereoType(viewNode->GetStereoType());
      }
    // Orientation marker type
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::OrientationMarkerTypeFlag)
      {
      vNode->SetOrientationMarkerType(viewNode->GetOrientationMarkerType());
      }
    // Orientation marker size
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::OrientationMarkerSizeFlag)
      {
      vNode->SetOrientationMarkerSize(viewNode->GetOrientationMarkerSize());
      }
    // Ruler type
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::RulerTypeFlag)
      {
      vNode->SetRulerType(viewNode->GetRulerType());
      }
    // Ruler type
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::UseDepthPeelingFlag)
      {
      vNode->SetUseDepthPeeling(viewNode->GetUseDepthPeeling());
      }
    // FPS visibility
    else if (viewNode->GetInteractionFlags() == vtkMRMLViewNode::FPSVisibleFlag)
      {
      vNode->SetFPSVisible(viewNode->GetFPSVisible());
      }
    }
}
