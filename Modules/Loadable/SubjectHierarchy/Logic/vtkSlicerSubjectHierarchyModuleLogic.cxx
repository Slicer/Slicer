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
#include "vtkSlicerSubjectHierarchyModuleLogic.h"
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// Slicer Libs includes
#include <vtkSlicerTransformLogic.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSubjectHierarchyModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerSubjectHierarchyModuleLogic::vtkSlicerSubjectHierarchyModuleLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerSubjectHierarchyModuleLogic::~vtkSlicerSubjectHierarchyModuleLogic()
{
}

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

//-----------------------------------------------------------------------------
void vtkSlicerSubjectHierarchyModuleLogic::RegisterNodes()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene!");
    return;
    }

  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLSubjectHierarchyNode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerSubjectHierarchyModuleLogic::UpdateFromMRMLScene()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("UpdateFromMRMLScene: Invalid MRML scene!");
    return;
    }

  this->Modified();
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* vtkSlicerSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy(
  vtkMRMLScene *scene, const char* patientId, const char* studyInstanceUID, const char* seriesInstanceUID )
{
  if ( !scene || !patientId || !studyInstanceUID || !seriesInstanceUID )
    {
    std::cerr << "vtkSlicerSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy: Invalid input arguments!" << std::endl;
    return NULL;
    }

  vtkMRMLSubjectHierarchyNode* patientNode = NULL;
  vtkMRMLSubjectHierarchyNode* studyNode = NULL;
  vtkMRMLSubjectHierarchyNode* seriesNode = NULL;

  std::vector<vtkMRMLNode*> subjectHierarchyNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkMRMLHierarchyNode", subjectHierarchyNodes);

  // Find referenced nodes
  for (unsigned int i=0; i<numberOfNodes; i++)
    {
    vtkMRMLSubjectHierarchyNode *node = vtkMRMLSubjectHierarchyNode::SafeDownCast(subjectHierarchyNodes[i]);
    if ( node && node->IsA("vtkMRMLSubjectHierarchyNode") )
      {
      std::string nodeDicomUIDStr = node->GetUID(vtkMRMLSubjectHierarchyConstants::GetDICOMUIDName());
      const char* nodeDicomUID = nodeDicomUIDStr.c_str();
      if (!nodeDicomUID)
        {
        // Having a UID is not mandatory
        continue;
        }
      if (!strcmp(patientId, nodeDicomUID))
        {
        patientNode = node;
        }
      else if (!strcmp(studyInstanceUID, nodeDicomUID))
        {
        studyNode = node;
        }
      else if (!strcmp(seriesInstanceUID, nodeDicomUID))
        {
        seriesNode = node;
        }
      }
    }

  if (!seriesNode)
    {
    vtkErrorWithObjectMacro(scene,
      "vtkSlicerSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy: Subject hierarchy node with DICOM UID '"
      << seriesInstanceUID << "' cannot be found!");
    return NULL;
    }

  // Create patient and study nodes if they do not exist yet
  if (!patientNode)
    {
    patientNode = vtkMRMLSubjectHierarchyNode::New();
    patientNode->SetLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient());
    patientNode->AddUID(vtkMRMLSubjectHierarchyConstants::GetDICOMUIDName(), patientId);
    patientNode->SetOwnerPluginName("DICOM");
    scene->AddNode(patientNode);
    patientNode->Delete(); // Return ownership to the scene only
    }

  if (!studyNode)
    {
    studyNode = vtkMRMLSubjectHierarchyNode::New();
    studyNode->SetLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy());
    studyNode->AddUID(vtkMRMLSubjectHierarchyConstants::GetDICOMUIDName(), studyInstanceUID);
    studyNode->SetOwnerPluginName("DICOM");
    studyNode->SetParentNodeID(patientNode->GetID());
    scene->AddNode(studyNode);
    studyNode->Delete(); // Return ownership to the scene only
    }

  seriesNode->SetParentNodeID(studyNode->GetID());

  return seriesNode;
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* vtkSlicerSubjectHierarchyModuleLogic::AreNodesInSameBranch(vtkMRMLNode* node1, vtkMRMLNode* node2,
                                                                const char* lowestCommonLevel)
{
  if ( !node1 || !node2 || node1->GetScene() != node2->GetScene() )
    {
    std::cerr << "vtkSlicerSubjectHierarchyModuleLogic::AreNodesInSameBranch: Invalid input nodes or they are not in the same scene!" << std::endl;
    return NULL;
    }

  if (!lowestCommonLevel)
    {
    vtkErrorWithObjectMacro(node1, "vtkSlicerSubjectHierarchyModuleLogic::AreNodesInSameBranch: Invalid lowest common level!");
    return NULL;
    }

  // If not hierarchy nodes, get the associated subject hierarchy node
  vtkMRMLSubjectHierarchyNode* hierarchyNode1 = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(node1);
  vtkMRMLSubjectHierarchyNode* hierarchyNode2 = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(node2);

  // Check if valid nodes are found
  if (!hierarchyNode1 || !hierarchyNode2)
    {
    return NULL;
    }

  // Walk the hierarchy up until we reach the lowest common level
  while (true)
    {
    hierarchyNode1 = vtkMRMLSubjectHierarchyNode::SafeDownCast(hierarchyNode1->GetParentNode());
    if (!hierarchyNode1)
      {
      vtkDebugWithObjectMacro(node1, "Node ('" << node1->GetName() << "') has no ancestor with DICOM level '" << lowestCommonLevel << "'");
      hierarchyNode1 = NULL;
      break;
      }
    const char* node1Level = hierarchyNode1->GetLevel();
    if (!node1Level)
      {
      vtkDebugWithObjectMacro(node1, "Node ('" << node1->GetName() << "') has no DICOM level '" << lowestCommonLevel << "'");
      hierarchyNode1 = NULL;
      break;
      }
    if (!strcmp(node1Level, lowestCommonLevel))
      {
      break;
      }
    }

  while (true)
    {
    hierarchyNode2 = vtkMRMLSubjectHierarchyNode::SafeDownCast(hierarchyNode2->GetParentNode());
    if (!hierarchyNode2)
      {
      vtkDebugWithObjectMacro(node2, "Node ('" << node2->GetName() << "') has no ancestor with DICOM level '" << lowestCommonLevel << "'");
      hierarchyNode2 = NULL;
      break;
      }
    const char* node2Level = hierarchyNode2->GetLevel();
    if (!node2Level)
      {
      vtkDebugWithObjectMacro(node2, "Node ('" << node2->GetName() << "') has no DICOM level '" << lowestCommonLevel << "'");
      hierarchyNode2 = NULL;
      break;
      }
    if (!strcmp(node2Level, lowestCommonLevel))
      {
      break;
      }
    }

  return (hierarchyNode1 == hierarchyNode2 ? hierarchyNode1 : NULL);
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
void vtkSlicerSubjectHierarchyModuleLogic::TransformBranch(vtkMRMLSubjectHierarchyNode* node, vtkMRMLTransformNode* transformNode, bool hardenExistingTransforms/*=true*/)
{
  if (!node)
    {
    std::cerr << "vtkSlicerSubjectHierarchyModuleLogic::TransformBranch: Invalid input node!" << std::endl;
    return;
    }

  // Get all associated data nodes from children nodes (and itself)
  vtkNew<vtkCollection> childTransformableNodes;
  node->GetAssociatedChildrenNodes(childTransformableNodes.GetPointer(), "vtkMRMLTransformableNode");

  childTransformableNodes->InitTraversal();
  for (int childNodeIndex = 0; childNodeIndex < childTransformableNodes->GetNumberOfItems(); ++childNodeIndex)
    {
    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(
      childTransformableNodes->GetItemAsObject(childNodeIndex) );
    if (!transformableNode)
      {
      vtkWarningWithObjectMacro(node, "TransformBranch: Non-transformable node found in a collection of transformable nodes!");
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
        vtkSlicerTransformLogic::hardenTransform(transformableNode);
        }
      }

    // Apply the transform
    transformableNode->SetAndObserveTransformNodeID(transformNode ? transformNode->GetID() : NULL);

    // Trigger update by setting the modified flag on the subject hierarchy node
    vtkMRMLSubjectHierarchyNode* subjectHierarchyNode =
      vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(transformableNode);
    if (!subjectHierarchyNode)
      {
      vtkErrorWithObjectMacro(node, "TransformBranch: Unable to find subject hierarchy node for transformable node " << transformableNode->GetName());
      continue;
      }
    subjectHierarchyNode->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSubjectHierarchyModuleLogic::HardenTransformOnBranch(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node)
    {
    std::cerr << "vtkSlicerSubjectHierarchyModuleLogic::TransformBranch: Invalid input node!" << std::endl;
    return;
    }

  // Get all associated data nodes from children nodes (and itself)
  vtkSmartPointer<vtkCollection> childTransformableNodes = vtkSmartPointer<vtkCollection>::New();
  node->GetAssociatedChildrenNodes(childTransformableNodes, "vtkMRMLTransformableNode");
  childTransformableNodes->InitTraversal();

  for (int childNodeIndex=0; childNodeIndex<childTransformableNodes->GetNumberOfItems(); ++childNodeIndex)
    {
    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(
      childTransformableNodes->GetItemAsObject(childNodeIndex) );
    vtkSlicerTransformLogic::hardenTransform(transformableNode);

    // Trigger update by setting the modified flag on the subject hierarchy node
    vtkMRMLSubjectHierarchyNode* subjectHierarchyNode =
      vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(transformableNode);
    if (!subjectHierarchyNode)
      {
      vtkErrorWithObjectMacro(node, "TransformBranch: Unable to find subject hierarchy node for transformable node " << transformableNode->GetName());
      continue;
      }
    subjectHierarchyNode->Modified();
    }
}
