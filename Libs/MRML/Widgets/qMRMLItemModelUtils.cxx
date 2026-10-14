/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "qMRMLItemModelUtils.h"

// Qt includes
#include <QDebug>
#include <QModelIndex>
#include <QSignalBlocker>
#include <QStandardItem>
#include <QStandardItemModel>

//------------------------------------------------------------------------------
void qMRMLItemModelUtils::insertRows(QStandardItem* parent, int row, const QList<QList<QStandardItem*>>& rows)
{
  if (!parent || rows.isEmpty())
  {
    return;
  }
  QStandardItemModel* model = parent->model();
  if (!model)
  {
    // The parent is not in a model, therefore no notifications are sent
    for (const QList<QStandardItem*>& rowItems : rows)
    {
      parent->insertRow(row++, rowItems);
    }
    return;
  }

  // Make sure the parent has enough columns. Column insertion is notified normally.
  int columnCount = parent->columnCount();
  for (const QList<QStandardItem*>& rowItems : rows)
  {
    columnCount = qMax(columnCount, static_cast<int>(rowItems.size()));
  }
  if (parent->columnCount() < columnCount)
  {
    parent->setColumnCount(columnCount);
  }

  const QModelIndex parentIndex = (parent == model->invisibleRootItem() ? QModelIndex() : parent->index());
  const int first = row;
  const int last = row + rows.size() - 1;

  // QStandardItem::insertRows(int, int) notifies about the insertion of empty rows, and setChild notifies about
  // each item change. Instead, the insertion is notified once, before the empty rows are inserted (rowsAboutToBeInserted)
  // and after all the items are set (rowsInserted). These signals are reserved for the model, therefore they are
  // invoked via the meta-object system. The model still updates its persistent indexes when the rows are inserted
  // (signal blocking only prevents emitting the signals).
  if (!QMetaObject::invokeMethod(model, "rowsAboutToBeInserted", Qt::DirectConnection, Q_ARG(QModelIndex, parentIndex), Q_ARG(int, first), Q_ARG(int, last)))
  {
    qCritical() << Q_FUNC_INFO << ": failed to notify about row insertion, insert rows one by one";
    for (const QList<QStandardItem*>& rowItems : rows)
    {
      parent->insertRow(row++, rowItems);
    }
    return;
  }
  {
    const QSignalBlocker blocker(model);
    parent->insertRows(first, rows.size());
    for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
    {
      const QList<QStandardItem*>& rowItems = rows[rowIndex];
      for (int column = 0; column < rowItems.size(); ++column)
      {
        // setChild sets the model in the item and all its children
        parent->setChild(first + rowIndex, column, rowItems[column]);
      }
    }
  }
  QMetaObject::invokeMethod(model, "rowsInserted", Qt::DirectConnection, Q_ARG(QModelIndex, parentIndex), Q_ARG(int, first), Q_ARG(int, last));
}
