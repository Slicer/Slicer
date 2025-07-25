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
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QToolTip>

// SubjectHierarchy includes
#include "qMRMLSubjectHierarchyTreeView.h"

#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"

#include "vtkSlicerSubjectHierarchyModuleLogic.h"

#include "qSlicerApplication.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"
#include "qSlicerSubjectHierarchyPluginLogic.h"

// Terminologies includes
#include "qSlicerTerminologyItemDelegate.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>

// qMRML includes
#include "qMRMLItemDelegate.h"
#include "qMRMLNodeFactory.h"

// VTK includes
#include <vtkIdList.h>

//------------------------------------------------------------------------------
class qMRMLSubjectHierarchyTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSubjectHierarchyTreeView);

protected:
  qMRMLSubjectHierarchyTreeView* const q_ptr;

public:
  enum VisibilityAction
  {
    Show,
    Hide,
    ToggleVisibility
  };

  qMRMLSubjectHierarchyTreeViewPrivate(qMRMLSubjectHierarchyTreeView& object);

  virtual void init();

  /// Update list of menu actions in all context menus
  void updateMenuActions();

  void updateSceneMenuActions();
  void updateVisibilityMenuActions();
  void updateNodeMenuActions();
  void updateTransformMenuActions();

  /// Get list of enabled plugins \sa PluginAllowList \sa PluginBlockList
  QList<qSlicerSubjectHierarchyAbstractPlugin*> enabledPlugins();

  void setSubjectHierarchyItemVisibility(vtkIdType itemID, VisibilityAction visibilityAction);

  /// Set visibility of selected subject hierarchy items
  void setVisibilityOfSelectedItems(VisibilityAction visibilityAction);

  void updateColors();

public:
  qMRMLSubjectHierarchyModel* Model{ nullptr };
  qMRMLSortFilterSubjectHierarchyProxyModel* SortFilterModel{ nullptr };

  qSlicerTerminologyItemDelegate* TerminologyItemDelegate{ nullptr };

  bool ShowRootItem{ false };
  vtkIdType RootItemID{ vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID };

  bool ContextMenuEnabled{ true };
  bool EditActionVisible{ true };
  bool AddNodeMenuActionVisible{ false };
  bool SelectRoleSubMenuVisible{ false };

  QMenu* NodeMenu{ nullptr };
  QList<QAction*> AddNodeActions;
  QAction* RenameAction{ nullptr };
  QAction* DeleteAction{ nullptr };
  QAction* EditAction{ nullptr };
  QAction* HideAction{ nullptr };
  QAction* ShowAction{ nullptr };
  QAction* ToggleVisibilityAction{ nullptr };
  QList<QAction*> SelectPluginActions;
  QAction* SelectPluginAction{ nullptr };
  QMenu* SelectPluginSubMenu{ nullptr };
  QActionGroup* SelectPluginActionGroup{ nullptr };
  QMenu* SceneMenu{ nullptr };
  QMenu* VisibilityMenu{ nullptr };
  QMenu* TransformMenu{ nullptr };
  QStringList PluginAllowList;
  QStringList PluginBlockList;

  /// Subject hierarchy node
  vtkWeakPointer<vtkMRMLSubjectHierarchyNode> SubjectHierarchyNode{ nullptr };

  /// Flag determining whether to highlight items referenced by DICOM. Storing DICOM references:
  ///   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  ///   -> SH node instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  bool HighlightReferencedItems{ true };

  /// Cached list of selected items to return the current selection
  QList<vtkIdType> SelectedItems;

  /// List of selected items to restore at the end of batch processing (the whole tree is rebuilt and selection is lost)
  QList<vtkIdType> SelectedItemsToRestore;

  /// Cached list of highlighted items to speed up clearing highlight after new selection
  QList<vtkIdType> HighlightedItems;

  /// Timestamp of the last update of the context menus. Used to make sure the context menus are always up to date
  QDateTime LastContextMenuUpdateTime;

  QColor IndirectReferenceColor;
  QColor DirectReferenceColor;
  QColor ReferencingColor;
  QColor TransformReferenceColor;

  /// The settings key used to specify whether standard terminologies are used for name and color.
  QString UseTerminologySelectorSettingsKey{ "SubjectHierarchy/UseTerminologySelector" };
  /// Use terminology selector if UseTerminologySelectorSettingsKey is empty
  bool NoSettingsUseTerminologySelector{ false };

  qMRMLNodeFactory* MRMLNodeFactory{ nullptr };
  QHash<QString, QString> NodeTypeLabels;
};

//------------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeViewPrivate::qMRMLSubjectHierarchyTreeViewPrivate(qMRMLSubjectHierarchyTreeView& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::init()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);

  this->MRMLNodeFactory = new qMRMLNodeFactory(q);

  // Set up scene model and sort and proxy model
  this->Model = new qMRMLSubjectHierarchyModel(q);
  QObject::connect(this->Model, SIGNAL(requestExpandItem(vtkIdType)), q, SLOT(expandItem(vtkIdType)));
  QObject::connect(this->Model, SIGNAL(requestCollapseItem(vtkIdType)), q, SLOT(collapseItem(vtkIdType)));
  QObject::connect(this->Model, SIGNAL(requestSelectItems(QList<vtkIdType>)), q, SLOT(setCurrentItems(QList<vtkIdType>)));
  QObject::connect(this->Model, SIGNAL(subjectHierarchyUpdated()), q, SLOT(updateRootItem()));

  this->SortFilterModel = new qMRMLSortFilterSubjectHierarchyProxyModel(q);
  q->QTreeView::setModel(this->SortFilterModel);
  QObject::connect(q->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), q, SLOT(onSelectionChanged(QItemSelection, QItemSelection)));

  this->SortFilterModel->setParent(q);
  this->SortFilterModel->setSourceModel(this->Model);

  // Set up headers
  q->resetColumnSizesToDefault();
  if (this->Model->descriptionColumn() >= 0)
  {
    q->setColumnHidden(this->Model->descriptionColumn(), true);
  }

  // Set generic MRML item delegate
  q->setItemDelegate(new qMRMLItemDelegate(q));

  // Set appropriate defaults
  q->setIndentation(8);
  q->setDragDropMode(QAbstractItemView::InternalMove);
  q->setSelectionMode(QAbstractItemView::ExtendedSelection);
  q->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  // Create default menu actions
  this->NodeMenu = new QMenu(q);
  this->NodeMenu->setObjectName("nodeMenuTreeView");

  this->RenameAction = new QAction(qMRMLSubjectHierarchyTreeView::tr("Rename"), nullptr);
  QObject::connect(this->RenameAction, SIGNAL(triggered()), q, SLOT(renameCurrentItem()));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->RenameAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode, 0);

  this->DeleteAction = new QAction(qMRMLSubjectHierarchyTreeView::tr("Delete"), nullptr);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->DeleteAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode, 1);
  QObject::connect(this->DeleteAction, SIGNAL(triggered()), q, SLOT(deleteSelectedItems()));

  this->EditAction = new QAction(qMRMLSubjectHierarchyTreeView::tr("Edit properties..."), nullptr);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->EditAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode, 2);
  QObject::connect(this->EditAction, SIGNAL(triggered()), q, SLOT(editCurrentItem()));

  this->HideAction = new QAction(qMRMLSubjectHierarchyTreeView::tr("Hide"), nullptr);
  QObject::connect(this->HideAction, SIGNAL(triggered()), q, SLOT(hideSelectedItems()));

  this->ShowAction = new QAction(qMRMLSubjectHierarchyTreeView::tr("Show"), nullptr);
  QObject::connect(this->ShowAction, SIGNAL(triggered()), q, SLOT(showSelectedItems()));

  this->ToggleVisibilityAction = new QAction(qMRMLSubjectHierarchyTreeView::tr("Toggle visibility"), nullptr);
  QObject::connect(this->ToggleVisibilityAction, SIGNAL(triggered()), q, SLOT(toggleVisibilityOfSelectedItems()));

  this->SceneMenu = new QMenu(q);
  this->SceneMenu->setObjectName("sceneMenuTreeView");

  this->VisibilityMenu = new QMenu(q);
  this->VisibilityMenu->setObjectName("visibilityMenuTreeView");

  this->TransformMenu = new QMenu(q);
  this->TransformMenu->setObjectName("transformMenuTreeView");

  this->updateColors();

  // Set item delegate for color column
  this->TerminologyItemDelegate = new qSlicerTerminologyItemDelegate(q);
  this->TerminologyItemDelegate->setUseTerminologySelectorCallback([q] { return q->useTerminologySelector(); });
  q->setItemDelegateForColumn(this->Model->colorColumn(), this->TerminologyItemDelegate);

  q->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(q, SIGNAL(customContextMenuRequested(const QPoint&)), q, SLOT(onCustomContextMenu(const QPoint&)));

  // Make connections
  QObject::connect(this->Model, SIGNAL(invalidateFilter()), this->SortFilterModel, SLOT(invalidate()));
  QObject::connect(q, SIGNAL(expanded(const QModelIndex&)), q, SLOT(onItemExpanded(const QModelIndex&)));
  QObject::connect(q, SIGNAL(collapsed(const QModelIndex&)), q, SLOT(onItemCollapsed(const QModelIndex&)));

  // Set up scene and node actions for the tree view
  this->updateMenuActions();
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
void qMRMLSubjectHierarchyTreeViewPrivate::updateMenuActions()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);

  this->updateSceneMenuActions();
  this->updateVisibilityMenuActions();
  this->updateNodeMenuActions();
  this->updateTransformMenuActions();

  // Connect plugin events to be handled by the tree view
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : this->enabledPlugins())
  {
    QObject::connect(plugin, SIGNAL(requestExpandItem(vtkIdType)), q, SLOT(expandItem(vtkIdType)), Qt::UniqueConnection);
    QObject::connect(plugin, SIGNAL(requestInvalidateFilter()), q->model(), SIGNAL(invalidateFilter()), Qt::UniqueConnection);
  }

  // Populate menu from actions
  this->LastContextMenuUpdateTime = QDateTime::currentDateTimeUtc();
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::updateSceneMenuActions()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);
  QList<QAction*> sceneMenuActions;
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : this->enabledPlugins())
  {
    for (QAction* const action : plugin->sceneContextMenuActions())
    {
      sceneMenuActions.append(action);
    }
  }

  this->AddNodeActions.clear();
  QStringList nodeTypes = q->nodeTypes();
  for (const QString& nodeType : q->nodeTypes())
  {
    QString label = q->nodeTypeLabel(nodeType);
    QAction* addNodeAction = new QAction(qMRMLSubjectHierarchyTreeView::tr("Create new %1").arg(label), nullptr);
    addNodeAction->setProperty("nodeType", nodeType);
    QObject::connect(addNodeAction, SIGNAL(triggered()), q, SLOT(addNode()));
    this->AddNodeActions.append(addNodeAction);
    sceneMenuActions.append(addNodeAction);
  }

  // Populate menu from actions
  qSlicerSubjectHierarchyPluginLogic::buildMenuFromActions(this->SceneMenu, sceneMenuActions);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::updateVisibilityMenuActions()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);
  QList<QAction*> visibilityMenuActions;
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : this->enabledPlugins())
  {
    // Add visibility context menu actions
    for (QAction* const action : plugin->visibilityContextMenuActions())
    {
      visibilityMenuActions.append(action);
    }
  }
  // Populate menu from actions
  qSlicerSubjectHierarchyPluginLogic::buildMenuFromActions(this->VisibilityMenu, visibilityMenuActions);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::updateNodeMenuActions()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);
  QList<QAction*> nodeMenuActions;
  // Add default node actions
  nodeMenuActions.append(this->RenameAction);
  nodeMenuActions.append(this->DeleteAction);
  nodeMenuActions.append(this->EditAction);
  nodeMenuActions.append(this->HideAction);
  nodeMenuActions.append(this->ShowAction);
  nodeMenuActions.append(this->ToggleVisibilityAction);
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : this->enabledPlugins())
  {
    for (QAction* const action : plugin->itemContextMenuActions())
    {
      nodeMenuActions.append(action);
    }
  }

  // Create a plugin selection action for each plugin in a sub-menu
  this->SelectPluginAction = new QAction(qMRMLSubjectHierarchyTreeView::tr("Select plugin"), this->NodeMenu);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->SelectPluginAction, qSlicerSubjectHierarchyAbstractPlugin::SectionFolder, 9);
  nodeMenuActions.append(this->SelectPluginAction);

  this->SelectPluginSubMenu = new QMenu();
  this->SelectPluginAction->setMenu(this->SelectPluginSubMenu);
  this->SelectPluginActionGroup = new QActionGroup(q);
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : this->enabledPlugins())
  {
    QAction* selectPluginAction = new QAction(plugin->name(), q);
    selectPluginAction->setCheckable(true);
    selectPluginAction->setActionGroup(this->SelectPluginActionGroup);
    selectPluginAction->setData(QVariant(plugin->name()));
    this->SelectPluginSubMenu->addAction(selectPluginAction);
    QObject::connect(selectPluginAction, SIGNAL(triggered()), q, SLOT(selectPluginForCurrentItem()));
    this->SelectPluginActions << selectPluginAction;
  }

  // Update actions in owner plugin sub-menu when opened
  QObject::connect(this->SelectPluginSubMenu, SIGNAL(aboutToShow()), q, SLOT(updateSelectPluginActions()));

  // Populate menu from actions
  qSlicerSubjectHierarchyPluginLogic::buildMenuFromActions(this->NodeMenu, nodeMenuActions);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::updateTransformMenuActions()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);
  QList<QAction*> transformMenuActions;
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : this->enabledPlugins())
  {
    for (QAction* const action : plugin->transformContextMenuActions())
    {
      transformMenuActions.append(action);
    }
  }
  // Populate menu from actions
  qSlicerSubjectHierarchyPluginLogic::buildMenuFromActions(this->TransformMenu, transformMenuActions);
}

//------------------------------------------------------------------------------
QList<qSlicerSubjectHierarchyAbstractPlugin*> qMRMLSubjectHierarchyTreeViewPrivate::enabledPlugins()
{
  QList<qSlicerSubjectHierarchyAbstractPlugin*> enabledPluginList;

  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
  {
    QString pluginName = plugin->name();
    bool allowlisted = (this->PluginAllowList.isEmpty() || this->PluginAllowList.contains(pluginName));
    bool blocklisted = (!this->PluginBlockList.isEmpty() && this->PluginBlockList.contains(pluginName));
    if ((allowlisted && !blocklisted) || !pluginName.compare("Default"))
    {
      enabledPluginList << plugin;
    }
  }

  return enabledPluginList;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::updateColors()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);
  QColor textColor = q->palette().color(QPalette::Normal, QPalette::Text);
  if (textColor.lightnessF() < 0.5)
  {
    // Dark text (light background)
    this->IndirectReferenceColor = QColor::fromRgb(255, 255, 170);
    this->DirectReferenceColor = Qt::yellow;
    this->TransformReferenceColor = this->DirectReferenceColor;
    this->ReferencingColor = QColor::fromRgb(69, 204, 69);
  }
  else
  {
    // Light text (darker background needed)
    this->IndirectReferenceColor = QColor::fromRgb(50, 50, 5);
    this->DirectReferenceColor = QColor::fromRgb(100, 100, 10);
    this->TransformReferenceColor = this->DirectReferenceColor;
    this->ReferencingColor = QColor::fromRgb(8, 80, 27);
  }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::setVisibilityOfSelectedItems(VisibilityAction visibilityAction)
{
  if (!this->SubjectHierarchyNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
  }

  // Remove items from the list whose ancestor item is also contained
  // to prevent toggling visibility multiple times on the same item
  QList<vtkIdType> consolidatedItemIDs(this->SelectedItems);
  for (const vtkIdType& itemID : this->SelectedItems)
  {
    // Get children recursively for current item
    std::vector<vtkIdType> childItemIDs;
    this->SubjectHierarchyNode->GetItemChildren(itemID, childItemIDs, true);

    // If any of the current item's children is also in the list,
    // then remove that child item from the consolidated list
    std::vector<vtkIdType>::iterator childIt;
    for (childIt = childItemIDs.begin(); childIt != childItemIDs.end(); ++childIt)
    {
      vtkIdType childItemID = (*childIt);
      if (this->SelectedItems.contains(childItemID))
      {
        consolidatedItemIDs.removeOne(childItemID);
      }
    }
  }

  // Toggle visibility on the remaining items
  for (const vtkIdType& itemID : consolidatedItemIDs)
  {
    this->setSubjectHierarchyItemVisibility(itemID, visibilityAction);
  }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::setSubjectHierarchyItemVisibility(vtkIdType itemID, VisibilityAction visibilityAction)
{
  if (!this->SubjectHierarchyNode)
  {
    return;
  }
  if (!itemID)
  {
    return;
  }
  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(itemID);
  if (!ownerPlugin)
  {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy item " << itemID << " (named " << this->SubjectHierarchyNode->GetItemName(itemID).c_str() << ") is not owned by any plugin";
    return;
  }

  // If more than 10 item visibilities are changed, then enter in batch processing state
  vtkNew<vtkIdList> childItemsList;
  this->SubjectHierarchyNode->GetItemChildren(itemID, childItemsList, true);
  bool batchProcessing = (childItemsList->GetNumberOfIds() > 10);
  if (batchProcessing)
  {
    this->SubjectHierarchyNode->GetScene()->StartState(vtkMRMLScene::BatchProcessState);
  }

  bool visible = true;
  switch (visibilityAction)
  {
    case qMRMLSubjectHierarchyTreeViewPrivate::Hide: visible = false; break;
    case qMRMLSubjectHierarchyTreeViewPrivate::Show: visible = true; break;
    case qMRMLSubjectHierarchyTreeViewPrivate::ToggleVisibility: visible = !ownerPlugin->getDisplayVisibility(itemID); break;
    default: break;
  }
  ownerPlugin->setDisplayVisibility(itemID, visible);

  if (batchProcessing)
  {
    this->SubjectHierarchyNode->GetScene()->EndState(vtkMRMLScene::BatchProcessState);
  }

  // Trigger view update for the modified item
  this->SubjectHierarchyNode->ItemModified(itemID);
}

//------------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSubjectHierarchyTreeView, bool, addNodeMenuActionVisible, AddNodeMenuActionVisible);
CTK_SET_CPP(qMRMLSubjectHierarchyTreeView, bool, setAddNodeMenuActionVisible, AddNodeMenuActionVisible);
CTK_GET_CPP(qMRMLSubjectHierarchyTreeView, QStringList, pluginAllowList, PluginAllowList);
CTK_GET_CPP(qMRMLSubjectHierarchyTreeView, QStringList, pluginBlockList, PluginBlockList);

//------------------------------------------------------------------------------
// qMRMLSubjectHierarchyTreeView
//------------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeView::qMRMLSubjectHierarchyTreeView(QWidget* parent)
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

  qvtkReconnect(shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, this, SLOT(onSubjectHierarchyItemModified(vtkObject*, void*)));
  qvtkReconnect(shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemTransformModifiedEvent, this, SLOT(onSubjectHierarchyItemTransformModified(vtkObject*, void*)));

  if (!shNode)
  {
    d->Model->setMRMLScene(nullptr);
    return;
  }

  vtkMRMLScene* scene = shNode->GetScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Given subject hierarchy node is not in a MRML scene";
  }

  d->Model->setMRMLScene(scene);
  this->setRootItem(shNode->GetSceneItemID());
  this->expandToDepth(4);
}

//------------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qMRMLSubjectHierarchyTreeView::subjectHierarchyNode() const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->SubjectHierarchyNode;
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLSubjectHierarchyTreeView::mrmlScene() const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->Model ? d->Model->mrmlScene() : nullptr;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  if (this->mrmlScene() == scene)
  {
    return;
  }

  this->setSubjectHierarchyNode(scene ? vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene) : nullptr);

  // Connect scene close ended event so that subject hierarchy can be cleared
  qvtkReconnect(scene, vtkMRMLScene::StartCloseEvent, this, SLOT(onMRMLSceneStartClose(vtkObject*)));
  qvtkReconnect(scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onMRMLSceneEndClose(vtkObject*)));
  qvtkReconnect(scene, vtkMRMLScene::StartBatchProcessEvent, this, SLOT(onMRMLSceneStartBatchProcess(vtkObject*)));
  qvtkReconnect(scene, vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onMRMLSceneEndBatchProcess(vtkObject*)));

  d->MRMLNodeFactory->setMRMLScene(scene);

  // Scene menu may need update
  d->updateMenuActions();
}

//------------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyTreeView::currentItem() const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->SelectedItems.count() ? d->SelectedItems[0] : vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSubjectHierarchyTreeView::currentNode() const
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

  for (const vtkIdType& item : d->SelectedItems)
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

  // Get requested selection
  QSet<QModelIndex> requestedSelectedItems;
  for (const vtkIdType& itemID : items)
  {
    QModelIndex itemIndex = d->SortFilterModel->indexFromSubjectHierarchyItem(itemID);
    if (itemIndex.isValid())
    {
      requestedSelectedItems.insert(itemIndex);
    }
  }

  // Get previous selection
  const QModelIndexList previouslySelectedItemsList = this->selectionModel()->selectedRows();
  QSet<QModelIndex> previouslySelectedItems(previouslySelectedItemsList.begin(), previouslySelectedItemsList.end());

  // Deselect items that were previously selected but not requested anymore
  for (const QModelIndex& itemIndex : previouslySelectedItems)
  {
    if (itemIndex.isValid() && !requestedSelectedItems.contains(itemIndex))
    {
      this->selectionModel()->select(itemIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }
  }

  // Select items that are requested but have not been previously selected
  for (const QModelIndex& itemIndex : requestedSelectedItems)
  {
    if (!previouslySelectedItems.contains(itemIndex))
    {
      this->selectionModel()->select(itemIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
  }
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setCurrentItems(vtkIdList* items)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!items)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid item list";
    return;
  }

  QList<vtkIdType> itemsToSelect;
  for (int index = 0; index < items->GetNumberOfIds(); ++index)
  {
    itemsToSelect.append(items->GetId(index));
  }

  this->setCurrentItems(itemsToSelect);
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
qMRMLSortFilterSubjectHierarchyProxyModel* qMRMLSubjectHierarchyTreeView::sortFilterProxyModel() const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!d->SortFilterModel)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid sort filter proxy model";
  }
  return d->SortFilterModel;
}

//--------------------------------------------------------------------------
qMRMLSubjectHierarchyModel* qMRMLSubjectHierarchyTreeView::model() const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!d->Model)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid data model";
  }
  return d->Model;
}

//--------------------------------------------------------------------------
int qMRMLSubjectHierarchyTreeView::displayedItemCount() const
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
bool qMRMLSubjectHierarchyTreeView::showRootItem() const
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

  // TODO: Connect SH node's item modified event if necessary
  // qvtkReconnect(this->rootItem(), rootItemID, vtkCommand::ModifiedEvent,
  //               this, SLOT(updateRootItem(vtkObject*)));

  d->RootItemID = rootItemID;
  this->setRootIndex(treeRootIndex);
}

//--------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyTreeView::rootItem() const
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
  this->setRootItem(this->rootItem());
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::highlightReferencedItems() const
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
bool qMRMLSubjectHierarchyTreeView::contextMenuEnabled() const
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
bool qMRMLSubjectHierarchyTreeView::editMenuActionVisible() const
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
bool qMRMLSubjectHierarchyTreeView::selectRoleSubMenuVisible() const
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
bool qMRMLSubjectHierarchyTreeView::noneEnabled() const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!d->Model)
  {
    return false;
  }
  return d->Model->noneEnabled();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setNoneEnabled(bool enable)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->Model)
  {
    return;
  }
  d->Model->setNoneEnabled(enable);
}

//--------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeView::noneDisplay() const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!d->Model)
  {
    return QString();
  }
  return d->Model->noneDisplay();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setNoneDisplay(const QString& displayName)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->Model)
  {
    return;
  }
  d->Model->setNoneDisplay(displayName);
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setIncludeItemAttributeNamesFilter(QStringList filter)
{
  this->sortFilterProxyModel()->setIncludeItemAttributeNamesFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::includeItemAttributeNamesFilter() const
{
  return this->sortFilterProxyModel()->includeItemAttributeNamesFilter();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setIncludeNodeAttributeNamesFilter(QStringList filter)
{
  this->sortFilterProxyModel()->setIncludeNodeAttributeNamesFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::includeNodeAttributeNamesFilter() const
{
  return this->sortFilterProxyModel()->includeNodeAttributeNamesFilter();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setExcludeItemAttributeNamesFilter(QStringList filter)
{
  this->sortFilterProxyModel()->setExcludeItemAttributeNamesFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::excludeItemAttributeNamesFilter() const
{
  return this->sortFilterProxyModel()->excludeItemAttributeNamesFilter();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setExcludeNodeAttributeNamesFilter(QStringList filter)
{
  this->sortFilterProxyModel()->setExcludeNodeAttributeNamesFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::excludeNodeAttributeNamesFilter() const
{
  return this->sortFilterProxyModel()->excludeNodeAttributeNamesFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setAttributeFilter(const QString& attributeName, const QVariant& attributeValue /*=QVariant()*/)
{
  this->sortFilterProxyModel()->setAttributeNameFilter(attributeName);
  this->sortFilterProxyModel()->setAttributeValueFilter(attributeValue.toString());

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setAttributeNameFilter(QString& attributeName)
{
  this->sortFilterProxyModel()->setAttributeNameFilter(attributeName);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//--------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeView::attributeNameFilter() const
{
  return this->sortFilterProxyModel()->attributeNameFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setAttributeValueFilter(QString& attributeValue)
{
  this->sortFilterProxyModel()->setAttributeValueFilter(attributeValue);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//--------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeView::attributeValueFilter() const
{
  return this->sortFilterProxyModel()->attributeValueFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::removeAttributeFilter()
{
  this->sortFilterProxyModel()->setAttributeNameFilter(QString());
  this->sortFilterProxyModel()->setAttributeValueFilter(QString());

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::addItemAttributeFilter(QString attributeName, QVariant attributeValue /*=QString()*/, bool include /*=true*/)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  this->sortFilterProxyModel()->addItemAttributeFilter(attributeName, attributeValue, include);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::removeItemAttributeFilter(QString attributeName, QVariant attributeValue, bool include)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  this->sortFilterProxyModel()->removeItemAttributeFilter(attributeName, attributeValue, include);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::removeItemAttributeFilter(QString attributeName, bool include)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  this->sortFilterProxyModel()->removeItemAttributeFilter(attributeName, include);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::addNodeAttributeFilter(QString attributeName, QVariant attributeValue /*=QString()*/, bool include /*=true*/, QString className /*=QString()*/)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  this->sortFilterProxyModel()->addNodeAttributeFilter(attributeName, attributeValue, include, className);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::removeNodeAttributeFilter(QString attributeName, QVariant attributeValue, bool include, QString className)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  this->sortFilterProxyModel()->removeNodeAttributeFilter(attributeName, attributeValue, include, className);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::removeNodeAttributeFilter(QString attributeName, bool include)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  this->sortFilterProxyModel()->removeNodeAttributeFilter(attributeName, include);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setLevelFilter(QStringList& levelFilter)
{
  this->sortFilterProxyModel()->setLevelFilter(levelFilter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//--------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::levelFilter() const
{
  return this->sortFilterProxyModel()->levelFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setNameFilter(QString& nameFilter)
{
  this->sortFilterProxyModel()->setNameFilter(nameFilter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//--------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeView::nameFilter() const
{
  return this->sortFilterProxyModel()->nameFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setNodeTypes(const QStringList& types)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  this->sortFilterProxyModel()->setNodeTypes(types);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();

  // Scene menu may need update
  d->updateMenuActions();
}

//--------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::nodeTypes() const
{
  return this->sortFilterProxyModel()->nodeTypes();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setHideChildNodeTypes(const QStringList& types)
{
  this->sortFilterProxyModel()->setHideChildNodeTypes(types);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  this->updateRootItem();
}

//--------------------------------------------------------------------------
QStringList qMRMLSubjectHierarchyTreeView::hideChildNodeTypes() const
{
  return this->sortFilterProxyModel()->hideChildNodeTypes();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setVisibilityColumnVisible(bool visible)
{
  this->setColumnHidden(this->model()->visibilityColumn(), !visible);
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::visibilityColumnVisible()
{
  return !this->isColumnHidden(this->model()->visibilityColumn());
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setIdColumnVisible(bool visible)
{
  this->setColumnHidden(this->model()->idColumn(), !visible);
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::idColumnVisible()
{
  return !this->isColumnHidden(this->model()->idColumn());
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setColorColumnVisible(bool visible)
{
  this->setColumnHidden(this->model()->colorColumn(), !visible);
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::colorColumnVisible()
{
  return !this->isColumnHidden(this->model()->colorColumn());
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setTransformColumnVisible(bool visible)
{
  this->setColumnHidden(this->model()->transformColumn(), !visible);
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::transformColumnVisible()
{
  return !this->isColumnHidden(this->model()->transformColumn());
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setDescriptionColumnVisible(bool visible)
{
  this->setColumnHidden(this->model()->descriptionColumn(), !visible);
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::descriptionColumnVisible()
{
  return !this->isColumnHidden(this->model()->descriptionColumn());
}

//------------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::clickDecoration(QMouseEvent* e)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  QModelIndex index = this->indexAt(e->pos());
  QStyleOptionViewItem opt = this->viewOptions();
  opt.rect = this->visualRect(index);
  qobject_cast<qMRMLItemDelegate*>(this->itemDelegate())->initStyleOption(&opt, index);
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
  if (sourceIndex.column() == this->model()->visibilityColumn() //
      || sourceIndex.column() == this->model()->colorColumn())
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
      d->setSubjectHierarchyItemVisibility(itemID, qMRMLSubjectHierarchyTreeViewPrivate::ToggleVisibility);
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
    for (const vtkIdType& itemID : currentItemIDs)
    {
      d->setSubjectHierarchyItemVisibility(itemID, qMRMLSubjectHierarchyTreeViewPrivate::ToggleVisibility);
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
  for (const QModelIndex& index : selectedIndices)
  {
    // Only consider the first column to avoid duplicates
    if (index.column() != 0)
    {
      continue;
    }
    vtkIdType itemID = this->sortFilterProxyModel()->subjectHierarchyItemFromIndex(index);
    selectedShItems << itemID;
  }
  // Make sure None selection is not mixed with valid item selection
  if (selectedShItems.contains(0) && selectedShItems.size() > 1)
  {
    selectedShItems.removeAll(0);
  }

  // If no item was selected, then the scene is considered to be selected unless None item is enabled
  if (selectedShItems.count() == 0)
  {
    if (!this->noneEnabled())
    {
      selectedShItems << d->SubjectHierarchyNode->GetSceneItemID();
    }
    else
    {
      selectedShItems << vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  }

  // Set current item(s) to plugin handler
  qSlicerSubjectHierarchyPluginHandler::instance()->setCurrentItems(selectedShItems);

  // Cache selected item(s) so that currentItem and currentItems can return them quickly
  d->SelectedItems = selectedShItems;

  // Highlight items referenced by DICOM or node reference
  //   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  //   -> SH item instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  if (this->highlightReferencedItems())
  {
    this->applyReferenceHighlightForItems(selectedShItems);
  }

  // Emit current item changed signal
  emit currentItemChanged(selectedShItems[0]);
  emit currentItemsChanged(selectedShItems);
  emit currentNodeChanged(this->currentNode());
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onItemExpanded(const QModelIndex& expandedItemIndex)
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
void qMRMLSubjectHierarchyTreeView::onItemCollapsed(const QModelIndex& collapsedItemIndex)
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
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
  {
    plugin->hideAllContextMenuActions();
  }

  // Show multi-selection context menu if there are more than one selected items,
  // and right-click didn't happen on the scene or the empty area
  if (d->SelectedItems.size() > 1 //
      && itemID && itemID != d->SubjectHierarchyNode->GetSceneItemID())
  {
    // Multi-selection: only show delete and toggle visibility actions
    d->EditAction->setVisible(false);
    d->RenameAction->setVisible(false);
    d->HideAction->setVisible(true);
    d->ShowAction->setVisible(true);
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

  // "Add new ..." node actions
  bool addNodeActionsVisible = d->AddNodeMenuActionVisible && (!currentItemID || currentItemID == d->SubjectHierarchyNode->GetSceneItemID());
  for (QAction* const addNodeAction : d->AddNodeActions)
  {
    addNodeAction->setVisible(addNodeActionsVisible);
  }

  // Do not show certain actions for the scene or empty area
  if (!currentItemID || currentItemID == d->SubjectHierarchyNode->GetSceneItemID())
  {
    d->EditAction->setVisible(false);
    d->RenameAction->setVisible(false);
    d->HideAction->setVisible(false);
    d->ShowAction->setVisible(false);
    d->ToggleVisibilityAction->setVisible(false);
    d->SelectPluginSubMenu->menuAction()->setVisible(false);
  }
  else
  {

    // Only display "Edit properties..." if the option is enabled and properties can be actually edited
    bool editActionVisible = false;
    if (d->EditActionVisible)
    {
      if (itemID)
      {
        qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(currentItemID);
        if (ownerPlugin)
        {
          editActionVisible = ownerPlugin->canEditProperties(currentItemID);
        }
      }
    }
    d->EditAction->setVisible(editActionVisible);

    d->RenameAction->setVisible(true);
    d->HideAction->setVisible(false);
    d->ShowAction->setVisible(false);
    d->ToggleVisibilityAction->setVisible(false);
    d->SelectPluginSubMenu->menuAction()->setVisible(d->SelectRoleSubMenuVisible);
  }

  // Have all enabled plugins show context menu actions for current item
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : d->enabledPlugins())
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
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
  {
    plugin->hideAllContextMenuActions();
  }
  // Have all enabled plugins show visibility context menu actions for current item
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : d->enabledPlugins())
  {
    plugin->showVisibilityContextMenuActionsForItem(itemID);
  }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::populateTransformContextMenuForItem(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!d->SubjectHierarchyNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
  }
  if (!itemID || itemID == d->SubjectHierarchyNode->GetSceneItemID())
  {
    qWarning() << Q_FUNC_INFO << ": Invalid subject hierarchy item for transform context menu: " << itemID;
    return;
  }

  // Transforms menu is a special case. It requires updates of the transform actions each time the menu is displayed,
  // because transforms may have been added to the scene.
  d->updateTransformMenuActions();

  // Have all plugins hide all transform context menu actions
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
  {
    plugin->hideAllContextMenuActions();
  }
  // Have all enabled plugins show transform context menu actions for current item
  for (qSlicerSubjectHierarchyAbstractPlugin* const plugin : d->enabledPlugins())
  {
    plugin->showTransformContextMenuActionsForItem(itemID);
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

  for (QAction* const currentSelectPluginAction : d->SelectPluginActions)
  {
    // Check select plugin action if it's the owner
    bool isOwner = !(currentSelectPluginAction->data().toString().compare(ownerPluginName));

    // Get confidence numbers and show the plugins with non-zero confidence
    qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName(currentSelectPluginAction->data().toString());
    double confidenceNumber = currentPlugin->canOwnSubjectHierarchyItem(currentItemID);

    // Do not show plugin in list if confidence is 0, or if it's disabled (by allowlist or blocklist).
    // Always show owner plugin.
    if ((confidenceNumber <= 0.0 || !enabledPluginsList.contains(currentPlugin)) //
        && !isOwner)
    {
      currentSelectPluginAction->setVisible(false);
    }
    else
    {
      // Set text to display for the role
      QString role = currentPlugin->roleForPlugin();
      QString currentSelectPluginActionText = QString("%1: '%2', (%3%)").arg(role).arg(currentPlugin->displayedItemName(currentItemID)).arg(confidenceNumber * 100.0, 0, 'f', 0);
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

  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(currentItemID);
  if (!ownerPlugin)
  {
    qCritical() << Q_FUNC_INFO << " failed: Invalid owner plugin";
    return;
  }
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
  for (const vtkIdType& itemID : currentItemIDs)
  {
    if (itemID == d->SubjectHierarchyNode->GetSceneItemID())
    {
      // Do not delete scene (if no item is selected then the scene will be marked as selected)
      continue;
    }
    // Ask the user whether to delete all the item's children
    bool deleteChildren = false;
    QMessageBox::StandardButton answer = QMessageBox::Yes;
    if (currentItemIDs.count() > 1 //
        && !qSlicerSubjectHierarchyPluginHandler::instance()->autoDeleteSubjectHierarchyChildren())
    {
      answer = QMessageBox::question(nullptr,
                                     tr("Delete subject hierarchy branch?"),
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
      qWarning() << Q_FUNC_INFO << ": Failed to remove subject hierarchy item (ID:" << itemID << ", name:" << d->SubjectHierarchyNode->GetItemName(itemID).c_str() << ")";
    }
  }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::hideSelectedItems()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->setVisibilityOfSelectedItems(qMRMLSubjectHierarchyTreeViewPrivate::Hide);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::showSelectedItems()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->setVisibilityOfSelectedItems(qMRMLSubjectHierarchyTreeViewPrivate::Show);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::toggleVisibilityOfSelectedItems()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->setVisibilityOfSelectedItems(qMRMLSubjectHierarchyTreeViewPrivate::ToggleVisibility);
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
  for (const vtkIdType& highlightedItemID : d->HighlightedItems)
  {
    QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(highlightedItemID, nameColumn);
    if (item && this->sortFilterProxyModel()->filterAcceptsItem(highlightedItemID))
    {
      item->setBackground(Qt::transparent);
    }
  }
  d->HighlightedItems.clear();

  // Go through all given items
  for (const vtkIdType& itemID : itemIDs)
  {
    if (itemID == d->SubjectHierarchyNode->GetSceneItemID() || !itemID)
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
      for (int index = 0; index != referencedNodes->GetNumberOfItems(); ++index)
      {
        vtkIdType nodeItemID = d->SubjectHierarchyNode->GetItemByDataNode(vtkMRMLNode::SafeDownCast(referencedNodes->GetItemAsObject(index)));
        if (nodeItemID && nodeItemID != itemID //
            && (std::find(directlyReferencedItems.begin(), directlyReferencedItems.end(), nodeItemID) == directlyReferencedItems.end()))
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
      for (std::vector<vtkMRMLNode*>::iterator refNodeIt = referencingNodes.begin(); refNodeIt != referencingNodes.end(); refNodeIt++)
      {
        vtkIdType nodeItemID = d->SubjectHierarchyNode->GetItemByDataNode(*refNodeIt);
        if (nodeItemID && nodeItemID != itemID //
            && (std::find(referencingItems.begin(), referencingItems.end(), nodeItemID) == referencingItems.end()))
        {
          referencingItems.push_back(nodeItemID);
        }
      }
    }

    // Highlight recursively referenced items
    for (int index = 0; index != recursivelyReferencedNodes->GetNumberOfItems(); ++index)
    {
      vtkIdType referencedItem = d->SubjectHierarchyNode->GetItemByDataNode(vtkMRMLNode::SafeDownCast(recursivelyReferencedNodes->GetItemAsObject(index)));
      if (referencedItem && referencedItem != itemID)
      {
        QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(referencedItem, nameColumn);
        if (item && !d->HighlightedItems.contains(referencedItem) && this->sortFilterProxyModel()->filterAcceptsItem(referencedItem))
        {
          item->setBackground(d->IndirectReferenceColor);
          d->HighlightedItems.append(referencedItem);
        }
      }
    }
    // Highlight directly referenced items
    std::vector<vtkIdType>::iterator itemIt;
    for (itemIt = directlyReferencedItems.begin(); itemIt != directlyReferencedItems.end(); ++itemIt)
    {
      vtkIdType referencedItem = (*itemIt);
      QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(referencedItem, nameColumn);
      // Note: these items have been added as the recursively referenced items already
      if (item && this->sortFilterProxyModel()->filterAcceptsItem(referencedItem))
      {
        item->setBackground(d->DirectReferenceColor);
      }
    }
    // Highlight referencing items
    for (itemIt = referencingItems.begin(); itemIt != referencingItems.end(); ++itemIt)
    {
      vtkIdType referencingItem = (*itemIt);
      QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(referencingItem, nameColumn);
      if (item && !d->HighlightedItems.contains(referencingItem) && this->sortFilterProxyModel()->filterAcceptsItem(referencingItem))
      {
        item->setBackground(d->ReferencingColor);
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
void qMRMLSubjectHierarchyTreeView::setPluginAllowList(QStringList allowlist)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->PluginAllowList = allowlist;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setPluginBlockList(QStringList blocklist)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->PluginBlockList = blocklist;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::disablePlugin(QString plugin)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->PluginBlockList << plugin;
}

//-----------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyTreeView::firstSelectedSubjectHierarchyItemInBranch(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  if (!d->SubjectHierarchyNode)
  {
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  }

  // Check if item itself is selected
  if (d->SelectedItems.contains(itemID))
  {
    return itemID;
  }

  // Look for selected item in children recursively
  std::vector<vtkIdType> childItemIDs;
  d->SubjectHierarchyNode->GetItemChildren(itemID, childItemIDs, true);
  for (std::vector<vtkIdType>::iterator childIt = childItemIDs.begin(); childIt != childItemIDs.end(); ++childIt)
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
void qMRMLSubjectHierarchyTreeView::onSubjectHierarchyItemModified(vtkObject* caller, void* callData)
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
void qMRMLSubjectHierarchyTreeView::onSubjectHierarchyItemTransformModified(vtkObject* caller, void* callData)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  Q_UNUSED(callData);

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(caller);
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->model());
  int nameColumn = sceneModel->nameColumn();

  // Update highlighting based on associated transforms
  // Note: applyReferenceHighlightForItems does not work here because the transform modified event is
  //       invoked before the scene updates the cached node references
  if (this->highlightReferencedItems())
  {
    // Remove highlighting from all transforms
    std::vector<vtkMRMLNode*> transformNodes;
    this->mrmlScene()->GetNodesByClass("vtkMRMLTransformNode", transformNodes);
    for (std::vector<vtkMRMLNode*>::iterator it = transformNodes.begin(); it != transformNodes.end(); ++it)
    {
      vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(*it);
      if (transformNode && !transformNode->GetHideFromEditors())
      {
        vtkIdType transformItem = shNode->GetItemByDataNode(transformNode);
        QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(transformItem, nameColumn);
        if (item)
        {
          item->setBackground(Qt::transparent);
        }
        int transformItemIndex = d->HighlightedItems.indexOf(transformItem);
        if (transformItemIndex >= 0)
        {
          d->HighlightedItems.removeAt(transformItemIndex);
        }
      }
    }

    for (const vtkIdType& itemID : d->SelectedItems)
    {
      vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
      if (node == nullptr)
      {
        continue;
      }
      vtkMRMLTransformNode* transformNode = node->GetParentTransformNode();
      if (transformNode)
      {
        vtkIdType transformItem = shNode->GetItemByDataNode(transformNode);
        QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(transformItem, nameColumn);
        if (item)
        {
          item->setBackground(Qt::yellow);
        }
        d->HighlightedItems.append(transformItem);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onMRMLSceneStartClose(vtkObject* sceneObject)
{
  Q_UNUSED(sceneObject);
  Q_D(qMRMLSubjectHierarchyTreeView);

  // Remove selection
  QList<vtkIdType> emptySelection;
  this->setCurrentItems(emptySelection);
  d->SelectedItems.clear();
  d->HighlightedItems.clear();

  // Do not restore selection after closing the scene
  d->SelectedItemsToRestore.clear();
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::onMRMLSceneEndClose(vtkObject* sceneObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
  {
    return;
  }

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
  if (scene->IsClosing())
  {
    // Do not restore items after closing the scene
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

  if (!d->SelectedItemsToRestore.empty())
  {
    this->setCurrentItems(d->SelectedItemsToRestore);
    d->SelectedItemsToRestore.clear();
  }
}

//------------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::showContextMenuHint(bool visibility /*=false*/)
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
    for (std::vector<vtkIdType>::iterator childIt = childItems.begin(); childIt != childItems.end(); ++childIt)
    {
      std::vector<vtkIdType> currentChildItems;
      d->SubjectHierarchyNode->GetItemChildren(*childIt, currentChildItems);
      if ((currentChildItems.empty() || d->SubjectHierarchyNode->IsItemVirtualBranchParent(*childIt))        // Leaf
          && (d->SubjectHierarchyNode->GetDisplayNodeForItem(*childIt)                                       // Displayable
              || vtkMRMLScalarVolumeNode::SafeDownCast(d->SubjectHierarchyNode->GetItemDataNode(*childIt)))) // Volume
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
  QPixmap pixmap = icon.pixmap(32, 32);
  QByteArray data;
  QBuffer buffer(&data);
  pixmap.save(&buffer, "PNG", 100);
  QString iconHtml = QString("<img src='data:image/png;base64, %0'>").arg(QString(data.toBase64()));

  if (!visibility)
  {
    // Get name cell position
    QModelIndex nameIndex = this->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID, this->model()->nameColumn());
    QRect nameRect = this->visualRect(nameIndex);

    // Show name tooltip
    QString nameTooltip = QString("<div align=\"left\" style=\"font-size:10pt;\"><!--&uarr;<br/>-->Right-click an item<br/>to access additional<br/>options</div><br/>") + iconHtml;
    QToolTip::showText(this->mapToGlobal(QPoint(nameRect.x() + nameRect.width() / 6, nameRect.y() + nameRect.height())), nameTooltip);
  }
  else
  {
    // Get visibility cell position
    QModelIndex visibilityIndex = this->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID, this->model()->visibilityColumn());
    QRect visibilityRect = this->visualRect(visibilityIndex);

    // Show visibility tooltip
    QString visibilityTooltip = QString("<div align=\"left\" style=\"font-size:10pt;\"><!--&uarr;<br/>-->Right-click the visibility<br/>"
                                        "button of an item to<br/>access additional<br/>visibility options</div><br/>")
                                + iconHtml;
    QToolTip::showText(this->mapToGlobal(QPoint(visibilityRect.x() + visibilityRect.width() / 2, visibilityRect.y() + visibilityRect.height())), visibilityTooltip);
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

  if (!d->SubjectHierarchyNode)
  {
    // No subject hierarchy node, ignore the event
    return;
  }

  // Save tree view for the plugins to access if needed
  qSlicerSubjectHierarchyPluginHandler::instance()->setCurrentTreeView(this);

  if (qSlicerSubjectHierarchyPluginHandler::instance()->LastPluginRegistrationTime > d->LastContextMenuUpdateTime)
  {
    d->updateMenuActions();
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
      if (sourceIndex.column() == this->model()->visibilityColumn() //
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
      else if (sourceIndex.column() == this->model()->transformColumn())
      {
        vtkIdType itemID = d->SortFilterModel->subjectHierarchyItemFromIndex(index);
        if (itemID) // Valid item is needed for transform actions
        {
          if (d->SelectedItems.size() > 0)
          {
            // Populate then show transform context menu. Actions are applied to all selected items.
            this->populateTransformContextMenuForItem(itemID);
            d->TransformMenu->exec(globalPoint);
            return;
          }
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

//---------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::changeEvent(QEvent* e)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  switch (e->type())
  {
    case QEvent::PaletteChange:
    {
      d->updateColors();
      QItemSelection selected;
      QItemSelection deselected;
      this->onSelectionChanged(selected, deselected);
      break;
    }
    default: break;
  }
  QTreeView::changeEvent(e);
}

// --------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setUseTerminologySelector(bool useTerminologySelector)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->NoSettingsUseTerminologySelector = useTerminologySelector;
  QSettings().setValue(d->UseTerminologySelectorSettingsKey, useTerminologySelector);
}

// --------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::useTerminologySelector() const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (d->UseTerminologySelectorSettingsKey.isEmpty())
  {
    return d->NoSettingsUseTerminologySelector;
  }
  else
  {
    return QSettings().value(d->UseTerminologySelectorSettingsKey, true).toBool();
  }
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setBaseName(const QString& baseName, const QString& nodeType /* ="" */)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!nodeType.isEmpty())
  {
    d->MRMLNodeFactory->setBaseName(nodeType, baseName);
    return;
  }
  // If no node type is defined then we set the base name for all already specified node types
  QStringList nodeTypes = this->nodeTypes();
  if (nodeTypes.isEmpty())
  {
    qWarning("qMRMLSubjectHierarchyTreeView::setBaseName failed: no node types have been set yet");
    return;
  }
  for (const QString& aNodeType : nodeTypes)
  {
    d->MRMLNodeFactory->setBaseName(aNodeType, baseName);
  }
}

//-----------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeView::baseName(const QString& nodeType /* ="" */) const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  if (!nodeType.isEmpty())
  {
    return d->MRMLNodeFactory->baseName(nodeType);
  }
  // If nodeType is not specified then base name of the first node type is returned.
  QStringList nodeClasses = this->nodeTypes();
  if (nodeClasses.isEmpty())
  {
    qWarning("qMRMLSubjectHierarchyTreeView::baseName failed: no node types have been set yet");
    return QString();
  }
  return d->MRMLNodeFactory->baseName(nodeClasses[0]);
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setNodeTypeLabel(const QString& label, const QString& nodeType)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (nodeType.isEmpty())
  {
    qWarning() << Q_FUNC_INFO << " failed: nodeType is invalid";
    return;
  }
  if (label.isEmpty())
  {
    d->NodeTypeLabels.remove(nodeType);
  }
  else
  {
    d->NodeTypeLabels[nodeType] = label;
  }
}

//-----------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeView::nodeTypeLabel(const QString& nodeType) const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  // If a label was explicitly specified then use that
  if (d->NodeTypeLabels.contains(nodeType))
  {
    return d->NodeTypeLabels[nodeType];
  }
  // Otherwise use the node tag
  if (this->mrmlScene())
  {
    QString label = QString::fromStdString(this->mrmlScene()->GetTypeDisplayNameByClassName(nodeType.toStdString()));
    if (!label.isEmpty())
    {
      return label;
    }
  }
  // Otherwise just label the node as "node"
  return tr("node");
}

// --------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::addNode()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  QAction* action = qobject_cast<QAction*>(this->sender());
  if (!action)
  {
    return;
  }
  QString nodeType = action->property("nodeType").toString();
  this->addNode(nodeType);
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLSubjectHierarchyTreeView::addNode(QString nodeType)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (!this->nodeTypes().contains(nodeType))
  {
    qWarning("qMRMLSubjectHierarchyTreeView::addNode() attempted with node type %s, which is not among the allowed node types", qPrintable(nodeType));
    return nullptr;
  }
  // Create the MRML node via the MRML Scene
  vtkMRMLNode* newNode = d->MRMLNodeFactory->createNode(nodeType);
  // The created node is appended at the bottom of the current list
  if (newNode == nullptr)
  {
    qWarning("qMRMLSubjectHierarchyTreeView::addNode() failed with node type %s", qPrintable(nodeType));
    return nullptr;
  }
  this->setCurrentNode(newNode);
  emit this->nodeAddedByUser(newNode);
  return newNode;
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLSubjectHierarchyTreeView::findFirstNodeByClass(const QString& className) const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  qMRMLSortFilterSubjectHierarchyProxyModel* model = this->sortFilterProxyModel();
  const QModelIndex& rootIndex = model->indexFromSubjectHierarchyItem(this->rootItem());

  QList<QModelIndex> indexes;
  indexes << rootIndex;
  while (!indexes.isEmpty())
  {
    QModelIndex index = indexes.takeFirst();
    if (!index.isValid())
    {
      continue;
    }

    // Return if this is a node that is of the requested class
    vtkIdType itemID = model->subjectHierarchyItemFromIndex(index);
    vtkMRMLNode* dataNode = d->SubjectHierarchyNode->GetItemDataNode(itemID);
    if (dataNode && dataNode->IsA(className.toStdString().c_str()))
    {
      // found a suitable data node
      return dataNode;
    }

    // Add item children to the list
    if (model->hasChildren(index) && !(index.flags() & Qt::ItemNeverHasChildren))
    {
      // depth-first search (children are added to the front of the list)
      for (int i = model->rowCount() - 1; i >= 0; --i)
      {
        indexes.push_front(model->index(i, 0, index));
      }
    }
  }

  // not found a suitable data node
  return nullptr;
}
