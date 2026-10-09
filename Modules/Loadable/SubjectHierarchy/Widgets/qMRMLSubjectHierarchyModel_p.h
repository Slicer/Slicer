/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qMRMLSubjectHierarchyModel_p_h
#define __qMRMLSubjectHierarchyModel_p_h

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
#include <QFlags>
#include <QMap>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

#include "qMRMLSubjectHierarchyModel.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>

// STD includes
#include <set>

class QStandardItemModel;
class vtkSlicerTerminologiesModuleLogic;
class qSlicerSubjectHierarchyAbstractPlugin;

//------------------------------------------------------------------------------
// qMRMLSubjectHierarchyModelPrivate
//------------------------------------------------------------------------------
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSubjectHierarchyModelPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSubjectHierarchyModel);

protected:
  qMRMLSubjectHierarchyModel* const q_ptr;

public:
  qMRMLSubjectHierarchyModelPrivate(qMRMLSubjectHierarchyModel& object);
  virtual ~qMRMLSubjectHierarchyModelPrivate();
  void init();

  /// Insert the subject hierarchy item (and its parent if not yet in the model) at the given row under its parent.
  /// By explicitly specifying the \a index, it skips item lookup within their parents
  /// happening in qMRMLSubjectHierarchyModel::subjectHierarchyItemIndex(vtkIdType).
  virtual QStandardItem* insertSubjectHierarchyItem(vtkIdType itemID, int index);

  /// Create the model items (one for each column) of a subject hierarchy item.
  /// The items are not inserted in the model.
  QList<QStandardItem*> createItemRow(vtkIdType itemID);

  /// Store the model index of all the subject hierarchy items in the subtree of \a parentItem in the RowCache.
  void updateRowCacheRecursively(QStandardItem* parentItem);

  /// Returns true if the scene is batch processing (or importing), or if the end of the batch processing
  /// has not been processed yet by the model. In this state per-item updates are not applied to the model,
  /// instead the whole model is rebuilt at the end of the batch processing.
  /// Note that the scene is no longer in batch processing state while the EndImportEvent and EndBatchProcessEvent
  /// observers are invoked, but observers that are invoked before the model (such as the subject hierarchy plugin
  /// logic that resolves the imported items) may still add many items.
  bool isBatchProcessing() const;

  /// Convenience function to get name for subject hierarchy item
  QString subjectHierarchyItemName(vtkIdType itemID);

  /// Get terminologies module logic. If not found in cache get from module object
  vtkSlicerTerminologiesModuleLogic* terminologiesModuleLogic();

  /// Get extra item identifier
  const QString extraItemIdentifier() { return QString("ExtraItem"); };

  /// Information about a drag-and-drop operation of an item. It can be used to undo the drag-and-drop
  /// by placing back the item into its original position.
  struct ItemDragInfo
  {
    vtkIdType parentItemID{ 0 };
    vtkIdType originalParentItemID{ 0 };
    int originalIndexUnderParent{ -1 };
    qSlicerSubjectHierarchyAbstractPlugin* plugin{ nullptr };
    bool revert{ false };
  };

  // Undo drag-and-drop of each item that has 'revert' set to true and remove from the itemsDragInfo.
  void completePendingDragAndDropReverts(QMap<vtkIdType, qMRMLSubjectHierarchyModelPrivate::ItemDragInfo>& itemsDragInfo);

  qSlicerSubjectHierarchyAbstractPlugin* pluginForReparenting(vtkIdType itemID, vtkIdType oldParentID, vtkIdType newParentID) const;

public:
  vtkSmartPointer<vtkCallbackCommand> CallBack;
  int PendingItemModified;

  int NameColumn;
  int IDColumn;
  int VisibilityColumn;
  int ColorColumn;
  int TransformColumn;
  int DescriptionColumn;

  bool NoneEnabled;
  QString NoneDisplay;

  /// Set when the structure of the subject hierarchy is changed (item added, removed, reparented, reordered,
  /// or the node is modified without item events) while the scene is batch processing (or importing).
  /// Per-item updates are skipped in this state and the whole model is rebuilt when the
  /// batch processing ends (and when the import ends).
  bool RebuildPending;
  /// Items that were modified while the scene was batch processing (or importing). The model items
  /// of these subject hierarchy items are updated when the batch processing ends. Modifying the
  /// existing items (instead of rebuilding the whole model) keeps the state of the views (scroll position,
  /// selection, widgets in the cells) intact, for example when the visibility of a folder is changed
  /// (which modifies all the items in the branch in a batch).
  std::set<vtkIdType> PendingItemUpdates;
  /// Set between the start and the end of the scene batch processing (see isBatchProcessing).
  bool BatchProcessing;
  /// Set while model items are created detached from the model (during rebuild), when
  /// expand/collapse requests cannot be processed by the views yet.
  bool SuppressExpandRequests;

  QIcon VisibleIcon;
  QIcon HiddenIcon;
  QIcon PartiallyVisibleIcon;

  QIcon UnknownIcon;
  QIcon WarningIcon;

  QIcon NoTransformIcon;
  QIcon FolderTransformIcon;
  QIcon LinearTransformIcon;
  QIcon DeformableTransformIcon;

  /// Subject hierarchy node
  vtkWeakPointer<vtkMRMLSubjectHierarchyNode> SubjectHierarchyNode;
  /// MRML scene (to get new subject hierarchy node if the stored one is deleted)
  vtkWeakPointer<vtkMRMLScene> MRMLScene;
  /// Terminology module logic. Needed to generate the terminology tooltip in the color column
  vtkSlicerTerminologiesModuleLogic* TerminologiesModuleLogic;

  mutable QList<vtkIdType> DraggedSubjectHierarchyItems;
  bool DelayedItemChangedInvoked;
  /// Indicates that the last drag-and-drop operation was finished with dropping inside the widget.
  /// It is necessary to distinguish between internal and external drops, because internal drop
  /// results in reparenting and there are additional steps necessary to handle that (e.g., restore item selection).
  bool IsDroppedInside;

  // Keep a list of QStandardItem instead of subject hierarchy item because they are likely to be
  // unreachable when browsing the model
  QList<QList<QStandardItem*>> Orphans;

  // Map from subject hierarchy item to row.
  // It just stores the result of the latest lookup by \sa indexFromSubjectHierarchyItem,
  // not guaranteed to contain up-to-date information, should be just used as a search hint.
  // If the item cannot be found at the given index then we need to browse through all model items.
  mutable QMap<vtkIdType, QPersistentModelIndex> RowCache;
};

#endif
