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

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported in part by CI3.

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLThreeDSliceEdgeDisplayableManager.h"
#include "vtkMRMLCameraNode.h"

// MRML includes
#include <vtkMRMLSliceEdgeWidgetRepresentation.h>
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkEventBroker.h>
#include <vtkFeatureEdges.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkStripper.h>
#include <vtkTubeFilter.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLThreeDSliceEdgeDisplayableManager);

//---------------------------------------------------------------------------
class vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal
{
public:
  typedef std::map<std::string, vtkSmartPointer<vtkMRMLSliceEdgeWidgetRepresentation>> SliceEdgeWidgetRepresentationsType; // SliceNodeID -> Widget

  vtkInternal(vtkMRMLThreeDSliceEdgeDisplayableManager* external);
  ~vtkInternal();

  // View
  vtkMRMLViewNode* GetViewNode();

  // SliceNodes
  void AddSliceNode(vtkMRMLSliceNode* sliceNode);
  void RemoveSliceNode(vtkMRMLSliceNode* sliceNode);
  void RemoveAllSliceNodes();
  void UpdateSliceNodes();
  vtkMRMLSliceNode* GetSliceNode(vtkMRMLSliceEdgeWidgetRepresentation* rep);
  void AddObservations(vtkMRMLSliceNode* node);
  void RemoveObservations(vtkMRMLSliceNode* node);

  // Widget
  vtkSmartPointer<vtkMRMLSliceEdgeWidgetRepresentation> NewSliceSliceEdgeWidgetRepresentation();
  vtkMRMLSliceEdgeWidgetRepresentation* GetSliceEdgeWidgetRepresenatation(vtkMRMLSliceNode*);
  // return with true if rendering is required
  void UpdateAllSliceEdgePipelines();
  // return with true if rendering is required
  void UpdateSliceEdgeWidgetRepresentation(vtkMRMLSliceNode*, vtkMRMLSliceEdgeWidgetRepresentation*);
  void UpdateSliceEdgeWidgetRepresentationVisibility(vtkMRMLSliceEdgeWidgetRepresentation* rep, vtkMRMLSliceNode* sliceNode);
  void UpdateSliceEdgeWidgetRepresentationMesh(vtkMRMLSliceEdgeWidgetRepresentation* rep, vtkMRMLSliceNode* sliceNode);

  SliceEdgeWidgetRepresentationsType SliceEdgeWidgetRepresentations;
  vtkMRMLThreeDSliceEdgeDisplayableManager* External;
  double ViewScaleFactorMmPerPixel;
  double ScreenSizePixel;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::vtkInternal(
  vtkMRMLThreeDSliceEdgeDisplayableManager* _external)
{
  this->External = _external;
}

//---------------------------------------------------------------------------
vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::~vtkInternal()
{
  this->RemoveAllSliceNodes();
}

//---------------------------------------------------------------------------
vtkMRMLViewNode* vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::GetViewNode()
{
  return this->External->GetMRMLViewNode();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::AddObservations(vtkMRMLSliceNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  if (!broker->GetObservationExist(node, vtkCommand::ModifiedEvent,
    this->External, this->External->GetMRMLNodesCallbackCommand()))
  {
    broker->AddObservation(node, vtkCommand::ModifiedEvent,
      this->External, this->External->GetMRMLNodesCallbackCommand());
  }
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::RemoveObservations(vtkMRMLSliceNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(
    node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::AddSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode)
  {
    return;
  }

  std::string sliceNodeID = sliceNode->GetID();
  auto widgetRepIt = this->SliceEdgeWidgetRepresentations.find(sliceNodeID);
  if (widgetRepIt == this->SliceEdgeWidgetRepresentations.end())
  {
    // We associate the node with the widget if an instantiation is called.
    // We add the sliceNode without instantiating the widget first.
    this->AddObservations(sliceNode);
    this->SliceEdgeWidgetRepresentations[sliceNodeID] = nullptr;
  }

  this->UpdateSliceEdgeWidgetRepresentation(sliceNode, nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::RemoveSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode)
  {
    return;
  }
  std::string sliceNodeID = sliceNode->GetID();
  this->RemoveObservations(sliceNode);
  // The manager has the responsibility to delete the rep.
  auto widgetRepIt = this->SliceEdgeWidgetRepresentations.find(sliceNodeID);
  if (widgetRepIt != this->SliceEdgeWidgetRepresentations.end())
  {
    vtkMRMLSliceEdgeWidgetRepresentation* rep = widgetRepIt->second;
    vtkRenderer* renderer = this->External->GetRenderer();
    if (renderer && rep && renderer->HasViewProp(rep))
    {
      renderer->RemoveViewProp(rep);
      this->External->RequestRender();
    }
    this->SliceEdgeWidgetRepresentations.erase(widgetRepIt);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::RemoveAllSliceNodes()
{
  this->RemoveObservations(nullptr);
  for (auto sliceNodeIDToWidgetRep : this->SliceEdgeWidgetRepresentations)
  {
    vtkMRMLSliceEdgeWidgetRepresentation* rep = sliceNodeIDToWidgetRep.second;
    vtkRenderer* renderer = this->External->GetRenderer();
    if (renderer && rep && renderer->HasViewProp(rep))
    {
      renderer->RemoveViewProp(rep);
      this->External->RequestRender();
    }
  }
  this->SliceEdgeWidgetRepresentations.clear();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::UpdateSliceNodes()
{
  if (this->External->GetMRMLScene() == nullptr)
  {
    this->RemoveAllSliceNodes();
    return;
  }

  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->External->GetMRMLScene()->GetNodes();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
  {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
    if (sliceNode)
    {
      this->AddSliceNode(sliceNode);
    }
  }
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::GetSliceNode(vtkMRMLSliceEdgeWidgetRepresentation* rep)
{
  if (!rep || !this->External->GetMRMLScene())
  {
    return nullptr;
  }

  // Get the slice node
  for (auto sliceNodeIDToWidgetRep : this->SliceEdgeWidgetRepresentations)
  {
    if (sliceNodeIDToWidgetRep.second.Get() == rep)
    {
      return vtkMRMLSliceNode::SafeDownCast(this->External->GetMRMLScene()->GetNodeByID(sliceNodeIDToWidgetRep.first));
    }
  }

  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::UpdateSliceEdgeWidgetRepresentationVisibility(
  vtkMRMLSliceEdgeWidgetRepresentation* rep, vtkMRMLSliceNode* sliceNode)
{
  if (!rep || !sliceNode)
  {
    return;
  }
  vtkRenderer* renderer = this->External->GetRenderer();
  if (!renderer)
  {
    return;
  }

  bool show =
    sliceNode->IsDisplayableInThreeDView(this->External->GetMRMLViewNode()->GetID()) //
    && sliceNode->GetSliceVisible() && sliceNode->GetSliceEdgeVisibility3D();

  bool isShown = renderer->HasViewProp(rep);
  if (show == isShown)
  {
    // no change
    return;
  }

  if (show)
  {
    renderer->AddViewProp(rep);
  }
  else
  {
    renderer->RemoveViewProp(rep);
  }

  this->External->RequestRender();
}

//---------------------------------------------------------------------------
vtkSmartPointer<vtkMRMLSliceEdgeWidgetRepresentation> vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::NewSliceSliceEdgeWidgetRepresentation()
{
  vtkSmartPointer<vtkMRMLSliceEdgeWidgetRepresentation> newRep = vtkSmartPointer<vtkMRMLSliceEdgeWidgetRepresentation>::New();
  newRep->SetRenderer(this->External->GetRenderer());
  newRep->SetViewNode(this->GetViewNode());
  return newRep;
}

//---------------------------------------------------------------------------
vtkMRMLSliceEdgeWidgetRepresentation* vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::GetSliceEdgeWidgetRepresenatation(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode)
  {
    return nullptr;
  }
  std::string nodeId = sliceNode->GetID();
  auto it = this->SliceEdgeWidgetRepresentations.find(nodeId);
  return (it != this->SliceEdgeWidgetRepresentations.end()) ? it->second.Get() : nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::UpdateSliceEdgeWidgetRepresentationMesh(
  vtkMRMLSliceEdgeWidgetRepresentation* rep, vtkMRMLSliceNode* sliceNode)
{
  if (!rep || !sliceNode)
  {
    return;
  }
  vtkMRMLApplicationLogic* mrmlAppLogic = this->External->GetMRMLApplicationLogic();
  vtkMRMLSliceLogic* sliceLogic = (mrmlAppLogic ? mrmlAppLogic->GetSliceLogic(sliceNode) : nullptr);
  vtkMRMLModelNode* sliceModelNode = (sliceLogic ? sliceLogic->GetSliceModelNode() : nullptr);
  if (!sliceModelNode)
  {
    return;
  }

  rep->UpdateFromMRML(nullptr, 0);
  rep->setSliceModelNode(sliceModelNode);
  rep->setSliceNode(sliceNode);
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::UpdateAllSliceEdgePipelines()
{
  vtkMRMLScene* scene = this->External->GetMRMLScene();
  if (!scene)
  {
    return;
  }
  for (auto sliceNodeIDToWidgetRep : this->SliceEdgeWidgetRepresentations)
  {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(scene->GetNodeByID(sliceNodeIDToWidgetRep.first));
    this->UpdateSliceEdgeWidgetRepresentation(sliceNode, sliceNodeIDToWidgetRep.second);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::vtkInternal::UpdateSliceEdgeWidgetRepresentation(
  vtkMRMLSliceNode* sliceNode, vtkMRMLSliceEdgeWidgetRepresentation* rep)
{
  if (!sliceNode || (!rep && !sliceNode->GetSliceVisible()))
  {
    return;
  }

  if (!rep)
  {
    // Instantiate widget and link it if
    // there is no one associated to the sliceNode yet
    vtkSmartPointer<vtkMRMLSliceEdgeWidgetRepresentation> newRep = this->NewSliceSliceEdgeWidgetRepresentation();
    rep = newRep;
    this->SliceEdgeWidgetRepresentations.find(sliceNode->GetID())->second = rep;
  }

  // Update rep
  this->UpdateSliceEdgeWidgetRepresentationMesh(rep, sliceNode);
  this->UpdateSliceEdgeWidgetRepresentationVisibility(rep, sliceNode);
}

//---------------------------------------------------------------------------
// vtkMRMLSliceModelDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLThreeDSliceEdgeDisplayableManager::vtkMRMLThreeDSliceEdgeDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLThreeDSliceEdgeDisplayableManager::~vtkMRMLThreeDSliceEdgeDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->RemoveAllSliceNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::UpdateFromMRMLScene()
{
  this->Internal->UpdateSliceNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* nodeAdded)
{
  if (this->GetMRMLScene()->IsBatchProcessing() || //
      !nodeAdded->IsA("vtkMRMLSliceNode"))
  {
    return;
  }

  this->Internal->AddSliceNode(vtkMRMLSliceNode::SafeDownCast(nodeAdded));
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* nodeRemoved)
{
  if (!nodeRemoved->IsA("vtkMRMLSliceNode"))
  {
    return;
  }

  this->Internal->RemoveSliceNode(vtkMRMLSliceNode::SafeDownCast(nodeRemoved));
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::OnMRMLNodeModified(vtkMRMLNode* node)
{
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
  {
    vtkMRMLSliceEdgeWidgetRepresentation* rep = this->Internal->GetSliceEdgeWidgetRepresenatation(sliceNode);
    this->Internal->UpdateSliceEdgeWidgetRepresentation(sliceNode, rep);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::OnMRMLViewNodeModifiedEvent()
{
  this->Internal->UpdateAllSliceEdgePipelines();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDSliceEdgeDisplayableManager::Create()
{
  this->Internal->UpdateSliceNodes();
}
