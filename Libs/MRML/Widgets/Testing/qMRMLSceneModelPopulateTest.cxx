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

// Tests populating scene models (qMRMLSceneModel and subclasses) from a scene in bulk,
// which happens when the scene is set, imported, or closed.
// Note: ctkModelTester is not used in these tests, because it does not support dataChanged signals
// that cover multiple rows (emitted when the rows are inserted in bulk).

// Qt includes
#include <QSignalSpy>
#include <QStandardItem>

// CTK includes
#include <ctkTest.h>

// qMRML includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLSceneCategoryModel.h"
#include "qMRMLSceneColorTableModel.h"
#include "qMRMLSceneDisplayableModel.h"
#include "qMRMLSceneHierarchyModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLTreeView.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

Q_DECLARE_METATYPE(vtkMRMLNode*)

// ----------------------------------------------------------------------------
class qMRMLSceneModelPopulateTester : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();
  void testPopulateMatchesIncrementalInsertion();
  void testPopulateMatchesIncrementalInsertion_data();
  void testAllNodesPresent();
  void testAllNodesPresent_data();
  void testColumns();
  void testExtraItems();
  void testImport();
  void testParentAfterChild();
  void testRowsInsertedConsumer();
  void testNodeModifiedAfterPopulate();
  void testClear();
  void testNodeComboBox();
  void testTreeView();

private:
  enum ModelType
  {
    Base,
    Transform,
    Displayable,
    Hierarchy,
    Category,
    ColorTable
  };
  static qMRMLSceneModel* createModel(int type, QObject* parent);
  /// Add nodes of various types (with transform and model hierarchy parents) to the scene
  static void populateScene(vtkMRMLScene* scene, const QString& namePrefix, int modelCount);
  /// Verify that the two models have the same item tree
  static void compareItems(QStandardItem* itemA, QStandardItem* itemB);
  /// Verify that all the scene nodes are in the model, exactly once, under the item of their parent node
  static void verifyAllNodesPresent(qMRMLSceneModel& model, vtkMRMLScene* scene);
  static int countNodeItems(QStandardItem* parentItem);
  static std::string sceneToXML(vtkMRMLScene* scene);
};

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::initTestCase()
{
  // Needed for QSignalSpy to store the node pointer argument of the nodeAdded signal
  qRegisterMetaType<vtkMRMLNode*>("vtkMRMLNode*");
}

// ----------------------------------------------------------------------------
qMRMLSceneModel* qMRMLSceneModelPopulateTester::createModel(int type, QObject* parent)
{
  qMRMLSceneModel* model = nullptr;
  switch (type)
  {
    case Transform: model = new qMRMLSceneTransformModel(parent); break;
    case Displayable: model = new qMRMLSceneDisplayableModel(parent); break;
    case Hierarchy: model = new qMRMLSceneHierarchyModel(parent); break;
    case Category: model = new qMRMLSceneCategoryModel(parent); break;
    case ColorTable: model = new qMRMLSceneColorTableModel(parent); break;
    default: model = new qMRMLSceneModel(parent); break;
  }
  // Observe all nodes (in the application the nodes are observed by the sort/filter proxy model),
  // so that the items are moved when the parent of a node changes
  model->setListenNodeModifiedEvent(qMRMLSceneModel::AllNodes);
  return model;
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::populateScene(vtkMRMLScene* scene, const QString& namePrefix, int modelCount)
{
  scene->AddNewNodeByClass("vtkMRMLTableNode", (namePrefix + "Table").toStdString());
  // Transforms (nested)
  vtkMRMLLinearTransformNode* transform1 =
    vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode", (namePrefix + "Transform1").toStdString()));
  vtkMRMLLinearTransformNode* transform2 =
    vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode", (namePrefix + "Transform2").toStdString()));
  transform2->SetAndObserveTransformNodeID(transform1->GetID());
  // Model hierarchy (nested): hierarchy nodes are added right before their associated model
  std::string hierarchy1Name = (namePrefix + "Hierarchy1").toStdString();
  std::string hierarchy2Name = (namePrefix + "Hierarchy2").toStdString();
  vtkMRMLModelHierarchyNode* hierarchy1 = vtkMRMLModelHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelHierarchyNode", hierarchy1Name));
  vtkMRMLModelHierarchyNode* hierarchy2 = vtkMRMLModelHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelHierarchyNode", hierarchy2Name));
  hierarchy2->SetParentNodeID(hierarchy1->GetID());
  // Color tables with categories
  for (int i = 0; i < 3; ++i)
  {
    vtkMRMLColorTableNode* colorNode =
      vtkMRMLColorTableNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLColorTableNode", QString("%1Color%2").arg(namePrefix).arg(i).toStdString()));
    colorNode->SetTypeToUser();
    colorNode->SetAttribute("Category", i == 0 ? "CategoryA" : "CategoryB");
  }
  for (int i = 0; i < modelCount; ++i)
  {
    vtkMRMLModelHierarchyNode* modelHierarchy = nullptr;
    if (i % 4 == 1)
    {
      modelHierarchy = vtkMRMLModelHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelHierarchyNode", //
                                                                                        QString("%1ModelHierarchy%2").arg(namePrefix).arg(i).toStdString()));
      modelHierarchy->SetParentNodeID(i % 8 == 1 ? hierarchy1->GetID() : hierarchy2->GetID());
      modelHierarchy->SetHideFromEditors(1);
    }
    std::string modelName = QString("%1Model%2").arg(namePrefix).arg(i).toStdString();
    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode", modelName));
    modelNode->CreateDefaultDisplayNodes();
    if (modelHierarchy)
    {
      modelHierarchy->SetAssociatedNodeID(modelNode->GetID());
    }
    if (i % 3 == 0)
    {
      modelNode->SetAndObserveTransformNodeID(transform1->GetID());
    }
    else if (i % 3 == 1)
    {
      modelNode->SetAndObserveTransformNodeID(transform2->GetID());
    }
    if (i % 5 == 0)
    {
      modelNode->SetAttribute("Category", "CategoryA");
    }
  }
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::compareItems(QStandardItem* itemA, QStandardItem* itemB)
{
  QVERIFY((itemA == nullptr) == (itemB == nullptr));
  if (!itemA)
  {
    return;
  }
  QCOMPARE(itemA->data(qMRMLSceneModel::UIDRole).toString(), itemB->data(qMRMLSceneModel::UIDRole).toString());
  QCOMPARE(itemA->text(), itemB->text());
  QCOMPARE(itemA->flags(), itemB->flags());
  QCOMPARE(itemA->rowCount(), itemB->rowCount());
  QCOMPARE(itemA->columnCount(), itemB->columnCount());
  for (int row = 0; row < itemA->rowCount(); ++row)
  {
    for (int column = 0; column < itemA->columnCount(); ++column)
    {
      compareItems(itemA->child(row, column), itemB->child(row, column));
    }
  }
}

// ----------------------------------------------------------------------------
int qMRMLSceneModelPopulateTester::countNodeItems(QStandardItem* parentItem)
{
  int count = 0;
  for (int row = 0; row < parentItem->rowCount(); ++row)
  {
    QStandardItem* child = parentItem->child(row, 0);
    if (!child)
    {
      continue;
    }
    if (child->data(qMRMLSceneModel::PointerRole).isValid())
    {
      ++count;
    }
    count += countNodeItems(child);
  }
  return count;
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::verifyAllNodesPresent(qMRMLSceneModel& model, vtkMRMLScene* scene)
{
  QStandardItem* sceneItem = model.mrmlSceneItem();
  QVERIFY(sceneItem != nullptr);
  const int columnCount = model.columnCount();
  vtkMRMLNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  int nodeCount = 0;
  for (scene->GetNodes()->InitTraversal(it); (node = (vtkMRMLNode*)scene->GetNodes()->GetNextItemAsObject(it));)
  {
    ++nodeCount;
    QModelIndex index = model.indexFromNode(node);
    QVERIFY2(index.isValid(), qPrintable(QString("Node %1 not found in model").arg(node->GetID())));
    QCOMPARE(model.mrmlNodeFromIndex(index), node);
    QStandardItem* item = model.itemFromNode(node);
    QVERIFY(item != nullptr);
    QCOMPARE(item->text(), QString(node->GetName()));
    // Parent item
    vtkMRMLNode* parentNode = model.parentNode(node);
    QStandardItem* expectedParentItem = (parentNode ? model.itemFromNode(parentNode) : sceneItem);
    QCOMPARE(item->parent(), expectedParentItem);
    // All columns
    for (int column = 0; column < columnCount; ++column)
    {
      QStandardItem* columnItem = item->parent()->child(item->row(), column);
      QVERIFY2(columnItem != nullptr, qPrintable(QString("Missing item in column %1 for node %2").arg(column).arg(node->GetID())));
      QCOMPARE(columnItem->data(qMRMLSceneModel::UIDRole).toString(), QString(node->GetID()));
      QCOMPARE(model.itemFromNode(node, column), columnItem);
    }
  }
  // Each node has exactly one item (and category items do not count)
  QCOMPARE(countNodeItems(sceneItem), nodeCount);
}

// ----------------------------------------------------------------------------
std::string qMRMLSceneModelPopulateTester::sceneToXML(vtkMRMLScene* scene)
{
  scene->SetSaveToXMLString(1);
  scene->Commit();
  scene->SetSaveToXMLString(0);
  return scene->GetSceneXMLString();
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testPopulateMatchesIncrementalInsertion_data()
{
  QTest::addColumn<int>("modelType");
  QTest::newRow("base") << static_cast<int>(Base);
  QTest::newRow("transform") << static_cast<int>(Transform);
  QTest::newRow("displayable") << static_cast<int>(Displayable);
  QTest::newRow("hierarchy") << static_cast<int>(Hierarchy);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testPopulateMatchesIncrementalInsertion()
{
  QFETCH(int, modelType);
  vtkNew<vtkMRMLScene> scene;

  // Reference model: nodes are inserted one by one as they are added to the scene
  qMRMLSceneModel* incrementalModel = this->createModel(modelType, this);
  incrementalModel->setMRMLScene(scene);
  this->populateScene(scene, "", 40);
  this->verifyAllNodesPresent(*incrementalModel, scene);

  // Model populated in bulk from the already populated scene
  qMRMLSceneModel* bulkModel = this->createModel(modelType, this);
  QSignalSpy rowsInsertedSpy(bulkModel, SIGNAL(rowsInserted(QModelIndex, int, int)));
  bulkModel->setMRMLScene(scene);
  this->verifyAllNodesPresent(*bulkModel, scene);
  this->compareItems(incrementalModel->mrmlSceneItem(), bulkModel->mrmlSceneItem());
  // Rows are inserted in bulk: one insertion for the scene item and one for all the top-level rows
  QVERIFY2(rowsInsertedSpy.count() <= 2, qPrintable(QString("rowsInserted emitted %1 times").arg(rowsInsertedSpy.count())));

  // Importing the scene (which repopulates the model) results in the same model
  std::string xml = this->sceneToXML(scene);
  vtkNew<vtkMRMLScene> importedScene;
  qMRMLSceneModel* importModel = this->createModel(modelType, this);
  importModel->setMRMLScene(importedScene);
  importedScene->SetLoadFromXMLString(1);
  importedScene->SetSceneXMLString(xml);
  QVERIFY(importedScene->Import());
  // (importing may add a few extra nodes, such as a subject hierarchy node)
  QVERIFY(importedScene->GetNumberOfNodes() >= scene->GetNumberOfNodes());
  this->verifyAllNodesPresent(*importModel, importedScene);

  delete incrementalModel;
  delete bulkModel;
  delete importModel;
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testAllNodesPresent_data()
{
  QTest::addColumn<int>("modelType");
  QTest::newRow("base") << static_cast<int>(Base);
  QTest::newRow("transform") << static_cast<int>(Transform);
  QTest::newRow("displayable") << static_cast<int>(Displayable);
  QTest::newRow("hierarchy") << static_cast<int>(Hierarchy);
  QTest::newRow("category") << static_cast<int>(Category);
  QTest::newRow("colorTable") << static_cast<int>(ColorTable);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testAllNodesPresent()
{
  QFETCH(int, modelType);
  vtkNew<vtkMRMLScene> scene;
  this->populateScene(scene, "", 30);

  qMRMLSceneModel* model = this->createModel(modelType, this);
  model->setMRMLScene(scene);
  this->verifyAllNodesPresent(*model, scene);

  // Nodes added after populating are inserted one by one
  this->populateScene(scene, "Added", 10);
  this->verifyAllNodesPresent(*model, scene);

  // Import repopulates the model
  std::string xml = this->sceneToXML(scene);
  const int nodeCount = scene->GetNumberOfNodes();
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  // (singleton nodes are not duplicated)
  QVERIFY(scene->GetNumberOfNodes() >= 2 * nodeCount - 2);
  this->verifyAllNodesPresent(*model, scene);

  delete model;
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testColumns()
{
  vtkNew<vtkMRMLScene> scene;
  this->populateScene(scene, "", 20);

  qMRMLSceneDisplayableModel model;
  model.setNameColumn(0);
  model.setIDColumn(1);
  model.setCheckableColumn(2);
  model.setVisibilityColumn(3);
  model.setToolTipNameColumn(4);
  QCOMPARE(model.columnCount(), 5);
  QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
  model.setMRMLScene(scene);
  QVERIFY2(rowsInsertedSpy.count() <= 2, qPrintable(QString("rowsInserted emitted %1 times").arg(rowsInsertedSpy.count())));
  this->verifyAllNodesPresent(model, scene);
  QCOMPARE(model.columnCount(model.mrmlSceneIndex()), 5);

  vtkMRMLNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (scene->GetNodes()->InitTraversal(it); (node = (vtkMRMLNode*)scene->GetNodes()->GetNextItemAsObject(it));)
  {
    QCOMPARE(model.itemFromNode(node, 0)->text(), QString(node->GetName()));
    QCOMPARE(model.itemFromNode(node, 1)->text(), QString(node->GetID()));
    QCOMPARE(model.itemFromNode(node, 2)->checkState(), node->GetSelected() ? Qt::Checked : Qt::Unchecked);
    QCOMPARE(model.itemFromNode(node, 4)->toolTip(), QString(node->GetName()));
  }
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testExtraItems()
{
  vtkNew<vtkMRMLScene> scene;
  this->populateScene(scene, "", 10);

  qMRMLSceneModel model;
  model.setMRMLScene(scene);
  QStandardItem* sceneItem = model.mrmlSceneItem();
  model.setPreItems(QStringList() << "None" << "separator", sceneItem);
  model.setPostItems(QStringList() << "separator" << "Create new node" << "Delete current node", sceneItem);
  QCOMPARE(model.rowCount(model.mrmlSceneIndex()), scene->GetNumberOfNodes() + 5);

  // Import repopulates the node rows, extra items are kept in place
  std::string xml = this->sceneToXML(scene);
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  this->verifyAllNodesPresent(model, scene);
  QCOMPARE(model.rowCount(model.mrmlSceneIndex()), scene->GetNumberOfNodes() + 5);
  QCOMPARE(model.preItems(sceneItem), QStringList() << "None" << "separator");
  QCOMPARE(model.postItems(sceneItem), QStringList() << "separator" << "Create new node" << "Delete current node");
  QCOMPARE(sceneItem->child(0)->text(), QString("None"));
  QCOMPARE(sceneItem->child(1)->data(Qt::AccessibleDescriptionRole).toString(), QString("separator"));
  QVERIFY(sceneItem->child(2)->data(qMRMLSceneModel::PointerRole).isValid());
  const int lastRow = sceneItem->rowCount() - 1;
  QVERIFY(sceneItem->child(lastRow - 3)->data(qMRMLSceneModel::PointerRole).isValid());
  QCOMPARE(sceneItem->child(lastRow - 2)->data(Qt::AccessibleDescriptionRole).toString(), QString("separator"));
  QCOMPARE(sceneItem->child(lastRow - 1)->text(), QString("Create new node"));
  QCOMPARE(sceneItem->child(lastRow)->text(), QString("Delete current node"));
  // Nodes are in scene order between the extra items
  vtkMRMLNode* firstNode = vtkMRMLNode::SafeDownCast(scene->GetNodes()->GetItemAsObject(0));
  QCOMPARE(model.indexFromNode(firstNode).row(), 2);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testImport()
{
  vtkNew<vtkMRMLScene> sourceScene;
  this->populateScene(sourceScene, "Imported", 25);
  std::string xml = this->sceneToXML(sourceScene);

  // Import into empty scene
  vtkNew<vtkMRMLScene> scene;
  qMRMLSceneTransformModel model;
  model.setMRMLScene(scene);
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  // (importing may add a few extra nodes, such as a subject hierarchy node)
  QVERIFY(scene->GetNumberOfNodes() >= sourceScene->GetNumberOfNodes());
  this->verifyAllNodesPresent(model, scene);

  // Import into populated scene (node IDs are renamed during import)
  QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
  QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)));
  const int modelNodeCount = scene->GetNumberOfNodesByClass("vtkMRMLModelNode");
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  QCOMPARE(scene->GetNumberOfNodesByClass("vtkMRMLModelNode"), 2 * modelNodeCount);
  this->verifyAllNodesPresent(model, scene);
  QVERIFY2(rowsInsertedSpy.count() <= 2, qPrintable(QString("rowsInserted emitted %1 times").arg(rowsInsertedSpy.count())));
  QCOMPARE(rowsRemovedSpy.count(), 1);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testParentAfterChild()
{
  // A hierarchy node that is in the scene after its child: the parent item is created when the child is
  // added (so that the child can be placed under it)
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLModelHierarchyNode* child = vtkMRMLModelHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelHierarchyNode", "Child"));
  vtkMRMLModelHierarchyNode* grandChild = vtkMRMLModelHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelHierarchyNode", "GrandChild"));
  vtkMRMLModelHierarchyNode* parent = vtkMRMLModelHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelHierarchyNode", "Parent"));
  grandChild->SetParentNodeID(child->GetID());
  child->SetParentNodeID(parent->GetID());
  this->populateScene(scene, "", 5);

  qMRMLSceneDisplayableModel model;
  QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
  model.setMRMLScene(scene);
  this->verifyAllNodesPresent(model, scene);
  QCOMPARE(model.itemFromNode(child)->parent(), model.itemFromNode(parent));
  QCOMPARE(model.itemFromNode(grandChild)->parent(), model.itemFromNode(child));
  // The parent is placed where its first child would be (before the other nodes)
  QCOMPARE(model.itemFromNode(parent)->row(), 0);
  QVERIFY2(rowsInsertedSpy.count() <= 2, qPrintable(QString("rowsInserted emitted %1 times").arg(rowsInsertedSpy.count())));
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testRowsInsertedConsumer()
{
  // Widgets may look up the nodes from the scene model as soon as the rows are inserted in the sort/filter
  // proxy model (e.g., qSlicerPresetComboBox sets icons from the nodeAdded signal of the node combobox,
  // which is emitted from the rowsInserted signal of the proxy model)
  vtkNew<vtkMRMLScene> scene;
  this->populateScene(scene, "", 15);

  qMRMLSceneDisplayableModel model;
  qMRMLSortFilterProxyModel proxyModel;
  proxyModel.setSourceModel(&model);
  int nodesFound = 0;
  bool allFound = true;
  QObject::connect(&proxyModel,
                   &QAbstractItemModel::rowsInserted,
                   [&](const QModelIndex& parent, int first, int last)
                   {
                     for (int row = first; row <= last; ++row)
                     {
                       vtkMRMLNode* node = proxyModel.mrmlNodeFromIndex(proxyModel.index(row, 0, parent));
                       if (!node)
                       {
                         continue;
                       }
                       ++nodesFound;
                       allFound &= model.indexFromNode(node).isValid();
                       allFound &= (model.itemFromNode(node) == model.itemFromIndex(proxyModel.mapToSource(proxyModel.index(row, 0, parent))));
                       allFound &= (model.itemFromNode(node)->text() == QString(node->GetName()));
                     }
                   });
  model.setMRMLScene(scene);
  // Make the proxy model map the scene item children (nodes under hierarchy nodes are not at the top level)
  QVERIFY(proxyModel.rowCount(proxyModel.mrmlSceneIndex()) > 0);
  QVERIFY(allFound);
  // Rows of the scene item are not mapped by the proxy model until they are requested (above),
  // nodes added afterwards are reported immediately
  this->populateScene(scene, "Added", 5);
  QVERIFY(allFound);
  QVERIFY(nodesFound > 0);
  this->verifyAllNodesPresent(model, scene);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testNodeModifiedAfterPopulate()
{
  vtkNew<vtkMRMLScene> scene;
  this->populateScene(scene, "", 10);

  qMRMLSceneTransformModel model;
  model.setListenNodeModifiedEvent(qMRMLSceneModel::AllNodes);
  model.setMRMLScene(scene);
  this->verifyAllNodesPresent(model, scene);

  // Nodes are observed after bulk population
  vtkMRMLNode* modelNode = scene->GetFirstNodeByName("Model3");
  QVERIFY(modelNode != nullptr);
  modelNode->SetName("RenamedModel3");
  QCOMPARE(model.itemFromNode(modelNode)->text(), QString("RenamedModel3"));

  // Reparenting a node (transform change) after bulk population moves its item
  vtkMRMLTransformableNode* transformable = vtkMRMLTransformableNode::SafeDownCast(modelNode);
  vtkMRMLNode* transform2 = scene->GetFirstNodeByName("Transform2");
  transformable->SetAndObserveTransformNodeID(transform2->GetID());
  QCOMPARE(model.itemFromNode(modelNode)->parent(), model.itemFromNode(transform2));
  transformable->SetAndObserveTransformNodeID(nullptr);
  QCOMPARE(model.itemFromNode(modelNode)->parent(), model.mrmlSceneItem());
  this->verifyAllNodesPresent(model, scene);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testClear()
{
  vtkNew<vtkMRMLScene> scene;
  this->populateScene(scene, "", 10);

  qMRMLSceneModel model;
  model.setMRMLScene(scene);
  model.setPreItems(QStringList() << "None", model.mrmlSceneItem());
  model.setPostItems(QStringList() << "Create new node", model.mrmlSceneItem());
  QCOMPARE(model.rowCount(model.mrmlSceneIndex()), scene->GetNumberOfNodes() + 2);

  scene->Clear(0);
  QCOMPARE(model.rowCount(model.mrmlSceneIndex()), scene->GetNumberOfNodes() + 2);
  this->verifyAllNodesPresent(model, scene);

  this->populateScene(scene, "AfterClear", 5);
  QCOMPARE(model.rowCount(model.mrmlSceneIndex()), scene->GetNumberOfNodes() + 2);
  this->verifyAllNodesPresent(model, scene);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testNodeComboBox()
{
  vtkNew<vtkMRMLScene> scene;
  this->populateScene(scene, "", 12);

  qMRMLNodeComboBox comboBox;
  comboBox.setNodeTypes(QStringList() << "vtkMRMLModelNode");
  comboBox.setNoneEnabled(true);
  QSignalSpy nodeAddedSpy(&comboBox, SIGNAL(nodeAdded(vtkMRMLNode*)));
  comboBox.setMRMLScene(scene);
  QCOMPARE(comboBox.nodeCount(), 12);
  // nodeAdded is emitted once for each node
  QCOMPARE(nodeAddedSpy.count(), 12);
  QSet<vtkMRMLNode*> addedNodes;
  for (const QList<QVariant>& arguments : nodeAddedSpy)
  {
    addedNodes.insert(arguments[0].value<vtkMRMLNode*>());
  }
  QCOMPARE(addedNodes.size(), 12);

  vtkMRMLNode* selectedNode = scene->GetFirstNodeByName("Model5");
  comboBox.setCurrentNode(selectedNode);
  QCOMPARE(comboBox.currentNode(), selectedNode);

  // Import: the model is repopulated
  std::string xml = this->sceneToXML(scene);
  nodeAddedSpy.clear();
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  QCOMPARE(comboBox.nodeCount(), 24);
  QCOMPARE(nodeAddedSpy.count(), 24);
  // The current node can be set to any node after import
  vtkMRMLNode* importedNode = scene->GetFirstNodeByName("Model5_1");
  if (!importedNode)
  {
    importedNode = scene->GetNthNodeByClass(23, "vtkMRMLModelNode");
  }
  QVERIFY(importedNode != nullptr);
  comboBox.setCurrentNode(importedNode);
  QCOMPARE(comboBox.currentNode(), importedNode);
  comboBox.setCurrentNode(selectedNode);
  QCOMPARE(comboBox.currentNode(), selectedNode);

  // Nodes added in a batch are shown immediately (behavior is not changed by bulk population)
  scene->StartState(vtkMRMLScene::BatchProcessState);
  vtkMRMLNode* batchNode = scene->AddNewNodeByClass("vtkMRMLModelNode", "BatchModel");
  QCOMPARE(comboBox.nodeCount(), 25);
  comboBox.setCurrentNode(batchNode);
  QCOMPARE(comboBox.currentNode(), batchNode);
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QCOMPARE(comboBox.nodeCount(), 25);
  QCOMPARE(comboBox.currentNode(), batchNode);

  scene->Clear(0);
  QCOMPARE(comboBox.nodeCount(), 0);
  QVERIFY(comboBox.currentNode() == nullptr);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelPopulateTester::testTreeView()
{
  vtkNew<vtkMRMLScene> scene;
  this->populateScene(scene, "", 12);

  const QStringList sceneModelTypes = QStringList() << "Transform" << "Displayable" << "";
  for (const QString& sceneModelType : sceneModelTypes)
  {
    qMRMLTreeView treeView;
    treeView.setSceneModelType(sceneModelType);
    treeView.setMRMLScene(scene);
    treeView.show();
    qApp->processEvents();
    this->verifyAllNodesPresent(*treeView.sceneModel(), scene);

    std::string xml = this->sceneToXML(scene);
    const int nodeCount = scene->GetNumberOfNodes();
    scene->SetLoadFromXMLString(1);
    scene->SetSceneXMLString(xml);
    QVERIFY(scene->Import());
    qApp->processEvents();
    QVERIFY(scene->GetNumberOfNodes() >= 2 * nodeCount - 2);
    this->verifyAllNodesPresent(*treeView.sceneModel(), scene);
    vtkMRMLNode* node = scene->GetFirstNodeByName("Model7");
    treeView.setCurrentNode(node);
    QCOMPARE(treeView.currentNode(), node);
    scene->Clear(0);
    qApp->processEvents();
    this->populateScene(scene, "", 12);
  }
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLSceneModelPopulateTest)
#include "qMRMLSceneModelPopulateTest.moc"
