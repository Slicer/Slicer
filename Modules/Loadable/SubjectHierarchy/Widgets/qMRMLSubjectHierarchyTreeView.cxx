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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QHeaderView>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QToolTip>
#include <QBuffer>
#include <QApplication>

// SubjectHierarchy includes
#include "qMRMLSubjectHierarchyTreeView.h"

#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"
#include "qMRMLTransformItemDelegate.h"

#include "vtkSlicerSubjectHierarchyModuleLogic.h"

#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Terminologies includes
#include "qSlicerTerminologyItemDelegate.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// qMRML includes
#include "qMRMLItemDelegate.h"

// VTK includes
#include <vtkIdList.h>

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qMRMLSubjectHierarchyTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSubjectHierarchyTreeView);

protected:
  qMRMLSubjectHierarchyTreeView* const q_ptr;

public:
  qMRMLSubjectHierarchyTreeViewPrivate(qMRMLSubjectHierarchyTreeView& object);

  virtual void init();

  /// Setup all actions for tree view
  void setupActions();

  /// Get list of enabled plugins \sa PluginWhitelist \sa PluginBlacklist
  QList<qSlicerSubjectHierarchyAbstractPlugin*> enabledPlugins();

public:
  qMRMLSubjectHierarchyModel* Model;
  qMRMLSortFilterSubjectHierarchyProxyModel* SortFilterModel;

  bool ShowRootItem;
  vtkIdType RootItemID;

  bool ContextMenuEnabled;
  bool EditActionVisible;
  bool SelectRoleSubMenuVisible;

  QMenu* NodeMenu;
  QAction* RenameAction;
  QAction* DeleteAction;
  QAction* EditAction;
  QAction* ToggleVisibilityAction;
  QList<QAction*> SelectPluginActions;
  QMenu* SelectPluginSubMenu;
  QActionGroup* SelectPluginActionGroup;
  QAction* ExpandToDepthAction;
  QMenu* SceneMenu;
  QMenu* VisibilityMenu;
  QStringList PluginWhitelist;
  QStringList PluginBlacklist;

  qMRMLTransformItemDelegate* TransformItemDelegate;

  /// Subject hierarchy node
  vtkWeakPointer<vtkMRMLSubjectHierarchyNode> SubjectHierarchyNode;

  /// Flag determining whether to highlight items referenced by DICOM. Storing DICOM references:
  ///   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  ///   -> SH node instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  bool HighlightReferencedItems;

  /// Cached list of selected items to return the current selection
  QList<vtkIdType> SelectedItems;

  /// List of selected items to restore at the end of batch processing (the whole tree is rebuilt and selection is lost)
  QList<vtkIdType> SelectedItemsToRestore;

  /// Cached list of highlighted items to speed up clearing highlight after new selection
  QList<vtkIdType> HighlightedItems;
};

//------------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeViewPrivate::qMRMLSubjectHierarchyTreeViewPrivate(qMRMLSubjectHierarchyTreeView& object)
  : q_ptr(&object)
  , Model(nullptr)
  , SortFilterModel(nullptr)
  , ShowRootItem(false)
  , RootItemID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  , ContextMenuEnabled(true)
  , EditActionVisible(true)
  , SelectRoleSubMenuVisible(false)
  , NodeMenu(nullptr)
  , RenameAction(nullptr)
  , DeleteAction(nullptr)
  , EditAction(nullptr)
  , ToggleVisibilityAction(nullptr)
  , SelectPluginSubMenu(nullptr)
  , SelectPluginActionGroup(nullptr)
  , ExpandToDepthAction(nullptr)
  , SceneMenu(nullptr)
  , VisibilityMenu(nullptr)
  , TransformItemDelegate(nullptr)
  , SubjectHierarchyNode(nullptr)
  , HighlightReferencedItems(true)
{
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::init()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);

  // Set up scene model and sort and proxy model
  this->Model = new qMRMLSubjectHierarchyModel(q);
  QObject::connect( this->Model, SIGNAL(requestExpandItem(vtkIdType)), q, SLOT(expandItem(vtkIdType)) );
  QObject::connect( this->Model, SIGNAL(requestCollapseItem(vtkIdType)), q, SLOT(collapseItem(vtkIdType)) );
  QObject::connect( this->Model, SIGNAL(requestSelectItems(QList<vtkIdType>)), q, SLOT(setCurrentItems(QList<vtkIdType>)) );
  QObject::connect( this->Model, SIGNAL(subjectHierarchyUpdated()), q, SLOT(updateRootItem()) );

  this->SortFilterModel = new qMRMLSortFilterSubjectHierarchyProxyModel(q);
  q->QTreeView::setModel(this->SortFilterModel);
  QObject::connect( q->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                    q, SLOT(onSelectionChanged(QItemSelection,QItemSelection)) );
  this->SortFilterModel->setParent(q);
  this->SortFilterModel->setSourceModel(this->Model);

  // Set up headers
  q->resetColumnSizesToDefault();
  if (this->Model->descriptionColumn()>=0)
    {
    q->setColumnHidden(this->Model->descriptionColumn(), true);
    }

  // Set generic MRML item delegate
  q->setItemDelegate(new qMRMLItemDelegate(q));

  // Create default menu actions
  this->NodeMenu = new QMenu(q);
  this->NodeMenu->setObjectName("nodeMenuTreeView");

  this->RenameAction = new QAction("Rename", this->NodeMenu);
  this->NodeMenu->addAction(this->RenameAction);
  QObject::connect(this->RenameAction, SIGNAL(triggered()), q, SLOT(renameCurrentItem()));

  this->DeleteAction = new QAction("Delete", this->NodeMenu);
  this->NodeMenu->addAction(this->DeleteAction);
  QObject::connect(this->DeleteAction, SIGNAL(triggered()), q, SLOT(deleteSelectedItems()));

  this->EditAction = new QAction("Edit properties...", this->NodeMenu);
  this->NodeMenu->addAction(this->EditAction);
  QObject::connect(this->EditAction, SIGNAL(triggered()), q, SLOT(editCurrentItem()));

  this->ToggleVisibilityAction = new QAction("Toggle visibility", this->NodeMenu);
  this->NodeMenu->addAction(this->ToggleVisibilityAction);
  QObject::connect(this->ToggleVisibilityAction, SIGNAL(triggered()), q, SLOT(toggleVisibilityOfSelectedItems()));

  this->SceneMenu = new QMenu(q);
  this->SceneMenu->setObjectName("sceneMenuTreeView");

  this->VisibilityMenu = new QMenu(q);
  this->VisibilityMenu->setObjectName("visibilityMenuTreeView");

  // Set item delegate for color column
  q->setItemDelegateForColumn(this->Model->colorColumn(), new qSlicerTerminologyItemDelegate(q));

  // Set item delegate for transform column (that creates widgets for certain types of data)
  this->TransformItemDelegate = new qMRMLTransformItemDelegate(q);
  this->TransformItemDelegate->setMRMLScene(q->mrmlScene());
  q->setItemDelegateForColumn(this->Model->transformColumn(), this->TransformItemDelegate);
  QObject::connect( this->TransformItemDelegate, SIGNAL(removeTransformsFromBranchOfCurrentItem()),
    this->Model, SLOT(onRemoveTransformsFromBranchOfCurrentItem()) );
  QObject::connect( this->TransformItemDelegate, SIGNAL(hardenTransformOnBranchOfCurrentItem()),
    this->Model, SLOT(onHardenTransformOnBranchOfCurrentItem()) );

  q->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(q, SIGNAL(customContextMenuRequested(const QPoint&)), q, SLOT(onCustomContextMenu(const QPoint&)));

  // Make connections
  QObject::connect( this->Model, SIGNAL(invalidateFilter()), this->SortFilterModel, SLOT(invalidate()) );
  QObject::connect( q, SIGNAL(expanded(const QModelIndex&)), q, SLOT(onItemExpanded(const QModelIndex&)) );
  QObject::connect( q, SIGNAL(collapsed(const QModelIndex&)), q, SLOT(onItemCollapsed(const QModelIndex&)) );

  // Set up scene and node actions for the tree view
  this->setupActions();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::resetColumnSizesToDefault()
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  // Set up headers
  this->header()->setStretchLastSection(false);
  if (this->header()->count() <= 0)
    {
    return;
    }
  if (d->Model->nameColumn() >= 0)
    {
    this->header()->setSectionResizeMode(d->Model->nameColumn(), QHeaderView::Stretch);
    }
  if (d->Model->descriptionColumn() >= 0)
    {
    this->header()->setSectionResizeMode(d->Model->descriptionColumn(), QHeaderView::Interactive);
    }
  if (d->Model->visibilityColumn() >= 0)
    {
    this->header()->setSectionResizeMode(d->Model->visibilityColumn(), QHeaderView::ResizeToContents);
    }
  if (d->Model->colorColumn() >= 0)
    {
    this->header()->setSectionResizeMode(d->Model->colorColumn(), QHeaderView::ResizeToContents);
    }
  if (d->Model->transformColumn() >= 0)
    {
    this->header()->setSectionResizeMode(d->Model->transformColumn(), QHeaderView::ResizeToContents);
    }
  if (d->Model->idColumn() >= 0)
    {
    this->header()->setSectionResizeMode(d->Model->idColumn(), QHeaderView::ResizeToContents);
    }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::setupActions()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);

  // Set up expand to level action and its menu
  this->ExpandToDepthAction = new QAction("Expand tree to level...", this->NodeMenu);
  this->SceneMenu->addAction(this->ExpandToDepthAction);

  QMenu* expandToDepthSubMenu = new QMenu();
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_1 = new QAction("1",q);
  QObject::connect(expandToDepth_1, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_1);
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_2 = new QAction("2",q);
  QObject::connect(expandToDepth_2, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_2);
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_3 = new QAction("3",q);
  QObject::connect(expandToDepth_3, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_3);
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_4 = new QAction("4",q);
  QObject::connect(expandToDepth_4, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_4);

  // Perform tasks needed for all plugins
  int index = 0; // Index used to insert actions before default tree actions
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    // Add node context menu actions
    foreach (QAction* action, plugin->itemContextMenuActions())
      {
      this->NodeMenu->insertAction(this->NodeMenu->actions()[index++], action);
      }

    // Add scene context menu actions
    foreach (QAction* action, plugin->sceneContextMenuActions())
      {
      this->SceneMenu->addAction(action);
      }

    // Add visibility context menu actions
    foreach (QAction* action, plugin->visibilityContextMenuActions())
      {
      this->VisibilityMenu->addAction(action);
      }

    // Connect plugin events to be handled by the tree view
    QObject::connect( plugin, SIGNAL(requestExpandItem(vtkIdType)), q, SLOT(expandItem(vtkIdType)) );
    QObject::connect( plugin, SIGNAL(requestInvalidateFilter()), q->model(), SIGNAL(invalidateFilter()) );
    }

  // Create a plugin selection action for each plugin in a sub-menu
  this->SelectPluginSubMenu = this->NodeMenu->addMenu("Select role");
  this->SelectPluginActionGroup = new QActionGroup(q);
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    QAction* selectPluginAction = new QAction(plugin->name(),q);
    selectPluginAction->setCheckable(true);
    selectPluginAction->setActionGroup(this->SelectPluginActionGroup);
    selectPluginAction->setData(QVariant(plugin->name()));
    this->SelectPluginSubMenu->addAction(selectPluginAction);
    QObject::connect(selectPluginAction, SIGNAL(triggered()), q, SLOT(selectPluginForCurrentItem()));
    this->SelectPluginActions << selectPluginAction;
    }

  // Update actions in owner plugin sub-menu when opened
  QObject::connect( this->SelectPluginSubMenu, SIGNAL(aboutToShow()), q, SLOT(updateSelectPluginActions()) );
}

//------------------------------------------------------------------------------
QList<qSlicerSubjectHierarchyAbstractPlugin*> qMRMLSubjectHierarchyTreeViewPrivate::enabledPlugins()
{
  QList<qSlicerSubjectHierarchyAbstractPlugin*> enabledPluginList;

  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    QString pluginName = plugin->name();
    bool whitelisted = (this->PluginWhitelist.isEmpty() || this->PluginWhitelist.contains(pluginName));
    bool blacklisted = (!this->PluginBlacklist.isEmpty() && this->PluginBlacklist.contains(pluginName));
    if ((whitelisted && !blacklisted) || !pluginName.compare("Default"))
      {
      enabledPluginList << plugin;
      }
    }

  return enabledPluginList;
}


//------------------------------------------------------------------------------
// qMRMLSubjectHierarchyTreeView
//------------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeView::qMRMLSubjectHierarchyTreeView(QWidget *parent)
  : QTreeView(parent)
  , d_ptr(new qMRMLSubjectHierarchyTreeViewPrivate(*this))
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeView::~qMRMLSubjectHierarchyTreeView() = default;

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* shNode)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  d->SubjectHierarchyNode = shNode;

  qvtkReconnect( shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent,
                 this, SLOT( onSubjectHierarchyItemModified(vtkObject*,void*) ) );

  if (!shNode)
    {
    d->Model->setMRMLScene(nullptr);
    d->TransformItemDelegate->setMRMLScene(nullptr);
    return;
    }

  vtkMRMLScene* scene = shNode->GetScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Given subject hierarchy node is not in a MRML scene";
    }

  d->Model->setMRMLScene(scene);
  d->TransformItemDelegate->setMRMLScene(scene);
  this->setRootItem(shNode->GetSceneItemID());
  this->expandToDepth(4);
}

//------------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qMRMLSubjectHierarchyTreeView::subjectHierarchyNode()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->SubjectHierarchyNode;
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLSubjectHierarchyTreeView::mrmlScene()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->Model ? d->Model->mrmlScene() : nullptr;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setMRMLScene(vtkMRMLScene* scene)
{
  if (this->mrmlScene() == scene)
    {
    return;
    }

  this->setSubjectHierarchyNode(scene ? vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene) : nullptr);

  // Connect scene close ended event so that subject hierarchy can be cleared
  qvtkReconnect( scene, vtkMRMLScene::EndCloseEvent, this, SLOT( onMRMLSceneCloseEnded(vtkObject*) ) );
  qvtkReconnect( scene, vtkMRMLScene::StartBatchProcessEvent, this, SLOT( onMRMLSceneStartBatchProcess(vtkObject*) ) );
  qvtkReconnect( scene, vtkMRMLScene::EndBatchProcessEvent, this, SLOT( onMRMLSceneEndBatchProcess(vtkObject*) ) );
}

//------------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyTreeView::currentItem()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->SelectedItems.count() ? d->SelectedItems[0] : vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSubjectHierarchyTreeView::currentNode()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  vtkIdType itemID = currentItem();
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID || !d->SubjectHierarchyNode)
    {
    return nullptr;
    }
 return d->SubjectHierarchyNode->GetItemDataNode(itemID);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setCurrentItem(vtkIdType itemID)
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!d->SortFilterModel)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid data model";
    return;
    }

  QModelIndex itemIndex = d->SortFilterModel->indexFromSubjectHierarchyItem(itemID);
  this->selectionModel()->select(itemIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

//------------------------------------------------------------------------------
QList<vtkIdType> qMRMLSubjectHierarchyTreeView::currentItems()
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->SelectedItems;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::currentItems(vtkIdList* selectedItems)
{
  Q_D(const qMRMLSubjectHierarchyTreeView);

  if (!selectedItems)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid item list";
    return;
    }

  foreach (vtkIdType item, d->SelectedItems)
    {
    selectedItems->InsertNextId(item);
    }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setCurrentItems(QList<vtkIdType> items)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SortFilterModel)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid data model";
    return;
    }

  this->clearSelection();

  foreach (long itemID, items)
    {
    QModelIndex itemIndex = d->SortFilterModel->indexFromSubjectHierarchyItem(vtkIdType(itemID));
    if (itemIndex.isValid())
      {
      this->selectionModel()->select(itemIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setCurrentItems(vtkIdList* items)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SortFilterModel)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid data model";
    return;
    }

  if (!items)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid item list";
    return;
    }

  this->clearSelection();

  for (int index=0; index<items->GetNumberOfIds(); ++index)
    {
    QModelIndex itemIndex = d->SortFilterModel->indexFromSubjectHierarchyItem(items->GetId(index));
    if (itemIndex.isValid())
      {
      this->selectionModel()->select(itemIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setCurrentNode(vtkMRMLNode* node)
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!node || !d->SubjectHierarchyNode)
    {
    return;
    }

  vtkIdType itemID = d->SubjectHierarchyNode->GetItemByDataNode(node);
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find subject hierarchy item by data node " << node->GetName();
    return;
    }

  this->setCurrentItem(itemID);
}

//--------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModel* qMRMLSubjectHierarchyTreeView::sortFilterProxyModel()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!d->SortFilterModel)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid sort filter proxy model";
    }
  return d->SortFilterModel;
}

//--------------------------------------------------------------------------
qMRMLSubjectHierarchyModel* qMRMLSubjectHierarchyTreeView::model()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!d->Model)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid data model";
    }
  return d->Model;
}

//--------------------------------------------------------------------------
int qMRMLSubjectHierarchyTreeView::displayedItemCount()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  int count = this->sortFilterProxyModel()->acceptedItemCount(this->rootItem());
  if (d->ShowRootItem)
    {
    count++;
    }
  return count;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setShowRootItem(bool show)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (d->ShowRootItem == show)
    {
    return;
    }
  vtkIdType oldRootItemID = this->rootItem();
  d->ShowRootItem = show;
  this->setRootItem(oldRootItemID);
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::showRootItem()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->ShowRootItem;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setRootItem(vtkIdType rootItemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    return;
    }

  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->model());

  // Reset item in unaffiliated filter (that hides all siblings and their children)
  this->sortFilterProxyModel()->setHideItemsUnaffiliatedWithItemID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);

  QModelIndex treeRootIndex;
  if (!rootItemID)
    {
    treeRootIndex = sceneModel->invisibleRootItem()->index();
    }
  else if (rootItemID == d->SubjectHierarchyNode->GetSceneItemID())
    {
    if (d->ShowRootItem)
      {
      // Scene is a special item, so it needs to be shown, then the invisible root item needs to be root
      treeRootIndex = sceneModel->invisibleRootItem()->index();
      }
    else
      {
      treeRootIndex = this->sortFilterProxyModel()->subjectHierarchySceneIndex();
      }
    }
  else
    {
    treeRootIndex = this->sortFilterProxyModel()->indexFromSubjectHierarchyItem(rootItemID);
    if (d->ShowRootItem)
      {
      // Hide the siblings of the root item and their children
      this->sortFilterProxyModel()->setHideItemsUnaffiliatedWithItemID(rootItemID);
      // The parent of the root node becomes the root for QTreeView.
      treeRootIndex = treeRootIndex.parent();
      rootItemID = this->sortFilterProxyModel()->subjectHierarchyItemFromIndex(treeRootIndex);
      }
    }

  //TODO: Connect SH node's item modified event if necessary
  //qvtkReconnect(this->rootItem(), rootItemID, vtkCommand::ModifiedEvent,
  //              this, SLOT(updateRootItem(vtkObject*)));

  d->RootItemID = rootItemID;
  this->setRootIndex(treeRootIndex);
}

//--------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyTreeView::rootItem()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  vtkIdType treeRootItemID = this->sortFilterProxyModel()->subjectHierarchyItemFromIndex(this->rootIndex());
  if (d->ShowRootItem)
    {
    if (d->RootItemID == d->SubjectHierarchyNode->GetSceneItemID())
      {
      // Scene is a special item, so it needs to be shown, then the invisible root item needs to be root.
      // So in that case no checks are performed
      return d->RootItemID;
      }
    else if (this->sortFilterProxyModel()->hideItemsUnaffiliatedWithItemID())
      {
      treeRootItemID = this->sortFilterProxyModel()->hideItemsUnaffiliatedWithItemID();
      }
    }
  // Check if stored root item ID matches the actual root item in the tree view.
  // If the tree is empty (e.g. due to filters), then treeRootItemID is invalid, and then it's not an error
  if (treeRootItemID && d->RootItemID != treeRootItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Root item mismatch";
    }
  return d->RootItemID;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::updateRootItem()
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  // The scene might have been updated, need to update root item as well to restore view
  this->setRootItem(d->RootItemID);
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::highlightReferencedItems()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->HighlightReferencedItems;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setHighlightReferencedItems(bool highlightOn)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->HighlightReferencedItems = highlightOn;
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::contextMenuEnabled()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->ContextMenuEnabled;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setContextMenuEnabled(bool enabled)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->ContextMenuEnabled = enabled;
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::editMenuActionVisible()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->EditActionVisible;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setEditMenuActionVisible(bool visible)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->EditActionVisible = visible;
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::selectRoleSubMenuVisible()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->SelectRoleSubMenuVisible;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setSelectRoleSubMenuVisible(bool visible)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->SelectRoleSubMenuVisible = visible;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setAttributeFilter(const QString& attributeName, const QVariant& attributeValue/*=QVariant()*/)
{
  this->sortFilterProxyModel()->setAttributeNameFilter(attributeName);
  this->sortFilterProxyModel()->setAttributeValueFilter(attributeValue.toString());

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->setRootItem(this->rootItem());
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setAttributeNameFilter(const QString& attributeName)
{
  this->sortFilterProxyModel()->setAttributeNameFilter(attributeName);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->setRootItem(this->rootItem());
}

//--------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeView::attributeNameFilter()const
{
  return this->sortFilterProxyModel()->attributeNameFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setAttributeValueFilter(const QString& attributeValue)
{
  this->sortFilterProxyModel()->setAttributeValueFilter(attributeValue);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->setRootItem(this->rootItem());
}

//--------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeView::attributeValueFilter()const
{
  return this->sortFilterProxyModel()->attributeValueFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::removeAttributeFilter()
{
  this->sortFilterProxyModel()->setAttributeNameFilter(QString());
  this->sortFilterProxyModel()->setAttributeValueFilter(QString());

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->setRootItem(this->rootItem());
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setLevelFilter(QStringList &levelFilter)
{
  this->sortFilterProxyModel()->setLevelFilter(levelFilter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->setRootItem(this->rootItem());
}

//--------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::levelFilter()const
{
  return this->sortFilterProxyModel()->levelFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setNameFilter(QString &nameFilter)
{
  this->sortFilterProxyModel()->setNameFilter(nameFilter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->setRootItem(this->rootItem());
}

//--------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeView::nameFilter()const
{
  return this->sortFilterProxyModel()->nameFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setNodeTypes(const QStringList& types)
{
  this->sortFilterProxyModel()->setNodeTypes(types);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->setRootItem(this->rootItem());
}

//--------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::nodeTypes()const
{
  return this->sortFilterProxyModel()->nodeTypes();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setHideChildNodeTypes(const QStringList& types)
{
  this->sortFilterProxyModel()->setHideChildNodeTypes(types);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->setRootItem(this->rootItem());
}

//--------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::hideChildNodeTypes()const
{
  return this->sortFilterProxyModel()->hideChildNodeTypes();
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::toggleSubjectHierarchyItemVisibility(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    return;
    }
  if (!itemID)
    {
    return;
    }

  // If more than 10 item visibilities are changed, then enter in batch processing state
  vtkNew<vtkIdList> childItemsList;
  d->SubjectHierarchyNode->GetItemChildren(itemID, childItemsList, true);
  bool batchProcessing = (childItemsList->GetNumberOfIds() > 10);
  if (batchProcessing)
    {
    d->SubjectHierarchyNode->GetScene()->StartState(vtkMRMLScene::BatchProcessState);
    }

  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
    qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(itemID);
  if (!ownerPlugin)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy item " << itemID << " (named "
      << d->SubjectHierarchyNode->GetItemName(itemID).c_str() << ") is not owned by any plugin";
    return;
    }

  int visible = (ownerPlugin->getDisplayVisibility(itemID) > 0 ? 0 : 1);
  ownerPlugin->setDisplayVisibility(itemID, visible);

  if (batchProcessing)
    {
    d->SubjectHierarchyNode->GetScene()->EndState(vtkMRMLScene::BatchProcessState);
    }

  // Trigger view update for the modified item
  d->SubjectHierarchyNode->ItemModified(itemID);
}

//------------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::clickDecoration(QMouseEvent* e)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  QModelIndex index = this->indexAt(e->pos());
  QStyleOptionViewItem opt = this->viewOptions();
  opt.rect = this->visualRect(index);
  qobject_cast<qMRMLItemDelegate*>(this->itemDelegate())->initStyleOption(&opt,index);
  QRect decorationElement = this->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, this);
  if (!decorationElement.contains(e->pos()))
    {
    // Mouse event is not within an item decoration
    return false;
    }

  QModelIndex sourceIndex = this->sortFilterProxyModel()->mapToSource(index);
  if (!(sourceIndex.flags() & Qt::ItemIsEnabled))
    {
    // Item is disabled
    return false;
    }

  // Visibility and color columns
  if ( sourceIndex.column() == this->model()->visibilityColumn()
    || sourceIndex.column() == this->model()->colorColumn() )
    {
    vtkIdType itemID = d->SortFilterModel->subjectHierarchyItemFromIndex(index);
    if (!itemID)
      {
      // Valid item is needed for visibility actions
      return false;
      }

    if (e->button() == Qt::LeftButton && sourceIndex.column() == this->model()->visibilityColumn())
      {
      // Toggle simple visibility
      this->toggleSubjectHierarchyItemVisibility(itemID);
      }

    return true;
    }

  return false;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::mousePressEvent(QMouseEvent* e)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    return;
    }

  // Perform default mouse press event (make selections etc.)
  this->QTreeView::mousePressEvent(e);

  if (e->button() == Qt::LeftButton)
    {
    // Custom left button action for item decorations (i.e. icon): simple visibility toggle
    if (this->clickDecoration(e))
      {
      return;
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::keyPressEvent(QKeyEvent* e)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (e->key() == Qt::Key_Space)
    {
    // Show/hide current item(s) using space
    QList<vtkIdType> currentItemIDs = d->SelectedItems;
    foreach (vtkIdType itemID, currentItemIDs)
      {
      this->toggleSubjectHierarchyItemVisibility(itemID);
      }
    }

  this->QTreeView::keyPressEvent(e);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SortFilterModel || !d->SubjectHierarchyNode || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  // Collect selected subject hierarchy items
  QList<vtkIdType> selectedShItems;
  QList<QModelIndex> selectedIndices = this->selectedIndexes();
  foreach (QModelIndex index, selectedIndices)
    {
    // Only consider the first column to avoid duplicates
    if (index.column() != 0)
      {
      continue;
      }
    vtkIdType itemID = this->sortFilterProxyModel()->subjectHierarchyItemFromIndex(index);
    if (itemID)
      {
      selectedShItems << itemID;
      }
    }

  // If no item was selected, then the scene is considered to be selected
  if (selectedShItems.count() == 0)
    {
    selectedShItems << d->SubjectHierarchyNode->GetSceneItemID();
    }

  // Set current item(s) to plugin handler
  qSlicerSubjectHierarchyPluginHandler::instance()->setCurrentItems(selectedShItems);

  // Cache selected item(s) so that currentItem and currentItems can return them quickly
  d->SelectedItems = selectedShItems;

  // Highlight items referenced by DICOM in case of single-selection
  //   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  //   -> SH item instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  if (this->highlightReferencedItems())
    {
    this->applyReferenceHighlightForItems(selectedShItems);
    }

  // Emit current item changed signal
  vtkIdType newCurrentItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (selectedIndices.count() > 0)
    {
    newCurrentItemID = d->SortFilterModel->subjectHierarchyItemFromIndex(selectedIndices[0]);
    }
  emit currentItemChanged(newCurrentItemID);
  emit currentItemsChanged(selectedShItems);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onItemExpanded(const QModelIndex &expandedItemIndex)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode || !d->SortFilterModel)
    {
    return;
    }

  vtkIdType expandedShItemID = d->SortFilterModel->subjectHierarchyItemFromIndex(expandedItemIndex);
  if (expandedShItemID)
    {
    d->SubjectHierarchyNode->SetItemExpanded(expandedShItemID, true);
    }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onItemCollapsed(const QModelIndex &collapsedItemIndex)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode || !d->SortFilterModel)
    {
    return;
    }

  vtkIdType collapsedShItemID = d->SortFilterModel->subjectHierarchyItemFromIndex(collapsedItemIndex);
  if (collapsedShItemID)
    {
    d->SubjectHierarchyNode->SetItemExpanded(collapsedShItemID, false);
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::populateContextMenuForItem(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  // Have all plugins hide all context menu actions
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    plugin->hideAllContextMenuActions();
    }

  // Show multi-selection context menu if there are more than one selected items,
  // and right-click didn't happen on the scene or the empty area
  if ( d->SelectedItems.size() > 1
    && itemID && itemID != d->SubjectHierarchyNode->GetSceneItemID() )
    {
    // Multi-selection: only show delete and toggle visibility actions
    d->EditAction->setVisible(false);
    d->RenameAction->setVisible(false);
    d->ToggleVisibilityAction->setVisible(true);
    d->SelectPluginSubMenu->menuAction()->setVisible(false);
    return;
    }

  // Single selection
  vtkIdType currentItemID = this->currentItem();
  // If clicked item is the scene or the empty area, then show scene menu regardless the selection
  if (!itemID || itemID == d->SubjectHierarchyNode->GetSceneItemID())
    {
    currentItemID = d->SubjectHierarchyNode->GetSceneItemID();
    }

  // Do not show certain actions for the scene or empty area
  if (!currentItemID || currentItemID == d->SubjectHierarchyNode->GetSceneItemID())
    {
    d->EditAction->setVisible(false);
    d->RenameAction->setVisible(false);
    d->ToggleVisibilityAction->setVisible(false);
    d->SelectPluginSubMenu->menuAction()->setVisible(false);
    }
  else
    {
    d->EditAction->setVisible(d->EditActionVisible);
    d->RenameAction->setVisible(true);
    d->ToggleVisibilityAction->setVisible(false);
    d->SelectPluginSubMenu->menuAction()->setVisible(d->SelectRoleSubMenuVisible);
    }

  // Have all enabled plugins show context menu actions for current item
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, d->enabledPlugins())
    {
    plugin->showContextMenuActionsForItem(currentItemID);
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::populateVisibilityContextMenuForItem(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  if (!itemID || itemID == d->SubjectHierarchyNode->GetSceneItemID())
    {
    qWarning() << Q_FUNC_INFO << ": Invalid subject hierarchy item for visibility context menu: " << itemID;
    return;
    }

  // Have all plugins hide all visibility context menu actions
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    plugin->hideAllContextMenuActions();
    }
  // Have all enabled plugins show visibility context menu actions for current item
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, d->enabledPlugins())
    {
    plugin->showVisibilityContextMenuActionsForItem(itemID);
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::expandItem(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (itemID)
    {
    QModelIndex itemIndex = d->SortFilterModel->indexFromSubjectHierarchyItem(itemID);
    if (itemIndex.isValid())
      {
      this->expand(itemIndex);
      }
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::collapseItem(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (itemID)
    {
    QModelIndex itemIndex = d->SortFilterModel->indexFromSubjectHierarchyItem(itemID);
    if (itemIndex.isValid())
      {
      this->collapse(itemIndex);
      }
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::selectPluginForCurrentItem()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  vtkIdType currentItemID = this->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item for manually selecting role";
    return;
    }
  QString selectedPluginName = d->SelectPluginActionGroup->checkedAction()->data().toString();
  if (selectedPluginName.isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ": No owner plugin found for item " << d->SubjectHierarchyNode->GetItemName(currentItemID).c_str();
    return;
    }
  else if (!selectedPluginName.compare(d->SubjectHierarchyNode->GetItemOwnerPluginName(currentItemID).c_str()))
    {
    // Do nothing if the owner plugin stays the same
    return;
    }

  // Set new owner plugin
  d->SubjectHierarchyNode->SetItemOwnerPluginName(currentItemID, selectedPluginName.toUtf8().constData());
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::updateSelectPluginActions()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  vtkIdType currentItemID = this->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }
  QString ownerPluginName = QString(d->SubjectHierarchyNode->GetItemOwnerPluginName(currentItemID).c_str());

  QList<qSlicerSubjectHierarchyAbstractPlugin*> enabledPluginsList = d->enabledPlugins();

  foreach (QAction* currentSelectPluginAction, d->SelectPluginActions)
    {
    // Check select plugin action if it's the owner
    bool isOwner = !(currentSelectPluginAction->data().toString().compare(ownerPluginName));

    // Get confidence numbers and show the plugins with non-zero confidence
    qSlicerSubjectHierarchyAbstractPlugin* currentPlugin =
      qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName( currentSelectPluginAction->data().toString() );
    double confidenceNumber = currentPlugin->canOwnSubjectHierarchyItem(currentItemID);

    // Do not show plugin in list if confidence is 0, or if it's disabled (by whitelist or blacklist).
    // Always show owner plugin.
    if ( (confidenceNumber <= 0.0 || !enabledPluginsList.contains(currentPlugin))
      && !isOwner )
      {
      currentSelectPluginAction->setVisible(false);
      }
    else
      {
      // Set text to display for the role
      QString role = currentPlugin->roleForPlugin();
      QString currentSelectPluginActionText = QString("%1: '%2', (%3%)").arg(
        role).arg(currentPlugin->displayedItemName(currentItemID)).arg(confidenceNumber*100.0, 0, 'f', 0);
      currentSelectPluginAction->setText(currentSelectPluginActionText);
      currentSelectPluginAction->setVisible(true);
      }

    currentSelectPluginAction->setChecked(isOwner);
    }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::renameCurrentItem()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  vtkIdType currentItemID = this->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  // Pop up an entry box for the new name, with the old name as default
  QString oldName = QString(d->SubjectHierarchyNode->GetItemName(currentItemID).c_str());

  bool ok = false;
  QString newName = QInputDialog::getText(this, "Rename " + oldName, "New name:", QLineEdit::Normal, oldName, &ok);
  if (!ok)
    {
    return;
    }
  d->SubjectHierarchyNode->SetItemName(currentItemID, newName.toUtf8().constData());
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::editCurrentItem()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  vtkIdType currentItemID = this->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
    qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(currentItemID);
  ownerPlugin->editProperties(currentItemID);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::deleteSelectedItems()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  // Remove each selected item
  QList<vtkIdType> currentItemIDs = d->SelectedItems;
  foreach (vtkIdType itemID, currentItemIDs)
    {
    if (itemID == d->SubjectHierarchyNode->GetSceneItemID())
      {
      // Do not delete scene (if no item is selected then the scene will be marked as selected)
      continue;
      }
    // Ask the user whether to delete all the item's children
    bool deleteChildren = false;
    QMessageBox::StandardButton answer = QMessageBox::Yes;
    if ( currentItemIDs.count() > 1
      && !qSlicerSubjectHierarchyPluginHandler::instance()->autoDeleteSubjectHierarchyChildren() )
      {
      answer =
        QMessageBox::question(nullptr, tr("Delete subject hierarchy branch?"),
        tr("The deleted subject hierarchy item has children. "
            "Do you want to remove those too?\n\n"
            "If you choose yes, the whole branch will be deleted, including all children.\n"
            "If you choose Yes to All, this question never appears again, and all subject hierarchy children "
            "are automatically deleted. This can be later changed in Application Settings."),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll,
        QMessageBox::No);
      }
    // Delete branch if the user chose yes
    if (answer == QMessageBox::Yes || answer == QMessageBox::YesToAll)
      {
      deleteChildren = true;
      }
    // Save auto-creation flag in settings
    if (answer == QMessageBox::YesToAll)
      {
      qSlicerSubjectHierarchyPluginHandler::instance()->setAutoDeleteSubjectHierarchyChildren(true);
      }

    // Remove item (and if requested its children) and its associated data node if any
    if (!d->SubjectHierarchyNode->RemoveItem(itemID, true, deleteChildren))
      {
      qWarning() << Q_FUNC_INFO << ": Failed to remove subject hierarchy item (ID:"
        << itemID << ", name:" << d->SubjectHierarchyNode->GetItemName(itemID).c_str() << ")";
      }
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::toggleVisibilityOfSelectedItems()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  // Remove items from the list whose ancestor item is also contained
  // to prevent toggling visibility multiple times on the same item
  QList<vtkIdType> consolidatedItemIDs(d->SelectedItems);
  foreach (vtkIdType itemID, d->SelectedItems)
    {
    // Get children recursively for current item
    std::vector<vtkIdType> childItemIDs;
    d->SubjectHierarchyNode->GetItemChildren(itemID, childItemIDs, true);

    // If any of the current item's children is also in the list,
    // then remove that child item from the consolidated list
    std::vector<vtkIdType>::iterator childIt;
    for (childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
      {
      vtkIdType childItemID = (*childIt);
      if (d->SelectedItems.contains(childItemID))
        {
        consolidatedItemIDs.removeOne(childItemID);
        }
      }
    }

  // Toggle visibility on the remaining items
  foreach (vtkIdType itemID, consolidatedItemIDs)
    {
    this->toggleSubjectHierarchyItemVisibility(itemID);
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::expandToDepthFromContextMenu()
{
  QAction* senderAction = qobject_cast<QAction*>(this->sender());
  if (!senderAction)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get sender action";
    return;
    }

  int depth = senderAction->text().toInt();
  this->expandToDepth(depth);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::applyReferenceHighlightForItems(QList<vtkIdType> itemIDs)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
    }
  if (scene->IsImporting())
    {
    return;
    }

  // Get scene model and column to highlight
  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->model());
  int nameColumn = sceneModel->nameColumn();

  // Clear highlight for previously highlighted items
  foreach (vtkIdType highlightedItemID, d->HighlightedItems)
    {
    QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(highlightedItemID, nameColumn);
    if (item)
      {
      item->setBackground(Qt::transparent);
      }
    }
  d->HighlightedItems.clear();

  // Go through all given items
  foreach (vtkIdType itemID, itemIDs)
    {
    if (itemID == d->SubjectHierarchyNode->GetSceneItemID())
      {
      continue;
      }
    vtkMRMLNode* node = d->SubjectHierarchyNode->GetItemDataNode(itemID);

    // Get items referenced recursively by argument node by MRML
    vtkSmartPointer<vtkCollection> recursivelyReferencedNodes;
    recursivelyReferencedNodes.TakeReference(scene->GetReferencedNodes(node));

    // Get items referenced by argument node by DICOM
    std::vector<vtkIdType> directlyReferencedItems = d->SubjectHierarchyNode->GetItemsReferencedFromItemByDICOM(itemID);
    // Get items referenced directly by argument node by MRML
    if (node)
      {
      vtkSmartPointer<vtkCollection> referencedNodes;
      referencedNodes.TakeReference(scene->GetReferencedNodes(node, false));
      for (int index=0; index!=referencedNodes->GetNumberOfItems(); ++index)
        {
        vtkIdType nodeItemID = d->SubjectHierarchyNode->GetItemByDataNode(
          vtkMRMLNode::SafeDownCast(referencedNodes->GetItemAsObject(index)) );
        if ( nodeItemID && nodeItemID != itemID
          && (std::find(directlyReferencedItems.begin(), directlyReferencedItems.end(), nodeItemID) == directlyReferencedItems.end()) )
          {
          directlyReferencedItems.push_back(nodeItemID);
          }
        }
      }

    // Get items referencing the argument node by DICOM
    std::vector<vtkIdType> referencingItems = d->SubjectHierarchyNode->GetItemsReferencingItemByDICOM(itemID);
    // Get items referencing the argument node by MRML
    if (node)
      {
      std::vector<vtkMRMLNode*> referencingNodes;
      scene->GetReferencingNodes(node, referencingNodes);
      for (std::vector<vtkMRMLNode*>::iterator refNodeIt=referencingNodes.begin(); refNodeIt!=referencingNodes.end(); refNodeIt++)
        {
        vtkIdType nodeItemID = d->SubjectHierarchyNode->GetItemByDataNode(*refNodeIt);
        if ( nodeItemID && nodeItemID != itemID
          && (std::find(referencingItems.begin(), referencingItems.end(), nodeItemID) == referencingItems.end()) )
          {
          referencingItems.push_back(nodeItemID);
          }
        }
      }

    // Highlight recursively referenced items
    for (int index=0; index!=recursivelyReferencedNodes->GetNumberOfItems(); ++index)
      {
      vtkIdType referencedItem = d->SubjectHierarchyNode->GetItemByDataNode(
        vtkMRMLNode::SafeDownCast(recursivelyReferencedNodes->GetItemAsObject(index)) );
      if (referencedItem && referencedItem != itemID)
        {
        QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(referencedItem, nameColumn);
        if (item && !d->HighlightedItems.contains(referencedItem))
          {
          item->setBackground(QColor::fromRgb(255, 255, 170));
          d->HighlightedItems.append(referencedItem);
          }
        }
      }
    // Highlight directly referenced items
    std::vector<vtkIdType>::iterator itemIt;
    for (itemIt=directlyReferencedItems.begin(); itemIt!=directlyReferencedItems.end(); ++itemIt)
      {
      vtkIdType referencedItem = (*itemIt);
      QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(referencedItem, nameColumn);
      if (item) // Note: these items have been added as the recursively referenced items already
        {
        item->setBackground(Qt::yellow);
        }
      }
    // Highlight referencing items
    for (itemIt=referencingItems.begin(); itemIt!=referencingItems.end(); ++itemIt)
      {
      vtkIdType referencingItem = (*itemIt);
      QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(referencingItem, nameColumn);
      if (item && !d->HighlightedItems.contains(referencingItem))
        {
        item->setBackground(QColor::fromRgb(69, 204, 69));
        d->HighlightedItems.append(referencingItem);
        }
      }
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setMultiSelection(bool multiSelectionOn)
{
  if (multiSelectionOn)
    {
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
  else
    {
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    }
}

//-----------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::multiSelection()
{
  return (this->selectionMode() == QAbstractItemView::ExtendedSelection);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setPluginWhitelist(QStringList whitelist)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->PluginWhitelist = whitelist;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setPluginBlacklist(QStringList blacklist)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->PluginBlacklist = blacklist;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::disablePlugin(QString plugin)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->PluginBlacklist << plugin;
}

//-----------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyTreeView::firstSelectedSubjectHierarchyItemInBranch(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  // Check if item itself is selected
  if (d->SelectedItems.contains(itemID))
    {
    return itemID;
    }

  // Look for selected item in children recursively
  std::vector<vtkIdType> childItemIDs;
  d->SubjectHierarchyNode->GetItemChildren(itemID, childItemIDs, true);
  for (std::vector<vtkIdType>::iterator childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
    {
    vtkIdType selectedId = this->firstSelectedSubjectHierarchyItemInBranch(*childIt);
    if (selectedId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      return selectedId;
      }
    }

  // That item is not selected and does not have
  // any children items selected
  return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onSubjectHierarchyItemModified(vtkObject *caller, void *callData)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(caller);
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get item ID
  vtkIdType itemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (callData)
    {
    vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
    itemID = *itemIdPtr;
    }

  // Highlight items referenced by DICOM in case of single-selection
  //   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  //   -> SH item instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  if (this->highlightReferencedItems() && d->SelectedItems.count() == 1)
    {
    this->applyReferenceHighlightForItems(d->SelectedItems);
    }

  // Forward `currentItemModified` if the modified item or one of
  // its children was selected, to adequately update other widgets
  // that use that modified item such as qMRMLSubjectHierarchyComboBox
  vtkIdType selectedId = this->firstSelectedSubjectHierarchyItemInBranch(itemID);
  if (selectedId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    emit currentItemModified(selectedId);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onMRMLSceneCloseEnded(vtkObject* sceneObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
    {
    return;
    }

  Q_D(qMRMLSubjectHierarchyTreeView);

  // Remove selection
  QList<vtkIdType> emptySelection;
  this->setCurrentItems(emptySelection);
  d->SelectedItems.clear();
  d->HighlightedItems.clear();


  // Get new subject hierarchy node (or if not created yet then trigger creating it, because
  // scene close removed the pseudo-singleton subject hierarchy node), and set it to the tree view
  this->setSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene));
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onMRMLSceneStartBatchProcess(vtkObject* sceneObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
    {
    return;
    }

  Q_D(qMRMLSubjectHierarchyTreeView);
  d->SelectedItemsToRestore = d->SelectedItems;
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onMRMLSceneEndBatchProcess(vtkObject* sceneObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
    {
    return;
    }

  Q_D(qMRMLSubjectHierarchyTreeView);
  this->setCurrentItems(d->SelectedItemsToRestore);
  d->SelectedItemsToRestore.clear();
}

//------------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::showContextMenuHint(bool visibility/*=false*/)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return false;
    }

  // Get current item
  vtkIdType itemID = this->currentItem();
  if (!itemID || !d->SubjectHierarchyNode->GetDisplayNodeForItem(itemID))
    {
    // If current item is not displayable, then find first displayable leaf item
    itemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    std::vector<vtkIdType> childItems;
    d->SubjectHierarchyNode->GetItemChildren(d->SubjectHierarchyNode->GetSceneItemID(), childItems, true);
    for (std::vector<vtkIdType>::iterator childIt=childItems.begin(); childIt!=childItems.end(); ++childIt)
      {
      std::vector<vtkIdType> currentChildItems;
      d->SubjectHierarchyNode->GetItemChildren(*childIt, currentChildItems);
      if ( (currentChildItems.empty() || d->SubjectHierarchyNode->IsItemVirtualBranchParent(*childIt)) // Leaf
        && ( d->SubjectHierarchyNode->GetDisplayNodeForItem(*childIt) // Displayable
          || vtkMRMLScalarVolumeNode::SafeDownCast(d->SubjectHierarchyNode->GetItemDataNode(*childIt)) ) ) // Volume
        {
        itemID = (*childIt);
        break;
        }
      }
    }
  if (!itemID)
    {
    // No displayable item in subject hierarchy
    return false;
    }

  // Create information icon
  QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
  QPixmap pixmap = icon.pixmap(32,32);
  QByteArray data;
  QBuffer buffer(&data);
  pixmap.save(&buffer, "PNG", 100);
  QString iconHtml = QString("<img src='data:image/png;base64, %0'>").arg(QString(data.toBase64()));

  if (!visibility)
    {
    // Get name cell position
    QModelIndex nameIndex = this->sortFilterProxyModel()->indexFromSubjectHierarchyItem(
      itemID, this->model()->nameColumn() );
    QRect nameRect = this->visualRect(nameIndex);

    // Show name tooltip
    QString nameTooltip = QString(
      "<div align=\"left\" style=\"font-size:10pt;\"><!--&uarr;<br/>-->Right-click an item<br/>to access additional<br/>options</div><br/>")
      + iconHtml;
    QToolTip::showText(
      this->mapToGlobal( QPoint( nameRect.x() + nameRect.width()/6, nameRect.y() + nameRect.height() ) ),
      nameTooltip );
    }
  else
    {
    // Get visibility cell position
    QModelIndex visibilityIndex = this->sortFilterProxyModel()->indexFromSubjectHierarchyItem(
      itemID, this->model()->visibilityColumn() );
    QRect visibilityRect = this->visualRect(visibilityIndex);

    // Show visibility tooltip
    QString visibilityTooltip = QString(
      "<div align=\"left\" style=\"font-size:10pt;\"><!--&uarr;<br/>-->Right-click the visibility<br/>"
      "button of an item to<br/>access additional<br/>visibility options</div><br/>")
      + iconHtml;
    QToolTip::showText( this->mapToGlobal( QPoint( visibilityRect.x() + visibilityRect.width()/2, visibilityRect.y() + visibilityRect.height() ) ),
      visibilityTooltip );
    }

  return true;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onCustomContextMenu(const QPoint& point)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  if (!d->ContextMenuEnabled)
    {
    // Context menu not enabled, ignore the event
    return;
    }

  QPoint globalPoint = this->viewport()->mapToGlobal(point);

  // Custom right button actions for item decorations (i.e. icon): visibility context menu
  QModelIndex index = this->indexAt(point);
  QStyleOptionViewItem opt = this->viewOptions();
  opt.rect = this->visualRect(index);
  qobject_cast<qMRMLItemDelegate*>(this->itemDelegate())->initStyleOption(&opt, index);
  QRect decorationElement = this->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, this);
  if (decorationElement.contains(point))
    {
    // Mouse event is within an item decoration
    QModelIndex sourceIndex = this->sortFilterProxyModel()->mapToSource(index);
    if (sourceIndex.flags() & Qt::ItemIsEnabled)
      {
      // Item is enabled
      if (sourceIndex.column() == this->model()->visibilityColumn()
        || sourceIndex.column() == this->model()->colorColumn())
        {
        vtkIdType itemID = d->SortFilterModel->subjectHierarchyItemFromIndex(index);
        if (itemID) // Valid item is needed for visibility actions
          {
          // If multiple items are selected then show the node menu instead of the visibility menu
          if (d->SelectedItems.size() > 1)
            {
            this->populateContextMenuForItem(itemID);
            d->NodeMenu->exec(globalPoint);
            }
          else
            {
            // Populate then show visibility context menu if only one item is selected
            this->populateVisibilityContextMenuForItem(itemID);
            d->VisibilityMenu->exec(globalPoint);
            }
          return;
          }
        }
      }
    }

  // Get subject hierarchy item at mouse click position
  vtkIdType itemID = this->sortFilterProxyModel()->subjectHierarchyItemFromIndex(index);
  // Populate context menu for the current item
  this->populateContextMenuForItem(itemID);
  // Show context menu
  if (!itemID || itemID == d->SubjectHierarchyNode->GetSceneItemID())
    {
    d->SceneMenu->exec(globalPoint);
    }
  else
    {
    d->NodeMenu->exec(globalPoint);
    }
}
