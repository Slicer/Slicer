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

  This file was originally developed by Michael Jeulin-L, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLThreeDReformatDisplayableManager.h"

// MRML includes
#include "vtkMRMLApplicationLogic.h"
#include <vtkMRMLColors.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkColor.h>
#include <vtkImageData.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTransform.h>

// STD includes
#include <algorithm>
#include <cmath>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLThreeDReformatDisplayableManager);

//---------------------------------------------------------------------------
class vtkMRMLThreeDReformatDisplayableManager::vtkInternal
{
public:
  typedef std::map<vtkMRMLSliceNode*, vtkImplicitPlaneWidget2*> SliceNodesLink;

  vtkInternal(vtkMRMLThreeDReformatDisplayableManager* external);
  ~vtkInternal();

  // View
  vtkMRMLViewNode* GetViewNode();

  // SliceNodes
  void AddSliceNode(vtkMRMLSliceNode*);
  void RemoveSliceNode(vtkMRMLSliceNode*);
  void RemoveSliceNode(SliceNodesLink::iterator);
  void RemoveAllSliceNodes();
  void UpdateSliceNodes();
  vtkMRMLSliceNode* GetSliceNode(vtkImplicitPlaneWidget2*);

  // Widget
  vtkImplicitPlaneWidget2* NewImplicitPlaneWidget();
  vtkImplicitPlaneWidget2* GetWidget(vtkMRMLSliceNode*);
  // return with true if rendering is required
  bool UpdateWidget(vtkMRMLSliceNode*, vtkImplicitPlaneWidget2*);

  SliceNodesLink                                SliceNodes;
  vtkMRMLThreeDReformatDisplayableManager*      External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLThreeDReformatDisplayableManager::vtkInternal::vtkInternal(
    vtkMRMLThreeDReformatDisplayableManager* _external)
{
  this->External = _external;

#if VTK_MAJOR_VERSION <= 7
  vtkWarningWithObjectMacro(_external, "Widget outline mode not available");
#endif
}

//---------------------------------------------------------------------------
vtkMRMLThreeDReformatDisplayableManager::vtkInternal::~vtkInternal()
{
  this->RemoveAllSliceNodes();
}

//---------------------------------------------------------------------------
vtkMRMLViewNode* vtkMRMLThreeDReformatDisplayableManager::vtkInternal
::GetViewNode()
{
  return this->External->GetMRMLViewNode();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::vtkInternal
::AddSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode ||
     this->SliceNodes.find(vtkMRMLSliceNode::SafeDownCast(sliceNode)) !=
     this->SliceNodes.end())
    {
    return;
    }

  // We associate the node with the widget if an instantiation is called.
  // We add the sliceNode without instantiating the widget first.
  sliceNode->AddObserver(vtkCommand::ModifiedEvent,
                           this->External->GetMRMLNodesCallbackCommand());
  this->SliceNodes.insert(
    std::pair<vtkMRMLSliceNode*, vtkImplicitPlaneWidget2*>(sliceNode, static_cast<vtkImplicitPlaneWidget2*>(nullptr)));
  this->UpdateWidget(sliceNode, nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::vtkInternal
::RemoveSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode)
    {
    return;
    }

   this->RemoveSliceNode(
     this->SliceNodes.find(vtkMRMLSliceNode::SafeDownCast(sliceNode)));
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::vtkInternal
::RemoveSliceNode(SliceNodesLink::iterator it)
{
  if (it == this->SliceNodes.end())
    {
    return;
    }

  // The manager has the responsabilty to delete the widget.
  if (it->second)
    {
    it->second->Delete();
    }

  // TODO: it->first might have already been deleted
  it->first->RemoveObserver(this->External->GetMRMLNodesCallbackCommand());
  this->SliceNodes.erase(it);
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::vtkInternal
::RemoveAllSliceNodes()
{
  // The manager has the responsabilty to delete the widgets.
  while (this->SliceNodes.size() > 0)
    {
    this->RemoveSliceNode(this->SliceNodes.begin());
    }
  this->SliceNodes.clear();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::vtkInternal::UpdateSliceNodes()
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
vtkMRMLSliceNode* vtkMRMLThreeDReformatDisplayableManager::vtkInternal::
GetSliceNode(vtkImplicitPlaneWidget2* planeWidget)
{
  if (!planeWidget)
    {
    return nullptr;
    }

  // Get the slice node
  vtkMRMLSliceNode* sliceNode = nullptr;
  for (SliceNodesLink::iterator it=this->SliceNodes.begin();
       it!=this->SliceNodes.end(); ++it)
    {
    if (it->second == planeWidget)
      {
      sliceNode = it->first;
      break;
      }
    }

  return sliceNode;
}

//---------------------------------------------------------------------------
vtkImplicitPlaneWidget2* vtkMRMLThreeDReformatDisplayableManager::vtkInternal::
NewImplicitPlaneWidget()
{
  // Instantiate implcite plane widget and his representation
  vtkNew<vtkImplicitPlaneRepresentation> rep;
  double defaultBounds[6] = {-100, 100, -100, 100, -100, 100};
  rep->PlaceWidget(defaultBounds);
  rep->SetOutlineTranslation(0);
  rep->SetScaleEnabled(0);
  rep->SetDrawPlane(0);

  // The Manager has to manage the destruction of the widgets
  vtkImplicitPlaneWidget2* planeWidget = vtkImplicitPlaneWidget2::New();
  planeWidget->SetInteractor(this->External->GetInteractor());
  planeWidget->SetRepresentation(rep.GetPointer());
  // TODO: enabling picking might make behavior more predicatable
  // when  multiple plane widgets are enabled
  // planeWidget->SetPickingManaged(true);
  planeWidget->SetEnabled(0);

  // Link widget evenement to the WidgetsCallbackCommand
  planeWidget->AddObserver(vtkCommand::StartInteractionEvent,
                           this->External->GetWidgetsCallbackCommand());
  planeWidget->AddObserver(vtkCommand::InteractionEvent,
                           this->External->GetWidgetsCallbackCommand());
  planeWidget->AddObserver(vtkCommand::EndInteractionEvent,
                           this->External->GetWidgetsCallbackCommand());
  planeWidget->AddObserver(vtkCommand::UpdateEvent,
                           this->External->GetWidgetsCallbackCommand());

  return planeWidget;
}

//---------------------------------------------------------------------------
vtkImplicitPlaneWidget2* vtkMRMLThreeDReformatDisplayableManager::vtkInternal
::GetWidget(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode)
    {
    return nullptr;
    }

  SliceNodesLink::iterator it = this->SliceNodes.find(sliceNode);
  return (it != this->SliceNodes.end()) ? it->second : 0;
}

//---------------------------------------------------------------------------
bool vtkMRMLThreeDReformatDisplayableManager::vtkInternal
::UpdateWidget(vtkMRMLSliceNode* sliceNode,
               vtkImplicitPlaneWidget2* planeWidget)
{
  if (!sliceNode || (!planeWidget && !sliceNode->GetWidgetVisible()))
    {
    return false;
    }

  if (!planeWidget)
    {
    // Instantiate widget and link it if
    // there is no one associated to the sliceNode yet
    planeWidget = this->NewImplicitPlaneWidget();
    this->SliceNodes.find(sliceNode)->second  = planeWidget;
    }

  // Update the representation
  vtkImplicitPlaneRepresentation* rep =
    planeWidget->GetImplicitPlaneRepresentation();
  vtkMatrix4x4* sliceToRAS = sliceNode->GetSliceToRAS();

  // Color the Edge of the plane representation depending on the Slice
  rep->SetEdgeColor(sliceNode->GetLayoutColor());

  // Update Bound size
  vtkMRMLSliceCompositeNode* sliceCompositeNode =
    vtkMRMLSliceLogic::GetSliceCompositeNode(sliceNode);
  const char* volumeNodeID = nullptr;
  if (!volumeNodeID)
    {
    volumeNodeID = sliceCompositeNode ? sliceCompositeNode->GetBackgroundVolumeID() : nullptr;
    }
  if (!volumeNodeID)
    {
    volumeNodeID = sliceCompositeNode ? sliceCompositeNode->GetForegroundVolumeID() : nullptr;
    }
  if (!volumeNodeID)
    {
    volumeNodeID = sliceCompositeNode ? sliceCompositeNode->GetLabelVolumeID() : nullptr;
    }
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(
    this->External->GetMRMLScene()->GetNodeByID(volumeNodeID));
  if (volumeNode)
    {
    double dimensions[3], center[3];
    vtkMRMLSliceLogic::GetVolumeRASBox(volumeNode, dimensions, center);
    double bounds[6] = {bounds[0] = center[0] - dimensions[0] / 2,
                        bounds[1] = center[0] + dimensions[0] / 2,
                        bounds[2] = center[1] - dimensions[1] / 2,
                        bounds[3] = center[1] + dimensions[1] / 2,
                        bounds[4] = center[2] - dimensions[2] / 2,
                        bounds[5] = center[2] + dimensions[2] / 2};
    rep->SetPlaceFactor(1.);
    rep->PlaceWidget(bounds);
    }

  // Update normal
  rep->SetNormal(sliceToRAS->GetElement(0,2),
                 sliceToRAS->GetElement(1,2),
                 sliceToRAS->GetElement(2,2));
  // Update origin position
  rep->SetOrigin(sliceToRAS->GetElement(0,3),
                 sliceToRAS->GetElement(1,3),
                 sliceToRAS->GetElement(2,3));

#if VTK_MAJOR_VERSION > 7
  rep->SetDrawOutline(sliceNode->GetWidgetOutlineVisible());
#endif

  // Update the widget itself if necessary
  bool visible =
    sliceNode->IsDisplayableInThreeDView(this->External->GetMRMLViewNode()->GetID())
    && sliceNode->GetWidgetVisible();

  // re-render if it was visible or now becomes visible
  bool renderingRequired = planeWidget->GetEnabled() || visible;

  if ((!planeWidget->GetEnabled() && visible) ||
     (planeWidget->GetEnabled() && !visible) ||
     (!rep->GetLockNormalToCamera() && sliceNode->GetWidgetNormalLockedToCamera()) ||
     (rep->GetLockNormalToCamera() && !sliceNode->GetWidgetNormalLockedToCamera()))
    {
    planeWidget->SetEnabled(sliceNode->GetWidgetVisible());
    planeWidget->SetLockNormalToCamera(sliceNode->GetWidgetNormalLockedToCamera());
    }

  return renderingRequired;
}

//---------------------------------------------------------------------------
// vtkMRMLSliceModelDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLThreeDReformatDisplayableManager::vtkMRMLThreeDReformatDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLThreeDReformatDisplayableManager::~vtkMRMLThreeDReformatDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->RemoveAllSliceNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::UpdateFromMRMLScene()
{
  this->Internal->UpdateSliceNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager
::OnMRMLSceneNodeAdded(vtkMRMLNode* nodeAdded)
{
  if (this->GetMRMLScene()->IsBatchProcessing() ||
      !nodeAdded->IsA("vtkMRMLSliceNode"))
    {
    return;
    }

  this->Internal->AddSliceNode(vtkMRMLSliceNode::SafeDownCast(nodeAdded));
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager
::OnMRMLSceneNodeRemoved(vtkMRMLNode* nodeRemoved)
{
  if (!nodeRemoved->IsA("vtkMRMLSliceNode"))
    {
    return;
    }

  this->Internal->RemoveSliceNode(vtkMRMLSliceNode::SafeDownCast(nodeRemoved));
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::
OnMRMLNodeModified(vtkMRMLNode* node)
{
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  assert(sliceNode);
  vtkImplicitPlaneWidget2* planeWidget = this->Internal->GetWidget(sliceNode);
  if (this->Internal->UpdateWidget(sliceNode, planeWidget))
    {
    this->RequestRender();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::
ProcessWidgetsEvents(vtkObject *caller,
                    unsigned long event,
                    void *vtkNotUsed(callData))
{
  vtkImplicitPlaneWidget2* planeWidget =
    vtkImplicitPlaneWidget2::SafeDownCast(caller);
  vtkMRMLSliceNode* sliceNode = Internal->GetSliceNode(planeWidget);
  vtkImplicitPlaneRepresentation* rep = (planeWidget) ?
    planeWidget->GetImplicitPlaneRepresentation() : nullptr;

  if (!planeWidget || !sliceNode || !rep)
    {
    return;
    }

  // Broadcast widget transformation
  vtkMRMLSliceLogic* sliceLogic = this->GetMRMLApplicationLogic()->GetSliceLogic(sliceNode);
  if (event == vtkCommand::StartInteractionEvent && sliceLogic )
    {
    sliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::MultiplanarReformatFlag);
    return;
    }
  else if (event == vtkCommand::EndInteractionEvent && sliceLogic)
    {
    sliceLogic->EndSliceNodeInteraction();
    return;
    }
  // We should listen to the interactorStyle instead when LockNormalToCamera on.
  else if (planeWidget->GetImplicitPlaneRepresentation()->GetLockNormalToCamera() && sliceLogic)
    {
    sliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::MultiplanarReformatFlag);
    }

  double cross[3], dot, rotation;
  vtkNew<vtkTransform> transform;
  vtkMatrix4x4* sliceToRAS = sliceNode->GetSliceToRAS();
  double sliceNormal[3] = {sliceToRAS->GetElement(0,2),
                           sliceToRAS->GetElement(1,2),
                           sliceToRAS->GetElement(2,2)};

  // Reset current translation
  sliceToRAS->SetElement(0,3,0);
  sliceToRAS->SetElement(1,3,0);
  sliceToRAS->SetElement(2,3,0);

  // Rotate the sliceNode to match the planeWidget normal
  vtkMath::Cross(sliceNormal, rep->GetNormal(), cross);
  dot = vtkMath::Dot(sliceNormal, rep->GetNormal());
  // Clamp the dot product
  dot = (dot < -1.0) ? -1.0 : (dot > 1.0 ? 1.0 : dot);
  rotation = vtkMath::DegreesFromRadians(acos(dot));

  // Apply the rotation
  transform->PostMultiply();
  transform->SetMatrix(sliceToRAS);
  transform->RotateWXYZ(rotation,cross);
  transform->GetMatrix(sliceToRAS); // Update the changes within sliceToRAS

  // Insert the widget translation
  double* planeWidgetOrigin = rep->GetOrigin();
  sliceToRAS->SetElement(0, 3, planeWidgetOrigin[0]);
  sliceToRAS->SetElement(1, 3, planeWidgetOrigin[1]);
  sliceToRAS->SetElement(2, 3, planeWidgetOrigin[2]);
  sliceNode->UpdateMatrices();

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDReformatDisplayableManager::Create()
{
  this->Internal->UpdateSliceNodes();
}
