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
#include <QApplication>
#include <QColor>
#include <QElapsedTimer>
#include <QPointer>
#include <QSignalSpy>
#include <QToolButton>

// CTK includes
#include <ctkTest.h>

// Subject hierarchy includes
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"
#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSubjectHierarchyTreeView.h"
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

// ----------------------------------------------------------------------------
class qMRMLSubjectHierarchyTreeViewTester : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();
  void cleanup();

  void testDisplayedItems();
  void testExpandedStateAfterRebuild();
  void testButtonsCreatedAndReused();
  void testVisibilityButton();
  void testExpandCollapseItem();
  void testSelectionAcrossBatchAndImport();
  void testSelectItemAddedInBatch();
  void testEditsInLargeScene();
  void testItemsAddedWhileModifyingNode();
  void testExpandedStateAfterMove();

private:
  static void populateSubjectHierarchy(vtkMRMLScene* scene, int folderCount, int modelsPerFolder, const QString& namePrefix = "");
  static std::vector<vtkIdType> allItems(vtkMRMLSubjectHierarchyNode* shNode);
  /// Collect the index widgets of all rows (recursively) in the given column
  static void collectIndexWidgets(qMRMLSubjectHierarchyTreeView& view, const QModelIndex& parent, int column, QHash<vtkIdType, QPointer<QWidget>>& widgets);
  static void verifyButtons(qMRMLSubjectHierarchyTreeView& view, vtkMRMLSubjectHierarchyNode* shNode, QHash<vtkIdType, QPointer<QWidget>>& visibilityButtons);
};

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::initTestCase()
{
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyFolderPlugin());
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::cleanup()
{
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(nullptr);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::populateSubjectHierarchy(vtkMRMLScene* scene, int folderCount, int modelsPerFolder, const QString& namePrefix /*=""*/)
{
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  for (int folderIndex = 0; folderIndex < folderCount; ++folderIndex)
  {
    vtkIdType folderID = shNode->CreateFolderItem(shNode->GetSceneItemID(), QString("%1Folder%2").arg(namePrefix).arg(folderIndex).toStdString());
    shNode->SetItemExpanded(folderID, folderIndex % 2 == 0);
    for (int modelIndex = 0; modelIndex < modelsPerFolder; ++modelIndex)
    {
      std::string modelName = QString("%1Model%2_%3").arg(namePrefix).arg(folderIndex).arg(modelIndex).toStdString();
      vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode", modelName));
      modelNode->CreateDefaultDisplayNodes();
      modelNode->GetDisplayNode()->SetColor(0.2 * modelIndex, 0.5, 0.1 * folderIndex);
      shNode->CreateItem(folderID, modelNode, "Default");
    }
    vtkIdType subFolderID = shNode->CreateFolderItem(folderID, QString("%1SubFolder%2").arg(namePrefix).arg(folderIndex).toStdString());
    shNode->SetItemExpanded(subFolderID, false);
    for (int modelIndex = 0; modelIndex < 2; ++modelIndex)
    {
      std::string modelName = QString("%1SubModel%2_%3").arg(namePrefix).arg(folderIndex).arg(modelIndex).toStdString();
      vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode", modelName));
      modelNode->CreateDefaultDisplayNodes();
      shNode->CreateItem(subFolderID, modelNode, "Default");
    }
  }
}

// ----------------------------------------------------------------------------
std::vector<vtkIdType> qMRMLSubjectHierarchyTreeViewTester::allItems(vtkMRMLSubjectHierarchyNode* shNode)
{
  std::vector<vtkIdType> allItemIDs;
  shNode->GetItemChildren(shNode->GetSceneItemID(), allItemIDs, true);
  return allItemIDs;
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::collectIndexWidgets(qMRMLSubjectHierarchyTreeView& view,
                                                              const QModelIndex& parent,
                                                              int column,
                                                              QHash<vtkIdType, QPointer<QWidget>>& widgets)
{
  qMRMLSortFilterSubjectHierarchyProxyModel* proxy = view.sortFilterProxyModel();
  for (int row = 0; row < proxy->rowCount(parent); ++row)
  {
    QModelIndex index = proxy->index(row, column, parent);
    vtkIdType itemID = proxy->subjectHierarchyItemFromIndex(index);
    widgets[itemID] = view.indexWidget(index);
    collectIndexWidgets(view, proxy->index(row, 0, parent), column, widgets);
  }
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::verifyButtons(qMRMLSubjectHierarchyTreeView& view,
                                                        vtkMRMLSubjectHierarchyNode* shNode,
                                                        QHash<vtkIdType, QPointer<QWidget>>& visibilityButtons)
{
  // Buttons are created from the event loop
  QTest::qWait(50);
  visibilityButtons.clear();
  collectIndexWidgets(view, view.rootIndex(), view.model()->visibilityColumn(), visibilityButtons);
  QHash<vtkIdType, QPointer<QWidget>> colorButtons;
  collectIndexWidgets(view, view.rootIndex(), view.model()->colorColumn(), colorButtons);
  for (vtkIdType itemID : allItems(shNode))
  {
    QVERIFY2(visibilityButtons.contains(itemID), qPrintable(QString("Item %1 is not displayed").arg(shNode->GetItemName(itemID).c_str())));
    auto* visibilityButton = qobject_cast<QToolButton*>(visibilityButtons[itemID].data());
    QVERIFY2(visibilityButton != nullptr, qPrintable(QString("No visibility button for item %1").arg(shNode->GetItemName(itemID).c_str())));
    QCOMPARE(visibilityButton->property("itemID").value<vtkIdType>(), itemID);
    QModelIndex colorIndex = view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID, view.model()->colorColumn());
    QColor color = colorIndex.data(qMRMLSubjectHierarchyModel::ColorRole).value<QColor>();
    if (color.isValid() && color.alpha() != 0)
    {
      // Color button is shown for items that have a color
      auto* colorButton = qobject_cast<QToolButton*>(colorButtons[itemID].data());
      QVERIFY2(colorButton != nullptr, qPrintable(QString("No color button for item %1").arg(shNode->GetItemName(itemID).c_str())));
      QCOMPARE(colorButton->property("itemID").value<vtkIdType>(), itemID);
    }
  }
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testDisplayedItems()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 4, 3);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QTest::qWait(50);
  QCOMPARE(view.subjectHierarchyNode(), shNode);
  QCOMPARE(view.displayedItemCount(), static_cast<int>(allItems(shNode).size()));
  for (vtkIdType itemID : allItems(shNode))
  {
    QVERIFY(view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID).isValid());
  }

  // Items added in a batch are displayed after the batch processing ends
  scene->StartState(vtkMRMLScene::BatchProcessState);
  this->populateSubjectHierarchy(scene, 2, 2, "Batch");
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QTest::qWait(50);
  QCOMPARE(view.displayedItemCount(), static_cast<int>(allItems(shNode).size()));

  // Clearing the scene empties the view
  scene->Clear(0);
  // The plugin handler needs to observe the new subject hierarchy node (in the application it is done by the plugin logic)
  qSlicerSubjectHierarchyPluginHandler::instance()->observeSubjectHierarchyNode(scene->GetSubjectHierarchyNode());
  QTest::qWait(50);
  QCOMPARE(view.displayedItemCount(), 0);
  this->populateSubjectHierarchy(scene, 1, 1, "AfterClear");
  QTest::qWait(50);
  QCOMPARE(view.displayedItemCount(), static_cast<int>(allItems(scene->GetSubjectHierarchyNode()).size()));
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testExpandedStateAfterRebuild()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 6, 3);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QTest::qWait(50);

  // Setting the scene expands the items (to a certain depth), set the expanded states now.
  // Changes are applied to the view immediately.
  std::vector<vtkIdType> itemIDs = allItems(shNode);
  for (size_t itemIndex = 0; itemIndex < itemIDs.size(); ++itemIndex)
  {
    shNode->SetItemExpanded(itemIDs[itemIndex], itemIndex % 3 != 1);
  }
  for (vtkIdType itemID : itemIDs)
  {
    QCOMPARE(view.isExpanded(view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID)), shNode->GetItemExpanded(itemID));
  }

  // Rebuild the model (items added in a batch)
  scene->StartState(vtkMRMLScene::BatchProcessState);
  this->populateSubjectHierarchy(scene, 2, 2, "Batch");
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QTest::qWait(50);

  // Expanded state of the tree matches the subject hierarchy
  for (vtkIdType itemID : allItems(shNode))
  {
    QModelIndex index = view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
    QVERIFY(index.isValid());
    QCOMPARE(view.isExpanded(index), shNode->GetItemExpanded(itemID));
  }
  // Children of expanded folders are visible, children of collapsed folders are not
  vtkIdType expandedFolder = shNode->GetItemByName("Folder0");
  vtkIdType collapsedFolder = shNode->GetItemByName("Folder1");
  shNode->SetItemExpanded(expandedFolder, true);
  shNode->SetItemExpanded(collapsedFolder, false);
  QTest::qWait(50);
  QVERIFY(view.visualRect(view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(shNode->GetItemByName("Model0_0"))).isValid());
  QVERIFY(!view.visualRect(view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(shNode->GetItemByName("Model1_0"))).isValid());
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testButtonsCreatedAndReused()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 4, 3);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QHash<vtkIdType, QPointer<QWidget>> buttons;
  this->verifyButtons(view, shNode, buttons);

  // Filter invalidation (layout change) does not recreate the buttons
  view.sortFilterProxyModel()->invalidate();
  QHash<vtkIdType, QPointer<QWidget>> buttonsAfterLayoutChange;
  this->verifyButtons(view, shNode, buttonsAfterLayoutChange);
  for (vtkIdType itemID : allItems(shNode))
  {
    QVERIFY(buttons[itemID] != nullptr);
    QCOMPARE(buttonsAfterLayoutChange[itemID].data(), buttons[itemID].data());
  }

  // Adding an item does not recreate the buttons of the other items
  vtkIdType newFolderID = shNode->CreateFolderItem(shNode->GetSceneItemID(), "NewFolder");
  QHash<vtkIdType, QPointer<QWidget>> buttonsAfterAdd;
  this->verifyButtons(view, shNode, buttonsAfterAdd);
  QVERIFY(buttonsAfterAdd[newFolderID] != nullptr);
  for (vtkIdType itemID : allItems(shNode))
  {
    if (itemID != newFolderID)
    {
      QCOMPARE(buttonsAfterAdd[itemID].data(), buttons[itemID].data());
    }
  }

  // All items have buttons after the model is rebuilt (end of batch processing)
  scene->StartState(vtkMRMLScene::BatchProcessState);
  this->populateSubjectHierarchy(scene, 2, 2, "Batch");
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QHash<vtkIdType, QPointer<QWidget>> buttonsAfterRebuild;
  this->verifyButtons(view, shNode, buttonsAfterRebuild);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testVisibilityButton()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QHash<vtkIdType, QPointer<QWidget>> buttons;
  this->verifyButtons(view, shNode, buttons);

  vtkIdType modelItemID = shNode->GetItemByName("Model0_1");
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(modelItemID));
  QVERIFY(modelNode != nullptr);
  QCOMPARE(modelNode->GetDisplayVisibility(), 1);
  auto* button = qobject_cast<QToolButton*>(buttons[modelItemID].data());
  QVERIFY(button != nullptr);
  button->click();
  QCOMPARE(modelNode->GetDisplayVisibility(), 0);
  QTest::qWait(50);
  // The same button is kept when the visibility changes
  QCOMPARE(view.indexWidget(view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(modelItemID, view.model()->visibilityColumn())), button);
  button->click();
  QCOMPARE(modelNode->GetDisplayVisibility(), 1);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testExpandCollapseItem()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 3, 3);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QTest::qWait(50);

  vtkIdType folderID = shNode->GetItemByName("Folder0");
  vtkIdType childID = shNode->GetItemByName("Model0_0");
  QModelIndex folderIndex = view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(folderID);
  QModelIndex childIndex = view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(childID);
  QVERIFY(view.isExpanded(folderIndex));

  // The expanded state is updated immediately, the layout is updated from the event loop
  view.collapseItem(folderID);
  QVERIFY(!view.isExpanded(folderIndex));
  QVERIFY(!shNode->GetItemExpanded(folderID));
  QTest::qWait(50);
  QVERIFY(!view.visualRect(childIndex).isValid());

  view.expandItem(folderID);
  QVERIFY(view.isExpanded(folderIndex));
  QVERIFY(shNode->GetItemExpanded(folderID));
  QTest::qWait(50);
  QVERIFY(view.visualRect(childIndex).isValid());

  // Many expand/collapse requests are processed quickly (no layout for each)
  QElapsedTimer timer;
  timer.start();
  for (int i = 0; i < 20; ++i)
  {
    for (vtkIdType itemID : allItems(shNode))
    {
      view.collapseItem(itemID);
    }
    for (vtkIdType itemID : allItems(shNode))
    {
      view.expandItem(itemID);
    }
  }
  QVERIFY2(timer.elapsed() < 5000, qPrintable(QString("Expanding/collapsing took %1 ms").arg(timer.elapsed())));
  QTest::qWait(50);
  for (vtkIdType itemID : allItems(shNode))
  {
    QVERIFY(view.isExpanded(view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID)));
  }
}

// ----------------------------------------------------------------------------
namespace
{
void resolveSubjectHierarchyCallback(vtkObject* caller, unsigned long vtkNotUsed(eid), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
  vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(vtkMRMLScene::SafeDownCast(caller));
}
} // namespace

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testSelectionAcrossBatchAndImport()
{
  vtkNew<vtkMRMLScene> sourceScene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(sourceScene);
  this->populateSubjectHierarchy(sourceScene, 3, 3, "Imported");
  sourceScene->SetSaveToXMLString(1);
  sourceScene->Commit();
  std::string xml = sourceScene->GetSceneXMLString();

  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  vtkNew<vtkCallbackCommand> resolveCallback;
  resolveCallback->SetCallback(resolveSubjectHierarchyCallback);
  scene->AddObserver(vtkMRMLScene::EndImportEvent, resolveCallback, 10.0);

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QTest::qWait(50);

  // Selection is kept across batch processing
  vtkIdType selectedItemID = shNode->GetItemByName("Model1_1");
  view.setCurrentItem(selectedItemID);
  QCOMPARE(view.currentItem(), selectedItemID);
  scene->StartState(vtkMRMLScene::BatchProcessState);
  this->populateSubjectHierarchy(scene, 2, 2, "Batch");
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QTest::qWait(50);
  QCOMPARE(view.currentItem(), selectedItemID);

  // Items can be selected after import
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(xml);
  QVERIFY(scene->Import());
  QTest::qWait(50);
  QCOMPARE(view.displayedItemCount(), static_cast<int>(allItems(shNode).size()));
  vtkIdType importedItemID = shNode->GetItemByName("ImportedModel2_1");
  QVERIFY(importedItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);
  view.setCurrentItem(importedItemID);
  QCOMPARE(view.currentItem(), importedItemID);
  QHash<vtkIdType, QPointer<QWidget>> buttons;
  this->verifyButtons(view, shNode, buttons);
}

// ----------------------------------------------------------------------------
namespace
{
struct SelectAddedNodeClientData
{
  qMRMLSubjectHierarchyTreeView* View{ nullptr };
  vtkIdType FolderID{ vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID };
  vtkIdType SelectedItemID{ vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID };
};

// Mimics a module that observes node added events (regardless of batch processing) and selects
// the newly added node in a subject hierarchy tree view right away
void selectAddedNodeCallback(vtkObject* caller, unsigned long vtkNotUsed(eid), void* clientData, void* callData)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(caller);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(reinterpret_cast<vtkObject*>(callData));
  SelectAddedNodeClientData* data = reinterpret_cast<SelectAddedNodeClientData*>(clientData);
  if (!scene || !modelNode || !data || QString(modelNode->GetName()) != "SelectedModel")
  {
    return;
  }
  // In the application the subject hierarchy item is created by the subject hierarchy plugin logic
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  data->SelectedItemID = shNode->CreateItem(data->FolderID, modelNode, "Default");
  data->View->setCurrentNode(modelNode);
}
} // namespace

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testSelectItemAddedInBatch()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QTest::qWait(50);
  view.setCurrentItem(shNode->GetItemByName("Model1_1"));

  SelectAddedNodeClientData clientData;
  clientData.View = &view;
  vtkNew<vtkCallbackCommand> selectCallback;
  selectCallback->SetCallback(selectAddedNodeCallback);
  selectCallback->SetClientData(&clientData);
  scene->AddObserver(vtkMRMLScene::NodeAddedEvent, selectCallback);

  scene->StartState(vtkMRMLScene::BatchProcessState);
  this->populateSubjectHierarchy(scene, 2, 2, "Batch");
  // A new folder and a node in it, selected by the node added observer
  clientData.FolderID = shNode->CreateFolderItem(shNode->GetSceneItemID(), "NewFolder");
  scene->AddNewNodeByClass("vtkMRMLModelNode", "SelectedModel");
  QVERIFY(clientData.SelectedItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);
  // The item is selected in the view right away (during batch processing).
  // Note: currentItem() is not updated during batch processing (selection changes are reported when it ends).
  QModelIndex currentIndex = view.selectionModel()->currentIndex();
  QVERIFY(currentIndex.isValid());
  QCOMPARE(view.sortFilterProxyModel()->subjectHierarchyItemFromIndex(currentIndex), clientData.SelectedItemID);
  QVERIFY(view.selectionModel()->isRowSelected(currentIndex.row(), currentIndex.parent()));
  this->populateSubjectHierarchy(scene, 1, 1, "BatchAfterSelection");
  scene->EndState(vtkMRMLScene::BatchProcessState);
  QTest::qWait(50);

  // The selection is kept after the model is rebuilt at the end of the batch processing
  QCOMPARE(view.currentItem(), clientData.SelectedItemID);
  QCOMPARE(view.currentItems().size(), 1);
  QCOMPARE(view.displayedItemCount(), static_cast<int>(allItems(shNode).size()));
  QHash<vtkIdType, QPointer<QWidget>> buttons;
  this->verifyButtons(view, shNode, buttons);

  scene->RemoveObserver(selectCallback);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testEditsInLargeScene()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 20, 20);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QTest::qWait(50);
  QCOMPARE(view.displayedItemCount(), static_cast<int>(allItems(shNode).size()));

  // Rows have uniform height, so that the view does not compute the size of every row when the layout is updated
  QVERIFY(view.uniformRowHeights());

  // Items added, moved, selected, and removed one by one in a large scene appear in the view right away
  QElapsedTimer timer;
  timer.start();
  vtkIdType lastFolderID = shNode->GetItemByName("Folder19");
  vtkIdType firstFolderID = shNode->GetItemByName("Folder0");
  QList<vtkIdType> addedItemIDs;
  for (int i = 0; i < 50; ++i)
  {
    vtkMRMLNode* modelNode = scene->AddNewNodeByClass("vtkMRMLModelNode", QString("AddedModel%1").arg(i).toStdString());
    vtkIdType itemID = shNode->CreateItem(lastFolderID, modelNode, "Default");
    addedItemIDs << itemID;
    if (i % 2)
    {
      shNode->SetItemParent(itemID, firstFolderID);
    }
    view.setCurrentItem(itemID);
    QCOMPARE(view.currentItem(), itemID);
    QModelIndex index = view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
    QVERIFY(index.isValid());
    QCOMPARE(view.sortFilterProxyModel()->subjectHierarchyItemFromIndex(index.parent()), (i % 2) ? firstFolderID : lastFolderID);
    QCOMPARE(view.sortFilterProxyModel()->data(index).toString(), QString("AddedModel%1").arg(i));
    if (i % 5 == 4)
    {
      // Remove an earlier added item
      vtkIdType removedItemID = addedItemIDs.takeFirst();
      QVERIFY(shNode->RemoveItem(removedItemID, false, true));
      QVERIFY(!view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(removedItemID).isValid());
    }
  }
  qDebug() << "Interactive edits in a large scene took" << timer.elapsed() << "ms";
  QVERIFY2(timer.elapsed() < 10000, qPrintable(QString("Interactive edits took %1 ms").arg(timer.elapsed())));
  QTest::qWait(50);
  QCOMPARE(view.displayedItemCount(), static_cast<int>(allItems(shNode).size()));
  QHash<vtkIdType, QPointer<QWidget>> buttons;
  this->verifyButtons(view, shNode, buttons);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testItemsAddedWhileModifyingNode()
{
  // Items added while the subject hierarchy node is being modified (item events are compressed and
  // invoked without item ID when the modification ends) are displayed
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 2, 2);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QTest::qWait(50);

  int wasModifying = shNode->StartModify();
  this->populateSubjectHierarchy(scene, 2, 2, "AddedWhileModifying");
  shNode->EndModify(wasModifying);
  QTest::qWait(50);
  QCOMPARE(view.displayedItemCount(), static_cast<int>(allItems(shNode).size()));
  vtkIdType addedItemID = shNode->GetItemByName("AddedWhileModifyingModel1_1");
  QVERIFY(addedItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);
  view.setCurrentItem(addedItemID);
  QCOMPARE(view.currentItem(), addedItemID);
  QHash<vtkIdType, QPointer<QWidget>> buttons;
  this->verifyButtons(view, shNode, buttons);
}

// ----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewTester::testExpandedStateAfterMove()
{
  vtkNew<vtkMRMLScene> scene;
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);
  this->populateSubjectHierarchy(scene, 3, 3);
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  // Nested folders with mixed expanded states in the branch that is moved
  vtkIdType movedFolderID = shNode->GetItemByName("Folder1");
  vtkIdType subFolderID = shNode->GetItemByName("SubFolder1");
  vtkIdType nestedFolderID = shNode->CreateFolderItem(subFolderID, "NestedFolder");
  vtkMRMLNode* modelNode = scene->AddNewNodeByClass("vtkMRMLModelNode", "NestedModel");
  shNode->CreateItem(nestedFolderID, modelNode, "Default");

  qMRMLSubjectHierarchyTreeView view;
  view.setMRMLScene(scene);
  view.show();
  QTest::qWait(50);
  shNode->SetItemExpanded(movedFolderID, true);
  shNode->SetItemExpanded(subFolderID, false);
  shNode->SetItemExpanded(nestedFolderID, true);

  auto verifyExpandedStates = [&]()
  {
    for (vtkIdType itemID : allItems(shNode))
    {
      if (shNode->GetNumberOfItemChildren(itemID) == 0)
      {
        continue;
      }
      QModelIndex index = view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
      QVERIFY(index.isValid());
      QVERIFY2(view.isExpanded(index) == shNode->GetItemExpanded(itemID), qPrintable(QString("Wrong expanded state for item %1").arg(shNode->GetItemName(itemID).c_str())));
    }
  };
  verifyExpandedStates();

  // The expanded state of all the items in the moved branch is kept
  shNode->SetItemParent(movedFolderID, shNode->GetItemByName("Folder0"));
  QTest::qWait(50);
  verifyExpandedStates();
  QVERIFY(view.visualRect(view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(shNode->GetItemByName("Model1_0"))).isValid());
  QVERIFY(!view.visualRect(view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(nestedFolderID)).isValid());

  // Modifying items does not request expanding or collapsing items (each request makes the views update their layout)
  int expandRequestCount = 0;
  int collapseRequestCount = 0;
  QObject::connect(view.model(), &qMRMLSubjectHierarchyModel::requestExpandItem, [&](vtkIdType) { ++expandRequestCount; });
  QObject::connect(view.model(), &qMRMLSubjectHierarchyModel::requestCollapseItem, [&](vtkIdType) { ++collapseRequestCount; });
  shNode->SetItemName(movedFolderID, "RenamedFolder");
  shNode->SetItemName(shNode->GetItemByName("Model0_0"), "RenamedModel");
  vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(shNode->GetItemByName("Model2_1")))->SetDisplayVisibility(0);
  QCOMPARE(expandRequestCount, 0);
  QCOMPARE(collapseRequestCount, 0);

  // Changing the expanded state in the subject hierarchy updates the view
  shNode->SetItemExpanded(subFolderID, true);
  QCOMPARE(expandRequestCount, 1);
  QTest::qWait(50);
  verifyExpandedStates();
  // Collapsing in the view updates the subject hierarchy
  view.collapse(view.sortFilterProxyModel()->indexFromSubjectHierarchyItem(movedFolderID));
  QVERIFY(!shNode->GetItemExpanded(movedFolderID));
  QTest::qWait(50);
  verifyExpandedStates();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLSubjectHierarchyTreeViewTest)
#include "qMRMLSubjectHierarchyTreeViewTest.moc"
