#include "qMRMLItemHelper.h"
#include "qMRMLItemModel.h"
#include "qMRMLUtils.h"

#include <vtkMRMLScene.h>

#include <QDebug>
#include <QMimeData>
#include <QStringList>
#include <QSharedPointer>
#include <QVector>
#include <QMap>

//------------------------------------------------------------------------------
class qMRMLItemModelPrivate: public qCTKPrivate<qMRMLItemModel>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLItemModel);
  qMRMLItemModelPrivate();
  qMRMLAbstractItemHelper* itemFromIndex(const QModelIndex &indexVariable)const;
  QModelIndex indexFromItem(const qMRMLAbstractItemHelper* itemHelper)const;

  qMRMLAbstractItemHelper* createItemFromVTKObject(vtkObject* object, int column = -1);
  qMRMLAbstractItemHelper* createItemFromUID(QVariant uid, int column = -1);

  vtkMRMLScene* MRMLScene;
  bool          TopLevelScene;
};

//------------------------------------------------------------------------------
qMRMLItemModelPrivate::qMRMLItemModelPrivate()
  :MRMLScene(0),
   TopLevelScene(true)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLItemModelPrivate::createItemFromVTKObject(vtkObject* object, int column)
{
  Q_ASSERT(object);
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLSceneItemHelper(vtkMRMLScene::SafeDownCast(object), column);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLNodeItemHelper(vtkMRMLNode::SafeDownCast(object), column);
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLItemModelPrivate::createItemFromUID(QVariant uid, int column)
{
  Q_ASSERT(this->MRMLScene);
  if (uid.toString().isNull())
    {
    return this->createItemFromVTKObject(this->MRMLScene, column);
    }
  Q_ASSERT(!uid.toString().isNull());
  vtkMRMLNode* node = this->MRMLScene->GetNodeByID(uid.toString().toLatin1().data());
  return this->createItemFromVTKObject(node, column);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLItemModelPrivate::itemFromIndex(const QModelIndex &indexVariable)const
{
  QCTK_P(const qMRMLItemModel);
  if ((indexVariable.row() < 0) || (indexVariable.column() < 0) || (indexVariable.model() != p))
    {
    return new qMRMLRootItemHelper(this->MRMLScene, this->TopLevelScene);
    }
  vtkObject* object = 
    reinterpret_cast<vtkObject*>(indexVariable.internalPointer());
  if (!object)
    {
    qDebug() << indexVariable;
    }
  Q_ASSERT(object);
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLSceneItemHelper(vtkMRMLScene::SafeDownCast(object), indexVariable.column());
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLNodeItemHelper(vtkMRMLNode::SafeDownCast(object), indexVariable.column());
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLItemModelPrivate::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  QCTK_P(const qMRMLItemModel);
  if (item == 0 || item->object() == 0)
    {
    return QModelIndex();
    }
  if (this->TopLevelScene && dynamic_cast<const qMRMLRootItemHelper*>(item) != 0)
    {
    return QModelIndex();
    }
  if (!this->TopLevelScene && dynamic_cast<const qMRMLSceneItemHelper*>(item) != 0)
    {
    return QModelIndex();
    }
  return p->createIndex(item->row(), item->column(), 
                        reinterpret_cast<void*>(item->object()));
}

//------------------------------------------------------------------------------
qMRMLItemModel::qMRMLItemModel(QObject *parentVariable)
  :QAbstractItemModel(parentVariable)
{
  QCTK_INIT_PRIVATE(qMRMLItemModel);
}

//------------------------------------------------------------------------------
qMRMLItemModel::~qMRMLItemModel()
{
}

//------------------------------------------------------------------------------
void qMRMLItemModel::setMRMLScene(vtkMRMLScene* scene)
{
  qctk_d()->MRMLScene = scene;
  this->reset();
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLItemModel::mrmlScene()const
{
  return qctk_d()->MRMLScene;
}

//------------------------------------------------------------------------------
/*
void qMRMLItemModel::setTopLevelScene(bool topLevel)
{
  QCTK_D(qMRMLItemModel);
  if (d->TopLevelScene != topLevel)
    {
    if (d->MRMLScene)
      {
      emit layoutAboutToBeChanged();
      }
    d->TopLevelScene = topLevel;
    if (d->MRMLScene)
      {
      emit layoutChanged();
      }
    }
}
*/

//------------------------------------------------------------------------------
/*
bool qMRMLItemModel::topLevelScene()const
{
  return qctk_d()->TopLevelScene;
}
*/

//------------------------------------------------------------------------------
int qMRMLItemModel::columnCount(const QModelIndex & vtkNotUsed( parentVariable ) )const
{
  return 2;
}

//------------------------------------------------------------------------------
QVariant qMRMLItemModel::data(const QModelIndex &indexVariable, int role)const
{
  QCTK_D(const qMRMLItemModel);
  if (!indexVariable.isValid())
    {
    return QVariant();
    }
  Q_ASSERT(d->MRMLScene);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(indexVariable));

  Q_ASSERT(!item.isNull());
  return item->data(role);
}

//------------------------------------------------------------------------------
bool qMRMLItemModel::dropMimeData(const QMimeData *dataVariable, Qt::DropAction action, 
                                  int row, int column, const QModelIndex &parentVariable)
{
  QCTK_D(qMRMLItemModel);
  // check if the action is supported
  if (!dataVariable || !(action == Qt::MoveAction))
    {
    return false;
    }
  // check if the format is supported
  QStringList types = this->mimeTypes();
  if (types.isEmpty())      
    {
    return false;
    }
  QString format = types.at(0);
  if (!dataVariable->hasFormat(format))
    {
    return false;
    }
  int rowCountVariable = this->rowCount(parentVariable);
  if (row > rowCountVariable)
    {
    row = rowCountVariable;
    }
  if (row == -1)
    {
    row = rowCountVariable;
    }
  if (column == -1)
    {
    column = 0;
    }

  // decode and insert
  QByteArray encoded = dataVariable->data(format);
  QDataStream stream(&encoded, QIODevice::ReadOnly);
  
  QVector<int> rows, columns;
  QVector<QMap<int, QVariant> > itemDataVariable;
  while (!stream.atEnd()) 
    {
    int r, c;
    QMap<int, QVariant> v;
    stream >> r >> c >> v;
    rows.append(r);
    columns.append(c);
    itemDataVariable.append(v);
    }

  emit layoutAboutToBeChanged();

  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(parentVariable));

  bool res = false;
  QVector<QMap<int, QVariant> >::const_iterator it = itemDataVariable.begin();
  QVector<int>::const_iterator rIt = rows.begin();
  QVector<int>::const_iterator cIt = columns.begin();
  const QVector<QMap<int, QVariant> >::iterator end = itemDataVariable.end();
  for ( ;it != end ; ++it, ++rIt, ++cIt)
    {
    if (*cIt != 0)
      {
      continue;
      }
    QSharedPointer<qMRMLAbstractItemHelper> item = 
      QSharedPointer<qMRMLAbstractItemHelper>(
        d->createItemFromUID((*it)[qMRML::UIDRole], column));
    Q_ASSERT(!item.isNull());
    if (!item.isNull())
      {
      QModelIndex indexVariable = d->indexFromItem(item.data());
      if (item->canReparent(parentItem.data()) && 
          parentVariable != indexVariable.parent())
        {
        this->beginRemoveRows(indexVariable.parent(), indexVariable.row(), indexVariable.row());
        item->reparent(parentItem.data());
        this->endRemoveRows();
        }
      }
    }
  
  emit layoutChanged();
  return res;
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLItemModel::flags(const QModelIndex &indexVariable)const
{
  QCTK_D(const qMRMLItemModel);
  if (!indexVariable.isValid())
    {
    return Qt::NoItemFlags;
    }

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(indexVariable));

  Q_ASSERT(!item.isNull());
  return item->flags();
}

// Has to be reimplemented for speed issues
//------------------------------------------------------------------------------
bool qMRMLItemModel::hasChildren(const QModelIndex &parentVariable)const
{
  QCTK_D(const qMRMLItemModel);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(parentVariable));
  Q_ASSERT(!item.isNull());
  
  return item->hasChildren();
}

//------------------------------------------------------------------------------
QVariant qMRMLItemModel::headerData(int section, Qt::Orientation orientation, int role) const
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
QModelIndex qMRMLItemModel::index(int row, int column, const QModelIndex &parentVariable)const
{
  QCTK_D(const qMRMLItemModel);
  if (d->MRMLScene == 0 || row < 0 || column < 0)
    {
    return QModelIndex();
    }

  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(parentVariable));
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(parentItem->child(row, column));
  Q_ASSERT(item.isNull() || item->object());

  return !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
}

//------------------------------------------------------------------------------
//bool qMRMLItemModel::insertRows(int row, int count, const QModelIndex &parentVariable)
//{
//}

//------------------------------------------------------------------------------
QMap<int, QVariant> qMRMLItemModel::itemData(const QModelIndex &indexVariable)const
{
  QMap<int, QVariant> roles = this->QAbstractItemModel::itemData(indexVariable);
  QVariant mrmlIdData = this->data(indexVariable, qMRML::UIDRole);
  if (mrmlIdData.type() != QVariant::Invalid)
    {
    roles.insert(qMRML::UIDRole, mrmlIdData);
    }
  return roles;
}

//------------------------------------------------------------------------------
//QMimeData *qMRMLItemModel::mimeData(const QModelIndexList &indexes)const
//{
//}

//------------------------------------------------------------------------------
//QStringList qMRMLItemModel::mimeTypes()const
//{
//}

//------------------------------------------------------------------------------
QModelIndex qMRMLItemModel::parent(const QModelIndex &indexVariable)const
{
  QCTK_D(const qMRMLItemModel);
  if (!indexVariable.isValid())
    {
    return QModelIndex();
    }
  QSharedPointer<const qMRMLAbstractItemHelper> item = 
    QSharedPointer<const qMRMLAbstractItemHelper>(d->itemFromIndex(indexVariable));
  Q_ASSERT(!item.isNull());
  if (item.isNull())
    {
    return QModelIndex();
    }
  QSharedPointer<const qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<const qMRMLAbstractItemHelper>(item->parent());
  if (parentItem.isNull())
    {
    return QModelIndex();
    }
  QModelIndex parentVariable = d->indexFromItem(parentItem.data());
  return parentVariable;
}

//------------------------------------------------------------------------------
//bool qMRMLItemModel::removeColumns(int column, int count, const QModelIndex &parentVariable=QModelIndex())
//{
//}

//------------------------------------------------------------------------------
//bool qMRMLItemModel::removeRows(int row, int count, const QModelIndex &parentVariable)
//{
//}

//------------------------------------------------------------------------------
int qMRMLItemModel::rowCount(const QModelIndex &parentVariable) const
{
  QCTK_D(const qMRMLItemModel);
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(parentVariable));
  Q_ASSERT(!item.isNull());
  return !item.isNull() ? item->childCount() : 0;
}

//------------------------------------------------------------------------------
bool qMRMLItemModel::setData(const QModelIndex &indexVariable, const QVariant &value, int role)
{
  QCTK_D(const qMRMLItemModel);
  if (!indexVariable.isValid())
    {
    return false;
    }
  Q_ASSERT(qctk_d()->MRMLScene);
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(indexVariable));
  Q_ASSERT(!item.isNull());
  bool changed = !item.isNull() ? item->setData(value, role) : false;
  if (changed)
    {
    emit dataChanged(indexVariable, indexVariable);
    }
  return changed;
}

//------------------------------------------------------------------------------
//bool qMRMLItemModel::setItemData(const QModelIndex &indexVariable, const QMap<int, QVariant> &roles)
//{
//}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLItemModel::supportedDropActions()const
{
  return Qt::MoveAction;
}
