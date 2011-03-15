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

#ifndef __qMRMLSceneViewsTreeWidget_h
#define __qMRMLSceneViewsTreeWidget_h

// Qt includes
#include <QTreeView>
// Annotation QT includes
#include "GUI/qSlicerSceneViewsModuleWidget.h"

// CTK includes
#include <ctkPimpl.h>

// qMRML includes
#include "qMRMLTreeWidget.h"

#include "qSlicerSceneViewsModuleExport.h"

// Logic includes
#include <vtkSlicerSceneViewLogic.h>

class qMRMLSortFilterProxyModel;
class qMRMLSceneViewsTreeWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScene;

class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT qMRMLSceneViewsTreeWidget
  : public qMRMLTreeWidget
{
  Q_OBJECT
  Q_PROPERTY(QString sceneModelType READ sceneModelType WRITE setSceneModelType)
  Q_PROPERTY(bool listenNodeModifiedEvent READ listenNodeModifiedEvent WRITE setListenNodeModifiedEvent)
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes)

public:
  qMRMLSceneViewsTreeWidget(QWidget *parent=0);
  virtual ~qMRMLSceneViewsTreeWidget();

  void hideScene();

  void setSelectedNode(const QString& id);

  QString firstSelectedNode()const;

  // Register the widget
  void setAndObserveWidget(qSlicerSceneViewsModuleWidget* widget);

  // Register the logic
  void setAndObserveLogic(vtkSlicerSceneViewLogic* logic);

public slots:
  void setMRMLScene(vtkMRMLScene* scene);
  void deleteSelected();

signals:
  void currentNodeChanged(vtkMRMLNode* node);

protected slots:
  void onClicked(const QModelIndex& index);

protected:
  QScopedPointer<qMRMLSceneViewsTreeWidgetPrivate> d_ptr;
  #ifndef QT_NO_CURSOR
    void mouseMoveEvent(QMouseEvent* e);
    bool viewportEvent(QEvent* e);
  #endif

private:
  Q_DECLARE_PRIVATE(qMRMLSceneViewsTreeWidget);
  Q_DISABLE_COPY(qMRMLSceneViewsTreeWidget);

  qSlicerSceneViewsModuleWidget* m_Widget;
  vtkSlicerSceneViewLogic* m_Logic;

};

#endif
