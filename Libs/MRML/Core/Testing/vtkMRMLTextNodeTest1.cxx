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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLTextNode.h"
#include "vtkMRMLScene.h"

int vtkMRMLTextNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLTextNode> textNode;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(textNode.GetPointer());

  std::string complicatedXMLString = "$ <\" > &lgt; =";
  textNode->SetText(complicatedXMLString);
  EXERCISE_ALL_BASIC_MRML_METHODS(textNode.GetPointer());

  std::string basicString = "Hello world!";
  textNode->SetText(basicString);
  CHECK_STD_STRING(textNode->GetText(), basicString);

  std::string emptyString = "";
  textNode->SetText(emptyString);
  CHECK_STD_STRING(textNode->GetText(), emptyString);

  // Storage node creation -- CreateStorageNodeAlways
  // Text node should create storage node regardless of length
  vtkNew<vtkMRMLTextNode> textNodeAlwaysStore;
  textNodeAlwaysStore->SetText("");
  textNodeAlwaysStore->SetForceCreateStorageNode(vtkMRMLTextNode::CreateStorageNodeAlways);
  CHECK_NOT_NULL(scene->AddNode(textNodeAlwaysStore.GetPointer()));
  CHECK_BOOL(textNodeAlwaysStore->AddDefaultStorageNode(), true);
  CHECK_NOT_NULL(textNodeAlwaysStore->GetStorageNode());

  // Storage node creation -- CreateStorageNodeAuto
  // Text is short enough to not create storage node with CreateStorageNodeAuto
  vtkNew<vtkMRMLTextNode> textNodeAutoStoreFalse;
  textNodeAutoStoreFalse->SetText("");
  textNodeAutoStoreFalse->SetForceCreateStorageNode(vtkMRMLTextNode::CreateStorageNodeAuto);
  CHECK_NOT_NULL(scene->AddNode(textNodeAutoStoreFalse.GetPointer()));
  CHECK_BOOL(textNodeAutoStoreFalse->AddDefaultStorageNode(), true);
  CHECK_NULL(textNodeAutoStoreFalse->GetStorageNode());

  // Text is long enough to create storage node with CreateStorageNodeAuto
  vtkNew<vtkMRMLTextNode> textNodeAutoStoreTrue;
  textNodeAutoStoreTrue->SetText(std::string(500, 'A'));
  textNodeAutoStoreTrue->SetForceCreateStorageNode(vtkMRMLTextNode::CreateStorageNodeAuto);
  CHECK_NOT_NULL(scene->AddNode(textNodeAutoStoreTrue.GetPointer()));
  CHECK_BOOL(textNodeAutoStoreTrue->AddDefaultStorageNode(), true);
  CHECK_NOT_NULL(textNodeAutoStoreTrue->GetStorageNode());

  // Add text node -- Never use storage node
  vtkNew<vtkMRMLTextNode> textNodeNeverStore;
  textNodeNeverStore->SetText(std::string(500, 'A'));
  textNodeNeverStore->SetForceCreateStorageNode(vtkMRMLTextNode::CreateStorageNodeNever);
  CHECK_NOT_NULL(scene->AddNode(textNodeNeverStore.GetPointer()));
  CHECK_BOOL(textNodeNeverStore->AddDefaultStorageNode(), true);
  CHECK_NULL(textNodeNeverStore->GetStorageNode());

  return EXIT_SUCCESS;
}
