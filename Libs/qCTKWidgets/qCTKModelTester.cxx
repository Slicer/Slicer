#include "qCTKModelTester.h"

#include <QDebug>
#include <QStack>

//-----------------------------------------------------------------------------
class qCTKModelTesterPrivate: public qCTKPrivate<qCTKModelTester>
{
public:
  qCTKModelTesterPrivate();
  QAbstractItemModel *Model;
  bool ThrowOnError;

  struct Change
  {
    QModelIndex Parent;
    Qt::Orientation Orientation;
    int Start;
    int End;
    
    int Count;
    QList<QPersistentModelIndex> Items;
  };

  QStack<Change> AboutToBeInserted;
  QStack<Change> AboutToBeRemoved;
  QList<QPersistentModelIndex> LayoutAboutToBeChanged;
};

//-----------------------------------------------------------------------------
qCTKModelTesterPrivate::qCTKModelTesterPrivate()
{
  this->Model = 0;
  this->ThrowOnError = true;
}

//-----------------------------------------------------------------------------
qCTKModelTester::qCTKModelTester(QAbstractItemModel *model, QObject *parent)
  :QObject(parent)
{
  QCTK_INIT_PRIVATE(qCTKModelTester);
  this->setModel(model);
}

//-----------------------------------------------------------------------------
void qCTKModelTester::setModel(QAbstractItemModel *model)
{
  QCTK_D(qCTKModelTester);
  if (d->Model)
    {
    // disconnect
    d->Model->disconnect(this);
    d->AboutToBeInserted.clear();
    d->AboutToBeRemoved.clear();
    d->LayoutAboutToBeChanged.clear();
    }
  if (model)
    {
    connect(model, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)), 
            this, SLOT(onColumnsAboutToBeInserted(const QModelIndex& , int, int)));
    connect(model, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)), 
            this, SLOT(onColumnsAboutToBeRemoved(const QModelIndex& , int, int)));
    connect(model, SIGNAL(columnsInserted(const QModelIndex &, int, int)), 
            this, SLOT(onColumnsInserted(const QModelIndex& , int, int)));
    connect(model, SIGNAL(columnsRemoved(const QModelIndex &, int, int)), 
            this, SLOT(onColumnsRemoved(const QModelIndex& , int, int)));
    connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), 
            this, SLOT(onDataChanged(const QModelIndex& , const QModelIndex &)));
    connect(model, SIGNAL(layoutAboutToBeChanged()), this, SLOT(onLayoutAboutToBeChanged()));
    connect(model, SIGNAL(layoutChanged()), this, SLOT(onLayoutChanged()));
    connect(model, SIGNAL(modelAboutToBeReset()), this, SLOT(onModelAboutToBeReset()));
    connect(model, SIGNAL(modelReset()), this, SLOT(onModelReset()));
    connect(model, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)), 
            this, SLOT(onRowsAboutToBeInserted(const QModelIndex& , int, int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), 
            this, SLOT(onRowsAboutToBeRemoved(const QModelIndex& , int, int)));
    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), 
            this, SLOT(onRowsInserted(const QModelIndex& , int, int)));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), 
            this, SLOT(onRowsRemoved(const QModelIndex& , int, int)));
    }
  d->Model = model;
  this->testModel();
}

//-----------------------------------------------------------------------------
QAbstractItemModel* qCTKModelTester::model()const
{
  return qctk_d()->Model;
}

//-----------------------------------------------------------------------------
void qCTKModelTester::setThrowOnError(bool throwException)
{
  qctk_d()->ThrowOnError = throwException;
}

//-----------------------------------------------------------------------------
bool qCTKModelTester::throwOnError()const
{
  return qctk_d()->ThrowOnError;
}

//-----------------------------------------------------------------------------
void  qCTKModelTester::test(bool result, const QString& errorString)const
{
  if (result)
    {
    return;
    }
  qDebug() << errorString;
  if (this->throwOnError())
    {
    throw errorString;
    }
}

//-----------------------------------------------------------------------------
void qCTKModelTester::testModelIndex(const QModelIndex& index)const
{
  QCTK_D(const qCTKModelTester);
  if (!index.isValid())
    {// invalid index
    this->test(index.model() == 0, "An invalid index can't have a valid model.");
    this->test(index.model() != d->Model, "An invalid index can't have a valid model.");
    this->test(index.column() == -1, "An invalid index can't have a valid column.");
    this->test(index.row() == -1, "An invalid index can't have a valid row.");
    this->test(index.parent().isValid() == false, "An invalid index can't have a valid row.");
    this->test(index.row() == -1, "An invalid index can't have a valid row.");
    for(int i = 0; i < 100; ++i)
      {
      this->test(index.sibling(i % 10, i / 10).isValid() == false, "An invalid index can't have valid sibling.");
      }
    this->testParent(index);
    }
  else
    {// valid index
    this->test(index.model() == d->Model, "A valid index must have a valid model.");
    this->test(index.column() >= 0, "An valid index can't have an invalid column.");
    this->test(index.row() >= 0, "An valid index can't have an invalid row.");
    this->test(index == index.sibling(index.row(), index.column()), "Index's row and/or column is wrong.");

    this->testData(index);
    this->testParent(index);
    
    }
}

//-----------------------------------------------------------------------------
void qCTKModelTester::testData(const QModelIndex& index)const
{
  if (!index.isValid())
    {
    this->test(!index.data(Qt::DisplayRole).isValid(), "An valid index can't have an invalid data.");
    }
  else
    {
    this->test(index.data(Qt::DisplayRole).isValid(), "An valid index can't have an invalid data.");
    }
}

//-----------------------------------------------------------------------------
void qCTKModelTester::testParent(const QModelIndex& parent)const
{
  QCTK_D(const qCTKModelTester);
  if (!d->Model->hasChildren(parent))
    {
    // it's asking a lot :-)
    //this->test(d->Model->columnCount(parent) <= 0, "A parent with no children can't have a columnCount > 0.");
    this->test(d->Model->rowCount(parent) <= 0, "A parent with no children can't have a rowCount > 0.");
    }
  else
    {
    this->test(d->Model->columnCount(parent) > 0, "A parent with children can't have a columnCount <= 0.");
    this->test(d->Model->rowCount(parent) > 0, "A parent with children can't have a rowCount <= 0.");
    }

  if (!parent.isValid())
    {// otherwise there will be an infinite loop
    return;
    }
  
  for (int i = 0 ; i < d->Model->rowCount(parent); ++i)
    {
    for (int j = 0; j < d->Model->columnCount(parent); ++j)
      {
      this->test(d->Model->hasIndex(i, j, parent), "hasIndex should return true for int range {0->rowCount(), 0->columnCount()}");
      QModelIndex child = parent.child(i, j);
      this->test(child.parent() == parent, "A child's parent can't be different from its parent");
      this->testModelIndex(child);
      }
    }
}
//-----------------------------------------------------------------------------
void qCTKModelTester::testPersistentModelIndex(const QPersistentModelIndex& index)const
{
  QCTK_D(const qCTKModelTester);
  //qDebug() << "Test persistent Index: " << index ;
  this->test(index.isValid(), "Persistent model index can't be invalid");
  this->test(d->Model->index(index.row(), index.column(), index.parent()) == index, 
             QString("Persistent index (%1, %2) can't be invalid").arg(index.row(), index.column()));
}

//-----------------------------------------------------------------------------
void qCTKModelTester::testModel()const
{
  QCTK_D(const qCTKModelTester);
  if (d->Model == 0)
    {
    return;
    }
  for (int i = 0 ; i < d->Model->rowCount(); ++i)
    {
    for (int j = 0; j < d->Model->columnCount(); ++j)
      {
      this->test(d->Model->hasIndex(i, j), "hasIndex should return true for int range {0->rowCount(), 0->columnCount()}");
      QModelIndex child = d->Model->index(i, j);
      this->test(!child.parent().isValid(), "A child's parent can't be different from its parent");
      this->testModelIndex(child);
      }
    }
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onColumnsAboutToBeInserted(const QModelIndex & parent, int start, int end)
{
  //qDebug() << "columnsAboutToBeInserted: " << parent << start << end;
  this->onItemsAboutToBeInserted(parent, Qt::Horizontal, start, end);
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onColumnsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
  //qDebug() << "columnsAboutToBeRemoved: " << parent << start << end;
  this->onItemsAboutToBeRemoved(parent, Qt::Horizontal, start, end);
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onColumnsInserted(const QModelIndex & parent, int start, int end)
{
  //qDebug() << "columnsInserted: " << parent << start << end;
  this->onItemsInserted(parent, Qt::Horizontal, start, end);
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onColumnsRemoved(const QModelIndex & parent, int start, int end)
{
  //qDebug() << "columnsRemoved: " << parent << start << end;
  this->onItemsRemoved(parent, Qt::Horizontal, start, end);
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
  this->test(topLeft.parent() == bottomRight.parent(), "DataChanged support items with the same parent only");
  this->test(topLeft.row() >= bottomRight.row(), "topLeft can't have a row lower than bottomRight");
  this->test(bottomRight.column() >= topLeft.column(), "topLeft can't have a column lower than bottomRight");
  for (int i = topLeft.row(); i <= bottomRight.row(); ++i)
    {
    for (int j = topLeft.column(); j < bottomRight.column(); ++j)
      {
      this->test(topLeft.sibling(i,j).isValid(), "Changed data must be valid");
      // do the test on the indexes here, it's easier to debug than in testModel();
      this->testModelIndex(topLeft.sibling(i,j));
      }
    }
  this->testModel();
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
  QCTK_D(qCTKModelTester);
  this->test(first <= last, "Changed headers have wrong indexes");
  switch (orientation)
    {
    case Qt::Horizontal:
      this->test(d->Model->columnCount() > last, "There is no more horizontal headers than columns.");
      break;
    case Qt::Vertical:
      this->test(d->Model->rowCount() > last, "There is no more vertical headers than rows.");
      break;
    default:
      this->test(orientation == Qt::Horizontal || orientation == Qt::Vertical, "Wrong orientation.");
      break;
    }
  this->testModel();
}

//-----------------------------------------------------------------------------
QList<QPersistentModelIndex> qCTKModelTester::persistentModelIndexes(const QModelIndex& index)const
{
  QCTK_D(const qCTKModelTester);
  QList<QPersistentModelIndex> list;
  for (int i = 0; i < d->Model->rowCount(index); ++i)
    {
    for (int j = 0; j < d->Model->columnCount(index); ++j)
      {
      QPersistentModelIndex child = d->Model->index(i, j, index);
      list.append(child);
      list += this->qCTKModelTester::persistentModelIndexes(child);
      }
    }
  return list;
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onLayoutAboutToBeChanged()
{
  QCTK_D(qCTKModelTester);

  d->LayoutAboutToBeChanged = this->persistentModelIndexes(QModelIndex());
  this->testModel();
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onLayoutChanged()
{
  QCTK_D(qCTKModelTester);
  foreach (const QPersistentModelIndex& index, d->LayoutAboutToBeChanged)
    {
    this->testPersistentModelIndex(index);
    }
  d->LayoutAboutToBeChanged.clear();
  this->testModel();
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onModelAboutToBeReset()
{
  this->testModel();
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onModelReset()
{
  this->testModel();
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
  //qDebug() << "rowsAboutToBeInserted: " << parent << start << end;
  this->onItemsAboutToBeInserted(parent, Qt::Vertical, start, end);
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
  //qDebug() << "rowsAboutToBeRemoved: " << parent << start << end;
  this->onItemsAboutToBeRemoved(parent, Qt::Vertical, start, end);
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onRowsInserted(const QModelIndex & parent, int start, int end)
{
  //qDebug() << "rowsInserted: " << parent << start << end;
  this->onItemsInserted(parent, Qt::Vertical, start, end);
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onRowsRemoved(const QModelIndex & parent, int start, int end)
{
  //qDebug() << "rowsRemoved: " << parent << start << end;
  this->onItemsRemoved(parent, Qt::Vertical, start, end);
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onItemsAboutToBeInserted(const QModelIndex &parent, Qt::Orientation orientation, int start, int end)
{
  QCTK_D(qCTKModelTester);
  this->test(start <= end, "Start can't be higher than end");
  //Not sure about that
  this->test(d->AboutToBeInserted.size() == 0, "While inserting items, you can't insert other items.");
  //Not sure about that
  this->test(d->AboutToBeRemoved.size() == 0, "While removing items, you can't insert other items.");

  qCTKModelTesterPrivate::Change change;
  change.Parent = parent;
  change.Orientation = orientation;
  change.Start = start;
  change.End = end;
  change.Count = (orientation == Qt::Vertical ? d->Model->rowCount(parent) :d->Model->columnCount(parent) );
  change.Items = this->persistentModelIndexes(parent);
  d->AboutToBeInserted.push(change);
  
  this->testModel();
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onItemsAboutToBeRemoved(const QModelIndex &parent, Qt::Orientation orientation, int start, int end)
{
  QCTK_D(qCTKModelTester);
  this->test(start <= end, "Start can't be higher than end");
  //Not sure about that
  this->test(d->AboutToBeInserted.size() == 0, "While inserting items, you can't remove other items.");
  //Not sure about that
  this->test(d->AboutToBeRemoved.size() == 0, "While removing items, you can't remove other items.");
  
  int count = (orientation == Qt::Vertical ? d->Model->rowCount(parent) :d->Model->columnCount(parent) );
  this->test(start < count, "Item to remove can't be invalid");
  this->test(end < count, "Item to remove can't be invalid");
  
  qCTKModelTesterPrivate::Change change;
  change.Parent = parent;
  change.Orientation = orientation;
  change.Start = start;
  change.End = end;
  change.Count = count;
  for (int i = 0 ; i < count; ++i)
    {
    QPersistentModelIndex index;
    index = (orientation == Qt::Vertical ? d->Model->index(i, 0, parent) : d->Model->index(0, i, parent));
    this->test(index.isValid(), "Index invalid");
    if (orientation == Qt::Vertical && (index.row() < start || index.row() > end))
      {
      change.Items.append(index);
      }
    if (orientation == Qt::Horizontal && (index.column() < start || index.column() > end))
      {
      change.Items.append(index);
      }
    }
  d->AboutToBeRemoved.push(change);

  this->testModel();
  //qDebug() << "About to be removed: " << start << " " << end <<parent << count << change.Items.count();
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onItemsInserted(const QModelIndex & parent, Qt::Orientation orientation, int start, int end)
{
  QCTK_D(qCTKModelTester);
  this->test(start <= end, "Start can't be higher end");
  this->test(d->AboutToBeInserted.size() != 0, "rowsInserted() has been emitted, but not rowsAboutToBeInserted.");
  //Not sure about that
  this->test(d->AboutToBeRemoved.size() == 0, "While removing items, you can't insert other items.");

  qCTKModelTesterPrivate::Change change = d->AboutToBeInserted.pop();
  this->test(change.Parent == parent, "Parent can't be different");
  this->test(change.Orientation == Qt::Vertical, "Orientation can't be different");
  this->test(change.Start == start, "Start can't be different");
  this->test(change.End == end, "End can't be different");
  int count =  (orientation == Qt::Vertical ? d->Model->rowCount(parent) :d->Model->columnCount(parent) );
  this->test(change.Count < count, "The new count number can't be lower");
  this->test(count - change.Count == (end - start + 1) , "The new count number can't be lower");
  foreach(const QPersistentModelIndex& index, change.Items)
    {
    this->testPersistentModelIndex(index);
    }
  change.Items.clear();
  
  this->testModel();
}

//-----------------------------------------------------------------------------
void qCTKModelTester::onItemsRemoved(const QModelIndex & parent, Qt::Orientation orientation, int start, int end)
{ 
  QCTK_D(qCTKModelTester);
  this->test(start <= end, "Start can't be higher end");
  this->test(d->AboutToBeRemoved.size() != 0, "rowsRemoved() has been emitted, but not rowsAboutToBeRemoved.");
  //Not sure about that
  this->test(d->AboutToBeInserted.size() == 0, "While inserted items, you can't remove other items.");

  qCTKModelTesterPrivate::Change change = d->AboutToBeRemoved.pop();
  this->test(change.Parent == parent, "Parent can't be different");
  this->test(change.Orientation == Qt::Vertical, "Orientation can't be different");
  this->test(change.Start == start, "Start can't be different");
  this->test(change.End == end, "End can't be different");
  int count = (orientation == Qt::Vertical ? d->Model->rowCount(parent) :d->Model->columnCount(parent) );
  this->test(change.Count > count, "The new count number can't be higher");
  this->test(change.Count - count == (end - start + 1) , "The new count number can't be higher");
  foreach(const QPersistentModelIndex& index, change.Items)
    {
    this->testPersistentModelIndex(index);
    }
  change.Items.clear();
  
  this->testModel();
}

