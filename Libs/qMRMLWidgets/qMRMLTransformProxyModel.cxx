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
qMRMLTransformProxyModelPrivate::qMRMLTransformProxyModelPrivate(QObject* _parent)
  :QObject(_parent)
{
  this->ObjectToRemove = 0;
}

//------------------------------------------------------------------------------
int qMRMLTransformProxyModelPrivate::actualRow(const qMRMLAbstractItemHelper* item)const
{
  Q_ASSERT(item);
  int actualRowValue = item->row();
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
      --actualRowValue;
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
      --actualRowValue;
      }
    }
  return actualRowValue;
}

//------------------------------------------------------------------------------
int qMRMLTransformProxyModelPrivate::oldRow(const qMRMLAbstractItemHelper* _parent, int row)const
{
  Q_ASSERT(_parent);
  int oldRowValue = row;
  foreach (const QSharedPointer<qMRMLAbstractItemHelper>& item, this->ItemsAboutToBeInserted)
    {
    if ((*item->parent()) == *_parent && item->column() == 0)
      {
      if (item->row() <= row)
        {
        ++oldRowValue;
        }
      }
    }
  foreach (const QSharedPointer<qMRMLAbstractItemHelper>& item, this->ItemsAboutToBeRemoved)
    {
    if (*(item->parent()) == *_parent && item->column() == 0)
      {
      if (item->row() <= row)
        {
        ++oldRowValue;
        }
      }
    }
  return oldRowValue;
}

//------------------------------------------------------------------------------
int qMRMLTransformProxyModelPrivate::actualRowCount(const qMRMLAbstractItemHelper* item)const
{
  Q_ASSERT(item);
  int actualRowCountValue = item->childCount();
  QVector<QSharedPointer<qMRMLAbstractItemHelper> >::const_iterator it;
  for(it = this->ItemsAboutToBeInserted.begin(); it != this->ItemsAboutToBeInserted.end(); ++it)
    {
    if (*(*it)->parent() == *item && (*it)->column() == item->column())
      {
      --actualRowCountValue;
      }
    }
  for(it = this->ItemsAboutToBeRemoved.begin(); it != this->ItemsAboutToBeRemoved.end(); ++it)
    {
    if (*(*it)->parent() == *item && (*it)->column() == item->column())
      {
      --actualRowCountValue;
      }
    }
  return actualRowCountValue;
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
qMRMLAbstractItemHelper* qMRMLTransformProxyModelPrivate::itemFromIndex(const QModelIndex &modelIndex)const
{
  QCTK_P(const qMRMLTransformProxyModel);
  if (modelIndex.model() == 0)
    {
    Q_ASSERT(modelIndex.model());
    return 0;
    }
  if (modelIndex.model() == p)
    {
    return this->proxyItemFromIndex(modelIndex);
    }
  else if(modelIndex.model() == p->sourceModel())
    {
    return this->sourceItemFromIndex(modelIndex);
    }
  Q_ASSERT( modelIndex.model() == p || modelIndex.model() == p->sourceModel());
  return 0;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTransformProxyModelPrivate::proxyItemFromIndex(const QModelIndex &modelIndex)const
{
  QCTK_P(const qMRMLTransformProxyModel);
  if ((modelIndex.row() < 0) || (modelIndex.column() < 0) || (modelIndex.model() != p))
    {
    return new qMRMLRootItemHelper(p->mrmlScene(), true);
    }
  vtkObject* object = 
    reinterpret_cast<vtkObject*>( modelIndex.internalPointer());
  if (!object)
    {
    qDebug() <<  modelIndex;
    }
  Q_ASSERT(object);
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLSceneItemHelper(vtkMRMLScene::SafeDownCast(object),  modelIndex.column());
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLNodeItemHelper(vtkMRMLNode::SafeDownCast(object),  modelIndex.column());
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}
//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTransformProxyModelPrivate::sourceItemFromIndex(const QModelIndex & modelIndex)const
{
  QCTK_P(const qMRMLTransformProxyModel);
  if (( modelIndex.row() < 0) || ( modelIndex.column() < 0) || ( modelIndex.model() != p->sourceModel()))
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
    reinterpret_cast<vtkObject*>( modelIndex.internalPointer());
  if (!object)
    {
    qDebug() <<  modelIndex;
    }
  Q_ASSERT(object);
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLFlatSceneItemHelper(vtkMRMLScene::SafeDownCast(object),  modelIndex.column());
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLAbstractNodeItemHelper(vtkMRMLNode::SafeDownCast(object),  modelIndex.column());
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}

//------------------------------------------------------------------------------
QVector<QSharedPointer<qMRMLAbstractItemHelper> > 
qMRMLTransformProxyModelPrivate::proxyItemsFromSourceIndexes(const QModelIndex &_parent, int start, int end) const
{
  QCTK_P(const qMRMLTransformProxyModel);
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > childrenVector;

  QSharedPointer<qMRMLAbstractItemHelper> parentSourceItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->sourceItemFromIndex(_parent));
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
      QModelIndex  modelIndex = this->indexFromItem(sourceItem.data());
      Q_ASSERT(j ==  modelIndex.column());
      QSharedPointer<qMRMLAbstractItemHelper> proxyItem =
        QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex( modelIndex));
      Q_ASSERT(j == proxyItem->column());
      childrenVector.append(proxyItem);
      }
    }
  return childrenVector;
}

//------------------------------------------------------------------------------
QVector<QSharedPointer<qMRMLAbstractItemHelper> > 
qMRMLTransformProxyModelPrivate::proxyItemsFromProxyIndexes(const QModelIndex &_parent,
  int start, int end) const
{
  QCTK_P(const qMRMLTransformProxyModel);
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > _children;

  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(_parent));
  Q_ASSERT(parentItem->childCount() > end);
  // for each row
  for (int i = start; i <= end; ++i)
    {
    // for each column
    for (int j = 0 ; j < p->sourceModel()->columnCount(); ++j)
      {
      // convert sourceItem to proxyItem;
      QSharedPointer<qMRMLAbstractItemHelper> item = 
        QSharedPointer<qMRMLAbstractItemHelper>(parentItem->child(i, j));
      Q_ASSERT(item.data());
      Q_ASSERT(j == item->column());    
      _children.append(item);
      }
    }
  return _children;
}

//------------------------------------------------------------------------------
QStack<int> 
qMRMLTransformProxyModelPrivate::consecutiveRows(const QVector<QSharedPointer<qMRMLAbstractItemHelper> >& items ) const
{
  QSharedPointer<qMRMLAbstractItemHelper> lastParentProxyItem;
  int lastRow = -1;
  QStack<int> consecutiveRowsStack;
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
      Q_ASSERT(consecutiveRowsStack.size());
      ++consecutiveRowsStack.top(); 
      }
    else
      {
      consecutiveRowsStack.push(1);
      }
    lastParentProxyItem = itemParent;
    lastRow = item->row();
    }
  Q_ASSERT(aValidItem);
  Q_ASSERT(!consecutiveRowsStack.empty());
  return consecutiveRowsStack;
}


//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceColumnsAboutToBeInserted(const QModelIndex & _parent, int start, int end)
{
  Q_UNUSED(_parent);
  Q_UNUSED(start);
  Q_UNUSED(end);
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceColumnsAboutToBeRemoved(const QModelIndex & _parent, int start, int end)
{
  Q_UNUSED(_parent);
  Q_UNUSED(start);
  Q_UNUSED(end);
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceColumnsInserted(const QModelIndex & _parent, int start, int end)
{
  Q_UNUSED(_parent);
  Q_UNUSED(start);
  Q_UNUSED(end);
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceColumnsRemoved(const QModelIndex & _parent, int start, int end)
{
  Q_UNUSED(_parent);
  Q_UNUSED(start);
  Q_UNUSED(end);
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
void qMRMLTransformProxyModelPrivate::onSourceRowsAboutToBeInserted(const QModelIndex & _parent, int start, int end)
{
  Q_UNUSED(_parent);
  Q_UNUSED(start);
  Q_UNUSED(end);
  //qDebug() << "onSourceRowsAboutToBeInserted" << parent << start << end;
  // We can't do anything here because 
  //  * we don't know where the new item will be added. 
  // ->we'll do all the process in onSourceRowsInserted
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceRowsAboutToBeRemoved(const QModelIndex & _parent, int start, int end)
{ 
  QCTK_P(qMRMLTransformProxyModel);
  //qDebug() << "onSourceRowsAboutToBeRemoved" << _parent << start << end;

  Q_ASSERT(this->ItemsAboutToBeRemoved.empty());

  QVector<QSharedPointer<qMRMLAbstractItemHelper> > itemsToRemove = 
    this->proxyItemsFromSourceIndexes(_parent, start, end);
  QStack<int> consecutiveRowsStack = this->consecutiveRows(itemsToRemove);
  Q_ASSERT_X(consecutiveRowsStack.size(), __FUNCTION__, QString("Start: %1, ").arg(start).toLatin1().data());

  // items inserted are in this->ItemsAboutToBeInserted.
  for (int i = 0 ; i < consecutiveRowsStack.count(); ++i)
    {

    QSharedPointer<qMRMLAbstractItemHelper> item = itemsToRemove.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent = 
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int proxyStart = item->row();
    int numberOfRows = consecutiveRowsStack[i];
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

  //qDebug() << "End onSourceRowsAboutToBeRemoved" << _parent << start << end;
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceRowsInserted(const QModelIndex & _parent, int start, int end)
{
  QCTK_P(qMRMLTransformProxyModel);
  Q_ASSERT(this->ItemsAboutToBeInserted.empty());

  //proxyItemsFromSourceIndexes returns also the column items
  this->ItemsAboutToBeInserted = this->proxyItemsFromSourceIndexes(_parent, start, end);
  QStack<int> consecutiveRowsStack = this->consecutiveRows(this->ItemsAboutToBeInserted);
  Q_ASSERT_X(consecutiveRowsStack.size(), __FUNCTION__, QString("Start: %1, ").arg(start).toLatin1().data());
/*
  QSharedPointer<qMRMLAbstractItemHelper> parentSourceItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->sourceItemFromIndex(_parent));
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
    
      QModelIndex  modelIndex = this->indexFromItem(sourceItem.data());
      
      proxyItem = QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex( modelIndex));
      
      if (proxyItem.isNull())
        {
        qDebug() << "no child" << parentSourceItem->row() << parentSourceItem->column() 
          <<_parent.row() << _parent.column();
        }
      else
        {
        qDebug() << proxyItem->row() << proxyItem->column();
        }
      this->ItemsAboutToBeInserted.append(proxyItem);
      }
    QSharedPointer<qMRMLAbstractItemHelper> _parent = 
      QSharedPointer<qMRMLAbstractItemHelper>(proxyItem->parent());

    if (!lastParentProxyItem.isNull() && _parent->object() == lastParentProxyItem->object() && proxyItem->row() == lastRow + 1)
      {
      ++consecutiveItems.top(); 
      }
    else
      {
      consecutiveItems.push(1);
      }
    lastParentProxyItem = _parent;
    lastRow = proxyItem->row();
    }
*/
  // items inserted are in this->ItemsAboutToBeInserted.
  QVector<int>::const_iterator it;
  for (it = consecutiveRowsStack.begin(); it != consecutiveRowsStack.end(); ++it)
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
             << "' start: "<< proxyStart << " _parent: " << itemParent->row() << " " << itemParent->column();
    */
    p->beginInsertRows(this->indexFromItem(itemParent.data()), proxyStart, proxyStart + numberOfRows - 1);

    this->ItemsAboutToBeInserted.remove(0, numberOfRows * p->sourceModel()->columnCount());

    p->endInsertRows();
    }
  Q_ASSERT(this->ItemsAboutToBeInserted.empty());
}

//------------------------------------------------------------------------------
void qMRMLTransformProxyModelPrivate::onSourceRowsRemoved(const QModelIndex & _parent, int start, int end)
{
  Q_UNUSED(_parent);
  Q_UNUSED(start);
  Q_UNUSED(end);
  //QCTK_P(qMRMLTransformProxyModel);
  this->ObjectToRemove = 0;
  this->ItemsAboutToBeRemoved.clear();
  /*
  
  
  this->ItemsAboutToBeRemoved = this->proxyItemsFromSourceIndexes(_parent, start, end);
  QStack<int> consecutiveRowsStack = this->consecutiveRows(this->ItemsAboutToBeRemoved);

  // items inserted are in this->ItemsAboutToBeInserted.
  QVector<int>::const_iterator it;
  for (it = consecutiveRowsStack.begin(); it != consecutiveRowsStack.end(); ++it)
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
qMRMLTransformProxyModel::qMRMLTransformProxyModel(QObject *_parent)
  :QAbstractProxyModel(_parent)
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
void qMRMLTransformProxyModel::setSourceModel(QAbstractItemModel * sourceModelItem)
{
  QCTK_D(qMRMLTransformProxyModel);
  Q_ASSERT_X(qobject_cast<qMRMLSceneModel*>(sourceModelItem), __FUNCTION__, "Only qMRMLSceneModels are supported");
  this->QAbstractProxyModel::setSourceModel(sourceModelItem);

  connect(sourceModelItem, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
    d, SLOT(onSourceColumnsAboutToBeInserted(const QModelIndex &, int, int)));
  connect(sourceModelItem, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
    d, SLOT(onSourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
  connect(sourceModelItem, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
    d, SLOT(onSourceColumnsInserted(const QModelIndex &, int, int)));
  connect(sourceModelItem, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
    d, SLOT(onSourceColumnsRemoved(const QModelIndex &, int, int)));
  connect(sourceModelItem, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
    d, SLOT(onSourceDataChanged(const QModelIndex &, const QModelIndex &)));
  connect(sourceModelItem, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
    d, SLOT(onSourceHeaderDataChanged(Qt::Orientation, int, int)));
  connect(sourceModelItem, SIGNAL(layoutAboutToBeChanged()),
    d, SLOT(onSourceLayoutAboutToBeChanged()));
  connect(sourceModelItem, SIGNAL(layoutChanged()),
    d, SLOT(onSourceLayoutChanged()));
  connect(sourceModelItem, SIGNAL(modelAboutToBeReset()),
    d, SLOT(onSourceModelAboutToBeReset()));
  connect(sourceModelItem, SIGNAL(modelReset()),
    d, SLOT(onSourceModelReset()));
  connect(sourceModelItem, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
    d, SLOT(onSourceRowsAboutToBeInserted(const QModelIndex &, int, int)));    
  connect(sourceModelItem, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
    d, SLOT(onSourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
  connect(sourceModelItem, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
    d, SLOT(onSourceRowsInserted(const QModelIndex &, int, int)));
  connect(sourceModelItem, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
    d, SLOT(onSourceRowsRemoved(const QModelIndex &, int, int)));
 }

//------------------------------------------------------------------------------
int qMRMLTransformProxyModel::columnCount(const QModelIndex &) const
{
  return this->sourceModel() ? this->sourceModel()->columnCount() : 0;
}

//------------------------------------------------------------------------------
QVariant qMRMLTransformProxyModel::data(const QModelIndex & modelIndex, int role)const
{
  QCTK_D(const qMRMLTransformProxyModel);
  if (! modelIndex.isValid())
    {
    return QVariant();
    }
  Q_ASSERT(this->mrmlScene());

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex( modelIndex));

  Q_ASSERT(!item.isNull());
  return item->data(role);
}


//------------------------------------------------------------------------------
bool qMRMLTransformProxyModel::dropMimeData(const QMimeData *dataValue, Qt::DropAction action, 
                                  int row, int column, const QModelIndex &_parent)
{
  QCTK_D(qMRMLTransformProxyModel);
  // check if the action is supported
  if (!dataValue || !(action == Qt::MoveAction))
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
  if (!dataValue->hasFormat(format))
    {
    return false;
    }
  int rowCountValue = this->rowCount(_parent);
  if (row > rowCountValue)
    {
    row = rowCountValue;
    }
  if (row == -1)
    {
    row = rowCountValue;
    }
  if (column == -1)
    {
    column = 0;
    }

  // decode and insert
  QByteArray encoded = dataValue->data(format);
  QDataStream stream(&encoded, QIODevice::ReadOnly);
  
  QVector<int> rows, columns;
  QVector<QMap<int, QVariant> > itemDataVector;
  while (!stream.atEnd()) 
    {
    int r, c;
    QMap<int, QVariant> v;
    stream >> r >> c >> v;
    rows.append(r);
    columns.append(c);
    itemDataVector.append(v);
    }

  //emit layoutAboutToBeChanged();

  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(_parent));

  bool res = false;
  QVector<QMap<int, QVariant> >::const_iterator it = itemDataVector.begin();
  QVector<int>::const_iterator rIt = rows.begin();
  QVector<int>::const_iterator cIt = columns.begin();
  const QVector<QMap<int, QVariant> >::iterator end = itemDataVector.end();

  for ( ;it != end ; ++it, ++rIt, ++cIt)
    {// process one by one
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
      QModelIndex oldIndex = d->indexFromItem(item.data());
      if (item->canReparent(parentItem.data()) && 
          _parent != oldIndex.parent())
        {
        this->beginRemoveRows(oldIndex.parent(), oldIndex.row(), oldIndex.row());

        item->reparent(parentItem.data());
        // the item row should be automatically updated
        QVector<QSharedPointer<qMRMLAbstractItemHelper> > newItems =
          d->proxyItemsFromProxyIndexes(_parent, item->row(), item->row());
        d->ItemsAboutToBeInserted += newItems;
        d->ObjectToRemove = item->object();

        this->endRemoveRows();
        // what's tricky here is that _parent might be invalid now. (if the
        // moved row was at the same level than the parent, it shifted up the 
        // parent). we must recompute the parent new index.
        this->beginInsertRows(d->indexFromItem(parentItem.data()), item->row(), item->row());

        d->ItemsAboutToBeInserted.remove(d->ItemsAboutToBeInserted.count() - newItems.count(), newItems.count());
        d->ObjectToRemove = 0;
        Q_ASSERT(d->ItemsAboutToBeInserted.empty());
        this->endInsertRows();
        }
      }
    }
  
  //emit layoutChanged();
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
bool qMRMLTransformProxyModel::hasChildren(const QModelIndex &_parent)const
{
  QCTK_D(const qMRMLTransformProxyModel);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(_parent));
  Q_ASSERT(!item.isNull());
  
  return d->actualRowCount(item.data());//item->hasChildren();
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTransformProxyModel::index(int row, int column, const QModelIndex &_parent)const
{
  //std::ofstream toto ("index.txt", std::ios_base::app);
  //toto<<"begin "<<this->mrmlScene() << " " << row << " " << column << std::endl;
  QCTK_D(const qMRMLTransformProxyModel);
  if (this->mrmlScene() == 0 || row < 0 || column < 0)
    {  //toto<<"end"<<std::endl;
    return QModelIndex();
    }
  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(_parent));
    //toto<<"parentItem"<<std::endl;
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(parentItem->child(row, column));
  if (!item.isNull() && d->actualRow(item.data()) != row)
    {
    //qDebug() << "shift: " << row << column << parent 
    //         << item->row() << item->column() << d->actualRow(item.data())
    //         << d->oldRow(parentItem.data(), row);
    item = QSharedPointer<qMRMLAbstractItemHelper>(
      parentItem->child(d->oldRow(parentItem.data(), row), column));
    }
    //toto<<"item"<<std::endl;
  Q_ASSERT(item.isNull() || item->object());
  
  QModelIndex i = !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
  //toto<<"end"<<std::endl;
  Q_ASSERT( d->ObjectToRemove == 0 || d->ObjectToRemove != item->object());
  return !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
}
//------------------------------------------------------------------------------
//bool qMRMLTransformProxyModel::insertRows(int row, int count, const QModelIndex &_parent)
//{
//}

//------------------------------------------------------------------------------
QMap<int, QVariant> qMRMLTransformProxyModel::itemData(const QModelIndex & modelIndex)const
{
  QMap<int, QVariant> roles = this->QAbstractItemModel::itemData( modelIndex);
  QVariant mrmlIdData = this->data( modelIndex, qMRML::UIDRole);
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
QModelIndex qMRMLTransformProxyModel::parent(const QModelIndex & modelIndex)const
{
  //std::ofstream toto ("parent.txt", std::ios_base::app);
  //toto<<"begin"<<std::endl;
  QCTK_D(const qMRMLTransformProxyModel);
  if (! modelIndex.isValid())
    {
    return QModelIndex();
    }
  QSharedPointer<const qMRMLAbstractItemHelper> item = 
    QSharedPointer<const qMRMLAbstractItemHelper>(d->proxyItemFromIndex( modelIndex));
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
  QModelIndex _parent = d->indexFromItem(parentItem.data());
  //  toto<<"end"<<std::endl;
  return _parent;
}

//------------------------------------------------------------------------------
//bool qMRMLTransformProxyModel::removeColumns(int column, int count, const QModelIndex &_parent=QModelIndex())
//{
//}

//------------------------------------------------------------------------------
//bool qMRMLTransformProxyModel::removeRows(int row, int count, const QModelIndex &_parent)
//{
//}


//------------------------------------------------------------------------------
int qMRMLTransformProxyModel::rowCount(const QModelIndex &_parent) const
{
  QCTK_D(const qMRMLTransformProxyModel);
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(_parent));
  Q_ASSERT(!item.isNull());
  return !item.isNull() ? d->actualRowCount(item.data()) : 0;
}

//------------------------------------------------------------------------------
bool qMRMLTransformProxyModel::setData(const QModelIndex & modelIndex, const QVariant &value, int role)
{
  QCTK_D(const qMRMLTransformProxyModel);
  if (! modelIndex.isValid())
    {
    return false;
    }
  Q_ASSERT(this->mrmlScene());
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex( modelIndex));
  Q_ASSERT(!item.isNull());
  bool changed = !item.isNull() ? item->setData(value, role) : false;
  if (changed)
    {
    emit dataChanged( modelIndex,  modelIndex);
    }
  return changed;
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLTransformProxyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

