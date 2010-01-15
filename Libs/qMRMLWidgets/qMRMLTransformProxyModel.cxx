#include "qMRMLItemHelper.h"
#include "qMRMLTransformProxyModel.h"
#include "qMRMLTransformProxyModel_p.h"
#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"

#include <vtkMRMLScene.h>

#include <QDebug>
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVector>

//------------------------------------------------------------------------------
qMRMLTransformProxyModelPrivate::qMRMLTransformProxyModelPrivate(QObject* parent)
  :QObject(parent)
{
  this->ObjectToRemove = 0;
}

//------------------------------------------------------------------------------
int qMRMLTransformProxyModelPrivate::actualRow(const qMRMLAbstractItemHelper* item)const
{
  Q_ASSERT(item);
  int actualRow = item->row();
  QVector<QSharedPointer<qMRMLAbstractItemHelper> >::const_iterator it;
  for(it = this->ItemsAboutToBeInserted.begin(); it != this->ItemsAboutToBeInserted.end(); ++it)
    {
    if (*item == *(*it))
      {
      return -1;
      }
    if (*(*it)->parent() == *item->parent() && 
        (*it)->column() == item->column() && 
        (*it)->row() <= item->row())
      {
      --actualRow;
      }
    }
  for(it = this->ItemsAboutToBeRemoved.begin(); it != this->ItemsAboutToBeRemoved.end(); ++it)
    {
    if (*item == *(*it))
      {
      return -1;
      }
    if (*(*it)->parent() == *item->parent() && 
        (*it)->column() == item->column() && 
        (*it)->row() <= item->row())
      {
      --actualRow;
      }
    }
  return actualRow;
}

//------------------------------------------------------------------------------
int qMRMLTransformProxyModelPrivate::oldRow(const qMRMLAbstractItemHelper* parent, int row)const
{
  Q_ASSERT(parent);
  int oldRow = row;
  foreach (const QSharedPointer<qMRMLAbstractItemHelper>& item, this->ItemsAboutToBeInserted)
    {
    if ((*item->parent()) == *parent && item->column() == 0)
      {
      if (item->row() <= row)
        {
        ++oldRow;
        }
      }
    }
  foreach (const QSharedPointer<qMRMLAbstractItemHelper>& item, this->ItemsAboutToBeRemoved)
    {
    if (*(item->parent()) == *parent && item->column() == 0)
      {
      if (item->row() <= row)
        {
        ++oldRow;
        }
      }
    }
  return oldRow;
}

//------------------------------------------------------------------------------
int qMRMLTransformProxyModelPrivate::actualRowCount(const qMRMLAbstractItemHelper* item)const
{
  Q_ASSERT(item);
  int actualRowCount = item->childCount();
  QVector<QSharedPointer<qMRMLAbstractItemHelper> >::const_iterator it;
  for(it = this->ItemsAboutToBeInserted.begin(); it != this->ItemsAboutToBeInserted.end(); ++it)
    {
    if (*(*it)->parent() == *item && (*it)->column() == item->column())
      {
      --actualRowCount;
      }
    }
  for(it = this->ItemsAboutToBeRemoved.begin(); it != this->ItemsAboutToBeRemoved.end(); ++it)
    {
    if (*(*it)->parent() == *item && (*it)->column() == item->column())
      {
      --actualRowCount;
      }
    }
  return actualRowCount;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTransformProxyModelPrivate::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  QCTK_P(const qMRMLTransformProxyModel);
  if (item == 0 || item->object() == 0)
    {
    return QModelIndex();
    }
  if (dynamic_cast<const qMRMLRootItemHelper*>(item) != 0)
    {
    return QModelIndex();
    }
  return p->createIndex(this->actualRow(item), item->column(), 
                        reinterpret_cast<void*>(item->object()));
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTransformProxyModelPrivate::createItemFromVTKObject(vtkObject* object, int column)
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
qMRMLAbstractItemHelper* qMRMLTransformProxyModelPrivate::createItemFromUID(QVariant uid, int column)
{
  QCTK_P(const qMRMLTransformProxyModel);
  Q_ASSERT(p->mrmlScene());
  if (uid.toString().isNull())
    {
    return this->createItemFromVTKObject(p->mrmlScene(), column);
    }
  Q_ASSERT(!uid.toString().isNull());
  vtkMRMLNode* node = p->mrmlScene()->GetNodeByID(uid.toString().toLatin1().data());
  return this->createItemFromVTKObject(node, column);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTransformProxyModelPrivate::proxyItemFromIndex(const QModelIndex &index)const
{
  QCTK_P(const qMRMLTransformProxyModel);
  if ((index.row() < 0) || (index.column() < 0) || (index.model() != p))
    {
    return new qMRMLRootItemHelper(p->mrmlScene(), true);
    }
  vtkObject* object = 
    reinterpret_cast<vtkObject*>(index.internalPointer());
  if (!object)
    {
    qDebug() << index;
    }
  Q_ASSERT(object);
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLSceneItemHelper(vtkMRMLScene::SafeDownCast(object), index.column());
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLNodeItemHelper(vtkMRMLNode::SafeDownCast(object), index.column());
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}
//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTransformProxyModelPrivate::sourceItemFromIndex(const QModelIndex &index)const
{
  QCTK_P(const qMRMLTransformProxyModel);
  if ((index.row() < 0) || (index.column() < 0) || (index.model() != p->sourceModel()))
    {
    if (p->sourceModel() == 0)
      {
      return 0;
      }
    qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(p->sourceModel());
    Q_ASSERT(sceneModel); // only qMRMLSceneModel is supported a source
    return new qMRMLRootItemHelper(sceneModel->mrmlScene(), true);
    }
  vtkObject* object = 
    reinterpret_cast<vtkObject*>(index.internalPointer());
  if (!object)
    {
    qDebug() << index;
    }
  Q_ASSERT(object);
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLFlatSceneItemHelper(vtkMRMLScene::SafeDownCast(object), index.column());
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLAbstractNodeItemHelper(vtkMRMLNode::SafeDownCast(object), index.column());
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}

//------------------------------------------------------------------------------
QVector<QSharedPointer<qMRMLAbstractItemHelper> > 
qMRMLTransformProxyModelPrivate::proxyItemsFromSourceIndexes(const QModelIndex &parent, int start, int end) const
{
  QCTK_P(const qMRMLTransformProxyModel);
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > children;

  QSharedPointer<qMRMLAbstractItemHelper> parentSourceItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->sourceItemFromIndex(parent));
  Q_ASSERT(parentSourceItem->childCount() > end);
  // for each row
  for (int i = start; i <= end; ++i)
    {
    // for each column
    for (int j = 0 ; j < p->sourceModel()->columnCount(); ++j)
      {
      // convert sourceItem to proxyItem;
      QSharedPointer<qMRMLAbstractItemHelper> sourceItem = 
        QSharedPointer<qMRMLAbstractItemHelper>(parentSourceItem->child(i, j));
      Q_ASSERT(sourceItem.data());    
      Q_ASSERT(j == sourceItem->column());    
      QModelIndex index = this->indexFromItem(sourceItem.data());
      Q_ASSERT(j == index.column());
      QSharedPointer<qMRMLAbstractItemHelper> proxyItem =
        QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(index));
      Q_ASSERT(j == proxyItem->column());
      children.append(proxyItem);
      }
    }
  return children;
}

//------------------------------------------------------------------------------
QStack<int> 
qMRMLTransformProxyModelPrivate::consecutiveRows(const QVector<QSharedPointer<qMRMLAbstractItemHelper> >& items ) const
{
  QSharedPointer<qMRMLAbstractItemHelper> lastParentProxyItem;
  int lastRow = -1;
  QStack<int> consecutiveRows;
  bool aValidItem = false;
  Q_ASSERT(items.size() == 2);
  foreach(const QSharedPointer<qMRMLAbstractItemHelper>& item, items)
    {
    // process only the column 0 items as we are only interested by the rows
    if (item->column() != 0)
      {
      continue;
      }
    aValidItem = true;
    QSharedPointer<qMRMLAbstractItemHelper> itemParent = 
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    if (!lastParentProxyItem.isNull() && 
        itemParent->object() == lastParentProxyItem->object() && 
        item->row() == lastRow + 1)
      {
      Q_ASSERT(consecutiveRows.size());
      ++consecutiveRows.top(); 
      }
    else
      {
      consecutiveRows.push(1);
      }
    lastParentProxyItem = itemParent;
    lastRow = item->row();
    }
  Q_ASSERT(aValidItem);
  Q_ASSERT(!consecutiveRows.empty());
  return consecutiveRows;
}


//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceColumnsAboutToBeInserted(const QModelIndex & parent, int start, int end)
{

}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceColumnsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceColumnsInserted(const QModelIndex & parent, int start, int end)
{
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceColumnsRemoved(const QModelIndex & parent, int start, int end)
{
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
  QCTK_P(qMRMLTransformProxyModel);
  QSharedPointer<qMRMLAbstractItemHelper> topLeftItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(topLeft));
  QSharedPointer<qMRMLAbstractItemHelper> bottomRightItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(bottomRight));
  //todo: probably should convert the indexes...
  emit p->dataChanged(this->indexFromItem(topLeftItem.data()), this->indexFromItem(bottomRightItem.data()));
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
  QCTK_P(qMRMLTransformProxyModel);
  //todo: probably should convert the indexes...
  emit p->headerDataChanged(orientation, first, last);
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceLayoutAboutToBeChanged()
{
  QCTK_P(qMRMLTransformProxyModel);
  emit p->layoutAboutToBeChanged();
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceLayoutChanged()
{
  QCTK_P(qMRMLTransformProxyModel);
  emit p->layoutChanged();
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceModelAboutToBeReset()
{
  QCTK_P(qMRMLTransformProxyModel);
  p->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceModelReset()
{
  QCTK_P(qMRMLTransformProxyModel);
  p->endResetModel();
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceRowsAboutToBeInserted(const QModelIndex & parent, int start, int end)
{
  //qDebug() << "onSourceRowsAboutToBeInserted" << parent << start << end;
  // We can't do anything here because 
  //  * we don't know where the new item will be added. 
  // ->we'll do all the process in onSourceRowsInserted
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceRowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{ 
  QCTK_P(qMRMLTransformProxyModel);
  //qDebug() << "onSourceRowsAboutToBeRemoved" << parent << start << end;

  Q_ASSERT(this->ItemsAboutToBeRemoved.empty());

  QVector<QSharedPointer<qMRMLAbstractItemHelper> > itemsToRemove = 
    this->proxyItemsFromSourceIndexes(parent, start, end);
  QStack<int> consecutiveRows = this->consecutiveRows(itemsToRemove);
  Q_ASSERT_X(consecutiveRows.size(), __FUNCTION__, QString("Start: %1, ").arg(start).toLatin1().data());

  // items inserted are in this->ItemsAboutToBeInserted.
  for (int i = 0 ; i < consecutiveRows.count(); ++i)
    {

    QSharedPointer<qMRMLAbstractItemHelper> item = itemsToRemove.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent = 
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int proxyStart = item->row();
    int numberOfRows = consecutiveRows[i];
    int numberOfItems = numberOfRows * p->sourceModel()->columnCount();
    /*
    qDebug() << "****Remove:" << proxyStart << proxyStart + numberOfRows - 1 << item->object() 
             << item->data().toString().toLatin1().data() << vtkMRMLNode::SafeDownCast(item->object())->GetID();
    */
    p->beginRemoveRows(this->indexFromItem(itemParent.data()), proxyStart, proxyStart + numberOfRows - 1);

    this->ObjectToRemove = item->object();
    this->ItemsAboutToBeRemoved += itemsToRemove.mid(0, numberOfItems);
    itemsToRemove.remove(0, numberOfItems);

    p->endRemoveRows();
    }

  //qDebug() << "End onSourceRowsAboutToBeRemoved" << parent << start << end;
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceRowsInserted(const QModelIndex & parent, int start, int end)
{
  QCTK_P(qMRMLTransformProxyModel);
  Q_ASSERT(this->ItemsAboutToBeInserted.empty());

  //proxyItemsFromSourceIndexes returns also the column items
  this->ItemsAboutToBeInserted = this->proxyItemsFromSourceIndexes(parent, start, end);
  QStack<int> consecutiveRows = this->consecutiveRows(this->ItemsAboutToBeInserted);
  Q_ASSERT_X(consecutiveRows.size(), __FUNCTION__, QString("Start: %1, ").arg(start).toLatin1().data());
/*
  QSharedPointer<qMRMLAbstractItemHelper> parentSourceItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->sourceItemFromIndex(parent));
  QSharedPointer<qMRMLAbstractItemHelper> lastParentProxyItem;
  int lastRow = -1;
  QStack<int> consecutiveItems;
  //store the indexes to be inserted
  for (int i = start; i <= end; ++i)
    {
    QSharedPointer<qMRMLAbstractItemHelper> proxyItem; 
    for (int j = 0 ; j < p->sourceModel()->columnCount(); ++j)
      {
      QSharedPointer<qMRMLAbstractItemHelper> sourceItem = 
        QSharedPointer<qMRMLAbstractItemHelper>(parentSourceItem->child(i, j));
    
      QModelIndex index = this->indexFromItem(sourceItem.data());
      
      proxyItem = QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(index));
      
      if (proxyItem.isNull())
        {
        qDebug() << "no child" << parentSourceItem->row() << parentSourceItem->column() 
          <<parent.row() << parent.column();
        }
      else
        {
        qDebug() << proxyItem->row() << proxyItem->column();
        }
      this->ItemsAboutToBeInserted.append(proxyItem);
      }
    QSharedPointer<qMRMLAbstractItemHelper> parent = 
      QSharedPointer<qMRMLAbstractItemHelper>(proxyItem->parent());

    if (!lastParentProxyItem.isNull() && parent->object() == lastParentProxyItem->object() && proxyItem->row() == lastRow + 1)
      {
      ++consecutiveItems.top(); 
      }
    else
      {
      consecutiveItems.push(1);
      }
    lastParentProxyItem = parent;
    lastRow = proxyItem->row();
    }
*/
  // items inserted are in this->ItemsAboutToBeInserted.
  QVector<int>::const_iterator it;
  for (it = consecutiveRows.begin(); it != consecutiveRows.end(); ++it)
    {
    Q_ASSERT(!this->ItemsAboutToBeInserted.empty());

    QSharedPointer<qMRMLAbstractItemHelper> item = this->ItemsAboutToBeInserted.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent = 
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int proxyStart = item->row();
    int numberOfRows = *it;
    /*
    qDebug() << "nbofrows:" << numberOfRows << " '"
             << item->data().toString().toLatin1().data() << vtkMRMLNode::SafeDownCast(item->object())->GetID()
             << "' start: "<< proxyStart << " parent: " << itemParent->row() << " " << itemParent->column();
    */
    p->beginInsertRows(this->indexFromItem(itemParent.data()), proxyStart, proxyStart + numberOfRows - 1);

    this->ItemsAboutToBeInserted.remove(0, numberOfRows * p->sourceModel()->columnCount());

    p->endInsertRows();
    }
  Q_ASSERT(this->ItemsAboutToBeInserted.empty());
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceRowsRemoved(const QModelIndex & parent, int start, int end)
{
  //QCTK_P(qMRMLTransformProxyModel);
  this->ObjectToRemove = 0;
  this->ItemsAboutToBeRemoved.clear();
  /*
  
  
  this->ItemsAboutToBeRemoved = this->proxyItemsFromSourceIndexes(parent, start, end);
  QStack<int> consecutiveRows = this->consecutiveRows(this->ItemsAboutToBeRemoved);

  // items inserted are in this->ItemsAboutToBeInserted.
  QVector<int>::const_iterator it;
  for (it = consecutiveRows.begin(); it != consecutiveRows.end(); ++it)
    {
    Q_ASSERT(!this->ItemsAboutToBeRemoved.empty());

    QSharedPointer<qMRMLAbstractItemHelper> item = this->ItemsAboutToBeRemoved.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent = 
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int proxyStart = item->row();
    int numberOfRows = *it;

    p->beginRemoveRows(this->indexFromItem(itemParent.data()), proxyStart, proxyStart + numberOfRows - 1);

    this->ItemsAboutToBeRemoved.remove(0, numberOfRows * p->sourceModel()->columnCount());

    p->endRemoveRows();
    }
  Q_ASSERT(this->ItemsAboutToBeRemoved.empty());
  */
}

//------------------------------------------------------------------------------
qMRMLTransformProxyModel::qMRMLTransformProxyModel(QObject *parent)
  :QAbstractProxyModel(parent)
{
  QCTK_INIT_PRIVATE(qMRMLTransformProxyModel);
}

//------------------------------------------------------------------------------
qMRMLTransformProxyModel::~qMRMLTransformProxyModel()
{
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLTransformProxyModel::mrmlScene()const
{
  if (this->sourceModel() == 0)
    {
    return 0;
    }
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());    
  Q_ASSERT(sceneModel); // only qMRMLSceneModel is supported as a source
  if (sceneModel == 0)
    {
    return 0;
    }
  return sceneModel->mrmlScene();
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModel::setSourceModel(QAbstractItemModel * sourceModel)
{
  QCTK_D(qMRMLTransformProxyModel);
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(sourceModel);    
  Q_ASSERT(sceneModel); // only qMRMLSceneModel is supported as a source
  this->QAbstractProxyModel::setSourceModel(sourceModel);

  connect(sourceModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
    d, SLOT(onSourceColumnsAboutToBeInserted(const QModelIndex &, int, int)));
  connect(sourceModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
    d, SLOT(onSourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
  connect(sourceModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
    d, SLOT(onSourceColumnsInserted(const QModelIndex &, int, int)));
  connect(sourceModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
    d, SLOT(onSourceColumnsRemoved(const QModelIndex &, int, int)));
  connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
    d, SLOT(onSourceDataChanged(const QModelIndex &, const QModelIndex &)));
  connect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
    d, SLOT(onSourceHeaderDataChanged(Qt::Orientation, int, int)));
  connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
    d, SLOT(onSourceLayoutAboutToBeChanged()));
  connect(sourceModel, SIGNAL(layoutChanged()),
    d, SLOT(onSourceLayoutChanged()));
  connect(sourceModel, SIGNAL(modelAboutToBeReset()),
    d, SLOT(onSourceModelAboutToBeReset()));
  connect(sourceModel, SIGNAL(modelReset()),
    d, SLOT(onSourceModelReset()));
  connect(sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
    d, SLOT(onSourceRowsAboutToBeInserted(const QModelIndex &, int, int)));    
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
    d, SLOT(onSourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
    d, SLOT(onSourceRowsInserted(const QModelIndex &, int, int)));
  connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
    d, SLOT(onSourceRowsRemoved(const QModelIndex &, int, int)));
 }

//------------------------------------------------------------------------------
int qMRMLTransformProxyModel::columnCount(const QModelIndex &) const
{
  return this->sourceModel() ? this->sourceModel()->columnCount() : 0;
}

//------------------------------------------------------------------------------
QVariant qMRMLTransformProxyModel::data(const QModelIndex &index, int role)const
{
  QCTK_D(const qMRMLTransformProxyModel);
  if (!index.isValid())
    {
    return QVariant();
    }
  Q_ASSERT(this->mrmlScene());

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(index));

  Q_ASSERT(!item.isNull());
  return item->data(role);
}


//------------------------------------------------------------------------------
bool qMRMLTransformProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, 
                                  int row, int column, const QModelIndex &parent)
{
  QCTK_D(qMRMLTransformProxyModel);
  // check if the action is supported
  if (!data || !(action == Qt::MoveAction))
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
  if (!data->hasFormat(format))
    {
    return false;
    }
  int rowCount = this->rowCount(parent);
  if (row > rowCount)
    {
    row = rowCount;
    }
  if (row == -1)
    {
    row = rowCount;
    }
  if (column == -1)
    {
    column = 0;
    }

  // decode and insert
  QByteArray encoded = data->data(format);
  QDataStream stream(&encoded, QIODevice::ReadOnly);
  
  QVector<int> rows, columns;
  QVector<QMap<int, QVariant> > itemData;
  while (!stream.atEnd()) 
    {
    int r, c;
    QMap<int, QVariant> v;
    stream >> r >> c >> v;
    rows.append(r);
    columns.append(c);
    itemData.append(v);
    }

  emit layoutAboutToBeChanged();

  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(parent));

  bool res = false;
  QVector<QMap<int, QVariant> >::const_iterator it = itemData.begin();
  QVector<int>::const_iterator rIt = rows.begin();
  QVector<int>::const_iterator cIt = columns.begin();
  const QVector<QMap<int, QVariant> >::iterator end = itemData.end();
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
      QModelIndex index = d->indexFromItem(item.data());
      if (item->canReparent(parentItem.data()) && 
          parent != index.parent())
        {
        this->beginRemoveRows(index.parent(), index.row(), index.row());
        item->reparent(parentItem.data());
        this->endRemoveRows();
        }
      }
    }
  
  emit layoutChanged();
  return res;
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLTransformProxyModel::flags(const QModelIndex &proxyIndex)const
{
  QCTK_D(const qMRMLTransformProxyModel);
  if (!proxyIndex.isValid())
    {
    return Qt::NoItemFlags;
    }

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(proxyIndex));

  Q_ASSERT(!item.isNull());
  return item->flags();
}

// Has to be reimplemented for speed issues
//------------------------------------------------------------------------------
bool qMRMLTransformProxyModel::hasChildren(const QModelIndex &parent)const
{
  QCTK_D(const qMRMLTransformProxyModel);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(parent));
  Q_ASSERT(!item.isNull());
  
  return d->actualRowCount(item.data());//item->hasChildren();
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTransformProxyModel::index(int row, int column, const QModelIndex &parent)const
{
  //std::ofstream toto ("index.txt", std::ios_base::app);
  //toto<<"begin "<<this->mrmlScene() << " " << row << " " << column << std::endl;
  QCTK_D(const qMRMLTransformProxyModel);
  if (this->mrmlScene() == 0 || row < 0 || column < 0)
    {  //toto<<"end"<<std::endl;
    return QModelIndex();
    }
  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(parent));
    //toto<<"parentItem"<<std::endl;
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(parentItem->child(row, column));
  if (!item.isNull() && d->actualRow(item.data()) != row)
    {
    item = QSharedPointer<qMRMLAbstractItemHelper>(
      parentItem->child(d->oldRow(parentItem.data(), row), column));
    }
    //toto<<"item"<<std::endl;
  Q_ASSERT(item.isNull() || item->object());
  
  QModelIndex i = !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
  //toto<<"end"<<std::endl;
  Q_ASSERT( d->ObjectToRemove != item->object());
  return !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
}
//------------------------------------------------------------------------------
//bool qMRMLTransformProxyModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//}

//------------------------------------------------------------------------------
QMap<int, QVariant> qMRMLTransformProxyModel::itemData(const QModelIndex &index)const
{
  QMap<int, QVariant> roles = this->QAbstractItemModel::itemData(index);
  QVariant mrmlIdData = this->data(index, qMRML::UIDRole);
  if (mrmlIdData.type() != QVariant::Invalid)
    {
    roles.insert(qMRML::UIDRole, mrmlIdData);
    }
  return roles;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTransformProxyModel::mapFromSource(const QModelIndex &sourceIndex)const
{
  QCTK_D(const qMRMLTransformProxyModel);
  return d->indexFromItem(d->proxyItemFromIndex(sourceIndex));
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTransformProxyModel::mapToSource(const QModelIndex &proxyIndex)const
{
  QCTK_D(const qMRMLTransformProxyModel);
  return d->indexFromItem(d->sourceItemFromIndex(proxyIndex));
}

//------------------------------------------------------------------------------
//QMimeData *qMRMLTransformProxyModel::mimeData(const QModelIndexList &indexes)const
//{
//}

//------------------------------------------------------------------------------
//QStringList qMRMLTransformProxyModel::mimeTypes()const
//{
//}


//------------------------------------------------------------------------------
QModelIndex qMRMLTransformProxyModel::parent(const QModelIndex &index)const
{
  //std::ofstream toto ("parent.txt", std::ios_base::app);
  //toto<<"begin"<<std::endl;
  QCTK_D(const qMRMLTransformProxyModel);
  if (!index.isValid())
    {
    return QModelIndex();
    }
  QSharedPointer<const qMRMLAbstractItemHelper> item = 
    QSharedPointer<const qMRMLAbstractItemHelper>(d->proxyItemFromIndex(index));
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
  QModelIndex parent = d->indexFromItem(parentItem.data());
  //  toto<<"end"<<std::endl;
  return parent;
}

//------------------------------------------------------------------------------
//bool qMRMLTransformProxyModel::removeColumns(int column, int count, const QModelIndex &parent=QModelIndex())
//{
//}

//------------------------------------------------------------------------------
//bool qMRMLTransformProxyModel::removeRows(int row, int count, const QModelIndex &parent)
//{
//}


//------------------------------------------------------------------------------
int qMRMLTransformProxyModel::rowCount(const QModelIndex &parent) const
{
  QCTK_D(const qMRMLTransformProxyModel);
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(parent));
  Q_ASSERT(!item.isNull());
  return !item.isNull() ? d->actualRowCount(item.data()) : 0;
}

//------------------------------------------------------------------------------
bool qMRMLTransformProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  QCTK_D(const qMRMLTransformProxyModel);
  if (!index.isValid())
    {
    return false;
    }
  Q_ASSERT(this->mrmlScene());
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(index));
  Q_ASSERT(!item.isNull());
  bool changed = !item.isNull() ? item->setData(value, role) : false;
  if (changed)
    {
    emit dataChanged(index, index);
    }
  return changed;
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLTransformProxyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

