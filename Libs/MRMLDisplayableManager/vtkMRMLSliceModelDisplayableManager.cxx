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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLSliceModelDisplayableManager.h"

// MRML includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLSliceModelDisplayableManager );
vtkCxxRevisionMacro(vtkMRMLSliceModelDisplayableManager, "$Revision: 13525 $");

//---------------------------------------------------------------------------
class vtkMRMLSliceModelDisplayableManager::vtkInternal
{
public:
  typedef std::map<vtkMRMLDisplayableNode*, std::vector<vtkMRMLDisplayNode*> > DisplayNodesType;
  typedef std::map<vtkMRMLDisplayNode*, vtkProp*> DisplayActorsType;

  vtkInternal(vtkMRMLSliceModelDisplayableManager * external);
  ~vtkInternal();

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
  void AddVolume(vtkMRMLDisplayableNode* volume);
  // Remove the volume pointed by the iterator, return an iterator pointing to
  // the following
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
  vtkMRMLSliceModelDisplayableManager*      External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLSliceModelDisplayableManager::vtkInternal
::vtkInternal(vtkMRMLSliceModelDisplayableManager * external)
{
  this->External = external;
  this->SliceCompositeNode = 0;
}

//---------------------------------------------------------------------------
vtkMRMLSliceModelDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetSliceCompositeNode(0);
  // everything should be empty
  assert(this->SliceCompositeNode == 0);
  assert(this->VolumeNodes.size() == 0);
  assert(this->DisplayNodes.size() == 0);
  assert(this->Actors.size() == 0);
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceModelDisplayableManager::vtkInternal
::GetSliceNode()
{
  return this->External->GetMRMLSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal::UpdateSliceNode()
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
vtkMRMLSliceCompositeNode* vtkMRMLSliceModelDisplayableManager::vtkInternal
::FindSliceCompositeNode()
{
  if (this->External->GetMRMLScene() == 0 ||
      !this->GetSliceNode())
    {
    return 0;
    }

  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->External->GetMRMLScene()->GetCurrentScene();
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
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal
::SetSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode)
{
  if (this->SliceCompositeNode == compositeNode)
    {
    return;
    }
  if (this->SliceCompositeNode)
    {
    this->SliceCompositeNode->RemoveObserver(
      this->External->GetMRMLCallbackCommand());
    }
  this->SliceCompositeNode = compositeNode;
  if (this->SliceCompositeNode)
    {
    this->SliceCompositeNode->AddObserver(vtkCommand::ModifiedEvent,
                                          this->External->GetMRMLCallbackCommand());
    }
  this->UpdateSliceCompositeNode(this->SliceCompositeNode);
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal
::UpdateSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode)
{
  assert(compositeNode == this->SliceCompositeNode);
  this->SetVolumeNodes(this->GetVolumeNodes(compositeNode));
}

//---------------------------------------------------------------------------
std::vector<vtkMRMLDisplayableNode*>  vtkMRMLSliceModelDisplayableManager::vtkInternal
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
void vtkMRMLSliceModelDisplayableManager::vtkInternal::SetVolumeNodes(
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
  assert(this->VolumeNodes.size() == volumes.size());
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal
::AddVolume(vtkMRMLDisplayableNode* volume)
{
  if (std::find(this->VolumeNodes.begin(), this->VolumeNodes.end(), volume) != this->VolumeNodes.end())
    {
    // volume already exists in the list, don't need to add it
    return;
    }
  volume->AddObserver(vtkMRMLDisplayableNode::DisplayModifiedEvent,
                      this->External->GetMRMLCallbackCommand());
  this->VolumeNodes.push_back(volume);
  this->UpdateVolume(volume);
}

//---------------------------------------------------------------------------
std::vector<vtkMRMLDisplayableNode*>::iterator vtkMRMLSliceModelDisplayableManager::vtkInternal
::RemoveVolume(std::vector<vtkMRMLDisplayableNode*>::iterator volumeIt)
{
  vtkMRMLDisplayableNode* volume = *volumeIt;
  // Stop listening to events
  volume->RemoveObserver(this->External->GetMRMLCallbackCommand());
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
void vtkMRMLSliceModelDisplayableManager::vtkInternal
::UpdateVolume(vtkMRMLDisplayableNode* volume)
{
  // Sanity check: make sure the volume exists in the list
  assert(std::find(this->VolumeNodes.begin(), this->VolumeNodes.end(), volume) != this->VolumeNodes.end());
  // The volume has just been added to the volume list or has been modified,
  // update its display node list.
  this->SetVolumeDisplayNodes(volume, volume->GetDisplayNodes());
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal
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
          displayNodesIt->first->GetDisplayNodes().size());
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal::AddVolumeDisplayNode(
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
                           this->External->GetMRMLCallbackCommand());
  displayNodesIt->second.push_back(displayNode);
  this->UpdateVolumeDisplayNode(displayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal::RemoveVolumeDisplayNodes(
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
std::vector<vtkMRMLDisplayNode*>::iterator vtkMRMLSliceModelDisplayableManager::vtkInternal
::RemoveVolumeDisplayNode(std::vector<vtkMRMLDisplayNode*>& displayNodes,
                          std::vector<vtkMRMLDisplayNode*>::iterator displayNodeIt)
{
  // Stop listening to events
  vtkMRMLDisplayNode* displayNode = *displayNodeIt;
  displayNode->RemoveObserver(this->External->GetMRMLCallbackCommand());
  // Remove actors associated with the displayNode
  this->RemoveActor(this->Actors.find(displayNode));

  return displayNodes.erase(displayNodeIt);
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal::UpdateVolumeDisplayNode(
  vtkMRMLDisplayNode* displayNode)
{
  if (!this->IsDisplayable(displayNode) ||
      !this->IsVisible(displayNode))
    {
    return;
    }
  DisplayActorsType::const_iterator it = this->Actors.find(displayNode);
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
void vtkMRMLSliceModelDisplayableManager::vtkInternal::AddActor(
  vtkMRMLDisplayNode* displayNode)
{
  vtkActor2D* actor = vtkActor2D::New();
  if (displayNode->IsA("vtkMRMLDiffusionTensorVolumeSliceDisplayNode"))
    {
    vtkSmartPointer<vtkPolyDataMapper2D> mapper =
      vtkSmartPointer<vtkPolyDataMapper2D>::New();
    actor->SetMapper( mapper );
    }
  this->External->GetRenderer()->AddActor( actor );
  this->Actors[displayNode] = actor;
  this->UpdateActor(displayNode, actor);
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal::UpdateActor(
  vtkMRMLDisplayNode* displayNode, vtkProp* actor)
{
  if (displayNode->IsA("vtkMRMLDiffusionTensorVolumeSliceDisplayNode"))
    {
    vtkMRMLDiffusionTensorVolumeSliceDisplayNode* dtiDisplayNode =
      vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SafeDownCast(displayNode);
    vtkPolyData* polyData = dtiDisplayNode->GetPolyDataTransformedToSlice();

    vtkActor2D* actor2D = vtkActor2D::SafeDownCast(actor);
    vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(
      actor2D->GetMapper());

    mapper->SetInput( polyData );
    mapper->SetLookupTable( dtiDisplayNode->GetColorNode() ?
                            dtiDisplayNode->GetColorNode()->GetScalarsToColors() : 0);
    }
  // TBD: Not sure a render request has to systematically be called
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::vtkInternal::RemoveActor(
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
bool vtkMRMLSliceModelDisplayableManager::vtkInternal::IsDisplayable(
  vtkMRMLDisplayNode* displayNode)
{
  // Currently only support DTI Slice display nodes, add here more type if
  // needed
  return displayNode->IsA("vtkMRMLDiffusionTensorVolumeSliceDisplayNode") &&
    std::string(displayNode->GetName()) == this->GetSliceNode()->GetLayoutName();
}

//---------------------------------------------------------------------------
bool vtkMRMLSliceModelDisplayableManager::vtkInternal::IsVisible(
  vtkMRMLDisplayNode* displayNode)
{
  return displayNode->GetVisibility() &&
    displayNode->GetScalarVisibility();
}

//---------------------------------------------------------------------------
// vtkMRMLSliceModelDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLSliceModelDisplayableManager::vtkMRMLSliceModelDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLSliceModelDisplayableManager::~vtkMRMLSliceModelDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::ProcessMRMLEvents(vtkObject * caller,
                                                            unsigned long vtkNotUsed(event),
                                                            void *callData)
{
  // TBD: don't update when the scene is processing
  if (vtkMRMLSliceCompositeNode::SafeDownCast(caller))
    {
    //std::cout << "SLICE COMPOSITE UPDATED" << std::endl;
    this->Internal->UpdateSliceCompositeNode(vtkMRMLSliceCompositeNode::SafeDownCast(caller));
    }
  else if (vtkMRMLDisplayableNode::SafeDownCast(caller))
    {
    //std::cout << "VOLUME UPDATED" << std::endl;
    if (callData == 0)
      {
      this->Internal->UpdateVolume(vtkMRMLDisplayableNode::SafeDownCast(caller));
      }
    }
  else if (vtkMRMLDisplayNode::SafeDownCast(caller))
    {
    //std::cout << "DISPLAY NODE UPDATED" << std::endl;
    this->Internal->UpdateVolumeDisplayNode(vtkMRMLDisplayNode::SafeDownCast(caller));
    }
  // Default MRML Event handler is NOT needed
//  else
//    {
//    this->Superclass::ProcessMRMLEvents(caller, event, callData);
//    }
}

//---------------------------------------------------------------------------
void vtkMRMLSliceModelDisplayableManager::Create()
{
  this->Internal->UpdateSliceNode();
}
