#ifndef __qMRMLItemModel_h
#define __qMRMLItemModel_h

#include <QAbstractItemModel>
#include "qCTKPimpl.h"
#include "qMRMLWidgetsWin32Header.h"

class vtkMRMLScene;
class vtkMRMLNode;
class qMRMLItemModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  qMRMLItemModel(QObject *parent=0);
  virtual ~qMRMLItemModel();
                           
  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene* mrmlScene()const;
  
  // doesn't cost anything
  vtkMRMLNode* mrmlNodeFromIndex(const QModelIndex &index)const;
  // cost a lot as we go through the tree
  QModelIndex indexFromMRMLNode(vtkMRMLNode* node, int column = 0)const;

  virtual int columnCount(const QModelIndex &parent=QModelIndex())const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)const;
  virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
  virtual Qt::ItemFlags flags(const QModelIndex &index)const;
  virtual bool hasChildren(const QModelIndex &parent=QModelIndex())const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole)const;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex())const;
  //virtual bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());
  //virtual QMap<int, QVariant> itemData(const QModelIndex &index)const;
  virtual QMimeData * mimeData(const QModelIndexList &indexes)const;
  virtual QStringList mimeTypes()const;
  virtual QModelIndex parent(const QModelIndex &index)const;
  //virtual bool removeColumns(int column, int count, const QModelIndex &parent=QModelIndex());
  //virtual bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex());
  virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
  //virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
  virtual Qt::DropActions supportedDropActions()const;
private:
  CTK_DECLARE_PRIVATE(qMRMLItemModel);
};

#endif
