// Qt includes
#include <QDebug>
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVector>

// qMRML includes
#include "qMRMLItemHelper.h"
#include "qMRMLTreeProxyModel.h"
#include "qMRMLTreeProxyModel_p.h"
#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>

//------------------------------------------------------------------------------
qMRMLTreeProxyModelPrivate::qMRMLTreeProxyModelPrivate(QObject* vparent)
  :QObject(vparent)
{
#ifndef QT_NO_DEBUG
  this->HiddenVTKObject = 0;
#endif
}

//------------------------------------------------------------------------------
int qMRMLTreeProxyModelPrivate::rowWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const
{
  Q_ASSERT(item);
  int rowWithHiddenItemsRemovedValue = item->row();
  foreach( const QSharedPointer<qMRMLAbstractItemHelper>& hiddenItem, this->HiddenItems)
    {
    if (*item == *hiddenItem)
      {
      return -1;
      }
    if ((*hiddenItem->parent()) == *item->parent() && 
        hiddenItem->column() == item->column() && 
        hiddenItem->row() <= item->row())
      {
      --rowWithHiddenItemsRemovedValue;
      }
    }
  return rowWithHiddenItemsRemovedValue;
}

//------------------------------------------------------------------------------
int qMRMLTreeProxyModelPrivate::childRowWithHiddenItemsAdded(const qMRMLAbstractItemHelper* vparent, int row)const
{
  Q_ASSERT(vparent);
  int childRowWithHiddenItemsAddedValue = row;
  foreach (const QSharedPointer<qMRMLAbstractItemHelper>& hiddenItem, this->HiddenItems)
    {
    if ((*hiddenItem->parent()) == *vparent
        && hiddenItem->column() == 0)
      {
      if (hiddenItem->row() <= row)
        {
        ++childRowWithHiddenItemsAddedValue;
        }
      }
    }
  return childRowWithHiddenItemsAddedValue;
}

//------------------------------------------------------------------------------
int qMRMLTreeProxyModelPrivate::rowCountWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const
{
  Q_ASSERT(item);
  int rowCountWithHiddenItemsRemovedValue = item->childCount();
  foreach (const QSharedPointer<qMRMLAbstractItemHelper>& hiddenItem, this->HiddenItems)
    {
    if ((*hiddenItem->parent()) == *item
        && hiddenItem->column() == item->column())
      {
      --rowCountWithHiddenItemsRemovedValue;
      }
    }
  return rowCountWithHiddenItemsRemovedValue;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTreeProxyModelPrivate::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  CTK_P(const qMRMLTreeProxyModel);
  if (item == 0 || item->object() == 0)
    {
    return QModelIndex();
    }
  if (dynamic_cast<const qMRMLRootItemHelper*>(item) != 0)
    {
    return QModelIndex();
    }
  return p->createIndex(this->rowWithHiddenItemsRemoved(item), item->column(), 
                        reinterpret_cast<void*>(item->object()));
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTreeProxyModelPrivate::itemFromUID(QVariant uid, int column)
{
  CTK_P(const qMRMLTreeProxyModel);
  Q_ASSERT(p->mrmlScene());
  if (uid.toString().isNull())
    {
    return p->itemFactory()->createItem(p->mrmlScene(), column);
    }
  Q_ASSERT(!uid.toString().isNull());
  vtkMRMLNode* node = p->mrmlScene()->GetNodeByID(uid.toString().toLatin1().data());
  return p->itemFactory()->createItem(node, column);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTreeProxyModelPrivate::itemFromIndex(const QModelIndex &modelIndex)const
{
  CTK_P(const qMRMLTreeProxyModel);
  if (modelIndex.model() == 0)
    {
    // There is no way to know for sure where the modelIndex comes from. If you
    // know, then use proxyItemFromIndex or sourceItemFromIndex directly
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
qMRMLAbstractItemHelper* qMRMLTreeProxyModelPrivate::proxyItemFromIndex(const QModelIndex &modelIndex)const
{
  CTK_P(const qMRMLTreeProxyModel);
  if ((modelIndex.row() < 0) || (modelIndex.column() < 0) || (modelIndex.model() != p))
    {
    return p->itemFactory()->createRootItem(p->mrmlScene());
    }
  vtkObject* object = 
    reinterpret_cast<vtkObject*>( modelIndex.internalPointer());
  return p->itemFactory()->createItem(object, modelIndex.column());
  /*
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
  */
}
//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTreeProxyModelPrivate::sourceItemFromIndex(const QModelIndex & modelIndex)const
{
  CTK_P(const qMRMLTreeProxyModel);
  if (( modelIndex.row() < 0) || ( modelIndex.column() < 0) || ( modelIndex.model() != p->sourceModel()))
    {
    if (p->sourceModel() == 0)
      {
      return 0;
      }
    qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(p->sourceModel());
    Q_ASSERT(sceneModel); // only qMRMLSceneModel is supported a source
    return p->itemFactory()->createRootItem(sceneModel->mrmlScene());
    }
  vtkObject* object = 
    reinterpret_cast<vtkObject*>( modelIndex.internalPointer());
  return p->sourceItemFactory()->createItem(object, modelIndex.column());
}
/*
  return this->sourceItemFromObject(object, modelIndex.column());
  
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
    return new qMRMLFlatNodeItemHelper(vtkMRMLNode::SafeDownCast(object),  modelIndex.column());
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
  
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTreeProxyModelPrivate::sourceItemFromObject(vtkObject* object, int column)const
{
  CTK_P(const qMRMLTreeProxyModel);
  
  if (!object)
    {
    Q_ASSERT(object);
    return 0;
    }
  
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLFlatSceneItemHelper(vtkMRMLScene::SafeDownCast(object),  column);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLFlatNodeItemHelper(vtkMRMLNode::SafeDownCast(object),  column);
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}
*/

//------------------------------------------------------------------------------
QVector<QSharedPointer<qMRMLAbstractItemHelper> > 
qMRMLTreeProxyModelPrivate::proxyItemsFromSourceIndexes(const QModelIndex &vparent, int start, int end) const
{
  CTK_P(const qMRMLTreeProxyModel);
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > childrenVector;

  QSharedPointer<qMRMLAbstractItemHelper> parentSourceItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->sourceItemFromIndex(vparent));
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
qMRMLTreeProxyModelPrivate::proxyItemsFromProxyIndexes(const QModelIndex &_parent,
                                                       int start, int end) const
{
  CTK_P(const qMRMLTreeProxyModel);
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
qMRMLTreeProxyModelPrivate::consecutiveRows(const QVector<QSharedPointer<qMRMLAbstractItemHelper> >& items ) const
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
void qMRMLTreeProxyModelPrivate::onSourceColumnsAboutToBeInserted(const QModelIndex & vparent, int start, int end)
{
  Q_UNUSED(vparent);
  Q_UNUSED(start);
  Q_UNUSED(end);
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceColumnsAboutToBeRemoved(const QModelIndex & vparent, int start, int end)
{
  Q_UNUSED(vparent);
  Q_UNUSED(start);
  Q_UNUSED(end);
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceColumnsInserted(const QModelIndex & vparent, int start, int end)
{
  Q_UNUSED(vparent);
  Q_UNUSED(start);
  Q_UNUSED(end);
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceColumnsRemoved(const QModelIndex & vparent, int start, int end)
{
  Q_UNUSED(vparent);
  Q_UNUSED(start);
  Q_UNUSED(end);
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
  CTK_P(qMRMLTreeProxyModel);
  QSharedPointer<qMRMLAbstractItemHelper> topLeftItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(topLeft));
  QSharedPointer<qMRMLAbstractItemHelper> bottomRightItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(bottomRight));
  //todo: probably should convert the indexes...
  emit p->dataChanged(this->indexFromItem(topLeftItem.data()), this->indexFromItem(bottomRightItem.data()));
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
  CTK_P(qMRMLTreeProxyModel);
  //todo: probably should convert the indexes...
  emit p->headerDataChanged(orientation, first, last);
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceLayoutAboutToBeChanged()
{
  CTK_P(qMRMLTreeProxyModel);
  emit p->layoutAboutToBeChanged();
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceLayoutChanged()
{
  CTK_P(qMRMLTreeProxyModel);
  emit p->layoutChanged();
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceModelAboutToBeReset()
{
  CTK_P(qMRMLTreeProxyModel);
  p->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceModelReset()
{
  CTK_P(qMRMLTreeProxyModel);
  p->endResetModel();
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceRowsAboutToBeInserted(const QModelIndex & vparent, int start, int end)
{
  Q_UNUSED(vparent);
  Q_UNUSED(start);
  Q_UNUSED(end);
  //qDebug() << "onSourceRowsAboutToBeInserted" << parent << start << end;
  // We can't do anything here because 
  //  * we don't know where the new item will be added. 
  // ->we'll do all the process in onSourceRowsInserted
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceRowsAboutToBeRemoved(const QModelIndex & vparent, int start, int end)
{ 
  CTK_P(qMRMLTreeProxyModel);
  // we test if it's empty here, but it's just because I never needed nested calls
  // to rows about to be removed/added. It can be removed if needed
  Q_ASSERT(this->HiddenItems.empty());

  QVector<QSharedPointer<qMRMLAbstractItemHelper> > itemsToRemove = 
    this->proxyItemsFromSourceIndexes(vparent, start, end);
  // We can process hidden items by bulk if they are consecutive. 
  // While start/end cover consecutive items, they might not all be consecutive 
  // if there are hidden items in the range.
  QStack<int> consecutiveRowsToBeRemoved = this->consecutiveRows(itemsToRemove);
  Q_ASSERT_X(consecutiveRowsToBeRemoved.size(), __FUNCTION__,QString("Start: %1, ").arg(start).toLatin1().data());

  // for each consecutive items
  //for (int i = 0 ; i < consecutiveRowsToBeRemoved.count(); ++i)
  foreach (int numberOfRows, consecutiveRowsToBeRemoved)
    {
    // get the first item to remove from the consecutive bulk
    QSharedPointer<qMRMLAbstractItemHelper> item = itemsToRemove.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent = 
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int proxyStart = item->row();
    //int numberOfRows = consecutiveRowsToBeRemoved[i];
    // proxyItemsFromSourceIndexes returned items for each column (not just 1 
    // per row), here we compute the total number of items.
    int numberOfItems = numberOfRows * p->sourceModel()->columnCount();
    /*
    qDebug() << "****Remove:" << proxyStart << proxyStart + numberOfRows - 1 << item->object() 
             << item->data().toString().toLatin1().data() << vtkMRMLNode::SafeDownCast(item->object())->GetID();
    */
    // send the Qt events for the qAbstractItemModel
    // the item to remove doesn't have to be hidden yet, it's valid to have it 
    // in the tree
    p->beginRemoveRows(this->indexFromItem(itemParent.data()), proxyStart, proxyStart + numberOfRows - 1);
    // now we fake that the item is removed from the model
    this->HiddenItems += itemsToRemove.mid(0, numberOfItems);
#ifndef QT_NO_DEBUG
    this->HiddenVTKObject = item->object();
#endif
    itemsToRemove.remove(0, numberOfItems);
    // here we call the function associated to beginRemoveRows to pretend that the 
    // items have been removed from the tree. They aren't exactly removed, just
    // hidden for the momement. They will be truly removed when 
    // onSourceRowsRemoved() will be called. Until then, we hide the items
    p->endRemoveRows();
    }
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceRowsInserted(const QModelIndex & vparent, int start, int end)
{
  CTK_P(qMRMLTreeProxyModel);
  // we test if it's empty here, but it's just because I never needed nested calls
  // to rows about to be removed/added. It can be removed if needed
  Q_ASSERT(this->HiddenItems.empty());

  //proxyItemsFromSourceIndexes returns also the column items
  this->HiddenItems = this->proxyItemsFromSourceIndexes(vparent, start, end) + this->HiddenItems;
  QStack<int> consecutiveRowsToInsert = this->consecutiveRows(this->HiddenItems);
  Q_ASSERT_X(consecutiveRowsToInsert.size(), __FUNCTION__, QString("Start: %1, ").arg(start).toLatin1().data());
  // items inserted are in this->HiddenItesm.
  //QVector<int>::const_iterator it;
  //for (it = consecutiveRowsStack.begin(); it != consecutiveRowsStack.end(); ++it)
  foreach(int numberOfRows, consecutiveRowsToInsert)
    {
    Q_ASSERT(!this->HiddenItems.empty());

    QSharedPointer<qMRMLAbstractItemHelper> item = this->HiddenItems.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent = 
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int proxyStart = item->row();
    //int numberOfRows = *it;
    /*
    qDebug() << "nbofrows:" << numberOfRows << " '"
             << item->data().toString().toLatin1().data() << vtkMRMLNode::SafeDownCast(item->object())->GetID()
             << "' start: "<< proxyStart << " vparent: " << itemParent->row() << " " << itemParent->column();
    */
    p->beginInsertRows(this->indexFromItem(itemParent.data()), proxyStart, proxyStart + numberOfRows - 1);

    this->HiddenItems.remove(0, numberOfRows * p->sourceModel()->columnCount());

    p->endInsertRows();
    }
  Q_ASSERT(this->HiddenItems.empty());
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModelPrivate::onSourceRowsRemoved(const QModelIndex & vparent, int start, int end)
{
  Q_UNUSED(vparent);
  Q_UNUSED(start);
  Q_UNUSED(end);
  //CTK_P(qMRMLTreeProxyModel);
#ifndef QT_NO_DEBUG
  this->HiddenVTKObject = 0;
#endif
  // we should probably not clear everything (in case if insertion/removes are 
  // nested)
  this->HiddenItems.clear(); 
}
//------------------------------------------------------------------------------
// qMRMLTreeProxyModel

//------------------------------------------------------------------------------
qMRMLTreeProxyModel::qMRMLTreeProxyModel(QObject *vparent)
  :QAbstractProxyModel(vparent)
{
  CTK_INIT_PRIVATE(qMRMLTreeProxyModel);
}

//------------------------------------------------------------------------------
qMRMLTreeProxyModel::~qMRMLTreeProxyModel()
{
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLTreeProxyModel::mrmlScene()const
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
vtkMRMLNode* qMRMLTreeProxyModel::mrmlNode(const QModelIndex &nodeIndex)const
{
  CTK_D(const qMRMLTreeProxyModel);
  if (!nodeIndex.isValid())
    {
    return 0;
    }

  QSharedPointer<qMRMLAbstractItemHelper> item =
    QSharedPointer<qMRMLAbstractItemHelper>(d->itemFromIndex(nodeIndex));
  Q_ASSERT(!item.isNull());

  return vtkMRMLNode::SafeDownCast(item->object());
}

//------------------------------------------------------------------------------
void qMRMLTreeProxyModel::setSourceModel(QAbstractItemModel * sourceModelItem)
{
  CTK_D(qMRMLTreeProxyModel);
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
int qMRMLTreeProxyModel::columnCount(const QModelIndex &) const
{
  return this->sourceModel() ? this->sourceModel()->columnCount() : 0;
}

//------------------------------------------------------------------------------
QVariant qMRMLTreeProxyModel::data(const QModelIndex & modelIndex, int role)const
{
  CTK_D(const qMRMLTreeProxyModel);
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
bool qMRMLTreeProxyModel::dropMimeData(const QMimeData *dataValue, Qt::DropAction action, 
                                  int row, int column, const QModelIndex &vparent)
{
  CTK_D(qMRMLTreeProxyModel);
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
  int rowCountValue = this->rowCount(vparent);
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
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(vparent));

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
        d->itemFromUID((*it)[qMRML::UIDRole], column));
    Q_ASSERT(!item.isNull());
    if (!item.isNull())
      {
      QModelIndex oldIndex = d->indexFromItem(item.data());
      if (item->canReparent(parentItem.data()) && 
          vparent != oldIndex.parent())
        {
        this->beginRemoveRows(oldIndex.parent(), oldIndex.row(), oldIndex.row());

        item->reparent(parentItem.data());
        // the item row should be automatically updated
        QVector<QSharedPointer<qMRMLAbstractItemHelper> > newItems =
          d->proxyItemsFromProxyIndexes(vparent, item->row(), item->row());
        d->HiddenItems += newItems;
#ifndef QT_NO_DEBUG
        d->HiddenVTKObject= item->object();
#endif

        this->endRemoveRows();
        // what's tricky here is that vparent might be invalid now. (if the 
        // moved row was at the same level than the parent, it shifted up the 
        // parent). we must recompute the parent new index.
        this->beginInsertRows(d->indexFromItem(parentItem.data()), item->row(), item->row());

        d->HiddenItems.remove(d->HiddenItems.count() - newItems.count(), newItems.count());
#ifndef QT_NO_DEBUG
        d->HiddenVTKObject= 0;
#endif
        Q_ASSERT(d->HiddenItems.empty());
        this->endInsertRows();
        }
      }
    }
  
  //emit layoutChanged();
  return res;
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLTreeProxyModel::flags(const QModelIndex &proxyIndex)const
{
  CTK_D(const qMRMLTreeProxyModel);
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
bool qMRMLTreeProxyModel::hasChildren(const QModelIndex &vparent)const
{
  CTK_D(const qMRMLTreeProxyModel);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(vparent));
  Q_ASSERT(!item.isNull());
  
  return d->rowCountWithHiddenItemsRemoved(item.data());//item->hasChildren();
}

//------------------------------------------------------------------------------
QVariant qMRMLTreeProxyModel::headerData(int section, Qt::Orientation orientation, int role)const
{
  Q_ASSERT(this->sourceModel());
  // QAbstractProxyModel doesn't work as it tries to map an index into the
  // source model. It can fail because the model can be empty... Here we know
  // that the section doesn't change, so let's use it directly.
  return this->sourceModel()->headerData(section, orientation, role);
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTreeProxyModel::index(int row, int column, const QModelIndex &vparent)const
{
  CTK_D(const qMRMLTreeProxyModel);
  // sanity check before going any further.
  if (this->mrmlScene() == 0 || row < 0 || column < 0)
    {
    return QModelIndex();
    }
  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(vparent));
    
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(parentItem->child(row, column));
    
  // Let's make sure that the item we got has the required row
  if (!item.isNull() && d->rowWithHiddenItemsRemoved(item.data()) != row)
    {
    // item has a different row because there are hidden items that shift 
    // item's row. The item corresponding to the required row is after some 
    // hidden items. childRowWithHiddenItemsAdded will get the row if the 
    // hidden items were not hidden. 
    item = QSharedPointer<qMRMLAbstractItemHelper>(
      parentItem->child(d->childRowWithHiddenItemsAdded(parentItem.data(), row), column));
    }
  Q_ASSERT(item.isNull() || item->object());
  
  QModelIndex i = !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
#ifndef QT_NO_DEBUG
  Q_ASSERT( d->HiddenVTKObject == 0 || d->HiddenVTKObject != item->object());
#endif
  return !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
}
//------------------------------------------------------------------------------
//bool qMRMLTreeProxyModel::insertRows(int row, int count, const QModelIndex &vparent)
//{
//}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTreeProxyModel::item(const QModelIndex &modelIndex)const
{
  CTK_D(const qMRMLTreeProxyModel);
  return d->itemFromIndex(modelIndex);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTreeProxyModel::proxyItem(const QModelIndex &modelIndex)const
{
  CTK_D(const qMRMLTreeProxyModel);
  return d->proxyItemFromIndex(modelIndex);
}

//------------------------------------------------------------------------------
QMap<int, QVariant> qMRMLTreeProxyModel::itemData(const QModelIndex & modelIndex)const
{
  QMap<int, QVariant> roles = this->QAbstractItemModel::itemData(modelIndex);
  // In order to have the drag/drop working without defining our own MIME type
  // we need to add some way of uniquely identify an item. Here it's done 
  // by adding a UID role to each item.
  QVariant mrmlIdData = this->data(modelIndex, qMRML::UIDRole);
  if (mrmlIdData.type() != QVariant::Invalid)
    {
    roles.insert(qMRML::UIDRole, mrmlIdData);
    }
  return roles;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTreeProxyModel::mapFromSource(const QModelIndex &sourceIndex)const
{
  CTK_D(const qMRMLTreeProxyModel);
  return d->indexFromItem(d->proxyItemFromIndex(sourceIndex));
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTreeProxyModel::mapToSource(const QModelIndex &proxyIndex)const
{
  CTK_D(const qMRMLTreeProxyModel);
  return d->indexFromItem(d->sourceItemFromIndex(proxyIndex));
}

//------------------------------------------------------------------------------
//QMimeData *qMRMLTreeProxyModel::mimeData(const QModelIndexList &indexes)const
//{
//}

//------------------------------------------------------------------------------
//QStringList qMRMLTreeProxyModel::mimeTypes()const
//{
//}


//------------------------------------------------------------------------------
QModelIndex qMRMLTreeProxyModel::parent(const QModelIndex & modelIndex)const
{
  //std::ofstream toto ("parent.txt", std::ios_base::app);
  //toto<<"begin"<<std::endl;
  CTK_D(const qMRMLTreeProxyModel);
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
  QModelIndex vparent = d->indexFromItem(parentItem.data());
  //  toto<<"end"<<std::endl;
  return vparent;
}

//------------------------------------------------------------------------------
//bool qMRMLTreeProxyModel::removeColumns(int column, int count, const QModelIndex &vparent=QModelIndex())
//{
//}

//------------------------------------------------------------------------------
//bool qMRMLTreeProxyModel::removeRows(int row, int count, const QModelIndex &vparent)
//{
//}


//------------------------------------------------------------------------------
int qMRMLTreeProxyModel::rowCount(const QModelIndex &vparent) const
{
  CTK_D(const qMRMLTreeProxyModel);
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(d->proxyItemFromIndex(vparent));
  Q_ASSERT(!item.isNull());
  return !item.isNull() ? d->rowCountWithHiddenItemsRemoved(item.data()) : 0;
}

//------------------------------------------------------------------------------
bool qMRMLTreeProxyModel::setData(const QModelIndex & modelIndex, const QVariant &value, int role)
{
  CTK_D(const qMRMLTreeProxyModel);
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
void qMRMLTreeProxyModel::setMRMLScene(vtkMRMLScene* scene)
{
  if (this->sourceModel() == 0)
    {
    Q_ASSERT(this->sourceModel());
    return;
    }
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());    
  Q_ASSERT(sceneModel); // only qMRMLSceneModel is supported as a source
  if (sceneModel == 0)
    {
    return;
    }
  return sceneModel->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelperFactory* qMRMLTreeProxyModel::sourceItemFactory()const
{
  return qobject_cast<qMRMLSceneModel*>(this->sourceModel())->itemFactory();
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLTreeProxyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

