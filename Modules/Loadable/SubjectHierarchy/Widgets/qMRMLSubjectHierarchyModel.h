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

// MRML includes
#include <vtkMRMLSubjectHierarchyNode.h>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qMRMLSubjectHierarchyModelPrivate;

/// TODO:
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
  /// Control in which column the data MRML node IDs are displayed (Qt::DisplayRole).
  /// A value of -1 hides it. Hidden by default (value of -1)
  Q_PROPERTY (int idColumn READ idColumn WRITE setIDColumn)
  /// Control in which column data MRML node visibility are displayed (Qt::DecorationRole).
  /// A value of -1 hides it. Hidden by default (value of -1).
  Q_PROPERTY (int visibilityColumn READ visibilityColumn WRITE setVisibilityColumn)
  /// Control in which column the parent transforms are displayed
  /// A MRML node combobox is displayed in the row of the transformable nodes, in which
  /// the current transform is selected. The user can change the transform using the combobox.
  /// A value of -1 (default) hides the column
  Q_PROPERTY (int transformColumn READ transformColumn WRITE setTransformColumn)

  typedef vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID SubjectHierarchyItemID;

public:
  typedef QStandardItemModel Superclass;
  qMRMLSubjectHierarchyModel(QObject *parent=0);
  virtual ~qMRMLSubjectHierarchyModel();

  enum ItemDataRole
    {
    /// Unique ID of the item, typed SubjectHierarchyItemID (unsigned long)
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

  int idColumn()const;
  void setIDColumn(int column);

  int visibilityColumn()const;
  void setVisibilityColumn(int column);

  int transformColumn()const;
  void setTransformColumn(int column);

  virtual Qt::DropActions supportedDropActions()const;
  virtual QMimeData* mimeData(const QModelIndexList& indexes)const;
  virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                            int row, int column, const QModelIndex &parent);

  Q_INVOKABLE virtual void setMRMLScene(vtkMRMLScene* scene);
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;

  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;

  /// NULL until a valid scene is set
  QStandardItem* subjectHierarchySceneItem()const;
  /// Invalid until a valid scene is set
  QModelIndex subjectHierarchySceneIndex()const;
  virtual bool canBeAChild(SubjectHierarchyItemID itemID)const;
  virtual bool canBeAParent(SubjectHierarchyItemID itemID)const;

  SubjectHierarchyItemID subjectHierarchyItemFromIndex(const QModelIndex &index)const;
  SubjectHierarchyItemID subjectHierarchyItemFromItem(QStandardItem* item)const;
  QModelIndex indexFromSubjectHierarchyItem(SubjectHierarchyItemID itemID, int column=0)const;
  QStandardItem* itemFromSubjectHierarchyItem(SubjectHierarchyItemID itemID, int column=0)const;
  // Return all the QModelIndexes (all the columns) for a given subject hierarchy item
  QModelIndexList indexes(SubjectHierarchyItemID itemID)const;

  virtual SubjectHierarchyItemID parentSubjectHierarchyItem(SubjectHierarchyItemID itemID)const;
  /// Returns the row model index relative to its parent independently of any filtering or proxy model
  /// Must be reimplemented in derived classes
  virtual int subjectHierarchyItemIndex(SubjectHierarchyItemID itemID)const;
  /// Insert/move node in subject hierarchy under new parent
  virtual bool reparent(SubjectHierarchyItemID itemID, SubjectHierarchyItemID newParentID);
  /// Utility method that returns true if \a child has \a parent as ancestor (parent, grandparent, etc.)
  /// \sa isAffiliatedItem()
  bool isAncestorItem(SubjectHierarchyItemID child, SubjectHierarchyItemID ancestor)const;
  /// Utility method that returns true if 2 nodes are child/parent (or any ancestor) for each other
  /// \sa isAncestorItem()
  bool isAffiliatedItem(SubjectHierarchyItemID itemA, SubjectHierarchyItemID itemB)const;

public slots:
  /// Remove transforms from nodes in branch of current item
  void onRemoveTransformsFromBranchOfCurrentItem();

  /// Harden transform on branch of current item
  void onHardenTransformOnBranchOfCurrentItem();

protected slots:
  virtual void onSubjectHierarchyItemAdded(SubjectHierarchyItemID itemID);
  virtual void onSubjectHierarchyItemAboutToBeRemoved(SubjectHierarchyItemID itemID);
  virtual void onSubjectHierarchyItemRemoved(SubjectHierarchyItemID itemID);
  virtual void onSubjectHierarchyItemModified(SubjectHierarchyItemID itemID);

  virtual void onMRMLSceneImported(vtkMRMLScene* scene);
  virtual void onMRMLSceneClosed(vtkMRMLScene* scene);
  virtual void onMRMLSceneStartBatchProcess(vtkMRMLScene* scene);
  virtual void onMRMLSceneEndBatchProcess(vtkMRMLScene* scene);
  virtual void onSubjectHierarchyNodeRemoved();

  virtual void onItemChanged(QStandardItem* item);

  //TODO: Needed?
  virtual void delayedItemChanged();

  /// Recompute the number of columns in the model.
  /// To be called when a XXXColumn is set.
  /// Needs maxColumnId() to be reimplemented in subclasses
  void updateColumnCount();

signals:
  /// This signal is sent when a user is about to reparent an item by drag and drop
  void aboutToReparentByDragAndDrop(SubjectHierarchyItemID itemID, SubjectHierarchyItemID newParentID);
  ///  This signal is sent after a user dragged and dropped an item in the tree view
  void reparentedByDragAndDrop(SubjectHierarchyItemID itemID, SubjectHierarchyItemID newParentID);
  /// This signal is sent when the whole subject hierarchy is about to be updated
  void subjectHierarchyAboutToBeUpdated();
  /// This signal is sent after the whole subject hierarchy is updated
  void subjectHierarchyUpdated();

protected:
  qMRMLSubjectHierarchyModel(qMRMLSubjectHierarchyModelPrivate* pimpl, QObject *parent=0);

  /// Set the subject hierarchy node found in the given scene. Called only internally.
  virtual void setSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* shNode);

  virtual void updateFromSubjectHierarchy();
  virtual QStandardItem* insertSubjectHierarchyItem(SubjectHierarchyItemID itemID);
  virtual QStandardItem* insertSubjectHierarchyItem(SubjectHierarchyItemID itemID, QStandardItem* parent, int row=-1);

  virtual QFlags<Qt::ItemFlag> subjectHierarchyItemFlags(SubjectHierarchyItemID itemID, int column)const;

  virtual void updateItemFromSubjectHierarchyItem(
    QStandardItem* item, SubjectHierarchyItemID shItemID, int column );
  virtual void updateItemDataFromSubjectHierarchyItem(
    QStandardItem* item, SubjectHierarchyItemID shItemID, int column );
  virtual void updateSubjectHierarchyItemFromItem(
    SubjectHierarchyItemID shItemID, QStandardItem* item );
  virtual void updateSubjectHierarchyItemFromItemData(
    SubjectHierarchyItemID shItemID, QStandardItem* item );

  /// Update the model items associated with the subject hierarchy item
  void updateModelItems(SubjectHierarchyItemID itemID);

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
