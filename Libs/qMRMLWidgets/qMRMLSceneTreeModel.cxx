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
#include "qMRMLSceneTreeModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>

//------------------------------------------------------------------------------
class qMRMLSceneTreeModelPrivate: public ctkPrivate<qMRMLSceneTreeModel>
{
public:
  CTK_DECLARE_PUBLIC(qMRMLSceneTreeModel);
  qMRMLSceneTreeModelPrivate();

  qMRMLAbstractItemHelper* itemFromUID(QVariant uid, int column = -1);
  QModelIndex indexFromItem(const qMRMLAbstractItemHelper* item)const;
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > itemsFromIndexes(
    const QModelIndex &_parent, int start, int end) const;
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > children(
    const qMRMLAbstractItemHelper* parentItem, int start, int end) const;

  int rowWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const;
  int childRowWithHiddenItemsAdded(const qMRMLAbstractItemHelper* parent, int row)const;
  int rowCountWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const;

  QStack<int> consecutiveRows(const QVector<QSharedPointer<qMRMLAbstractItemHelper> >& items ) const;

protected:
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > HiddenItems;
#ifndef QT_NO_DEBUG
  vtkObject* HiddenVTKObject;
#endif
};

//------------------------------------------------------------------------------
qMRMLSceneTreeModelPrivate::qMRMLSceneTreeModelPrivate()
{
#ifndef QT_NO_DEBUG
  this->HiddenVTKObject = 0;
#endif
}

//------------------------------------------------------------------------------
int qMRMLSceneTreeModelPrivate::rowWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const
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
int qMRMLSceneTreeModelPrivate::childRowWithHiddenItemsAdded(const qMRMLAbstractItemHelper* vparent, int row)const
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
int qMRMLSceneTreeModelPrivate::rowCountWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const
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
QModelIndex qMRMLSceneTreeModelPrivate::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  CTK_P(const qMRMLSceneTreeModel);
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
qMRMLAbstractItemHelper* qMRMLSceneTreeModelPrivate::itemFromUID(QVariant uid, int column)
{
  CTK_P(const qMRMLSceneTreeModel);
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
// qMRMLAbstractItemHelper* qMRMLSceneTreeModelPrivate::itemFromIndex(const QModelIndex &modelIndex)const
// {
//   CTK_P(const qMRMLSceneTreeModel);
//   if ((modelIndex.row() < 0) || (modelIndex.column() < 0) || (modelIndex.model() != p))
//     {
//     return p->itemFactory()->createRootItem(p->mrmlScene());
//     }
//   vtkObject* object =
//     reinterpret_cast<vtkObject*>( modelIndex.internalPointer());
//   return p->itemFactory()->createItem(object, modelIndex.column());
// }
// //------------------------------------------------------------------------------
// qMRMLAbstractItemHelper* qMRMLSceneTreeModelPrivate::sourceItemFromIndex(const QModelIndex & modelIndex)const
// {
//   CTK_P(const qMRMLSceneTreeModel);
//   if (( modelIndex.row() < 0) || ( modelIndex.column() < 0) || ( modelIndex.model() != p->sourceModel()))
//     {
//     if (p->sourceModel() == 0)
//       {
//       return 0;
//       }
//     qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(p->sourceModel());
//     Q_ASSERT(sceneModel); // only qMRMLSceneModel is supported a source
//     return p->itemFactory()->createRootItem(sceneModel->mrmlScene());
//     }
//   vtkObject* object = 
//     reinterpret_cast<vtkObject*>( modelIndex.internalPointer());
//   return p->sourceItemFactory()->createItem(object, modelIndex.column());
// }

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
qMRMLAbstractItemHelper* qMRMLSceneTreeModelPrivate::sourceItemFromObject(vtkObject* object, int column)const
{
  CTK_P(const qMRMLSceneTreeModel);

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

// //------------------------------------------------------------------------------
// QVector<QSharedPointer<qMRMLAbstractItemHelper> >
// qMRMLSceneTreeModelPrivate::proxyItemsFromSourceIndexes(const QModelIndex &vparent, int start, int end) const
// {
//   CTK_P(const qMRMLSceneTreeModel);
//   QVector<QSharedPointer<qMRMLAbstractItemHelper> > childrenVector;

//   QSharedPointer<qMRMLAbstractItemHelper> parentSourceItem =
//     QSharedPointer<qMRMLAbstractItemHelper>(this->sourceItemFromIndex(vparent));
//   Q_ASSERT(parentSourceItem->childCount() > end);
//   // for each row
//   for (int i = start; i <= end; ++i)
//     {
//     // for each column
//     for (int j = 0 ; j < p->sourceModel()->columnCount(); ++j)
//       {
//       // convert sourceItem to proxyItem;
//       QSharedPointer<qMRMLAbstractItemHelper> sourceItem =
//         QSharedPointer<qMRMLAbstractItemHelper>(parentSourceItem->child(i, j));
//       Q_ASSERT(sourceItem.data());
//       Q_ASSERT(j == sourceItem->column());
//       QModelIndex  modelIndex = this->indexFromItem(sourceItem.data());
//       Q_ASSERT(j ==  modelIndex.column());
//       QSharedPointer<qMRMLAbstractItemHelper> proxyItem =
//         QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex( modelIndex));
//       Q_ASSERT(j == proxyItem->column());
//       childrenVector.append(proxyItem);
//       }
//     }
//   return childrenVector;
// }


// //------------------------------------------------------------------------------
// QVector<QSharedPointer<qMRMLAbstractItemHelper> >
// qMRMLSceneTreeModelPrivate::proxyItemsFromProxyIndexes(const QModelIndex &_parent,
//                                                        int start, int end) const
// {
//   CTK_P(const qMRMLSceneTreeModel);
//   QVector<QSharedPointer<qMRMLAbstractItemHelper> > _children;

//   QSharedPointer<qMRMLAbstractItemHelper> parentItem =
//     QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(_parent));
//   Q_ASSERT(parentItem->childCount() > end);
//   // for each row
//   for (int i = start; i <= end; ++i)
//     {
//     // for each column
//     for (int j = 0 ; j < p->sourceModel()->columnCount(); ++j)
//       {
//       // convert sourceItem to proxyItem;
//       QSharedPointer<qMRMLAbstractItemHelper> item =
//         QSharedPointer<qMRMLAbstractItemHelper>(parentItem->child(i, j));
//       Q_ASSERT(item.data());
//       Q_ASSERT(j == item->column());
//       _children.append(item);
//       }
//     }
//   return _children;
// }
//
//------------------------------------------------------------------------------
QVector<QSharedPointer<qMRMLAbstractItemHelper> >
qMRMLSceneTreeModelPrivate::itemsFromIndexes(const QModelIndex &_parent,
                                             int start, int end) const
{
  CTK_P(const qMRMLSceneTreeModel);
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > _children;

  QSharedPointer<qMRMLAbstractItemHelper> parentItem =
    QSharedPointer<qMRMLAbstractItemHelper>(p->itemFromIndex(_parent));
  Q_ASSERT(parentItem->childCount() > end);
  return this->children(parentItem.data(), start, end);
}

//------------------------------------------------------------------------------
QVector<QSharedPointer<qMRMLAbstractItemHelper> >
qMRMLSceneTreeModelPrivate::children(const qMRMLAbstractItemHelper* parentItem,
                                     int start, int end) const
{
  CTK_P(const qMRMLSceneTreeModel);
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > _children;

  Q_ASSERT(parentItem->childCount() > end);
  // for each row
  for (int i = start; i <= end; ++i)
    {
    // for each column
    for (int j = 0 ; j < p->columnCount(); ++j)
      {
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
qMRMLSceneTreeModelPrivate::consecutiveRows(const QVector<QSharedPointer<qMRMLAbstractItemHelper> >& items ) const
{
  QSharedPointer<qMRMLAbstractItemHelper> lastParentItem;
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

    if (!lastParentItem.isNull() &&
        itemParent->object() == lastParentItem->object() &&
        item->row() == lastRow + 1)
      {
      Q_ASSERT(consecutiveRowsStack.size());
      ++consecutiveRowsStack.top();
      }
    else
      {
      consecutiveRowsStack.push(1);
      }
    lastParentItem = itemParent;
    lastRow = item->row();
    }
  Q_ASSERT(aValidItem);
  Q_ASSERT(!consecutiveRowsStack.empty());
  return consecutiveRowsStack;
}


// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceColumnsAboutToBeInserted(const QModelIndex & vparent, int start, int end)
// {
//   Q_UNUSED(vparent);
//   Q_UNUSED(start);
//   Q_UNUSED(end);
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceColumnsAboutToBeRemoved(const QModelIndex & vparent, int start, int end)
// {
//   Q_UNUSED(vparent);
//   Q_UNUSED(start);
//   Q_UNUSED(end);
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceColumnsInserted(const QModelIndex & vparent, int start, int end)
// {
//   Q_UNUSED(vparent);
//   Q_UNUSED(start);
//   Q_UNUSED(end);
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceColumnsRemoved(const QModelIndex & vparent, int start, int end)
// {
//   Q_UNUSED(vparent);
//   Q_UNUSED(start);
//   Q_UNUSED(end);
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
// {
//   CTK_P(qMRMLSceneTreeModel);
//   QSharedPointer<qMRMLAbstractItemHelper> topLeftItem = 
//     QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(topLeft));
//   QSharedPointer<qMRMLAbstractItemHelper> bottomRightItem = 
//     QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(bottomRight));
//   //todo: probably should convert the indexes...
//   emit p->dataChanged(this->indexFromItem(topLeftItem.data()), this->indexFromItem(bottomRightItem.data()));
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
// {
//   CTK_P(qMRMLSceneTreeModel);
//   //todo: probably should convert the indexes...
//   emit p->headerDataChanged(orientation, first, last);
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceLayoutAboutToBeChanged()
// {
//   CTK_P(qMRMLSceneTreeModel);
//   emit p->layoutAboutToBeChanged();
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceLayoutChanged()
// {
//   CTK_P(qMRMLSceneTreeModel);
//   emit p->layoutChanged();
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceModelAboutToBeReset()
// {
//   CTK_P(qMRMLSceneTreeModel);
//   p->beginResetModel();
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceModelReset()
// {
//   CTK_P(qMRMLSceneTreeModel);
//   p->endResetModel();
// }

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneNodeAboutToBeAdded(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  //qDebug() << "onSourceRowsAboutToBeInserted" << parent << start << end;
  // We can't do anything here because
  //  * we don't know where the new item will be added.
  // ->we'll do all the process in onSourceRowsInserted
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneNodeAboutToBeRemoved(vtkObject* scene, vtkObject* node)
{
  CTK_D(qMRMLSceneTreeModel);
  // we test if it's empty here, but it's just because I never needed nested calls
  // to rows about to be removed/added. It can be removed if needed
  Q_ASSERT(d->HiddenItems.empty());

  QSharedPointer<qMRMLAbstractItemHelper> nodeItem =
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromObject(node, 0));
  QSharedPointer<qMRMLAbstractItemHelper> nodeItemParent =
    QSharedPointer<qMRMLAbstractItemHelper>(nodeItem->parent());
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > itemsToRemove =
    d->children(nodeItemParent.data(), nodeItem->row(), nodeItem->row());
//itemsToRemove.push_back();
//itemsToRemove.push_back(QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromObject(node, 1)));
  // We can process hidden items by bulk if they are consecutive.
  // While start/end cover consecutive items, they might not all be consecutive
  // if there are hidden items in the range.
  QStack<int> consecutiveRowsToBeRemoved = d->consecutiveRows(itemsToRemove);
  Q_ASSERT_X(consecutiveRowsToBeRemoved.size(), __FUNCTION__,QString("Node: %1, ").arg(node->GetClassName()).toLatin1().data());

  // for each consecutive items
  //for (int i = 0 ; i < consecutiveRowsToBeRemoved.count(); ++i)
  foreach (int numberOfRows, consecutiveRowsToBeRemoved)
    {
    // get the first item to remove from the consecutive bulk
    QSharedPointer<qMRMLAbstractItemHelper> item = itemsToRemove.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent =
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int start = item->row();
    //int numberOfRows = consecutiveRowsToBeRemoved[i];
    // proxyItemsFromSourceIndexes returned items for each column (not just 1
    // per row), here we compute the total number of items.
    int numberOfItems = numberOfRows * this->columnCount();
    /*
    qDebug() << "****Remove:" << start << start + numberOfRows - 1 << item->object()
             << item->data().toString().toLatin1().data() << vtkMRMLNode::SafeDownCast(item->object())->GetID();
    */
    // send the Qt events for the qAbstractItemModel
    // the item to remove doesn't have to be hidden yet, it's valid to have it
    // in the tree
    this->beginRemoveRows(this->indexFromItem(itemParent.data()), start, start + numberOfRows - 1);
    // now we fake that the item is removed from the model
    d->HiddenItems += itemsToRemove.mid(0, numberOfItems);
#ifndef QT_NO_DEBUG
    d->HiddenVTKObject = item->object();
#endif
    itemsToRemove.remove(0, numberOfItems);
    // here we call the function associated to beginRemoveRows to pretend that the
    // items have been removed from the tree. They aren't exactly removed, just
    // hidden for the momement. They will be truly removed when
    // onSourceRowsRemoved() will be called. Until then, we hide the items
    this->endRemoveRows();
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneNodeAdded(vtkObject* scene, vtkObject* node)
{
  CTK_D(qMRMLSceneTreeModel);
  // we test if it's empty here, but it's just because I never needed nested calls
  // to rows about to be removed/added. It can be removed if needed
  Q_ASSERT(d->HiddenItems.empty());

  //proxyItemsFromSourceIndexes returns also the column items
  //d->HiddenItems << QSharedPointer<qMRMLAbstractItemHelper>(this->itemFactory()->createItem(node,0));
  QSharedPointer<qMRMLAbstractItemHelper> nodeItem =
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromObject(node, 0));
  QSharedPointer<qMRMLAbstractItemHelper> nodeItemParent =
    QSharedPointer<qMRMLAbstractItemHelper>(nodeItem->parent());
  d->HiddenItems = d->children(nodeItemParent.data(), nodeItem->row(), nodeItem->row());
  QStack<int> consecutiveRowsToInsert = d->consecutiveRows(d->HiddenItems);
  Q_ASSERT_X(consecutiveRowsToInsert.size(), __FUNCTION__, QString("Start: %1, ").arg(node->GetClassName()).toLatin1().data());
  // items inserted are in this->HiddenItesm.
  //QVector<int>::const_iterator it;
  //for (it = consecutiveRowsStack.begin(); it != consecutiveRowsStack.end(); ++it)
  foreach(int numberOfRows, consecutiveRowsToInsert)
    {
    Q_ASSERT(!d->HiddenItems.empty());

    QSharedPointer<qMRMLAbstractItemHelper> item = d->HiddenItems.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent = 
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int start = item->row();
    //int numberOfRows = *it;
    /*
    qDebug() << "nbofrows:" << numberOfRows << " '"
             << item->data().toString().toLatin1().data() << vtkMRMLNode::SafeDownCast(item->object())->GetID()
             << "' start: "<< start << " vparent: " << itemParent->row() << " " << itemParent->column();
    */
    this->beginInsertRows(this->indexFromItem(itemParent.data()), start, start + numberOfRows - 1);

    d->HiddenItems.remove(0, numberOfRows * this->columnCount());

    this->endInsertRows();
    }
  Q_ASSERT(d->HiddenItems.empty());
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneNodeRemoved(vtkObject* scene, vtkObject *node)
{
  CTK_D(qMRMLSceneTreeModel);
#ifndef QT_NO_DEBUG
  d->HiddenVTKObject = 0;
#endif
  // we should probably not clear everything (in case if insertion/removes are 
  // nested)
  d->HiddenItems.clear();
}
//------------------------------------------------------------------------------
// qMRMLSceneTreeModel

//------------------------------------------------------------------------------
qMRMLSceneTreeModel::qMRMLSceneTreeModel(qMRMLSceneModelItemHelperFactory* factory, QObject *parentObject)
  :qMRMLSceneModel(factory, parentObject)
{
  CTK_INIT_PRIVATE(qMRMLSceneTreeModel);
}

//------------------------------------------------------------------------------
qMRMLSceneTreeModel::~qMRMLSceneTreeModel()
{
}

//------------------------------------------------------------------------------
// void qMRMLSceneTreeModel::setSourceModel(QAbstractItemModel * sourceModelItem)
// {
//   CTK_D(qMRMLSceneTreeModel);
//   Q_ASSERT_X(qobject_cast<qMRMLSceneModel*>(sourceModelItem), __FUNCTION__, "Only qMRMLSceneModels are supported");
//   this->QAbstractProxyModel::setSourceModel(sourceModelItem);

//   connect(sourceModelItem, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
//     d, SLOT(onSourceColumnsAboutToBeInserted(const QModelIndex &, int, int)));
//   connect(sourceModelItem, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
//     d, SLOT(onSourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
//   connect(sourceModelItem, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
//     d, SLOT(onSourceColumnsInserted(const QModelIndex &, int, int)));
//   connect(sourceModelItem, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
//     d, SLOT(onSourceColumnsRemoved(const QModelIndex &, int, int)));
//   connect(sourceModelItem, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
//     d, SLOT(onSourceDataChanged(const QModelIndex &, const QModelIndex &)));
//   connect(sourceModelItem, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
//     d, SLOT(onSourceHeaderDataChanged(Qt::Orientation, int, int)));
//   connect(sourceModelItem, SIGNAL(layoutAboutToBeChanged()),
//     d, SLOT(onSourceLayoutAboutToBeChanged()));
//   connect(sourceModelItem, SIGNAL(layoutChanged()),
//     d, SLOT(onSourceLayoutChanged()));
//   connect(sourceModelItem, SIGNAL(modelAboutToBeReset()),
//     d, SLOT(onSourceModelAboutToBeReset()));
//   connect(sourceModelItem, SIGNAL(modelReset()),
//     d, SLOT(onSourceModelReset()));
//   connect(sourceModelItem, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
//     d, SLOT(onSourceRowsAboutToBeInserted(const QModelIndex &, int, int)));    
//   connect(sourceModelItem, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
//     d, SLOT(onSourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
//   connect(sourceModelItem, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
//     d, SLOT(onSourceRowsInserted(const QModelIndex &, int, int)));
//   connect(sourceModelItem, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
//     d, SLOT(onSourceRowsRemoved(const QModelIndex &, int, int)));
// }

//------------------------------------------------------------------------------
// int qMRMLSceneTreeModel::columnCount(const QModelIndex &) const
// {
//   return this->sourceModel() ? this->sourceModel()->columnCount() : 0;
// }

//------------------------------------------------------------------------------
// QVariant qMRMLSceneTreeModel::data(const QModelIndex & modelIndex, int role)const
// {
//   CTK_D(const qMRMLSceneTreeModel);
//   if (! modelIndex.isValid())
//     {
//     return QVariant();
//     }
//   Q_ASSERT(this->mrmlScene());

//   QSharedPointer<qMRMLAbstractItemHelper> item = 
//     QSharedPointer<qMRMLAbstractItemHelper>(this->item( modelIndex));

//   Q_ASSERT(!item.isNull());
//   return item->data(role);
// }


//------------------------------------------------------------------------------
bool qMRMLSceneTreeModel::dropMimeData(const QMimeData *dataValue, Qt::DropAction action, 
                                  int row, int column, const QModelIndex &vparent)
{
  CTK_D(qMRMLSceneTreeModel);
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
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(vparent));

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
      QModelIndex oldIndex = this->indexFromItem(item.data());
      if (item->canReparent(parentItem.data()) && 
          vparent != oldIndex.parent())
        {
        this->beginRemoveRows(oldIndex.parent(), oldIndex.row(), oldIndex.row());

        item->reparent(parentItem.data());
        // the item row should be automatically updated
        QVector<QSharedPointer<qMRMLAbstractItemHelper> > newItems =
          d->itemsFromIndexes(vparent, item->row(), item->row());
        d->HiddenItems += newItems;
#ifndef QT_NO_DEBUG
        d->HiddenVTKObject= item->object();
#endif

        this->endRemoveRows();
        // what's tricky here is that vparent might be invalid now. (if the 
        // moved row was at the same level than the parent, it shifted up the 
        // parent). we must recompute the parent new index.
        this->beginInsertRows(this->indexFromItem(parentItem.data()), item->row(), item->row());

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
Qt::ItemFlags qMRMLSceneTreeModel::flags(const QModelIndex &modelIndex)const
{
  if (!modelIndex.isValid())
    {
    return Qt::NoItemFlags;
    }

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(modelIndex));

  Q_ASSERT(!item.isNull());
  return item->flags();
}

// Has to be reimplemented for speed issues
//------------------------------------------------------------------------------
bool qMRMLSceneTreeModel::hasChildren(const QModelIndex &vparent)const
{
  CTK_D(const qMRMLSceneTreeModel);

  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(vparent));
  Q_ASSERT(!item.isNull());
  
  return d->rowCountWithHiddenItemsRemoved(item.data());
}

//------------------------------------------------------------------------------
// QVariant qMRMLSceneTreeModel::headerData(int section, Qt::Orientation orientation, int role)const
// {
//   Q_ASSERT(this->sourceModel());
//   // QAbstractProxyModel doesn't work as it tries to map an index into the
//   // source model. It can fail because the model can be empty... Here we know
//   // that the section doesn't change, so let's use it directly.
//   return this->sourceModel()->headerData(section, orientation, role);
// }

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneTreeModel::index(int row, int column, const QModelIndex &vparent)const
{
  CTK_D(const qMRMLSceneTreeModel);
  // sanity check before going any further.
  if (this->mrmlScene() == 0 || row < 0 || column < 0)
    {
    return QModelIndex();
    }
  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(vparent));
    
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
//bool qMRMLSceneTreeModel::insertRows(int row, int count, const QModelIndex &vparent)
//{
//}

//------------------------------------------------------------------------------
// qMRMLAbstractItemHelper* qMRMLSceneTreeModel::item(const QModelIndex &modelIndex)const
// {
//   CTK_D(const qMRMLSceneTreeModel);
//   return this->item(modelIndex);
// }

//------------------------------------------------------------------------------
// qMRMLAbstractItemHelper* qMRMLSceneTreeModel::proxyItem(const QModelIndex &modelIndex)const
// {
//   CTK_D(const qMRMLSceneTreeModel);
//   return d->proxyItemFromIndex(modelIndex);
// }

//------------------------------------------------------------------------------
// QMap<int, QVariant> qMRMLSceneTreeModel::itemData(const QModelIndex & modelIndex)const
// {
//   QMap<int, QVariant> roles = this->QAbstractItemModel::itemData(modelIndex);
//   // In order to have the drag/drop working without defining our own MIME type
//   // we need to add some way of uniquely identify an item. Here it's done 
//   // by adding a UID role to each item.
//   QVariant mrmlIdData = this->data(modelIndex, qMRML::UIDRole);
//   if (mrmlIdData.type() != QVariant::Invalid)
//     {
//     roles.insert(qMRML::UIDRole, mrmlIdData);
//     }
//   return roles;
// }

//------------------------------------------------------------------------------
//QMimeData *qMRMLSceneTreeModel::mimeData(const QModelIndexList &indexes)const
//{
//}

//------------------------------------------------------------------------------
//QStringList qMRMLSceneTreeModel::mimeTypes()const
//{
//}


//------------------------------------------------------------------------------
// QModelIndex qMRMLSceneTreeModel::parent(const QModelIndex & modelIndex)const
// {
//   CTK_D(const qMRMLSceneTreeModel);
//   if (! modelIndex.isValid())
//     {
//     return QModelIndex();
//     }
//   QSharedPointer<const qMRMLAbstractItemHelper> item = 
//     QSharedPointer<const qMRMLAbstractItemHelper>(d->proxyItemFromIndex( modelIndex));
//   Q_ASSERT(!item.isNull());
//   if (item.isNull())
//     {
//     return QModelIndex();
//     }
//   QSharedPointer<const qMRMLAbstractItemHelper> parentItem = 
//     QSharedPointer<const qMRMLAbstractItemHelper>(item->parent());
//   if (parentItem.isNull())
//     {
//     return QModelIndex();
//     }
//   QModelIndex vparent = this->indexFromItem(parentItem.data());
//   return vparent;
// }

//------------------------------------------------------------------------------
//bool qMRMLSceneTreeModel::removeColumns(int column, int count, const QModelIndex &vparent=QModelIndex())
//{
//}

//------------------------------------------------------------------------------
//bool qMRMLSceneTreeModel::removeRows(int row, int count, const QModelIndex &vparent)
//{
//}


//------------------------------------------------------------------------------
int qMRMLSceneTreeModel::rowCount(const QModelIndex &parentIndex) const
{
  CTK_D(const qMRMLSceneTreeModel);
  QSharedPointer<qMRMLAbstractItemHelper> item =
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(parentIndex));
  Q_ASSERT(!item.isNull());
  return !item.isNull() ? d->rowCountWithHiddenItemsRemoved(item.data()) : 0;
}

//------------------------------------------------------------------------------
// bool qMRMLSceneTreeModel::setData(const QModelIndex & modelIndex, const QVariant &value, int role)
// {
//   CTK_D(const qMRMLSceneTreeModel);
//   if (! modelIndex.isValid())
//     {
//     return false;
//     }
//   Q_ASSERT(this->mrmlScene());
//   QSharedPointer<qMRMLAbstractItemHelper> item =
//     QSharedPointer<qMRMLAbstractItemHelper>(d->item( modelIndex));
//   Q_ASSERT(!item.isNull());
//   bool changed = !item.isNull() ? item->setData(value, role) : false;
//   if (changed)
//     {
//     emit dataChanged( modelIndex,  modelIndex);
//     }
//   return changed;
// }

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneTreeModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneTreeModel::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  CTK_D(const qMRMLSceneTreeModel);
  return d->indexFromItem(item);
}
