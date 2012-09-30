/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLTreeView_h
#define __qMRMLTreeView_h

// Qt includes
#include <QTreeView>
//#include <QStyledItemDelegate>
class QShowEvent;

// CTK includes
#include <ctkVTKObject.h>

// MRMLWidgets includes
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLWidgetsExport.h"

class qMRMLSceneModel;
class qMRMLTreeViewPrivate;
class vtkMRMLNode;
class vtkMRMLScene;

/// \todo Rename to qMRMLSceneTreeView
class QMRML_WIDGETS_EXPORT qMRMLTreeView : public QTreeView
{
  Q_OBJECT
  QVTK_OBJECT
  /// This property controls what type of scene representation 
  /// (which qMRMLSceneModel implementation/subclass)is used to populate
  /// the nodes of the scene.
  /// Some built-in model types are available :
  /// - "" -> qMRMLSceneModel
  /// - "Transform" -> qMRMLSceneTransformModel
  /// - "Displayable" -> qMRMLSceneDisplayableModel
  /// - "ModelHierarchy" -> qMRMLSceneModelHierarchyModel
  /// Transform by default
  /// \sa setSceneModel()
  Q_PROPERTY(QString sceneModelType READ sceneModelType WRITE setSceneModelType)
  /// This property controls whether to actively listen to the nodes
  /// to synchronize their representation. As it can be time consuming, you
  /// can disable it if you want a lazy update.
  /// True by default.
  Q_PROPERTY(bool listenNodeModifiedEvent READ listenNodeModifiedEvent WRITE setListenNodeModifiedEvent)
  /// This property controls which node types are visible in the view.
  /// This behaves as a filter, the nodes that have a type not included in the
  /// list will be hidden.
  /// For example, a value of "vtkMRMLModelHierarchyNode", "vtkMRMLModelNode"
  /// will show the model nodes. When dealing with hierarchies, please note
  /// that the node type of the parents should not be filtered out (should
  /// be included in nodeTypes) to ensure the children are visible.
  /// All nodes are visible by default (QStringList()).
  /// \sa qMRMLSortFilterProxyModel::nodeTypes
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes)
  /// This property controls whether the list auto resize to fit its size
  /// to show the number indexes without scrollbar.
  Q_PROPERTY(bool fitSizeToVisibleIndexes READ fitSizeToVisibleIndexes WRITE setFitSizeToVisibleIndexes)
  /// This property controls the minimum size of (minimum)sizeHint.
  /// When fitSizeToVisibleIndexes is true, the default size can be small if the
  /// contains few nodes. Setting \a minSizeHint ensures a minimum size of the
  /// tree minimumSizeHint and sizeHint properties.
  /// \sa fitSizeToVisibleIndexes
  Q_PROPERTY(QSize minSizeHint READ minSizeHint WRITE setMinSizeHint)
  /// This property controls whether the "Rename" context menu entry is visible
  /// Visible by default
  /// \sa deleteMenuActionVisible, editMenuActionVisible, prependNodeMenuAction()
  Q_PROPERTY(bool renameMenuActionVisible READ isRenameMenuActionVisible WRITE setRenameMenuActionVisible)
  /// This property controls whether the "Delete" context menu entry is visible
  /// Visible by default
  /// \sa renameMenuActionVisible, editMenuActionVisible, prependNodeMenuAction()
  Q_PROPERTY(bool deleteMenuActionVisible READ isDeleteMenuActionVisible WRITE setDeleteMenuActionVisible)
  /// This property controls whether the "Edit properties..." context menu entry is visible
  /// Hidden by default
  /// \sa renameMenuActionVisible, deleteMenuActionVisible, prependNodeMenuAction()
  Q_PROPERTY(bool editMenuActionVisible READ isEditMenuActionVisible WRITE setEditMenuActionVisible)
  /// This property controls whether nodes with the property
  /// vtkMRMLNode::HideFromEditors set to true are visible in the view.
  /// False by default (only nodes with HideFromEditors = 0 are visible)
  /// \sa vtkMRMLNode::GetHideFromEditors()
  Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden)

public:
  typedef QTreeView Superclass;

  qMRMLTreeView(QWidget *parent=0);
  virtual ~qMRMLTreeView();

  vtkMRMLScene* mrmlScene()const;

  /// Return a pointer to the current node (not necessarilly selected),
  /// 0 if no node is current
  /// \sa QItemSelectionModel::currentIndex(), QItemSelectionModel::selectedIndexes()
  vtkMRMLNode* currentNode()const;

  /// Could be "", "Transform", "Displayable", "ModelHierarchy" or the type
  /// associated with the scene model added with qMRMLTreeView::setSceneModel()
  /// \sa setSceneModel()
  QString sceneModelType()const;

  /// \sa qMRMLSceneModel::setListenNodeModifiedEvent
  void setListenNodeModifiedEvent(bool listen);
  /// \sa qMRMLSceneModel::listenNodeModifiedEvent
  bool listenNodeModifiedEvent()const;

  /// Customize the model
  void setSceneModel(qMRMLSceneModel* newSceneModel, const QString& modelType);

  /// Set/Get node types to display in the list
  /// NodeTypes are the class names, i.e. vtkMRMLViewNode,
  /// vtkMRMLTransformNode
  QStringList nodeTypes()const;
  void setNodeTypes(const QStringList& nodeTypes);

  /// Show/Hide the "Rename" menu item on right context menu
  /// Visible by default
  bool isRenameMenuActionVisible()const;
  void setRenameMenuActionVisible(bool show);

  /// Show/Hide the "Delete" menu item on right context menu
  /// Visible by default
  bool isDeleteMenuActionVisible()const;
  void setDeleteMenuActionVisible(bool show);

  /// Show/Hide the "Edit properties..." menu item on right context menu
  /// Hidden by default
  bool isEditMenuActionVisible()const;
  void setEditMenuActionVisible(bool show);

  /// Add a custom QAction to add into the context menu that opens with a right
  /// click on a MRML node.
  /// \sa prependNodeMenuAction(), appendSceneMenuAction()
  void appendNodeMenuAction(QAction* action);

  /// \sa prependSceneMenuAction(), appendNodeMenuAction()
  void prependNodeMenuAction(QAction* action);

  /// Add a custom QAction to add into the context menu that opens with a right
  /// click on the MRML scene.
  /// \sa appendNodeMenuAction(), prependSceneMenuAction()
  void appendSceneMenuAction(QAction* action);

  /// \sa prependNodeMenuAction()
  void prependSceneMenuAction(QAction* action);

  /// Remove action from the "NodeMenu"
  void removeNodeMenuAction(QAction* action);

  /// If a vtkMRMLNode has the property HideFromEditors set to true,
  /// bypass the property and show the node anyway.
  inline void setShowHidden(bool);
  inline bool showHidden()const;

  /// Similar to setRootIndex(QModelIndex) but observe the ModifiedEvent of
  /// the node to stay in sync.
  /// A null node (default) means QModelIndex() is the root index.
  Q_INVOKABLE void setRootNode(vtkMRMLNode* root);
  vtkMRMLNode* rootNode()const;

  /// Retrieve the sortFilterProxyModel used to filter/sort
  /// the nodes.
  /// The returned value can't be null.
  qMRMLSortFilterProxyModel* sortFilterProxyModel()const;
  qMRMLSceneModel* sceneModel()const;

  /// When true, the tree widget resize itself so that
  /// it's sizeHint is right for the visible indexes
  void setFitSizeToVisibleIndexes(bool);
  bool fitSizeToVisibleIndexes()const;

  void setMinSizeHint(QSize min);
  QSize minSizeHint()const;

  /// Return true if \a potentialAncestor is an ancestor
  /// of index
  static bool isAncestor(const QModelIndex& index, const QModelIndex& potentialAncestor);

  /// Return an ancestor in the list of potential ancestors if any, otherwise
  /// return an invalid QModelIndex
  static QModelIndex findAncestor(const QModelIndex& index, const QModelIndexList& potentialAncestors);

  /// Remove indexes that have at least one ancestor in the list
  static QModelIndexList removeChildren(const QModelIndexList& indexes);

  virtual QSize minimumSizeHint()const;
  virtual QSize sizeHint()const;

  virtual void showEvent(QShowEvent* show);
  virtual bool eventFilter(QObject* object, QEvent* e);

  virtual bool clickDecoration(const QModelIndex& index);

public slots:
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// If the modelType doesn't match any known model, nothing
  /// will happen
  void setSceneModelType(const QString& modelType);

  void deleteCurrentNode();
  void editCurrentNode();
  void renameCurrentNode();

signals:
  void currentNodeChanged(vtkMRMLNode* node);
  void currentNodeDeleted(const QModelIndex& index);
  void currentNodeRenamed(const QString& newName);
  void editNodeRequested(vtkMRMLNode* node);
  void decorationClicked(const QModelIndex&);

protected slots:
  virtual void onCurrentRowChanged(const QModelIndex& index);
  void onNumberOfVisibleIndexChanged();

  void updateRootNode(vtkObject* modifiedRootNode);
protected:
  QScopedPointer<qMRMLTreeViewPrivate> d_ptr;

  // reimplemented for performance issues
  virtual void updateGeometries();
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);

  virtual void toggleVisibility(const QModelIndex& index);

private:
  Q_DECLARE_PRIVATE(qMRMLTreeView);
  Q_DISABLE_COPY(qMRMLTreeView);
};

// --------------------------------------------------------------------------
void qMRMLTreeView::setShowHidden(bool enable)
{
  this->sortFilterProxyModel()->setShowHidden(enable);
}

// --------------------------------------------------------------------------
bool qMRMLTreeView::showHidden()const
{
  return this->sortFilterProxyModel()->showHidden();
}

#endif
