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
#include "vtkMRMLSubjectHierarchyNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFolderDisplayNode);

//-----------------------------------------------------------------------------
vtkMRMLFolderDisplayNode::vtkMRMLFolderDisplayNode()
  : ApplyDisplayPropertiesOnBranch(false)
{
}

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
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFolderDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(ApplyDisplayPropertiesOnBranch);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
}

//---------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(vtkMRMLDisplayableNode* node)
{
  if (!node)
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
    if (!node->GetHideFromEditors())
      {
      vtkErrorWithObjectMacro(node, "GetOverridingHierarchyDisplayNode: Failed to get subject hierarchy item for node " << node->GetName());
      }
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
    vtkErrorWithObjectMacro(node, "GetHierarchyVisibility: Failed to get subject hierarchy item from node " << node->GetName());
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
    vtkErrorWithObjectMacro(node, "GetHierarchyOpacity: Failed to get subject hierarchy item from node " << node->GetName());
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
