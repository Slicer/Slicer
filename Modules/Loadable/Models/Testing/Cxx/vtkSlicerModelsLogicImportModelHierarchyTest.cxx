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

==============================================================================*/

// Models logic
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>

namespace
{
//-----------------------------------------------------------------------------
// Create subject hierarchy items for the imported model nodes (in the application this is done by the
// subject hierarchy plugin logic, which is not available in this test). Invoked at the end of the import,
// before the models logic converts the model hierarchy nodes.
void addSubjectHierarchyItems(vtkObject* caller, unsigned long vtkNotUsed(eid), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(caller);
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene);
  std::vector<vtkMRMLNode*> modelNodes;
  scene->GetNodesByClass("vtkMRMLModelNode", modelNodes);
  for (vtkMRMLNode* modelNode : modelNodes)
  {
    if (!shNode->GetItemByDataNode(modelNode))
    {
      shNode->CreateItem(shNode->GetSceneItemID(), modelNode);
    }
  }
}
} // end of anonymous namespace

//-----------------------------------------------------------------------------
// Test conversion of legacy model hierarchy nodes to subject hierarchy folders when a scene is imported
int vtkSlicerModelsLogicImportModelHierarchyTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerModelsLogic> modelsLogic;
  modelsLogic->SetMRMLScene(scene);
  vtkNew<vtkCallbackCommand> addItemsCommand;
  addItemsCommand->SetCallback(addSubjectHierarchyItems);
  scene->AddObserver(vtkMRMLScene::EndImportEvent, addItemsCommand, 10.0);

  // Legacy scene with two model hierarchies, each containing a model:
  // - "Collapsed" is not expanded, which means that its display properties (color) are applied to all its children
  // - "Expanded" is expanded, each child is displayed with its own display properties
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString("<MRML version=\"Slicer4.4.0\" userTags=\"\">"
                           " <ModelDisplay id=\"vtkMRMLModelDisplayNode1\" name=\"CollapsedDisplay\" color=\"0 0 1\""
                           " visibility=\"false\" opacity=\"0.5\"></ModelDisplay>"
                           " <ModelHierarchy id=\"vtkMRMLModelHierarchyNode1\" name=\"Collapsed\""
                           " displayNodeID=\"vtkMRMLModelDisplayNode1\" expanded=\"false\"></ModelHierarchy>"
                           " <ModelDisplay id=\"vtkMRMLModelDisplayNode2\" name=\"Model1Display\" color=\"1 0 0\"></ModelDisplay>"
                           " <Model id=\"vtkMRMLModelNode1\" name=\"Model1\" displayNodeRef=\"vtkMRMLModelDisplayNode2\"></Model>"
                           " <ModelHierarchy id=\"vtkMRMLModelHierarchyNode2\" name=\"Model1Hierarchy\" parentNodeRef=\"vtkMRMLModelHierarchyNode1\""
                           "  associatedNodeRef=\"vtkMRMLModelNode1\" expanded=\"true\"></ModelHierarchy>"
                           " <ModelDisplay id=\"vtkMRMLModelDisplayNode3\" name=\"ExpandedDisplay\" color=\"0 1 0\""
                           " visibility=\"false\" opacity=\"0.5\"></ModelDisplay>"
                           " <ModelHierarchy id=\"vtkMRMLModelHierarchyNode3\" name=\"Expanded\""
                           " displayNodeID=\"vtkMRMLModelDisplayNode3\" expanded=\"true\"></ModelHierarchy>"
                           " <ModelDisplay id=\"vtkMRMLModelDisplayNode4\" name=\"Model2Display\" color=\"1 0 0\"></ModelDisplay>"
                           " <Model id=\"vtkMRMLModelNode2\" name=\"Model2\" displayNodeRef=\"vtkMRMLModelDisplayNode4\"></Model>"
                           " <ModelHierarchy id=\"vtkMRMLModelHierarchyNode4\" name=\"Model2Hierarchy\" parentNodeRef=\"vtkMRMLModelHierarchyNode3\""
                           "  associatedNodeRef=\"vtkMRMLModelNode2\" expanded=\"true\"></ModelHierarchy>"
                           "</MRML>");
  CHECK_BOOL(scene->Import() != 0, true);

  // Model hierarchy nodes and their display nodes are replaced by subject hierarchy folders with folder display nodes
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode"), 0);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelDisplayNode"), 2);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLFolderDisplayNode"), 2);
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  CHECK_NOT_NULL(shNode);

  vtkMRMLFolderDisplayNode* collapsedFolderDisplayNode = vtkMRMLFolderDisplayNode::SafeDownCast(scene->GetFirstNodeByName("Collapsed"));
  CHECK_NOT_NULL(collapsedFolderDisplayNode);
  CHECK_BOOL(collapsedFolderDisplayNode->GetApplyDisplayPropertiesOnBranch(), true);
  CHECK_DOUBLE(collapsedFolderDisplayNode->GetColor()[0], 0.0);
  CHECK_DOUBLE(collapsedFolderDisplayNode->GetColor()[2], 1.0);
  // Visibility and opacity of a collapsed hierarchy are applied to the branch
  CHECK_INT(collapsedFolderDisplayNode->GetVisibility(), 0);
  CHECK_DOUBLE(collapsedFolderDisplayNode->GetOpacity(), 0.5);
  CHECK_STD_STRING(collapsedFolderDisplayNode->GetAttribute(vtkSlicerModelsLogic::GetModelHierarchyNodeIDAttributeName()), "vtkMRMLModelHierarchyNode1");

  vtkMRMLFolderDisplayNode* expandedFolderDisplayNode = vtkMRMLFolderDisplayNode::SafeDownCast(scene->GetFirstNodeByName("Expanded"));
  CHECK_NOT_NULL(expandedFolderDisplayNode);
  CHECK_BOOL(expandedFolderDisplayNode->GetApplyDisplayPropertiesOnBranch(), false);
  CHECK_DOUBLE(expandedFolderDisplayNode->GetColor()[1], 1.0);
  // Display node of an expanded hierarchy had no effect on the children, so the folder must not hide or fade the branch
  CHECK_INT(expandedFolderDisplayNode->GetVisibility(), 1);
  CHECK_DOUBLE(expandedFolderDisplayNode->GetOpacity(), 1.0);
  CHECK_STD_STRING(expandedFolderDisplayNode->GetAttribute(vtkSlicerModelsLogic::GetModelHierarchyNodeIDAttributeName()), "vtkMRMLModelHierarchyNode3");

  // Models are moved under the folders
  vtkMRMLModelNode* model1 = vtkMRMLModelNode::SafeDownCast(scene->GetFirstNodeByName("Model1"));
  vtkMRMLModelNode* model2 = vtkMRMLModelNode::SafeDownCast(scene->GetFirstNodeByName("Model2"));
  CHECK_NOT_NULL(model1);
  CHECK_NOT_NULL(model2);
  CHECK_INT(shNode->GetItemParent(shNode->GetItemByDataNode(model1)), shNode->GetItemByDataNode(collapsedFolderDisplayNode));
  CHECK_INT(shNode->GetItemParent(shNode->GetItemByDataNode(model2)), shNode->GetItemByDataNode(expandedFolderDisplayNode));

  // The collapsed folder overrides the display properties of its children, the expanded folder does not
  CHECK_POINTER(vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(model1), collapsedFolderDisplayNode);
  CHECK_NULL(vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(model2));

  return EXIT_SUCCESS;
}
