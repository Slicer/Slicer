/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Johan Andruejol, Kitware Inc.

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include <ctkCoreTestingMacros.h>

// qMRML includes
#include "qMRMLNodeComboBox.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// test the filtering with many cases
int qMRMLNodeComboBoxTest9( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLScalarVolumeNode> noAttributeNode;
  scene->AddNode(noAttributeNode.GetPointer());

  const char *testingAttributeName = "testingAttribute";
  const char *testingAttribute = noAttributeNode->GetAttribute(testingAttributeName);
  std::cout << "Volume node with no call to SetAttribute, GetAttribute returns " << (testingAttribute ? testingAttribute : "0") << "." << std::endl;
  CHECK_NULL(testingAttribute);

  vtkNew<vtkMRMLScalarVolumeNode> emptyStringAttributeNode;
  emptyStringAttributeNode->SetAttribute(testingAttributeName, "");
  scene->AddNode(emptyStringAttributeNode.GetPointer());

  testingAttribute = emptyStringAttributeNode->GetAttribute(testingAttributeName);
  CHECK_STRING(testingAttribute, "");

  vtkNew<vtkMRMLScalarVolumeNode> validAttributeNode;
  validAttributeNode->SetAttribute(testingAttributeName, "a");
  scene->AddNode(validAttributeNode.GetPointer());

  testingAttribute = validAttributeNode->GetAttribute(testingAttributeName);
  CHECK_STRING(testingAttribute, "a");

  // a node selector with no filtering attribute, three volumes should be
  // counted
  qMRMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList("vtkMRMLScalarVolumeNode"));
  nodeSelector.setMRMLScene(scene.GetPointer());

  CHECK_INT(nodeSelector.nodeCount(), 3);
  QVariant filter = nodeSelector.sortFilterProxyModel()->attributeFilter("vtkMRMLScalarVolumeNode", testingAttributeName);
  CHECK_QVARIANT(filter, QVariant());
  std::cout << "Passed with no filtering\n" << std::endl;
  nodeSelector.show();

  // a node selector, remove any attribute
  qMRMLNodeComboBox nodeSelectorA;
  nodeSelectorA.setNodeTypes(QStringList("vtkMRMLScalarVolumeNode"));
  nodeSelectorA.removeAttribute("vtkMRMLScalarVolumeNode", testingAttributeName);
  nodeSelectorA.setMRMLScene(scene.GetPointer());

  CHECK_INT(nodeSelectorA.nodeCount(), 3);
  filter = nodeSelectorA.sortFilterProxyModel()->attributeFilter("vtkMRMLScalarVolumeNode", testingAttributeName);
  CHECK_QVARIANT(filter, QVariant());
  std::cout << "Passed with removing attribute before anything\n" << std::endl;
  nodeSelectorA.show();

  // a node selector with a defined filtering attribute that doesn't match any
  // volumes, count should be zero
  qMRMLNodeComboBox nodeSelectorB;
  nodeSelectorB.setNodeTypes(QStringList("vtkMRMLScalarVolumeNode"));
  nodeSelectorB.addAttribute("vtkMRMLScalarVolumeNode", testingAttributeName, "a");
  nodeSelectorB.setMRMLScene(scene.GetPointer());

  CHECK_INT(nodeSelectorB.nodeCount(), 1);

  filter = nodeSelectorB.sortFilterProxyModel()->attributeFilter("vtkMRMLScalarVolumeNode", testingAttributeName);
  CHECK_QVARIANT(filter, QVariant("a"));

  nodeSelectorB.removeAttribute("vtkMRMLScalarVolumeNode", testingAttributeName);
  CHECK_INT(nodeSelectorB.nodeCount(), 3);

  filter = nodeSelectorB.sortFilterProxyModel()->attributeFilter("vtkMRMLScalarVolumeNode", testingAttributeName);
  CHECK_QVARIANT(filter, QVariant());
  std::cout << "Passed with removing attribute after stuff happened\n" << std::endl;
  nodeSelectorB.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
