/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Data Logic includes
#include "vtkSlicerDataModuleLogic.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerDataModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerDataModuleLogic::vtkSlicerDataModuleLogic()
{
  this->SceneChanged = false;
  this->AutoRemoveDisplayAndStorageNodes = true;
}

//----------------------------------------------------------------------------
vtkSlicerDataModuleLogic::~vtkSlicerDataModuleLogic() = default;

//----------------------------------------------------------------------------
void vtkSlicerDataModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "AutoRemoveDisplayAndStorageNode: " <<
    (AutoRemoveDisplayAndStorageNodes ? "On" : "Off") << "\n";

 }

//---------------------------------------------------------------------------
void vtkSlicerDataModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerDataModuleLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != nullptr);
}

//---------------------------------------------------------------------------
void vtkSlicerDataModuleLogic::UpdateFromMRMLScene()
{
  this->SceneChangedOn();
}


//----------------------------------------------------------------------------
void vtkSlicerDataModuleLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!displayableNode || this->GetMRMLScene()->IsClosing() || !this->AutoRemoveDisplayAndStorageNodes)
    {
    return;
    }

  // Collect a list of storage and display nodes that are only
  // referenced by the node to be removed.
  std::vector< vtkWeakPointer<vtkMRMLNode> > nodesToRemove;

  /// we can't get the display node directly as it might be 0 because the
  /// displayable node has no longer access to the scene
  std::vector<vtkMRMLNode *> referencingNodes;
  for (int i = 0; i < displayableNode->GetNumberOfDisplayNodes(); ++i)
    {
    vtkMRMLNode *dnode = this->GetMRMLScene()->GetNodeByID(
      displayableNode->GetNthDisplayNodeID(i));

    // make sure no other nodes reference this display node
    this->GetMRMLScene()->GetReferencingNodes(dnode, referencingNodes);

    if (referencingNodes.size() == 0 ||
        (referencingNodes.size() == 1 && referencingNodes[0] == node) )
      {
      nodesToRemove.emplace_back(dnode);
      }
    }
  for (int i = 0; i < displayableNode->GetNumberOfStorageNodes(); ++i)
    {
    vtkMRMLNode *snode = this->GetMRMLScene()->GetNodeByID(
      displayableNode->GetNthStorageNodeID(i));

    // make sure no other nodes reference this storage node
    this->GetMRMLScene()->GetReferencingNodes(snode, referencingNodes);

    if (referencingNodes.size() == 0 ||
        (referencingNodes.size() == 1 && referencingNodes[0] == node) )
      {
      nodesToRemove.emplace_back(snode);
      }
    }

  // Now remove the collected nodes. Batch process is only used if many nodes will be removed
  // because entering/exiting batch processing is a very expensive operation (the display flickers,
  // lots of things are recomputed), so it should be only done if we save time by skipping many small updates.
  int toRemove = nodesToRemove.size();
  bool useBatchMode = toRemove > 10; // Switch to batch mode if more than 10 nodes to remove
  int progress = 0;
  if (useBatchMode)
    {
    this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState, toRemove);
    }
  std::vector< vtkWeakPointer<vtkMRMLNode> >::const_iterator nodeIterator;
  nodeIterator = nodesToRemove.begin();
  while (nodeIterator != nodesToRemove.end())
    {
    if (nodeIterator->GetPointer())
       {
      this->GetMRMLScene()->RemoveNode(*nodeIterator);
      if (useBatchMode)
        {
        this->GetMRMLScene()->ProgressState(vtkMRMLScene::BatchProcessState, ++progress);
        }
      }
    ++nodeIterator;
    }
  if (useBatchMode)
    {
    this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
    }
}
