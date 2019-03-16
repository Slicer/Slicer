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

#ifndef __qMRMLAnnotationTreeView_h
#define __qMRMLAnnotationTreeView_h

// qMRML includes
#include "qMRMLTreeView.h"

// Annotations includes
#include "qSlicerAnnotationsModuleWidgetsExport.h"
class qMRMLAnnotationTreeViewPrivate;
class qMRMLSceneAnnotationModel;

// Logic includes
class vtkSlicerAnnotationModuleLogic;

// MRML includes
class vtkMRMLNode;
class vtkMRMLScene;

// VTK includes
class vtkCollection;

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_MODULE_ANNOTATIONS_WIDGETS_EXPORT qMRMLAnnotationTreeView
  : public qMRMLTreeView
{
  Q_OBJECT

public:
  typedef qMRMLTreeView Superclass;
  qMRMLAnnotationTreeView(QWidget *parent=nullptr);
  ~qMRMLAnnotationTreeView() override;

  void hideScene();

  const char* firstSelectedNode();

  // Register the logic
  void setLogic(vtkSlicerAnnotationModuleLogic* logic);


  void toggleLockForSelected();

  void toggleVisibilityForSelected();

  void deleteSelected();

  void selectedAsCollection(vtkCollection* collection);

  qMRMLSceneAnnotationModel* annotationModel()const;

signals:
  void onPropertyEditButtonClicked(QString id);

protected slots:
  void onClicked(const QModelIndex& index);

protected:
  QScopedPointer<qMRMLAnnotationTreeViewPrivate> d_ptr;
#ifndef QT_NO_CURSOR
  void mouseMoveEvent(QMouseEvent* e) override;
  bool viewportEvent(QEvent* e) override;
#endif
  void mousePressEvent(QMouseEvent* event) override;

  void toggleVisibility(const QModelIndex& index) override;

  /// Reimplemented to also set the active hierarchy node when the current
  /// index changes.
  /// \sa qMRMLTreeView::onSelectionChanged(),
  /// vtkSlicerAnnotationModuleLogic::SetActiveHierarchyNodeID()
  void onSelectionChanged(const QItemSelection & selected,
                                  const QItemSelection & deselected) override;

private:
  Q_DECLARE_PRIVATE(qMRMLAnnotationTreeView);
  Q_DISABLE_COPY(qMRMLAnnotationTreeView);

  vtkSlicerAnnotationModuleLogic* m_Logic;

  // toggle un-/lock of an annotation
  void onLockColumnClicked(vtkMRMLNode* node);

};

#endif
