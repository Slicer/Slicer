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
class qMRMLSceneTreeModelPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSceneTreeModel);
protected:
  qMRMLSceneTreeModel* const q_ptr;
public:
  qMRMLSceneTreeModelPrivate(qMRMLSceneTreeModel& object);

  qMRMLAbstractItemHelper* itemFromUID(QVariant uid, int column = -1);
  QModelIndex indexFromItem(const qMRMLAbstractItemHelper* item)const;
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > itemsFromIndexes(
    const QModelIndex &_parent, int start, int end) const;
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > children(
    const qMRMLAbstractItemHelper* parentItem) const;
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > children(
    const qMRMLAbstractItemHelper* parentItem, int start, int end) const;

  int rowWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const;
  int childRowWithHiddenItemsAdded(const qMRMLAbstractItemHelper* parent, int row)const;
  int rowCountWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const;

  QStack<int> consecutiveRows(const QVector<QSharedPointer<qMRMLAbstractItemHelper> >& items ) const;
  int hiddenItem(vtkObject* object, int column)const;

protected:
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > HiddenItems;
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > ItemsToAdd;
  QVector<vtkObject*>                               NodesAboutToBeAdded;
#ifndef QT_NO_DEBUG
  vtkObject* HiddenVTKObject;
#endif
};

//------------------------------------------------------------------------------
qMRMLSceneTreeModelPrivate::qMRMLSceneTreeModelPrivate(qMRMLSceneTreeModel& object)
  : q_ptr(&object)
{
#ifndef QT_NO_DEBUG
  this->HiddenVTKObject = 0;
#endif
}

//------------------------------------------------------------------------------
int qMRMLSceneTreeModelPrivate::rowWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const
{
  Q_ASSERT(item);
  int itemRow = item->row();
  if (this->HiddenItems.size() == 0)
    {
    return itemRow;
    }
  int rowWithHiddenItemsRemovedValue = itemRow;
  QSharedPointer<qMRMLAbstractItemHelper> parentItem =
    QSharedPointer<qMRMLAbstractItemHelper>(item->parent());
  int parentItemRow = parentItem->row();
  foreach( const QSharedPointer<qMRMLAbstractItemHelper>& hiddenItem, this->HiddenItems)
    {
    int hiddenItemRow = hiddenItem->row();
    if (itemRow == hiddenItemRow &&
        item->column() == hiddenItem->column() &&
        item->object() == hiddenItem->object())
      {
      continue;
      }
    QSharedPointer<qMRMLAbstractItemHelper> hiddenItemParent =
      QSharedPointer<qMRMLAbstractItemHelper>(hiddenItem->parent());
    if (hiddenItemParent->row() == parentItemRow &&
        hiddenItemParent->column() == parentItem->column() &&
        hiddenItemParent->object() == parentItem->object() &&
        hiddenItem->column() == item->column() &&
        hiddenItemRow < itemRow)
      {
      --rowWithHiddenItemsRemovedValue;
      }
    }
  return rowWithHiddenItemsRemovedValue;
}

//------------------------------------------------------------------------------
int qMRMLSceneTreeModelPrivate::childRowWithHiddenItemsAdded(const qMRMLAbstractItemHelper* parentItem, int row)const
{
  Q_ASSERT(parentItem);
  int parentRow = parentItem->row();
  int childRowWithHiddenItemsAddedValue = row;
  foreach (const QSharedPointer<qMRMLAbstractItemHelper>& hiddenItem, this->HiddenItems)
    {
    QSharedPointer<qMRMLAbstractItemHelper> hiddenItemParent =
      QSharedPointer<qMRMLAbstractItemHelper>(hiddenItem->parent());
    if (hiddenItemParent->row() == parentRow &&
        hiddenItemParent->column() == parentItem->column() &&
        hiddenItemParent->object() == parentItem->object() &&
        hiddenItem->column() == 0)
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
  int itemRow = item->row();
  int rowCountWithHiddenItemsRemovedValue = item->childCount();
  foreach (const QSharedPointer<qMRMLAbstractItemHelper>& hiddenItem, this->HiddenItems)
    {
    if (hiddenItem->column() != 0)
      {
      continue;
      }
    QSharedPointer<qMRMLAbstractItemHelper> hiddenItemParent =
      QSharedPointer<qMRMLAbstractItemHelper>(hiddenItem->parent());
    if (hiddenItemParent->row() == itemRow
        && hiddenItemParent->column() == item->column()
        && hiddenItemParent->object() == item->object())
      {
      --rowCountWithHiddenItemsRemovedValue;
      }
    }
  return rowCountWithHiddenItemsRemovedValue;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneTreeModelPrivate::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  Q_Q(const qMRMLSceneTreeModel);
  if (item == 0 || item->object() == 0)
    {
    return QModelIndex();
    }
  if (dynamic_cast<const qMRMLRootItemHelper*>(item) != 0)
    {
    return QModelIndex();
    }
  return q->createIndex(this->rowWithHiddenItemsRemoved(item), item->column(),
                        reinterpret_cast<void*>(item->object()));
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneTreeModelPrivate::itemFromUID(QVariant uid, int column)
{
  Q_Q(const qMRMLSceneTreeModel);
  Q_ASSERT(q->mrmlScene());
  if (uid.toString().isNull())
    {
    return q->itemFactory()->createItem(q->mrmlScene(), column);
    }
  Q_ASSERT(!uid.toString().isNull());
  vtkMRMLNode* node = q->mrmlScene()->GetNodeByID(uid.toString().toLatin1().data());
  return q->itemFactory()->createItem(node, column);
}

//------------------------------------------------------------------------------
// qMRMLAbstractItemHelper* qMRMLSceneTreeModelPrivate::itemFromIndex(const QModelIndex &modelIndex)const
// {
//   Q_Q(const qMRMLSceneTreeModel);
//   if ((modelIndex.row() < 0) || (modelIndex.column() < 0) || (modelIndex.model() != p))
//     {
//     return q->itemFactory()->createRootItem(p->mrmlScene());
//     }
//   vtkObject* object =
//     reinterpret_cast<vtkObject*>( modelIndex.internalPointer());
//   return q->itemFactory()->createItem(object, modelIndex.column());
// }
// //------------------------------------------------------------------------------
// qMRMLAbstractItemHelper* qMRMLSceneTreeModelPrivate::sourceItemFromIndex(const QModelIndex & modelIndex)const
// {
//   Q_Q(const qMRMLSceneTreeModel);
//   if (( modelIndex.row() < 0) || ( modelIndex.column() < 0) || ( modelIndex.model() != q->sourceModel()))
//     {
//     if (p->sourceModel() == 0)
//       {
//       return 0;
//       }
//     qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(p->sourceModel());
//     Q_ASSERT(sceneModel); // only qMRMLSceneModel is supported a source
//     return q->itemFactory()->createRootItem(sceneModel->mrmlScene());
//     }
//   vtkObject* object = 
//     reinterpret_cast<vtkObject*>( modelIndex.internalPointer());
//   return q->sourceItemFactory()->createItem(object, modelIndex.column());
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
  Q_Q(const qMRMLSceneTreeModel);

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
//   Q_Q(const qMRMLSceneTreeModel);
//   QVector<QSharedPointer<qMRMLAbstractItemHelper> > childrenVector;

//   QSharedPointer<qMRMLAbstractItemHelper> parentSourceItem =
//     QSharedPointer<qMRMLAbstractItemHelper>(this->sourceItemFromIndex(vparent));
//   Q_ASSERT(parentSourceItem->childCount() > end);
//   // for each row
//   for (int i = start; i <= end; ++i)
//     {
//     // for each column
//     for (int j = 0 ; j < q->sourceModel()->columnCount(); ++j)
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
//   Q_Q(const qMRMLSceneTreeModel);
//   QVector<QSharedPointer<qMRMLAbstractItemHelper> > _children;

//   QSharedPointer<qMRMLAbstractItemHelper> parentItem =
//     QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(_parent));
//   Q_ASSERT(parentItem->childCount() > end);
//   // for each row
//   for (int i = start; i <= end; ++i)
//     {
//     // for each column
//     for (int j = 0 ; j < q->sourceModel()->columnCount(); ++j)
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
  Q_Q(const qMRMLSceneTreeModel);
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > _children;

  QSharedPointer<qMRMLAbstractItemHelper> parentItem =
    QSharedPointer<qMRMLAbstractItemHelper>(q->itemFromIndex(_parent));
  Q_ASSERT(parentItem->childCount() > end);
  return this->children(parentItem.data(), start, end);
}

//------------------------------------------------------------------------------
QVector<QSharedPointer<qMRMLAbstractItemHelper> >
qMRMLSceneTreeModelPrivate::children(const qMRMLAbstractItemHelper* parentItem) const
{
  return this->children(parentItem, 0, parentItem->childCount()-1);
}

//------------------------------------------------------------------------------
QVector<QSharedPointer<qMRMLAbstractItemHelper> >
qMRMLSceneTreeModelPrivate::children(const qMRMLAbstractItemHelper* parentItem,
                                     int start, int end) const
{
  Q_Q(const qMRMLSceneTreeModel);
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > _children;

  Q_ASSERT(parentItem->childCount() > end);
  // for each row
  for (int i = start; i <= end; ++i)
    {
    QSharedPointer<qMRMLAbstractItemHelper> childItem =
      QSharedPointer<qMRMLAbstractItemHelper>(parentItem->child(i, 0));
    // for each column
    for (int j = 0 ; j < q->columnCount(); ++j)
      {
      QModelIndex index = q->createIndex(i, j, childItem->object());
      QSharedPointer<qMRMLAbstractItemHelper> item =
        QSharedPointer<qMRMLAbstractItemHelper>(q->itemFromIndex(index));
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
  Q_Q(const qMRMLSceneTreeModel);
  Q_ASSERT(items.size() % q->columnCount() == 0);
  Q_UNUSED(q);

  QSharedPointer<qMRMLAbstractItemHelper> lastParentItem;
  int lastRow = -1;
  QStack<int> consecutiveRowsStack;
  bool aValidItem = false;
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
    int itemRow = itemParent->row(item.data());
    if (!lastParentItem.isNull() &&
        itemParent->object() == lastParentItem->object() &&
        itemRow == lastRow + 1)
      {
      Q_ASSERT(consecutiveRowsStack.size());
      ++consecutiveRowsStack.top();
      }
    else
      {
      consecutiveRowsStack.push(1);
      }
    lastParentItem = itemParent;
    lastRow = itemRow;
    }
  Q_ASSERT(aValidItem);
  Q_ASSERT(!consecutiveRowsStack.empty());
  return consecutiveRowsStack;
}

int qMRMLSceneTreeModelPrivate::hiddenItem(vtkObject* object, int column)const
{
  int index = -1;
  foreach(QSharedPointer<qMRMLAbstractItemHelper> item, this->HiddenItems)
    {
    ++index;
    if (item->object() == object && item->column() == column)
      {
      Q_ASSERT(index != 1);
      return index;
      }
    }
  Q_ASSERT(0);
  return -1;
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
//   Q_Q(qMRMLSceneTreeModel);
//   QSharedPointer<qMRMLAbstractItemHelper> topLeftItem = 
//     QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(topLeft));
//   QSharedPointer<qMRMLAbstractItemHelper> bottomRightItem = 
//     QSharedPointer<qMRMLAbstractItemHelper>(this->proxyItemFromIndex(bottomRight));
//   //todo: probably should convert the indexes...
//   emit q->dataChanged(this->indexFromItem(topLeftItem.data()), this->indexFromItem(bottomRightItem.data()));
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
// {
//   Q_Q(qMRMLSceneTreeModel);
//   //todo: probably should convert the indexes...
//   emit q->headerDataChanged(orientation, first, last);
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceLayoutAboutToBeChanged()
// {
//   Q_Q(qMRMLSceneTreeModel);
//   emit q->layoutAboutToBeChanged();
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceLayoutChanged()
// {
//   Q_Q(qMRMLSceneTreeModel);
//   emit q->layoutChanged();
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceModelAboutToBeReset()
// {
//   Q_Q(qMRMLSceneTreeModel);
//   q->beginResetModel();
// }

// //------------------------------------------------------------------------------
// void qMRMLSceneTreeModelPrivate::onSourceModelReset()
// {
//   Q_Q(qMRMLSceneTreeModel);
//   q->endResetModel();
// }

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneNodeAboutToBeAdded(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_D(qMRMLSceneTreeModel);
  if (scene->GetIsUpdating())
    {
    return;
    }

  Q_UNUSED(scene);
  Q_UNUSED(node);
  //qDebug() << "onSourceRowsAboutToBeInserted" << parent << start << end;
  // We can't do anything here because
  //  * we don't know where the new item will be added.
  // ->we'll do all the process in onSourceRowsInserted
  d->NodesAboutToBeAdded << node;
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneNodeAboutToBeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_D(qMRMLSceneTreeModel);
  if (scene->GetIsUpdating())
    {
    return;
    }

  // we test if it's empty here, but it's just because I never needed nested calls
  // to rows about to be removed/added. It can be removed if needed
  Q_ASSERT(d->HiddenItems.empty());
  if (this->listenNodeModifiedEvent())
    {
    qvtkDisconnect(node, vtkCommand::ModifiedEvent,
                   this, SLOT(onMRMLNodeModified(vtkObject*)));
    }
  QSharedPointer<qMRMLAbstractItemHelper> nodeItem =
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromObject(node, 0));
  QSharedPointer<qMRMLAbstractItemHelper> nodeItemParent =
    QSharedPointer<qMRMLAbstractItemHelper>(nodeItem->parent());
  int nodeRow = nodeItemParent->row(nodeItem.data());
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > itemsToRemove =
    d->children(nodeItemParent.data(), nodeRow, nodeRow);
//itemsToRemove.push_back();
//itemsToRemove.push_back(QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromObject(node, 1)));
  // We can process hidden items by bulk if they are consecutive.
  // While start/end cover consecutive items, they might not all be consecutive
  // if there are hidden items in the range.
  QStack<int> consecutiveRowsToBeRemoved;// = d->consecutiveRows(itemsToRemove);
  consecutiveRowsToBeRemoved << 1;
  Q_ASSERT_X(consecutiveRowsToBeRemoved.size(), __FUNCTION__,QString("Node: %1, ").arg(node->GetClassName()).toLatin1().data());

  // for each consecutive items
  //for (int i = 0 ; i < consecutiveRowsToBeRemoved.count(); ++i)
  foreach (int numberOfRows, consecutiveRowsToBeRemoved)
    {
    // get the first item to remove from the consecutive bulk
    QSharedPointer<qMRMLAbstractItemHelper> item = itemsToRemove.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent =
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int start = d->rowWithHiddenItemsRemoved(item.data());
    //qDebug() << (void *)item->object() << d->HiddenItems.size();
    //int numberOfRows = consecutiveRowsToBeRemoved[i];
    // proxyItemsFromSourceIndexes returned items for each column (not just 1
    // per row), here we compute the total number of items.
    QModelIndex parentIndex = this->indexFromItem(itemParent.data());
    int numberOfItems = numberOfRows * this->columnCount(parentIndex);
    /*
    qDebug() << "****Remove:" << start << start + numberOfRows - 1 << item->object()
             << item->data().toString().toLatin1().data() << vtkMRMLNode::SafeDownCast(item->object())->GetID();
    */
    // send the Qt events for the qAbstractItemModel
    // the item to remove doesn't have to be hidden yet, it's valid to have it
    // in the tree
    //qDebug() << "Remove: " << parentIndex << this->rowCount(parentIndex) << start;
    //qDebug() << qMRMLUtils::childCount(vtkMRMLNode::SafeDownCast(node));
    Q_ASSERT(start < this->rowCount(parentIndex));
    //qDebug() << "Remove: " << this->mrmlScene()->GetNumberOfNodes() << qMRMLUtils::childCount(this->mrmlScene());
    this->beginRemoveRows(parentIndex, start, start + numberOfRows - 1);

    d->ItemsToAdd = d->children(item.data());
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
    // we could move endRemoveRows in onMRMLNodeSceneRemoved as we know only 1
    // node has been removed
    this->endRemoveRows();
    //qDebug() << "Removed: " << parentIndex << this->rowCount(parentIndex);
    QSharedPointer<qMRMLAbstractItemHelper> sceneItem =
      QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromObject(scene, 0));
    //qDebug() << "onMRMLSceneNodeAboutToBeRemoved: scenecount: " << sceneItem->childCount();
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneNodeAdded(vtkMRMLScene* scene, vtkMRMLNode* node)
{
  Q_D(qMRMLSceneTreeModel);
  if (scene->GetIsUpdating())
    {
    return;
    }

  // we test if it's empty here, but it's just because I never needed nested calls
  // to rows about to be removed/added. It can be removed if needed
  //Q_ASSERT(d->HiddenItems.empty());

  QSharedPointer<qMRMLAbstractItemHelper> item;
  QSharedPointer<qMRMLAbstractItemHelper> itemParent;
  //int start = -1;
  int hiddenItemsIndex = -1;
  if (d->NodesAboutToBeAdded.empty())
    {
    hiddenItemsIndex = d->hiddenItem(node, 0);
    item = d->HiddenItems[hiddenItemsIndex];
    itemParent = QSharedPointer<qMRMLAbstractItemHelper>(item->parent());
    Q_ASSERT( (hiddenItemsIndex % this->columnCount()) ==0 );
    }
  else
    {
    int i = d->HiddenItems.count();
    foreach(vtkObject* object, d->NodesAboutToBeAdded)
      {
      QSharedPointer<qMRMLAbstractItemHelper> nodeItem =
        QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromObject(object, 0));
      QSharedPointer<qMRMLAbstractItemHelper> nodeItemParent =
        QSharedPointer<qMRMLAbstractItemHelper>(nodeItem->parent());
      int nodeRow = nodeItemParent->row(nodeItem.data());
      d->HiddenItems << d->children(nodeItemParent.data(), nodeRow, nodeRow);
      if(object == node)
        {
        Q_ASSERT(hiddenItemsIndex == -1);
        hiddenItemsIndex = i;
        item = nodeItem;
        itemParent = nodeItemParent;
        //start = nodeRow;
        }
      i += this->columnCount();
      }
    Q_ASSERT( (hiddenItemsIndex % this->columnCount()) ==0 );
    d->NodesAboutToBeAdded.clear();
    }
  // items inserted are in this->HiddenItesm.
  //QVector<int>::const_iterator it;
  //for (it = consecutiveRowsStack.begin(); it != consecutiveRowsStack.end(); ++it)
  Q_ASSERT(!d->HiddenItems.empty());

  //QSharedPointer<qMRMLAbstractItemHelper> item = d->HiddenItems.front();
  //QSharedPointer<qMRMLAbstractItemHelper> itemParent = 
  //  QSharedPointer<qMRMLAbstractItemHelper>(item->parent());
  //int start = item->row();
  //int numberOfRows = *it;
  /*
    qDebug() << "nbofrows:" << numberOfRows << " '"
    << item->data().toString().toLatin1().data() << vtkMRMLNode::SafeDownCast(item->object())->GetID()
    << "' start: "<< start << " vparent: " << itemParent->row() << " " << itemParent->column();
  */
  int start = d->rowWithHiddenItemsRemoved(item.data());
  /*
  qMRMLExtraItemsHelper* itemParentExtra = dynamic_cast<qMRMLExtraItemsHelper*>(itemParent.data());
  if (itemParentExtra)
    {
    int postItemsCount = itemParentExtra->postItems() ?
      itemParentExtra->postItems()->GetNumberOfItems() :0;
    Q_ASSERT(start == itemParentExtra->childCount() - postItemsCount - 1 );
    }
  */
  //qDebug() << "Scene: " << this->mrmlScene()->GetNumberOfNodes() << qMRMLUtils::childCount(this->mrmlScene());
  this->beginInsertRows(this->indexFromItem(itemParent.data()), start, start);
  d->HiddenItems.remove(hiddenItemsIndex, this->columnCount());
  this->endInsertRows();
  //Q_ASSERT(d->HiddenItems.empty());
  if (this->listenNodeModifiedEvent())
    {
    qvtkConnect(node, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneNodeRemoved(vtkMRMLScene* scene, vtkMRMLNode *node)
{
  Q_D(qMRMLSceneTreeModel);
  if (scene->GetIsUpdating())
    {
    return;
    }

#ifndef QT_NO_DEBUG
  d->HiddenVTKObject = 0;
  QSharedPointer<qMRMLAbstractItemHelper> sceneItem =
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromObject(scene, 0));
  //qDebug() << "onMRMLSceneNodeRemoved: scenecount: " << sceneItem->childCount();
#endif
  // we should probably not clear everything (in case if insertion/removes are
  // nested)
  int hiddenItemsIndex = d->hiddenItem(node, 0);
  d->HiddenItems.remove(hiddenItemsIndex, this->columnCount());

  if (!d->ItemsToAdd.size())
    {
    return;
    }
  QStack<int> consecutiveRowsToInsert;// = d->consecutiveRows(d->ItemsToAdd);
  consecutiveRowsToInsert.push(1);
  // items inserted are in this->ItemsToAdd.
  foreach(int numberOfRows, consecutiveRowsToInsert)
    {
    Q_ASSERT(!d->ItemsToAdd.empty());

    QSharedPointer<qMRMLAbstractItemHelper> item = d->ItemsToAdd.front();
    QSharedPointer<qMRMLAbstractItemHelper> itemParent =
      QSharedPointer<qMRMLAbstractItemHelper>(item->parent());

    int start = d->rowWithHiddenItemsRemoved(item.data());
    //qDebug() << "************onMRMLSceneNodeRemoved: "<<  start << this->indexFromItem(itemParent.data());
    this->beginInsertRows(this->indexFromItem(itemParent.data()), start, start + numberOfRows - 1);
    d->ItemsToAdd.remove(0, numberOfRows * this->columnCount());
    this->endInsertRows();
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneAboutToBeImported(vtkMRMLScene* scene)
{
  this->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneImported(vtkMRMLScene* scene)
{
  this->endResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneAboutToBeClosed(vtkMRMLScene* scene)
{
  this->beginResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSceneTreeModel::onMRMLSceneClosed(vtkMRMLScene* scene)
{
  this->endResetModel();
}



//------------------------------------------------------------------------------
// qMRMLSceneTreeModel

//------------------------------------------------------------------------------
qMRMLSceneTreeModel::qMRMLSceneTreeModel(qMRMLSceneModelItemHelperFactory* factory, QObject *parentObject)
  :qMRMLSceneModel(factory, parentObject)
  , d_ptr(new qMRMLSceneTreeModelPrivate(*this))
{
}

//------------------------------------------------------------------------------
qMRMLSceneTreeModel::~qMRMLSceneTreeModel()
{
}

//------------------------------------------------------------------------------
// void qMRMLSceneTreeModel::setSourceModel(QAbstractItemModel * sourceModelItem)
// {
//   Q_D(qMRMLSceneTreeModel);
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
//   Q_D(const qMRMLSceneTreeModel);
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
  Q_D(qMRMLSceneTreeModel);
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
        int itemRow = parentItem->row(item.data());
        QVector<QSharedPointer<qMRMLAbstractItemHelper> > newItems =
          d->itemsFromIndexes(vparent, itemRow, itemRow);
        d->HiddenItems += newItems;
#ifndef QT_NO_DEBUG
        d->HiddenVTKObject= item->object();
#endif

        this->endRemoveRows();

        // what's tricky here is that vparent might be invalid now. (if the 
        // moved row was at the same level than the parent, it shifted up the 
        // parent). we must recompute the parent new index.
        itemRow = parentItem->row(item.data());
        this->beginInsertRows(this->indexFromItem(parentItem.data()), itemRow, itemRow);

        d->HiddenItems.remove(d->HiddenItems.count() - newItems.count(), newItems.count());
#ifndef QT_NO_DEBUG
        d->HiddenVTKObject= 0;
#endif
        Q_ASSERT(d->HiddenItems.empty());
        this->endInsertRows();
        }
      }
    }
  // Qt should work fine even without the layoutChanged... however
  emit layoutChanged();
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
  Q_D(const qMRMLSceneTreeModel);

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
  Q_D(const qMRMLSceneTreeModel);
  // sanity check before going any further.
  if (this->mrmlScene() == 0 || row < 0 || column < 0)
    {
    return QModelIndex();
    }
  QSharedPointer<qMRMLAbstractItemHelper> parentItem = 
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(vparent));
    
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(
      parentItem->child(d->childRowWithHiddenItemsAdded(parentItem.data(), row)
                        , column));
#ifndef QT_NO_DEBUG
  //QModelIndex i = !item.isNull() ? this->createIndex(row, column, item->object()) : QModelIndex();
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
//   Q_D(const qMRMLSceneTreeModel);
//   return this->item(modelIndex);
// }

//------------------------------------------------------------------------------
// qMRMLAbstractItemHelper* qMRMLSceneTreeModel::proxyItem(const QModelIndex &modelIndex)const
// {
//   Q_D(const qMRMLSceneTreeModel);
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
//   Q_D(const qMRMLSceneTreeModel);
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
  Q_D(const qMRMLSceneTreeModel);
  QSharedPointer<qMRMLAbstractItemHelper> item =
    QSharedPointer<qMRMLAbstractItemHelper>(this->itemFromIndex(parentIndex));
  Q_ASSERT(!item.isNull());
  return !item.isNull() ? d->rowCountWithHiddenItemsRemoved(item.data()) : 0;
}

//------------------------------------------------------------------------------
// bool qMRMLSceneTreeModel::setData(const QModelIndex & modelIndex, const QVariant &value, int role)
// {
//   Q_D(const qMRMLSceneTreeModel);
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
//qMRMLAbstractItemHelper* qMRMLSceneTreeModel::itemFromIndex(const QModelIndex &modelIndex)const
//{
//  Q_D(const qMRMLSceneModel);
//  return d->itemFromIndex(modelIndex);
//}

//------------------------------------------------------------------------------
QModelIndex qMRMLSceneTreeModel::indexFromItem(const qMRMLAbstractItemHelper* item)const
{
  Q_D(const qMRMLSceneTreeModel);
  return d->indexFromItem(item);
}
