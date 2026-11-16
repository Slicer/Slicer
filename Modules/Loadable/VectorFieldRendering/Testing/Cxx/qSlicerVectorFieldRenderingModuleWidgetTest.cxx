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

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// Qt includes
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

// CTK includes
#include "ctkTest.h"

// VectorFieldRendering includes
#include "qSlicerSubjectHierarchyVectorFieldRenderingPlugin.h"
#include "qSlicerVectorFieldRenderingModule.h"
#include "qSlicerVectorFieldRenderingModuleWidget.h"
#include "vtkMRMLVectorFieldTestingHelpers.h"
#include "vtkSlicerVectorFieldRenderingLogic.h"

// MRML includes
#include <qMRMLWidget.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>

// SubjectHierarchy includes
#include <qSlicerSubjectHierarchyPluginHandler.h>
#include <qSlicerSubjectHierarchyPluginLogic.h>
#include <vtkSlicerSubjectHierarchyModuleLogic.h>

// Slicer includes
#include <qSlicerApplication.h>

// VTK includes
#include <vtkNew.h>

using namespace vtkMRMLVectorFieldTesting;

// ----------------------------------------------------------------------------
class qSlicerVectorFieldRenderingModuleWidgetTester : public QObject
{
  Q_OBJECT

public:
  QSharedPointer<qSlicerVectorFieldRenderingModule> vectorFieldRenderingModule;
  QSharedPointer<qSlicerSubjectHierarchyPluginLogic> pluginLogic;
  vtkSmartPointer<vtkSlicerSubjectHierarchyModuleLogic> shModuleLogic;
  vtkSmartPointer<vtkMRMLScene> scene;

private slots:
  void initTestCase();
  void cleanupTestCase();
  void testAddAndRemove();
  void testAddAndRemove_data();
  void testNoVectorField();
  void testEditedNode();
  void testShowAsVectorField();

private:
  qSlicerVectorFieldRenderingModuleWidget* moduleWidget();
  /// A node of the given kind with a vector field, in the scene
  vtkMRMLDisplayableNode* addNodeWithVectorField(const QString& kind);
};

// ----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidgetTester::initTestCase()
{
  // The module is initialized once: initializing it again would register its subject hierarchy plugin again
  scene = vtkSmartPointer<vtkMRMLScene>::New();
  shModuleLogic = vtkSmartPointer<vtkSlicerSubjectHierarchyModuleLogic>::New();
  shModuleLogic->SetMRMLScene(scene);
  pluginLogic = QSharedPointer<qSlicerSubjectHierarchyPluginLogic>(new qSlicerSubjectHierarchyPluginLogic());
  pluginLogic->setMRMLScene(scene);
  qSlicerSubjectHierarchyPluginHandler::instance()->setPluginLogic(pluginLogic.get());
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);

  vectorFieldRenderingModule = QSharedPointer<qSlicerVectorFieldRenderingModule>(new qSlicerVectorFieldRenderingModule);
  vectorFieldRenderingModule->initialize(qSlicerApplication::application()->applicationLogic());
  vectorFieldRenderingModule->setMRMLScene(scene);
  vectorFieldRenderingModule->logic();
}

// ----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidgetTester::cleanupTestCase()
{
  vectorFieldRenderingModule.clear();
  pluginLogic.clear();
  shModuleLogic = nullptr;
  scene = nullptr;
}

// ----------------------------------------------------------------------------
qSlicerVectorFieldRenderingModuleWidget* qSlicerVectorFieldRenderingModuleWidgetTester::moduleWidget()
{
  return dynamic_cast<qSlicerVectorFieldRenderingModuleWidget*>(vectorFieldRenderingModule->widgetRepresentation());
}

// ----------------------------------------------------------------------------
vtkMRMLDisplayableNode* qSlicerVectorFieldRenderingModuleWidgetTester::addNodeWithVectorField(const QString& kind)
{
  if (kind == "model")
  {
    vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(50.0);
    AddPointField(surface, "Velocity", LinearField);
    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode", "Model"));
    modelNode->SetAndObserveMesh(surface);
    modelNode->CreateDefaultDisplayNodes();
    return modelNode;
  }
  if (kind == "volume")
  {
    vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode", "Volume"));
    volumeNode->SetAndObserveImageData(CreateVectorImage(LinearField));
    volumeNode->CreateDefaultDisplayNodes();
    return volumeNode;
  }
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "Transform"));
  transformNode->CreateDefaultDisplayNodes();
  return transformNode;
}

// ----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidgetTester::testAddAndRemove_data()
{
  QTest::addColumn<QString>("kind");
  QTest::addColumn<QString>("defaultDisplayNodeClass");
  QTest::newRow("model") << "model" << "vtkMRMLModelDisplayNode";
  QTest::newRow("vector volume") << "volume" << "vtkMRMLVectorVolumeDisplayNode";
  QTest::newRow("transform") << "transform" << "vtkMRMLTransformDisplayNode";
}

// ----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidgetTester::testAddAndRemove()
{
  QFETCH(QString, kind);
  QFETCH(QString, defaultDisplayNodeClass);
  qSlicerVectorFieldRenderingModuleWidget* widget = this->moduleWidget();
  QVERIFY(widget != nullptr);
  vtkMRMLDisplayableNode* node = this->addNodeWithVectorField(kind);
  QVERIFY(widget->setEditedNode(node));
  QCOMPARE(widget->currentNode(), node);
  QListWidget* listWidget = widget->findChild<QListWidget*>("DisplayNodesListWidget");
  QPushButton* addButton = widget->findChild<QPushButton*>("AddDisplayNodeButton");
  QPushButton* removeButton = widget->findChild<QPushButton*>("RemoveDisplayNodeButton");
  QVERIFY(listWidget && addButton && removeButton);
  QCOMPARE(listWidget->count(), 0);
  QVERIFY(addButton->isEnabled());
  QVERIFY(!removeButton->isEnabled());

  // Any number of displays can be added, after the default display node, and each one is selected when added
  addButton->click();
  vtkMRMLVectorFieldDisplayNode* firstDisplayNode = widget->currentDisplayNode();
  QVERIFY(firstDisplayNode != nullptr);
  addButton->click();
  vtkMRMLVectorFieldDisplayNode* secondDisplayNode = widget->currentDisplayNode();
  QVERIFY(secondDisplayNode != nullptr && secondDisplayNode != firstDisplayNode);
  QCOMPARE(listWidget->count(), 2);
  QCOMPARE(QString(node->GetNthDisplayNode(0)->GetClassName()), defaultDisplayNodeClass);
  QCOMPARE(static_cast<int>(vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(node).size()), 2);
  QCOMPARE(listWidget->item(1)->checkState(), Qt::Checked);

  // Unchecking a display hides it, and leaves it hidden when the eye icon shows the node
  listWidget->item(1)->setCheckState(Qt::Unchecked);
  QCOMPARE(secondDisplayNode->GetVisibility(), 0);
  QCOMPARE(secondDisplayNode->GetShowMode(), static_cast<int>(vtkMRMLDisplayNode::ShowIgnore));

  // Removing the selected display leaves the other
  QVERIFY(removeButton->isEnabled());
  removeButton->click();
  QCOMPARE(listWidget->count(), 1);
  std::vector<vtkMRMLVectorFieldDisplayNode*> remainingDisplayNodes = vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(node);
  QCOMPARE(static_cast<int>(remainingDisplayNodes.size()), 1);
  QCOMPARE(remainingDisplayNodes[0], firstDisplayNode);
  QCOMPARE(widget->currentDisplayNode(), firstDisplayNode);
}

// ----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidgetTester::testNoVectorField()
{
  qSlicerVectorFieldRenderingModuleWidget* widget = this->moduleWidget();
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(50.0);
  AddPointScalar(surface, "Scalar", LinearScalar);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode", "Scalar model"));
  modelNode->SetAndObserveMesh(surface);
  // A node without a vector field is not opened for editing here
  QVERIFY(!widget->setEditedNode(modelNode));
  widget->setCurrentNode(modelNode);
  QVERIFY(!widget->findChild<QPushButton*>("AddDisplayNodeButton")->isEnabled());
  QVERIFY(!widget->findChild<QLabel*>("NoVectorFieldLabel")->isHidden());

  // Adding a vector array makes it possible to add a display
  AddPointField(surface, "Velocity", LinearField);
  // Adding an array to the point data does not modify the mesh itself
  surface->Modified();
  QVERIFY(widget->findChild<QPushButton*>("AddDisplayNodeButton")->isEnabled());
  QVERIFY(widget->findChild<QLabel*>("NoVectorFieldLabel")->isHidden());
}

// ----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidgetTester::testEditedNode()
{
  qSlicerVectorFieldRenderingModuleWidget* widget = this->moduleWidget();
  vtkMRMLDisplayableNode* node = this->addNodeWithVectorField("volume");
  vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(node);
  vtkMRMLVectorFieldDisplayNode* secondDisplayNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(node);
  widget->setCurrentNode(nullptr);
  // Editing a display node selects its node and the display node
  QVERIFY(widget->setEditedNode(secondDisplayNode));
  QCOMPARE(widget->currentNode(), node);
  QCOMPARE(widget->currentDisplayNode(), secondDisplayNode);
}

// ----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidgetTester::testShowAsVectorField()
{
  qSlicerSubjectHierarchyVectorFieldRenderingPlugin* plugin =
    qobject_cast<qSlicerSubjectHierarchyVectorFieldRenderingPlugin*>(qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("VectorFieldRendering"));
  QVERIFY(plugin != nullptr);
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  vtkMRMLDisplayableNode* node = this->addNodeWithVectorField("transform");
  // No plugin that owns transforms is registered in this test, so the item is not created automatically
  vtkIdType itemID = shNode->CreateItem(shNode->GetSceneItemID(), node);
  QVERIFY(itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);
  QVERIFY(!plugin->isVectorFieldShown(itemID));

  // Shown as vector field: a display is added, shown
  QVERIFY(plugin->showVectorField(true, itemID));
  std::vector<vtkMRMLVectorFieldDisplayNode*> displayNodes = vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(node);
  QCOMPARE(static_cast<int>(displayNodes.size()), 1);
  vtkMRMLVectorFieldDisplayNode* displayNode = displayNodes[0];
  QCOMPARE(displayNode->GetVisibility(), 1);
  QVERIFY(plugin->isVectorFieldShown(itemID));

  // The eye icon hides it and shows it again
  node->SetDisplayVisibility(0);
  QCOMPARE(displayNode->GetVisibility(), 0);
  QVERIFY(plugin->isVectorFieldShown(itemID));
  node->SetDisplayVisibility(1);
  QCOMPARE(displayNode->GetVisibility(), 1);

  // Not shown as vector field: hidden, and the eye icon does not show it again
  QVERIFY(plugin->showVectorField(false, itemID));
  QCOMPARE(displayNode->GetVisibility(), 0);
  QVERIFY(!plugin->isVectorFieldShown(itemID));
  node->SetDisplayVisibility(0);
  node->SetDisplayVisibility(1);
  QCOMPARE(displayNode->GetVisibility(), 0);

  // Shown again: the same display, no new one
  QVERIFY(plugin->showVectorField(true, itemID));
  QCOMPARE(static_cast<int>(vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(node).size()), 1);
  QCOMPARE(displayNode->GetVisibility(), 1);

  // Items without a vector field are left alone
  vtkIdType folderItemID = shNode->CreateFolderItem(shNode->GetSceneItemID(), "Folder");
  QVERIFY(!plugin->showVectorField(true, folderItemID));
}

// ----------------------------------------------------------------------------
int qSlicerVectorFieldRenderingModuleWidgetTest(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  qSlicerApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qSlicerVectorFieldRenderingModuleWidgetTester tc;
  return QTest::qExec(&tc, argc, argv);
}

#include "qSlicerVectorFieldRenderingModuleWidgetTest.moc"
