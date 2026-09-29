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

==============================================================================*/

// MRML includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLTextNode.h"
#include "vtkSlicerSequencesLogic.h"
// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

namespace
{
int TestLogicWithoutScene()
{
  // Testing sequences logic without setting a scene.
  // Errors are be returned but there should be no crash.

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  vtkNew<vtkMRMLSequenceBrowserNode> browserNode;
  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  vtkNew<vtkMRMLTextNode> proxyNode;
  vtkNew<vtkMRMLSequenceNode> sequenceNode2;

  CHECK_NULL(sequencesLogic->AddSequence(nullptr /*filename*/));

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_NULL(sequencesLogic->AddSynchronizedNode(nullptr, proxyNode, browserNode));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_NOT_NULL(sequencesLogic->AddSynchronizedNode(sequenceNode, proxyNode, browserNode));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  sequencesLogic->UpdateProxyNodesFromSequences(browserNode);

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  sequencesLogic->UpdateSequencesFromProxyNodes(browserNode, proxyNode);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  sequencesLogic->UpdateAllProxyNodes();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkNew<vtkCollection> compatibleNodes;
  sequencesLogic->GetCompatibleNodesFromScene(compatibleNodes, sequenceNode);
  CHECK_INT(compatibleNodes->GetNumberOfItems(), 0);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  CHECK_BOOL(vtkSlicerSequencesLogic::IsNodeCompatibleForBrowsing(sequenceNode, sequenceNode2), true);

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkNew<vtkCollection> foundBrowserNodes;
  sequencesLogic->GetBrowserNodesForSequenceNode(sequenceNode, foundBrowserNodes);
  CHECK_INT(foundBrowserNodes->GetNumberOfItems(), 0);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_NULL(sequencesLogic->GetFirstBrowserNodeForSequenceNode(sequenceNode));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  foundBrowserNodes->RemoveAllItems();
  sequencesLogic->GetBrowserNodesForProxyNode(proxyNode, foundBrowserNodes);
  CHECK_INT(foundBrowserNodes->GetNumberOfItems(), 0);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_NULL(sequencesLogic->GetFirstBrowserNodeForProxyNode(proxyNode));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  return EXIT_SUCCESS;
}

int TestAddSequence()
{
  // Basic test of creating and browsing a sequence

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  sequencesLogic->SetMRMLScene(scene);

  vtkMRMLSequenceBrowserNode* browserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceBrowserNode"));
  vtkMRMLTextNode* proxyNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));

  vtkMRMLSequenceNode* sequenceNode = sequencesLogic->AddSynchronizedNode(nullptr, proxyNode, browserNode);
  CHECK_NOT_NULL(sequenceNode);
  CHECK_INT(sequenceNode->GetNumberOfDataNodes(), 0);

  // Add some nodes to the sequence
  vtkNew<vtkMRMLTextNode> proxyNode2;
  proxyNode2->SetText("Zero");
  sequenceNode->SetDataNodeAtValue(proxyNode2, "0");
  proxyNode2->SetText("One");
  sequenceNode->SetDataNodeAtValue(proxyNode2, "1");
  proxyNode2->SetText("Two");
  sequenceNode->SetDataNodeAtValue(proxyNode2, "2");

  // Browse the sequence
  CHECK_INT(browserNode->GetSelectedItemNumber(), 0); // first item is selected by default
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("0"), true);
  CHECK_STD_STRING(proxyNode->GetText(), "Zero");
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("1"), true);
  CHECK_STD_STRING(proxyNode->GetText(), "One");
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("2"), true);
  CHECK_STD_STRING(proxyNode->GetText(), "Two");

  return EXIT_SUCCESS;
}

int TestBatchProxyNodeUpdates()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  sequencesLogic->SetMRMLScene(scene);

  vtkMRMLSequenceBrowserNode* browserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceBrowserNode"));
  vtkMRMLTextNode* masterProxyNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));
  vtkMRMLTextNode* synchronizedProxyNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));
  vtkMRMLSequenceNode* masterSequenceNode = sequencesLogic->AddSynchronizedNode(nullptr, masterProxyNode, browserNode);
  vtkMRMLSequenceNode* synchronizedSequenceNode = sequencesLogic->AddSynchronizedNode(nullptr, synchronizedProxyNode, browserNode);
  CHECK_NOT_NULL(masterSequenceNode);
  CHECK_NOT_NULL(synchronizedSequenceNode);

  vtkNew<vtkMRMLTextNode> dataNode;
  dataNode->SetText("Master 0");
  masterSequenceNode->SetDataNodeAtValue(dataNode, "0");
  dataNode->SetText("Master 1");
  masterSequenceNode->SetDataNodeAtValue(dataNode, "1");
  dataNode->SetText("Synchronized 0");
  synchronizedSequenceNode->SetDataNodeAtValue(dataNode, "0");
  dataNode->SetText("Synchronized 1");
  synchronizedSequenceNode->SetDataNodeAtValue(dataNode, "1");
  browserNode->SetSaveChanges(nullptr, true);
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("1"), true);

  int wasModifying = browserNode->StartModify();
  masterProxyNode->SetText("Master edited");
  synchronizedProxyNode->SetText("Synchronized edited");
  CHECK_BOOL(browserNode->GetCustomModifiedEventPending(vtkMRMLSequenceBrowserNode::ProxyNodeModifiedEvent) > 0, true);
  browserNode->EndModify(wasModifying);

  vtkMRMLTextNode* masterItem = vtkMRMLTextNode::SafeDownCast(masterSequenceNode->GetDataNodeAtValue("1"));
  vtkMRMLTextNode* synchronizedItem = vtkMRMLTextNode::SafeDownCast(synchronizedSequenceNode->GetDataNodeAtValue("1"));
  CHECK_NOT_NULL(masterItem);
  CHECK_NOT_NULL(synchronizedItem);
  CHECK_STD_STRING(masterItem->GetText(), "Master edited");
  CHECK_STD_STRING(synchronizedItem->GetText(), "Synchronized edited");
  CHECK_STD_STRING(vtkMRMLTextNode::SafeDownCast(masterSequenceNode->GetDataNodeAtValue("0"))->GetText(), "Master 0");
  CHECK_STD_STRING(vtkMRMLTextNode::SafeDownCast(synchronizedSequenceNode->GetDataNodeAtValue("0"))->GetText(), "Synchronized 0");

  synchronizedProxyNode->SetText("Synchronized single");
  CHECK_STD_STRING(synchronizedItem->GetText(), "Synchronized single");

  browserNode->SetSaveChanges(synchronizedSequenceNode, false);
  wasModifying = browserNode->StartModify();
  masterProxyNode->SetText("Master saved");
  synchronizedProxyNode->SetText("Synchronized unsaved");
  browserNode->EndModify(wasModifying);
  CHECK_STD_STRING(masterItem->GetText(), "Master saved");
  CHECK_STD_STRING(synchronizedItem->GetText(), "Synchronized single");

  return EXIT_SUCCESS;
}

int TestBatchProxyNodeRecording()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  sequencesLogic->SetMRMLScene(scene);

  vtkMRMLSequenceBrowserNode* browserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceBrowserNode"));
  vtkMRMLTextNode* masterProxyNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));
  vtkMRMLTextNode* synchronizedProxyNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));
  vtkMRMLSequenceNode* masterSequenceNode = sequencesLogic->AddSynchronizedNode(nullptr, masterProxyNode, browserNode);
  vtkMRMLSequenceNode* synchronizedSequenceNode = sequencesLogic->AddSynchronizedNode(nullptr, synchronizedProxyNode, browserNode);
  CHECK_NOT_NULL(masterSequenceNode);
  CHECK_NOT_NULL(synchronizedSequenceNode);

  browserNode->SetRecording(masterSequenceNode, true);
  browserNode->SetRecording(synchronizedSequenceNode, true);
  browserNode->SetRecordMasterOnly(true);
  browserNode->SetRecordingSamplingMode(vtkMRMLSequenceBrowserNode::SamplingAll);
  browserNode->SetRecordingActive(true);

  int wasModifying = browserNode->StartModify();
  synchronizedProxyNode->SetText("Synchronized batched");
  CHECK_BOOL(browserNode->GetCustomModifiedEventPending(vtkMRMLSequenceBrowserNode::ProxyNodeModifiedEvent) > 0, true);
  browserNode->EndModify(wasModifying);
  CHECK_INT(masterSequenceNode->GetNumberOfDataNodes(), 0);
  CHECK_INT(synchronizedSequenceNode->GetNumberOfDataNodes(), 0);

  // Deferred events lose proxy identity, so even a master-only batch cannot trigger master-only recording.
  wasModifying = browserNode->StartModify();
  masterProxyNode->SetText("Master batched");
  CHECK_BOOL(browserNode->GetCustomModifiedEventPending(vtkMRMLSequenceBrowserNode::ProxyNodeModifiedEvent) > 0, true);
  browserNode->EndModify(wasModifying);
  CHECK_INT(masterSequenceNode->GetNumberOfDataNodes(), 0);
  CHECK_INT(synchronizedSequenceNode->GetNumberOfDataNodes(), 0);

  masterProxyNode->SetText("Master recorded");
  CHECK_INT(masterSequenceNode->GetNumberOfDataNodes(), 1);
  CHECK_INT(synchronizedSequenceNode->GetNumberOfDataNodes(), 1);
  std::string recordedIndex = masterSequenceNode->GetNthIndexValue(0);
  vtkMRMLTextNode* masterItem = vtkMRMLTextNode::SafeDownCast(masterSequenceNode->GetDataNodeAtValue(recordedIndex));
  vtkMRMLTextNode* synchronizedItem = vtkMRMLTextNode::SafeDownCast(synchronizedSequenceNode->GetDataNodeAtValue(recordedIndex));
  CHECK_NOT_NULL(masterItem);
  CHECK_NOT_NULL(synchronizedItem);
  CHECK_STD_STRING(masterItem->GetText(), "Master recorded");
  CHECK_STD_STRING(synchronizedItem->GetText(), "Synchronized batched");

  return EXIT_SUCCESS;
}

int TestBatchProxyNodeRecordingAll()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  sequencesLogic->SetMRMLScene(scene);

  vtkMRMLSequenceBrowserNode* browserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceBrowserNode"));
  vtkMRMLTextNode* masterProxyNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));
  vtkMRMLTextNode* synchronizedProxyNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));
  vtkMRMLSequenceNode* masterSequenceNode = sequencesLogic->AddSynchronizedNode(nullptr, masterProxyNode, browserNode);
  vtkMRMLSequenceNode* synchronizedSequenceNode = sequencesLogic->AddSynchronizedNode(nullptr, synchronizedProxyNode, browserNode);
  CHECK_NOT_NULL(masterSequenceNode);
  CHECK_NOT_NULL(synchronizedSequenceNode);

  masterProxyNode->SetText("Master unchanged");
  browserNode->SetRecording(masterSequenceNode, true);
  browserNode->SetRecording(synchronizedSequenceNode, true);
  browserNode->SetRecordMasterOnly(false);
  browserNode->SetRecordingSamplingMode(vtkMRMLSequenceBrowserNode::SamplingAll);
  browserNode->SetRecordingActive(true);

  int wasModifying = browserNode->StartModify();
  synchronizedProxyNode->SetText("Synchronized batched");
  CHECK_BOOL(browserNode->GetCustomModifiedEventPending(vtkMRMLSequenceBrowserNode::ProxyNodeModifiedEvent) > 0, true);
  browserNode->EndModify(wasModifying);

  CHECK_INT(masterSequenceNode->GetNumberOfDataNodes(), 1);
  CHECK_INT(synchronizedSequenceNode->GetNumberOfDataNodes(), 1);
  std::string recordedIndex = masterSequenceNode->GetNthIndexValue(0);
  vtkMRMLTextNode* masterItem = vtkMRMLTextNode::SafeDownCast(masterSequenceNode->GetDataNodeAtValue(recordedIndex));
  vtkMRMLTextNode* synchronizedItem = vtkMRMLTextNode::SafeDownCast(synchronizedSequenceNode->GetDataNodeAtValue(recordedIndex));
  CHECK_NOT_NULL(masterItem);
  CHECK_NOT_NULL(synchronizedItem);
  CHECK_STD_STRING(masterItem->GetText(), "Master unchanged");
  CHECK_STD_STRING(synchronizedItem->GetText(), "Synchronized batched");

  return EXIT_SUCCESS;
}

int TestSparseSequence()
{
  // Test sparse sequences, where not all sequences have items for all index values

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  sequencesLogic->SetMRMLScene(scene);

  // Create browser node
  vtkMRMLSequenceBrowserNode* browserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceBrowserNode"));

  vtkNew<vtkMRMLTextNode> proxyNodeTemp;

  // Add some nodes to the master sequence
  vtkMRMLTextNode* masterProxyNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));
  vtkMRMLSequenceNode* masterSequenceNode = sequencesLogic->AddSynchronizedNode(nullptr, masterProxyNode, browserNode);
  proxyNodeTemp->SetText("ZeroM");
  masterSequenceNode->SetDataNodeAtValue(proxyNodeTemp, "0");
  proxyNodeTemp->SetText("OneM");
  masterSequenceNode->SetDataNodeAtValue(proxyNodeTemp, "1");
  proxyNodeTemp->SetText("TwoM");
  masterSequenceNode->SetDataNodeAtValue(proxyNodeTemp, "2");
  proxyNodeTemp->SetText("ThreeM");
  masterSequenceNode->SetDataNodeAtValue(proxyNodeTemp, "3");
  proxyNodeTemp->SetText("FourM");
  masterSequenceNode->SetDataNodeAtValue(proxyNodeTemp, "4");

  // Add some nodes to a synchronized sequence
  vtkMRMLTextNode* synchronizedProxyNode1 = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));
  vtkMRMLSequenceNode* synchronizedSequenceNode1 = sequencesLogic->AddSynchronizedNode(nullptr, synchronizedProxyNode1, browserNode);
  proxyNodeTemp->SetText("ZeroS1");
  synchronizedSequenceNode1->SetDataNodeAtValue(proxyNodeTemp, "0");
  proxyNodeTemp->SetText("OneS1");
  synchronizedSequenceNode1->SetDataNodeAtValue(proxyNodeTemp, "1");
  proxyNodeTemp->SetText("FourS1");
  synchronizedSequenceNode1->SetDataNodeAtValue(proxyNodeTemp, "4");

  // Browse the sequence and check how missing items are handled.

  // Check defaults
  CHECK_INT(browserNode->GetMissingItemMode(synchronizedSequenceNode1), vtkMRMLSequenceBrowserNode::MissingItemCreateFromPrevious);
  CHECK_BOOL(browserNode->GetSaveChanges(synchronizedSequenceNode1), false);

  // 1. MissingItemCreateFromPrevious

  browserNode->SetMissingItemMode(synchronizedSequenceNode1, vtkMRMLSequenceBrowserNode::MissingItemCreateFromPrevious);
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("0"), true);
  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("0"));
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "ZeroS1");

  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("1"));
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("1"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "OneS1");

  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is missing
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("3"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "OneS1");   // previous value is copied
  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is still missing because save mode is not enabled

  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("4"));
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("4"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "FourS1");

  // Check if missing item is created if "save changes" is enabled
  browserNode->SetSaveChanges(synchronizedSequenceNode1, true);

  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is missing
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("3"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "OneS1");       // previous value is copied
  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is now created

  // Remove the created missing item and reset "save changes" to false
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("0"), true);
  synchronizedSequenceNode1->RemoveDataNodeAtValue("3");
  browserNode->SetSaveChanges(synchronizedSequenceNode1, false);

  // 2. MissingItemCreateFromDefault

  browserNode->SetMissingItemMode(synchronizedSequenceNode1, vtkMRMLSequenceBrowserNode::MissingItemCreateFromDefault);

  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is missing
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("3"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "");        // default value is used
  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is still missing because save mode is not enabled

  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("0")); // the item is present
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("0"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "ZeroS1");      // default value is used
  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("0")); // the item is still present

  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("4"), true);

  // Check if missing item is created if "save changes" is enabled
  browserNode->SetSaveChanges(synchronizedSequenceNode1, true);

  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is missing
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("3"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "");            // default value is used
  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is now created

  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("0")); // the item is present
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("0"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "ZeroS1");      // default value is used
  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("0")); // the item is still present

  // Remove the created missing item and reset "save changes" to false
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("0"), true);
  synchronizedSequenceNode1->RemoveDataNodeAtValue("3");
  browserNode->SetSaveChanges(synchronizedSequenceNode1, false);

  // 3. MissingItemSetToDefault

  browserNode->SetMissingItemMode(synchronizedSequenceNode1, vtkMRMLSequenceBrowserNode::MissingItemSetToDefault);

  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is missing
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("3"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "");        // default value is used
  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is still missing because save mode is not enabled

  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("0")); // the item is present
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("0"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "ZeroS1");      // default value is used
  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("0")); // the item is still present

  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("4"), true);

  // Check if missing item is created if "save changes" is enabled
  browserNode->SetSaveChanges(synchronizedSequenceNode1, true);

  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is missing
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("3"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "");        // default value is used
  CHECK_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("3")); // the item is still missing because MissingItemSetToDefault

  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("0")); // the item is present
  CHECK_BOOL(browserNode->SetSelectedItemByIndexValue("0"), true);
  CHECK_STD_STRING(synchronizedProxyNode1->GetText(), "ZeroS1");      // default value is used
  CHECK_NOT_NULL(synchronizedSequenceNode1->GetDataNodeAtValue("0")); // the item is still present

  return EXIT_SUCCESS;
}
} // namespace

int vtkSlicerSequencesLogicTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestLogicWithoutScene());
  CHECK_EXIT_SUCCESS(TestAddSequence());
  CHECK_EXIT_SUCCESS(TestBatchProxyNodeUpdates());
  CHECK_EXIT_SUCCESS(TestBatchProxyNodeRecording());
  CHECK_EXIT_SUCCESS(TestBatchProxyNodeRecordingAll());
  CHECK_EXIT_SUCCESS(TestSparseSequence());
  return EXIT_SUCCESS;
}
