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

#ifndef __qMRMLSceneViewsTreeView_h
#define __qMRMLSceneViewsTreeView_h

// Qt includes
#include <QTreeView>
// SceneViews QT includes
#include "GUI/qSlicerSceneViewsModuleWidget.h"

// CTK includes
#include <ctkPimpl.h>

// qMRML includes
#include "qMRMLTreeView.h"

#include "qSlicerSceneViewsModuleExport.h"

// Logic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

class qMRMLSortFilterProxyModel;
class qMRMLSceneViewsTreeViewPrivate;
class vtkMRMLNode;
class vtkMRMLScene;

/// \ingroup Slicer_QtModules_SceneViews
class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT qMRMLSceneViewsTreeView
  : public qMRMLTreeView
{
  Q_OBJECT
  Q_PROPERTY(QString sceneModelType READ sceneModelType WRITE setSceneModelType)
  Q_PROPERTY(bool listenNodeModifiedEvent READ listenNodeModifiedEvent WRITE setListenNodeModifiedEvent)
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes)

public:
  typedef qMRMLTreeView Superclass;
  qMRMLSceneViewsTreeView(QWidget *parent=0);
  virtual ~qMRMLSceneViewsTreeView();

  void hideScene();

  void setSelectedNode(const QString& id);

  QString firstSelectedNode()const;

  // Register the widget
  void setAndObserveWidget(qSlicerSceneViewsModuleWidget* widget);

  // Register the logic
  void setAndObserveLogic(vtkSlicerSceneViewsModuleLogic* logic);

public slots:
  void onSelectionChanged(const QItemSelection& index,const QItemSelection& beforeIndex);
  virtual void setMRMLScene(vtkMRMLScene* scene);
  void deleteSelected();

signals:
  void currentNodeChanged(vtkMRMLNode* node);
  void restoreSceneViewRequested(const QString& nodeID);
  void editSceneViewRequested(const QString& nodeID);

protected slots:
  void onClicked(const QModelIndex& index);

protected:
  QScopedPointer<qMRMLSceneViewsTreeViewPrivate> d_ptr;
  #ifndef QT_NO_CURSOR
    void mouseMoveEvent(QMouseEvent* e);
    bool viewportEvent(QEvent* e);
  #endif
  virtual void mousePressEvent(QMouseEvent* event);

private:
  Q_DECLARE_PRIVATE(qMRMLSceneViewsTreeView);
  Q_DISABLE_COPY(qMRMLSceneViewsTreeView);

  qSlicerSceneViewsModuleWidget* m_Widget;
  vtkSlicerSceneViewsModuleLogic* m_Logic;

};

#endif
