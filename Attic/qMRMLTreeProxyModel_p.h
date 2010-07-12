#ifndef __qMRMLTreeProxyModel_p_h
#define __qMRMLTreeProxyModel_p_h

/// Qt includes
#include <QObject>
#include <QVector>
#include <QStack>
#include <QSharedPointer>

/// CTK includes
#include <ctkPimpl.h>

/// qMRML includes
#include "qMRMLTreeProxyModel.h"
#include "qMRMLItemHelper.h"

class qMRMLAbstractItemHelper;
class vtkObject;

//------------------------------------------------------------------------------
class qMRMLTreeProxyModelPrivate: public QObject, public ctkPrivate<qMRMLTreeProxyModel>
{
  Q_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLTreeProxyModel);
  qMRMLTreeProxyModelPrivate(QObject* parent = 0);

  QModelIndex indexFromItem(const qMRMLAbstractItemHelper* itemHelper)const;
  qMRMLAbstractItemHelper* proxyItemFromIndex(const QModelIndex &index)const;
  qMRMLAbstractItemHelper* sourceItemFromIndex(const QModelIndex &index)const;
  //qMRMLAbstractItemHelper* sourceItemFromObject(vtkObject* object, int column)const;
  qMRMLAbstractItemHelper* itemFromIndex(const QModelIndex &index)const;
  
  qMRMLAbstractItemHelper* itemFromUID(QVariant uid, int column = -1);

  int rowWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const;
  int childRowWithHiddenItemsAdded(const qMRMLAbstractItemHelper* parent, int row)const;
  int rowCountWithHiddenItemsRemoved(const qMRMLAbstractItemHelper* item)const;

  QVector<QSharedPointer<qMRMLAbstractItemHelper> > proxyItemsFromSourceIndexes(const QModelIndex &parent, int start, int end)const;
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > proxyItemsFromProxyIndexes(const QModelIndex &parent, int start, int end)const;
  QStack<int> consecutiveRows(const QVector<QSharedPointer<qMRMLAbstractItemHelper> >& items ) const;

public slots:
  void onSourceColumnsAboutToBeInserted(const QModelIndex & parent, int start, int end);
  void onSourceColumnsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
  void onSourceColumnsInserted(const QModelIndex & parent, int start, int end);
  void onSourceColumnsRemoved(const QModelIndex & parent, int start, int end);
  void onSourceDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
  void onSourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);
  void onSourceLayoutAboutToBeChanged();
  void onSourceLayoutChanged();
  void onSourceModelAboutToBeReset();
  void onSourceModelReset();
  void onSourceRowsAboutToBeInserted(const QModelIndex & parent, int start, int end);
  void onSourceRowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
  void onSourceRowsInserted(const QModelIndex & parent, int start, int end);
  void onSourceRowsRemoved(const QModelIndex & parent, int start, int end);

protected:
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > HiddenItems;
  //QVector<QSharedPointer<qMRMLAbstractItemHelper> > ItemsAboutToBeInserted;
  //QVector<QSharedPointer<qMRMLAbstractItemHelper> > ItemsAboutToBeRemoved;
#ifndef QT_NO_DEBUG
  vtkObject* HiddenVTKObject;
#endif
};

#endif
