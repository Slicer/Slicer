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
#include "qMRMLItemHelper.h"
#include "qMRMLSceneModel2.h"
#include "qMRMLSceneModel2_p.h"
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
  friend class qMRMLSceneModel2ItemHelperFactory;
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
  friend class qMRMLSceneModel2ItemHelperFactory;
  qMRMLFlatNodeItemHelper(vtkMRMLNode* node, int column, const qMRMLAbstractItemHelperFactory* factory, int row);
};

//------------------------------------------------------------------------------
class qMRMLSceneModel2ItemHelperFactoryPrivate
{
public:
  vtkSmartPointer<vtkCollection> PreItems;
  vtkSmartPointer<vtkCollection> PostItems;
};

//------------------------------------------------------------------------------
qMRMLSceneModel2ItemHelperFactory::qMRMLSceneModel2ItemHelperFactory()
  : d_ptr(new qMRMLSceneModel2ItemHelperFactoryPrivate)
{
}

//------------------------------------------------------------------------------
qMRMLSceneModel2ItemHelperFactory::~qMRMLSceneModel2ItemHelperFactory()
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModel2ItemHelperFactory::createItem(vtkObject* object, int column, int row)const
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
void qMRMLSceneModel2ItemHelperFactory::setPostItems(vtkCollection* itemCollection)
{
  Q_D(qMRMLSceneModel2ItemHelperFactory);
  d->PostItems = itemCollection;
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLSceneModel2ItemHelperFactory::postItems()const
{
  Q_D(const qMRMLSceneModel2ItemHelperFactory);
  return d->PostItems.GetPointer();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2ItemHelperFactory::setPreItems(vtkCollection* itemCollection)
{
  Q_D(qMRMLSceneModel2ItemHelperFactory);
  d->PreItems = itemCollection;
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLSceneModel2ItemHelperFactory::preItems()const
{
  Q_D(const qMRMLSceneModel2ItemHelperFactory);
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
qMRMLSceneModel2Private::qMRMLSceneModel2Private(qMRMLSceneModel2& object)
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
qMRMLSceneModel2Private::~qMRMLSceneModel2Private()
{
  if (this->MRMLScene)
    {
    this->MRMLScene->RemoveObserver(this->CallBack);
    }
  //delete this->ItemFactory;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2Private::init(/*qMRMLSceneModel2ItemHelperFactory* factory*/)
{
  Q_Q(qMRMLSceneModel2);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qMRMLSceneModel2::onMRMLSceneEvent);
  q->setColumnCount(2);
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)));
  //this->ItemFactory = factory;
}
/*
//------------------------------------------------------------------------------
vtkObject* qMRMLSceneModel2Private::object(const QModelIndex &index)const
{
  return reinterpret_cast<vtkObject*>(index.internalPointer());
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModel2Private::itemFromIndex(const QModelIndex &index)const
{
  Q_Q(const qMRMLSceneModel2);
  if ((index.row() < 0) || (index.column() < 0) || (index.model() != q))
    {
    //return new qMRMLFlatRootItemHelper(this->MRMLScene);
    return this->ItemFactory->createRootItem(this->MRMLScene);
    }
  //return this->itemFromObject(reinterpret_cast<vtkObject*>(index.internalPointer()), index.column());
  return this->ItemFactory->createItem(this->object(index), index.column(), index.row());
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel2Private::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  Q_Q(const qMRMLSceneModel2);
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
void qMRMLSceneModel2Private::listenNodeModifiedEvent()
{
  Q_Q(qMRMLSceneModel2);
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
QStringList qMRMLSceneModel2Private::extraItems(QStandardItem* parent, const QString extraType)const
{
  Q_Q(const qMRMLSceneModel2);
  QStringList res;
  if (parent == 0)
    {
    parent = q->invisibleRootItem();
    }
  int rowCount = parent->rowCount();
  for (int i = 0; i < rowCount; ++i)
    {
    QStandardItem* child = parent->child(i);
    if (child && child->data(qMRML::UIDRole).toString() == extraType)
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
void qMRMLSceneModel2Private::removeAllExtraItems(QStandardItem* parent, const QString extraType)
{
  Q_Q(qMRMLSceneModel2);
  QModelIndex start = parent ? parent->index().child(0,0) : QModelIndex().child(0,0);
  QModelIndexList indexes =
    q->match(start, qMRML::UIDRole, extraType, 1, Qt::MatchExactly);
  while (start != QModelIndex() && indexes.size())
    {
    int row = indexes[0].row();
    q->removeRow(row, indexes[0].parent());
    // don't start the whole search from scratch, only from where we ended it
    start = parent ? parent->index().child(row,0) : QModelIndex().child(row,0);
    indexes = q->match(start, qMRML::UIDRole, extraType, 1, Qt::MatchExactly);
    }
}

//------------------------------------------------------------------------------
// qMRMLSceneModel2
//------------------------------------------------------------------------------
qMRMLSceneModel2::qMRMLSceneModel2(QObject *_parent)
  :QStandardItemModel(_parent)
  , d_ptr(new qMRMLSceneModel2Private(*this))
{
  Q_D(qMRMLSceneModel2);
  d->init(/*new qMRMLSceneModel2ItemHelperFactory*/);
}

//------------------------------------------------------------------------------
qMRMLSceneModel2::qMRMLSceneModel2(qMRMLSceneModel2Private* pimpl, QObject *parentObject)
  :QStandardItemModel(parentObject)
  , d_ptr(pimpl)
{
  Q_D(qMRMLSceneModel2);
  d->init(/*factory*/);
}

//------------------------------------------------------------------------------
qMRMLSceneModel2::~qMRMLSceneModel2()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::setPreItems(const QStringList& extraItems, QStandardItem* parent)
{
  Q_D(qMRMLSceneModel2);
  d->removeAllExtraItems(parent, "preItem");
  int row = 0;
  foreach(QString extraItem, extraItems)
    {
    QStandardItem* item = new QStandardItem;
    item->setData(QString("preItem"), qMRML::UIDRole);
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
QStringList qMRMLSceneModel2::preItems(QStandardItem* parent)const
{
  Q_D(const qMRMLSceneModel2);
  return d->extraItems(parent, "preItems");
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::setPostItems(const QStringList& extraItems, QStandardItem* parent)
{
  Q_D(qMRMLSceneModel2);
  d->removeAllExtraItems(parent, "postItem");
  foreach(QString extraItem, extraItems)
    {
    QStandardItem* item = new QStandardItem;
    item->setData(QString("postItem"), qMRML::UIDRole);
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
QStringList qMRMLSceneModel2::postItems(QStandardItem* parent)const
{
  Q_D(const qMRMLSceneModel2);
  return d->extraItems(parent, "postItem");
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneModel2);
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
vtkMRMLScene* qMRMLSceneModel2::mrmlScene()const
{
  Q_D(const qMRMLSceneModel2);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneModel2::mrmlSceneItem()const
{
  Q_D(const qMRMLSceneModel2);
  if (d->MRMLScene == 0)
    {
    return 0;
    }
  int count = this->invisibleRootItem()->rowCount();
  for (int i = 0; i < count; ++i)
    {
    QStandardItem* child = this->invisibleRootItem()->child(i);
    if (child->text() == "Scene" &&
        child->data(qMRML::UIDRole).type() == QVariant::String)
      {
      return child;
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel2::mrmlSceneIndex()const
{
  QStandardItem* scene = this->mrmlSceneItem();
  if (scene == 0)
    {
    return QModelIndex();
    }
  return scene ? scene->index() : QModelIndex();
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneModel2::mrmlNodeFromItem(QStandardItem* nodeItem)const
{
  Q_D(const qMRMLSceneModel2);
  // TODO: fasten by saving the pointer into the data
  if (d->MRMLScene == 0)
    {
    return 0;
    }
  return nodeItem ? d->MRMLScene->GetNodeByID(
    nodeItem->data(qMRML::UIDRole).toString().toLatin1()) : 0;
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneModel2::itemFromNode(vtkMRMLNode* node, int column)const
{
  //Q_D(const qMRMLSceneModel2);
  if (node == 0)
    {
    return 0;
    }
  QModelIndexList indexes = this->match(this->mrmlSceneIndex(), qMRML::UIDRole,
                                      QString(node->GetID()), 1,
                                      Qt::MatchExactly | Qt::MatchRecursive);
  while (indexes.size())
    {
    if (indexes[0].column() == column)
      {
      return this->itemFromIndex(indexes[0]);
      }
    indexes = this->match(indexes[0], qMRML::UIDRole,
                          QString(node->GetID()), 1,
                          Qt::MatchExactly | Qt::MatchRecursive);
    }
  return 0;
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLSceneModel2::indexes(vtkMRMLNode* node)const
{
  //Q_D(const qMRMLSceneModel2);
  QModelIndex scene = this->mrmlSceneIndex();
  if (scene == QModelIndex())
    {
    return QModelIndexList();
    }
  return this->match(scene, qMRML::UIDRole, QString(node->GetID()), -1, Qt::MatchExactly | Qt::MatchRecursive);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::setListenNodeModifiedEvent(bool listen)
{
  Q_D(qMRMLSceneModel2);
  if (d->ListenNodeModifiedEvent == listen)
    {
    return;
    }
  d->ListenNodeModifiedEvent = listen;
  d->listenNodeModifiedEvent();
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel2::listenNodeModifiedEvent()const
{
  Q_D(const qMRMLSceneModel2);
  return d->ListenNodeModifiedEvent;
}
/*
//------------------------------------------------------------------------------
int qMRMLSceneModel2::columnCount(const QModelIndex &_parent)const
{
  Q_UNUSED(_parent);
  return 2;
}

//------------------------------------------------------------------------------
QVariant qMRMLSceneModel2::data(const QModelIndex &_index, int role)const
{
#ifndef QT_NO_DEBUG
  Q_D(const qMRMLSceneModel2);
#endif
  if (!_index.isValid())
    {
    return QVariant();
    }
  Q_ASSERT(d->MRMLScene);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(_index));

  Q_ASSERT(!item.isNull());
  //qDebug() << "qMRMLSceneModel2::data: " << item << item->data(role);
  return item->data(role);
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLSceneModel2::flags(const QModelIndex &_index)const
{
  //Q_D(const qMRMLSceneModel2);
  if (!_index.isValid())
    {
    return Qt::NoItemFlags;
    }

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(_index));

  Q_ASSERT(!item.isNull());
  return item->flags() & ~Qt::ItemIsDropEnabled;
}

// Has to be reimplemented for speed issues
//------------------------------------------------------------------------------
bool qMRMLSceneModel2::hasChildren(const QModelIndex &_parent)const
{
  //Q_D(const qMRMLSceneModel2);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(_parent));
  Q_ASSERT(!item.isNull());
  if (dynamic_cast<qMRMLAbstractNodeItemHelper*>(item.data()))
    {
    return 0;
    }
  return item->hasChildren();
}

//------------------------------------------------------------------------------
QVariant qMRMLSceneModel2::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical || role != Qt::DisplayRole)
    {
    return QVariant();
    }
  switch (section)
    {
    case 0:
      return tr("Name");
      break;
    case 1:
      return tr("Id");
      break;
    default:
      break;
    };

  return QVariant();
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel2::index(int row, int column, const QModelIndex &_parent)const
{
  Q_D(const qMRMLSceneModel2);
  if (d->MRMLScene == 0 || row < 0 || column < 0)
    {
    return QModelIndex();
    }

  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(_parent));
  Q_ASSERT(!parentItem.isNull() || parentItem->object());
  if (dynamic_cast<qMRMLAbstractNodeItemHelper*>(parentItem.data()))
    {
    return QModelIndex();
    }
  int child = row;
  if(d->MRMLNodeToBeAdded)
    {
    int pos = this->mrmlScene()->IsNodePresent(d->MRMLNodeToBeAdded);
    --pos;
    if (pos != -1 && pos <= child)
      {
      --child;
      }
    }
  if(d->MRMLNodeToBe)
    {
    int pos = this->mrmlScene()->IsNodePresent(d->MRMLNodeToBe);
    --pos;
    if (pos != -1 && pos <= child)
      {
      ++child;
      }
    }
  if (child < 0)
    {
    return QModelIndex();
    }
  //qDebug() << (void *) this << "index:" << row << column << child << _parent << d->MRMLNodeToBe;
  QSharedPointer<qMRMLAbstractItemHelper> item =
    QSharedPointer<qMRMLAbstractItemHelper>(
      parentItem->child(child, column));
  Q_ASSERT(item.isNull() || item->object());

  return !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
}

//------------------------------------------------------------------------------
//bool qMRMLSceneModel2::insertRows(int row, int count, const QModelIndex &parent)
//{
//}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModel2::itemFromIndex(const QModelIndex &modelIndex)const
{
  Q_D(const qMRMLSceneModel2);
  return d->itemFromIndex(modelIndex);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModel2::itemFromObject(vtkObject* object, int column)const
{
  Q_D(const qMRMLSceneModel2);
  return d->ItemFactory->createItem(object, column);
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel2::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  Q_D(const qMRMLSceneModel2);
  return d->indexFromItem(item);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelperFactory* qMRMLSceneModel2::itemFactory()const
{
  Q_D(const qMRMLSceneModel2);
  return d->ItemFactory;
}

//------------------------------------------------------------------------------
QMap<int, QVariant> qMRMLSceneModel2::itemData(const QModelIndex &_index)const
{
  QMap<int, QVariant> roles = this->QAbstractItemModel::itemData(_index);
  // In order to have the drag/drop working without defining our own MIME type
  // we need to add some way of uniquely identify an item. Here it's done
  // by adding a UID role to each item.
  QVariant mrmlIdData = this->data(_index, qMRML::UIDRole);
  if (mrmlIdData.type() != QVariant::Invalid)
    {
    roles.insert(qMRML::UIDRole, mrmlIdData);
    }
  return roles;
}
*/

//------------------------------------------------------------------------------
QMimeData* qMRMLSceneModel2::mimeData(const QModelIndexList& indexes)const
{
  Q_D(const qMRMLSceneModel2);
  d->LastMimeData = indexes;
  return this->QStandardItemModel::mimeData(indexes);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::updateScene()
{
  Q_D(qMRMLSceneModel2);
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
  this->clear();
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
  sceneItem->setData(QString::number(reinterpret_cast<long int>(d->MRMLScene)), qMRML::UIDRole);
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
void qMRMLSceneModel2::populateScene()
{
  Q_D(qMRMLSceneModel2);
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
void qMRMLSceneModel2::insertNode(vtkMRMLNode* node)
{
  QStandardItem* parent = this->mrmlSceneItem();
  this->insertNode(node, parent, parent->rowCount() - this->postItems(parent).count());
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::insertNode(vtkMRMLNode* node, QStandardItem* parent, int row)
{
  Q_D(qMRMLSceneModel2);
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node));

  QStandardItem* newNodeItem = new QStandardItem();
  this->updateItemFromNode(newNodeItem, node, 0);
  QStandardItem* newNodeIDItem = new QStandardItem();
  this->updateItemFromNode(newNodeIDItem, node, 1);

  QList<QStandardItem*> items;
  items.append(newNodeItem);
  items.append(newNodeIDItem);
  parent->insertRow(row, items);
  Q_ASSERT(parent->columnCount() == 2);
  if (d->ListenNodeModifiedEvent)
    {
    qvtkConnect(node, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  bool oldBlock = this->blockSignals(true);
  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
  this->blockSignals(oldBlock);
  item->setData(QString(node->GetID()), qMRML::UIDRole);
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
void qMRMLSceneModel2::updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item)
{
  if (item->column() == 0)
    {
    node->SetName(item->text().toLatin1());
    }
}

//-----------------------------------------------------------------------------
void qMRMLSceneModel2::onMRMLSceneEvent(vtkObject* vtk_obj, unsigned long event,
                                        void* client_data, void* call_data)
{
  vtkMRMLScene* scene = reinterpret_cast<vtkMRMLScene*>(vtk_obj);
  qMRMLSceneModel2* sceneModel = reinterpret_cast<qMRMLSceneModel2*>(client_data);
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
void qMRMLSceneModel2::onMRMLSceneNodeAboutToBeAdded(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_UNUSED(scene);
#ifndef QT_NO_DEBUG
  Q_D(qMRMLSceneModel2);
  Q_ASSERT(scene != 0);
  Q_ASSERT(scene == d->MRMLScene);
#endif
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::onMRMLSceneNodeAdded(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_D(qMRMLSceneModel2);
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node));

  this->insertNode(node);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::onMRMLSceneNodeAboutToBeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_UNUSED(scene);
  Q_D(qMRMLSceneModel2);
  Q_ASSERT(scene == d->MRMLScene);

  qvtkDisconnect(node, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));
  // TODO: can be fasten by browsing the tree only once
  QModelIndexList indexes = this->match(this->mrmlSceneIndex(), qMRML::UIDRole, QString(node->GetID()), 1, Qt::MatchExactly | Qt::MatchRecursive);
  while (indexes.count())
    {
    this->removeRow(indexes[0].row(), indexes[0].parent());
    indexes = this->match(this->mrmlSceneIndex(), qMRML::UIDRole, QString(node->GetID()), 1, Qt::MatchExactly | Qt::MatchRecursive);
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::onMRMLSceneNodeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::onMRMLSceneDeleted(vtkObject* scene)
{
  Q_UNUSED(scene);
#ifndef QT_NO_DEBUG
  Q_D(qMRMLSceneModel2);
  Q_ASSERT(scene == d->MRMLScene);
#endif
  this->setMRMLScene(0);
}

//------------------------------------------------------------------------------
void printStandardItem(QStandardItem* item, const QString& offset)
{
  if (!item)
    {
    qDebug() << offset << item;
    return;
    }
  qDebug() << offset << item << item->index() << item->text() << item->data(qMRML::UIDRole).toString() << item->row() << item->column() << item->rowCount() << item->columnCount();
  for(int i = 0; i < item->rowCount(); ++i )
    {
    for (int j = 0; j < item->columnCount(); ++j)
      {
      printStandardItem(item->child(i,j), offset + "   ");
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::onMRMLNodeModified(vtkObject* node)
{
  Q_D(qMRMLSceneModel2);
  vtkMRMLNode* modifiedNode = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(modifiedNode && modifiedNode->GetScene());
  Q_ASSERT(modifiedNode->GetScene()->IsNodePresent(modifiedNode));
  QModelIndexList nodeIndexes = this->match(this->mrmlSceneIndex(), qMRML::UIDRole, QString(modifiedNode->GetID()), -1, Qt::MatchExactly | Qt::MatchRecursive);
  //qDebug() << "onMRMLNodeModified" << modifiedNode->GetID() << nodeIndexes;

  foreach (QModelIndex index, nodeIndexes)
    {
    if (d->LastMimeData.contains(index))
      {
      continue;
      }
    QStandardItem* item = this->itemFromIndex(index);
    //qDebug() << "onMRMLNodeModified" << index.column() << ": "<< item << ":" << item->text() << item->data(qMRML::UIDRole).toString() << "node:" << modifiedNode;
    //qDebug() << "  parent" << item->parent() << item->parent()->text();
    this->updateItemFromNode(item, modifiedNode, item->column());
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::onItemChanged(QStandardItem * item)
{
  // when a dnd occurs, the order of the items called with onItemChanged is
  // random, it could be the item in column 1 then the item in column 0
  qDebug() << "onItemChanged: " << item << item->row() << item->column();
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
void qMRMLSceneModel2::onMRMLSceneAboutToBeImported(vtkMRMLScene* scene)
{
  //this->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::onMRMLSceneImported(vtkMRMLScene* scene)
{
  //this->endResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::onMRMLSceneAboutToBeClosed(vtkMRMLScene* scene)
{
  //this->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel2::onMRMLSceneClosed(vtkMRMLScene* scene)
{
  //this->endResetModel();
}

//------------------------------------------------------------------------------
//QMimeData *qMRMLSceneModel2::mimeData(const QModelIndexList &indexes)const
//{
//}

//------------------------------------------------------------------------------
//QStringList qMRMLSceneModel2::mimeTypes()const
//{
//}
/*
//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel2::parent(const QModelIndex &_index)const
{
  Q_D(const qMRMLSceneModel2);
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
//bool qMRMLSceneModel2::removeColumns(int column, int count, const QModelIndex &parent=QModelIndex())
//{
//}

//------------------------------------------------------------------------------
//bool qMRMLSceneModel2::removeRows(int row, int count, const QModelIndex &parent)
//{
//}
/*
//------------------------------------------------------------------------------
int qMRMLSceneModel2::rowCount(const QModelIndex &_parent) const
{
  Q_D(const qMRMLSceneModel2);
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
bool qMRMLSceneModel2::setData(const QModelIndex &modelIndex, const QVariant &value, int role)
{
#ifndef QT_NO_DEBUG
  Q_D(const qMRMLSceneModel2);
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
//bool qMRMLSceneModel2::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
//{
//}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneModel2::supportedDropActions()const
{
  return Qt::IgnoreAction;
}
