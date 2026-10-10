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

#ifndef __qMRMLSceneModel_p_h
#define __qMRMLSceneModel_p_h

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
class QStandardItemModel;
#include <QFlags>
#include <QHash>
#include <QList>
#include <QMap>

// qMRML includes
#include "qMRMLSceneModel.h"

// MRML includes
class vtkMRMLScene;

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//------------------------------------------------------------------------------
// qMRMLSceneModelPrivate
//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSceneModelPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSceneModel);

protected:
  qMRMLSceneModel* const q_ptr;

public:
  qMRMLSceneModelPrivate(qMRMLSceneModel& object);
  virtual ~qMRMLSceneModelPrivate();
  void init();

  QModelIndexList indexes(const QString& nodeID) const;

  QStringList extraItems(QStandardItem* parent, const QString& extraType) const;
  void insertExtraItem(int row, QStandardItem* parent, const QString& text, const QString& extraType, const Qt::ItemFlags& flags, const QString& extraItemData = "");
  void removeAllExtraItems(QStandardItem* parent, const QString extraType);
  bool isExtraItem(const QStandardItem* item) const;
  void listenNodeModifiedEvent();
  void reparentItems(QList<QStandardItem*>& children, int newIndex, QStandardItem* newParent);

  /// Insert the node (and its parent if not in the model yet) at the given row under its parent.
  /// By explicitly specifying the \a index, it skips repetitive scene traversal calls caused by
  /// qMRMLSceneModel::nodeIndex(vtkMRMLNode*).
  QStandardItem* insertNode(vtkMRMLNode* node, int index);

  /// Create the model items (one for each column) of a node. The items are not inserted in the model.
  QList<QStandardItem*> createNodeItems(vtkMRMLNode* node);

  /// Add all the node items in the subtree of \a item (including \a item) to the RowCache.
  void addToRowCacheRecursively(QStandardItem* item);
  /// Remove all the node items in the subtree of \a item (including \a item) from the RowCache.
  void removeFromRowCacheRecursively(QStandardItem* item);

  vtkSmartPointer<vtkCallbackCommand> CallBack;
  qMRMLSceneModel::NodeTypes ListenNodeModifiedEvent;
  bool LazyUpdate;
  int PendingItemModified;

  int NameColumn;
  int IDColumn;
  int CheckableColumn;
  int VisibilityColumn;
  int ToolTipNameColumn;
  int ExtraItemColumn;

  QIcon VisibleIcon;
  QIcon HiddenIcon;
  QIcon PartiallyVisibleIcon;

  vtkWeakPointer<vtkMRMLScene> MRMLScene;
  QStandardItem* DraggedItem;
  mutable QList<vtkMRMLNode*> DraggedNodes;
  // We keep a list of QStandardItem instead of vtkMRMLNode* because they are
  // likely to be unreachable when browsing the model
  QList<QList<QStandardItem*>> Orphans;

  // Map from MRML node to the model item (in the first column) of the node.
  // It is kept up-to-date from the rowsInserted and rowsAboutToBeRemoved signals of the model
  // (QStandardItemModel emits these signals for all item insertions and removals, including
  // moving items with takeRow/insertRow), therefore it only contains items that are in the model.
  // An item pointer is used instead of a persistent model index, because inserting a row in a
  // QStandardItemModel updates all the persistent indexes (which would make inserting a node
  // in a model of N nodes an O(N) operation).
  mutable QHash<vtkMRMLNode*, QStandardItem*> RowCache;
};

#endif
