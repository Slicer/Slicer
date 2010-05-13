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
#include <vtkVariantArray.h>

// STD includes
#include <iostream>
#include <fstream>

class qMRMLSceneModelItemHelperFactoryPrivate;
//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSceneModelItemHelperFactory : public qMRMLAbstractItemHelperFactory
{
public:
  qMRMLSceneModelItemHelperFactory();
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object,int column) const;
  virtual qMRMLAbstractItemHelper* createRootItem(vtkMRMLScene* scene) const;
  
  void setPreItems(vtkCollection* itemCollection);
  vtkCollection* preItems()const;
  void setPostItems(vtkCollection* itemCollection);
  vtkCollection* postItems()const;
private:
  CTK_DECLARE_PRIVATE(qMRMLSceneModelItemHelperFactory);
};

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
  qMRMLFlatSceneItemHelper(vtkMRMLScene* scene, int column, const qMRMLAbstractItemHelperFactory* factory);
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
  qMRMLFlatNodeItemHelper(vtkMRMLNode* node, int column, const qMRMLAbstractItemHelperFactory* factory);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLFlatRootItemHelper : public qMRMLAbstractRootItemHelper
{
public:
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
protected:
  friend class qMRMLSceneModelItemHelperFactory;
  qMRMLFlatRootItemHelper(vtkMRMLScene* scene, const qMRMLAbstractItemHelperFactory* factory);
};

//------------------------------------------------------------------------------
class qMRMLSceneModelItemHelperFactoryPrivate: public ctkPrivate<qMRMLSceneModelItemHelperFactory>
{
public:
  vtkSmartPointer<vtkCollection> PreItems;
  vtkSmartPointer<vtkCollection> PostItems;
};

//------------------------------------------------------------------------------
qMRMLSceneModelItemHelperFactory::qMRMLSceneModelItemHelperFactory()
{
  CTK_INIT_PRIVATE(qMRMLSceneModelItemHelperFactory);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModelItemHelperFactory::createItem(vtkObject* object, int column)const
{
  if (!object)
    {
    Q_ASSERT(object);
    return 0;
    }
  if (object->IsA("vtkMRMLScene"))
    {
    qMRMLAbstractItemHelper* source = new qMRMLFlatSceneItemHelper(vtkMRMLScene::SafeDownCast(object), column, this);
    return new qMRMLExtraItemsHelper(this->preItems(), this->postItems(), source);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLFlatNodeItemHelper(vtkMRMLNode::SafeDownCast(object), column, this);
    }
  else if (object->IsA("vtkVariantArray"))
    {
    return new qMRMLVariantArrayItemHelper(vtkVariantArray::SafeDownCast(object), column, this);    
    }
  else 
    {
    Q_ASSERT(false);
    }
  return 0;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModelItemHelperFactory::createRootItem(vtkMRMLScene* scene)const
{
  return new qMRMLFlatRootItemHelper(scene, this);
}

//------------------------------------------------------------------------------
void qMRMLSceneModelItemHelperFactory::setPostItems(vtkCollection* itemCollection)
{
  CTK_D(qMRMLSceneModelItemHelperFactory);
  d->PostItems = itemCollection;
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLSceneModelItemHelperFactory::postItems()const
{
  CTK_D(const qMRMLSceneModelItemHelperFactory);
  return d->PostItems.GetPointer();
}

//------------------------------------------------------------------------------
void qMRMLSceneModelItemHelperFactory::setPreItems(vtkCollection* itemCollection)
{
  CTK_D(qMRMLSceneModelItemHelperFactory);
  d->PreItems = itemCollection;
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLSceneModelItemHelperFactory::preItems()const
{
  CTK_D(const qMRMLSceneModelItemHelperFactory);
  return d->PreItems.GetPointer();
}

// qMRMLFlatSceneItemHelper

//------------------------------------------------------------------------------
qMRMLFlatSceneItemHelper::qMRMLFlatSceneItemHelper(vtkMRMLScene* scene, int _column, 
                                                   const qMRMLAbstractItemHelperFactory* itemFactory)
  :qMRMLAbstractSceneItemHelper(scene, _column, itemFactory)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLFlatSceneItemHelper::child(int _row, int _column) const
{
  vtkMRMLNode* childNode = this->mrmlScene()->GetNthNode(_row);
  return this->factory()->createItem(childNode, _column);
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
  //return new qMRMLFlatRootItemHelper(this->mrmlScene());
  //return this->mrmlScene();
  return this->factory()->createRootItem(this->mrmlScene());
}

//------------------------------------------------------------------------------
// qMRMLFlatNodeItemHelper

//------------------------------------------------------------------------------
qMRMLFlatNodeItemHelper::qMRMLFlatNodeItemHelper(vtkMRMLNode* node, int _column, 
                                                 const qMRMLAbstractItemHelperFactory* itemFactory)
  :qMRMLAbstractNodeItemHelper(node, _column, itemFactory)
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
// qMRMLFlatRootItemHelper

//------------------------------------------------------------------------------
qMRMLFlatRootItemHelper::qMRMLFlatRootItemHelper(vtkMRMLScene* scene, const qMRMLAbstractItemHelperFactory* itemFactory)
  :qMRMLAbstractRootItemHelper(scene, itemFactory)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLFlatRootItemHelper::child(int _row, int _column) const
{
  if (_row == 0)
    {
    //return new qMRMLFlatSceneItemHelper(this->mrmlScene(), _column);
    //return this->mrmlScene();
    return this->factory()->createItem(this->mrmlScene(), _column);
    }
  return 0;
}

//------------------------------------------------------------------------------
// qMRMLSceneModelPrivate
//------------------------------------------------------------------------------
class qMRMLSceneModelPrivate: public ctkPrivate<qMRMLSceneModel>
{
public:
  CTK_DECLARE_PUBLIC(qMRMLSceneModel);
  qMRMLSceneModelPrivate();
  ~qMRMLSceneModelPrivate();
  
  //qMRMLAbstractItemHelper* itemFromObject(vtkObject* object, int column)const;
  qMRMLAbstractItemHelper* itemFromIndex(const QModelIndex &index)const;
  QModelIndex indexFromItem(const qMRMLAbstractItemHelper* itemHelper)const;

  qMRMLSceneModelItemHelperFactory* ItemFactory;
  vtkMRMLScene* MRMLScene;
  vtkMRMLNode*  MRMLNodeToBe;
  QList<QAction*> Actions;
};

//------------------------------------------------------------------------------
qMRMLSceneModelPrivate::qMRMLSceneModelPrivate()
{
  this->MRMLScene = 0;
  this->MRMLNodeToBe = 0;
  this->ItemFactory = new qMRMLSceneModelItemHelperFactory();
}

//------------------------------------------------------------------------------
qMRMLSceneModelPrivate::~qMRMLSceneModelPrivate()
{
  delete this->ItemFactory;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModelPrivate::itemFromIndex(const QModelIndex &index)const
{
  CTK_P(const qMRMLSceneModel);
  if ((index.row() < 0) || (index.column() < 0) || (index.model() != p))
    {
    //return new qMRMLFlatRootItemHelper(this->MRMLScene);
    return this->ItemFactory->createRootItem(this->MRMLScene);
    }
  //return this->itemFromObject(reinterpret_cast<vtkObject*>(index.internalPointer()), index.column());
  return this->ItemFactory->createItem(reinterpret_cast<vtkObject*>(index.internalPointer()), index.column());
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModelPrivate::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  CTK_P(const qMRMLSceneModel);
  if (item == 0 || item->object() == 0)
    {
    return QModelIndex();
    }
  if (dynamic_cast<const qMRMLAbstractRootItemHelper*>(item) != 0)
    {
    return QModelIndex();
    }
  return p->createIndex(item->row(), item->column(), 
                        reinterpret_cast<void*>(item->object()));
}

//------------------------------------------------------------------------------
// qMRMLSceneModel
//------------------------------------------------------------------------------
qMRMLSceneModel::qMRMLSceneModel(QObject *_parent)
  :QAbstractItemModel(_parent)
{
  CTK_INIT_PRIVATE(qMRMLSceneModel);
}

//------------------------------------------------------------------------------
qMRMLSceneModel::~qMRMLSceneModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setPreItems(vtkObject* itemParent, const QStringList& extraItems)
{
  CTK_D(qMRMLSceneModel);
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
  CTK_D(const qMRMLSceneModel);
  QStringList res; 

  vtkCollection* collection = d->ItemFactory->preItems();
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
  CTK_D(qMRMLSceneModel);
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
  CTK_D(const qMRMLSceneModel);
  QStringList res; 

  vtkCollection* collection = d->ItemFactory->postItems();
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
  CTK_D(qMRMLSceneModel);
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
  this->beginResetModel();
  d->MRMLScene = scene;
  this->endResetModel();
  //this->reset();
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLSceneModel::mrmlScene()const
{
  return ctk_d()->MRMLScene;
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
  CTK_D(const qMRMLSceneModel);
  if (!_index.isValid())
    {
    return QVariant();
    }
  Q_ASSERT(d->MRMLScene);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(_index));

  Q_ASSERT(!item.isNull());
  //qDebug() << "qMRMLSceneModel::data: " << item << item->data(role);
  return item->data(role);
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLSceneModel::flags(const QModelIndex &_index)const
{
  CTK_D(const qMRMLSceneModel);
  if (!_index.isValid())
    {
    return Qt::NoItemFlags;
    }

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(_index));

  Q_ASSERT(!item.isNull());
  return item->flags() & ~Qt::ItemIsDropEnabled;
}

// Has to be reimplemented for speed issues
//------------------------------------------------------------------------------
bool qMRMLSceneModel::hasChildren(const QModelIndex &_parent)const
{
  CTK_D(const qMRMLSceneModel);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(_parent));
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
  CTK_D(const qMRMLSceneModel);
  if (d->MRMLScene == 0 || row < 0 || column < 0)
    {
    return QModelIndex();
    }

  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(_parent));
  Q_ASSERT(!parentItem.isNull() || parentItem->object());
  if (dynamic_cast<qMRMLAbstractNodeItemHelper*>(parentItem.data()))
    {
    return QModelIndex();
    }
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(
      parentItem->child(row, column));
  Q_ASSERT(item.isNull() || item->object());

  return !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
}

//------------------------------------------------------------------------------
//bool qMRMLSceneModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModel::item(const QModelIndex &modelIndex)const
{
  CTK_D(const qMRMLSceneModel);
  return d->itemFromIndex(modelIndex);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelperFactory* qMRMLSceneModel::itemFactory()const
{
  CTK_D(const qMRMLSceneModel);
  return d->ItemFactory;
}

//------------------------------------------------------------------------------
QMap<int, QVariant> qMRMLSceneModel::itemData(const QModelIndex &_index)const
{
  QMap<int, QVariant> roles = this->QAbstractItemModel::itemData(_index);
  QVariant mrmlIdData = this->data(_index, qMRML::UIDRole);
  if (mrmlIdData.type() != QVariant::Invalid)
    {
    roles.insert(qMRML::UIDRole, mrmlIdData);
    }
  return roles;
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAboutToBeAdded(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  CTK_D(qMRMLSceneModel);
  Q_ASSERT(scene != 0);
  Q_ASSERT(scene == d->MRMLScene);
  
  Q_ASSERT(d->MRMLNodeToBe == 0);
  d->MRMLNodeToBe = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(d->MRMLNodeToBe);
  QSharedPointer<qMRMLAbstractItemHelper> sceneItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->ItemFactory->createItem(d->MRMLScene, 0));
  // vtkMRMLScene adds nodes at the end of its collection (but before the extra Items
  // FIXME: handle cases where extraItems are not set to the mrmlscene but to other items
  // (root, mrmlnode?)
  // Warning, if you change the next 2 lines, make sure you do it also in onMRMLSceneNodeAdded
  int insertLocation = sceneItem->childCount() - (d->ItemFactory->postItems()?d->ItemFactory->postItems()->GetNumberOfItems():0);
  this->beginInsertRows(d->indexFromItem(sceneItem.data()), insertLocation, insertLocation);
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAdded(vtkObject* scene, vtkObject* node)
{
  CTK_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  if (d->MRMLNodeToBe == 0)
    {
    // it's kind of ugly to call just NodeAddedEvent without NodeAboutToBeAddedEvent, 
    // but we can handle that case...
    qDebug() << "Warning, vtkMRMLScene::NodeAddedEvent has been fired without"
      " vtkMRMLScene::NodeAboutToBeAddedEvent.";
    //this->onMRMLSceneNodeAboutToBeAdded(scene, node);
    d->MRMLNodeToBe = vtkMRMLNode::SafeDownCast(node);
    Q_ASSERT(d->MRMLNodeToBe);
    Q_ASSERT(d->MRMLScene->IsNodePresent(d->MRMLNodeToBe));
    QSharedPointer<qMRMLAbstractItemHelper> sceneItem = 
      QSharedPointer<qMRMLAbstractItemHelper>(d->ItemFactory->createItem(d->MRMLScene, 0));
    int insertLocation = sceneItem->childCount() -
      (d->ItemFactory->postItems()?d->ItemFactory->postItems()->GetNumberOfItems():0);
    // the node has already been added, offset the position
    insertLocation -= 1;
    this->beginInsertRows(d->indexFromItem(sceneItem.data()), insertLocation , insertLocation);
    }
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node) == d->MRMLNodeToBe);
  d->MRMLNodeToBe = 0;
  this->endInsertRows();

}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAboutToBeRemoved(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  CTK_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(d->MRMLNodeToBe == 0);
  d->MRMLNodeToBe = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(d->MRMLNodeToBe);
  
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->ItemFactory->createItem(node, 0));
  QSharedPointer<qMRMLAbstractItemHelper> _parent = 
    QSharedPointer<qMRMLAbstractItemHelper>(
      item->parent());
  this->beginRemoveRows(d->indexFromItem(_parent.data()), item->row(), item->row());
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeRemoved(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  CTK_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node) == d->MRMLNodeToBe);
  //qDebug() << "onMRMLSceneNodeRemoved: " << d->MRMLNodeToBe->GetName();
  d->MRMLNodeToBe = 0;
  this->endRemoveRows();
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneDeleted(vtkObject* scene)
{
  Q_UNUSED(scene);
  Q_ASSERT(scene == ctk_d()->MRMLScene);
  qDebug() << "onMRMLSceneDeleted";
  this->setMRMLScene(0);
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
  CTK_D(const qMRMLSceneModel);
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
  QModelIndex _parent = d->indexFromItem(parentItem.data());
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
  CTK_D(const qMRMLSceneModel);
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(_parent));
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
  return !item.isNull() ? item->childCount() : 0;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::setData(const QModelIndex &_index, const QVariant &value, int role)
{
  CTK_D(const qMRMLSceneModel);
  if (!_index.isValid())
    {
    return false;
    }
  Q_ASSERT(ctk_d()->MRMLScene);
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(_index));
  Q_ASSERT(!item.isNull());
  bool changed = !item.isNull() ? item->setData(value, role) : false;
  if (changed)
    {
    emit dataChanged(_index, _index);
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
