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
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLWidgetsExport.h"

class qMRMLTreeViewPrivate;
class vtkMRMLNode;
class vtkMRMLScene;

/// \todo Rename to qMRMLSceneTreeView
/// In debug mode, pressing the '!' key on the view switches of
/// qMRMLSortFilterProxyModel::filterType, it can be useful to debug the scene model
/// and filters applied to them.
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
  /// Transform by default
  /// \sa setSceneModel()
  Q_PROPERTY(QString sceneModelType READ sceneModelType WRITE setSceneModelType)
  /// This property controls whether to actively listen to the nodes
  /// to synchronize their representation. As it can be time consuming, you
  /// can disable it if you want a lazy update.
  /// OnlyVisibleNodes by default.
  /// \sa qMRMLSceneModel::listenNodeModifiedEvent
  Q_PROPERTY(qMRMLSceneModel::NodeTypes listenNodeModifiedEvent READ listenNodeModifiedEvent WRITE setListenNodeModifiedEvent)
  /// This property controls which node types are visible in the view.
  /// This behaves as a filter, the nodes that have a type not included in the
  /// list will be hidden.
  /// For example, a value of "qMRMLSceneDisplayableModel", "vtkMRMLModelNode"
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
  /// This property controls whether the scene is visible (is a top-level item).
  /// It doesn't have any effect if \a rootNode() is not null.
  /// Visible by default.
  /// \sa setShowScene(), showScene(),
  ///  showRootNode, setRootNode(), setRootIndex()
  Q_PROPERTY(bool showScene READ showScene WRITE setShowScene)
  /// This property controls whether the root node if any is visible.
  /// When the root node is visible, it appears as a top-level item, if it is
  /// hidden only its children are top-level items.
  /// It doesn't have any effect if \a rootNode() is null.
  /// Hidden by default.
  /// Don't use qMRMLSortFilterProxyModel::HideNodesUnaffiliatedWithNodeID if
  /// showRootNode is true, it is internally being used.
  /// \sa setShowRootNode(), showRootNode(),
  ///  showScene, setRootNode(), setRootIndex()
  Q_PROPERTY(bool showRootNode READ showRootNode WRITE setShowRootNode)

public:
  typedef QTreeView Superclass;

  qMRMLTreeView(QWidget *parent=nullptr);
  ~qMRMLTreeView() override;

  vtkMRMLScene* mrmlScene()const;

  /// Return a pointer to the current node (not necessarily selected),
  /// 0 if no node is current
  /// \sa QItemSelectionModel::currentIndex(), QItemSelectionModel::selectedIndexes()
  vtkMRMLNode* currentNode()const;

  /// Could be "", "Transform", "Displayable" or the type
  /// associated with the scene model added with qMRMLTreeView::setSceneModel()
  /// \sa setSceneModel()
  QString sceneModelType()const;

  /// Set the node types on the scene model.
  /// \sa listenNodeModifiedEvent qMRMLSceneModel::setListenNodeModifiedEvent
  void setListenNodeModifiedEvent(qMRMLSceneModel::NodeTypes listen);
  /// Get the node types of the scene model.
  /// \sa qMRMLSceneModel::listenNodeModifiedEvent
  qMRMLSceneModel::NodeTypes listenNodeModifiedEvent()const;

  /// Customize the model
  void setSceneModel(qMRMLSceneModel* newSceneModel, const QString& modelType);
  /// Customize the filter model
  void setSortFilterProxyModel(qMRMLSortFilterProxyModel* newFilterModel);

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

  /// Set the show root node flag.
  /// \sa showRootNode, showRootNode()
  void setShowRootNode(bool show);
  /// Return the show root node flag.
  /// \sa showRootNode, setShowRootNode()
  bool showRootNode()const;

  /// Set the show scene flag.
  /// \sa showScene, showScene()
  void setShowScene(bool show);
  /// Return the show scene flag.
  /// \sa showScene, setShowScene()
  bool showScene()const;

  /// Return the root node of the tree.
  /// \sa setRootNode(), showRootNode
  vtkMRMLNode* rootNode()const;

  /// Retrieve the sortFilterProxyModel used to filter/sort
  /// the nodes.
  /// The returned value can't be null.
  Q_INVOKABLE qMRMLSortFilterProxyModel* sortFilterProxyModel()const;
  Q_INVOKABLE qMRMLSceneModel* sceneModel()const;

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

  QSize minimumSizeHint()const override;
  QSize sizeHint()const override;

  void showEvent(QShowEvent* show) override;
  bool eventFilter(QObject* object, QEvent* e) override;

  virtual bool clickDecoration(const QModelIndex& index);

  using QTreeView::scrollTo;

public slots:
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// If the modelType doesn't match any known model, nothing
  /// will happen
  void setSceneModelType(const QString& modelType);

  /// Similar to setRootIndex(QModelIndex) but observe the ModifiedEvent of
  /// the node to stay in sync.
  /// A null node (default) means QModelIndex() is the root index.
  /// \sa rootNode(), setRootIndex(), showRootNode
  void setRootNode(vtkMRMLNode* root);

  /// Change the current view node to \a node.
  /// \sa currentNode
  void setCurrentNode(vtkMRMLNode* node);
  void deleteCurrentNode();
  void editCurrentNode();
  void renameCurrentNode();

  /// Bypass all the filters on the view and show all the nodes.
  /// \sa setHideAll(), qMRMLSortFilterProxyModel::FilterType
  inline void setShowAll(bool);
  /// Bypass all the filters on the view and hide all the nodes.
  /// \sa showAll(), setDontHideAll(), qMRMLSortFilterProxyModel::FilterType
  inline void setHideAll(bool);
  /// Convenient slot to call hideAll() with the opposite value.
  /// \sa setHideAll()
  inline void setDontHideAll(bool);

  /// Search the nodes matching displayName; if found, ensure a matching
  /// node is visible and current. If the current node matches displayName, then
  /// make sure it is visible (by scrolling and expanding as needed). If the
  /// current node does not match displayName, then make the first matching node
  /// current and visible. This slot can be connected as-is with a QLineEdit or
  /// a ctkSearchBox textChanged(QString) signal.
  /// \sa scrollToNext()
  void scrollTo(const QString& displayName);
  /// Repeat the last scrollTo() command, and if more than one node was matching
  /// select the next node.
  /// This slot can be connected as-is with a QLineEdit or a ctkSearchBox
  /// returnPressed() signal.
  /// \sa scrollTo()
  void scrollToNext();

signals:
  void currentNodeChanged(vtkMRMLNode* node);
  void currentNodeDeleted(const QModelIndex& index);
  void currentNodeRenamed(const QString& newName);
  void editNodeRequested(vtkMRMLNode* node);
  void decorationClicked(const QModelIndex&);

protected slots:
  /// This slot is being triggered when the current node has changed.
  /// \sa currentNodeChanged()
  virtual void onSelectionChanged(const QItemSelection & selected,
                                  const QItemSelection & deselected);

  void onNumberOfVisibleIndexChanged();

  void updateRootNode(vtkObject* modifiedRootNode);

  /// Save the nodes currently expanded nodes so that their state can later
  /// be restored by \a loadTreeExpandState(). Successive calls to
  /// \a saveTreeExpandState() erase previous tree expand state.
  /// \sa loadTreeExpandState()
  void saveTreeExpandState();

  /// Expand the nodes previously saved by \a saveTreeExpandState()
  /// \sa saveTreeExpandState()
  void loadTreeExpandState();

  void onCustomContextMenu(const QPoint& point);

protected:
  qMRMLTreeView(qMRMLTreeViewPrivate* pimpl, QWidget *parent=nullptr);

protected:
  QScopedPointer<qMRMLTreeViewPrivate> d_ptr;

  // reimplemented for performance issues
  void updateGeometries() override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

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

// --------------------------------------------------------------------------
void qMRMLTreeView::setShowAll(bool show)
{
  this->sortFilterProxyModel()->setShowAll(show);
}

// --------------------------------------------------------------------------
void qMRMLTreeView::setHideAll(bool hide)
{
  this->sortFilterProxyModel()->setHideAll(hide);
}
#include <QDebug>
// --------------------------------------------------------------------------
void qMRMLTreeView::setDontHideAll(bool dontHide)
{
  qDebug() << "DontHide: " << dontHide;
  this->setHideAll(!dontHide);
}

#endif
