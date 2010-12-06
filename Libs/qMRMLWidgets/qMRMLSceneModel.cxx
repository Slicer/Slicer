/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QAction>
#include <QDebug>
#include <QMimeData>
#include <QStringList>
#include <QSharedPointer>
#include <QVector>
#include <QMap>

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSceneModel_p.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>
#include <vtkVariantArray.h>

// STD includes
#include <iostream>
#include <fstream>
/*
//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLFlatSceneItemHelper : public qMRMLAbstractSceneItemHelper
{
public:
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent()const;
  
protected:
  friend class qMRMLSceneModelItemHelperFactory;
  qMRMLFlatSceneItemHelper(vtkMRMLScene* scene, int column, const qMRMLAbstractItemHelperFactory* factory, int _row);
  /// here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLFlatNodeItemHelper : public qMRMLAbstractNodeItemHelper
{
public:
  virtual qMRMLAbstractItemHelper* parent() const;
protected:
  friend class qMRMLSceneModelItemHelperFactory;
  qMRMLFlatNodeItemHelper(vtkMRMLNode* node, int column, const qMRMLAbstractItemHelperFactory* factory, int row);
};

//------------------------------------------------------------------------------
class qMRMLSceneModelItemHelperFactoryPrivate
{
public:
  vtkSmartPointer<vtkCollection> PreItems;
  vtkSmartPointer<vtkCollection> PostItems;
};

//------------------------------------------------------------------------------
qMRMLSceneModelItemHelperFactory::qMRMLSceneModelItemHelperFactory()
  : d_ptr(new qMRMLSceneModelItemHelperFactoryPrivate)
{
}

//------------------------------------------------------------------------------
qMRMLSceneModelItemHelperFactory::~qMRMLSceneModelItemHelperFactory()
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModelItemHelperFactory::createItem(vtkObject* object, int column, int row)const
{
  if (!object)
    {
    Q_ASSERT(object);
    return 0;
    }
  if (object->IsA("vtkMRMLScene"))
    {
    qMRMLAbstractItemHelper* source = new qMRMLFlatSceneItemHelper(vtkMRMLScene::SafeDownCast(object), column, this, 0);
    return new qMRMLExtraItemsHelper(this->preItems(), this->postItems(), source);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLFlatNodeItemHelper(vtkMRMLNode::SafeDownCast(object), column, this, row);
    }
  else if (object->IsA("vtkVariantArray"))
    {
    return new qMRMLVariantArrayItemHelper(vtkVariantArray::SafeDownCast(object), column, this, row);
    }
  else
    {
    qDebug() << object->GetClassName();
    Q_ASSERT(false);
    }
  return 0;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelItemHelperFactory::setPostItems(vtkCollection* itemCollection)
{
  Q_D(qMRMLSceneModelItemHelperFactory);
  d->PostItems = itemCollection;
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLSceneModelItemHelperFactory::postItems()const
{
  Q_D(const qMRMLSceneModelItemHelperFactory);
  return d->PostItems.GetPointer();
}

//------------------------------------------------------------------------------
void qMRMLSceneModelItemHelperFactory::setPreItems(vtkCollection* itemCollection)
{
  Q_D(qMRMLSceneModelItemHelperFactory);
  d->PreItems = itemCollection;
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLSceneModelItemHelperFactory::preItems()const
{
  Q_D(const qMRMLSceneModelItemHelperFactory);
  return d->PreItems.GetPointer();
}

// qMRMLFlatSceneItemHelper

//------------------------------------------------------------------------------
qMRMLFlatSceneItemHelper::qMRMLFlatSceneItemHelper(vtkMRMLScene* scene, int _column, 
                                                   const qMRMLAbstractItemHelperFactory* itemFactory, int _row)
  :qMRMLAbstractSceneItemHelper(scene, _column, itemFactory, _row)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLFlatSceneItemHelper::child(int _row, int _column) const
{
  vtkMRMLNode* childNode = this->mrmlScene()->GetNthNode(_row);
  return this->factory()->createItem(childNode, _column, _row);
  //if (childNode == 0)
  //  {
  //  return 0;
  //  }
  //qMRMLAbstractNodeItemHelper* _child = 
  //  new qMRMLFlatNodeItemHelper(childNode, _column);
  //return _child;
  //return childNode;
}

//------------------------------------------------------------------------------
int qMRMLFlatSceneItemHelper::childCount() const
{ 
  if (this->column() != 0)
    {
    return 0;
    }
  return this->mrmlScene() ? this->mrmlScene()->GetNumberOfNodes() : 0;; 
}

//------------------------------------------------------------------------------
int qMRMLFlatSceneItemHelper::childIndex(const qMRMLAbstractItemHelper* _child) const
{
  const qMRMLAbstractNodeItemHelper* nodeItemHelper = 
    dynamic_cast<const qMRMLAbstractNodeItemHelper*>(_child);
  //Q_ASSERT(nodeItemHelper);
  if (nodeItemHelper == 0)
    {
    return -1;
    }
  // Check what kind of value IsNodePresent(0) returns. If a node is not found: 0 or -1?
  // if it's 0, then we should decrease the index as we work in 0-based arrays.
  Q_ASSERT(!this->mrmlScene() || this->mrmlScene()->IsNodePresent(0) == 0);
  return this->mrmlScene() ? this->mrmlScene()->IsNodePresent(nodeItemHelper->mrmlNode()) - 1 : -1;
}

//------------------------------------------------------------------------------
bool qMRMLFlatSceneItemHelper::hasChildren() const
{
  if (this->column() != 0)
    {
    return 0;
    }
  return this->mrmlScene() ? this->mrmlScene()->GetNumberOfNodes() > 0 : false;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLFlatSceneItemHelper::parent() const
{
  return this->factory()->createRootItem(this->mrmlScene());
}

//------------------------------------------------------------------------------
// qMRMLFlatNodeItemHelper

//------------------------------------------------------------------------------
qMRMLFlatNodeItemHelper::qMRMLFlatNodeItemHelper(vtkMRMLNode* node, int _column, 
                                                 const qMRMLAbstractItemHelperFactory* itemFactory, int _row)
  :qMRMLAbstractNodeItemHelper(node, _column, itemFactory, _row)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLFlatNodeItemHelper::parent() const
{
  //return new qMRMLFlatSceneItemHelper(this->mrmlNode()->GetScene(), 0);
  //return this->mrmlNode()->GetScene();
  return this->factory()->createItem(this->mrmlNode()->GetScene(), 0);
}
*/

//------------------------------------------------------------------------------
qMRMLSceneModelPrivate::qMRMLSceneModelPrivate(qMRMLSceneModel& object)
  : q_ptr(&object)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->ListenNodeModifiedEvent = false;
  //this->ItemFactory = 0;
  this->MRMLScene = 0;
  //this->MRMLNodeToBe = 0;
  //this->MRMLNodeToBeAdded = 0;
}

//------------------------------------------------------------------------------
qMRMLSceneModelPrivate::~qMRMLSceneModelPrivate()
{
  if (this->MRMLScene)
    {
    this->MRMLScene->RemoveObserver(this->CallBack);
    }
  //delete this->ItemFactory;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelPrivate::init(/*qMRMLSceneModelItemHelperFactory* factory*/)
{
  Q_Q(qMRMLSceneModel);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qMRMLSceneModel::onMRMLSceneEvent);
  q->setColumnCount(2);
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)));
  //this->ItemFactory = factory;
}
/*
//------------------------------------------------------------------------------
vtkObject* qMRMLSceneModelPrivate::object(const QModelIndex &index)const
{
  return reinterpret_cast<vtkObject*>(index.internalPointer());
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModelPrivate::itemFromIndex(const QModelIndex &index)const
{
  Q_Q(const qMRMLSceneModel);
  if ((index.row() < 0) || (index.column() < 0) || (index.model() != q))
    {
    //return new qMRMLFlatRootItemHelper(this->MRMLScene);
    return this->ItemFactory->createRootItem(this->MRMLScene);
    }
  //return this->itemFromObject(reinterpret_cast<vtkObject*>(index.internalPointer()), index.column());
  return this->ItemFactory->createItem(this->object(index), index.column(), index.row());
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModelPrivate::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  Q_Q(const qMRMLSceneModel);
  if (item == 0 || item->object() == 0)
    {
    return QModelIndex();
    }
  if (dynamic_cast<const qMRMLRootItemHelper*>(item) != 0)
    {
    return QModelIndex();
    }
  return q->createIndex(item->row(), item->column(), 
                        reinterpret_cast<void*>(item->object()));
}
*/
//------------------------------------------------------------------------------
void qMRMLSceneModelPrivate::listenNodeModifiedEvent()
{
  Q_Q(qMRMLSceneModel);
  q->qvtkDisconnect(0, vtkCommand::ModifiedEvent, q, SLOT(onMRMLNodeModified(vtkObject*)));
  if (!this->ListenNodeModifiedEvent)
    {
    return;
    }
  QModelIndex sceneIndex = q->mrmlSceneIndex();
  const int count = q->rowCount(sceneIndex);
  for (int i = 0; i < count; ++i)
    {
    q->qvtkConnect(q->mrmlNodeFromIndex(sceneIndex.child(i,0)),vtkCommand::ModifiedEvent,
                   q, SLOT(onMRMLNodeModified(vtkObject*)));
    }
}

//------------------------------------------------------------------------------
QStringList qMRMLSceneModelPrivate::extraItems(QStandardItem* parent, const QString extraType)const
{
  Q_Q(const qMRMLSceneModel);
  QStringList res;
  if (parent == 0)
    {
    parent = q->invisibleRootItem();
    }
  int rowCount = parent->rowCount();
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
  return res;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelPrivate::removeAllExtraItems(QStandardItem* parent, const QString extraType)
{
  Q_Q(qMRMLSceneModel);
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
qMRMLSceneModel::~qMRMLSceneModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setPreItems(const QStringList& extraItems, QStandardItem* parent)
{
  Q_D(qMRMLSceneModel);
  d->removeAllExtraItems(parent, "preItem");
  int row = 0;
  foreach(QString extraItem, extraItems)
    {
    QStandardItem* item = new QStandardItem;
    item->setData(QString("preItem"), qMRMLSceneModel::UIDRole);
    if (extraItem == "separator")
      {
      item->setData("separator", Qt::AccessibleDescriptionRole);
      }
    else
      {
      item->setText(extraItem);
      }
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    QList<QStandardItem*> items;
    items << item;
    items << new QStandardItem;
    items[1]->setFlags(0);
    if (parent)
      {
      parent->insertRow(row++, items);
      }
    else
      {
      this->insertRow(row++, items);
      }
    }
}

//------------------------------------------------------------------------------
QStringList qMRMLSceneModel::preItems(QStandardItem* parent)const
{
  Q_D(const qMRMLSceneModel);
  return d->extraItems(parent, "preItems");
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setPostItems(const QStringList& extraItems, QStandardItem* parent)
{
  Q_D(qMRMLSceneModel);
  d->removeAllExtraItems(parent, "postItem");
  foreach(QString extraItem, extraItems)
    {
    QStandardItem* item = new QStandardItem;
    item->setData(QString("postItem"), qMRMLSceneModel::UIDRole);
    if (extraItem == "separator")
      {
      item->setData("separator", Qt::AccessibleDescriptionRole);
      }
    else
      {
      item->setText(extraItem);
      }
    item->setFlags(Qt::ItemIsEnabled);
    QList<QStandardItem*> items;
    items << item;
    items << new QStandardItem;
    items[1]->setFlags(0);
    if (parent)
      {
      parent->appendRow(items);
      }
    else
      {
      this->appendRow(items);
      }
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
  if (scene)
    {
    scene->AddObserver(vtkMRMLScene::NodeAboutToBeAddedEvent, d->CallBack, -10.);
    scene->AddObserver(vtkMRMLScene::NodeAddedEvent, d->CallBack, 10.);
    scene->AddObserver(vtkMRMLScene::NodeAboutToBeRemovedEvent, d->CallBack, -10.);
    scene->AddObserver(vtkMRMLScene::NodeRemovedEvent, d->CallBack, 10.);
    scene->AddObserver(vtkCommand::DeleteEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::SceneAboutToBeClosedEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::SceneClosedEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::SceneAboutToBeImportedEvent, d->CallBack);
    scene->AddObserver(vtkMRMLScene::SceneImportedEvent, d->CallBack);
    }
  d->MRMLScene = scene;
  this->updateScene();
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
  if (d->MRMLScene == 0)
    {
    return 0;
    }
  int count = this->invisibleRootItem()->rowCount();
  for (int i = 0; i < count; ++i)
    {
    QStandardItem* child = this->invisibleRootItem()->child(i);
    QVariant uid = child->data(qMRMLSceneModel::UIDRole);
    if (uid.type() == QVariant::String &&
        uid.toString() == "scene")
      {
      return child;
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel::mrmlSceneIndex()const
{
  QStandardItem* scene = this->mrmlSceneItem();
  if (scene == 0)
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
  if (d->MRMLScene == 0 || nodeItem == 0)
    {
    return 0;
    }
  QVariant nodePointer = nodeItem->data(qMRMLSceneModel::PointerRole);
  if (!nodePointer.isValid() || nodeItem->data(qMRMLSceneModel::UIDRole).toString() == "scene")
    {
    return 0;
    }
  //return nodeItem ? d->MRMLScene->GetNodeByID(
  //  nodeItem->data(qMRMLSceneModel::UIDRole).toString().toLatin1()) : 0;
  vtkMRMLNode* node = static_cast<vtkMRMLNode*>(
    reinterpret_cast<void *>(
      nodePointer.toLongLong()));
  Q_ASSERT(node);
  return node;
}
//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneModel::itemFromNode(vtkMRMLNode* node, int column)const
{
  return this->itemFromIndex(this->indexFromNode(node, column));
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel::indexFromNode(vtkMRMLNode* node, int column)const
{
  if (node == 0)
    {
    return QModelIndex();
    }
  // QAbstractItemModel::match doesn't browse through columns
  // we need to do it manually
  QModelIndexList nodeIndexes = this->match(
    this->mrmlSceneIndex(), qMRMLSceneModel::UIDRole, QString(node->GetID()),
    1, Qt::MatchExactly | Qt::MatchRecursive);
  Q_ASSERT(nodeIndexes.size() <= 1); // we know for sure it won't be more than 1
  if (nodeIndexes.size() == 0)
    {
    return QModelIndex();
    }
  if (column == 0)
    {
    // QAbstractItemModel::match only search through the first column
    // (because scene is in the first column)
    return nodeIndexes[0];
    }
  // Add the QModelIndexes from the other columns
  const int row = nodeIndexes[0].row();
  QModelIndex nodeParentIndex = nodeIndexes[0].parent();
  Q_ASSERT( column < this->columnCount(nodeParentIndex) );
  return nodeParentIndex.child(row, column);
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLSceneModel::indexes(vtkMRMLNode* node)const
{
  QModelIndex scene = this->mrmlSceneIndex();
  if (scene == QModelIndex())
    {
    return QModelIndexList();
    }
  // QAbstractItemModel::match doesn't browse through columns
  // we need to do it manually
  QModelIndexList nodeIndexes = this->match(
    scene, qMRMLSceneModel::UIDRole, QString(node->GetID()), 1, Qt::MatchExactly | Qt::MatchRecursive);
  Q_ASSERT(nodeIndexes.size() <= 1); // we know for sure it won't be more than 1
  if (nodeIndexes.size() == 0)
    {
    return nodeIndexes;
    }
  // Add the QModelIndexes from the other columns
  const int row = nodeIndexes[0].row();
  QModelIndex nodeParentIndex = nodeIndexes[0].parent();
  const int sceneColumnCount = this->columnCount(nodeParentIndex);
  for (int j = 1; j < sceneColumnCount; ++j)
    {
    nodeIndexes << nodeParentIndex.child(row, j);
    }
  return nodeIndexes;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setListenNodeModifiedEvent(bool listen)
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
bool qMRMLSceneModel::listenNodeModifiedEvent()const
{
  Q_D(const qMRMLSceneModel);
  return d->ListenNodeModifiedEvent;
}

//------------------------------------------------------------------------------
QMimeData* qMRMLSceneModel::mimeData(const QModelIndexList& indexes)const
{
  Q_D(const qMRMLSceneModel);
  d->LastMimeData = indexes;
  return this->QStandardItemModel::mimeData(indexes);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateScene()
{
  Q_D(qMRMLSceneModel);
  // save extra items
  QStringList oldPreItems = this->preItems(0);
  QStringList oldPostItems = this->postItems(0);

  QStringList oldScenePreItems, oldScenePostItems;
  QList<QStandardItem*> oldSceneItem = this->findItems("Scene");
  if (oldSceneItem.size())
    {
    oldScenePreItems = this->preItems(oldSceneItem[0]);
    oldScenePostItems = this->postItems(oldSceneItem[0]);
    }

  int oldColumnCount = this->columnCount();
  this->setRowCount(0);
  this->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);
  this->setColumnCount(oldColumnCount);

  // restore extra items
  this->setPreItems(oldPreItems, 0);
  this->setPostItems(oldPostItems, 0);
  if (d->MRMLScene == 0)
    {
    return;
    }

  // Add scene item
  QList<QStandardItem*> sceneItems;
  QStandardItem* sceneItem = new QStandardItem;
  sceneItem->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
  sceneItem->setText("Scene");
  sceneItem->setData("scene", qMRMLSceneModel::UIDRole);
  sceneItem->setData(QVariant::fromValue(reinterpret_cast<long long>(d->MRMLScene)), qMRMLSceneModel::PointerRole);
  sceneItems << sceneItem;
  sceneItems << new QStandardItem;
  sceneItems[1]->setFlags(0);
  this->insertRow(oldPreItems.count(), sceneItems);
  this->setPreItems(oldScenePreItems, sceneItem);
  this->setPostItems(oldScenePostItems, sceneItem);

  // Populate scene with nodes
  this->populateScene();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::populateScene()
{
  Q_D(qMRMLSceneModel);
  Q_ASSERT(d->MRMLScene);
  // Add nodes
  vtkMRMLNode *node = 0;
  vtkCollectionSimpleIterator it;
  int row = 0;
  for (d->MRMLScene->GetCurrentScene()->InitTraversal(it);
       (node = (vtkMRMLNode*)d->MRMLScene->GetCurrentScene()->GetNextItemAsObject(it)) ;)
    {
    this->insertNode(node, this->mrmlSceneItem(), row++);
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::insertNode(vtkMRMLNode* node)
{
  QStandardItem* parent = this->mrmlSceneItem();
  this->insertNode(node, parent, parent->rowCount() - this->postItems(parent).count());
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::insertNode(vtkMRMLNode* node, QStandardItem* parent, int row)
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
  if (parent)
    {
    parent->insertRow(row, items);
    //Q_ASSERT(parent->columnCount() == 2);
    }
  else
    {
    this->insertRow(row,items);
    }
  // TODO: don't listen to nodes that are hidden from editors ?
  if (d->ListenNodeModifiedEvent)
    {
    qvtkConnect(node, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  // Here we set the flags in multiple times (for code clarity), however it has
  // the drawback of firing some itemChanged signals, we cascade useless and infinite loops
  // a solution is to prevent ourself from firing any of these signals.
  // Another solution is to create a virtual method that returns the desired flag and set
  // it only once.
  bool oldBlock = this->blockSignals(true);
  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
  this->blockSignals(oldBlock);
  // set UIDRole and set PointerRole need to be atomic
  this->blockSignals(true);
  item->setData(QString(node->GetID()), qMRMLSceneModel::UIDRole);
  item->setData(QVariant::fromValue(reinterpret_cast<long long>(node)), qMRMLSceneModel::PointerRole);
  this->blockSignals(oldBlock);
  switch (column)
    {
    case 0:
    default:
      this->blockSignals(true);
      item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
      this->blockSignals(oldBlock);
      item->setText(QString(node->GetName()));
      break;
    case 1:
      this->blockSignals(true);
      item->setFlags(item->flags() | Qt::ItemIsSelectable);
      this->blockSignals(oldBlock);
      item->setText(QString(node->GetID()));
      break;
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item)
{
  if (item->column() == qMRMLSceneModel::NameColumn)
    {
    node->SetName(item->text().toLatin1());
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
    case vtkMRMLScene::SceneAboutToBeClosedEvent:
      sceneModel->onMRMLSceneAboutToBeClosed(scene);
      break;
    case vtkMRMLScene::SceneClosedEvent:
      sceneModel->onMRMLSceneClosed(scene);
      break;
    case vtkMRMLScene::SceneAboutToBeImportedEvent:
      sceneModel->onMRMLSceneAboutToBeImported(scene);
      break;
    case vtkMRMLScene::SceneImportedEvent:
      sceneModel->onMRMLSceneImported(scene);
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
  Q_ASSERT(scene != 0);
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

  this->insertNode(node);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAboutToBeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_D(qMRMLSceneModel);
  Q_UNUSED(d);
  Q_UNUSED(scene);
  Q_ASSERT(scene == d->MRMLScene);

  qvtkDisconnect(node, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));
  // TODO: can be fasten by browsing the tree only once
  QModelIndexList indexes = this->match(this->mrmlSceneIndex(), qMRMLSceneModel::UIDRole,
                                        QString(node->GetID()), 1,
                                        Qt::MatchExactly | Qt::MatchRecursive);
  if (indexes.count())
    {
    this->removeRow(indexes[0].row(), indexes[0].parent());
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneDeleted(vtkObject* scene)
{
  Q_UNUSED(scene);
#ifndef QT_NO_DEBUG
  Q_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
#endif
  this->setMRMLScene(0);
}

//------------------------------------------------------------------------------
void printStandardItem(QStandardItem* item, const QString& offset)
{
  if (!item)
    {
    return;
    }
  //qDebug() << offset << item << item->index() << item->text()
  //         << item->data(qMRMLSceneModel::UIDRole).toString() << item->row()
  //         << item->column() << item->rowCount() << item->columnCount();
  for(int i = 0; i < item->rowCount(); ++i )
    {
    for (int j = 0; j < item->columnCount(); ++j)
      {
      printStandardItem(item->child(i,j), offset + "   ");
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLNodeModified(vtkObject* node)
{
  Q_D(qMRMLSceneModel);
  vtkMRMLNode* modifiedNode = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(modifiedNode && modifiedNode->GetScene());
  Q_ASSERT(modifiedNode->GetScene()->IsNodePresent(modifiedNode));
  QModelIndexList nodeIndexes = this->indexes(modifiedNode);
  //qDebug() << "onMRMLNodeModified" << modifiedNode->GetID() << nodeIndexes;
  
  for (int i = 0; i < nodeIndexes.size(); ++i)
    {
    QModelIndex index = nodeIndexes[i];
    if (d->LastMimeData.contains(index))
      {
      continue;
      }
    QStandardItem* item = this->itemFromIndex(index);
    int oldRow = item->row();
    QStandardItem* oldParent = item->parent();

    this->updateItemFromNode(item, modifiedNode, item->column());
    // maybe the item has been reparented, then we need to rescan the
    // indexes again as may are wrong.
    if (item->row() != oldRow || item->parent() != oldParent)
      {
      int oldSize = nodeIndexes.size();      
      nodeIndexes = this->indexes(modifiedNode);
      //the number of columns shouldn't change
      Q_ASSERT(oldSize == nodeIndexes.size());
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onItemChanged(QStandardItem * item)
{
  // when a dnd occurs, the order of the items called with onItemChanged is
  // random, it could be the item in column 1 then the item in column 0
  //qDebug() << "onItemChanged: " << item << item->row() << item->column();
  //printStandardItem(this->mrmlSceneItem(), "");
  //return;
  // check on the column is optional(no strong feeling), it is just there to be
  // faster though
  if (item == this->mrmlSceneItem())
    {
    return;
    }
  vtkMRMLNode* mrmlNode = this->mrmlNodeFromItem(item);
  Q_ASSERT(mrmlNode);
  this->updateNodeFromItem(mrmlNode, item);
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
  Q_UNUSED(scene);
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
  //this->endResetModel();
}

//------------------------------------------------------------------------------
//QMimeData *qMRMLSceneModel::mimeData(const QModelIndexList &indexes)const
//{
//}

//------------------------------------------------------------------------------
//QStringList qMRMLSceneModel::mimeTypes()const
//{
//}
/*
//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel::parent(const QModelIndex &_index)const
{
  Q_D(const qMRMLSceneModel);
  if (!_index.isValid())
    {
    return QModelIndex();
    }
  QSharedPointer<const qMRMLAbstractItemHelper> item = 
    QSharedPointer<const qMRMLAbstractItemHelper>(d->itemFromIndex(_index));
  Q_ASSERT(!item.isNull());
  if (item.isNull())
    {
    return QModelIndex();
    }
  // let polymorphism plays its role here...
  QSharedPointer<const qMRMLAbstractItemHelper> parentItem =
    QSharedPointer<const qMRMLAbstractItemHelper>(item->parent());
  if (parentItem.isNull())
    {
    return QModelIndex();
    }
  QModelIndex _parent = this->indexFromItem(parentItem.data());
  return _parent;
}
*/
//------------------------------------------------------------------------------
//bool qMRMLSceneModel::removeColumns(int column, int count, const QModelIndex &parent=QModelIndex())
//{
//}

//------------------------------------------------------------------------------
//bool qMRMLSceneModel::removeRows(int row, int count, const QModelIndex &parent)
//{
//}
/*
//------------------------------------------------------------------------------
int qMRMLSceneModel::rowCount(const QModelIndex &_parent) const
{
  Q_D(const qMRMLSceneModel);
  QSharedPointer<qMRMLAbstractItemHelper> item =
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(_parent));
  Q_ASSERT(!item.isNull());
  if (dynamic_cast<qMRMLAbstractNodeItemHelper*>(item.data()))
    {
    return 0;
    }
//  std::ofstream rowCountDebugFile("rowCount.txt", std::ios_base::app);
//  rowCountDebugFile << _parent.row() << " " << _parent.column()
//                    << " " << _parent.parent().row() << " " << _parent.parent().column()
//                    << " : " << item->childCount() << std::endl;
//  rowCountDebugFile.close();
  int count = !item.isNull() ? item->childCount() : 0;
  if (d->MRMLNodeToBeAdded)
    {
    --count;
    }
  if (d->MRMLNodeToBe)
    {
    --count;
    }
  Q_ASSERT(count >= 0);

  return count;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::setData(const QModelIndex &modelIndex, const QVariant &value, int role)
{
#ifndef QT_NO_DEBUG
  Q_D(const qMRMLSceneModel);
#endif
  if (!modelIndex.isValid())
    {
    return false;
    }
  Q_ASSERT(d->MRMLScene);
  QSharedPointer<qMRMLAbstractItemHelper> item =
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(modelIndex));
  Q_ASSERT(!item.isNull());
  bool changed = !item.isNull() ? item->setData(value, role) : false;
  if (changed)
    {
    emit dataChanged(modelIndex, modelIndex);
    }
  return changed;
}
*/
//------------------------------------------------------------------------------
//bool qMRMLSceneModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
//{
//}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneModel::supportedDropActions()const
{
  return Qt::IgnoreAction;
}
