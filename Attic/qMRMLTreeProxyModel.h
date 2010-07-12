#ifndef __qMRMLTreeProxyModel_h
#define __qMRMLTreeProxyModel_h

// Qt includes
#include <QAbstractProxyModel>

// CTK includes
#include "ctkPimpl.h"

#include "qMRMLWidgetsExport.h"
//#include "qMRMLItemModel.h"

class vtkMRMLScene;
class vtkMRMLNode;
class qMRMLTreeProxyModelPrivate;
class vtkObject;
class qMRMLAbstractItemHelper;
class qMRMLAbstractItemHelperFactory;

class QMRML_WIDGETS_EXPORT qMRMLTreeProxyModel : public QAbstractProxyModel
{
  Q_OBJECT

public:
  qMRMLTreeProxyModel(QObject *parent=0);
  virtual ~qMRMLTreeProxyModel();

  vtkMRMLScene* mrmlScene()const;
  vtkMRMLNode*  mrmlNode(const QModelIndex& index)const;

  virtual QModelIndex mapFromSource(const QModelIndex & sourceIndex)const;
  virtual QModelIndex mapToSource(const QModelIndex & proxyIndex)const;
  virtual void setSourceModel(QAbstractItemModel * sourceModel);

  virtual int columnCount(const QModelIndex &) const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)const;
  virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
  virtual Qt::ItemFlags flags(const QModelIndex &index)const;
  virtual bool hasChildren(const QModelIndex &parent=QModelIndex())const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole)const;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex())const;
  //virtual bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());
  virtual QMap<int, QVariant> itemData(const QModelIndex &index)const;
  //virtual QMimeData * mimeData(const QModelIndexList &indexes)const;
  //virtual QStringList mimeTypes()const;
  virtual QModelIndex parent(const QModelIndex &index)const;

  //virtual bool removeColumns(int column, int count, const QModelIndex &parent=QModelIndex());
  //virtual bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex());
  virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
  //virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
  virtual Qt::DropActions supportedDropActions()const;

public slots:
  /// Utility slot that propagate the scene to the source (qMRMLSourceModel)
  void setMRMLScene(vtkMRMLScene* scene);

protected:
  friend class qMRMLSortFilterProxyModel;
  /// if you are not sure what model the index belongs to (proxy or source)
  /// you can use this function.
  virtual qMRMLAbstractItemHelper* item(const QModelIndex& index)const;
  /// if you know for sure it is a proxy model index you can use this function
  /// otherwise, use item(const QModelIndex& index)const
  virtual qMRMLAbstractItemHelper* proxyItem(const QModelIndex& index)const;
  /// The factory that is used to create all the items of the model.
  /// The subclass is responsible for deleting the itemFactory memory.
  virtual qMRMLAbstractItemHelperFactory* itemFactory()const = 0;
  //virtual qMRMLAbstractItemHelper* itemFromVTKObject(vtkObject* object, int column)const =0;
  //virtual qMRMLAbstractRootItemHelper* rootItem(vtkMRMLScene* scene)const =0;
  
private:
  qMRMLAbstractItemHelperFactory* sourceItemFactory()const;
  CTK_DECLARE_PRIVATE(qMRMLTreeProxyModel);
};

#endif
