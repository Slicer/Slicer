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
qMRMLFlatSceneItemHelper::qMRMLFlatSceneItemHelper(vtkMRMLScene* scene, int column)
  :qMRMLAbstractSceneItemHelper(scene, column)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLFlatSceneItemHelper::child(int row, int column) const
{
  vtkMRMLNode* childNode = this->mrmlScene()->GetNthNode(row);
  if (childNode == 0)
    {
    return 0;
    }
  qMRMLAbstractNodeItemHelper* child = 
    new qMRMLAbstractNodeItemHelper(childNode, column);
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
int qMRMLFlatSceneItemHelper::childIndex(const qMRMLAbstractItemHelper* child) const
{
  const qMRMLAbstractNodeItemHelper* nodeItemHelper = 
    dynamic_cast<const qMRMLAbstractNodeItemHelper*>(child);
  Q_ASSERT(nodeItemHelper);
  if (nodeItemHelper == 0)
    {
    return -1;
    }
  Q_ASSERT(!this->mrmlScene() || this->mrmlScene()->IsNodePresent(0) == -1);
  return this->mrmlScene() ? this->mrmlScene()->IsNodePresent(nodeItemHelper->mrmlNode()) : -1;
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
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
qMRMLSceneModelPrivate::qMRMLSceneModelPrivate()
  :MRMLScene(0)
  ,MRMLNodeToBe(0)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneModelPrivate::itemFromObject(vtkObject* object, int column)const
{
  Q_ASSERT(object);
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLFlatSceneItemHelper(vtkMRMLScene::SafeDownCast(object), column);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLAbstractNodeItemHelper(vtkMRMLNode::SafeDownCast(object), column);
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
    return new qMRMLRootItemHelper(this->MRMLScene, true);
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
  if (dynamic_cast<const qMRMLRootItemHelper*>(item) != 0)
    {
    return QModelIndex();
    }
  return p->createIndex(item->row(), item->column(), 
                        reinterpret_cast<void*>(item->object()));
}

//------------------------------------------------------------------------------
qMRMLSceneModel::qMRMLSceneModel(QObject *parent)
  :QAbstractItemModel(parent)
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
int qMRMLSceneModel::columnCount(const QModelIndex &parent)const
{
  return 2;
}

//------------------------------------------------------------------------------
QVariant qMRMLSceneModel::data(const QModelIndex &index, int role)const
{
  QCTK_D(const qMRMLSceneModel);
  if (!index.isValid())
    {
    return QVariant();
    }
  Q_ASSERT(d->MRMLScene);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(index));

  Q_ASSERT(!item.isNull());
  return item->data(role);
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLSceneModel::flags(const QModelIndex &index)const
{
  QCTK_D(const qMRMLSceneModel);
  if (!index.isValid())
    {
    return Qt::NoItemFlags;
    }

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(index));

  Q_ASSERT(!item.isNull());
  return item->flags() & ~Qt::ItemIsDropEnabled;
}

// Has to be reimplemented for speed issues
//------------------------------------------------------------------------------
bool qMRMLSceneModel::hasChildren(const QModelIndex &parent)const
{
  QCTK_D(const qMRMLSceneModel);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(parent));
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
QModelIndex qMRMLSceneModel::index(int row, int column, const QModelIndex &parent)const
{
  QCTK_D(const qMRMLSceneModel);
  if (d->MRMLScene == 0 || row < 0 || column < 0)
    {
    return QModelIndex();
    }

  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(parent));
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
QMap<int, QVariant> qMRMLSceneModel::itemData(const QModelIndex &index)const
{
  QMap<int, QVariant> roles = this->QAbstractItemModel::itemData(index);
  QVariant mrmlIdData = this->data(index, qMRML::UIDRole);
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
  QCTK_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  
  Q_ASSERT(vtkMRMLNode::SafeDownCast(node) == d->MRMLNodeToBe);
  d->MRMLNodeToBe = 0;
  this->endInsertRows();

}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeAboutToBeRemoved(vtkObject* scene, vtkObject* node)
{
  QCTK_D(qMRMLSceneModel);
  Q_ASSERT(scene == d->MRMLScene);
  Q_ASSERT(d->MRMLNodeToBe == 0);
  d->MRMLNodeToBe = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(d->MRMLNodeToBe);
  
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromObject(node, 0));
  QSharedPointer<qMRMLAbstractItemHelper> parent = 
    QSharedPointer<qMRMLAbstractItemHelper>(item->parent());
  this->beginRemoveRows(d->indexFromItem(parent.data()), item->row(), item->row());
}

//------------------------------------------------------------------------------
void qMRMLSceneModel::onMRMLSceneNodeRemoved(vtkObject* scene, vtkObject* node)
{
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
QModelIndex qMRMLSceneModel::parent(const QModelIndex &index)const
{
  QCTK_D(const qMRMLSceneModel);
  if (!index.isValid())
    {
    return QModelIndex();
    }
  QSharedPointer<const qMRMLAbstractItemHelper> item = 
    QSharedPointer<const qMRMLAbstractItemHelper>(d->itemFromIndex(index));
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
  QModelIndex parent = d->indexFromItem(parentItem.data());
  return parent;
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
int qMRMLSceneModel::rowCount(const QModelIndex &parent) const
{
  QCTK_D(const qMRMLSceneModel);
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(parent));
  Q_ASSERT(!item.isNull());
  if (dynamic_cast<qMRMLAbstractNodeItemHelper*>(item.data()))
    {
    return 0;
    }
  /*std::ofstream toto("rowCount.txt", std::ios_base::app);
  toto << parent.row() << " " << parent.column() 
       << " " << parent.parent().row() << " " << parent.parent().column() 
       << " : " << item->childCount() << std::endl;
  toto.close();*/
  return !item.isNull() ? item->childCount() : 0;
}

//------------------------------------------------------------------------------
bool qMRMLSceneModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  QCTK_D(const qMRMLSceneModel);
  if (!index.isValid())
    {
    return false;
    }
  Q_ASSERT(qctk_d()->MRMLScene);
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(index));
  Q_ASSERT(!item.isNull());
  bool changed = !item.isNull() ? item->setData(value, role) : false;
  if (changed)
    {
    emit dataChanged(index, index);
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
