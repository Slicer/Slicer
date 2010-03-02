#ifndef __qMRMLSceneModel_h
#define __qMRMLSceneModel_h

// Qt includes
#include <QAbstractListModel>

// qCTKWidgets includes 
#include <qCTKPimpl.h>

// qVTKWidgets includes 
#include "qVTKObject.h"

// qMRMLWidgets Includes
#include "qMRMLWidgetsExport.h"
#include "qMRMLItemHelper.h"

class vtkMRMLScene;
class QAction;
class qMRMLSceneModelPrivate;

namespace qMRML
{
 enum ItemDataRole {
   UIDRole = Qt::UserRole
 };
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

  void setExtraItems(vtkObject* parent, const QStringList& extraItems);

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
  void onMRMLSceneDeleted(vtkObject* scene);
protected:
  friend class qMRMLSortFilterProxyModel;
  friend class qMRMLTreeProxyModel;
  qMRMLAbstractItemHelperFactory* itemFactory()const;
  qMRMLAbstractItemHelper* item(const QModelIndex &modelIndex)const;
private:
  QCTK_DECLARE_PRIVATE(qMRMLSceneModel);
};

#endif
