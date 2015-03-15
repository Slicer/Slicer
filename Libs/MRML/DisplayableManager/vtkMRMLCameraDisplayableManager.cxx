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
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLCameraDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLCameraDisplayableManager::vtkInternal
{
public:
  vtkInternal();

  vtkMRMLCameraNode* CameraNode;
  int UpdatingCameraNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLCameraDisplayableManager::vtkInternal::vtkInternal()
{
  this->CameraNode = 0;
  this->UpdatingCameraNode = 0;

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
void vtkMRMLCameraDisplayableManager::OnMRMLSceneStartImport()
{
  if (this->Internal->CameraNode != NULL)
  {
    this->Internal->CameraNode->SetActiveTag(NULL);
  }
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
  vtkMRMLNode *nodeInScene = NULL;
  if (camera_node)
    {
    // camera node is defined but can it be found in the current scene?
    nodeInScene = this->GetMRMLScene()->GetNodeByID(camera_node->GetID());
    if (!nodeInScene)
      {
      // reset the internal camera node
      camera_node = NULL;
      this->SetAndObserveCameraNode(0);
      }
    }
  if (!camera_node)
    {
    camera_node = vtkMRMLCameraNode::SafeDownCast(
        this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLCameraNode"));
    if (camera_node)
      {
      camera_node->SetActiveTag(this->GetMRMLViewNode()->GetID());
      }
    // if the internal one needs updating
    if (!nodeInScene)
      {
      this->UpdateCameraNode();
      }
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
void vtkMRMLCameraDisplayableManager::OnMRMLNodeModified(
#ifndef NDEBUG
 vtkMRMLNode* node)
#else
  vtkMRMLNode* vtkNotUsed(node))
#endif
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
vtkMRMLCameraNode* vtkMRMLCameraDisplayableManager::GetCameraNode()
{
  return this->Internal->CameraNode;
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::SetAndObserveCameraNode(vtkMRMLCameraNode * newCameraNode)
{
  // If a camera node already points to me. Do I already have it?
  if (newCameraNode == this->Internal->CameraNode)
    {
    if (newCameraNode)
      {
      // I'm already pointing to it
      vtkDebugMacro("UpdateCamera: CameraNode [" << newCameraNode->GetID()
                    << "] is already pointing to my ViewNode and I'm observing it - "
                    << "Internal->CameraNode [" << this->Internal->CameraNode->GetID() << "]");
      }
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
  if (this->Internal->UpdatingCameraNode)
    {
    return;
    }
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

  this->Internal->UpdatingCameraNode = 1;

  const char * defaultCameraName = "Default Scene Camera";
  // how many camera and view nodes are in the scene?
  vtkDebugMacro("UpdateCamera: num camera nodes = :" << this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLCameraNode") << ", num view nodes = " << this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLViewNode"));

  vtkMRMLCameraNode *camera_node = NULL;
  vtkMRMLCameraNode *unassignedCamera = NULL;
  vtkMRMLCameraNode  *pruneDefaultCamera = NULL;
  int foundDefaultCamera = 0;

  vtkMRMLViewNode *viewNode = this->GetMRMLViewNode();

  if (viewNode && viewNode->GetName())
    {
    std::vector<vtkMRMLNode *> cnodes;
    int nnodes = this->GetMRMLScene()->GetNodesByClass("vtkMRMLCameraNode", cnodes);
    vtkMRMLCameraNode *node = NULL;
    for (int n=0; n<nnodes; n++)
      {
      node = vtkMRMLCameraNode::SafeDownCast (cnodes[n]);
      if (node)
        {
        // does the node point to my view node?
        if (node->GetActiveTag())
          {
          if ( !strcmp(node->GetActiveTag(), viewNode->GetID()) &&
               node->GetName() &&
               !strcmp(node->GetName(), defaultCameraName) &&
               !foundDefaultCamera)
            {
            // is this a default camera node that we created when the view
            // node was created?
            vtkDebugMacro("UpdateCamera: found a default camera node pointing to my view node");
            pruneDefaultCamera = node;
            // If there is a new camera with a non-null ActiveTag,
            // We want to be sure to null-ify the Active Tag of the
            // camera created upon view node creation (given the
            // default name = defaultCameraName.
            //
            // Logic here will deactivate viewNode's original camera
            // (the one with defaultCameraName) marked with ActiveTag
            // ONLY IF another camera node is found marked with a non-null ActiveTag
            // matching viewNode.
            //
            // BUT: a new scene may be loaded which has another camera with the
            // name = defaultCameraName. If this is parameterized with a non-null
            // ActiveTag, then we want to be sure NOT to deactivate it instead of
            // the original camera by mistake.
            //
            // Add this flag once we find the first camera. Hacky: oh yes, but...
            foundDefaultCamera = 1;
            }
          if (!strcmp(node->GetActiveTag(), viewNode->GetID()))
            {
            // We found a node with a non-null ActiveTag. Might be original
            // camera with defaultCameraName, or might be a new camera
            // also with defaultCameraName, or might be a new camera
            // with some unique name.
            vtkDebugMacro("UpdateCamera: found a camera pointing to me with id = " << (node->GetID() ? node->GetID() : "NULL"));
            camera_node = node;
            // Whichever camera is the last found pointing at the view node
            // will be the selected "ActiveCamera"
            }
          else
            {
            // it points to another view node
            }
          }
        else
          {
          // steal it!
          unassignedCamera = node;
          }
        }
      }
    }

  if (pruneDefaultCamera != NULL)
    {
    // is there a camera node that's not the default that wants to point to my
    // view node
    if (camera_node != NULL &&
        camera_node != pruneDefaultCamera)
      {
      // unhook the default node
      vtkDebugMacro("UpdateCamera: pruning default camera node, set it's active tag to null");
      pruneDefaultCamera->SetActiveTag(NULL);
      }
    }
  // if a camera node already points to me
  // do I already have it?
  if (camera_node != NULL &&
      this->Internal->CameraNode != NULL &&
      this->Internal->CameraNode == camera_node)
    {
    // I'm already pointing to it
    vtkDebugMacro("UpdateCamera: camera node " << camera_node->GetID() << " is already pointing to my view node and I'm observing it, CameraNode = " << this->Internal->CameraNode->GetID());
    this->Internal->UpdatingCameraNode = 0;
    return;
    }

  // do I have a camera node?
  if (this->Internal->CameraNode == NULL)
    {
    if (camera_node != NULL)
      {
      // I'm not observing the camera node that is using my view node's id as
      // it's active tag
      vtkDebugMacro("UpdateCamera: am not observing any camera nodes, now observe the camera node that's pointing at my view node");
      this->SetAndObserveCameraNode(camera_node);
      }
    else
      {
      // is there an unasigned camera node?
      if (unassignedCamera != NULL)
        {
        // use it!
        vtkDebugMacro("UpdateCamera: setting active tag on unassinged camera");
        unassignedCamera->SetActiveTag(viewNode->GetID());
        vtkDebugMacro("UpdateCamera: setting and observing unassigned camera");
        this->SetAndObserveCameraNode(unassignedCamera);
        }
      else
        {
        // create one
        unassignedCamera =  vtkMRMLCameraNode::New();
        vtkDebugMacro("Viewer widget: Created new unassigned camera");
        unassignedCamera->SetName(defaultCameraName);
        //this->GetMRMLScene()->GetUniqueNameByString(camera_node->GetNodeTagName()));
        unassignedCamera->SetActiveTag(
                                       viewNode ? viewNode->GetID() : NULL);
        this->GetMRMLScene()->AddNode(unassignedCamera);
        this->SetAndObserveCameraNode(unassignedCamera);
        unassignedCamera->Delete();
        }
      }
    }
  else
    {
    if (camera_node != NULL)
      {
      // I'm not observing the camera node that is using my view node's id as
      // it's active tag
      vtkDebugMacro("Resetting to observe the camera node that's pointing at my view node");
      this->SetAndObserveCameraNode(camera_node);
      }
    else
      {
      // can get here if a view node steals my camera node
      vtkDebugMacro("I have a camera node, but nothing is pointing to my view node");
      // can I swap for an unassigned one?
      if (unassignedCamera != NULL)
        {
        // swap!
        //vtkDebugMacro("Stealing an unasigned camera node " << unassignedCamera->GetID() << " for view node " << viewNode->GetID());
        vtkDebugMacro("why am I here?  I have a camera node already!" << unassignedCamera->GetID() << " for view node " << viewNode->GetID());
        //unassignedCamera->SetActiveTag(viewNode->GetID());
        //this->SetAndObserveCameraNode(unassignedCamera);
        }
      else
        {
        // create one
        unassignedCamera =  vtkMRMLCameraNode::New();
        vtkDebugMacro("Viewer widget: Created new unassigned camera");
        unassignedCamera->SetName(defaultCameraName);
        //this->GetMRMLScene()->GetUniqueNameByString(camera_node->GetNodeTagName()));
        unassignedCamera->SetActiveTag(
                                       viewNode ? viewNode->GetID() : NULL);
        this->GetMRMLScene()->AddNode(unassignedCamera);
        this->SetAndObserveCameraNode(unassignedCamera);
        unassignedCamera->Delete();
        }
      }
    }
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
//  //!this->GetMRMLScene || this->SceneClosing
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
