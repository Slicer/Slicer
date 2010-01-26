#include "qMRMLItemHelper.h"
#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"

#include <vtkMRMLScene.h>

#include <QDebug>
#include <QMimeData>
#include <QStringList>
#include <QSharedPointer>
#include <QVector>
#include <QMap>

#include <iostream>
#include <fstream>


// qMRMLFlatSceneItemHelper

//------------------------------------------------------------------------------
qMRMLFlatSceneItemHelper::qMRMLFlatSceneItemHelper(vtkMRMLScene* scene, int _column)
  :qMRMLAbstractSceneItemHelper(scene, _column)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLFlatSceneItemHelper::child(int _row, int _column) const
{
  vtkMRMLNode* childNode = this->mrmlScene()->GetNthNode(_row);
  if (childNode == 0)
    {
    return 0;
    }
  qMRMLAbstractNodeItemHelper* child = 
    new qMRMLFlatNodeItemHelper(childNode, _column);
  return child;
}

//------------------------------------------------------------------------------
int qMRMLFlatSceneItemHelper::childCount() const
{ 
  if (this->column() != 0)
    {
    return 0;
    }
  return this->mrmlScene() ? this->mrmlScene()->GetNumberOfNodes() : 0; 
}

//------------------------------------------------------------------------------
int qMRMLFlatSceneItemHelper::childIndex(const qMRMLAbstractItemHelper* _child) const
{
  const qMRMLAbstractNodeItemHelper* nodeItemHelper = 
    dynamic_cast<const qMRMLAbstractNodeItemHelper*>(_child);
  Q_ASSERT(nodeItemHelper);
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
  return new qMRMLFlatRootItemHelper(this->mrmlScene());
}



//------------------------------------------------------------------------------
qMRMLFlatNodeItemHelper::qMRMLFlatNodeItemHelper(vtkMRMLNode* node, int _column)
  :qMRMLAbstractNodeItemHelper(node, _column)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLFlatNodeItemHelper::parent() const
{
  return new qMRMLFlatSceneItemHelper(this->mrmlNode()->GetScene(), 0);
}

//------------------------------------------------------------------------------
qMRMLFlatRootItemHelper::qMRMLFlatRootItemHelper(vtkMRMLScene* scene)
 :qMRMLAbstractRootItemHelper(scene)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLFlatRootItemHelper::child(int _row, int _column) const
{
  if (_row == 0)
    {
    return new qMRMLFlatSceneItemHelper(this->mrmlScene(), _column);
    }
  return 0;
}




//------------------------------------------------------------------------------
class qMRMLSceneModelPrivate: public qCTKPrivate<qMRMLSceneModel>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLSceneModel);
  qMRMLSceneModelPrivate();
  
  qMRMLAbstractItemHelper* itemFromObject(vtkObject* object, int column)const;
  qMRMLAbstractItemHelper* itemFromIndex(const QModelIndex &index)const;
  QModelIndex indexFromItem(const qMRMLAbstractItemHelper* itemHelper)const;

  vtkMRMLScene* MRMLScene;
  vtkMRMLNode*  MRMLNodeToBe;
};

qMRMLSceneModelPrivate::qMRMLSceneModelPrivate()
{
  this->MRMLScene = 0;
  this->MRMLNodeToBe = 0;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModelPrivate::itemFromObject(vtkObject* object, int _column)const
{
  Q_ASSERT(object);
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLFlatSceneItemHelper(vtkMRMLScene::SafeDownCast(object), _column);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLFlatNodeItemHelper(vtkMRMLNode::SafeDownCast(object), _column);
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModelPrivate::itemFromIndex(const QModelIndex &index)const
{
  QCTK_P(const qMRMLSceneModel);
  if ((index.row() < 0) || (index.column() < 0) || (index.model() != p))
    {
    return new qMRMLFlatRootItemHelper(this->MRMLScene);
    }
  return this->itemFromObject(reinterpret_cast<vtkObject*>(index.internalPointer()), index.column());
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneModelPrivate::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  QCTK_P(const qMRMLSceneModel);
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
qMRMLSceneModel::qMRMLSceneModel(QObject *_parent)
  :QAbstractItemModel(_parent)
{
  QCTK_INIT_PRIVATE(qMRMLSceneModel);
}

//------------------------------------------------------------------------------
qMRMLSceneModel::~qMRMLSceneModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::setMRMLScene(vtkMRMLScene* scene)
{
  QCTK_D(qMRMLSceneModel);
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAboutToBeAddedEvent,
                      this, SLOT(onMRMLSceneNodeAboutToBeAdded(vtkObject*, vtkObject*)));
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAddedEvent,
                      this, SLOT(onMRMLSceneNodeAdded(vtkObject*, vtkObject*)));
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAboutToBeRemovedEvent,
                      this, SLOT(onMRMLSceneNodeAboutToBeRemoved(vtkObject*, vtkObject*)));
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeRemovedEvent,
                      this, SLOT(onMRMLSceneNodeRemoved(vtkObject*, vtkObject*)));

  d->MRMLScene = scene;
  this->reset();
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLSceneModel::mrmlScene()const
{
  return qctk_d()->MRMLScene;
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
  QCTK_D(const qMRMLSceneModel);
  if (!_index.isValid())
    {
    return QVariant();
    }
  Q_ASSERT(d->MRMLScene);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(_index));

  Q_ASSERT(!item.isNull());
  return item->data(role);
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLSceneModel::flags(const QModelIndex &_index)const
{
  QCTK_D(const qMRMLSceneModel);
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
  QCTK_D(const qMRMLSceneModel);

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
  QCTK_D(const qMRMLSceneModel);
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
    QSharedPointer<qMRMLAbstractItemHelper>(parentItem->child(row, column));
  Q_ASSERT(item.isNull() || item->object());

  return !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
}

//------------------------------------------------------------------------------
//bool qMRMLSceneModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//}

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
#include <iostream>
#include <fstream>
//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAboutToBeAdded(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  QCTK_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  
  Q_ASSERT(d->MRMLNodeToBe == 0);
  d->MRMLNodeToBe = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(d->MRMLNodeToBe);
  QSharedPointer<qMRMLAbstractItemHelper> sceneItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromObject(d->MRMLScene, 0));
  // vtkMRMLScene adds nodes at the end of its collection
  this->beginInsertRows(d->indexFromItem(sceneItem.data()), sceneItem->childCount() , sceneItem->childCount());
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAdded(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  QCTK_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node) == d->MRMLNodeToBe);
  d->MRMLNodeToBe = 0;
  this->endInsertRows();

}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAboutToBeRemoved(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  QCTK_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(d->MRMLNodeToBe == 0);
  d->MRMLNodeToBe = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(d->MRMLNodeToBe);
  
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromObject(node, 0));
  QSharedPointer<qMRMLAbstractItemHelper> _parent = 
    QSharedPointer<qMRMLAbstractItemHelper>(item->parent());
  this->beginRemoveRows(d->indexFromItem(_parent.data()), item->row(), item->row());
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeRemoved(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  QCTK_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node) == d->MRMLNodeToBe);
  std::cerr << "onMRMLSceneNodeRemoved: " << d->MRMLNodeToBe->GetName() << std::endl;
  d->MRMLNodeToBe = 0;
  this->endRemoveRows();
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
  QCTK_D(const qMRMLSceneModel);
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
  QCTK_D(const qMRMLSceneModel);
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
  QCTK_D(const qMRMLSceneModel);
  if (!_index.isValid())
    {
    return false;
    }
  Q_ASSERT(qctk_d()->MRMLScene);
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
























































