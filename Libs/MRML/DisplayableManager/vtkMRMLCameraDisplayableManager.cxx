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
#include "vtkThreeDViewInteractorStyle.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLCameraDisplayableManager );
vtkCxxRevisionMacro(vtkMRMLCameraDisplayableManager, "$Revision: 13525 $");

//---------------------------------------------------------------------------
class vtkMRMLCameraDisplayableManager::vtkInternal
{
public:
  vtkInternal();
  
  vtkMRMLCameraNode* CameraNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLCameraDisplayableManager::vtkInternal::vtkInternal()
{
  this->CameraNode = 0;
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
  //this->RemoveCameraObservers();
  this->SetAndObserveCameraNode(0);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneEndImport()
{
  if (!this->GetMRMLScene())
    {
    this->SetAndObserveCameraNode(0);
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
  assert(this->GetMRMLViewNode());
  if (!this->GetMRMLViewNode())
    {
    return;
    }

  // Backward compatibility change: all scenes saved so far do have
  // camera nodes with an empty or false active tag. Restoring a snapshot
  // will invalidate the current active camera. Try to catch that
  // by grabbing the first available camera. Sounds like a hack, but
  // too  much time was wasted on this thing.
  vtkMRMLCameraNode *camera_node = this->Internal->CameraNode;
  if (!camera_node)
    {
    camera_node = vtkMRMLCameraNode::SafeDownCast(
        this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLCameraNode"));
    camera_node->SetActiveTag(this->GetMRMLViewNode()->GetID());
    }
  else if (!camera_node->GetActiveTag())
    {
    camera_node->SetActiveTag(this->GetMRMLViewNode()->GetID());
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (this->GetMRMLScene()->IsBatchProcessing() ||
      !node->IsA("vtkMRMLCameraNode"))
    {
    return;
    }
  // maybe the camera node is a good match for the observed view node?
  this->UpdateCameraNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (this->GetMRMLScene()->IsBatchProcessing() ||
      !node->IsA("vtkMRMLCameraNode"))
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
void vtkMRMLCameraDisplayableManager::ProcessMRMLNodesEvents(vtkObject *caller,
                                                        unsigned long event,
                                                        void *callData)
{
  switch(event)
    {
    // Maybe something external removed the view/camera link, make sure the
    // view observes a camera node, create a new camera node if needed
    case vtkMRMLCameraNode::ActiveTagModifiedEvent:
      assert(vtkMRMLCameraNode::SafeDownCast(caller));
      this->UpdateCameraNode();
      vtkDebugMacro("ProcessingMRML: got a camera node modified event");
      break;
    default:
      this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
      break;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLNodeModified(vtkMRMLNode* node)
{
  // ModifiedEvent is fired anytime vtkCamera is modified (when there is a
  // pan, zoom, rotation...)
  // The observation is setup here: vtkSetAndObserveMRMLNodeMacro(
  // this->Internal->CameraNode, newCameraNode);
  assert(vtkMRMLCameraNode::SafeDownCast(node));
  vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(
    this->GetInteractor()->GetInteractorStyle());
  if (interactorStyle &&
      interactorStyle->GetState() != VTKIS_NONE)
    {
    // The interactor style is in a "state" mode (rotate, dolly...), that
    // means it is doing multiple actions on the camera at once (azimuth,
    // elevation...). When the interactor style is done with updating the
    // camera, it will call Render() on the interactor which will trigger a
    // rendering. We don't need to do it here then, there is even a risk to
    // trigger a rendering before the camera is done being updated (between
    // azimuth and elevation for example).
    return;
    }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::SetAndObserveCameraNode(vtkMRMLCameraNode * newCameraNode)
{
  // If a camera node already points to me. Do I already have it?
  if (newCameraNode == this->Internal->CameraNode)
    {
    // I'm already pointing to it
    vtkDebugMacro("UpdateCamera: CameraNode [" << newCameraNode->GetID()
                  << "] is already pointing to my ViewNode and I'm observing it - "
                  << "Internal->CameraNode [" << this->Internal->CameraNode->GetID() << "]");
    return;
    }

  if (newCameraNode)
    {
    newCameraNode->SetActiveTag(this->GetMRMLViewNode()->GetID());
    }
  vtkNew<vtkIntArray> cameraNodeEvents;
  cameraNodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  cameraNodeEvents->InsertNextValue(vtkMRMLCameraNode::ActiveTagModifiedEvent);

  vtkSetAndObserveMRMLNodeEventsMacro(
    this->Internal->CameraNode, newCameraNode, cameraNodeEvents.GetPointer());

  this->SetCameraToRenderer();
  this->SetCameraToInteractor();
  this->InvokeEvent(vtkMRMLCameraDisplayableManager::ActiveCameraChangedEvent, newCameraNode);
};

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::RemoveMRMLObservers()
{
//  this->RemoveCameraObservers();
  this->SetAndObserveCameraNode(0);

  this->Superclass::RemoveMRMLObservers();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::UpdateCameraNode()
{
  if (this->GetMRMLScene() == 0)
    {
    vtkErrorMacro("UpdateCameraNode: DisplayableManager does NOT have a scene set, "
                  "can't find CameraNodes");
    return;
    }

  if (this->GetMRMLViewNode() == 0)
    {
    vtkErrorMacro("UpdateCameraNode: DisplayableManager does NOT have a ViewNode!");
    return;
    }

  const char * defaultCameraName = "Default Scene Camera";
#ifndef NDEBUG
  // How many cameras are in the scene ?
  int numCameraNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLCameraNode");

  // How many view nodes are in the scene ?
  int numViewNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLViewNode");
  vtkDebugMacro("UpdateCamera: Number of CameraNodes = " << numCameraNodes
                << ", Number of ViewNodes = " << numViewNodes);
#endif

  vtkMRMLCameraNode * viewCameraNode = 0;
  vtkMRMLCameraNode * unassignedCamera = 0;
  //vtkMRMLCameraNode * pruneDefaultCamera = 0;

  if (!this->GetMRMLViewNode()->GetName())
    {
    vtkErrorMacro("View has no name");
    return;
    }

  vtkCollection* scene = this->GetMRMLScene()->GetNodes();
  vtkCollectionSimpleIterator it;
  vtkMRMLNode* node = 0;
  for (scene->InitTraversal(it);
       (node = (vtkMRMLNode*)scene->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(node);
    if (!cameraNode)
      {
      continue;
      }
    // does the node point to my view node?
    if (cameraNode->GetActiveTag())
      {
/*
      if (cameraNode->GetName() && !strcmp(cameraNode->GetName(), defaultCameraName))
        {
        vtkDebugMacro("UpdateCamera: Found a default CameraNode pointing to my ViewNode");
        pruneDefaultCamera = cameraNode;
        }
*/
      if (!strcmp(cameraNode->GetActiveTag(), this->GetMRMLViewNode()->GetID()))
        {
        // Is this a default camera node that we created when the view
        // node was created?
        vtkDebugMacro("UpdateCamera: found a camera pointing to me with id = " << (cameraNode->GetID() ? cameraNode->GetID() : "0"));
        viewCameraNode = cameraNode;
        break;
        }
      // else it points to another view node
      }
    else
      {
      // steal it!
      unassignedCamera = cameraNode;
      }
    }
/*
  // Is there a camera node that's not the default that wants to point to my view node
  if (pruneDefaultCamera != 0 &&
      viewCameraNode != 0 &&
      viewCameraNode != pruneDefaultCamera)
    {
    // Unhook the default node
    vtkDebugMacro("UpdateCamera: Pruning default CameraNode, set it's ActiveTag to 0");
    //pruneDefaultCamera->SetActiveTag(0);
    }
*/
  if (viewCameraNode != 0)
    {
    this->SetAndObserveCameraNode(viewCameraNode);
    return;
    }

  // Do I have a camera node?
  if (this->Internal->CameraNode == 0)
    {
    // is there an unasigned camera node?
    if (unassignedCamera != 0)
      {
      // use it!
      vtkDebugMacro("UpdateCamera: Setting ActiveTag on unassigned CameraNode");
      unassignedCamera->SetActiveTag(this->GetMRMLViewNode()->GetID());
      vtkDebugMacro("UpdateCamera: Setting and observing unassigned CameraNode");
      this->SetAndObserveCameraNode(unassignedCamera);
      }
    else
      {
      // create one
      unassignedCamera =  vtkMRMLCameraNode::New();
      vtkDebugMacro("Viewer widget: Created new unassigned CameraNode");
      unassignedCamera->SetName(
        this->GetMRMLScene()->GetUniqueNameByString(defaultCameraName));
      //this->MRMLScene->GetUniqueNameByString(viewCameraNode->GetNodeTagName()));
      unassignedCamera->SetActiveTag(
        this->GetMRMLViewNode() ? this->GetMRMLViewNode()->GetID() : 0);
      this->SetAndObserveCameraNode(unassignedCamera);
      this->GetMRMLScene()->AddNode(unassignedCamera);
      unassignedCamera->Delete();
      }
    }
  else
    {
    // Can get here if a view node steals my camera node
    vtkDebugMacro("UpdateCamera: I don't have a CameraNode, nothing is pointing to my ViewNode");
    // Can I swap for an unassigned one?
    if (unassignedCamera != 0)
      {
      // swap!
      vtkWarningMacro("UpdateCamera: Stealing an unassigned CameraNode ["
                      << unassignedCamera->GetID() << "], vtkCamera:"
                      << unassignedCamera->GetCamera());
      unassignedCamera->SetActiveTag(this->GetMRMLViewNode()->GetID());
      this->SetAndObserveCameraNode(unassignedCamera);
      }
    else
      {
      vtkDebugMacro("UpdateCamera: No unassigned CameraNode in the scene to steal!");
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::AdditionnalInitializeStep()
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
  vtkCamera *camera = this->Internal->CameraNode ? this->Internal->CameraNode->GetCamera() : 0;
  this->GetRenderer()->SetActiveCamera(camera);
  if (camera)
    {
    // Do not call if there is no camera otherwise it will create a new one without a CameraNode
    this->GetRenderer()->ResetCameraClippingRange();
    // Default to ParallelProjection Off
    //camera->ParallelProjectionOff();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::SetCameraToInteractor()
{
  if (!this->GetInteractor())
    {
    return;
    }
  vtkInteractorObserver *iobs = this->GetInteractor()->GetInteractorStyle();
  vtkThreeDViewInteractorStyle *istyle =
    vtkThreeDViewInteractorStyle::SafeDownCast(iobs);
  if (istyle)
    {
    istyle->SetCameraNode(this->Internal->CameraNode);
    //if (istyle->GetApplicationLogic() == 0 &&
    //    this->GetApplicationLogic() != 0)
    //  {
    //  vtkDebugMacro("Updating interactor style's application logic, since it was 0");
    //  istyle->SetApplicationLogic(this->GetApplicationLogic());
    //  }
    }
}

////---------------------------------------------------------------------------
//void vtkMRMLCameraDisplayableManager::AddCameraObservers()
//{
//  //!this->MRMLScene || this->SceneClosing
//  assert(this->Internal->CameraNode);
//
//  vtkEventBroker *broker = vtkEventBroker::GetInstance();
//
//  std::vector< vtkObservation *> observations = broker->GetObservations(
//      this->Internal->CameraNode, vtkMRMLCameraNode::ActiveTagModifiedEvent,
//      this, this->GetMRMLCallbackCommand());
//
//  assert(observations.size() == 0);
//
//  broker->AddObservation(this->Internal->CameraNode, vtkMRMLCameraNode::ActiveTagModifiedEvent,
//                         this, this->GetMRMLCallbackCommand());
//}
//
////---------------------------------------------------------------------------
//void vtkMRMLCameraDisplayableManager::RemoveCameraObservers()
//{
//  assert(this->Internal->CameraNode);
//
//  vtkEventBroker *broker = vtkEventBroker::GetInstance();
//
//  std::vector< vtkObservation *> observations = broker->GetObservations(
//      this->Internal->CameraNode, vtkMRMLCameraNode::ActiveTagModifiedEvent,
//      this, this->GetMRMLCallbackCommand());
//
//  assert(observations.size() == 1);
//
//  broker->RemoveObservations(observations);
//}
