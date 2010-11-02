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

#ifndef __qMRMLSceneTreeModel_h
#define __qMRMLSceneTreeModel_h

// Qt includes
#include <QAbstractProxyModel>

// CTK includes
#include "ctkPimpl.h"

#include "qMRMLWidgetsExport.h"
#include "qMRMLSceneModel.h"

class vtkMRMLScene;
class vtkMRMLNode;
class qMRMLSceneTreeModelPrivate;
class vtkObject;
class qMRMLAbstractItemHelper;
class qMRMLAbstractItemHelperFactory;

class QMRML_WIDGETS_EXPORT qMRMLSceneTreeModel : public qMRMLSceneModel
{
  Q_OBJECT

public:
  qMRMLSceneTreeModel(qMRMLSceneModelItemHelperFactory* factory, QObject *parent=0);
  virtual ~qMRMLSceneTreeModel();

  //virtual int columnCount(const QModelIndex &) const;
  //virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)const;
  virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
  virtual Qt::ItemFlags flags(const QModelIndex &index)const;
  virtual bool hasChildren(const QModelIndex &parent=QModelIndex())const;
  //virtual QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole)const;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex())const;
  //virtual bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());
  //virtual QMap<int, QVariant> itemData(const QModelIndex &index)const;
  //virtual QMimeData * mimeData(const QModelIndexList &indexes)const;
  //virtual QStringList mimeTypes()const;
  //virtual QModelIndex parent(const QModelIndex &index)const;

  //virtual bool removeColumns(int column, int count, const QModelIndex &parent=QModelIndex());
  //virtual bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex());
  virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
  //virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
  //virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
  virtual Qt::DropActions supportedDropActions()const;

protected:
  friend class qMRMLSortFilterProxyModel;
  /// if you are not sure what model the index belongs to (proxy or source)
  /// you can use this function.
  //virtual qMRMLAbstractItemHelper* item(const QModelIndex& index)const;
  /// if you know for sure it is a proxy model index you can use this function
  /// otherwise, use item(const QModelIndex& index)const
  //virtual qMRMLAbstractItemHelper* proxyItem(const QModelIndex& index)const;
  /// The factory that is used to create all the items of the model.
  /// The subclass is responsible for deleting the itemFactory memory.
  ///virtual qMRMLAbstractItemHelperFactory* itemFactory()const = 0;
  //virtual qMRMLAbstractItemHelper* itemFromVTKObject(vtkObject* object, int column)const =0;
  //virtual qMRMLAbstractRootItemHelper* rootItem(vtkMRMLScene* scene)const =0;
  virtual void onMRMLSceneNodeAboutToBeAdded(vtkMRMLScene* scene, vtkMRMLNode* node);
  virtual void onMRMLSceneNodeAboutToBeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node);
  virtual void onMRMLSceneNodeAdded(vtkMRMLScene* scene, vtkMRMLNode* node);
  virtual void onMRMLSceneNodeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node);

  virtual void onMRMLSceneAboutToBeImported(vtkMRMLScene* scene);
  virtual void onMRMLSceneImported(vtkMRMLScene* scene);
  virtual void onMRMLSceneAboutToBeClosed(vtkMRMLScene* scene);
  virtual void onMRMLSceneClosed(vtkMRMLScene* scene);
  virtual QModelIndex indexFromItem(const qMRMLAbstractItemHelper* item)const;

  QScopedPointer<qMRMLSceneTreeModelPrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qMRMLSceneTreeModel);
  Q_DISABLE_COPY(qMRMLSceneTreeModel);
};

#endif
