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
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SubjectHierarchy includes
#include <vtkSlicerSubjectHierarchyModuleLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
const char* vtkSlicerSubjectHierarchyModuleLogic::CLONED_NODE_NAME_POSTFIX = " Copy";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSubjectHierarchyModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerSubjectHierarchyModuleLogic::vtkSlicerSubjectHierarchyModuleLogic() = default;

//----------------------------------------------------------------------------
vtkSlicerSubjectHierarchyModuleLogic::~vtkSlicerSubjectHierarchyModuleLogic() = default;

//----------------------------------------------------------------------------
void vtkSlicerSubjectHierarchyModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerSubjectHierarchyModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEvents(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerSubjectHierarchyModuleLogic::UpdateFromMRMLScene()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("UpdateFromMRMLScene: Invalid MRML scene");
    return;
    }

  this->Modified();
}

//---------------------------------------------------------------------------
vtkIdType vtkSlicerSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy(
  vtkMRMLSubjectHierarchyNode* shNode, const char* patientId, const char* studyInstanceUID, const char* seriesInstanceUID )
{
  if ( !shNode || !patientId || !studyInstanceUID || !seriesInstanceUID )
    {
    vtkGenericWarningMacro("vtkSlicerSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy: Invalid input arguments");
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  vtkIdType patientItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  vtkIdType studyItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  std::vector<vtkIdType> seriesItemIDs;

  // Find referenced items
  std::vector<vtkIdType> allItemIDs;
  shNode->GetItemChildren(shNode->GetSceneItemID(), allItemIDs, true);
  for (std::vector<vtkIdType>::iterator itemIt=allItemIDs.begin(); itemIt!=allItemIDs.end(); ++itemIt)
    {
    vtkIdType currentItemID = (*itemIt);
    std::string nodeDicomUIDStr = shNode->GetItemUID(currentItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMUIDName());
    const char* nodeDicomUID = nodeDicomUIDStr.c_str();
    if (!nodeDicomUID)
      {
      // Having a UID is not mandatory
      continue;
      }
    if (!strcmp(patientId, nodeDicomUID))
      {
      patientItemID = currentItemID;
      }
    else if (!strcmp(studyInstanceUID, nodeDicomUID))
      {
      studyItemID = currentItemID;
      }
    else if (!strcmp(seriesInstanceUID, nodeDicomUID))
      {
      seriesItemIDs.push_back(currentItemID);
      }
    }

  if (seriesItemIDs.empty())
    {
    vtkErrorWithObjectMacro(shNode,
      "vtkSlicerSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy: Subject hierarchy item with DICOM UID '"
      << seriesInstanceUID << "' cannot be found");
    return 0;
    }

  // Create patient and study nodes if they do not exist yet
  if (!patientItemID)
    {
    // This temporary name is updated with correct one specified in the DICOM plugin after calling this function
    std::string name = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNewItemNamePrefix()
      + vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient();
    name = shNode->GenerateUniqueItemName(name);

    patientItemID = shNode->CreateSubjectItem(shNode->GetSceneItemID(), name);
    shNode->SetItemUID(patientItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMUIDName(), patientId);
    shNode->SetItemOwnerPluginName(patientItemID, "DICOM");
    }

  if (!studyItemID)
    {
    // This temporary name is updated with correct one specified in the DICOM plugin after calling this function
    std::string name = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNewItemNamePrefix()
      + vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy();
    name = shNode->GenerateUniqueItemName(name);

    studyItemID = shNode->CreateStudyItem(patientItemID, name);
    shNode->SetItemUID(studyItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMUIDName(), studyInstanceUID);
    shNode->SetItemOwnerPluginName(studyItemID, "DICOM");
    }

  // In some cases there might be multiple subject hierarchy items for the same DICOM series,
  // for example if a series contains instances that load to different node types that cannot
  // be simply added under one series folder item. This can happen if for one type the item
  // corresponds to the series, but in the other to the instances.
  for (std::vector<vtkIdType>::iterator seriesIt = seriesItemIDs.begin(); seriesIt != seriesItemIDs.end(); ++seriesIt)
  {
    vtkIdType currentSeriesID = (*seriesIt);
    shNode->SetItemParent(currentSeriesID, studyItemID);
  }

  if (seriesItemIDs.size() > 1)
  {
    vtkDebugWithObjectMacro(shNode,
      "vtkSlicerSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy: DICOM UID '"
      << seriesInstanceUID << "' corresponds to multiple series subject hierarchy nodes, but only the first one is returned");
  }

  return *(seriesItemIDs.begin());
}

//---------------------------------------------------------------------------
vtkIdType vtkSlicerSubjectHierarchyModuleLogic::AreItemsInSameBranch(
    vtkMRMLSubjectHierarchyNode* shNode, vtkIdType item1, vtkIdType item2, std::string lowestCommonLevel )
{
  if (!shNode)
    {
    vtkGenericWarningMacro("vtkSlicerSubjectHierarchyModuleLogic::AreItemsInSameBranch: Invalid subject hierarchy node given");
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  if (!item1 || !item2)
    {
    vtkErrorWithObjectMacro(shNode, "vtkSlicerSubjectHierarchyModuleLogic::AreItemsInSameBranch: Invalid input items");
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  if (lowestCommonLevel.empty())
    {
    vtkErrorWithObjectMacro(shNode, "vtkSlicerSubjectHierarchyModuleLogic::AreItemsInSameBranch: Invalid lowest common level");
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  // Walk the hierarchy up until we reach the lowest common level
  vtkIdType ancestor1 = item1;
  while (true)
    {
    ancestor1 = shNode->GetItemParent(ancestor1);
    if (!ancestor1 || ancestor1 == shNode->GetSceneItemID())
      {
      vtkDebugWithObjectMacro(shNode, "Item ('" << shNode->GetItemName(item1) << "') has no ancestor with level '" << lowestCommonLevel << "'");
      ancestor1 = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
      break;
      }
    std::string item1Level = shNode->GetItemLevel(ancestor1);
    if (item1Level.empty())
      {
      vtkDebugWithObjectMacro(shNode, "Item ('" << shNode->GetItemName(ancestor1) << "') has invalid level property");
      break;
      }
    if (!item1Level.compare(lowestCommonLevel))
      {
      break;
      }
    }

  vtkIdType ancestor2 = item2;
  while (true)
    {
    ancestor2 = shNode->GetItemParent(ancestor2);
    if (!ancestor2 || ancestor2 == shNode->GetSceneItemID())
      {
      vtkDebugWithObjectMacro(shNode, "Item ('" << shNode->GetItemName(item2) << "') has no ancestor with level '" << lowestCommonLevel << "'");
      ancestor2 = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
      break;
      }
    std::string item2Level = shNode->GetItemLevel(ancestor2);
    if (item2Level.empty())
      {
      vtkDebugWithObjectMacro(shNode, "Item ('" << shNode->GetItemName(ancestor2) << "') has invalid level property");
      break;
      }
    if (!item2Level.compare(lowestCommonLevel))
      {
      break;
      }
    }

  return (ancestor1 == ancestor2 ? ancestor1 : vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);
}

//---------------------------------------------------------------------------
vtkIdType vtkSlicerSubjectHierarchyModuleLogic::AreNodesInSameBranch(vtkMRMLNode* node1, vtkMRMLNode* node2, std::string lowestCommonLevel)
{
  if (!node1 || !node2 || !node1->GetScene() || node1->GetScene() != node2->GetScene())
    {
    vtkGenericWarningMacro("vtkSlicerSubjectHierarchyModuleLogic::AreNodesInSameBranch: Invalid input nodes or they are not in the same scene");
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(node1->GetScene());
  vtkIdType item1 = shNode->GetItemByDataNode(node1);
  vtkIdType item2 = shNode->GetItemByDataNode(node2);

  return vtkSlicerSubjectHierarchyModuleLogic::AreItemsInSameBranch(shNode, item1, item2, lowestCommonLevel);
}

//---------------------------------------------------------------------------
bool vtkSlicerSubjectHierarchyModuleLogic::IsPatientTag(std::string tagName)
{
  std::vector<std::string> patientTagNames = vtkMRMLSubjectHierarchyConstants::GetDICOMPatientTagNames();
  for ( std::vector<std::string>::iterator patientTagIt = patientTagNames.begin();
    patientTagIt != patientTagNames.end(); ++patientTagIt )
    {
    if (!tagName.compare(*patientTagIt))
      {
      // Argument was found in patient tag names list, so given tag is a patient tag
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerSubjectHierarchyModuleLogic::IsStudyTag(std::string tagName)
{
  std::vector<std::string> studyTagNames = vtkMRMLSubjectHierarchyConstants::GetDICOMStudyTagNames();
  for ( std::vector<std::string>::iterator studyTagIt = studyTagNames.begin();
    studyTagIt != studyTagNames.end(); ++studyTagIt )
    {
    if (!tagName.compare(*studyTagIt))
      {
      // Argument was found in study tag names list, so given tag is a study tag
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkSlicerSubjectHierarchyModuleLogic::TransformBranch(
  vtkMRMLSubjectHierarchyNode* shNode, vtkIdType itemID, vtkMRMLTransformNode* transformNode, bool hardenExistingTransforms/*=true*/)
{
  if (!shNode)
    {
    vtkGenericWarningMacro("vtkSlicerSubjectHierarchyModuleLogic::TransformBranch: Invalid subject hierarchy node");
    return;
    }

  // Get all associated data nodes from children nodes (and itself)
  std::vector<vtkIdType> childIDs;
  shNode->GetItemChildren(itemID, childIDs, true);
  childIDs.push_back(itemID);

  for (std::vector<vtkIdType>::iterator childIt=childIDs.begin(); childIt!=childIDs.end(); ++childIt)
    {
    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(
      shNode->GetItemDataNode(*childIt) );
    if (!transformableNode)
      {
      continue;
      }
    if (transformableNode == transformNode)
      {
      // Transform node cannot be transformed by itself
      continue;
      }

    vtkMRMLTransformNode* parentTransformNode = transformableNode->GetParentTransformNode();
    if (parentTransformNode)
      {
      // Do nothing if the parent transform matches the specified transform to apply
      if (parentTransformNode == transformNode)
        {
        //vtkDebugMacro("TransformBranch: Specified transform " << transformNode->GetName() << " already applied on data node belonging to subject hierarchy node " << this->Name);
        continue;
        }
      // Harden existing parent transform if this option was chosen
      if (hardenExistingTransforms)
        {
        //vtkDebugMacro("TransformBranch: Hardening transform " << transformNode->GetName() << " on node " << transformableNode->GetName());
        transformableNode->HardenTransform();
        }
      }

    // Apply the transform
    transformableNode->SetAndObserveTransformNodeID(transformNode ? transformNode->GetID() : nullptr);

    // Trigger update by invoking the modified event for the subject hierarchy item
    shNode->ItemModified(*childIt);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSubjectHierarchyModuleLogic::HardenTransformOnBranch(vtkMRMLSubjectHierarchyNode* shNode, vtkIdType itemID)
{
  if (!shNode)
    {
    vtkGenericWarningMacro("vtkSlicerSubjectHierarchyModuleLogic::HardenTransformOnBranch: Invalid subject hierarchy node");
    return;
    }

  // Get all associated data nodes from children nodes (and itself)
  std::vector<vtkIdType> childIDs;
  shNode->GetItemChildren(itemID, childIDs, true);
  childIDs.push_back(itemID);

  for (std::vector<vtkIdType>::iterator childIt=childIDs.begin(); childIt!=childIDs.end(); ++childIt)
    {
    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(
      shNode->GetItemDataNode(*childIt) );
    if (!transformableNode)
      {
      continue;
      }
    transformableNode->HardenTransform();

    // Trigger update by invoking the modified event for the subject hierarchy item
    shNode->ItemModified(*childIt);
    }
}

//---------------------------------------------------------------------------
vtkIdType vtkSlicerSubjectHierarchyModuleLogic::CloneSubjectHierarchyItem(
  vtkMRMLSubjectHierarchyNode* shNode, vtkIdType itemID, const char* name/*=nullptr*/)
{
  if (!shNode)
    {
    vtkGenericWarningMacro("vtkSlicerSubjectHierarchyModuleLogic::CloneSubjectHierarchyItem: Invalid subject hierarchy node");
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  vtkMRMLScene* scene = shNode->GetScene();
  if (!scene)
    {
    vtkErrorWithObjectMacro( shNode, "vtkSlicerSubjectHierarchyModuleLogic::CloneSubjectHierarchyItem: "
      "Invalid scene for subject hierarchy node " << shNode->GetName() );
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  vtkIdType clonedShItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  vtkMRMLNode* associatedDataNode = shNode->GetItemDataNode(itemID);
  if (associatedDataNode)
    {
    // Create data node clone
    vtkSmartPointer<vtkMRMLNode> clonedDataNode;
    clonedDataNode.TakeReference(scene->CreateNodeByClass(associatedDataNode->GetClassName()));
    std::string clonedDataNodeName = ( name ? std::string(name) : std::string(associatedDataNode->GetName()) + std::string(CLONED_NODE_NAME_POSTFIX) );
    scene->AddNode(clonedDataNode);

    // Clone display node
    vtkSmartPointer<vtkMRMLDisplayNode> clonedDisplayNode;
    vtkMRMLDisplayableNode* displayableDataNode = vtkMRMLDisplayableNode::SafeDownCast(associatedDataNode);
    if (displayableDataNode && displayableDataNode->GetDisplayNode())
      {
      // If display node was automatically created by the specific module logic when the data node was added to the scene, then do not create it
      vtkMRMLDisplayableNode* clonedDisplayableDataNode = vtkMRMLDisplayableNode::SafeDownCast(clonedDataNode);
      if (clonedDisplayableDataNode->GetDisplayNode())
        {
        clonedDisplayNode = clonedDisplayableDataNode->GetDisplayNode();
        }
      else
        {
        clonedDisplayNode = vtkSmartPointer<vtkMRMLDisplayNode>::Take( vtkMRMLDisplayNode::SafeDownCast(
          scene->CreateNodeByClass(displayableDataNode->GetDisplayNode()->GetClassName()) ) );
        clonedDisplayNode->Copy(displayableDataNode->GetDisplayNode());
        std::string clonedDisplayNodeName = clonedDataNodeName + "_Display";
        clonedDisplayNode->SetName(clonedDisplayNodeName.c_str());
        scene->AddNode(clonedDisplayNode);
        clonedDisplayableDataNode->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());
        }
      }

    // Clone storage node
    vtkSmartPointer<vtkMRMLStorageNode> clonedStorageNode;
    vtkMRMLStorableNode* storableDataNode = vtkMRMLStorableNode::SafeDownCast(associatedDataNode);
    if (storableDataNode && storableDataNode->GetStorageNode())
      {
      // If storage node was automatically created by the specific module logic when the data node was added to the scene, then do not create it
      vtkMRMLStorableNode* clonedStorableDataNode = vtkMRMLStorableNode::SafeDownCast(clonedDataNode);
      if (clonedStorableDataNode->GetStorageNode())
        {
        clonedStorageNode = clonedStorableDataNode->GetStorageNode();
        }
      else
        {
        clonedStorageNode = vtkSmartPointer<vtkMRMLStorageNode>::Take( vtkMRMLStorageNode::SafeDownCast(
          scene->CreateNodeByClass(storableDataNode->GetStorageNode()->GetClassName()) ) );
        clonedStorageNode->Copy(storableDataNode->GetStorageNode());
        if (storableDataNode->GetStorageNode()->GetFileName())
          {
          std::string clonedStorageNodeFileName = std::string(storableDataNode->GetStorageNode()->GetFileName()) + std::string(CLONED_NODE_NAME_POSTFIX);
          clonedStorageNode->SetFileName(clonedStorageNodeFileName.c_str());
          }
        scene->AddNode(clonedStorageNode);
        clonedStorableDataNode->SetAndObserveStorageNodeID(clonedStorageNode->GetID());
        }
      }

    // Copy data node
    // Display and storage nodes might be involved in the copy process, so they are needed to be set up before the copy operation
    clonedDataNode->Copy(associatedDataNode);
    clonedDataNode->SetName(clonedDataNodeName.c_str());
    // Copy overwrites display and storage node references too, need to restore
    if (clonedDisplayNode.GetPointer())
      {
      vtkMRMLDisplayableNode::SafeDownCast(clonedDataNode)->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());
      }
    if (clonedStorageNode.GetPointer())
      {
      vtkMRMLStorableNode::SafeDownCast(clonedDataNode)->SetAndObserveStorageNodeID(clonedStorageNode->GetID());
      }
    // Trigger display update (needed to invoke update of transforms in displayable managers)
    vtkMRMLTransformableNode* transformableClonedNode = vtkMRMLTransformableNode::SafeDownCast(clonedDataNode);
    if (transformableClonedNode && transformableClonedNode->GetParentTransformNode())
    {
      transformableClonedNode->GetParentTransformNode()->Modified();
    }

    // Put data node in the same subject hierarchy branch as current node
    clonedShItemID = shNode->GetItemByDataNode(clonedDataNode);
    shNode->SetItemParent(clonedShItemID, shNode->GetItemParent(itemID));

    // Make sure the most suitable plugin owns the item (it may depend on attributes set by Copy, and place in branch)
    shNode->RequestOwnerPluginSearch(clonedShItemID);
    // Trigger update by invoking the modified event for the subject hierarchy item
    shNode->ItemModified(clonedShItemID);
    }
  else // No associated node
    {
    std::string clonedItemName = ( name ? std::string(name) : std::string(shNode->GetItemName(itemID)) + std::string(CLONED_NODE_NAME_POSTFIX) );
    clonedShItemID = shNode->CreateHierarchyItem(shNode->GetItemParent(itemID), clonedItemName, shNode->GetItemLevel(itemID));
    }

  return clonedShItemID;
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* vtkSlicerSubjectHierarchyModuleLogic::GetSubjectHierarchyNode()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkGenericWarningMacro("vtkSlicerSubjectHierarchyModuleLogic::GetSubjectHierarchyNode: Invalid scene");
    return nullptr;
    }

  return vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
}
