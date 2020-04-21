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

// Qt includes
#include <QDebug>
#include <QTimer>

// qMRML includes
#include "qMRMLSceneModel_p.h"

// MRML includes
#include <vtkMRMLDisplayableHierarchyNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkCollection.h>

// STD includes

//------------------------------------------------------------------------------
qMRMLSceneModelPrivate::qMRMLSceneModelPrivate(qMRMLSceneModel& object)
  : q_ptr(&object)
{
  qRegisterMetaType<qMRMLSceneModel::NodeTypes>("qMRMLSceneModel::NodeTypes");

  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->LazyUpdate = false;
  this->ListenNodeModifiedEvent = qMRMLSceneModel::NoNodes;
  this->PendingItemModified = -1; // -1 means not updating

  this->NameColumn = -1;
  this->IDColumn = -1;
  this->CheckableColumn = -1;
  this->VisibilityColumn = -1;
  this->ToolTipNameColumn = -1;
  this->ExtraItemColumn = 0;

  this->HiddenIcon = QIcon(":Icons/VisibleOff.png");
  this->VisibleIcon = QIcon(":Icons/VisibleOn.png");
  this->PartiallyVisibleIcon = QIcon(":Icons/VisiblePartially.png");

  this->MRMLScene = nullptr;
  this->DraggedItem = nullptr;

  qRegisterMetaType<QStandardItem* >("QStandardItem*");
}

//------------------------------------------------------------------------------
qMRMLSceneModelPrivate::~qMRMLSceneModelPrivate()
{
  if (this->MRMLScene)
    {
    this->MRMLScene->RemoveObserver(this->CallBack);
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModelPrivate::init()
{
  Q_Q(qMRMLSceneModel);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qMRMLSceneModel::onMRMLSceneEvent);

  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)));

  q->setNameColumn(0);
  q->setListenNodeModifiedEvent(qMRMLSceneModel::OnlyVisibleNodes);
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLSceneModelPrivate::indexes(const QString& nodeID)const
{
  Q_Q(const qMRMLSceneModel);
  QModelIndex scene = q->mrmlSceneIndex();
  if (scene == QModelIndex())
    {
    return QModelIndexList();
    }
  // QAbstractItemModel::match doesn't browse through columns
  // we need to do it manually
  QModelIndexList nodeIndexes = q->match(
    scene, qMRMLSceneModel::UIDRole, nodeID,
    1, Qt::MatchExactly | Qt::MatchRecursive);
  Q_ASSERT(nodeIndexes.size() <= 1); // we know for sure it won't be more than 1
  if (nodeIndexes.size() == 0)
    {
    return nodeIndexes;
    }
  // Add the QModelIndexes from the other columns
  const int row = nodeIndexes[0].row();
  QModelIndex nodeParentIndex = nodeIndexes[0].parent();
  const int sceneColumnCount = q->columnCount(nodeParentIndex);
  for (int j = 1; j < sceneColumnCount; ++j)
    {
    nodeIndexes << q->index(row, j, nodeParentIndex);
    }
  return nodeIndexes;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelPrivate::listenNodeModifiedEvent()
{
  Q_Q(qMRMLSceneModel);
  QModelIndex sceneIndex = q->mrmlSceneIndex();
  const int count = q->rowCount(sceneIndex);
  for (int i = 0; i < count; ++i)
    {
    vtkMRMLNode* node = q->mrmlNodeFromIndex(sceneIndex.child(i,0));
    q->qvtkDisconnect(node, vtkCommand::NoEvent, q, nullptr);
    if (this->ListenNodeModifiedEvent == qMRMLSceneModel::AllNodes)
      {
      q->observeNode(node);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModelPrivate::insertExtraItem(int row, QStandardItem* parent,
                                             const QString& text,
                                             const QString& extraType,
                                             const Qt::ItemFlags& flags)
{
  Q_ASSERT(parent);

  QList<QStandardItem*> items;
  // fill with empty column items
  for (int column = 0; column < parent->columnCount(); ++column)
    {
    QStandardItem* extraItem = new QStandardItem;
    if (column == this->ExtraItemColumn)
      {
      extraItem->setData(extraType, qMRMLSceneModel::UIDRole);
      if (text == "separator")
        {
        extraItem->setData("separator", Qt::AccessibleDescriptionRole);
        }
      else
        {
        extraItem->setText(text);
        }
      extraItem->setFlags(flags);
      }
    else
      {
      extraItem->setFlags(nullptr);
      }
    items << extraItem;
    }
  parent->insertRow(row, items);

  // update extra item cache info (for faster retrieval)
  QMap<QString, QVariant> extraItems = parent->data(qMRMLSceneModel::ExtraItemsRole).toMap();
  extraItems[extraType] = extraItems[extraType].toStringList() << text;
  parent->setData(extraItems, qMRMLSceneModel::ExtraItemsRole );
}

//------------------------------------------------------------------------------
QStringList qMRMLSceneModelPrivate::extraItems(QStandardItem* parent, const QString& extraType)const
{
  QStringList res;
  if (parent == nullptr)
    {
    //parent = q->invisibleRootItem();
    return res;
    }
  // It is expensive to search, cache the extra items.
  res = parent->data(qMRMLSceneModel::ExtraItemsRole).toMap()[extraType].toStringList();
  /*
  const int rowCount = parent->rowCount();
  for (int i = 0; i < rowCount; ++i)
    {
    QStandardItem* child = parent->child(i);
    if (child && child->data(qMRMLSceneModel::UIDRole).toString() == extraType)
      {
      if (child->data(Qt::AccessibleDescriptionRole) == "separator")
        {
        res << "separator";
        }
      else
        {
        res << child->text();
        }
      }
    }
  */
  return res;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelPrivate::removeAllExtraItems(QStandardItem* parent, const QString extraType)
{
  Q_Q(qMRMLSceneModel);
  Q_ASSERT(parent);
  QMap<QString, QVariant> extraItems =
    parent->data(qMRMLSceneModel::ExtraItemsRole).toMap();
  if (extraItems[extraType].toStringList().size() == 0)
    {
    return;
    }
  QModelIndex start = parent ? parent->index().child(0,0) : QModelIndex().child(0,0);
  QModelIndexList indexes =
    q->match(start, qMRMLSceneModel::UIDRole, extraType, 1, Qt::MatchExactly);
  while (start != QModelIndex() && indexes.size())
    {
    QModelIndex parentIndex = indexes[0].parent();
    int row = indexes[0].row();
    q->removeRow(row, parentIndex);
    // don't start the whole search from scratch, only from where we ended it
    start = parentIndex.child(row,0);
    indexes = q->match(start, qMRMLSceneModel::UIDRole, extraType, 1, Qt::MatchExactly);
    }
  extraItems[extraType] = QStringList();
  parent->setData(extraItems, qMRMLSceneModel::ExtraItemsRole);
}

//------------------------------------------------------------------------------
bool qMRMLSceneModelPrivate::isExtraItem(const QStandardItem* item)const
{
  QString uid =
    item ? item->data(qMRMLSceneModel::UIDRole).toString() : QString();
  return uid == "preItem" || uid == "postItem";
}

//------------------------------------------------------------------------------
void qMRMLSceneModelPrivate::reparentItems(
  QList<QStandardItem*>& children, int newIndex, QStandardItem* newParentItem)
{
  Q_Q(qMRMLSceneModel);
  int min = q->preItems(newParentItem).count();
  int max = newParentItem->rowCount() - q->postItems(newParentItem).count();
  int pos = qMin(min + newIndex, max);
  newParentItem->insertRow(pos, children);
}

//------------------------------------------------------------------------------
// qMRMLSceneModel
//------------------------------------------------------------------------------
qMRMLSceneModel::qMRMLSceneModel(QObject *_parent)
  :QStandardItemModel(_parent)
  , d_ptr(new qMRMLSceneModelPrivate(*this))
{
  Q_D(qMRMLSceneModel);
  d->init(/*new qMRMLSceneModelItemHelperFactory*/);
}

//------------------------------------------------------------------------------
qMRMLSceneModel::qMRMLSceneModel(qMRMLSceneModelPrivate* pimpl, QObject *parentObject)
  :QStandardItemModel(parentObject)
  , d_ptr(pimpl)
{
  Q_D(qMRMLSceneModel);
  d->init(/*factory*/);
}

//------------------------------------------------------------------------------
qMRMLSceneModel::~qMRMLSceneModel() = default;

//------------------------------------------------------------------------------
void qMRMLSceneModel::setPreItems(const QStringList& extraItems, QStandardItem* parent)
{
  Q_D(qMRMLSceneModel);

  if (parent == nullptr)
    {
    return;
    }

  d->removeAllExtraItems(parent, "preItem");

  int row = 0;
  foreach(QString extraItem, extraItems)
    {
    d->insertExtraItem(row++, parent, extraItem, "preItem", Qt::ItemIsEnabled  | Qt::ItemIsSelectable);
    }
}

//------------------------------------------------------------------------------
QStringList qMRMLSceneModel::preItems(QStandardItem* parent)const
{
  Q_D(const qMRMLSceneModel);
  return d->extraItems(parent, "preItem");
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setPostItems(const QStringList& extraItems, QStandardItem* parent)
{
  Q_D(qMRMLSceneModel);

  if (parent == nullptr)
    {
    return;
    }

  d->removeAllExtraItems(parent, "postItem");
  foreach(QString extraItem, extraItems)
    {
    d->insertExtraItem(parent->rowCount(), parent, extraItem, "postItem", Qt::ItemIsEnabled);
    }
}

//------------------------------------------------------------------------------
QStringList qMRMLSceneModel::postItems(QStandardItem* parent)const
{
  Q_D(const qMRMLSceneModel);
  return d->extraItems(parent, "postItem");
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneModel);
  /// it could go wrong if you try to set the same scene (specially because
  /// while updating the scene your signals/slots might call setMRMLScene again
  if (scene == d->MRMLScene)
    {
    return;
    }

  if (d->MRMLScene)
    {
    d->MRMLScene->RemoveObserver(d->CallBack);
    }
  d->MRMLScene = scene;
  this->updateScene();
  if (scene)
    {
    scene->AddObserver(vtkMRMLScene::NodeAboutToBeAddedEvent, d->CallBack, -10.);
    scene->AddObserver(vtkMRMLScene::NodeAddedEvent, d->CallBack, 10.);
    scene->AddObserver(vtkMRMLScene::NodeAboutToBeRemovedEvent, d->CallBack, -10.);
    scene->AddObserver(vtkMRMLScene::NodeRemovedEvent, d->CallBack, 10.);
    scene->AddObserver(vtkCommand::DeleteEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::StartCloseEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::EndCloseEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::StartImportEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::EndImportEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::StartBatchProcessEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::EndBatchProcessEvent, d->CallBack);
    }
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLSceneModel::mrmlScene()const
{
  Q_D(const qMRMLSceneModel);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneModel::mrmlSceneItem()const
{
  Q_D(const qMRMLSceneModel);
  if (d->MRMLScene == nullptr || this->maxColumnId() == -1)
    {
    return nullptr;
    }
  int count = this->invisibleRootItem()->rowCount();
  for (int i = 0; i < count; ++i)
    {
    QStandardItem* child = this->invisibleRootItem()->child(i);
    if (!child)
      {
      continue;
      }
    QVariant uid = child->data(qMRMLSceneModel::UIDRole);
    if (uid.type() == QVariant::String &&
        uid.toString() == "scene")
      {
      return child;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel::mrmlSceneIndex()const
{
  QStandardItem* scene = this->mrmlSceneItem();
  if (scene == nullptr)
    {
    return QModelIndex();
    }
  return scene ? scene->index() : QModelIndex();
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneModel::mrmlNodeFromItem(QStandardItem* nodeItem)const
{
  Q_D(const qMRMLSceneModel);
  // TODO: fasten by saving the pointer into the data
  if (d->MRMLScene == nullptr || nodeItem == nullptr)
    {
    return nullptr;
    }
  QVariant nodePointer = nodeItem->data(qMRMLSceneModel::PointerRole);
  if (!nodePointer.isValid() || nodeItem->data(qMRMLSceneModel::UIDRole).toString() == "scene")
    {
    return nullptr;
    }
  return nodeItem ? d->MRMLScene->GetNodeByID(
    nodeItem->data(qMRMLSceneModel::UIDRole).toString().toUtf8()) : nullptr;
}
//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneModel::itemFromNode(vtkMRMLNode* node, int column)const
{
  QModelIndex nodeIndex = this->indexFromNode(node, column);
  QStandardItem* nodeItem = this->itemFromIndex(nodeIndex);
  return nodeItem;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel::indexFromNode(vtkMRMLNode* node, int column)const
{
  Q_D(const qMRMLSceneModel);

  if (node == nullptr || node->GetID() == nullptr )
    {
    return QModelIndex();
    }

  QModelIndex nodeIndex;

  // Try to find the nodeIndex in the cache first
  QMap<vtkMRMLNode*,QPersistentModelIndex>::iterator rowCacheIt=d->RowCache.find(node);
  if (rowCacheIt==d->RowCache.end())
    {
    // not found in cache, therefore it cannot be in the model
    return nodeIndex;
    }
  if (rowCacheIt.value().isValid())
    {
    // An entry found in the cache. If the item at the cached index matches the requested node ID
    // then we use it.
    QStandardItem* nodeItem = this->itemFromIndex(rowCacheIt.value());
    if (nodeItem!=nullptr)
      {
      if (nodeItem->data(qMRMLSceneModel::UIDRole).toString().compare(QString::fromUtf8(node->GetID()))==0)
        {
        // id matched
        nodeIndex=rowCacheIt.value();
        }
      }
    }

  // The cache was not up-to-date. Do a slow linear search.
  if (!nodeIndex.isValid())
    {
    // QAbstractItemModel::match doesn't browse through columns
    // we need to do it manually
    QModelIndexList nodeIndexes = this->match(
      this->mrmlSceneIndex(), qMRMLSceneModel::UIDRole, QString(node->GetID()),
      1, Qt::MatchExactly | Qt::MatchRecursive);
    Q_ASSERT(nodeIndexes.size() <= 1); // we know for sure it won't be more than 1
    if (nodeIndexes.size() == 0)
      {
      // maybe the node hasn't been added to the scene yet...
      // (if it's called from populateScene/inserteNode)
      d->RowCache.remove(node);
      return QModelIndex();
      }
    nodeIndex=nodeIndexes[0];
    d->RowCache[node]=nodeIndex;
    }
  if (column == 0)
    {
    // QAbstractItemModel::match only search through the first column
    // (because scene is in the first column)
    Q_ASSERT(nodeIndex.isValid());
    return nodeIndex;
    }
  // Add the QModelIndexes from the other columns
  const int row = nodeIndex.row();
  QModelIndex nodeParentIndex = nodeIndex.parent();
  Q_ASSERT( column < this->columnCount(nodeParentIndex) );
  return nodeParentIndex.child(row, column);
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLSceneModel::indexes(vtkMRMLNode* node)const
{
  Q_D(const qMRMLSceneModel);
  return d->indexes(QString(node->GetID()));
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneModel::parentNode(vtkMRMLNode* node)const
{
  Q_UNUSED(node);
  return nullptr;
}

//------------------------------------------------------------------------------
int qMRMLSceneModel::nodeIndex(vtkMRMLNode* node)const
{
  Q_D(const qMRMLSceneModel);
  if (!d->MRMLScene)
    {
    return -1;
    }
  const char* nodeId = node ? node->GetID() : nullptr;
  if (nodeId == nullptr)
    {
    return -1;
    }

  const char* nId = nullptr;
  int index = -1;
  vtkMRMLNode* parent = this->parentNode(node);

  // Iterate through the scene and see if there is any matching node.
  // First try to find based on ptr value, as it's much faster than comparing string IDs.
  vtkCollection* nodes = d->MRMLScene->GetNodes();
  vtkMRMLNode* n = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (n = (vtkMRMLNode*)(nodes->GetNextItemAsObject(it))) ;)
    {
    // note: parent can be nullptr, it means that the scene is the parent
    if (parent == this->parentNode(n))
      {
      ++index;
      if (node==n)
        {
        // found the node
        return index;
        }
      }
    }

  // Not found by node ptr, try to find it by ID (much slower)
  for (nodes->InitTraversal(it);
       (n = (vtkMRMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be nullptr, it means that the scene is the parent
    if (parent == this->parentNode(n))
      {
      ++index;
      nId = n->GetID();
      if (nId && !strcmp(nodeId, nId))
        {
        return index;
        }
      }
    }

  // Not found
  return -1;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::canBeAChild(vtkMRMLNode* node)const
{
  Q_UNUSED(node);
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::canBeAParent(vtkMRMLNode* node)const
{
  Q_UNUSED(node);
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::reparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
  Q_UNUSED(node);
  Q_UNUSED(newParent);
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::isParentNode(vtkMRMLNode* child, vtkMRMLNode* parent)const
{
  for (; child; child = this->parentNode(child))
    {
    if (child == parent)
      {
      return true;
      }
    }
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel
::isAffiliatedNode(vtkMRMLNode* nodeA, vtkMRMLNode* nodeB)const
{
  return this->isParentNode(nodeA, nodeB) || this->isParentNode(nodeB, nodeA);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setListenNodeModifiedEvent(qMRMLSceneModel::NodeTypes listen)
{
  Q_D(qMRMLSceneModel);
  if (d->ListenNodeModifiedEvent == listen)
    {
    return;
    }
  d->ListenNodeModifiedEvent = listen;
  d->listenNodeModifiedEvent();
}

//------------------------------------------------------------------------------
qMRMLSceneModel::NodeTypes qMRMLSceneModel::listenNodeModifiedEvent()const
{
  Q_D(const qMRMLSceneModel);
  return d->ListenNodeModifiedEvent;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setLazyUpdate(bool lazy)
{
  Q_D(qMRMLSceneModel);
  if (d->LazyUpdate == lazy)
    {
    return;
    }
  d->LazyUpdate = lazy;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::lazyUpdate()const
{
  Q_D(const qMRMLSceneModel);
  return d->LazyUpdate;
}

//------------------------------------------------------------------------------
QMimeData* qMRMLSceneModel::mimeData(const QModelIndexList& indexes)const
{
  Q_D(const qMRMLSceneModel);
  if (!indexes.size())
    {
    return nullptr;
    }
  QModelIndexList allColumnsIndexes;
  foreach(const QModelIndex& index, indexes)
    {
    QModelIndex parent = index.parent();
    for (int column = 0; column < this->columnCount(parent); ++column)
      {
      allColumnsIndexes << this->index(index.row(), column, parent);
      }
    d->DraggedNodes << this->mrmlNodeFromIndex(index);
    }
  // Remove duplicates
  allColumnsIndexes = allColumnsIndexes.toSet().toList();
  return this->QStandardItemModel::mimeData(allColumnsIndexes);
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                   int row, int column, const QModelIndex &parent)
{
  Q_UNUSED(column);
  // We want to do drag&drop only into the first item of a line (and not on a
  // random column.
  bool res = this->Superclass::dropMimeData(
    data, action, row, 0, parent.sibling(parent.row(), 0));
  // Do not clear d->DraggedNodes yet, as node modification events may come
  // in before delayedItemChanged() is executed.
  return res;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateScene()
{
  Q_D(qMRMLSceneModel);

  // Stop listening to all the nodes before we remove them (setRowCount) as some
  // weird behavior could arise when removing the nodes (e.g onMRMLNodeModified
  // could be called ...)
  qvtkDisconnect(nullptr, vtkCommand::ModifiedEvent,
                 this, SLOT(onMRMLNodeModified(vtkObject*)));
  qvtkDisconnect(nullptr, vtkMRMLNode::IDChangedEvent,
                 this, SLOT(onMRMLNodeIDChanged(vtkObject*,void*)));

  d->RowCache.clear();

  // Enabled so it can be interacted with
  this->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);

  // Extra items before the scene item. Typically there is no top-level extra
  // items, only at the Scene level (before and after the nodes)
  const int preSceneItemCount = this->preItems(nullptr).count();
  const int postSceneItemCount = this->postItems(nullptr).count();

  if (!this->mrmlSceneItem() && d->MRMLScene)
    {
    // No scene item has been created yet, but the MRMLScene is valid so we
    // need to create one.
    QList<QStandardItem*> sceneItems;
    QStandardItem* sceneItem = new QStandardItem;
    sceneItem->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
    sceneItem->setText("Scene");
    sceneItem->setData("scene", qMRMLSceneModel::UIDRole);
    sceneItems << sceneItem;
    for (int i = 1; i < this->columnCount(); ++i)
      {
      QStandardItem* sceneOtherColumn = new QStandardItem;
      sceneOtherColumn->setFlags(nullptr);
      sceneItems << sceneOtherColumn;
      }
    // We need to set the column count in case there extra items,
    // they need to know how many columns the scene item has.
    sceneItem->setColumnCount(this->columnCount());
    this->insertRow(preSceneItemCount, sceneItems);
    }
  else if (!d->MRMLScene)
    {
    // TBD: Because we don't call clear, I don't think restoring the column
    // count is necessary because it shouldn't be changed.
    const int oldColumnCount = this->columnCount();
    this->removeRows(
      preSceneItemCount,
      this->rowCount() - preSceneItemCount - postSceneItemCount);
    this->setColumnCount(oldColumnCount);
    return;
    }

  // if there is no column, there is no scene item.
  if (!this->mrmlSceneItem())
    {
    return;
    }

  // Update the scene pointer in case d->MRMLScene has changed
  this->mrmlSceneItem()->setData(
    QVariant::fromValue(reinterpret_cast<long long>(d->MRMLScene)),
    qMRMLSceneModel::PointerRole);

  const int preNodesItemCount = this->preItems(this->mrmlSceneItem()).count();
  const int postNodesItemCount = this->postItems(this->mrmlSceneItem()).count();
  // Just remove the nodes, not the extra items like "None", "Create node" etc.
  this->mrmlSceneItem()->removeRows(
    preNodesItemCount,
    this->mrmlSceneItem()->rowCount() - preNodesItemCount - postNodesItemCount);

  // Populate scene with nodes
  this->populateScene();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::populateScene()
{
  Q_D(qMRMLSceneModel);
  // Add nodes
  int index = -1;
  vtkMRMLNode *node = nullptr;
  vtkCollectionSimpleIterator it;
  d->MisplacedNodes.clear();
  if (!d->MRMLScene)
    {
    return;
    }
  for (d->MRMLScene->GetNodes()->InitTraversal(it);
       (node = (vtkMRMLNode*)d->MRMLScene->GetNodes()->GetNextItemAsObject(it)) ;)
    {
    index++;
    d->insertNode(node, index);
    }
  foreach(vtkMRMLNode* misplacedNode, d->MisplacedNodes)
    {
    this->onMRMLNodeModified(misplacedNode);
    }
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneModel::insertNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSceneModel);
  return d->insertNode(node, this->nodeIndex(node));
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneModelPrivate::insertNode(vtkMRMLNode* node, int nodeIndex)
{
  Q_Q(qMRMLSceneModel);
  QStandardItem* nodeItem = q->itemFromNode(node);
  if (nodeItem != nullptr)
    {
    // It is possible that the node has been already added if it is the parent
    // of a child node already inserted.
    return nodeItem;
    }
  vtkMRMLNode* parentNode = q->parentNode(node);
  QStandardItem* parentItem =
    parentNode ? q->itemFromNode(parentNode) : q->mrmlSceneItem();
  if (!parentItem)
    {
    Q_ASSERT(parentNode);
    parentItem = q->insertNode(parentNode);
    Q_ASSERT(parentItem);
    }
  int min = q->preItems(parentItem).count();
  int max = parentItem->rowCount() - q->postItems(parentItem).count();
  int row = min + nodeIndex;
  if (row > max)
    {
    this->MisplacedNodes << node;
    row = max;
    }
  nodeItem = q->insertNode(node, parentItem, row);
  Q_ASSERT(q->itemFromNode(node) == nodeItem);
  return nodeItem;
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneModel::insertNode(vtkMRMLNode* node, QStandardItem* parent, int row)
{
  Q_D(qMRMLSceneModel);
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node));

  QList<QStandardItem*> items;
  for (int i= 0; i < this->columnCount(); ++i)
    {
    QStandardItem* newNodeItem = new QStandardItem();
    this->updateItemFromNode(newNodeItem, node, i);
    items.append(newNodeItem);
    }

  // Insert an invalid item in the cache to indicate that the node is in the model
  // but we don't know its index yet. This is needed because a custom widget may be notified
  // abot row insertion before insertRow() returns (and the RowCache entry is added).
  // For example, qSlicerPresetComboBox::setIconToPreset() is called at the end of insertRow,
  // before the RowCache entry is added.
  d->RowCache[node]=QModelIndex();

  if (parent)
    {
    parent->insertRow(row, items);
    //Q_ASSERT(parent->columnCount() == 2);
    }
  else
    {
    this->insertRow(row,items);
    }
  d->RowCache[node]=items[0]->index();
  // TODO: don't listen to nodes that are hidden from editors ?
  if (d->ListenNodeModifiedEvent == AllNodes)
    {
    this->observeNode(node);
    }
  return items[0];
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::observeNode(vtkMRMLNode* node)
{
  qvtkConnect(node, vtkCommand::ModifiedEvent,
              this, SLOT(onMRMLNodeModified(vtkObject*)));
  qvtkConnect(node, vtkMRMLNode::IDChangedEvent,
              this, SLOT(onMRMLNodeIDChanged(vtkObject*,void*)));
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_D(qMRMLSceneModel);
  // We are going to make potentially multiple changes to the item. We want to
  // refresh the node only once, so we "block" the updates in onItemChanged().
  d->PendingItemModified = 0;
  item->setFlags(this->nodeFlags(node, column));
  // set UIDRole and set PointerRole need to be atomic
  bool blocked  = this->blockSignals(true);
  item->setData(QString(node->GetID()), qMRMLSceneModel::UIDRole);
  item->setData(QVariant::fromValue(reinterpret_cast<long long>(node)), qMRMLSceneModel::PointerRole);
  this->blockSignals(blocked);
  this->updateItemDataFromNode(item, node, column);

  bool itemChanged = (d->PendingItemModified > 0);
  d->PendingItemModified = -1;

  // Update parent, but only if the item is not being drag-and-dropped
  // (drag-and-drop is performed using delayed update, therefore
  // any node modifications, even those unrelated to changing the parent
  // would override drag-and-drop result).
  if (this->canBeAChild(node) && !d->DraggedNodes.contains(node))
    {
    QStandardItem* parentItem = item->parent();
    QStandardItem* newParentItem = this->itemFromNode(this->parentNode(node));
    if (newParentItem == nullptr)
      {
      newParentItem = this->mrmlSceneItem();
      }
    // If the item has no parent, then it means it hasn't been put into the scene yet.
    // and it will do it automatically.
    if (parentItem && parentItem != newParentItem)
      {
      int newIndex = this->nodeIndex(node);
      if (parentItem != newParentItem ||
          newIndex != item->row())
        {
        QList<QStandardItem*> children = parentItem->takeRow(item->row());
        d->reparentItems(children, newIndex, newParentItem);
        }
      }
    }
  if (itemChanged)
    {
    this->onItemChanged(item);
    }
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  QFlags<Qt::ItemFlag> flags = Qt::ItemIsEnabled
                             | Qt::ItemIsSelectable;
  if (column == this->checkableColumn() && node->GetSelectable())
    {
    flags = flags | Qt::ItemIsUserCheckable;
    }
  if (column == this->nameColumn())
    {
    flags = flags | Qt::ItemIsEditable;
    }
  if (this->canBeAChild(node))
    {
    flags = flags | Qt::ItemIsDragEnabled;
    }
  if (this->canBeAParent(node))
    {
    flags = flags | Qt::ItemIsDropEnabled;
    }

  return flags;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateItemDataFromNode(
  QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_D(qMRMLSceneModel);
  if (column == this->nameColumn())
    {
    item->setText(QString(node->GetName()));
    item->setToolTip(node->GetNodeTagName());
    }
  if (column == this->toolTipNameColumn())
    {
    item->setToolTip(QString(node->GetName()));
    }
  if (column == this->idColumn())
    {
    item->setText(QString(node->GetID()));
    }
  if (column == this->checkableColumn())
    {
    item->setCheckState(node->GetSelected() ? Qt::Checked : Qt::Unchecked);
    }
  if (column == this->visibilityColumn())
    {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(node);
    vtkMRMLDisplayableNode* displayableNode =
      vtkMRMLDisplayableNode::SafeDownCast(node);
    vtkMRMLDisplayableHierarchyNode* displayableHierarchyNode =
      vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
    if (displayableHierarchyNode)
      {
      displayNode = displayableHierarchyNode->GetDisplayNode();
      }
    int visible = -1;
    if (displayNode)
      {
      visible = displayNode->GetVisibility();
      }
    else if (displayableNode)
      {
      visible = displayableNode->GetDisplayVisibility();
      }
    // It should be fine to set the icon even if it is the same, but due
    // to a bug in Qt (http://bugreports.qt.nokia.com/browse/QTBUG-20248),
    // it would fire a superfluous itemChanged() signal.
    if (item->data(VisibilityRole).isNull() ||
        item->data(VisibilityRole).toInt() != visible)
      {
      item->setData(visible, VisibilityRole);
      switch (visible)
        {
        case 0:
          item->setIcon(d->HiddenIcon);
          break;
        case 1:
          item->setIcon(d->VisibleIcon);
          break;
        case 2:
          item->setIcon(d->PartiallyVisibleIcon);
          break;
        default:
          // can get here if not a display or displayable node
          //qWarning() << "Unsupported visibility value: " << visible;
          break;
        }
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item)
{
  int wasModifying = node->StartModify();
  this->updateNodeFromItemData(node, item);
  node->EndModify(wasModifying);

  // the following only applies to tree hierarchies
  if (!this->canBeAChild(node))
    {
    return;
    }

 Q_ASSERT(node != this->mrmlNodeFromItem(item->parent()));

  QStandardItem* parentItem = item->parent();
  int columnCount = parentItem ? parentItem->columnCount() : 0;

  // Don't do the following if the row is not complete (reparenting an
  // incomplete row might lead to errors). (if there is no child yet for a given
  // column, it will get there next time updateNodeFromItem is called).
  // updateNodeFromItem() is called for every item drag&dropped (we ensure that
  // all the indexes of the row are reparented when entering the d&d function
  for (int i = 0; i < columnCount; ++i)
    {
    if (parentItem->child(item->row(), i) == nullptr)
      {
      return;
      }
    }

  vtkMRMLNode* parent = this->mrmlNodeFromItem(parentItem);
  int desiredNodeIndex = -1;
  if (this->parentNode(node) != parent)
    {
    emit aboutToReparentByDragAndDrop(node, parent);
    if (this->reparent(node, parent))
      {
      emit reparentedByDragAndDrop(node, parent);
      }
    else
      {
      this->updateItemFromNode(item, node, item->column());
      }
    }
  else if ((desiredNodeIndex = this->nodeIndex(node)) != item->row())
    {
    QStandardItem* parentItem = item->parent();
    if (parentItem && desiredNodeIndex <
          (parentItem->rowCount() - this->postItems(parentItem).count()))
      {
      this->updateItemFromNode(item, node, item->column());
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
  if (item->column() == this->nameColumn())
    {
    node->SetName(item->text().toUtf8());
    }
  // ToolTip can't be edited, don't change the node
  // if (item->column() == this->toolTipNameColumn())
  // {
  // }
  if (item->column() == this->idColumn())
    {
    // Too dangerous
    //node->SetName(item->text().toUtf8());
    }
  if (item->column() == this->checkableColumn())
    {
    node->SetSelected(item->checkState() == Qt::Checked ? 1 : 0);
    }
  if (item->column() == this->visibilityColumn())
    {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(node);
    vtkMRMLDisplayableNode* displayableNode =
      vtkMRMLDisplayableNode::SafeDownCast(node);
    vtkMRMLDisplayableHierarchyNode* displayableHierarchyNode =
      vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
    if (displayableHierarchyNode)
      {
      displayNode = displayableHierarchyNode->GetDisplayNode();
      }
    Q_ASSERT(!item->data(VisibilityRole).isNull());
    int visible = item->data(VisibilityRole).toInt();
    if (displayNode)
      {
      displayNode->SetVisibility(visible);
      }
    else if (displayableNode)
      {
      displayableNode->SetDisplayVisibility(visible);
      }
    }
}

//-----------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneEvent(vtkObject* vtk_obj, unsigned long event,
                                        void* client_data, void* call_data)
{
  vtkMRMLScene* scene = reinterpret_cast<vtkMRMLScene*>(vtk_obj);
  qMRMLSceneModel* sceneModel = reinterpret_cast<qMRMLSceneModel*>(client_data);
  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(call_data);
  Q_ASSERT(scene);
  Q_ASSERT(sceneModel);
  switch(event)
    {
    case vtkMRMLScene::NodeAboutToBeAddedEvent:
      Q_ASSERT(node);
      sceneModel->onMRMLSceneNodeAboutToBeAdded(scene, node);
      break;
    case vtkMRMLScene::NodeAddedEvent:
      Q_ASSERT(node);
      sceneModel->onMRMLSceneNodeAdded(scene, node);
      break;
    case vtkMRMLScene::NodeAboutToBeRemovedEvent:
      Q_ASSERT(node);
      sceneModel->onMRMLSceneNodeAboutToBeRemoved(scene, node);
      break;
    case vtkMRMLScene::NodeRemovedEvent:
      Q_ASSERT(node);
      sceneModel->onMRMLSceneNodeRemoved(scene, node);
      break;
    case vtkCommand::DeleteEvent:
      sceneModel->onMRMLSceneDeleted(scene);
      break;
    case vtkMRMLScene::StartCloseEvent:
      sceneModel->onMRMLSceneAboutToBeClosed(scene);
      break;
    case vtkMRMLScene::EndCloseEvent:
      sceneModel->onMRMLSceneClosed(scene);
      break;
    case vtkMRMLScene::StartImportEvent:
      sceneModel->onMRMLSceneAboutToBeImported(scene);
      break;
    case vtkMRMLScene::EndImportEvent:
      sceneModel->onMRMLSceneImported(scene);
      break;
    case vtkMRMLScene::StartBatchProcessEvent:
      sceneModel->onMRMLSceneStartBatchProcess(scene);
      break;
    case vtkMRMLScene::EndBatchProcessEvent:
      sceneModel->onMRMLSceneEndBatchProcess(scene);
      break;
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAboutToBeAdded(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
#ifndef QT_NO_DEBUG
  Q_D(qMRMLSceneModel);
  Q_ASSERT(scene != nullptr);
  Q_ASSERT(scene == d->MRMLScene);
#endif
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAdded(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_D(qMRMLSceneModel);
  Q_UNUSED(d);
  Q_UNUSED(scene);
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node));

  if (d->MRMLScene->IsImporting() || (d->LazyUpdate && d->MRMLScene->IsBatchProcessing()))
    {
    // Node IDs and references are not valid until the import is completed, therefore do not attempt
    // to add a node during importing (see https://issues.slicer.org/view.php?id=4080).
    return;
    }
  this->insertNode(node);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAboutToBeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_D(qMRMLSceneModel);
  Q_UNUSED(d);
  Q_UNUSED(scene);
  Q_ASSERT(scene == d->MRMLScene);

  if (d->MRMLScene->IsClosing() || (d->LazyUpdate && d->MRMLScene->IsBatchProcessing()))
    {
    return;
    }

  int connectionsRemoved =
    qvtkDisconnect(node, vtkCommand::ModifiedEvent,
                   this, SLOT(onMRMLNodeModified(vtkObject*)));

  Q_ASSERT_X(((d->ListenNodeModifiedEvent == NoNodes) && connectionsRemoved == 0) ||
             (d->ListenNodeModifiedEvent != NoNodes && connectionsRemoved <= 1),
             "qMRMLSceneModel::onMRMLSceneNodeAboutToBeRemoved()",
             "A node has been removed from the scene but the scene model has "
             "never been notified it has been added in the first place. Maybe"
             " vtkMRMLScene::AddNodeNoNotify() has been used instead of "
             "vtkMRMLScene::AddNode");
  Q_UNUSED(connectionsRemoved);
  // Remove all the observations on the node
  qvtkDisconnect(node, vtkCommand::NoEvent, this, nullptr);

  // TODO: can be fasten by browsing the tree only once
  QModelIndexList indexes = this->match(this->mrmlSceneIndex(), qMRMLSceneModel::UIDRole,
                                        QString(node->GetID()), 1,
                                        Qt::MatchExactly | Qt::MatchRecursive);
  if (indexes.count())
    {
    QStandardItem* item = this->itemFromIndex(indexes[0].sibling(indexes[0].row(),0));
    // The children may be lost if not reparented, we ensure they got reparented.
    while (item->rowCount())
      {
      // we need to remove the children from the node to remove because they
      // would be automatically deleted in QStandardItemModel::removeRow()
      d->Orphans.push_back(item->takeRow(0));
      }
    // Remove the item from any orphan list if it exist as we don't want to
    // add it back later in onMRMLSceneNodeRemoved
    foreach(QList<QStandardItem*> orphans, d->Orphans)
      {
      if (orphans.contains(item))
        {
        d->Orphans.removeAll(orphans);
        }
      }
    this->removeRow(indexes[0].row(), indexes[0].parent());
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_D(qMRMLSceneModel);
  Q_UNUSED(scene);
  Q_UNUSED(node);
  if (d->MRMLScene->IsClosing() || (d->LazyUpdate && d->MRMLScene->IsBatchProcessing()))
    {
    return;
    }
  // The removed node may had children, if they haven't been updated, they
  // are likely to be lost (not reachable when browsing the model), we need
  // to reparent them.
  foreach(QList<QStandardItem*> orphans, d->Orphans)
    {
    QStandardItem* orphan = orphans[0];
    // Make sure that the orphans have not already been reparented.
    if (orphan->parent())
      {
      // Not sure how it is possible, but if it is, then we might want to
      // review the logic behind.
      Q_ASSERT(orphan->parent() == nullptr);
      continue;
      }
    vtkMRMLNode* node = this->mrmlNodeFromItem(orphan);
    int newIndex = this->nodeIndex(node);
    QStandardItem* newParentItem = this->itemFromNode(this->parentNode(node));
    if (newParentItem == nullptr)
      {
      newParentItem = this->mrmlSceneItem();
      }
    Q_ASSERT(newParentItem);
    d->reparentItems(orphans, newIndex, newParentItem);
    }
  d->Orphans.clear();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneDeleted(vtkObject* scene)
{
  Q_UNUSED(scene);
#ifndef QT_NO_DEBUG
  Q_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
#endif
  this->setMRMLScene(nullptr);
}

//------------------------------------------------------------------------------
void printStandardItem(QStandardItem* item, const QString& offset)
{
  if (!item)
    {
    return;
    }
  qDebug() << offset << item << item->index() << item->text()
           << item->data(qMRMLSceneModel::UIDRole).toString() << item->row()
           << item->column() << item->rowCount() << item->columnCount();
  for(int i = 0; i < item->rowCount(); ++i )
    {
    for (int j = 0; j < item->columnCount(); ++j)
      {
      printStandardItem(item->child(i,j), offset + "   ");
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateNodeItems()
{
  QStandardItem* sceneItem = this->mrmlSceneItem();
  if (sceneItem == nullptr)
    {
    return;
    }
  sceneItem->setColumnCount(this->columnCount());
  vtkCollection* nodes = this->mrmlScene()->GetNodes();
  vtkMRMLNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    this->updateNodeItems(node, QString(node->GetID()));
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLNodeModified(vtkObject* node)
{
  vtkMRMLNode* modifiedNode = vtkMRMLNode::SafeDownCast(node);
  this->updateNodeItems(modifiedNode, QString(modifiedNode->GetID()));
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLNodeIDChanged(vtkObject* node, void* callData)
{
  char* oldID = reinterpret_cast<char *>(callData);
  this->updateNodeItems(vtkMRMLNode::SafeDownCast(node), QString(oldID));
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateNodeItems(vtkMRMLNode* node, const QString& nodeUID)
{
  Q_D(qMRMLSceneModel);

  if (d->MRMLScene->IsClosing() || d->MRMLScene->IsImporting() || (d->LazyUpdate && d->MRMLScene->IsBatchProcessing()))
    {
    return;
    }

  // If there is no node here or if the node has no scene. that means the node
  // has been removed from the scene but the scene model hasn't been notified
  // or that the scene model is still observing the node (in a subclass).
  // Another reason is that the scene has been closed, and when triggering the
  // SceneClosed event to all observers, one observer modifies the node and
  // the scene model has not yet been notified the scene was closed.
  Q_ASSERT(node);
  if (!node || !node->GetScene())
    {
    return;
    }
  //Q_ASSERT(node->GetScene()->IsNodePresent(node));
  QModelIndexList nodeIndexes = d->indexes(nodeUID);
  //qDebug() << "onMRMLNodeModified" << node->GetID() << nodeIndexes;
  Q_ASSERT(nodeIndexes.count());
  for (int i = 0; i < nodeIndexes.size(); ++i)
    {
    QModelIndex index = nodeIndexes[i];
    QStandardItem* item = this->itemFromIndex(index);
    int oldRow = item->row();
    QStandardItem* oldParent = item->parent();

    this->updateItemFromNode(item, node, item->column());
    // maybe the item has been reparented, then we need to rescan the
    // indexes again as they may be wrong.
    if (item->row() != oldRow || item->parent() != oldParent)
      {
      int oldSize = nodeIndexes.size();
      nodeIndexes = this->indexes(node);
      int newSize = nodeIndexes.size();
      // the number of columns shouldn't change
      Q_ASSERT(oldSize == newSize);
      Q_UNUSED(oldSize);
      Q_UNUSED(newSize);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onItemChanged(QStandardItem * item)
{
  Q_D(qMRMLSceneModel);

  if (d->PendingItemModified >= 0)
    {
    ++d->PendingItemModified;
    return;
    }
  // when a dnd occurs, the order of the items called with onItemChanged is
  // random, it could be the item in column 1 then the item in column 0
  //qDebug() << "onItemChanged: " << item << item->row() << item->column() << d->DraggedNodes.count();
  //printStandardItem(this->mrmlSceneItem(), "");
  //return;
  // check on the column is optional(no strong feeling), it is just there to be
  // faster though
  if (!this->isANode(item))
    {
    return;
    }

  if (d->DraggedNodes.count())
    {
    if (item->column() == 0)
      {
      //this->metaObject()->invokeMethod(
      //  this, "onItemChanged", Qt::QueuedConnection, Q_ARG(QStandardItem*, item));
      d->DraggedItem = item;
      QTimer::singleShot(200, this, SLOT(delayedItemChanged()));
      }
    return;
    }

  // Only nodes can be changed, scene and extra items should be not editable
  vtkMRMLNode* mrmlNode = this->mrmlNodeFromItem(item);
  Q_ASSERT(mrmlNode);
  if (mrmlNode==nullptr)
  {
    qCritical() << "qMRMLSceneModel::onItemChanged: Failed to get MRML node from scene model item";
    return;
  }
  this->updateNodeFromItem(mrmlNode, item);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::delayedItemChanged()
{
  Q_D(qMRMLSceneModel);
  // Clear d->DraggedNodes before calling onItemChanged
  // to make process item changes immediately (instead of
  // triggering another delayed update)
  d->DraggedNodes.clear();

  this->onItemChanged(d->DraggedItem);
  d->DraggedItem = nullptr;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::isANode(const QStandardItem * item)const
{
  Q_D(const qMRMLSceneModel);
  return item
    && item != this->mrmlSceneItem()
    && !d->isExtraItem(item);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneAboutToBeImported(vtkMRMLScene* scene)
{
  Q_UNUSED(scene);
  //this->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneImported(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneModel);
  Q_UNUSED(scene);
  // Node IDs and references are not valid until the import is completed,
  // therefore we must update the model now (see https://issues.slicer.org/view.php?id=4080).
  this->updateScene();
  //this->endResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneAboutToBeClosed(vtkMRMLScene* scene)
{
  Q_UNUSED(scene);
  //this->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneClosed(vtkMRMLScene* scene)
{
  Q_UNUSED(scene);
  this->updateScene();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneStartBatchProcess(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneModel);
  Q_UNUSED(scene);
  if (d->LazyUpdate)
    {
    emit sceneAboutToBeUpdated();
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneEndBatchProcess(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneModel);
  Q_UNUSED(scene);
  if (d->LazyUpdate)
    {
    this->updateScene();
    emit sceneUpdated();
    }
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneModel::supportedDropActions()const
{
  return Qt::IgnoreAction;
}

//------------------------------------------------------------------------------
int qMRMLSceneModel::nameColumn()const
{
  Q_D(const qMRMLSceneModel);
  return d->NameColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setNameColumn(int column)
{
  Q_D(qMRMLSceneModel);
  d->NameColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneModel::idColumn()const
{
  Q_D(const qMRMLSceneModel);
  return d->IDColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setIDColumn(int column)
{
  Q_D(qMRMLSceneModel);
  d->IDColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneModel::checkableColumn()const
{
  Q_D(const qMRMLSceneModel);
  return d->CheckableColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setCheckableColumn(int column)
{
  Q_D(qMRMLSceneModel);
  d->CheckableColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneModel::visibilityColumn()const
{
  Q_D(const qMRMLSceneModel);
  return d->VisibilityColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setVisibilityColumn(int column)
{
  Q_D(qMRMLSceneModel);
  d->VisibilityColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneModel::toolTipNameColumn()const
{
  Q_D(const qMRMLSceneModel);
  return d->ToolTipNameColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setToolTipNameColumn(int column)
{
  Q_D(qMRMLSceneModel);
  d->ToolTipNameColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneModel::extraItemColumn()const
{
  Q_D(const qMRMLSceneModel);
  return d->ExtraItemColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setExtraItemColumn(int column)
{
  Q_D(qMRMLSceneModel);
  d->ExtraItemColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateColumnCount()
{
  int max = this->maxColumnId();
  int oldColumnCount = this->columnCount();
  this->setColumnCount(max + 1);
  if (oldColumnCount == 0)
    {
    this->updateScene();
    }
  else
    {
    this->updateNodeItems();
    }
}

//------------------------------------------------------------------------------
int qMRMLSceneModel::maxColumnId()const
{
  Q_D(const qMRMLSceneModel);
  int maxId = 0; // information (scene, node uid... ) is stored in the 1st column
  maxId = qMax(maxId, d->NameColumn);
  maxId = qMax(maxId, d->IDColumn);
  maxId = qMax(maxId, d->CheckableColumn);
  maxId = qMax(maxId, d->VisibilityColumn);
  maxId = qMax(maxId, d->ToolTipNameColumn);
  maxId = qMax(maxId, d->ExtraItemColumn);
  return maxId;
}
