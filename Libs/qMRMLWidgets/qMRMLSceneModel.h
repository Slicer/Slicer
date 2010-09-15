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

#ifndef __qMRMLSceneModel_h
#define __qMRMLSceneModel_h

// Qt includes
#include <QAbstractListModel>

// CTK includes 
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"
#include "qMRMLItemHelper.h"

class vtkMRMLNode;
class vtkMRMLScene;
class QAction;

namespace qMRML
{
 enum ItemDataRole {
   UIDRole = Qt::UserRole
 };
};

class qMRMLSceneModelItemHelperFactoryPrivate;
//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSceneModelItemHelperFactory : public qMRMLAbstractItemHelperFactory
{
public:
  qMRMLSceneModelItemHelperFactory();
  virtual ~qMRMLSceneModelItemHelperFactory();
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object,int column, int row = -1) const;

  void setPreItems(vtkCollection* itemCollection);
  vtkCollection* preItems()const;
  void setPostItems(vtkCollection* itemCollection);
  vtkCollection* postItems()const;
protected:
  QScopedPointer<qMRMLSceneModelItemHelperFactoryPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSceneModelItemHelperFactory);
  Q_DISABLE_COPY(qMRMLSceneModelItemHelperFactory);
};

class qMRMLSceneModelPrivate;


//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSceneModel : public QAbstractItemModel
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY (bool listenNodeModifiedEvent READ listenNodeModifiedEvent WRITE setListenNodeModifiedEvent)
public:
  typedef QAbstractItemModel Superclass;
  qMRMLSceneModel(QObject *parent=0);
  virtual ~qMRMLSceneModel();

  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene* mrmlScene()const;
  QModelIndex mrmlSceneIndex()const;

  /// Return the vtkMRMLNode associated to the node index.
  /// 0 if the node index is not a MRML node (i.e. vtkMRMLScene, extra item...)
  vtkMRMLNode* mrmlNode(const QModelIndex &nodeIndex)const;
  QModelIndexList indexes(vtkMRMLNode* node)const;

  /// Option that activates the expensive listening of the vtkMRMLNode Modified
  /// events. When listening, the signal itemDataChanged() is fired when a
  /// vtkMRMLNode is modified.False by default.
  void setListenNodeModifiedEvent(bool listen);
  bool listenNodeModifiedEvent()const;

  ///
  /// Extra items that are prepended to the node list
  /// Warning, setPreItems() resets the model, the currently selected item is lost
  void setPreItems(vtkObject* parent, const QStringList& extraItems);
  QStringList preItems(vtkObject* parent)const;

  ///
  /// Extra items that are appended to the node list
  /// Warning, setPostItems() resets the model, the currently selected item is lost
  void setPostItems(vtkObject* parent, const QStringList& extraItems);
  QStringList postItems(vtkObject* parent)const;

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
  virtual void onMRMLSceneNodeAboutToBeAdded(vtkMRMLScene* scene, vtkMRMLNode* node);
  virtual void onMRMLSceneNodeAboutToBeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node);
  virtual void onMRMLSceneNodeAdded(vtkMRMLScene* scene, vtkMRMLNode* node);
  virtual void onMRMLSceneNodeRemoved(vtkMRMLScene* scene, vtkMRMLNode* node);

  virtual void onMRMLSceneAboutToBeImported(vtkMRMLScene* scene);
  virtual void onMRMLSceneImported(vtkMRMLScene* scene);
  virtual void onMRMLSceneAboutToBeClosed(vtkMRMLScene* scene);
  virtual void onMRMLSceneClosed(vtkMRMLScene* scene);

  void onMRMLSceneDeleted(vtkObject* scene);

  void onMRMLNodeModified(vtkObject* node);
protected:
  qMRMLSceneModel(qMRMLSceneModelItemHelperFactory* factory, QObject *parent=0);
  friend class qMRMLSortFilterProxyModel;
  friend class qMRMLTreeProxyModel;
  virtual qMRMLAbstractItemHelperFactory* itemFactory()const;
  virtual qMRMLAbstractItemHelper* itemFromIndex(const QModelIndex &modelIndex)const;
  virtual qMRMLAbstractItemHelper* itemFromObject(vtkObject* object, int column)const;
  virtual QModelIndex indexFromItem(const qMRMLAbstractItemHelper* item)const;
  static void DoCallback(vtkObject* vtk_obj, unsigned long event,
                         void* client_data, void* call_data);
protected:
  QScopedPointer<qMRMLSceneModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSceneModel);
  Q_DISABLE_COPY(qMRMLSceneModel);
};

#endif
