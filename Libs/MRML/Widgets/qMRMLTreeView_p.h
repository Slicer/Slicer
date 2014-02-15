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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
class QAction;
class QMenu;

// MRML includes
class vtkMRMLHierarchyNode;

// MRMLWidgets includes
#include "qMRMLTreeView.h"
class qMRMLSceneModel;
class qMRMLSortFilterProxyModel;

// VTK includes
class vtkCollection;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLTreeView);
protected:
  qMRMLTreeView* const q_ptr;
public:
  qMRMLTreeViewPrivate(qMRMLTreeView& object);
  virtual ~qMRMLTreeViewPrivate();
  virtual void init();
  void setSceneModel(qMRMLSceneModel* newModel);
  void setSortFilterProxyModel(qMRMLSortFilterProxyModel* newSortModel);
  QSize sizeHint()const;
  void recomputeSizeHint(bool forceUpdate = false);
  /// Save the current expansion state of children nodes of a
  /// vtkMRMLDisplayableHierarchyNode
  void saveChildrenExpandState(QModelIndex& parentIndex);
  void scrollTo(const QString& name, bool next);

  qMRMLSceneModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
  QString                    SceneModelType;
  bool                       FitSizeToVisibleIndexes;
  mutable QSize              TreeViewSizeHint;
  QSize                      TreeViewMinSizeHint;
  bool                       ShowScene;
  bool                       ShowRootNode;
  QString                    LastScrollToName;

  QMenu*                     NodeMenu;
  QAction*                   RenameAction;
  QAction*                   DeleteAction;
  QAction*                   EditAction;
  QMenu*                     SceneMenu;

  vtkCollection*             ExpandedNodes;

};

#endif
