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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLVolumeGlyphSliceDisplayableManager.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkRenderer.h>
#include <vtkWeakPointer.h>
#include <vtkVersion.h>

// STD includes
#include <algorithm>
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLVolumeGlyphSliceDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
{
public:
  typedef std::map<vtkMRMLDisplayableNode*, std::vector<vtkMRMLDisplayNode*> > DisplayNodesType;
  typedef std::map<vtkMRMLDisplayNode*, vtkProp*> DisplayActorsType;

  vtkInternal(vtkMRMLVolumeGlyphSliceDisplayableManager * external);
  ~vtkInternal();

  vtkObserverManager* GetMRMLNodesObserverManager()const;
  void Modified();

  bool IsDisplayable(vtkMRMLDisplayNode*);
  bool IsVisible(vtkMRMLDisplayNode*);

  // Slice
  vtkMRMLSliceNode* GetSliceNode();
  void UpdateSliceNode();
  // Slice Composite
  vtkMRMLSliceCompositeNode* FindSliceCompositeNode();
  void SetSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode);
  void UpdateSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode);
  std::vector<vtkMRMLDisplayableNode*>  GetVolumeNodes(
    vtkMRMLSliceCompositeNode*);

  // Volume
  void SetVolumeNodes(const std::vector<vtkMRMLDisplayableNode*>& volumes);
  // Add a volume to the list if it is not yet added. Observe its events
  // Return true on success, false if the volume already exists.
  bool AddVolume(vtkMRMLDisplayableNode* volume);
  // Remove the volume pointed by the iterator, return an iterator pointing to
  // the following
  void RemoveVolume(vtkMRMLDisplayableNode* volume);
  std::vector<vtkMRMLDisplayableNode*>::iterator RemoveVolume(
    std::vector<vtkMRMLDisplayableNode*>::iterator volumeIt);
  void UpdateVolume(vtkMRMLDisplayableNode* volume);

  // VolumeDisplayNode
  void SetVolumeDisplayNodes(vtkMRMLDisplayableNode* volume,
                             std::vector<vtkMRMLDisplayNode*> displayNodes);
  void AddVolumeDisplayNode(DisplayNodesType::iterator displayNodesIt,
                             vtkMRMLDisplayNode* displayNode);
  void RemoveVolumeDisplayNodes(DisplayNodesType::iterator displayNodesIt);
  std::vector<vtkMRMLDisplayNode*>::iterator RemoveVolumeDisplayNode(
    std::vector<vtkMRMLDisplayNode*>& displayNodes,
    std::vector<vtkMRMLDisplayNode*>::iterator displayNodeIt);
  void UpdateVolumeDisplayNode(vtkMRMLDisplayNode* displayNode);

  // Actors
  void AddActor(vtkMRMLDisplayNode* displayNode);
  void RemoveActor(DisplayActorsType::iterator actorIt);
  void UpdateActor(vtkMRMLDisplayNode* displayNode, vtkProp* actor);

  vtkWeakPointer<vtkMRMLSliceCompositeNode> SliceCompositeNode;
  std::vector<vtkMRMLDisplayableNode*>      VolumeNodes;
  DisplayNodesType                          DisplayNodes;
  DisplayActorsType                         Actors;
  vtkMRMLVolumeGlyphSliceDisplayableManager*      External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::vtkInternal(vtkMRMLVolumeGlyphSliceDisplayableManager * external)
{
  this->External = external;
  this->SliceCompositeNode = nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetSliceCompositeNode(nullptr);
  // everything should be empty
  assert(this->SliceCompositeNode == nullptr);
  assert(this->VolumeNodes.size() == 0);
  assert(this->DisplayNodes.size() == 0);
  assert(this->Actors.size() == 0);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::GetMRMLNodesObserverManager()const
{
  return this->External->GetMRMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::GetSliceNode()
{
  return this->External->GetMRMLSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::UpdateSliceNode()
{
  assert(!this->GetSliceNode() || this->GetSliceNode()->GetLayoutName());
  // search the scene for a matching slice composite node
  if (!this->SliceCompositeNode.GetPointer() || // the slice composite has been deleted
      !this->SliceCompositeNode->GetLayoutName() || // the slice composite points to a diff slice node
      strcmp(this->SliceCompositeNode->GetLayoutName(),
             this->GetSliceNode()->GetLayoutName()))
    {
    vtkMRMLSliceCompositeNode* sliceCompositeNode =
      this->FindSliceCompositeNode();
    this->SetSliceCompositeNode(sliceCompositeNode);
    }
}

//---------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::FindSliceCompositeNode()
{
  if (this->External->GetMRMLScene() == nullptr ||
      !this->GetSliceNode())
    {
    return nullptr;
    }

  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->External->GetMRMLScene()->GetNodes();
  for (scene->InitTraversal(it);
       (node = (vtkMRMLNode*)scene->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLSliceCompositeNode* sliceCompositeNode =
      vtkMRMLSliceCompositeNode::SafeDownCast(node);
    if (sliceCompositeNode && sliceCompositeNode->GetLayoutName() &&
        !strcmp(sliceCompositeNode->GetLayoutName(),
                this->GetSliceNode()->GetLayoutName()) )
      {
      return sliceCompositeNode;
      }
    }
  // no matching slice composite node is found
  assert(0);
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::SetSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode)
{
  if (this->SliceCompositeNode == compositeNode)
    {
    return;
    }

  vtkSetAndObserveMRMLNodeMacro(this->SliceCompositeNode, compositeNode);
  this->UpdateSliceCompositeNode(this->SliceCompositeNode);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::UpdateSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode)
{
  assert(compositeNode == this->SliceCompositeNode);
  this->SetVolumeNodes(this->GetVolumeNodes(compositeNode));
}

//---------------------------------------------------------------------------
std::vector<vtkMRMLDisplayableNode*>  vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::GetVolumeNodes(vtkMRMLSliceCompositeNode* compositeNode)
{
  std::vector<vtkMRMLDisplayableNode*> res;
  if (!compositeNode)
    {
    return res;
    }
  std::vector<vtkMRMLNode*> allVolumes;
  allVolumes.push_back(
    this->External->GetMRMLScene()->GetNodeByID(
      compositeNode->GetBackgroundVolumeID()));
  allVolumes.push_back(
    this->External->GetMRMLScene()->GetNodeByID(
      compositeNode->GetForegroundVolumeID()));
  // As we only support diffusiontensorvolumes for now, we can't find them in
  // the label layer. Feel free to uncomment if needed
  //allVolumes.push_back(
  //  this->External->GetMRMLScene()->GetNodeByID(compositeNode->GetLabelVolumeID()));
  for (unsigned int i = 0; i < allVolumes.size(); ++i)
    {
    // currently only support diffusiontensorvolumes but can easily be extended to other volumes
    vtkMRMLDiffusionTensorVolumeNode* dtiNode =
      vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(allVolumes[i]);
    if (dtiNode)
      {
      res.push_back(dtiNode);
      }
    // Add other volumes supported here
    }
  return res;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::SetVolumeNodes(
  const std::vector<vtkMRMLDisplayableNode*>& volumes)
{
  if (volumes == this->VolumeNodes)
    {
    return;
    }
  // Removes the volumes that are not into the new volume list
  for (std::vector<vtkMRMLDisplayableNode*>::iterator it = this->VolumeNodes.begin();
       it != this->VolumeNodes.end(); )
    {
    vtkMRMLDisplayableNode* volume = *it;
    // Don't remove the volume if it belongs to the new list
    if (std::find(volumes.begin(), volumes.end(), volume) == volumes.end())
      {
      // RemoveVolume() returns a pointer to the next volume
      it = this->RemoveVolume(it);
      }
    else
      {
      ++it;
      }
    }
  // Add the new volumes if they are not present yet.
  for (unsigned int i = 0; i < volumes.size(); ++i)
    {
    this->AddVolume(volumes[i]);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::AddVolume(vtkMRMLDisplayableNode* volume)
{
  if (std::find(this->VolumeNodes.begin(), this->VolumeNodes.end(), volume) != this->VolumeNodes.end())
    {
    // volume already exists in the list, don't need to add it
    return false;
    }
  // Only observe when a display node is added/removed from the displayable
  // node.
  volume->AddObserver(vtkMRMLDisplayableNode::DisplayModifiedEvent,
                      this->External->GetMRMLNodesCallbackCommand());
  this->VolumeNodes.push_back(volume);
  this->UpdateVolume(volume);
  return true;
}

//---------------------------------------------------------------------------
std::vector<vtkMRMLDisplayableNode*>::iterator vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::RemoveVolume(std::vector<vtkMRMLDisplayableNode*>::iterator volumeIt)
{
  vtkMRMLDisplayableNode* volume = *volumeIt;
  // Stop listening to events
  volume->RemoveObserver(this->External->GetMRMLNodesCallbackCommand());
  // Remove displaynodes and actors
  DisplayNodesType::iterator it = this->DisplayNodes.find(volume);
  if (it != this->DisplayNodes.end())
    {
    this->RemoveVolumeDisplayNodes(it);
    }
  // vector::erase() returns a random access iterator pointing to the new location
  // of the element that followed the last element erased by the function call.
  return this->VolumeNodes.erase(volumeIt);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::RemoveVolume(vtkMRMLDisplayableNode* volume)
{
  // Sanity check: make sure the volume exists in the list
  std::vector<vtkMRMLDisplayableNode*>::iterator volumeIt = std::find(this->VolumeNodes.begin(), this->VolumeNodes.end(), volume);
  if (volumeIt != this->VolumeNodes.end())
    {
    this->RemoveVolume(volumeIt);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::UpdateVolume(vtkMRMLDisplayableNode* volume)
{
  // Sanity check: make sure the volume exists in the list
  assert(std::find(this->VolumeNodes.begin(), this->VolumeNodes.end(), volume) != this->VolumeNodes.end());
  // The volume has just been added to the volume list or has been modified,
  // update its display node list.
  int nnodes = volume->GetNumberOfDisplayNodes();
  std::vector<vtkMRMLDisplayNode*> displayNodes;
  for (int i=0; i<nnodes; i++)
  {
    displayNodes.push_back(volume->GetNthDisplayNode(i));
  }
  this->SetVolumeDisplayNodes(volume, displayNodes);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::SetVolumeDisplayNodes(vtkMRMLDisplayableNode* volume,
                        std::vector<vtkMRMLDisplayNode*> newDisplayNodes)
{
  // Make sure it doesn't exist yet in the display node list
  DisplayNodesType::iterator displayNodesIt = this->DisplayNodes.find(volume);
  if (displayNodesIt != this->DisplayNodes.end())
    {
    if (displayNodesIt->second == newDisplayNodes)
      {
      // Nothing to do here, the list has already been added and is the same.
      // All the display nodes are already observed.
      return;
      }
    // Remove the display nodes that are not in the new display node list
    std::vector<vtkMRMLDisplayNode*>::iterator it;
    for (it = displayNodesIt->second.begin();
         it != displayNodesIt->second.end(); )
      {
      if (std::find(newDisplayNodes.begin(), newDisplayNodes.end(), *it) ==
          newDisplayNodes.end())
        {
        it = this->RemoveVolumeDisplayNode(displayNodesIt->second, it);
        }
      else
        {
        ++it;
        }
      }
    }
  else // volume is not yet in the DisplayNodes list, add it.
    {
    this->DisplayNodes[volume] = std::vector<vtkMRMLDisplayNode*>();
    displayNodesIt = this->DisplayNodes.find(volume);
    assert(displayNodesIt != this->DisplayNodes.end());
    }
  // Add the display nodes that are not added yet
  for (unsigned int i = 0; i < newDisplayNodes.size(); ++i)
    {
    this->AddVolumeDisplayNode(displayNodesIt, newDisplayNodes[i]);
    }
  // Make sure there is no more display nodes than the volume has
  assert( displayNodesIt->second.size() <=
          (unsigned int) displayNodesIt->first->GetNumberOfDisplayNodes());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::AddVolumeDisplayNode(
  DisplayNodesType::iterator displayNodesIt, vtkMRMLDisplayNode* displayNode)
{
  std::vector<vtkMRMLDisplayNode*>& displayNodes = displayNodesIt->second;
  if (std::find(displayNodes.begin(), displayNodes.end(), displayNode) !=
      displayNodes.end())
    {
    // displayNode is already in the list and already taken care of
    return;
    }
  if (!this->IsDisplayable(displayNode))
    {
    return;
    }
  displayNode->AddObserver(vtkCommand::ModifiedEvent,
                           this->External->GetMRMLNodesCallbackCommand());
  displayNodesIt->second.push_back(displayNode);
  this->UpdateVolumeDisplayNode(displayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::RemoveVolumeDisplayNodes(
  DisplayNodesType::iterator displayNodesIt)
{
  if (displayNodesIt == this->DisplayNodes.end())
    {
    return;
    }
  // Stop listening to all the displayNodes associtaed with the volume
  for (std::vector<vtkMRMLDisplayNode*>::iterator it = displayNodesIt->second.begin();
       it != displayNodesIt->second.end(); )
    {
    it = this->RemoveVolumeDisplayNode(displayNodesIt->second, it);
    }
  assert(displayNodesIt->second.size() == 0);
  this->DisplayNodes.erase(displayNodesIt);
  return;
}

//---------------------------------------------------------------------------
std::vector<vtkMRMLDisplayNode*>::iterator vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal
::RemoveVolumeDisplayNode(std::vector<vtkMRMLDisplayNode*>& displayNodes,
                          std::vector<vtkMRMLDisplayNode*>::iterator displayNodeIt)
{
  // Stop listening to events
  vtkMRMLDisplayNode* displayNode = *displayNodeIt;
  displayNode->RemoveObserver(this->External->GetMRMLNodesCallbackCommand());
  // Remove actors associated with the displayNode
  this->RemoveActor(this->Actors.find(displayNode));

  return displayNodes.erase(displayNodeIt);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::UpdateVolumeDisplayNode(
  vtkMRMLDisplayNode* displayNode)
{
  if (!this->IsDisplayable(displayNode))
    {
    return;
    }
  DisplayActorsType::const_iterator it = this->Actors.find(displayNode);
  // If the actors haven't been created yet and the actors should not be visible
  // anyway, then don't even create them. However if the actors were created
  // and the visibility has later been turned off, then update the actor by
  // setting its visibility to off.
  if (!this->IsVisible(displayNode) && it == this->Actors.end())
    {
    return;
    }
  std::cout << "vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::UpdateVolumeDisplayNode()" << std::endl;
  if (it == this->Actors.end())
    {
    this->AddActor(displayNode);
    }
  else
    {
    // there is already an actor, just update
    this->UpdateActor(it->first, it->second);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::AddActor(
  vtkMRMLDisplayNode* displayNode)
{
  vtkActor2D* actor = vtkActor2D::New();
  if (displayNode->IsA("vtkMRMLDiffusionTensorVolumeSliceDisplayNode"))
    {
    vtkNew<vtkPolyDataMapper2D> mapper;
    actor->SetMapper(mapper.GetPointer());
    }
  this->External->GetRenderer()->AddActor( actor );
  this->Actors[displayNode] = actor;
  this->UpdateActor(displayNode, actor);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::UpdateActor(
  vtkMRMLDisplayNode* displayNode, vtkProp* actor)
{
  if (displayNode->IsA("vtkMRMLDiffusionTensorVolumeSliceDisplayNode"))
    {
    vtkMRMLDiffusionTensorVolumeSliceDisplayNode* dtiDisplayNode =
      vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SafeDownCast(displayNode);

    vtkActor2D* actor2D = vtkActor2D::SafeDownCast(actor);
    vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(
      actor2D->GetMapper());
    mapper->SetInputConnection( dtiDisplayNode->GetSliceOutputPort() );
    mapper->SetLookupTable( dtiDisplayNode->GetColorNode() ?
                            dtiDisplayNode->GetColorNode()->GetScalarsToColors() : nullptr);
    mapper->SetScalarRange(dtiDisplayNode->GetScalarRange());
    }
  actor->SetVisibility(this->IsVisible(displayNode));
  // TBD: Not sure a render request has to systematically be called
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::RemoveActor(
  DisplayActorsType::iterator actorIt)
{
  if (actorIt == this->Actors.end())
    {
    return;
    }
  this->External->GetRenderer()->RemoveActor( actorIt->second );
  actorIt->second->Delete();
  this->Actors.erase(actorIt);
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::IsDisplayable(
  vtkMRMLDisplayNode* displayNode)
{
  // Currently only support DTI Slice display nodes, add here more type if
  // needed
  return displayNode
    && displayNode->IsA("vtkMRMLDiffusionTensorVolumeSliceDisplayNode")
    && (std::string(displayNode->GetName()) == this->GetSliceNode()->GetLayoutName());
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeGlyphSliceDisplayableManager::vtkInternal::IsVisible(
  vtkMRMLDisplayNode* displayNode)
{
  return displayNode->GetVisibility() && displayNode->GetVisibility2D() &&
         displayNode->GetScalarVisibility();
}

//---------------------------------------------------------------------------
// vtkMRMLVolumeGlyphSliceDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLVolumeGlyphSliceDisplayableManager::vtkMRMLVolumeGlyphSliceDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLVolumeGlyphSliceDisplayableManager::~vtkMRMLVolumeGlyphSliceDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager
::UnobserveMRMLScene()
{
  this->Internal->SetSliceCompositeNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager
::UpdateFromMRMLScene()
{
  this->Internal->UpdateSliceNode();
}
//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager
::OnMRMLSceneStartClose()
{
  std::vector<vtkMRMLDisplayableNode*> volumes = this->Internal->VolumeNodes;
  for (std::vector<vtkMRMLDisplayableNode*>::iterator it = volumes.begin();
       it != volumes.end(); it++)
    {
    this->Internal->RemoveVolume(*it);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager
::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (event == vtkMRMLDisplayableNode::DisplayModifiedEvent)
    {
    if (vtkMRMLDisplayableNode::SafeDownCast(caller))
      {
        if (callData == nullptr || vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SafeDownCast((vtkObject *)callData) !=nullptr ) // a display node is added/removed/modified
        {
        this->Internal->UpdateVolume(vtkMRMLDisplayableNode::SafeDownCast(caller));
        }
      }
    }
  else if (event == vtkCommand::ModifiedEvent)
    {
    if (vtkMRMLSliceCompositeNode::SafeDownCast(caller))
      {
      this->Internal->UpdateSliceCompositeNode(vtkMRMLSliceCompositeNode::SafeDownCast(caller));
      }
    else if (vtkMRMLDisplayNode::SafeDownCast(caller))
      {
      this->Internal->UpdateVolumeDisplayNode(vtkMRMLDisplayNode::SafeDownCast(caller));
      }
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeGlyphSliceDisplayableManager::Create()
{
  this->Internal->UpdateSliceNode();
}
