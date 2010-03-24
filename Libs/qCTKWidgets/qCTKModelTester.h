/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKModelTester_h
#define __qCTKModelTester_h

/// QT includes
#include <QObject> 
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QList>

/// CTK includes
#include <qCTKWidgetsExport.h>
#include <qCTKPimpl.h>

#include "qCTKWidgetsExport.h"

class QAbstractItemModel;
class qCTKModelTesterPrivate;

class QCTK_WIDGETS_EXPORT qCTKModelTester: public QObject
{
  Q_OBJECT
public:
  explicit qCTKModelTester(QObject *parent = 0);
  qCTKModelTester(QAbstractItemModel *model, QObject *parent);

  void setModel(QAbstractItemModel* model);
  QAbstractItemModel* model()const;

  void setThrowOnError(bool throwException);
  bool throwOnError()const;
 
  virtual void testData(const QModelIndex& index)const;
  virtual void testModel()const;
  virtual void testModelIndex(const QModelIndex& index)const;
  virtual void testParent(const QModelIndex& parent)const;
  virtual void testPersistentModelIndex(const QPersistentModelIndex& index)const;

protected slots:
  void onColumnsAboutToBeInserted(const QModelIndex & parent, int start, int end);
  void onColumnsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
  void onColumnsInserted(const QModelIndex & parent, int start, int end);
  void onColumnsRemoved(const QModelIndex & parent, int start, int end);
  void onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
  void onHeaderDataChanged(Qt::Orientation orientation, int first, int last);
  void onLayoutAboutToBeChanged();
  void onLayoutChanged();
  void onModelAboutToBeReset();
  void onModelReset();
  void onRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
  void onRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
  void onRowsInserted(const QModelIndex & parent, int start, int end);
  void onRowsRemoved(const QModelIndex & parent, int start, int end);
protected:
  virtual void onItemsAboutToBeInserted(const QModelIndex& parent, Qt::Orientation, int start, int end);
  virtual void onItemsAboutToBeRemoved(const QModelIndex& parent, Qt::Orientation, int start, int end);
  virtual void onItemsInserted(const QModelIndex& parent, Qt::Orientation, int start, int end);
  virtual void onItemsRemoved(const QModelIndex& parent, Qt::Orientation, int start, int end);
  QList<QPersistentModelIndex> persistentModelIndexes(const QModelIndex& index)const;
  virtual void test(bool result, const QString& errorString)const;
private:
  QCTK_DECLARE_PRIVATE(qCTKModelTester);
};

#endif
