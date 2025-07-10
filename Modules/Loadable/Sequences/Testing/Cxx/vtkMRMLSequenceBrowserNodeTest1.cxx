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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkSlicerSequencesLogic.h"

// VTK includes
#include <vtkNew.h>

namespace
{

double valueForIndex(int i)
{
  return i * 1234.567890;
}

int TestIndexFormatting()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  const int numberOfDataNodes = 135;
  for (int i = 0; i < numberOfDataNodes; i++)
  {
    vtkNew<vtkMRMLTransformNode> transform;
    std::string indexValue = vtkVariant(valueForIndex(i)).ToString();
    sequenceNode->SetDataNodeAtValue(transform.GetPointer(), indexValue);
  }
  scene->AddNode(sequenceNode.GetPointer());

  vtkNew<vtkMRMLSequenceBrowserNode> browserNode;
  scene->AddNode(browserNode.GetPointer());
  browserNode->SetAndObserveMasterSequenceNodeID(sequenceNode->GetID());

  int numberOfDecimals = 5;
  std::string prefix = "%%x%t%y";
  std::stringstream formatSS;
  formatSS << "%." << numberOfDecimals << "f";
  std::string format = formatSS.str();
  std::string suffix = "%s";

  // Basic sprintf string "%.5f"
  browserNode->SetIndexDisplayFormat(format);
  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
  {
    std::string formattedIndexValue = browserNode->GetFormattedIndexValue(i);
    std::stringstream expectedFormatSS;
    expectedFormatSS.precision(numberOfDecimals);

    // Perform same conversion as sequence browser node (string to float with vtkVariant)
    expectedFormatSS << std::fixed << vtkVariant(vtkVariant(valueForIndex(i)).ToString()).ToFloat();
    std::string expectedFormat = expectedFormatSS.str();

    CHECK_STD_STRING(formattedIndexValue, expectedFormat);
  }

  // Basic sprintf string "%s"
  browserNode->SetIndexDisplayFormat("%s");
  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
  {
    std::string formattedIndexValue = browserNode->GetFormattedIndexValue(i);
    std::string expectedFormat = sequenceNode->GetNthIndexValue(i);
    CHECK_STD_STRING(formattedIndexValue, expectedFormat);
  }

  // Complex sprintf string ""%%x%t%y%.7f%.7f%s". Should only match with first "%.7f"
  numberOfDecimals = 7;
  std::stringstream formatSS2;
  formatSS2 << "%." << numberOfDecimals << "f";
  format = formatSS2.str();
  suffix = format + suffix;
  browserNode->SetIndexDisplayFormat(prefix + format + suffix);
  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
  {
    std::string formattedIndexValue = browserNode->GetFormattedIndexValue(i);
    std::stringstream expectedFormatSS;
    expectedFormatSS.precision(numberOfDecimals);

    // Perform same conversion as sequence browser node (string to float with vtkVariant)
    expectedFormatSS << std::fixed << prefix << vtkVariant(vtkVariant(valueForIndex(i)).ToString()).ToFloat() << suffix;
    std::string expectedFormat = expectedFormatSS.str();

    CHECK_STD_STRING(formattedIndexValue, expectedFormat);
  }

  return EXIT_SUCCESS;
}

int TestSelectNextItem()
{
  vtkNew<vtkMRMLScene> scene;

  // Register vtkMRMLSequenceBrowserNode
  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  sequencesLogic->SetMRMLScene(scene.GetPointer());

  vtkMRMLSequenceNode* sequenceNode =
    vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  vtkMRMLSequenceBrowserNode* browserNode =
    vtkMRMLSequenceBrowserNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceBrowserNode"));
  CHECK_NOT_NULL(browserNode);
  browserNode->SetAndObserveMasterSequenceNodeID(sequenceNode->GetID());

  // Test empty sequence
  CHECK_INT(browserNode->GetSelectedItemNumber(), -1);
  browserNode->SelectNextItem(1);
  CHECK_INT(browserNode->GetSelectedItemNumber(), -1);
  browserNode->SelectNextItem(-1);
  CHECK_INT(browserNode->GetSelectedItemNumber(), -1);
  browserNode->SelectNextItem(2);
  CHECK_INT(browserNode->GetSelectedItemNumber(), -1);
  browserNode->SelectNextItem(-2);
  CHECK_INT(browserNode->GetSelectedItemNumber(), -1);

  // Test with single-item sequence
  vtkNew<vtkMRMLTransformNode> transform;
  sequenceNode->SetDataNodeAtValue(transform, "1");
  CHECK_INT(browserNode->GetSelectedItemNumber(), 0);
  browserNode->SelectNextItem(1);
  CHECK_INT(browserNode->GetSelectedItemNumber(), 0);
  browserNode->SelectNextItem(-1);
  CHECK_INT(browserNode->GetSelectedItemNumber(), 0);
  browserNode->SelectNextItem(2);
  CHECK_INT(browserNode->GetSelectedItemNumber(), 0);
  browserNode->SelectNextItem(-2);
  CHECK_INT(browserNode->GetSelectedItemNumber(), 0);

  // Test with two-item sequence
  sequenceNode->SetDataNodeAtValue(transform, "2");
  CHECK_INT(browserNode->GetSelectedItemNumber(), 0);
  browserNode->SelectNextItem(1);
  CHECK_INT(browserNode->GetSelectedItemNumber(), 1);
  browserNode->SelectNextItem(1);
  CHECK_INT(browserNode->GetSelectedItemNumber(), 0);
  browserNode->SelectNextItem(-1);
  CHECK_INT(browserNode->GetSelectedItemNumber(), 1);
  browserNode->SelectNextItem(2);
  CHECK_INT(browserNode->GetSelectedItemNumber(), 1);
  browserNode->SelectNextItem(-2);
  CHECK_INT(browserNode->GetSelectedItemNumber(), 1);

  return EXIT_SUCCESS;
}

int TestRemoveItem()
{
  vtkNew<vtkMRMLScene> scene;

  // Register vtkMRMLSequenceBrowserNode
  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  sequencesLogic->SetMRMLScene(scene.GetPointer());

  vtkMRMLSequenceNode* sequenceNode =
    vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  vtkMRMLSequenceBrowserNode* browserNode =
    vtkMRMLSequenceBrowserNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceBrowserNode"));
  CHECK_NOT_NULL(browserNode);
  browserNode->SetAndObserveMasterSequenceNodeID(sequenceNode->GetID());

  // Test with recording mode enabled
  vtkNew<vtkMRMLTransformNode> transform;

  browserNode->SetSaveChanges(sequenceNode, true);
  sequenceNode->SetDataNodeAtValue(transform, "1");
  sequenceNode->SetDataNodeAtValue(transform, "2");

  // Check if a proxy node is created
  sequencesLogic->UpdateAllProxyNodes();
  vtkMRMLNode* proxyNode = browserNode->GetProxyNode(sequenceNode);
  CHECK_NOT_NULL(proxyNode);

  // Remove a data node from the sequence
  sequenceNode->RemoveDataNodeAtValue("2");
  CHECK_INT(browserNode->GetSelectedItemNumber(), 0);
  CHECK_INT(sequenceNode->GetNumberOfDataNodes(), 1);

  // Check if modifying a proxy does not create a new timepoint
  proxyNode->Modified();
  CHECK_INT(sequenceNode->GetNumberOfDataNodes(), 1);

  // Remove the last data node from the sequence
  sequenceNode->RemoveDataNodeAtValue("1");
  CHECK_INT(browserNode->GetSelectedItemNumber(), -1);
  CHECK_INT(sequenceNode->GetNumberOfDataNodes(), 0);

  // Check that modifying a proxy node does not create a new timepoint
  // (enabling "save changes" does not create a new timepoint, it just updates the existing timepoint;
  // "recording" creates a new timepoint each time there is a modification, if the browser is in recording mode).
  proxyNode->Modified();
  CHECK_INT(browserNode->GetSelectedItemNumber(), -1);
  CHECK_INT(sequenceNode->GetNumberOfDataNodes(), 0);

  return EXIT_SUCCESS;
}

} // end anonymous namespace

int vtkMRMLSequenceBrowserNodeTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  CHECK_EXIT_SUCCESS(TestIndexFormatting());
  CHECK_EXIT_SUCCESS(TestSelectNextItem());
  CHECK_EXIT_SUCCESS(TestRemoveItem());
  return EXIT_SUCCESS;
}
