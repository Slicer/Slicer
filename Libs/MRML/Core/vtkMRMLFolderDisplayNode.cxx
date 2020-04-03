/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE.

==============================================================================*/

#include "vtkMRMLFolderDisplayNode.h"

// MRML includes
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// VTK includes
#include <vtkCallbackCommand.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFolderDisplayNode);

//-----------------------------------------------------------------------------
vtkMRMLFolderDisplayNode::vtkMRMLFolderDisplayNode() = default;

//-----------------------------------------------------------------------------
vtkMRMLFolderDisplayNode::~vtkMRMLFolderDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLFolderDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(ApplyDisplayPropertiesOnBranch);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLFolderDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(applyDisplayPropertiesOnBranch, ApplyDisplayPropertiesOnBranch);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLFolderDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(applyDisplayPropertiesOnBranch, ApplyDisplayPropertiesOnBranch);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLFolderDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLFolderDisplayNode* node = vtkMRMLFolderDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(ApplyDisplayPropertiesOnBranch);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLFolderDisplayNode::SetScene(vtkMRMLScene* scene)
{
  Superclass::SetScene(scene);

  if (scene)
    {
    // Observe subject hierarchy item reparented event
    vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    if (!shNode)
      {
      vtkErrorMacro("SetScene: Failed to get subject hierarchy node from current scene");
      return;
      }
    if (!shNode->HasObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemReparentedEvent, this->MRMLCallbackCommand))
      {
      shNode->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemReparentedEvent, this->MRMLCallbackCommand);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLFolderDisplayNode::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if ( event == vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemReparentedEvent
    && vtkMRMLSubjectHierarchyNode::SafeDownCast(caller) )
    {
    // No-op if this folder node does not apply display properties on its branch
    if (!this->ApplyDisplayPropertiesOnBranch)
      {
      return;
      }
    // Get item ID for subject hierarchy node events
    vtkIdType reparentedItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    if (callData)
      {
      vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
      if (itemIdPtr)
        {
        reparentedItemID = *itemIdPtr;
        }
      }
    vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(caller);
    vtkMRMLDisplayableNode* displayableReparentedNode = vtkMRMLDisplayableNode::SafeDownCast(
      shNode->GetItemDataNode(reparentedItemID) );
    // Trigger display update for reparented displayable node if it is in a folder that applies
    // display properties on its branch (only display nodes that allow overriding)
    for (int i=0; i<displayableReparentedNode->GetNumberOfDisplayNodes(); ++i)
      {
      vtkMRMLDisplayNode* currentDisplayNode = displayableReparentedNode->GetNthDisplayNode(i);
      if (currentDisplayNode->GetFolderDisplayOverrideAllowed())
        {
        currentDisplayNode->Modified();
        }
      } // For all display nodes
    } // SubjectHierarchyItemReparentedEvent
}

//----------------------------------------------------------------------------
void vtkMRMLFolderDisplayNode::SetApplyDisplayPropertiesOnBranch(bool on)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ApplyDisplayPropertiesOnBranch to " << on);
  if (this->ApplyDisplayPropertiesOnBranch == on)
  {
    return;
  }

  this->ApplyDisplayPropertiesOnBranch = on;
  this->Superclass::Modified();

  // Trigger display update of branch both when turned on and off
  this->ChildDisplayNodesModified();
}

//----------------------------------------------------------------------------
void vtkMRMLFolderDisplayNode::Modified()
{
  this->Superclass::Modified();

  // Always invoke modified on display nodes in branch (that allow overriding), because
  // visibility and opacity are applied even if ApplyDisplayPropertiesOnBranch is off
  this->ChildDisplayNodesModified();
}

//---------------------------------------------------------------------------
void vtkMRMLFolderDisplayNode::ChildDisplayNodesModified()
{
  if (!this->GetScene())
    {
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetScene());
  if (!shNode)
    {
    vtkErrorMacro("ChildDisplayNodesModified: Failed to get subject hierarchy node from scene");
    return;
    }
  vtkIdType folderItemId = shNode->GetItemByDataNode(this);
  if (!folderItemId)
    {
    return;
    }

  // Get items in branch
  std::vector<vtkIdType> childItemIDs;
  shNode->GetItemChildren(folderItemId, childItemIDs, true);

  bool batchProcessing = (childItemIDs.size() > 10);
  if (batchProcessing)
    {
    this->GetScene()->StartState(vtkMRMLScene::BatchProcessState);
    }

  std::vector<vtkIdType>::iterator childIt;
  for (childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
    {
    vtkMRMLDisplayableNode* childDisplayableNode = vtkMRMLDisplayableNode::SafeDownCast(
      shNode->GetItemDataNode(*childIt) );
    if (!childDisplayableNode)
      {
      continue;
      }
    // Trigger display update for display node of child nodes that allow overriding
    for (int i=0; i<childDisplayableNode->GetNumberOfDisplayNodes(); ++i)
      {
      vtkMRMLDisplayNode* currentDisplayNode = childDisplayableNode->GetNthDisplayNode(i);
      if (currentDisplayNode->GetFolderDisplayOverrideAllowed())
        {
        currentDisplayNode->Modified();
        }
      } // For all display nodes
    }

  if (batchProcessing)
    {
    this->GetScene()->EndState(vtkMRMLScene::BatchProcessState);
    }
}

//---------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(vtkMRMLDisplayableNode* node)
{
  if (!node || !node->GetScene() || node->GetScene()->IsImporting())
    {
    return nullptr;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(node->GetScene());
  if (!shNode)
    {
    vtkErrorWithObjectMacro(node, "GetOverridingHierarchyDisplayNode: Failed to get subject hierarchy node from current scene");
    return nullptr;
    }
  vtkIdType nodeShId = shNode->GetItemByDataNode(node);
  if (!nodeShId)
    {
    // May happen if an AddNode event is caught before SH had the chance to add the item
    return nullptr;
    }

  // Get effective display node from hierarchy:
  // go through parents and return display node of first that applies color
  vtkIdType currentParentId = nodeShId;
  bool applyDisplayPropertiesToBranch = false;
  while (!applyDisplayPropertiesToBranch)
    {
    // Get next parent
    currentParentId = shNode->GetItemParent(currentParentId);
    if (!currentParentId)
      {
      // Did not find an ancestor with apply display properties to branch flag on
      return nullptr;
      }

    vtkMRMLFolderDisplayNode* folderDisplayNode = vtkMRMLFolderDisplayNode::SafeDownCast(
      shNode->GetItemDataNode(currentParentId) );
    if (folderDisplayNode)
      {
      applyDisplayPropertiesToBranch = folderDisplayNode->GetApplyDisplayPropertiesOnBranch();
      }
    }

  return vtkMRMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(currentParentId));
}

//---------------------------------------------------------------------------
bool vtkMRMLFolderDisplayNode::GetHierarchyVisibility(vtkMRMLDisplayableNode* node)
{
  if (!node || node->GetHideFromEditors())
    {
    // Nodes that have HideFromEditors on do not appear in the hierarchy
    return true;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(node->GetScene());
  if (!shNode)
    {
    vtkErrorWithObjectMacro(node, "GetHierarchyVisibility: Failed to get subject hierarchy node from current scene");
    return true;
    }
  vtkIdType sceneItemID = shNode->GetSceneItemID();
  vtkIdType nodeShId = shNode->GetItemByDataNode(node);
  if (!nodeShId)
    {
    return true;
    }

  // Traverse all parents
  vtkIdType parentItemID = nodeShId;
  while ( (parentItemID = shNode->GetItemParent(parentItemID)) != sceneItemID ) // The double parentheses avoids a Linux build warning
    {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
    if (displayNode && displayNode->GetVisibility() == 0)
      {
      // If any of the ancestors are hidden, then the visibility of the node defined by the hierarchy is off
      return false;
      }
    }

  return true;
}

//---------------------------------------------------------------------------
double vtkMRMLFolderDisplayNode::GetHierarchyOpacity(vtkMRMLDisplayableNode* node)
{
  if (!node || node->GetHideFromEditors())
    {
    // Nodes that have HideFromEditors on do not appear in the hierarchy
    return 1.0;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(node->GetScene());
  if (!shNode)
    {
    vtkErrorWithObjectMacro(node, "GetHierarchyOpacity: Failed to get subject hierarchy node from current scene");
    return 1.0;
    }
  vtkIdType sceneItemID = shNode->GetSceneItemID();
  vtkIdType nodeShId = shNode->GetItemByDataNode(node);
  if (!nodeShId)
    {
    return 1.0;
    }

  // Traverse all parents
  double opacityProduct = 1.0;
  vtkIdType parentItemID = nodeShId;
  while ( (parentItemID = shNode->GetItemParent(parentItemID)) != sceneItemID ) // The double parentheses avoids a Linux build warning
    {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
    if (displayNode)
      {
      opacityProduct *= displayNode->GetOpacity();
      }
    }

  return opacityProduct;
}
