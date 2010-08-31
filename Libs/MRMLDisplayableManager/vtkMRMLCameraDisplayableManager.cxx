/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <iostream>
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
void vtkMRMLCameraDisplayableManager::OnMRMLSceneClosedEvent()
{
  //this->RemoveCameraObservers();
  this->SetAndObserveCameraNode(0);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneImportedEvent()
{
  this->UpdateCameraNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneRestoredEvent()
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
void vtkMRMLCameraDisplayableManager::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  vtkEventBroker::GetInstance()->AddObservation(
      node, vtkMRMLCameraNode::ActiveTagModifiedEvent,
      this, this->GetMRMLCallbackCommand());
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node)
{
  vtkEventBroker::GetInstance()->RemoveObservations(
      node, vtkMRMLCameraNode::ActiveTagModifiedEvent,
      this, this->GetMRMLCallbackCommand());
  this->UpdateCameraNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::ProcessMRMLEvents(vtkObject *caller,
                                                        unsigned long event,
                                                        void *callData)
{
  if (vtkMRMLCameraNode::SafeDownCast(caller))
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
      case vtkMRMLCameraNode::ActiveTagModifiedEvent:
        this->UpdateCameraNode();
        vtkDebugMacro("ProcessingMRML: got a camera node modified event");
        this->RequestRender();
        break;
      }
    }
  else
    {
    this->Superclass::ProcessMRMLEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraDisplayableManager::SetAndObserveCameraNode(vtkMRMLCameraNode * newCameraNode)
{
  vtkSetAndObserveMRMLNodeMacro(this->Internal->CameraNode, newCameraNode);
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

  // How many cameras are in the scene ?
  int numCameraNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLCameraNode");

  // How many view nodes are in the scene ?
  int numViewNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLViewNode");
  vtkDebugMacro("UpdateCamera: Number of CameraNodes = " << numCameraNodes
                << ", Number of ViewNodes = " << numViewNodes);

  vtkMRMLCameraNode * camera_node = 0;
  vtkMRMLCameraNode * unassignedCamera = 0;
  vtkMRMLCameraNode * pruneDefaultCamera = 0;

  if (this->GetMRMLViewNode() && this->GetMRMLViewNode()->GetName())
    {
    std::vector<vtkMRMLNode *> cnodes;
    int nnodes = this->GetMRMLScene()->GetNodesByClass("vtkMRMLCameraNode", cnodes);
    vtkMRMLCameraNode *node = 0;
    for (int n=0; n<nnodes; n++)
      {
      node = vtkMRMLCameraNode::SafeDownCast (cnodes[n]);
      if (node)
        {
        // does the node point to my view node?
        if (node->GetActiveTag())
          {
          if (node->GetName() && !strcmp(node->GetName(), defaultCameraName))
            {
            vtkDebugMacro("UpdateCamera: Found a default CameraNode pointing to my ViewNode");
            pruneDefaultCamera = node;
            }
          if (!strcmp(node->GetActiveTag(), this->GetMRMLViewNode()->GetID()))
            {
            // Is this a default camera node that we created when the view
            // node was created?
            vtkDebugMacro("UpdateCamera: found a camera pointing to me with id = " << (node->GetID() ? node->GetID() : "0"));
            camera_node = node;
            // Take out the break, find the last one pointing to me
            //break;
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

  if (pruneDefaultCamera != 0)
    {
    // Is there a camera node that's not the default that wants to point to my view node
    if (camera_node != 0 &&
        camera_node != pruneDefaultCamera)
      {
      // Unhook the default node
      vtkDebugMacro("UpdateCamera: Pruning default CameraNode, set it's ActiveTag to 0");
      pruneDefaultCamera->SetActiveTag(0);
      }
    }
  // If a camera node already points to me. Do I already have it?
  if (camera_node != 0 &&
      this->Internal->CameraNode != 0 &&
      this->Internal->CameraNode == camera_node)
    {
    // I'm already pointing to it
    vtkDebugMacro("UpdateCamera: CameraNode [" << camera_node->GetID()
                  << "] is already pointing to my ViewNode and I'm observing it - "
                  << "Internal->CameraNode [" << this->Internal->CameraNode->GetID() << "]");
    return;
    }

  // Do I have a camera node?
  if (this->Internal->CameraNode == 0)
    {
    if (camera_node != 0)
      {
      // I'm not observing the camera node that is using my view node's id as
      // it's active tag
      vtkDebugMacro("UpdateCamera: I am not observing any CameraNodes, now observe the "
                    "CameraNode that's pointing at my ViewNode");
      this->SetAndObserveCameraNode(camera_node);
      }
    else
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
        unassignedCamera->SetName(defaultCameraName);
        //this->MRMLScene->GetUniqueNameByString(camera_node->GetNodeTagName()));
        unassignedCamera->SetActiveTag(
            this->GetMRMLViewNode() ? this->GetMRMLViewNode()->GetID() : 0);
        this->GetMRMLScene()->AddNode(unassignedCamera);
        this->SetAndObserveCameraNode(unassignedCamera);
        unassignedCamera->Delete();
        }
      }
    }
  else
    {
    if (camera_node != 0)
      {
      // I'm not observing the camera node that is using my view node's id as it's active tag
      vtkDebugMacro("UpdateCamera: Resetting to observe the CameraNode that's "
                    "pointing at my ViewNode");
      this->SetAndObserveCameraNode(camera_node);
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

  vtkCamera *camera = this->Internal->CameraNode ? this->Internal->CameraNode->GetCamera() : 0;
  assert(this->GetRenderer());
  assert(camera);
  this->GetRenderer()->SetActiveCamera(camera);
  if (camera)
    {
    // Do not call if there is no camera otherwise it will create a new one without a CameraNode
    this->GetRenderer()->ResetCameraClippingRange();

    // Default to ParallelProjection Off
    camera->ParallelProjectionOff();
    }

  if (this->GetInteractor())
    {
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
  this->InvokeEvent(vtkMRMLCameraDisplayableManager::ActiveCameraChangedEvent, camera);
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
