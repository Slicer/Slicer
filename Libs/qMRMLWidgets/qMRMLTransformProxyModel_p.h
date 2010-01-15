#ifndef __qMRMLTransformProxyModel_p_h
#define __qMRMLTransformProxyModel_p_h

// qCTK includes 
#include <qCTKPimpl.h>

// qMRML includes
#include "qMRMLTransformProxyModel.h"
#include "qMRMLItemHelper.h"

// QT includes
#include <QObject>
#include <QVector>
#include <QStack>
#include <QSharedPointer>

class qMRMLAbstractItemHelper;
class vtkObject;

//------------------------------------------------------------------------------
class qMRMLTransformProxyModelPrivate: public QObject, public qCTKPrivate<qMRMLTransformProxyModel>
{
  Q_OBJECT
public:
  QCTK_DECLARE_PUBLIC(qMRMLTransformProxyModel);
  qMRMLTransformProxyModelPrivate(QObject* parent = 0);

  QModelIndex indexFromItem(const qMRMLAbstractItemHelper* itemHelper)const;
  qMRMLAbstractItemHelper* proxyItemFromIndex(const QModelIndex &index)const;
  qMRMLAbstractItemHelper* sourceItemFromIndex(const QModelIndex &index)const;
  
  qMRMLAbstractItemHelper* createItemFromVTKObject(vtkObject* object, int column = -1);
  qMRMLAbstractItemHelper* createItemFromUID(QVariant uid, int column = -1);

  int actualRow(const qMRMLAbstractItemHelper* item)const;
  int oldRow(const qMRMLAbstractItemHelper* parent, int row)const;
  int actualRowCount(const qMRMLAbstractItemHelper* item)const;

  QVector<QSharedPointer<qMRMLAbstractItemHelper> > proxyItemsFromSourceIndexes(const QModelIndex &parent, int start, int end)const;
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
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > ItemsAboutToBeInserted;
  QVector<QSharedPointer<qMRMLAbstractItemHelper> > ItemsAboutToBeRemoved;
  vtkObject* ObjectToRemove;
};

#endif
