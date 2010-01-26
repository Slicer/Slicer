#ifndef __qMRMLSceneModel_h
#define __qMRMLSceneModel_h

#include <QAbstractListModel>
#include "qCTKPimpl.h"
#include "qMRMLWidgetsExport.h"
#include "qMRMLItemHelper.h"
#include "qVTKObject.h"

class vtkMRMLScene;
class vtkMRMLNode;
class qMRMLSceneModelPrivate;

namespace qMRML
{
 enum ItemDataRole {
   UIDRole = Qt::UserRole
 };
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLFlatSceneItemHelper : public qMRMLAbstractSceneItemHelper
{
public:
  qMRMLFlatSceneItemHelper(vtkMRMLScene* scene, int column = -1);
  
  virtual qMRMLAbstractItemHelper* child(int row = 0, int column = 0) const;
  virtual int childCount() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent()const;
  
protected:
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLFlatNodeItemHelper : public qMRMLAbstractNodeItemHelper
{
public:
  qMRMLFlatNodeItemHelper(vtkMRMLNode* node, int column = -1);
  virtual qMRMLAbstractItemHelper* parent() const;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLFlatRootItemHelper : public qMRMLAbstractRootItemHelper
{
public:
  qMRMLFlatRootItemHelper(vtkMRMLScene* scene);
  // child MUST be reimplemented
  virtual qMRMLAbstractItemHelper* child(int row = 0, int column = 0) const;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSceneModel : public QAbstractItemModel
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QAbstractItemModel Superclass;
  qMRMLSceneModel(QObject *parent=0);
  virtual ~qMRMLSceneModel();
  
  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene* mrmlScene()const;

  virtual int columnCount(const QModelIndex &parent=QModelIndex())const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)const;
  //virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
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

protected slots:
  void onMRMLSceneNodeAboutToBeAdded(vtkObject* scene, vtkObject* node);
  void onMRMLSceneNodeAboutToBeRemoved(vtkObject* scene, vtkObject* node);
  void onMRMLSceneNodeAdded(vtkObject* scene, vtkObject* node);
  void onMRMLSceneNodeRemoved(vtkObject* scene, vtkObject* node);
private:
  QCTK_DECLARE_PRIVATE(qMRMLSceneModel);
};

#endif
