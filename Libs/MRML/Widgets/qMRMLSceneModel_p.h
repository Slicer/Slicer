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
#include <QMap>

// qMRML includes
#include "qMRMLSceneModel.h"

// MRML includes
class vtkMRMLScene;

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>

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

  QModelIndexList indexes(const QString& nodeID)const;

  QStringList extraItems(QStandardItem* parent, const QString& extraType)const;
  void insertExtraItem(int row, QStandardItem* parent,
                       const QString& text, const QString& extraType,
                       const Qt::ItemFlags& flags);
  void removeAllExtraItems(QStandardItem* parent, const QString extraType);
  bool isExtraItem(const QStandardItem* item)const;
  void listenNodeModifiedEvent();
  void reparentItems(QList<QStandardItem*>& children, int newIndex, QStandardItem* newParent);

  /// This method is called by qMRMLSceneModel::populateScene() to speed up
  /// the loading of large scene. By explicitly specifying the \a index, it
  /// skips repetitive scene traversal calls caused by
  /// qMRMLSceneModel::nodeIndex(vtkMRMLNode*).
  QStandardItem* insertNode(vtkMRMLNode* node, int index);

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

  vtkMRMLScene* MRMLScene;
  QStandardItem* DraggedItem;
  mutable QList<vtkMRMLNode*>  DraggedNodes;
  QList<vtkMRMLNode*> MisplacedNodes;
  // We keep a list of QStandardItem instead of vtkMRMLNode* because they are
  // likely to be unreachable when browsing the model
  QList<QList<QStandardItem*> > Orphans;

  // Map from MRML node to row.
  // It just stores the result of the latest lookup by indexFromNode,
  // not guaranteed to contain up-to-date information, should be just used
  // as a search hint. If the node cannot be found at the given index then
  // we need to browse through all model items.
  mutable QMap<vtkMRMLNode*,QPersistentModelIndex> RowCache;
};

#endif
