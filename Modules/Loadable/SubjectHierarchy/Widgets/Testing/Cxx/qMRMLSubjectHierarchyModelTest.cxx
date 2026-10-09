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

// Qt includes
#include <QSignalSpy>
#include <QStandardItem>

// CTK includes
#include <ctkTest.h>

// Subject hierarchy includes
#include "qMRMLSubjectHierarchyModel.h"
#include "qSlicerSubjectHierarchyFolderPlugin.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// ----------------------------------------------------------------------------
class qMRMLSubjectHierarchyModelTester : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();
  void cleanup();

  void testStructureAfterSetScene();
  void testRebuildSignals();
  void testItemsAddedOutsideBatch();
  void testItemsAddedInBatch();
  void testBatchWithoutChange();
  void testRemoveReparentReorderInBatch();
  void testModifiedEventInBatch();
  void testItemModifiedInBatch();
  void testExpandRequests();
  void testImportIntoEmptyScene();
  void testImportIntoPopulatedScene();
  void testItemsAddedByEndImportObserver();
  void testClearScene();

private:
  /// Create folders with model nodes (and a collapsed sub-folder with models in each folder)
  static void populateSubjectHierarchy(vtkMRMLScene* scene, int folderCount, int modelsPerFolder, const QString& namePrefix = "");
  /// Check that the model exactly matches the subject hierarchy node (structure, order, columns, cache)
  static void verifyModelMatchesSubjectHierarchy(qMRMLSubjectHierarchyModel& model, vtkMRMLSubjectHierarchyNode* shNode);
  static int itemCount(vtkMRMLSubjectHierarchyNode* shNode);
  static std::string sceneToXML(vtkMRMLScene* scene);
};

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::initTestCase()
{
  // Register the folder plugin so that folder items have an owner plugin (like in the application).
  // Data node items are owned by the default plugin.
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyFolderPlugin());
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::cleanup()
{
  // The plugin handler is a singleton that stores the scene, make sure it does not refer to a deleted scene
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(nullptr);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::populateSubjectHierarchy(vtkMRMLScene* scene, int folderCount, int modelsPerFolder, const QString& namePrefix /*=""*/)
{
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  for (int folderIndex = 0; folderIndex < folderCount; ++folderIndex)
  {
    vtkIdType folderID = shNode->CreateFolderItem(shNode->GetSceneItemID(), QString("%1Folder%2").arg(namePrefix).arg(folderIndex).toStdString());
    shNode->SetItemExpanded(folderID, folderIndex % 2 == 0);
    for (int modelIndex = 0; modelIndex < modelsPerFolder; ++modelIndex)
    {
      std::string modelName = QString("%1Model%2_%3").arg(namePrefix).arg(folderIndex).arg(modelIndex).toStdString();
      vtkMRMLNode* modelNode = scene->AddNewNodeByClass("vtkMRMLModelNode", modelName);
      shNode->CreateItem(folderID, modelNode, "Default");
    }
    vtkIdType subFolderID = shNode->CreateFolderItem(folderID, QString("%1SubFolder%2").arg(namePrefix).arg(folderIndex).toStdString());
    shNode->SetItemExpanded(subFolderID, false);
    for (int modelIndex = 0; modelIndex < 2; ++modelIndex)
    {
      std::string modelName = QString("%1SubModel%2_%3").arg(namePrefix).arg(folderIndex).arg(modelIndex).toStdString();
      vtkMRMLNode* modelNode = scene->AddNewNodeByClass("vtkMRMLModelNode", modelName);
      shNode->CreateItem(subFolderID, modelNode, "Default");
    }
  }
}

// ----------------------------------------------------------------------------
int qMRMLSubjectHierarchyModelTester::itemCount(vtkMRMLSubjectHierarchyNode* shNode)
{
  std::vector<vtkIdType> allItemIDs;
  shNode->GetItemChildren(shNode->GetSceneItemID(), allItemIDs, true);
  return static_cast<int>(allItemIDs.size());
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::verifyModelMatchesSubjectHierarchy(qMRMLSubjectHierarchyModel& model, vtkMRMLSubjectHierarchyNode* shNode)
{
  QVERIFY(shNode != nullptr);
  QStandardItem* sceneItem = model.subjectHierarchySceneItem();
  QVERIFY(sceneItem != nullptr);
  QCOMPARE(model.subjectHierarchyItemFromItem(sceneItem), shNode->GetSceneItemID());

  const int columnCount = model.columnCount();
  QVERIFY(columnCount >= 6);

  std::vector<vtkIdType> allItemIDs;
  shNode->GetItemChildren(shNode->GetSceneItemID(), allItemIDs, true);
  for (vtkIdType itemID : allItemIDs)
  {
    QModelIndex index = model.indexFromSubjectHierarchyItem(itemID);
    QVERIFY2(index.isValid(), qPrintable(QString("Item %1 (%2) not found in model").arg(itemID).arg(shNode->GetItemName(itemID).c_str())));
    QCOMPARE(model.subjectHierarchyItemFromIndex(index), itemID);
    // Parent
    vtkIdType parentID = shNode->GetItemParent(itemID);
    QCOMPARE(model.subjectHierarchyItemFromIndex(index.parent()), parentID);
    // Row (position under parent)
    QCOMPARE(index.row(), model.subjectHierarchyItemIndex(itemID));
    // All columns have an item with the ID role set
    QStandardItem* parentItem = model.itemFromIndex(index.parent());
    QVERIFY(parentItem != nullptr);
    for (int column = 0; column < columnCount; ++column)
    {
      QStandardItem* item = parentItem->child(index.row(), column);
      QVERIFY2(item != nullptr, qPrintable(QString("Missing item in column %1 for item %2").arg(column).arg(itemID)));
      QCOMPARE(item->data(qMRMLSubjectHierarchyModel::SubjectHierarchyItemIDRole).toLongLong(), static_cast<qlonglong>(itemID));
      QCOMPARE(model.itemFromSubjectHierarchyItem(itemID, column), item);
    }
    // Name column shows the item name
    QCOMPARE(model.itemFromSubjectHierarchyItem(itemID, model.nameColumn())->text(), QString::fromStdString(shNode->GetItemName(itemID)));
    // Number of children
    std::vector<vtkIdType> childIDs;
    shNode->GetItemChildren(itemID, childIDs, false);
    QCOMPARE(model.rowCount(index), static_cast<int>(childIDs.size()));
  }
  // Number of top-level rows
  std::vector<vtkIdType> topLevelItemIDs;
  shNode->GetItemChildren(shNode->GetSceneItemID(), topLevelItemIDs, false);
  QCOMPARE(model.rowCount(sceneItem->index()), static_cast<int>(topLevelItemIDs.size()) + (model.noneEnabled() ? 1 : 0));
}

// ----------------------------------------------------------------------------
std::string qMRMLSubjectHierarchyModelTester::sceneToXML(vtkMRMLScene* scene)
{
  scene->SetSaveToXMLString(1);
  scene->Commit();
  scene->SetSaveToXMLString(0);
  return scene->GetSceneXMLString();
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testStructureAfterSetScene()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 5, 4);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  QCOMPARE(this->itemCount(shNode), 5 * (1 + 4 + 1 + 2));

  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);
  QCOMPARE(model.mrmlScene(), scene.GetPointer());
  QCOMPARE(model.subjectHierarchyNode(), shNode);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);

  // With None item enabled
  qMRMLSubjectHierarchyModel modelWithNone;
  modelWithNone.setNoneEnabled(true);
  modelWithNone.setMRMLScene(scene);
  this->verifyModelMatchesSubjectHierarchy(modelWithNone, shNode);
  QStandardItem* noneItem = modelWithNone.subjectHierarchySceneItem()->child(0, modelWithNone.nameColumn());
  QVERIFY(noneItem != nullptr);
  QCOMPARE(noneItem->text(), modelWithNone.noneDisplay());
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testRebuildSignals()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 10, 10);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyModel model;
  QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
  QSignalSpy updatedSpy(&model, SIGNAL(subjectHierarchyUpdated()));
  model.setMRMLScene(scene);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
  QCOMPARE(updatedSpy.count(), 1);
  // The items are inserted in bulk: one insertion for the scene item and one for all the top-level rows
  // (and not one for each of the 140 items)
  QVERIFY2(rowsInsertedSpy.count() <= 3, qPrintable(QString("rowsInserted emitted %1 times").arg(rowsInsertedSpy.count())));
  // All items are available when the rows are inserted
  bool topLevelRowsInserted = false;
  for (const QList<QVariant>& arguments : rowsInsertedSpy)
  {
    QModelIndex parent = arguments[0].toModelIndex();
    if (parent == model.subjectHierarchySceneIndex())
    {
      topLevelRowsInserted = true;
      QCOMPARE(arguments[2].toInt() - arguments[1].toInt() + 1, 10);
    }
  }
  QVERIFY(topLevelRowsInserted);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testItemsAddedOutsideBatch()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);
  QSignalSpy updatedSpy(&model, SIGNAL(subjectHierarchyUpdated()));

  // Items added outside batch processing appear immediately, without rebuilding the model
  vtkIdType folderID = shNode->CreateFolderItem(shNode->GetSceneItemID(), "NewFolder");
  QVERIFY(model.indexFromSubjectHierarchyItem(folderID).isValid());
  vtkMRMLNode* modelNode = scene->AddNewNodeByClass("vtkMRMLModelNode", "NewModel");
  vtkIdType modelItemID = shNode->CreateItem(folderID, modelNode, "Default");
  QVERIFY(model.indexFromSubjectHierarchyItem(modelItemID).isValid());
  QCOMPARE(model.indexFromSubjectHierarchyItem(modelItemID).parent(), model.indexFromSubjectHierarchyItem(folderID));
  // Insert in the middle of a branch
  vtkIdType firstFolderID = shNode->GetItemByPositionUnderParent(shNode->GetSceneItemID(), 0);
  vtkMRMLNode* modelNode2 = scene->AddNewNodeByClass("vtkMRMLModelNode", "NewModel2");
  vtkIdType modelItem2ID = shNode->CreateItem(firstFolderID, modelNode2, "Default");
  vtkIdType firstChildID = shNode->GetItemByPositionUnderParent(firstFolderID, 0);
  QVERIFY(shNode->MoveItem(modelItem2ID, firstChildID));
  QCOMPARE(updatedSpy.count(), 0);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testItemsAddedInBatch()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);
  QSignalSpy updatedSpy(&model, SIGNAL(subjectHierarchyUpdated()));
  QSignalSpy aboutToBeUpdatedSpy(&model, SIGNAL(subjectHierarchyAboutToBeUpdated()));
  QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
  const int rowCountBefore = model.rowCount(model.subjectHierarchySceneIndex());

  scene->StartState(vtkMRMLScene::BatchProcessState);
  QCOMPARE(aboutToBeUpdatedSpy.count(), 1);
  this->populateSubjectHierarchy(scene, 3, 5, "Batch");
  vtkIdType newFolderID = shNode->GetItemByName("BatchFolder0");
  QVERIFY(newFolderID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);
  // Items are not added to the model one by one during batch processing
  QVERIFY(!model.indexFromSubjectHierarchyItem(newFolderID).isValid());
  QCOMPARE(model.rowCount(model.subjectHierarchySceneIndex()), rowCountBefore);
  QCOMPARE(rowsInsertedSpy.count(), 0);
  QCOMPARE(updatedSpy.count(), 0);
  // Nested batch processing does not trigger an update
  scene->StartState(vtkMRMLScene::BatchProcessState);
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QCOMPARE(updatedSpy.count(), 0);
  scene->EndState(vtkMRMLScene::BatchProcessState);

  // The model is rebuilt once at the end of the batch processing
  QCOMPARE(updatedSpy.count(), 1);
  QVERIFY2(rowsInsertedSpy.count() <= 2, qPrintable(QString("rowsInserted emitted %1 times").arg(rowsInsertedSpy.count())));
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testBatchWithoutChange()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);
  QSignalSpy updatedSpy(&model, SIGNAL(subjectHierarchyUpdated()));
  QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)));

  // Batch processing that does not change the subject hierarchy does not rebuild the model
  scene->StartState(vtkMRMLScene::BatchProcessState);
  scene->AddNewNodeByClass("vtkMRMLModelNode", "ModelWithoutItem");
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QCOMPARE(updatedSpy.count(), 0);
  QCOMPARE(rowsRemovedSpy.count(), 0);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testRemoveReparentReorderInBatch()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 3, 4);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);
  QSignalSpy updatedSpy(&model, SIGNAL(subjectHierarchyUpdated()));

  vtkIdType folder0 = shNode->GetItemByName("Folder0");
  vtkIdType folder1 = shNode->GetItemByName("Folder1");
  vtkIdType folder2 = shNode->GetItemByName("Folder2");
  vtkIdType model00 = shNode->GetItemByName("Model0_0");
  vtkIdType model01 = shNode->GetItemByName("Model0_1");
  vtkIdType model10 = shNode->GetItemByName("Model1_0");
  vtkIdType model13 = shNode->GetItemByName("Model1_3");

  // Remove (items only: removing the data nodes relies on the subject hierarchy plugin logic,
  // which is not available in this test)
  scene->StartState(vtkMRMLScene::BatchProcessState);
  QVERIFY(shNode->RemoveItem(model00, false, true));
  QVERIFY(shNode->RemoveItem(folder2, false, true));
  // Reparent
  shNode->SetItemParent(model01, folder1);
  // Reorder
  QVERIFY(shNode->MoveItem(model13, model10));
  QCOMPARE(updatedSpy.count(), 0);
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QCOMPARE(updatedSpy.count(), 1);

  QVERIFY(!model.indexFromSubjectHierarchyItem(model00).isValid());
  QVERIFY(!model.indexFromSubjectHierarchyItem(folder2).isValid());
  QCOMPARE(model.subjectHierarchyItemFromIndex(model.indexFromSubjectHierarchyItem(model01).parent()), folder1);
  QCOMPARE(model.indexFromSubjectHierarchyItem(model13).row(), 0);
  QCOMPARE(model.indexFromSubjectHierarchyItem(model10).row(), 1);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);

  // The same operations outside batch processing are applied immediately
  vtkIdType model11 = shNode->GetItemByName("Model1_1");
  QVERIFY(shNode->RemoveItem(model11, false, true));
  QVERIFY(!model.indexFromSubjectHierarchyItem(model11).isValid());
  shNode->SetItemParent(model10, folder0);
  QCOMPARE(model.subjectHierarchyItemFromIndex(model.indexFromSubjectHierarchyItem(model10).parent()), folder0);
  QCOMPARE(updatedSpy.count(), 1);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testModifiedEventInBatch()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);
  QSignalSpy updatedSpy(&model, SIGNAL(subjectHierarchyUpdated()));

  // Modification of the subject hierarchy node without item events (e.g., when the node content is copied)
  // triggers a rebuild at the end of the batch processing
  scene->StartState(vtkMRMLScene::BatchProcessState);
  shNode->Modified();
  QCOMPARE(updatedSpy.count(), 0);
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QCOMPARE(updatedSpy.count(), 1);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);

  // Modified event outside batch processing does not rebuild the model
  shNode->Modified();
  QCOMPARE(updatedSpy.count(), 1);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testItemModifiedInBatch()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyModel model;
  model.setVisibilityColumn(1);
  model.setMRMLScene(scene);
  QSignalSpy updatedSpy(&model, SIGNAL(subjectHierarchyUpdated()));
  QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)));
  QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));

  vtkIdType modelItemID = shNode->GetItemByName("Model0_0");
  QVERIFY(modelItemID);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(modelItemID));
  QVERIFY(modelNode);
  modelNode->CreateDefaultDisplayNodes();
  QCOMPARE(model.itemFromSubjectHierarchyItem(modelItemID, model.visibilityColumn())->data(qMRMLSubjectHierarchyModel::VisibilityRole).toInt(), 1);
  QCOMPARE(updatedSpy.count(), 0);

  // Item modifications during batch processing are applied to the existing model items when the batch
  // processing ends. The model is not rebuilt (that would reset the scroll position, selection, and cell
  // widgets in the views).
  scene->StartState(vtkMRMLScene::BatchProcessState);
  shNode->SetItemName(modelItemID, "RenamedModel");
  modelNode->GetDisplayNode()->SetVisibility(0);
  QCOMPARE(model.itemFromSubjectHierarchyItem(modelItemID)->text(), QString("Model0_0"));
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QCOMPARE(updatedSpy.count(), 0);
  QCOMPARE(rowsRemovedSpy.count(), 0);
  QCOMPARE(rowsInsertedSpy.count(), 0);
  QCOMPARE(model.itemFromSubjectHierarchyItem(modelItemID)->text(), QString("RenamedModel"));
  QCOMPARE(model.itemFromSubjectHierarchyItem(modelItemID, model.visibilityColumn())->data(qMRMLSubjectHierarchyModel::VisibilityRole).toInt(), 0);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);

  // Folder visibility is applied to the whole branch in a batch (by the folder display node)
  vtkIdType folderItemID = shNode->GetItemByName("Folder0");
  QVERIFY(folderItemID);
  qSlicerSubjectHierarchyAbstractPlugin* plugin = qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Folder");
  qSlicerSubjectHierarchyFolderPlugin* folderPlugin = qobject_cast<qSlicerSubjectHierarchyFolderPlugin*>(plugin);
  QVERIFY(folderPlugin);
  folderPlugin->setDisplayVisibility(folderItemID, 1); // creates the folder display node
  updatedSpy.clear();
  rowsRemovedSpy.clear();
  rowsInsertedSpy.clear();
  folderPlugin->setDisplayVisibility(folderItemID, 0);
  QCOMPARE(updatedSpy.count(), 0);
  QCOMPARE(rowsRemovedSpy.count(), 0);
  QCOMPARE(rowsInsertedSpy.count(), 0);
  QCOMPARE(model.itemFromSubjectHierarchyItem(folderItemID, model.visibilityColumn())->data(qMRMLSubjectHierarchyModel::VisibilityRole).toInt(), 0);
  folderPlugin->setDisplayVisibility(folderItemID, 1);
  QCOMPARE(model.itemFromSubjectHierarchyItem(folderItemID, model.visibilityColumn())->data(qMRMLSubjectHierarchyModel::VisibilityRole).toInt(), 1);
  QCOMPARE(updatedSpy.count(), 0);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testExpandRequests()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 4, 3);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyModel model;
  QList<vtkIdType> expandedItems;
  QList<vtkIdType> collapsedItems;
  bool allRequestsValid = true;
  // Expand/collapse requests must only be emitted when the item is already in the model
  QObject::connect(&model,
                   &qMRMLSubjectHierarchyModel::requestExpandItem,
                   [&](vtkIdType itemID)
                   {
                     expandedItems << itemID;
                     allRequestsValid &= model.indexFromSubjectHierarchyItem(itemID).isValid();
                   });
  QObject::connect(&model,
                   &qMRMLSubjectHierarchyModel::requestCollapseItem,
                   [&](vtkIdType itemID)
                   {
                     collapsedItems << itemID;
                     allRequestsValid &= model.indexFromSubjectHierarchyItem(itemID).isValid();
                   });

  model.setMRMLScene(scene);
  QVERIFY(allRequestsValid);
  std::vector<vtkIdType> allItemIDs;
  shNode->GetItemChildren(shNode->GetSceneItemID(), allItemIDs, true);
  int expectedExpanded = 0;
  int expectedCollapsed = 0;
  for (vtkIdType itemID : allItemIDs)
  {
    if (shNode->GetItemExpanded(itemID))
    {
      ++expectedExpanded;
      QVERIFY(expandedItems.contains(itemID));
    }
    else
    {
      ++expectedCollapsed;
      QVERIFY(collapsedItems.contains(itemID));
    }
  }
  // One request per item (not one per column or per rebuild pass)
  QCOMPARE(expandedItems.size(), expectedExpanded);
  QCOMPARE(collapsedItems.size(), expectedCollapsed);

  // Rebuild (at the end of a batch) emits the requests again
  expandedItems.clear();
  collapsedItems.clear();
  scene->StartState(vtkMRMLScene::BatchProcessState);
  shNode->CreateFolderItem(shNode->GetSceneItemID(), "BatchFolder");
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QVERIFY(allRequestsValid);
  QCOMPARE(expandedItems.size() + collapsedItems.size(), static_cast<int>(allItemIDs.size()) + 1);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testImportIntoEmptyScene()
{
  // Create the scene to import
  vtkNew<vtkMRMLScene> sourceScene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(sourceScene);
  this->populateSubjectHierarchy(sourceScene, 3, 3, "Imported");
  const int importedItemCount = this->itemCount(sourceScene->GetSubjectHierarchyNode());
  std::string xml = this->sceneToXML(sourceScene);

  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);
  QSignalSpy updatedSpy(&model, SIGNAL(subjectHierarchyUpdated()));

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  // The imported subject hierarchy node is merged into the existing one (in the application this is done
  // by the subject hierarchy plugin logic at the end of the import)
  QCOMPARE(vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene), shNode);
  QCOMPARE(scene->GetSubjectHierarchyNode(), shNode);
  QCOMPARE(model.subjectHierarchyNode(), shNode);
  QCOMPARE(this->itemCount(shNode), importedItemCount);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
  QVERIFY(model.indexFromSubjectHierarchyItem(shNode->GetItemByName("ImportedModel2_2")).isValid());
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testImportIntoPopulatedScene()
{
  vtkNew<vtkMRMLScene> sourceScene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(sourceScene);
  this->populateSubjectHierarchy(sourceScene, 3, 3, "Imported");
  const int importedItemCount = this->itemCount(sourceScene->GetSubjectHierarchyNode());
  std::string xml = this->sceneToXML(sourceScene);

  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  const int existingItemCount = this->itemCount(shNode);
  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  QCOMPARE(vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene), shNode);
  QCOMPARE(model.subjectHierarchyNode(), shNode);
  QCOMPARE(this->itemCount(shNode), existingItemCount + importedItemCount);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
  QVERIFY(model.indexFromSubjectHierarchyItem(shNode->GetItemByName("Model1_1")).isValid());
  QVERIFY(model.indexFromSubjectHierarchyItem(shNode->GetItemByName("ImportedModel2_2")).isValid());

  // Import once more, then the model must contain all the items exactly once
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  QCOMPARE(vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene), shNode);
  QCOMPARE(this->itemCount(shNode), existingItemCount + 2 * importedItemCount);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
}

// ----------------------------------------------------------------------------
namespace
{
// Mimics the subject hierarchy plugin logic: resolves the subject hierarchy (merges the imported items)
// at the end of the import, with a priority that makes it run before the model
void resolveSubjectHierarchyCallback(vtkObject* caller, unsigned long vtkNotUsed(eid), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
  vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(vtkMRMLScene::SafeDownCast(caller));
}
} // namespace

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testItemsAddedByEndImportObserver()
{
  vtkNew<vtkMRMLScene> sourceScene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(sourceScene);
  this->populateSubjectHierarchy(sourceScene, 10, 10, "Imported");
  const int importedItemCount = this->itemCount(sourceScene->GetSubjectHierarchyNode());
  std::string xml = this->sceneToXML(sourceScene);

  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  const int existingItemCount = this->itemCount(shNode);
  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);

  vtkNew<vtkCallbackCommand> resolveCallback;
  resolveCallback->SetCallback(resolveSubjectHierarchyCallback);
  scene->AddObserver(vtkMRMLScene::EndImportEvent, resolveCallback, 10.0);

  QSignalSpy updatedSpy(&model, SIGNAL(subjectHierarchyUpdated()));
  QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  QCOMPARE(scene->GetSubjectHierarchyNode(), shNode);
  QCOMPARE(this->itemCount(shNode), existingItemCount + importedItemCount);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
  // Items added by the end-of-import observer are not inserted one by one, but in a single rebuild
  // (at the end of the import, so that other observers can already use the updated model)
  QCOMPARE(updatedSpy.count(), 1);
  QVERIFY2(rowsInsertedSpy.count() <= 2, qPrintable(QString("rowsInserted emitted %1 times").arg(rowsInsertedSpy.count())));

  // Items added after the import are inserted immediately
  QSignalSpy rowsInsertedAfterImportSpy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
  vtkIdType folderID = shNode->CreateFolderItem(shNode->GetSceneItemID(), "AfterImport");
  QVERIFY(model.indexFromSubjectHierarchyItem(folderID).isValid());
  QCOMPARE(rowsInsertedAfterImportSpy.count(), 1);
  QCOMPARE(updatedSpy.count(), 1);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyModelTester::testClearScene()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 3, 3);

  qMRMLSubjectHierarchyModel model;
  model.setMRMLScene(scene);
  QVERIFY(model.rowCount(model.subjectHierarchySceneIndex()) > 0);

  scene->Clear(0);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  QVERIFY(shNode != nullptr);
  // The plugin handler needs to observe the new subject hierarchy node (in the application it is done by the plugin logic)
  qSlicerSubjectHierarchyPluginHandler::instance()->observeSubjectHierarchyNode(shNode);
  QCOMPARE(model.subjectHierarchyNode(), shNode);
  QCOMPARE(model.rowCount(model.subjectHierarchySceneIndex()), 0);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);

  // The model works after clearing the scene
  this->populateSubjectHierarchy(scene, 2, 2, "AfterClear");
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
  scene->StartState(vtkMRMLScene::BatchProcessState);
  this->populateSubjectHierarchy(scene, 2, 2, "AfterClearBatch");
  scene->EndState(vtkMRMLScene::BatchProcessState);
  this->verifyModelMatchesSubjectHierarchy(model, shNode);
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLSubjectHierarchyModelTest)
#include "qMRMLSubjectHierarchyModelTest.moc"
