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
#include "qMRMLSceneModel.h"
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

//------------------------------------------------------------------------------
// qMRMLSceneModelPrivate
//------------------------------------------------------------------------------
class qMRMLSceneModelPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSceneModel);
protected:
  qMRMLSceneModel* const q_ptr;
public:
  qMRMLSceneModelPrivate(qMRMLSceneModel& object);
  ~qMRMLSceneModelPrivate();
  void init(qMRMLSceneModelItemHelperFactory*);
  
  vtkObject* object(const QModelIndex &index)const;

  //qMRMLAbstractItemHelper* itemFromObject(vtkObject* object, int column)const;
  qMRMLAbstractItemHelper* itemFromIndex(const QModelIndex &index)const;
  QModelIndex indexFromItem(const qMRMLAbstractItemHelper* itemHelper)const;

  void listenNodeModifiedEvent();

  vtkSmartPointer<vtkCallbackCommand> CallBack;
  bool ListenNodeModifiedEvent;
  qMRMLSceneModelItemHelperFactory* ItemFactory;
  vtkMRMLScene* MRMLScene;
  vtkMRMLNode*  MRMLNodeToBe;
  vtkMRMLNode*  MRMLNodeToBeAdded;
  QList<QAction*> Actions;
};

//------------------------------------------------------------------------------
qMRMLSceneModelPrivate::qMRMLSceneModelPrivate(qMRMLSceneModel& object)
  : q_ptr(&object)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->ListenNodeModifiedEvent = false;
  this->ItemFactory = 0;
  this->MRMLScene = 0;
  this->MRMLNodeToBe = 0;
  this->MRMLNodeToBeAdded = 0;
}

//------------------------------------------------------------------------------
qMRMLSceneModelPrivate::~qMRMLSceneModelPrivate()
{
  if (this->MRMLScene)
    {
    this->MRMLScene->RemoveObserver(this->CallBack);
    }
  delete this->ItemFactory;
}

//------------------------------------------------------------------------------
void qMRMLSceneModelPrivate::init(qMRMLSceneModelItemHelperFactory* factory)
{
  Q_Q(qMRMLSceneModel);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qMRMLSceneModel::DoCallback);
  this->ItemFactory = factory;
}

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
    q->qvtkConnect(q->mrmlNode(sceneIndex.child(i,0)),vtkCommand::ModifiedEvent,
                   q, SLOT(onMRMLNodeModified(vtkObject*)));
    }
}

//------------------------------------------------------------------------------
// qMRMLSceneModel
//------------------------------------------------------------------------------
qMRMLSceneModel::qMRMLSceneModel(QObject *_parent)
  :QAbstractItemModel(_parent)
  , d_ptr(new qMRMLSceneModelPrivate(*this))
{
  Q_D(qMRMLSceneModel);
  d->init(new qMRMLSceneModelItemHelperFactory);
}

//------------------------------------------------------------------------------
qMRMLSceneModel::qMRMLSceneModel(qMRMLSceneModelItemHelperFactory* factory, QObject *parentObject)
  :QAbstractItemModel(parentObject)
  , d_ptr(new qMRMLSceneModelPrivate(*this))
{
  Q_D(qMRMLSceneModel);
  d->init(factory);
}

//------------------------------------------------------------------------------
qMRMLSceneModel::~qMRMLSceneModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setPreItems(vtkObject* itemParent, const QStringList& extraItems)
{
  Q_D(qMRMLSceneModel);
  vtkCollection* collection = vtkCollection::New();
  vtkVariantArray* variantArray = 0;

  foreach(const QString& extraItem, extraItems)
    {
    variantArray = vtkVariantArray::New();
    qMRMLVariantArrayItemHelper::createProperties(*variantArray, 
      itemParent, 
      extraItem.toStdString(), 
      Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    collection->AddItem(variantArray);
    variantArray->Delete();
    }
  d->ItemFactory->setPreItems(collection);
  collection->Delete();
  // FIXME should probably do a row insertion thingy
  this->reset();
}

//------------------------------------------------------------------------------
QStringList qMRMLSceneModel::preItems(vtkObject* itemParent)const 
{
  Q_D(const qMRMLSceneModel);
  QStringList res; 

  vtkCollection* collection = d->ItemFactory->preItems();
  if (collection == 0)
    {
    return res;
    }
  // FIXME: not thread safe
  collection->InitTraversal();
  vtkObject* item = 0;
  for(item = collection->GetNextItemAsObject(); item; item = collection->GetNextItemAsObject())
    {
    res.append(vtkVariantArray::SafeDownCast(item)->GetValue(1).ToString().c_str());
    }
  return res;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setPostItems(vtkObject* itemParent, const QStringList& extraItems)
{
  Q_D(qMRMLSceneModel);
  vtkCollection* collection = vtkCollection::New();
  foreach(const QString& extraItem, extraItems)
    {
    vtkVariantArray* variantArray = vtkVariantArray::New();
    qMRMLVariantArrayItemHelper::createProperties(*variantArray, itemParent, extraItem.toStdString());
    collection->AddItem(variantArray);
    variantArray->Delete();
    }

  // FIXME should probably do a row insertion thingy
  //this->reset();
  this->beginResetModel();
  d->ItemFactory->setPostItems(collection);
  this->endResetModel();
  collection->Delete();
}

//------------------------------------------------------------------------------
QStringList qMRMLSceneModel::postItems(vtkObject* itemParent)const
{
  Q_D(const qMRMLSceneModel);
  QStringList res; 

  vtkCollection* collection = d->ItemFactory->postItems();
  if (collection == 0)
    {
    return res;
    }

  // FIXME: not thread safe
  collection->InitTraversal();
  vtkObject* item = 0;
  for(item = collection->GetNextItemAsObject(); item; item = collection->GetNextItemAsObject())
    {
    res.append(vtkVariantArray::SafeDownCast(item)->GetValue(1).ToString().c_str());
    }
  return res;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneModel);
  /*
  // onMRMLSceneNodeAboutToBeAdded must be call as late as possible after 
  // vtkMRMLScene::NodeAboutToBeAddedEvent is fired. This fix the pb when a node
  // is created in the callback of the NodeAboutToBeAddedEvent.
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAboutToBeAddedEvent,
                      this, SLOT(onMRMLSceneNodeAboutToBeAdded(vtkObject*, vtkObject*)),
                      -10.);
  // onMRMLSceneNodeAdded must be call as soon as possible after 
  // vtkMRMLScene::NodeAddedEvent is fired. This fix the pb when a node is created in the 
  // callback of the NodeAddedEvent.
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAddedEvent,
                      this, SLOT(onMRMLSceneNodeAdded(vtkObject*, vtkObject*)), 
                      10.);
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAboutToBeRemovedEvent,
                      this, SLOT(onMRMLSceneNodeAboutToBeRemoved(vtkObject*, vtkObject*)),
                      -10.);
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeRemovedEvent,
                      this, SLOT(onMRMLSceneNodeRemoved(vtkObject*, vtkObject*)), 10.);
  this->qvtkReconnect(d->MRMLScene, scene, vtkCommand::DeleteEvent,
                      this, SLOT(onMRMLSceneDeleted(vtkObject*)));
  */
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
  this->beginResetModel();
  d->MRMLScene = scene;
  this->endResetModel();
  d->listenNodeModifiedEvent();
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLSceneModel::mrmlScene()const
{
  Q_D(const qMRMLSceneModel);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel::mrmlSceneIndex()const
{
  Q_D(const qMRMLSceneModel);
  if (d->MRMLScene == 0)
    {
    return QModelIndex();
    }
  QSharedPointer<qMRMLAbstractItemHelper> sceneItem =
    QSharedPointer<qMRMLAbstractItemHelper>(d->ItemFactory->createItem(d->MRMLScene, 0));
  return this->indexFromItem(sceneItem.data());
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneModel::mrmlNode(const QModelIndex &nodeIndex)const
{
  Q_D(const qMRMLSceneModel);
  return vtkMRMLNode::SafeDownCast(d->object(nodeIndex));
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLSceneModel::indexes(vtkMRMLNode* node)const
{
  Q_D(const qMRMLSceneModel);
  QModelIndexList nodeIndexList;
  if (node == 0)
    {
    return nodeIndexList;
    }
  QSharedPointer<qMRMLAbstractItemHelper> firstNodeItem =
    QSharedPointer<qMRMLAbstractItemHelper>(d->ItemFactory->createItem(node, 0));
  Q_ASSERT(!firstNodeItem.isNull());
  QModelIndex firstNodeIndex = this->indexFromItem(firstNodeItem.data());
  const int nodeRow = firstNodeIndex.row();
  QModelIndex nodeParent = firstNodeIndex.parent();
  const int columns = this->columnCount(nodeParent);
  for (int i = 0; i < columns; ++i)
    {
    nodeIndexList.push_back(nodeParent.child(nodeRow, i));
    }
  return nodeIndexList;
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
int qMRMLSceneModel::columnCount(const QModelIndex &_parent)const
{
  Q_UNUSED(_parent);
  return 2;
}

//------------------------------------------------------------------------------
QVariant qMRMLSceneModel::data(const QModelIndex &_index, int role)const
{
#ifndef QT_NO_DEBUG
  Q_D(const qMRMLSceneModel);
#endif
  if (!_index.isValid())
    {
    return QVariant();
    }
  Q_ASSERT(d->MRMLScene);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(_index));

  Q_ASSERT(!item.isNull());
  //qDebug() << "qMRMLSceneModel::data: " << item << item->data(role);
  return item->data(role);
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLSceneModel::flags(const QModelIndex &_index)const
{
  //Q_D(const qMRMLSceneModel);
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
bool qMRMLSceneModel::hasChildren(const QModelIndex &_parent)const
{
  //Q_D(const qMRMLSceneModel);

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
QVariant qMRMLSceneModel::headerData(int section, Qt::Orientation orientation, int role) const
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
QModelIndex qMRMLSceneModel::index(int row, int column, const QModelIndex &_parent)const
{
  Q_D(const qMRMLSceneModel);
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
//bool qMRMLSceneModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModel::itemFromIndex(const QModelIndex &modelIndex)const
{
  Q_D(const qMRMLSceneModel);
  return d->itemFromIndex(modelIndex);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModel::itemFromObject(vtkObject* object, int column)const
{
  Q_D(const qMRMLSceneModel);
  return d->ItemFactory->createItem(object, column);
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModel::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  Q_D(const qMRMLSceneModel);
  return d->indexFromItem(item);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelperFactory* qMRMLSceneModel::itemFactory()const
{
  Q_D(const qMRMLSceneModel);
  return d->ItemFactory;
}

//------------------------------------------------------------------------------
QMap<int, QVariant> qMRMLSceneModel::itemData(const QModelIndex &_index)const
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

//-----------------------------------------------------------------------------
void qMRMLSceneModel::DoCallback(vtkObject* vtk_obj, unsigned long event,
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
#ifndef QT_NO_DEBUG
  Q_D(qMRMLSceneModel);
  Q_ASSERT(scene != 0);
  Q_ASSERT(scene == d->MRMLScene);
#endif
  /*
  Q_ASSERT(d->MRMLNodeToBe == 0);
  Q_ASSERT(d->MRMLNodeToBeAdded == 0);
  d->MRMLNodeToBeAdded = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(d->MRMLNodeToBeAdded);
  QSharedPointer<qMRMLAbstractItemHelper> sceneItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->ItemFactory->createItem(d->MRMLScene, 0));
  // vtkMRMLScene adds nodes at the end of its collection (but before the extra Items
  // FIXME: handle cases where extraItems are not set to the mrmlscene but to other items
  // (root, mrmlnode?)
  // Warning, if you change the next 2 lines, make sure you do it also in onMRMLSceneNodeAdded
  int insertLocation = sceneItem->childCount() - (d->ItemFactory->postItems()?d->ItemFactory->postItems()->GetNumberOfItems():0);
  this->beginInsertRows(this->indexFromItem(sceneItem.data()), insertLocation, insertLocation);
  */
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAdded(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node));
  //Q_ASSERT(d->MRMLNodeToBeAdded);
  int insertLocation = -1;
  if (d->MRMLNodeToBeAdded == 0)
    {
    // it's kind of ugly to call just NodeAddedEvent without NodeAboutToBeAddedEvent, 
    // but we can handle that case...
    //qDebug() << "Warning, vtkMRMLScene::NodeAddedEvent has been fired without"
    //  " vtkMRMLScene::NodeAboutToBeAddedEvent.";
    //this->onMRMLSceneNodeAboutToBeAdded(scene, node);
    d->MRMLNodeToBeAdded = vtkMRMLNode::SafeDownCast(node);
    Q_ASSERT(d->MRMLNodeToBeAdded);
    Q_ASSERT(d->MRMLScene->IsNodePresent(d->MRMLNodeToBeAdded));
    qMRMLAbstractItemHelper* sceneItem = d->ItemFactory->createItem(d->MRMLScene, 0);
    insertLocation = sceneItem->childCount() -
      (d->ItemFactory->postItems()?d->ItemFactory->postItems()->GetNumberOfItems():0);
    // the node has already been added, offset the position
    insertLocation -= 1;
    this->beginInsertRows(this->indexFromItem(sceneItem), insertLocation , insertLocation);
    delete sceneItem;
    }
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node) == d->MRMLNodeToBeAdded);
  d->MRMLNodeToBeAdded = 0;
  // endInsertRows fires the Qt signals
  this->endInsertRows();

  if (d->ListenNodeModifiedEvent)
    {
    qvtkConnect(node, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAboutToBeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_UNUSED(scene);
  Q_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(d->MRMLNodeToBe == 0);
  Q_ASSERT(d->MRMLNodeToBeAdded == 0);

  qvtkDisconnect(node, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->ItemFactory->createItem(node, 0));
  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(
      item->parent());
  int itemRow = parentItem->row(item.data());
  this->beginRemoveRows(this->indexFromItem(parentItem.data()), itemRow, itemRow);
  //qDebug() << (void *) this << "about to remove:" << vtkMRMLNode::SafeDownCast(node) << itemRow;
  //qDebug() << node->GetID() << this->mrmlScene()->GetNodeByID(node->GetID());
  d->MRMLNodeToBe = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(d->MRMLNodeToBe);
  this->endRemoveRows();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  
  Q_D(qMRMLSceneModel);
  /*
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node) == d->MRMLNodeToBe);
  
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->ItemFactory->createItem(node, 0));
  QSharedPointer<qMRMLAbstractItemHelper> _parent = 
    QSharedPointer<qMRMLAbstractItemHelper>(
      item->parent());
  int itemRow = item->row();
  this->beginRemoveRows(this->indexFromItem(_parent.data()), itemRow, itemRow);
  
  //qDebug() << "onMRMLSceneNodeRemoved: " << d->MRMLNodeToBe->GetName();
  d->MRMLNodeToBe = 0;
  this->endRemoveRows();
  */
  //qDebug() << (void *) this << "removed:" << node;

  Q_ASSERT(d->MRMLNodeToBe == node);
  d->MRMLNodeToBe = 0;
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
void qMRMLSceneModel::onMRMLNodeModified(vtkObject* node)
{
  vtkMRMLNode* modifiedNode = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(modifiedNode && modifiedNode->GetScene());
  Q_ASSERT(modifiedNode->GetScene()->IsNodePresent(modifiedNode));
  QModelIndexList nodeIndexes = this->indexes(modifiedNode);
  emit dataChanged(nodeIndexes.first(), nodeIndexes.last());
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneAboutToBeImported(vtkMRMLScene* scene)
{
  //this->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneImported(vtkMRMLScene* scene)
{
  //this->endResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneAboutToBeClosed(vtkMRMLScene* scene)
{
  //this->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneClosed(vtkMRMLScene* scene)
{
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

//------------------------------------------------------------------------------
//bool qMRMLSceneModel::removeColumns(int column, int count, const QModelIndex &parent=QModelIndex())
//{
//}

//------------------------------------------------------------------------------
//bool qMRMLSceneModel::removeRows(int row, int count, const QModelIndex &parent)
//{
//}

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
  /*std::ofstream rowCountDebugFile("rowCount.txt", std::ios_base::app);
  rowCountDebugFile << _parent.row() << " " << _parent.column()
                    << " " << _parent.parent().row() << " " << _parent.parent().column() 
                    << " : " << item->childCount() << std::endl;
  rowCountDebugFile.close();*/
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

//------------------------------------------------------------------------------
//bool qMRMLSceneModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
//{
//}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneModel::supportedDropActions()const
{
  return Qt::IgnoreAction;
}
