/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSubjectHierarchyNode.h"

int vtkMRMLSubjectHierarchyNodeTest1(int , char * [])
{
  // Add a scene with 3 text nodes
  vtkNew<vtkMRMLScene> scene;

  // Check if subject hierarchy node is provided
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  CHECK_NOT_NULL(shNode);

  vtkNew<vtkMRMLModelNode> dataNode1;
  scene->AddNode(dataNode1);

  // Create subject hierarchy item
  vtkIdType itemId1 = shNode->CreateItem(shNode->GetSceneItemID(), dataNode1, "test");
  CHECK_BOOL(itemId1 == vtkMRMLSubjectHierarchyNode::GetInvalidItemID(), false);

  // Test UIDs
  /////////////////////////

  std::string uidName = "TESTID";
  std::string uidValue = "123";

  // Test API with missing UID
  CHECK_BOOL(shNode->HasItemUID(itemId1, uidName), false);
  CHECK_STD_STRING(shNode->GetItemUID(itemId1, uidName), "");
  CHECK_INT(shNode->GetItemUIDNames(itemId1).size(), 0);

  // Test after UID is set

  shNode->SetItemUID(itemId1, uidName, uidValue);

  CHECK_BOOL(shNode->HasItemUID(itemId1, uidName), true);
  CHECK_STD_STRING(shNode->GetItemUID(itemId1, uidName), uidValue);
  CHECK_INT(shNode->GetItemUIDNames(itemId1).size(), 1);

  // Test after UID is removed

  CHECK_BOOL(shNode->RemoveItemUID(itemId1, uidName), true);

  CHECK_BOOL(shNode->HasItemUID(itemId1, uidName), false);
  CHECK_STD_STRING(shNode->GetItemUID(itemId1, uidName), "");
  CHECK_INT(shNode->GetItemUIDNames(itemId1).size(), 0);

  // Test getting list of all UID names

  shNode->SetItemUID(itemId1, "abc", "3");
  shNode->SetItemUID(itemId1, "defffff", "5");
  shNode->SetItemUID(itemId1, "ggggg", "8");

  auto ids = shNode->GetItemUIDNames(itemId1);
  CHECK_INT(ids.size(), 3);
  // Check if all 3 strings are found in the returned list
  CHECK_BOOL(std::find(ids.begin(), ids.end(), "abc") != ids.end(), true);
  CHECK_BOOL(std::find(ids.begin(), ids.end(), "defffff") != ids.end(), true);
  CHECK_BOOL(std::find(ids.begin(), ids.end(), "ggggg") != ids.end(), true);

  // Test attributes
  /////////////////////////

  std::string attributeName = "TESTATT";
  std::string attributeValue = "987";

  // Test API with missing attributes
  CHECK_BOOL(shNode->HasItemAttribute(itemId1, attributeName), false);
  CHECK_STD_STRING(shNode->GetItemAttribute(itemId1, attributeName), "");
  CHECK_INT(shNode->GetItemAttributeNames(itemId1).size(), 0);

  // Test after attributes is set

  shNode->SetItemAttribute(itemId1, attributeName, attributeValue);

  CHECK_BOOL(shNode->HasItemAttribute(itemId1, attributeName), true);
  CHECK_STD_STRING(shNode->GetItemAttribute(itemId1, attributeName), attributeValue);
  CHECK_INT(shNode->GetItemAttributeNames(itemId1).size(), 1);

  // Test after attributes is removed

  CHECK_BOOL(shNode->RemoveItemAttribute(itemId1, attributeName), true);

  CHECK_BOOL(shNode->HasItemAttribute(itemId1, attributeName), false);
  CHECK_STD_STRING(shNode->GetItemAttribute(itemId1, attributeName), "");
  CHECK_INT(shNode->GetItemAttributeNames(itemId1).size(), 0);

  // Test getting list of all attributes names

  shNode->SetItemAttribute(itemId1, "asd", "rrr");
  shNode->SetItemAttribute(itemId1, "zxcv", "fff");
  shNode->SetItemAttribute(itemId1, "qwer", "vvv");

  auto atts = shNode->GetItemAttributeNames(itemId1);
  CHECK_INT(atts.size(), 3);
  // Check if all 3 strings are found in the returned list
  CHECK_BOOL(std::find(atts.begin(), atts.end(), "asd") != atts.end(), true);
  CHECK_BOOL(std::find(atts.begin(), atts.end(), "zxcv") != atts.end(), true);
  CHECK_BOOL(std::find(atts.begin(), atts.end(), "qwer") != atts.end(), true);

  return EXIT_SUCCESS;
}
