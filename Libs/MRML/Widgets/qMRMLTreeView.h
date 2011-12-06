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
#include <QStyledItemDelegate>

// CTK includes
#include <ctkPimpl.h>

// MRMLWidgets includes
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLWidgetsExport.h"

class qMRMLSceneModel;
class qMRMLTreeViewPrivate;
class vtkMRMLNode;
class vtkMRMLScene;

// TODO: Rename to qMRMLSceneTreeView
class QMRML_WIDGETS_EXPORT qMRMLTreeView : public QTreeView
{
  Q_OBJECT
  Q_PROPERTY(QString sceneModelType READ sceneModelType WRITE setSceneModelType)
  Q_PROPERTY(bool listenNodeModifiedEvent READ listenNodeModifiedEvent WRITE setListenNodeModifiedEvent)
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes)
  Q_PROPERTY(bool fitSizeToVisibleIndexes READ fitSizeToVisibleIndexes WRITE setFitSizeToVisibleIndexes)
  Q_PROPERTY(bool editMenuActionVisible READ isEditMenuActionVisible WRITE setEditMenuActionVisible)
  Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden)

public:
  qMRMLTreeView(QWidget *parent=0);
  virtual ~qMRMLTreeView();

  vtkMRMLScene* mrmlScene()const;
  
  vtkMRMLNode* currentNode()const;

  /// Could be Transform, Displayable, ModelHierarchy or ""
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

  /// Show/Hide the "Edit properties..." menu item on right context menu
  bool isEditMenuActionVisible()const;
  void setEditMenuActionVisible(bool show);
  
  /// Add a custom QAction to add into the context menu on a right click
  void prependNodeMenuAction(QAction* action);
  void prependSceneMenuAction(QAction* action);
  
  /// Remove action from the "NodeMenu"
  void removeNodeMenuAction(QAction* action);

  /// If a vtkMRMLNode has the property HideFromEditors set to true,
  /// bypass the property and show the node anyway.
  inline void setShowHidden(bool);
  inline bool showHidden()const;
  
  /// Retrieve the sortFilterProxyModel used to filter/sort
  /// the nodes.
  /// The returned value can't be null.
  qMRMLSortFilterProxyModel* sortFilterProxyModel()const;
  qMRMLSceneModel* sceneModel()const;

  /// When true, the tree widget resize itself so that
  /// it's sizeHint is right for the visible indexes
  bool fitSizeToVisibleIndexes()const;
  void setFitSizeToVisibleIndexes(bool);

  virtual QSize minimumSizeHint()const;
  virtual QSize sizeHint()const;

  virtual bool eventFilter(QObject* object, QEvent* e);

public slots:
  void setMRMLScene(vtkMRMLScene* scene);

  /// If the modelType doesn't match any known model, nothing
  /// will happen
  void setSceneModelType(const QString& modelType);

signals:
  void currentNodeChanged(vtkMRMLNode* node);
  void editNodeRequested(vtkMRMLNode* node);

protected slots:
  virtual void onCurrentRowChanged(const QModelIndex& index);
  void onNumberOfVisibleIndexChanged();
  void deleteCurrentNode();
  void editCurrentNode();
  void renameCurrentNode();

protected:
  QScopedPointer<qMRMLTreeViewPrivate> d_ptr;

  // reimplemented for performance issues
  virtual void updateGeometries();
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);

  bool onDecorationClicked(const QModelIndex& index);
  void toggleVisibility(const QModelIndex& index);

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
