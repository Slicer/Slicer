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

#ifndef __qMRMLSubjectHierarchyModel_h
#define __qMRMLSubjectHierarchyModel_h

// Qt includes
#include <QStandardItemModel>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qMRMLSubjectHierarchyModelPrivate;
class vtkMRMLSubjectHierarchyNode;
class vtkMRMLNode;
class vtkMRMLScene;

/// \brief Item model for subject hierarchy
///
/// It is associated to the pseudo-singleton subject hierarchy node, and it creates one model item
/// for each subject hierarchy item. It handles reparenting, reordering, etc.
/// The whole model is regenerated when the Modified event is invoked on the subject hierarchy node,
/// but only the individual items are updated when per-item events are invoked (such as
/// vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent)
///
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSubjectHierarchyModel : public QStandardItemModel
{
  Q_OBJECT
  QVTK_OBJECT

  /// Control in which column the data MRML node names or if not assigned then subject hierarchy
  /// item names are displayed (Qt::DisplayRole).
  /// The icons corresponding to the role provided by the owner subject hierarchy plugin is also
  /// displayed in this column (Qt::DecorationRole).
  /// A value of -1 hides it. First column (0) by default.
  /// If no property is set in a column, nothing is displayed.
  Q_PROPERTY (int nameColumn READ nameColumn WRITE setNameColumn)
  /// Control in which column data MRML node visibility are displayed (Qt::DecorationRole).
  /// A value of -1 (default) hides the column
  Q_PROPERTY (int visibilityColumn READ visibilityColumn WRITE setVisibilityColumn)
  /// Control in which column data MRML node color is displayed.
  /// A value of -1 (default) hides the column
  Q_PROPERTY(int colorColumn READ colorColumn WRITE setColorColumn)
    /// Control in which column the parent transforms are displayed
  /// A MRML node combobox is displayed in the row of the transformable nodes, in which
  /// the current transform is selected. The user can change the transform using the combobox.
  /// A value of -1 (default) hides the column
  Q_PROPERTY (int transformColumn READ transformColumn WRITE setTransformColumn)
  /// Control in which column the node descriptions are displayed
  /// A value of -1 (default) hides the column
  Q_PROPERTY (int descriptionColumn READ descriptionColumn WRITE setDescriptionColumn)
  /// Control in which column the data MRML node IDs are displayed (Qt::DisplayRole).
  /// A value of -1 hides it. Hidden by default (value of -1)
  Q_PROPERTY (int idColumn READ idColumn WRITE setIDColumn)

public:
  typedef QStandardItemModel Superclass;
  qMRMLSubjectHierarchyModel(QObject *parent=nullptr);
  ~qMRMLSubjectHierarchyModel() override;

  enum ItemDataRole
    {
    /// Unique ID of the item, typed vtkIdType
    SubjectHierarchyItemIDRole = Qt::UserRole + 1,
    /// Integer that contains the visibility property of an item.
    /// It is closely related to the item icon.
    VisibilityRole,
    /// MRML node ID of the parent transform
    TransformIDRole,
    /// Must stay the last enum in the list.
    LastRole
    };

  int nameColumn()const;
  void setNameColumn(int column);

  int visibilityColumn()const;
  void setVisibilityColumn(int column);

  int colorColumn()const;
  void setColorColumn(int column);

  int transformColumn()const;
  void setTransformColumn(int column);

  int descriptionColumn()const;
  void setDescriptionColumn(int column);

  int idColumn()const;
  void setIDColumn(int column);

  Qt::DropActions supportedDropActions()const override;
  QMimeData* mimeData(const QModelIndexList& indexes)const override;
  bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                            int row, int column, const QModelIndex &parent) override;

  Q_INVOKABLE virtual void setMRMLScene(vtkMRMLScene* scene);
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;

  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;

  /// nullptr until a valid scene is set
  QStandardItem* subjectHierarchySceneItem()const;
  /// Invalid until a valid scene is set
  QModelIndex subjectHierarchySceneIndex()const;
  virtual bool canBeAChild(vtkIdType itemID)const;
  virtual bool canBeAParent(vtkIdType itemID)const;

  vtkIdType subjectHierarchyItemFromIndex(const QModelIndex &index)const;
  vtkIdType subjectHierarchyItemFromItem(QStandardItem* item)const;
  QModelIndex indexFromSubjectHierarchyItem(vtkIdType itemID, int column=0)const;
  QStandardItem* itemFromSubjectHierarchyItem(vtkIdType itemID, int column=0)const;

  /// Return all the QModelIndexes (all the columns) for a given subject hierarchy item
  QModelIndexList indexes(vtkIdType itemID)const;

  Q_INVOKABLE virtual vtkIdType parentSubjectHierarchyItem(vtkIdType itemID)const;
  /// Returns the row model index relative to its parent independently of any filtering or proxy model
  /// Must be reimplemented in derived classes
  Q_INVOKABLE virtual int subjectHierarchyItemIndex(vtkIdType itemID)const;
  /// Insert/move item in subject hierarchy under new parent
  Q_INVOKABLE virtual bool reparent(vtkIdType itemID, vtkIdType newParentID);
  /// Move item in subject hierarchy branch to a new row (re-order)
  Q_INVOKABLE virtual bool moveToRow(vtkIdType itemID, int newRow);
  /// Utility method that returns true if \a child has \a parent as ancestor (parent, grandparent, etc.)
  /// \sa isAffiliatedItem()
  Q_INVOKABLE bool isAncestorItem(vtkIdType child, vtkIdType ancestor)const;
  /// Utility method that returns true if 2 nodes are child/parent (or any ancestor) for each other
  /// \sa isAncestorItem()
  Q_INVOKABLE bool isAffiliatedItem(vtkIdType itemA, vtkIdType itemB)const;

signals:
  /// This signal is sent when a user is about to reparent an item by drag and drop
  void aboutToReparentByDragAndDrop(vtkIdType itemID, vtkIdType newParentID);
  ///  This signal is sent after a user dragged and dropped an item in the tree view
  void reparentedByDragAndDrop(vtkIdType itemID, vtkIdType newParentID);
  /// This signal is sent when the whole subject hierarchy is about to be updated
  void subjectHierarchyAboutToBeUpdated();
  /// This signal is sent after the whole subject hierarchy is updated
  void subjectHierarchyUpdated();
  /// Signal requesting expanding of the subject hierarchy tree item belonging to an item
  void requestExpandItem(vtkIdType itemID);
  /// Signal requesting collapsing of the subject hierarchy tree item belonging to an item
  void requestCollapseItem(vtkIdType itemID);
  /// Signal requesting selecting items in the tree
  void requestSelectItems(QList<vtkIdType> itemIDs);
  /// Triggers invalidating the sort filter proxy model
  void invalidateFilter();

public slots:
  /// Remove transforms from nodes in branch of current item
  void onRemoveTransformsFromBranchOfCurrentItem();

  /// Harden transform on branch of current item
  void onHardenTransformOnBranchOfCurrentItem();

protected slots:
  virtual void onSubjectHierarchyItemAdded(vtkIdType itemID);
  virtual void onSubjectHierarchyItemAboutToBeRemoved(vtkIdType itemID);
  virtual void onSubjectHierarchyItemRemoved(vtkIdType itemID);
  virtual void onSubjectHierarchyItemModified(vtkIdType itemID);

  virtual void onMRMLSceneImported(vtkMRMLScene* scene);
  virtual void onMRMLSceneClosed(vtkMRMLScene* scene);
  virtual void onMRMLSceneStartBatchProcess(vtkMRMLScene* scene);
  virtual void onMRMLSceneEndBatchProcess(vtkMRMLScene* scene);
  virtual void onMRMLNodeRemoved(vtkMRMLNode* node);

  virtual void onItemChanged(QStandardItem* item);
  virtual void delayedItemChanged();

  /// Recompute the number of columns in the model. Called when a [some]Column property is set.
  /// Needs maxColumnId() to be reimplemented in subclasses
  void updateColumnCount();

protected:
  qMRMLSubjectHierarchyModel(qMRMLSubjectHierarchyModelPrivate* pimpl, QObject *parent=nullptr);

  /// Set the subject hierarchy node found in the given scene. Called only internally.
  virtual void setSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* shNode);

  /// Rebuild model from scratch.
  /// This is a hard-update that is uses more resources. Use sparingly.
  virtual void rebuildFromSubjectHierarchy();
  /// Updates properties in the model based on subject hierarchy.
  /// This is a soft update that is quick. Calls \sa rebuildFromSubjectHierarchy if necessary.
  virtual void updateFromSubjectHierarchy();

  virtual QStandardItem* insertSubjectHierarchyItem(vtkIdType itemID);
  virtual QStandardItem* insertSubjectHierarchyItem(vtkIdType itemID, QStandardItem* parent, int row=-1);

  virtual QFlags<Qt::ItemFlag> subjectHierarchyItemFlags(vtkIdType itemID, int column)const;

  virtual void updateItemFromSubjectHierarchyItem(
    QStandardItem* item, vtkIdType shItemID, int column );
  virtual void updateItemDataFromSubjectHierarchyItem(
    QStandardItem* item, vtkIdType shItemID, int column );
  virtual void updateSubjectHierarchyItemFromItem(
    vtkIdType shItemID, QStandardItem* item );
  virtual void updateSubjectHierarchyItemFromItemData(
    vtkIdType shItemID, QStandardItem* item );

  /// Update the model items associated with the subject hierarchy item
  void updateModelItems(vtkIdType itemID);

  static void onEvent(vtkObject* caller, unsigned long event, void* clientData, void* callData);

  /// Must be reimplemented in subclasses that add new column types
  virtual int maxColumnId()const;

protected:
  QScopedPointer<qMRMLSubjectHierarchyModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSubjectHierarchyModel);
  Q_DISABLE_COPY(qMRMLSubjectHierarchyModel);
};

void printStandardItem(QStandardItem* item, const QString& offset);

#endif
