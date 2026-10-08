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

// Tests conversion of legacy scene view nodes (vtkMRMLSceneViewNode, storing a snapshot of the scene)
// to scene views stored in sequence browser nodes.

// MRML includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLFolderDisplayNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// Sequences logic includes
#include <vtkSlicerSequencesLogic.h>

// SceneView logic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

namespace
{
int testConvertLegacySceneView(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int testImportLegacySceneViewIntoPopulatedScene(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int testConvertLegacySceneViewWithModelHierarchy(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);

//---------------------------------------------------------------------------
void setupLogic(vtkMRMLScene* scene, vtkMRMLApplicationLogic* appLogic, vtkSlicerSequencesLogic* sequencesLogic, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  appLogic->SetMRMLScene(scene);
  sequencesLogic->SetMRMLScene(scene);
  sequencesLogic->SetMRMLApplicationLogic(appLogic);
  appLogic->SetModuleLogic("Sequences", sequencesLogic);
  sceneViewLogic->SetMRMLScene(scene);
  sceneViewLogic->SetMRMLApplicationLogic(appLogic);
  appLogic->SetModuleLogic("SceneViews", sceneViewLogic);
}

//---------------------------------------------------------------------------
// Add a model with a display node, a 3D view with a camera, and a slice view to the scene
void populateScene(vtkMRMLScene* scene)
{
  vtkNew<vtkMRMLModelNode> modelNode;
  modelNode->SetName("Model");
  scene->AddNode(modelNode);
  modelNode->CreateDefaultDisplayNodes();
  modelNode->GetDisplayNode()->SetVisibility(1);
  modelNode->GetDisplayNode()->SetColor(1.0, 0.0, 0.0);

  vtkNew<vtkMRMLViewNode> viewNode;
  viewNode->SetLayoutName("1");
  scene->AddNode(viewNode);

  vtkNew<vtkMRMLCameraNode> cameraNode;
  cameraNode->SetLayoutName("1");
  cameraNode->SetPosition(10.0, 20.0, 30.0);
  scene->AddNode(cameraNode);

  // Slice orientation presets are needed for writing and reading the slice node orientation
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene);
  vtkSmartPointer<vtkMRMLSliceNode> sliceNode = //
    vtkSmartPointer<vtkMRMLSliceNode>::Take(vtkMRMLSliceNode::SafeDownCast(scene->CreateNodeByClass("vtkMRMLSliceNode")));
  sliceNode->SetLayoutName("Red");
  sliceNode->SetOrientationToAxial();
  sliceNode->SetSliceVisible(1);
  scene->AddNode(sliceNode);
}

//---------------------------------------------------------------------------
// Create a legacy scene view node that stores a snapshot of the current scene
vtkMRMLSceneViewNode* addLegacySceneView(vtkMRMLScene* scene, const char* name)
{
  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  sceneViewNode->SetName(name);
  scene->AddNode(sceneViewNode);
  sceneViewNode->StoreScene();
  return sceneViewNode;
}

//---------------------------------------------------------------------------
// Change all the properties that are stored in the scene view
void modifyScene(vtkMRMLScene* scene)
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->GetFirstNodeByName("Model"));
  modelNode->GetDisplayNode()->SetVisibility(0);
  modelNode->GetDisplayNode()->SetColor(0.0, 1.0, 0.0);
  vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLCameraNode"));
  cameraNode->SetPosition(100.0, 200.0, 300.0);
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLSliceNode"));
  sliceNode->SetSliceVisible(0);
}

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkSceneViewConvertLegacySceneViewTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLApplicationLogic> appLogic;
  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  vtkNew<vtkSlicerSceneViewsModuleLogic> sceneViewLogic;
  setupLogic(scene, appLogic, sequencesLogic, sceneViewLogic);

  CHECK_EXIT_SUCCESS(testConvertLegacySceneView(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(testImportLegacySceneViewIntoPopulatedScene(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(testConvertLegacySceneViewWithModelHierarchy(scene, sceneViewLogic));
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
int testConvertLegacySceneView(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  scene->Clear(1);
  populateScene(scene);
  addLegacySceneView(scene, "LegacySceneView1");
  modifyScene(scene);
  addLegacySceneView(scene, "LegacySceneView2");

  const int modelDisplayNodeCount = scene->GetNumberOfNodesByClass("vtkMRMLModelDisplayNode");
  const int cameraNodeCount = scene->GetNumberOfNodesByClass("vtkMRMLCameraNode");
  const int sliceNodeCount = scene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  const int viewNodeCount = scene->GetNumberOfNodesByClass("vtkMRMLViewNode");
  CHECK_INT(modelDisplayNodeCount, 1);
  CHECK_INT(cameraNodeCount, 1);

  sceneViewLogic->ConvertSceneViewNodesToSequenceBrowserNodes(scene);

  // Legacy scene view nodes are removed, scene views are available in the logic
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLSceneViewNode"), 0);
  CHECK_INT(sceneViewLogic->GetNumberOfSceneViews(), 2);
  CHECK_STD_STRING(sceneViewLogic->GetNthSceneViewName(0), "LegacySceneView1");
  CHECK_STD_STRING(sceneViewLogic->GetNthSceneViewName(1), "LegacySceneView2");

  // Existing display and view nodes are used as proxy nodes, no duplicates are created
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelDisplayNode"), modelDisplayNodeCount);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLCameraNode"), cameraNodeCount);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLSliceNode"), sliceNodeCount);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLViewNode"), viewNodeCount);

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->GetFirstNodeByName("Model"));
  vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLCameraNode"));
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLSliceNode"));
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLViewNode"));

  // Display, camera, slice, and view nodes are all stored in the scene views
  CHECK_NOT_NULL(sceneViewLogic->GetNthSceneViewDataNode(0, modelNode->GetDisplayNode()));
  CHECK_NOT_NULL(sceneViewLogic->GetNthSceneViewDataNode(0, cameraNode));
  CHECK_NOT_NULL(sceneViewLogic->GetNthSceneViewDataNode(0, sliceNode));
  CHECK_NOT_NULL(sceneViewLogic->GetNthSceneViewDataNode(0, viewNode));
  // Data nodes (model) are not stored
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkMRMLNode* modelNodeDataNode = sceneViewLogic->GetNthSceneViewDataNode(0, modelNode);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_NULL(modelNodeDataNode);

  // Restoring the first scene view restores the original state
  CHECK_BOOL(sceneViewLogic->RestoreSceneView(0), true);
  CHECK_INT(modelNode->GetDisplayNode()->GetVisibility(), 1);
  CHECK_DOUBLE(modelNode->GetDisplayNode()->GetColor()[0], 1.0);
  CHECK_DOUBLE(modelNode->GetDisplayNode()->GetColor()[1], 0.0);
  CHECK_DOUBLE(cameraNode->GetPosition()[0], 10.0);
  CHECK_DOUBLE(cameraNode->GetPosition()[1], 20.0);
  CHECK_DOUBLE(cameraNode->GetPosition()[2], 30.0);
  CHECK_INT(sliceNode->GetSliceVisible(), 1);

  // Restoring the second scene view restores the modified state
  CHECK_BOOL(sceneViewLogic->RestoreSceneView(1), true);
  CHECK_INT(modelNode->GetDisplayNode()->GetVisibility(), 0);
  CHECK_DOUBLE(modelNode->GetDisplayNode()->GetColor()[0], 0.0);
  CHECK_DOUBLE(modelNode->GetDisplayNode()->GetColor()[1], 1.0);
  CHECK_DOUBLE(cameraNode->GetPosition()[0], 100.0);
  CHECK_DOUBLE(cameraNode->GetPosition()[1], 200.0);
  CHECK_DOUBLE(cameraNode->GetPosition()[2], 300.0);
  CHECK_INT(sliceNode->GetSliceVisible(), 0);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int testImportLegacySceneViewIntoPopulatedScene(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  // Create a scene file with a legacy scene view (in a separate scene, not observed by the logic,
  // so that the scene view is not converted)
  vtkNew<vtkMRMLScene> sourceScene;
  sourceScene->RegisterNodeClass(vtkNew<vtkMRMLSceneViewNode>());
  populateScene(sourceScene);
  addLegacySceneView(sourceScene, "ImportedSceneView");
  modifyScene(sourceScene);
  sourceScene->SetSaveToXMLString(1);
  CHECK_BOOL(sourceScene->Commit() != 0, true);
  std::string sceneXML = sourceScene->GetSceneXMLString();

  // Make the camera nodes look like in legacy scene files:
  // - the ID of the camera node of the view is not based on the singleton tag (vtkMRMLCameraNode4),
  // - there is a camera node that is not used by any view, with an ID (vtkMRMLCameraNode1) that
  //   conflicts with the ID of the camera node of the view in the scene that the file is imported into,
  // - the snapshot contains a stale copy of the camera node of the view (same singleton tag, different ID).
  const std::string viewCameraID = "id=\"vtkMRMLCameraNode1\"";
  int viewCameraIDCount = 0;
  for (size_t pos = sceneXML.find(viewCameraID); pos != std::string::npos; pos = sceneXML.find(viewCameraID, pos))
  {
    sceneXML.replace(pos, viewCameraID.size(), "id=\"vtkMRMLCameraNode4\"");
    viewCameraIDCount++;
  }
  CHECK_INT(viewCameraIDCount, 2); // in the scene and in the snapshot
  size_t sceneViewElementStart = sceneXML.find("<SceneView");
  CHECK_BOOL(sceneViewElementStart != std::string::npos, true);
  sceneXML.insert(sceneViewElementStart,
                  "<Camera id=\"vtkMRMLCameraNode1\" name=\"UnusedCamera\""
                  " position=\"5 5 5\" focalPoint=\"0 0 0\" viewUp=\"0 0 1\"></Camera>\n");
  sceneViewElementStart = sceneXML.find("<SceneView");
  size_t sceneViewElementEnd = sceneXML.find(">", sceneViewElementStart);
  CHECK_BOOL(sceneViewElementEnd != std::string::npos, true);
  sceneXML.insert(sceneViewElementEnd + 1,
                  "<Camera id=\"vtkMRMLCameraNode99\" name=\"StaleCamera\" singletonTag=\"1\""
                  " position=\"1 2 3\" focalPoint=\"0 0 0\" viewUp=\"0 0 1\"></Camera>");

  // Import the scene into a scene that already contains nodes with the same IDs.
  // The imported nodes get new IDs, the snapshot in the legacy scene view refers to the original IDs.
  scene->Clear(1);
  populateScene(scene);
  vtkMRMLCameraNode* existingCameraNode = vtkMRMLCameraNode::SafeDownCast(scene->GetSingletonNode("1", "vtkMRMLCameraNode"));
  CHECK_NOT_NULL(existingCameraNode);
  CHECK_STD_STRING(existingCameraNode->GetID(), "vtkMRMLCameraNode1");
  vtkMRMLModelNode* existingModelNode = vtkMRMLModelNode::SafeDownCast(scene->GetFirstNodeByName("Model"));
  vtkMRMLDisplayNode* existingDisplayNode = existingModelNode->GetDisplayNode();
  existingDisplayNode->SetVisibility(1);
  const int existingDisplayNodeCount = scene->GetNumberOfNodesByClass("vtkMRMLModelDisplayNode");
  CHECK_INT(existingDisplayNodeCount, 1);

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(sceneXML);
  // The logic converts the legacy scene view at the end of the import
  CHECK_BOOL(scene->Import() != 0, true);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLSceneViewNode"), 0);
  CHECK_INT(sceneViewLogic->GetNumberOfSceneViews(), 1);

  // The scene view refers to the imported display node (no duplicate display node is created)
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelDisplayNode"), existingDisplayNodeCount + 1);
  vtkMRMLDisplayNode* importedDisplayNode = nullptr;
  for (int i = 0; i < scene->GetNumberOfNodesByClass("vtkMRMLModelDisplayNode"); ++i)
  {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLModelDisplayNode"));
    if (displayNode != existingDisplayNode)
    {
      importedDisplayNode = displayNode;
    }
  }
  CHECK_NOT_NULL(importedDisplayNode);
  CHECK_NOT_NULL(sceneViewLogic->GetNthSceneViewDataNode(0, importedDisplayNode));
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkMRMLNode* existingDisplayNodeDataNode = sceneViewLogic->GetNthSceneViewDataNode(0, existingDisplayNode);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_NULL(existingDisplayNodeDataNode);

  // The camera node of the view in the file was merged into the existing singleton camera node (its ID was changed),
  // the unused camera node in the file got a new ID (it conflicted with the existing singleton camera node ID),
  // and the stale camera node in the snapshot was not added to the scene.
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLCameraNode"), 2);
  CHECK_POINTER(scene->GetSingletonNode("1", "vtkMRMLCameraNode"), existingCameraNode);
  CHECK_NULL(scene->GetFirstNodeByName("StaleCamera"));
  vtkMRMLNode* importedUnusedCameraNode = scene->GetFirstNodeByName("UnusedCamera");
  CHECK_NOT_NULL(importedUnusedCameraNode);
  CHECK_BOOL(std::string(importedUnusedCameraNode->GetID()) != "vtkMRMLCameraNode1", true);
  // The scene view stores the camera node that is used by the view (not the unused camera node)
  CHECK_NOT_NULL(sceneViewLogic->GetNthSceneViewDataNode(0, existingCameraNode));
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkMRMLNode* importedUnusedCameraNodeDataNode = sceneViewLogic->GetNthSceneViewDataNode(0, importedUnusedCameraNode);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_NULL(importedUnusedCameraNodeDataNode);

  // Restoring the scene view updates the imported display node only
  importedDisplayNode->SetVisibility(0);
  CHECK_BOOL(sceneViewLogic->RestoreSceneView(0), true);
  CHECK_INT(importedDisplayNode->GetVisibility(), 1);
  CHECK_INT(existingDisplayNode->GetVisibility(), 1);

  // Restoring the scene view restores the camera position that was stored for the view's camera
  // (not the position of the stale camera node that was in the snapshot)
  CHECK_DOUBLE(existingCameraNode->GetPosition()[0], 10.0);
  CHECK_DOUBLE(existingCameraNode->GetPosition()[1], 20.0);
  CHECK_DOUBLE(existingCameraNode->GetPosition()[2], 30.0);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
// Convert legacy model hierarchy nodes to subject hierarchy folders with folder display nodes,
// the same way as vtkSlicerModelsLogic::OnMRMLSceneEndImport() does (the models logic is not available in this test).
// Invoked at the end of the scene import, before the scene views logic converts the legacy scene views.
void convertModelHierarchyNodesToFolders(vtkObject* caller, unsigned long vtkNotUsed(eid), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(caller);
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene);
  std::vector<vtkMRMLNode*> hierarchyNodes;
  scene->GetNodesByClass("vtkMRMLModelHierarchyNode", hierarchyNodes);
  std::vector<vtkMRMLNode*> nodesToRemove;
  for (vtkMRMLNode* node : hierarchyNodes)
  {
    vtkMRMLModelHierarchyNode* hierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
    nodesToRemove.push_back(hierarchyNode);
    vtkMRMLModelDisplayNode* hierarchyDisplayNode = hierarchyNode->GetModelDisplayNode();
    if (!hierarchyDisplayNode)
    {
      // leaf hierarchy node, associated with a model node
      continue;
    }
    nodesToRemove.push_back(hierarchyDisplayNode);
    vtkIdType folderItemID = shNode->CreateFolderItem(shNode->GetSceneItemID(), hierarchyNode->GetName());
    vtkMRMLFolderDisplayNode* folderDisplayNode = vtkMRMLFolderDisplayNode::AddDisplayNodeForItem(shNode, folderItemID);
    folderDisplayNode->SetColor(hierarchyDisplayNode->GetColor());
    folderDisplayNode->SetApplyDisplayPropertiesOnBranch(!hierarchyNode->GetExpanded());
    folderDisplayNode->SetAttribute(vtkSlicerSceneViewsModuleLogic::GetModelHierarchyNodeIDAttributeName(), hierarchyNode->GetID());
    std::vector<vtkMRMLHierarchyNode*> childHierarchyNodes = hierarchyNode->GetChildrenNodes();
    for (vtkMRMLHierarchyNode* childHierarchyNode : childHierarchyNodes)
    {
      vtkMRMLNode* associatedNode = childHierarchyNode->GetAssociatedNode();
      if (!associatedNode)
      {
        continue;
      }
      vtkIdType associatedItemID = shNode->GetItemByDataNode(associatedNode);
      if (!associatedItemID)
      {
        associatedItemID = shNode->CreateItem(shNode->GetSceneItemID(), associatedNode);
      }
      shNode->SetItemParent(associatedItemID, folderItemID);
    }
  }
  for (vtkMRMLNode* node : nodesToRemove)
  {
    scene->RemoveNode(node);
  }
}

//---------------------------------------------------------------------------
int testConvertLegacySceneViewWithModelHierarchy(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  // Create a scene file with legacy model hierarchy nodes and legacy scene views
  // (in a separate scene, not observed by the logic, so that the scene views are not converted)
  vtkNew<vtkMRMLScene> sourceScene;
  sourceScene->RegisterNodeClass(vtkNew<vtkMRMLSceneViewNode>());
  populateScene(sourceScene);
  vtkMRMLModelNode* sourceModelNode = vtkMRMLModelNode::SafeDownCast(sourceScene->GetFirstNodeByName("Model"));

  // Legacy model hierarchy: a folder hierarchy node with a display node, and a leaf hierarchy node associated with the model.
  // The folder is collapsed (not expanded), which means that its display properties are applied to all its children.
  vtkNew<vtkMRMLModelDisplayNode> folderHierarchyDisplayNode;
  folderHierarchyDisplayNode->SetColor(0.0, 0.0, 1.0);
  folderHierarchyDisplayNode->SetVisibility(0);
  folderHierarchyDisplayNode->SetOpacity(0.5);
  sourceScene->AddNode(folderHierarchyDisplayNode);
  vtkNew<vtkMRMLModelHierarchyNode> folderHierarchyNode;
  folderHierarchyNode->SetName("Folder");
  sourceScene->AddNode(folderHierarchyNode);
  folderHierarchyNode->SetAndObserveDisplayNodeID(folderHierarchyDisplayNode->GetID());
  folderHierarchyNode->SetExpanded(0);
  vtkNew<vtkMRMLModelHierarchyNode> leafHierarchyNode;
  leafHierarchyNode->SetName("ModelHierarchy");
  leafHierarchyNode->HideFromEditorsOn();
  sourceScene->AddNode(leafHierarchyNode);
  leafHierarchyNode->SetParentNodeID(folderHierarchyNode->GetID());
  leafHierarchyNode->SetAssociatedNodeID(sourceModelNode->GetID());

  addLegacySceneView(sourceScene, "Collapsed");
  folderHierarchyNode->SetExpanded(1);
  folderHierarchyDisplayNode->SetColor(0.0, 1.0, 0.0);
  addLegacySceneView(sourceScene, "Expanded");
  sourceScene->SetSaveToXMLString(1);
  CHECK_BOOL(sourceScene->Commit() != 0, true);
  std::string sceneXML = sourceScene->GetSceneXMLString();

  // Import the scene. The model hierarchy nodes are converted to folders at the end of the import,
  // before the scene views logic converts the scene views (the scene views logic uses a lower priority).
  scene->Clear(1);
  vtkNew<vtkCallbackCommand> convertCommand;
  convertCommand->SetCallback(convertModelHierarchyNodesToFolders);
  unsigned long observerTag = scene->AddObserver(vtkMRMLScene::EndImportEvent, convertCommand, 0.0);
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(sceneXML);
  CHECK_BOOL(scene->Import() != 0, true);
  scene->RemoveObserver(observerTag);

  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode"), 0);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLFolderDisplayNode"), 1);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLSceneViewNode"), 0);
  CHECK_INT(sceneViewLogic->GetNumberOfSceneViews(), 2);
  // The display node of the model hierarchy node in the snapshot is not added to the scene
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelDisplayNode"), 1);

  // The folder display node stores the properties of the model hierarchy display node in each scene view
  vtkMRMLFolderDisplayNode* folderDisplayNode = vtkMRMLFolderDisplayNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLFolderDisplayNode"));
  CHECK_NOT_NULL(folderDisplayNode);
  CHECK_BOOL(folderDisplayNode->GetApplyDisplayPropertiesOnBranch(), false);
  vtkMRMLFolderDisplayNode* collapsedDataNode = vtkMRMLFolderDisplayNode::SafeDownCast(sceneViewLogic->GetNthSceneViewDataNode(0, folderDisplayNode));
  CHECK_NOT_NULL(collapsedDataNode);
  CHECK_BOOL(collapsedDataNode->GetApplyDisplayPropertiesOnBranch(), true);
  CHECK_DOUBLE(collapsedDataNode->GetColor()[1], 0.0);
  CHECK_DOUBLE(collapsedDataNode->GetColor()[2], 1.0);
  // Visibility and opacity of a collapsed hierarchy are applied to the branch
  CHECK_INT(collapsedDataNode->GetVisibility(), 0);
  CHECK_DOUBLE(collapsedDataNode->GetOpacity(), 0.5);
  vtkMRMLFolderDisplayNode* expandedDataNode = vtkMRMLFolderDisplayNode::SafeDownCast(sceneViewLogic->GetNthSceneViewDataNode(1, folderDisplayNode));
  CHECK_NOT_NULL(expandedDataNode);
  CHECK_BOOL(expandedDataNode->GetApplyDisplayPropertiesOnBranch(), false);
  CHECK_DOUBLE(expandedDataNode->GetColor()[1], 1.0);
  CHECK_DOUBLE(expandedDataNode->GetColor()[2], 0.0);
  // Display node of an expanded hierarchy had no effect on the children, so the folder must not hide or fade the branch
  CHECK_INT(expandedDataNode->GetVisibility(), 1);
  CHECK_DOUBLE(expandedDataNode->GetOpacity(), 1.0);

  // Restoring the scene views restores the folder display properties
  folderDisplayNode->SetColor(1.0, 1.0, 1.0);
  CHECK_BOOL(sceneViewLogic->RestoreSceneView(0), true);
  CHECK_BOOL(folderDisplayNode->GetApplyDisplayPropertiesOnBranch(), true);
  CHECK_DOUBLE(folderDisplayNode->GetColor()[0], 0.0);
  CHECK_DOUBLE(folderDisplayNode->GetColor()[2], 1.0);
  CHECK_INT(folderDisplayNode->GetVisibility(), 0);
  CHECK_BOOL(sceneViewLogic->RestoreSceneView(1), true);
  CHECK_BOOL(folderDisplayNode->GetApplyDisplayPropertiesOnBranch(), false);
  CHECK_DOUBLE(folderDisplayNode->GetColor()[1], 1.0);
  CHECK_DOUBLE(folderDisplayNode->GetColor()[2], 0.0);
  CHECK_INT(folderDisplayNode->GetVisibility(), 1);
  CHECK_DOUBLE(folderDisplayNode->GetOpacity(), 1.0);

  return EXIT_SUCCESS;
}

} // end of anonymous namespace
