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

  Based on Slicer/Base/GUI/vtkSlicerViewerWidget.cxx,
  this file was developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLCameraDisplayableManager.h"
#include "vtkMRMLThreeDViewInteractorStyle.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLCameraWidget.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLViewLogic.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLCameraDisplayableManager);

//---------------------------------------------------------------------------
class vtkMRMLCameraDisplayableManager::vtkInternal
{
public:
  vtkInternal();
  ~vtkInternal();

  vtkMRMLCameraNode* CameraNode;
  int UpdatingCameraNode;
  vtkSmartPointer<vtkMRMLCameraWidget> CameraWidget;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLCameraDisplayableManager::vtkInternal::vtkInternal()
{
  this->CameraNode = nullptr;
  this->UpdatingCameraNode = 0;
  this->CameraWidget = vtkSmartPointer<vtkMRMLCameraWidget>::New();
}

//---------------------------------------------------------------------------
vtkMRMLCameraDisplayableManager::vtkInternal::~vtkInternal()
{
  this->CameraWidget->SetRenderer(nullptr);
  this->CameraWidget->SetCameraNode(nullptr);
}

//---------------------------------------------------------------------------
// vtkMRMLCameraDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLCameraDisplayableManager::vtkMRMLCameraDisplayableManager()
{
  this->Internal = new vtkInternal;
}

//---------------------------------------------------------------------------
vtkMRMLCameraDisplayableManager::~vtkMRMLCameraDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "CameraNode: " << this->Internal->CameraNode << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::Create()
{
  this->UpdateCameraNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneEndClose()
{
  this->UpdateCameraNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneStartImport() {}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneEndImport()
{
  if (!this->GetMRMLScene())
  {
    this->SetAndObserveCameraNode(nullptr);
    return;
  }
  if (this->GetMRMLViewNode())
  {
    this->UpdateCameraNode();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneEndRestore()
{
  if (this->GetMRMLViewNode())
  {
    this->UpdateCameraNode();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (this->GetMRMLScene()->IsBatchProcessing() || !node->IsA("vtkMRMLCameraNode"))
  {
    return;
  }
  // maybe the camera node is a good match for the observed view node?
  this->UpdateCameraNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (this->GetMRMLScene()->IsBatchProcessing() || !node->IsA("vtkMRMLCameraNode"))
  {
    return;
  }
  if (node == this->Internal->CameraNode)
  {
    // we need to find another camera node for the observed view node
    this->UpdateCameraNode();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  switch (event)
  {
    // Maybe something external removed the view/camera link, make sure the
    // view observes a camera node, create a new camera node if needed
    case vtkMRMLCameraNode::LayoutNameModifiedEvent:
      assert(vtkMRMLCameraNode::SafeDownCast(caller));
      this->UpdateCameraNode();
      vtkDebugMacro("ProcessingMRML: got a camera node modified event");
      break;
    case vtkMRMLCameraNode::ResetCameraClippingEvent:
      assert(vtkMRMLCameraNode::SafeDownCast(caller));
      vtkDebugMacro("ProcessingMRML: got a camera node modified event");
      if (this->GetRenderer())
      {
        this->GetRenderer()->ResetCameraClippingRange();
        this->GetRenderer()->UpdateLightsGeometryToFollowCamera();
      }
      else if (this->GetCameraNode() && this->GetCameraNode()->GetCamera())
      {
        vtkCamera* camera = this->GetCameraNode()->GetCamera();
        camera->SetClippingRange(0.1, camera->GetDistance() * 2);
      }
      break;
    default:
      this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
      break;
  }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLNodeModified(vtkMRMLNode* vtkNotUsed(node))
{
  this->RequestRender();
}

//---------------------------------------------------------------------------
vtkMRMLCameraNode* vtkMRMLCameraDisplayableManager::GetCameraNode()
{
  return this->Internal->CameraNode;
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::SetAndObserveCameraNode(vtkMRMLCameraNode* newCameraNode)
{
  // If a camera node already points to me. Do I already have it?
  if (newCameraNode == this->Internal->CameraNode)
  {
    if (newCameraNode)
    {
      // I'm already pointing to it
      vtkDebugMacro("UpdateCamera: CameraNode ["
                    << newCameraNode->GetID() << "] is already pointing to my ViewNode and I'm observing it - "
                    << "Internal->CameraNode [" << this->Internal->CameraNode->GetID() << "]");
    }
    return;
  }
  // Do not associate the old camera node anymore with this view
  if (this->Internal->CameraNode && this->Internal->CameraNode->GetLayoutName() && this->GetMRMLViewNode()
      && this->GetMRMLViewNode()->GetLayoutName()
      && strcmp(this->Internal->CameraNode->GetLayoutName(), this->GetMRMLViewNode()->GetLayoutName()) == 0)
  {
    this->Internal->CameraNode->SetLayoutName(nullptr);
  }
  // Associate the new camera node
  if (newCameraNode)
  {
    newCameraNode->SetLayoutName(this->GetMRMLViewNode()->GetLayoutName());
  }
  vtkNew<vtkIntArray> cameraNodeEvents;
  cameraNodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  cameraNodeEvents->InsertNextValue(vtkMRMLCameraNode::LayoutNameModifiedEvent);
  cameraNodeEvents->InsertNextValue(vtkMRMLCameraNode::ResetCameraClippingEvent);

  vtkSetAndObserveMRMLNodeEventsMacro(this->Internal->CameraNode, newCameraNode, cameraNodeEvents.GetPointer());

  this->SetCameraToRenderer();
  this->InvokeEvent(vtkMRMLCameraDisplayableManager::ActiveCameraChangedEvent, newCameraNode);
  vtkMRMLViewNode* viewNode = this->GetMRMLViewNode();
  if (viewNode)
  {
    viewNode->Modified(); // update vtkCamera from view node (perspective/parallel, etc)
  }

  this->Internal->CameraWidget->SetRenderer(this->GetRenderer());
  this->Internal->CameraWidget->SetCameraNode(this->Internal->CameraNode);
};

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::RemoveMRMLObservers()
{
  //  this->RemoveCameraObservers();
  this->SetAndObserveCameraNode(nullptr);

  this->Superclass::RemoveMRMLObservers();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::UpdateCameraNode()
{
  if (this->Internal->UpdatingCameraNode)
  {
    return;
  }
  if (this->GetMRMLScene() == nullptr)
  {
    vtkErrorMacro("UpdateCameraNode: DisplayableManager does NOT have a scene set, "
                  "can't find CameraNodes");
    return;
  }

  vtkMRMLViewNode* viewNode = this->GetMRMLViewNode();
  if (!viewNode)
  {
    vtkErrorMacro("UpdateCameraNode: DisplayableManager does NOT have a valid view node");
    return;
  }

  // find ViewNode in the scene
  vtkSmartPointer<vtkMRMLCameraNode> updatedCameraNode =
    vtkMRMLViewLogic::GetCameraNode(this->GetMRMLScene(), viewNode->GetLayoutName());
  if (updatedCameraNode != nullptr && this->Internal->CameraNode == updatedCameraNode)
  {
    // no change
    return;
  }

  this->Internal->UpdatingCameraNode = 1;

  if (!updatedCameraNode && viewNode->GetLayoutName() && strlen(viewNode->GetLayoutName()) > 0)
  {
    // Use CreateNodeByClass instead of New to make the new node based on default node stored in the scene
    updatedCameraNode = vtkSmartPointer<vtkMRMLCameraNode>::Take(
      vtkMRMLCameraNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLCameraNode")));
    updatedCameraNode->SetName(this->GetMRMLScene()->GetUniqueNameByString(updatedCameraNode->GetNodeTagName()));
    updatedCameraNode->SetDescription(
      "Default Scene Camera"); // indicates that this is an automatically created default camera
    updatedCameraNode->SetLayoutName(viewNode->GetLayoutName());
    this->GetMRMLScene()->AddNode(updatedCameraNode);
  }
  this->SetAndObserveCameraNode(updatedCameraNode);

  this->Internal->UpdatingCameraNode = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::AdditionalInitializeStep()
{
  assert(this->GetRenderer());
  this->SetCameraToRenderer();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::SetCameraToRenderer()
{
  if (!this->GetRenderer())
  {
    return;
  }
  vtkCamera* camera = this->Internal->CameraNode ? this->Internal->CameraNode->GetCamera() : nullptr;
  this->GetRenderer()->SetActiveCamera(camera);
  if (camera)
  {
    // Do not call if there is no camera otherwise it will create a new one without a CameraNode
    this->GetRenderer()->ResetCameraClippingRange();
    // Default to ParallelProjection Off
    // camera->ParallelProjectionOff();
  }
}

//---------------------------------------------------------------------------
bool vtkMRMLCameraDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData,
                                                                 double& closestDistance2)
{
  // The CameraWidget does not have representation, so it cannot use the usual representation->GetInteractionNode()
  // method to get the interactor, therefore we make sure here that the view node is passed to it.
  if (!eventData->GetViewNode())
  {
    eventData->SetViewNode(this->GetMRMLViewNode());
  }
  return this->Internal->CameraWidget->CanProcessInteractionEvent(eventData, closestDistance2);
}

//---------------------------------------------------------------------------
bool vtkMRMLCameraDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  // The CameraWidget does not have representation, so it cannot use the usual representation->GetInteractionNode()
  // method to get the interactor, therefore we make sure here that the view node is passed to it.
  if (!eventData->GetViewNode())
  {
    eventData->SetViewNode(this->GetMRMLViewNode());
  }
  return this->Internal->CameraWidget->ProcessInteractionEvent(eventData);
}

//---------------------------------------------------------------------------
vtkMRMLCameraWidget* vtkMRMLCameraDisplayableManager::GetCameraWidget()
{
  return this->Internal->CameraWidget;
}
