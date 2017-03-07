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
#include <QInputDialog>
#include <QDebug>

// SlicerQt includes
#include "qSlicerApplication.h"

// SubjectHierarchy includes
#include "qMRMLSubjectHierarchyTreeView.h"

#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"
#include "qMRMLTransformItemDelegate.h"

#include "vtkSlicerSubjectHierarchyModuleLogic.h"

#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>

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

  /// Save the current expansion state of child items
  void saveChildrenExpandState(QModelIndex& parentIndex);

public:
  qMRMLSubjectHierarchyModel* Model;
  qMRMLSortFilterSubjectHierarchyProxyModel* SortFilterModel;

  bool ShowRootItem;

  QMenu* NodeMenu;
  QAction* RenameAction;
  QAction* DeleteAction;
  QAction* EditAction;
  QList<QAction*> SelectPluginActions;
  QMenu* SelectPluginSubMenu;
  QActionGroup* SelectPluginActionGroup;
  QAction* ExpandToDepthAction;
  QMenu* SceneMenu;

  qMRMLTransformItemDelegate* TransformItemDelegate;

  /// Subject hierarchy node
  vtkWeakPointer<vtkMRMLSubjectHierarchyNode> SubjectHierarchyNode;

  /// Flag determining whether to highlight items referenced by DICOM. Storing DICOM references:
  ///   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  ///   -> SH node instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  bool HighlightReferencedItems;

  /// Cached list of highlighted items to speed up clearing highlight after new selection
  QList<vtkIdType> HighlightedItems;
};

//------------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeViewPrivate::qMRMLSubjectHierarchyTreeViewPrivate(qMRMLSubjectHierarchyTreeView& object)
  : q_ptr(&object)
  , Model(NULL)
  , SortFilterModel(NULL)
  , ShowRootItem(true)
  , RenameAction(NULL)
  , DeleteAction(NULL)
  , EditAction(NULL)
  , SelectPluginSubMenu(NULL)
  , SelectPluginActionGroup(NULL)
  , ExpandToDepthAction(NULL)
  , SceneMenu(NULL)
  , TransformItemDelegate(NULL)
  , SubjectHierarchyNode(NULL)
  , HighlightReferencedItems(true)
{
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::init()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);

  // Set up scene model and sort and proxy model
  this->Model = new qMRMLSubjectHierarchyModel(q);
  //TODO: Needed?
  //QObject::connect( this->Model, SIGNAL(saveTreeExpandState()), q, SLOT(saveTreeExpandState()) );
  //QObject::connect( this->Model, SIGNAL(loadTreeExpandState()), q, SLOT(loadTreeExpandState()) );

  this->SortFilterModel = new qMRMLSortFilterSubjectHierarchyProxyModel(q);
  q->QTreeView::setModel(this->SortFilterModel);
  QObject::connect( q->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                    q, SLOT(onSelectionChanged(QItemSelection,QItemSelection)) );
  this->SortFilterModel->setParent(q);
  this->SortFilterModel->setSourceModel(this->Model);
  //TODO: Needed?
  // Resize the view if new rows are added/removed
  //QObject::connect( this->SortFilterModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
  //                  q, SLOT(onNumberOfVisibleIndexChanged()) );
  //QObject::connect( this->SortFilterModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
  //                  q, SLOT(onNumberOfVisibleIndexChanged()) );
  //q->onNumberOfVisibleIndexChanged();

  //TODO: this would be desirable to set, but results in showing the scrollbar, which makes
  //      subject hierarchy much less usable (because there will be two scrollbars)
  //q->setUniformRowHeights(false);

  // Set up headers
  q->header()->setStretchLastSection(false);
  q->header()->setResizeMode(this->Model->nameColumn(), QHeaderView::Stretch);
  q->header()->setResizeMode(this->Model->visibilityColumn(), QHeaderView::ResizeToContents);
  q->header()->setResizeMode(this->Model->transformColumn(), QHeaderView::Interactive);
  q->header()->setResizeMode(this->Model->idColumn(), QHeaderView::ResizeToContents);

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

  this->SceneMenu = new QMenu(q);
  this->SceneMenu->setObjectName("sceneMenuTreeView");

  // Set item delegate (that creates widgets for certain types of data)
  this->TransformItemDelegate = new qMRMLTransformItemDelegate(q);
  this->TransformItemDelegate->setFixedRowHeight(16);
  this->TransformItemDelegate->setMRMLScene(q->mrmlScene());
  q->setItemDelegateForColumn(this->Model->transformColumn(), this->TransformItemDelegate);
  QObject::connect( this->TransformItemDelegate, SIGNAL(removeTransformsFromBranchOfCurrentItem()),
    this->Model, SLOT(onRemoveTransformsFromBranchOfCurrentItem()) );
  QObject::connect( this->TransformItemDelegate, SIGNAL(hardenTransformOnBranchOfCurrentItem()),
    this->Model, SLOT(onHardenTransformOnBranchOfCurrentItem()) );

  // Make connections
  QObject::connect( this->Model, SIGNAL(invalidateFilter()), this->SortFilterModel, SLOT(invalidate()) );
  //TODO:
  QObject::connect( q, SIGNAL(expanded(const QModelIndex&)), q, SLOT(onItemExpanded(const QModelIndex&)) );

  // Set up scene and node actions for the tree view
  this->setupActions();
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
void qMRMLSubjectHierarchyTreeViewPrivate::saveChildrenExpandState(QModelIndex &parentIndex)
{
//TODO:
  //Q_Q(qMRMLTreeView);
  //vtkMRMLNode* parentNode = q->sortFilterProxyModel()->mrmlNodeFromIndex(parentIndex);

  //// Check if the node is currently present in the scene.
  //// When a node/hierarchy is being deleted from the vtkMRMLScene, there is
  //// some reference of the deleted node left dangling in the qMRMLSceneModel.
  //// As a result, mrmlNodeFromIndex returns a reference to a non-existent node.
  //// We do not need to save the tree hierarchy in such cases.
  //if (!parentNode ||
  //    !q->sortFilterProxyModel()->mrmlScene()->IsNodePresent(parentNode))
  //  {
  //  return;
  //  }

  //  if (q->isExpanded(parentIndex))
  //    {
  //    this->ExpandedNodes->AddItem(parentNode);
  //    }
  //  // Iterate over children nodes recursively to save their expansion state
  //  unsigned int numChildrenRows = q->sortFilterProxyModel()->rowCount(parentIndex);
  //  for(unsigned int row = 0; row < numChildrenRows; ++row)
  //    {
  //    QModelIndex childIndex = q->sortFilterProxyModel()->index(row, 0, parentIndex);
  //    this->saveChildrenExpandState(childIndex);
  //    }
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
qMRMLSubjectHierarchyTreeView::~qMRMLSubjectHierarchyTreeView()
{
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* shNode)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy node";
    return;
    }

  vtkMRMLScene* scene = shNode->GetScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Given subject hierarchy node is not in a MRML scene";
    }

  d->SubjectHierarchyNode = shNode;

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
  return d->Model ? d->Model->mrmlScene() : NULL;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setMRMLScene(vtkMRMLScene* scene)
{
  this->setSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene));
}

//------------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyTreeView::currentItem()const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setCurrentItem(vtkIdType itemID)
{
  qSlicerSubjectHierarchyPluginHandler::instance()->setCurrentItem(itemID);
  emit currentItemChanged(itemID);
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

  QModelIndex treeRootIndex;
  if (rootItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->model());
    treeRootIndex = sceneModel->invisibleRootItem()->index();
    }
  if (rootItemID == d->SubjectHierarchyNode->GetSceneItemID())
    {
    treeRootIndex = this->sortFilterProxyModel()->subjectHierarchySceneIndex();
    }
  else
    {
    treeRootIndex = this->sortFilterProxyModel()->indexFromSubjectHierarchyItem(rootItemID);
    if (d->ShowRootItem)
      {
      // Hide the siblings of the root item
      this->sortFilterProxyModel()->setHideItemsUnaffiliatedWithItemID(rootItemID);
      // The parent of the root node becomes the root for QTreeView.
      treeRootIndex = treeRootIndex.parent();
      rootItemID = this->sortFilterProxyModel()->subjectHierarchyItemFromIndex(treeRootIndex);
      }
    }
  //TODO: Connect SH node's item modified event if necessary
  //qvtkReconnect(this->rootItem(), rootItemID, vtkCommand::ModifiedEvent,
  //              this, SLOT(updateRootItem(vtkObject*)));
  this->setRootIndex(treeRootIndex);
}

//--------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyTreeView::rootItem()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  vtkIdType treeRootItem = this->sortFilterProxyModel()->subjectHierarchyItemFromIndex(this->rootIndex());
  if ( d->ShowRootItem && this->mrmlScene()
    && this->sortFilterProxyModel()->hideItemsUnaffiliatedWithItemID() != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    treeRootItem = this->sortFilterProxyModel()->hideItemsUnaffiliatedWithItemID();
    }
  return treeRootItem;
}

//--------------------------------------------------------------------------
//void qMRMLSubjectHierarchyTreeView::updateRootItem(vtkObject* node)
//{
//TODO:
  //// Maybe the node has changed of QModelIndex, need to re-sync
  //this->setRootItem(vtkMRMLNode::SafeDownCast(node));
//}

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

//------------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::clickDecoration(const QModelIndex& index)
{
  bool result = false;
  QModelIndex sourceIndex = this->sortFilterProxyModel()->mapToSource(index);
  if (!(sourceIndex.flags() & Qt::ItemIsEnabled))
    {
    result = false;
    }
  else if (sourceIndex.column() == this->model()->visibilityColumn())
    {
    this->toggleVisibility(index);
    result = true;
    }

  //TODO: Used by event translator, which seems to only be used for QtTesting test cases
  //if (result)
  //  {
  //  emit decorationClicked(index);
  //  }
  return result;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::toggleVisibility(const QModelIndex& index)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  vtkIdType itemID = d->SortFilterModel->subjectHierarchyItemFromIndex(index);
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    return;
    }

  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
    qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(itemID);

  int visible = (ownerPlugin->getDisplayVisibility(itemID) > 0 ? 0 : 1);
  ownerPlugin->setDisplayVisibility(itemID, visible);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::updateGeometries()
{
  // Don't update the geometries if it's not visible on screen
  // UpdateGeometries is for tree child widgets geometry
  if (!this->isVisible())
    {
    return;
    }
  this->QTreeView::updateGeometries();
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

//TODO: Move to selection changed?
  // Collect selected subject hierarchy items
  QList<vtkIdType> selectedShItems;
  QList<QModelIndex> selectedIndices = this->selectedIndexes();
  foreach(QModelIndex index, selectedIndices)
    {
    // Only consider the first column to avoid duplicates
    if (index.column() != 0)
      {
      continue;
      }
    vtkIdType itemID = this->sortFilterProxyModel()->subjectHierarchyItemFromIndex(index);
    if (itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
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

  // Highlight items referenced by DICOM in case of single-selection
  //   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  //   -> SH item instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  if (this->highlightReferencedItems())
    {
    this->applyReferenceHighlightForItems(selectedShItems);
    }

  // Not the right button clicked, handle events the default way
  if (e->button() == Qt::RightButton)
    {
    QModelIndex index = this->indexAt(e->pos()); // Get the index of the current column
    vtkIdType itemID = this->sortFilterProxyModel()->subjectHierarchyItemFromIndex(index);

    // Make sure the shown context menu is up-to-date
    this->populateContextMenuForItem(itemID);

    // Show context menu
    if (itemID == d->SubjectHierarchyNode->GetSceneItemID() || itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      d->SceneMenu->exec(e->globalPos());
      }
    else
      {
      d->NodeMenu->exec(e->globalPos());
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::LeftButton)
    {
    // Get the index of the current column
    QModelIndex index = this->indexAt(e->pos());
    QStyleOptionViewItemV4 opt = this->viewOptions();
    opt.rect = this->visualRect(index);
    //TODO: Needed? (Here only transform item delegate is used)
    //qobject_cast<qMRMLItemDelegate*>(this->itemDelegate())->initStyleOption(&opt,index);
    QRect decorationElement = this->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, this);
    //decorationElement.translate(this->visualRect(index).topLeft());
    if (decorationElement.contains(e->pos()))
      {
      if (this->clickDecoration(index))
        {
        return;
        }
      }
    }

  this->QTreeView::mouseReleaseEvent(e);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(deselected);
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SortFilterModel)
    {
    return;
    }

  vtkIdType newCurrentItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (selected.indexes().count() > 0)
    {
    newCurrentItemID = d->SortFilterModel->subjectHierarchyItemFromIndex(selected.indexes()[0]);
    }
  emit currentItemChanged(newCurrentItemID);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onItemExpanded(const QModelIndex &expandedItemIndex)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode || !d->Model)
    {
    return;
    }

  vtkIdType expandedShItemID = d->Model->subjectHierarchyItemFromIndex(expandedItemIndex);
  if (expandedShItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    d->SubjectHierarchyNode->SetItemExpanded(expandedShItemID, true);
    }
}

//------------------------------------------------------------------------------
//void qMRMLSubjectHierarchyTreeView::saveTreeExpandState()
//{
//TODO:
  //Q_D(qMRMLSubjectHierarchyTreeView);
  //// Check if there is a scene loaded
  //QStandardItem* sceneItem = this->sceneModel()->mrmlSceneItem();
  //if (!sceneItem)
  //  {
  //  return;
  //  }
  //// Erase previous tree expand state
  //d->ExpandedNodes->RemoveAllItems();
  //QModelIndex sceneIndex = this->sortFilterProxyModel()->mrmlSceneIndex();

  //// First pass for the scene node
  //vtkMRMLNode* sceneNode = this->sortFilterProxyModel()->mrmlNodeFromIndex(sceneIndex);
  //if (this->isExpanded(sceneIndex))
  //  {
  //  if (sceneNode && this->sortFilterProxyModel()->mrmlScene()->IsNodePresent(sceneNode))
  //    d->ExpandedNodes->AddItem(sceneNode);
  //  }
  //unsigned int numChildrenRows = this->sortFilterProxyModel()->rowCount(sceneIndex);
  //for(unsigned int row = 0; row < numChildrenRows; ++row)
  //  {
  //  QModelIndex childIndex = this->sortFilterProxyModel()->index(row, 0, sceneIndex);
  //  d->saveChildrenExpandState(childIndex);
  //  }
//}

//------------------------------------------------------------------------------
//void qMRMLSubjectHierarchyTreeView::loadTreeExpandState()
//{
//TODO:
  //Q_D(qMRMLSubjectHierarchyTreeView);
  //// Check if there is a scene loaded
  //QStandardItem* sceneItem = this->sceneModel()->mrmlSceneItem();
  //if (!sceneItem)
  //  {
  //  return;
  //  }
  //// Iterate over the vtkCollection of expanded nodes
  //vtkCollectionIterator* iter = d->ExpandedNodes->NewIterator();
  //for(iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  //  {
  //  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(iter->GetCurrentObject());
  //  // Check if the node is currently present in the scene.
  //  if (node && this->sortFilterProxyModel()->mrmlScene()->IsNodePresent(node))
  //    {
  //    // Expand the node
  //    QModelIndex nodeIndex = this->sortFilterProxyModel()->indexFromMRMLNode(node);
  //    this->expand(nodeIndex);
  //    }
  //  }
  //// Clear the vtkCollection now
  //d->ExpandedNodes->RemoveAllItems();
  //iter->Delete();
//}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::populateContextMenuForItem(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  if (!d->SubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  // Get current item(s)
  QList<vtkIdType> currentItemIDs = qSlicerSubjectHierarchyPluginHandler::instance()->currentItems();
  // Show multi-selection context menu if there are more than one selected items,
  // and right-click didn't happen on the scene or the empty area
  if ( currentItemIDs.size() > 1
    && itemID != d->SubjectHierarchyNode->GetSceneItemID() && itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )
    {
    // Multi-selection: only show delete action
    d->EditAction->setVisible(false);
    d->RenameAction->setVisible(false);
    d->SelectPluginSubMenu->menuAction()->setVisible(false);

    // Hide all plugin context menu items
    foreach(qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
      {
      plugin->hideAllContextMenuActions();
      }

    return;
    }

  // Single selection
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  // If clicked item is the scene or the empty area, then show scene menu regardless the selection
  if ( itemID == d->SubjectHierarchyNode->GetSceneItemID()
    || itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )
    {
      currentItemID = d->SubjectHierarchyNode->GetSceneItemID();
    }

  // Do not show certain actions for the scene or empty area
  if ( currentItemID == d->SubjectHierarchyNode->GetSceneItemID()
    || currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )
    {
    d->EditAction->setVisible(false);
    d->SelectPluginSubMenu->menuAction()->setVisible(false);
    }
  else
    {
    d->EditAction->setVisible(true);
    d->SelectPluginSubMenu->menuAction()->setVisible(true);
    }

  // Have all plugins show context menu items for current item
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    plugin->showContextMenuActionsForItem(currentItemID);
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::expandItem(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    QModelIndex nodeIndex = d->SortFilterModel->indexFromSubjectHierarchyItem(itemID);
    this->expand(nodeIndex);
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
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item for manually selecting role!";
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

  // Check if the user is setting the plugin that would otherwise be chosen automatically
  qSlicerSubjectHierarchyAbstractPlugin* mostSuitablePluginByConfidenceNumbers =
    qSlicerSubjectHierarchyPluginHandler::instance()->findOwnerPluginForSubjectHierarchyItem(currentItemID);

  // Set owner plugin auto search flag to false if the user manually selected a plugin other
  // than the most suitable one by confidence numbers
  //TODO: No auto search flag any more. Remove if doesn't cause problems
  //bool mostSuitablePluginByConfidenceNumbersSelected =
  //  !mostSuitablePluginByConfidenceNumbers->name().compare(selectedPluginName);
  //currentNode->SetOwnerPluginAutoSearch(mostSuitablePluginByConfidenceNumbersSelected);

  // Set new owner plugin
  d->SubjectHierarchyNode->SetItemOwnerPluginName(currentItemID, selectedPluginName.toLatin1().constData());
  //qDebug() << Q_FUNC_INFO << ": Owner plugin of subject hierarchy node '"
  //  << currentNode->GetName() << "' has been manually changed to '" << d->SelectPluginActionGroup->checkedAction()->data().toString() << "'";
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
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item!";
    return;
    }
  QString ownerPluginName = QString(d->SubjectHierarchyNode->GetItemOwnerPluginName(currentItemID).c_str());

  foreach (QAction* currentSelectPluginAction, d->SelectPluginActions)
    {
    // Check select plugin action if it's the owner
    bool isOwner = !(currentSelectPluginAction->data().toString().compare(ownerPluginName));

    // Get confidence numbers and show the plugins with non-zero confidence
    qSlicerSubjectHierarchyAbstractPlugin* currentPlugin =
      qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName( currentSelectPluginAction->data().toString() );
    double confidenceNumber = currentPlugin->canOwnSubjectHierarchyItem(currentItemID);

    if (confidenceNumber <= 0.0 && !isOwner)
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

  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item!";
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
  d->SubjectHierarchyNode->SetItemName(currentItemID, newName.toLatin1().constData());
  //emit currentNodeRenamed(newName);
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

  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item!";
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

  QList<vtkIdType> currentItemIDs = qSlicerSubjectHierarchyPluginHandler::instance()->currentItems();
  foreach (vtkIdType itemID, currentItemIDs)
    {
    // Remove the subject hierarchy item and all its children
    //TODO: Ask the user whether to delete all children (snippet on the bottom of this file)
    // If it has an associated data node then remove that too
    if (!d->SubjectHierarchyNode->RemoveItem(itemID))
      {
      qWarning() << Q_FUNC_INFO << ": Failed to remove subject hierarchy item (ID:"
        << itemID << ", name:" << d->SubjectHierarchyNode->GetItemName(itemID).c_str() << ")";
      }
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::expandToDepthFromContextMenu()
{
  QAction* senderAction = qobject_cast<QAction*>(this->sender());
  if (!senderAction)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get sender action!";
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

  // Get scene model and column to highlight
  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->model());
  int nameColumn = sceneModel->nameColumn();

  // Clear highlight for previously highlighted items
  foreach(vtkIdType highlightedItemID, d->HighlightedItems)
    {
    QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(highlightedItemID, nameColumn);
    if (item)
      {
      item->setBackground(Qt::transparent);
      }
    }
  d->HighlightedItems.clear();

  // Go through all given items
  foreach(vtkIdType itemID, itemIDs)
    {
    if (itemID == d->SubjectHierarchyNode->GetSceneItemID())
      {
      continue;
      }

    // Get items referenced by argument node by DICOM
    std::vector<vtkIdType> referencedItems = d->SubjectHierarchyNode->GetItemsReferencedFromItemByDICOM(itemID);

    // Highlight referenced items
    std::vector<vtkIdType>::iterator itemIt;
    for (itemIt=referencedItems.begin(); itemIt!=referencedItems.end(); ++itemIt)
      {
      vtkIdType referencedItem = (*itemIt);
      QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(referencedItem, nameColumn);
      if (item && !d->HighlightedItems.contains(referencedItem))
        {
        item->setBackground(Qt::yellow);
        d->HighlightedItems.append(referencedItem);
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

//TODO: Snippet for asking whether whole branch is to be deleted
  //QMessageBox::StandardButton answer = QMessageBox::Yes;
  //if (!d->AutoDeleteSubjectHierarchyChildren)
  //  {
  //  answer =
  //    QMessageBox::question(NULL, tr("Delete subject hierarchy branch?"),
  //    tr("The deleted subject hierarchy node has children. "
  //        "Do you want to remove those too?\n\n"
  //        "If you choose yes, the whole branch will be deleted, including all children.\n"
  //        "If you choose Yes to All, this question never appears again, and all subject hierarchy children "
  //        "are automatically deleted. This can be later changed in Application Settings."),
  //    QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll,
  //    QMessageBox::No);
  //  }
  //// Delete branch if the user chose yes
  //if (answer == QMessageBox::Yes || answer == QMessageBox::YesToAll)
  //  {
  //  d->DeleteBranchInProgress = true;
  //  for (std::vector<vtkMRMLHierarchyNode*>::iterator childIt = nonVirtualChildNodes.begin();
  //    childIt != nonVirtualChildNodes.end(); ++childIt)
  //    {
  //    scene->RemoveNode(*childIt);
  //    }
  //  d->DeleteBranchInProgress = false;
  //  }
  //// Save auto-creation flag in settings
  //if (answer == QMessageBox::YesToAll)
  //  {
  //  d->AutoDeleteSubjectHierarchyChildren = true;
  //  QSettings *settings = qSlicerApplication::application()->settingsDialog()->settings();
  //  settings->setValue("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren", "true");
  //  }
