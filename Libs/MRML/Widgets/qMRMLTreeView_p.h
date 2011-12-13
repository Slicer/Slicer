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

#ifndef __qMRMLTreeView_p_h
#define __qMRMLTreeView_p_h

// Qt includes
#include <QStyledItemDelegate>
class QAction;
class QMenu;

// MRMLWidgets includes
#include "qMRMLTreeView.h"
#include "qMRMLWidgetsExport.h"
class qMRMLSceneModel;
class qMRMLSortFilterProxyModel;
class vtkMRMLModelHierarchyLogic;

//------------------------------------------------------------------------------
class qMRMLTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLTreeView);
protected:
  qMRMLTreeView* const q_ptr;
public:
  qMRMLTreeViewPrivate(qMRMLTreeView& object);
  virtual ~qMRMLTreeViewPrivate();
  void init();
  void setSceneModel(qMRMLSceneModel* newModel);
  void setSortFilterProxyModel(qMRMLSortFilterProxyModel* newSortModel);
  QSize sizeHint()const;
  void recomputeSizeHint(bool forceUpdate = false);

  qMRMLSceneModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
  QString                    SceneModelType;
  bool                       FitSizeToVisibleIndexes;
  mutable QSize              TreeViewSizeHint;

  QMenu*                     NodeMenu;
  QAction*                   EditAction;
  QMenu*                     SceneMenu;
  vtkMRMLModelHierarchyLogic* ModelHierarchyLogic;

};

#endif
