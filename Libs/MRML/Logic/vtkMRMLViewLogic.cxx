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
#include "vtkMRMLViewLogic.h"
#include "vtkMRMLSliceLayerLogic.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkVersion.h>

// VTKAddon includes
#include <vtkAddonMathUtilities.h>

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLViewLogic);

//----------------------------------------------------------------------------
vtkMRMLViewLogic::vtkMRMLViewLogic()
{
  this->Name = nullptr;
  this->SetName("");
  this->ViewNode = nullptr;
  this->CameraNode = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLViewLogic::~vtkMRMLViewLogic()
{
  if (this->Name)
    {
    delete [] this->Name;
    }
  this->Name = nullptr;

  if (this->CameraNode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->CameraNode, 0);
    this->CameraNode = nullptr;
    }

  if (this->ViewNode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->ViewNode, 0);
    this->ViewNode = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  // Sanity checks
  if (!this->GetName() || strlen(this->GetName()) == 0)
    {
    vtkErrorMacro(<< "Name is nullptr - Make sure you call SetName before SetMRMLScene !");
    return;
    }

  // List of events the slice logics should listen
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());

  this->ProcessMRMLSceneEvents(newScene, vtkMRMLScene::EndBatchProcessEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (node->IsA("vtkMRMLViewNode") || node->IsA("vtkMRMLCameraNode"))
    {
    this->UpdateMRMLNodes();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (node->IsA("vtkMRMLViewNode") || node->IsA("vtkMRMLCameraNode"))
    {
    this->UpdateMRMLNodes();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::UpdateFromMRMLScene()
{
  this->UpdateMRMLNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::UpdateMRMLNodes()
{
  if (this->GetMRMLScene()
      && this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }

  // Set up the nodes
  this->UpdateViewNode();
  this->UpdateCameraNode();
}

//----------------------------------------------------------------------------
vtkMRMLViewNode* vtkMRMLViewLogic::GetViewNode(vtkMRMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return nullptr;
    }

  vtkSmartPointer<vtkCollection> viewNodes = vtkSmartPointer<vtkCollection>::Take
      (scene->GetNodesByClass("vtkMRMLViewNode"));
  for(int viewNodeIndex = 0; viewNodeIndex < viewNodes->GetNumberOfItems(); ++viewNodeIndex)
    {
    vtkMRMLViewNode* viewNode =
        vtkMRMLViewNode::SafeDownCast(viewNodes->GetItemAsObject(viewNodeIndex));
    if (viewNode &&
        viewNode->GetLayoutName() &&
        !strcmp(viewNode->GetLayoutName(), layoutName))
      {
      return viewNode;
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLCameraNode* vtkMRMLViewLogic::GetCameraNode(vtkMRMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return nullptr;
    }

  vtkSmartPointer<vtkCollection> cameraNodes = vtkSmartPointer<vtkCollection>::Take
      (scene->GetNodesByClass("vtkMRMLCameraNode"));
  for (int cameraIndex = 0; cameraIndex < cameraNodes->GetNumberOfItems(); ++cameraIndex)
    {
    vtkMRMLCameraNode* cameraNode =
        vtkMRMLCameraNode::SafeDownCast(cameraNodes->GetItemAsObject(cameraIndex));
    if (!cameraNode || !cameraNode->GetActiveTag())
      {
      continue;
      }
    vtkSmartPointer<vtkCollection> viewNodes = vtkSmartPointer<vtkCollection>::Take
        (scene->GetNodesByClass("vtkMRMLViewNode"));
    for(int viewNodeIndex = 0; viewNodeIndex < viewNodes->GetNumberOfItems(); ++viewNodeIndex)
      {
      vtkMRMLViewNode* viewNode =
          vtkMRMLViewNode::SafeDownCast(viewNodes->GetItemAsObject(viewNodeIndex));
      if (viewNode &&  viewNode->GetLayoutName() &&
          !strcmp(viewNode->GetLayoutName(), layoutName) &&
          !strcmp(cameraNode->GetActiveTag(), viewNode->GetID()))
        {
        return cameraNode;
        }
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::SetCameraNode(vtkMRMLCameraNode* newCameraNode)
{
  if (this->CameraNode == newCameraNode)
    {
    return;
    }

  // Observe the camera node for general properties.
  vtkSetAndObserveMRMLNodeMacro(this->CameraNode, newCameraNode);

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::SetViewNode(vtkMRMLViewNode* newViewNode)
{
  if (this->ViewNode == newViewNode)
    {
    return;
    }

  // Observe the view node for general properties.
  vtkSetAndObserveMRMLNodeMacro(this->ViewNode, newViewNode);
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();

  os << indent << "SlicerViewLogic:             " << this->GetClassName() << "\n";

  if (this->CameraNode)
    {
    os << indent << "CameraNode: ";
    os << (this->CameraNode->GetID() ? this->CameraNode->GetID() : "(0 ID)") << "\n";
    this->CameraNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "CameraNode: (none)\n";
    }

  if (this->ViewNode)
    {
    os << indent << "ViewNode: ";
    os << (this->ViewNode->GetID() ? this->ViewNode->GetID() : "(0 ID)") << "\n";
    this->ViewNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "ViewNode: (none)\n";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::StartViewNodeInteraction(unsigned int parameters)
{
  if (!this->ViewNode)
    {
    return;
    }

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on LinkedControl
  this->ViewNode->SetInteractionFlags(parameters);

  // If we have linked controls, then we want to broadcast changes
  if (this->ViewNode->GetLinkedControl())
    {
    // Activate interaction
    this->ViewNode->InteractingOn();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::EndViewNodeInteraction()
{
  if (!this->ViewNode)
    {
    return;
    }

  if (this->ViewNode->GetLinkedControl())
    {
    this->ViewNode->InteractingOff();
    this->ViewNode->SetInteractionFlags(0);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::UpdateCameraNode()
{
  if (!this->GetMRMLScene())
    {
    this->SetCameraNode(nullptr);
    return;
    }

  // find ViewNode in the scene
  vtkMRMLCameraNode* node = vtkMRMLViewLogic::GetCameraNode(this->GetMRMLScene(), this->GetName());

  if (this->CameraNode != nullptr && node != nullptr &&
      (this->CameraNode->GetID() == nullptr ||
      strcmp(this->CameraNode->GetID(), node->GetID()) != 0))
    {
    // local ViewNode is out of sync with the scene
    this->SetCameraNode(nullptr);
    }

  // find ViewNode in the scene
  vtkMRMLViewNode* viewNode = vtkMRMLViewLogic::GetViewNode(this->GetMRMLScene(), this->GetName());

  if (this->CameraNode == nullptr)
    {
    if (node == nullptr && viewNode)
      {
      // Use CreateNodeByClass instead of New to use default node specified in the scene
      node = vtkMRMLCameraNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLCameraNode"));
      node ->SetActiveTag(viewNode->GetID());
      this->SetCameraNode(node);
      node->Delete();
      }
    else
      {
      this->SetCameraNode(node);
      }
    }

  if (this->GetMRMLScene()->GetNodeByID(this->CameraNode->GetID()) == nullptr)
    {
    // local node not in the scene
    node = this->CameraNode;
    node->Register(this);
    this->SetCameraNode(nullptr);
    this->GetMRMLScene()->AddNode(node);
    this->SetCameraNode(node);
    node->UnRegister(this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::UpdateViewNode()
{
  if (!this->GetMRMLScene())
    {
    this->SetViewNode(nullptr);
    return;
    }

  // find ViewNode in the scene
  vtkMRMLViewNode* node = vtkMRMLViewLogic::GetViewNode(this->GetMRMLScene(), this->GetName());

  if (this->ViewNode != nullptr && node != nullptr &&
      (this->ViewNode->GetID() == nullptr ||
      strcmp(this->ViewNode->GetID(), node->GetID()) != 0))
    {
    // local ViewNode is out of sync with the scene
    this->SetViewNode(nullptr);
    }

  if (this->ViewNode == nullptr)
    {
    if (node == nullptr)
      {
      // Use CreateNodeByClass instead of New to use default node specified in the scene
      node = vtkMRMLViewNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLViewNode"));
      node->SetLayoutName(this->GetName());
      this->SetViewNode(node);
      node->Delete();
      }
    else
      {
      this->SetViewNode(node);
      }
    }

  if (this->GetMRMLScene()->GetNodeByID(this->ViewNode->GetID()) == nullptr)
    {
    // local node not in the scene
    node = this->ViewNode;
    node->Register(this);
    this->SetViewNode(nullptr);
    this->GetMRMLScene()->AddNode(node);
    this->SetViewNode(node);
    node->UnRegister(this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::StartCameraNodeInteraction(unsigned int parameters)
{
  if (!this->ViewNode || !this->CameraNode)
    {
    return;
    }

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on LinkedControl
  this->CameraNode->SetInteractionFlags(parameters);

  // If we have hot linked controls, then we want to broadcast changes
  if (this->ViewNode->GetLinkedControl())
    {
    // Activate interaction
    this->CameraNode->InteractingOn();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLViewLogic::EndCameraNodeInteraction()
{
  if (!this->ViewNode || !this->CameraNode)
    {
    return;
    }

  if (this->ViewNode->GetLinkedControl())
    {
    this->CameraNode->InteractingOff();
    this->CameraNode->SetInteractionFlags(0);
    }
}
