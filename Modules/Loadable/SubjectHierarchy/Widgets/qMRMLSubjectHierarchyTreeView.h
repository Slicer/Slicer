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

#ifndef __qMRMLSubjectHierarchyTreeView_h
#define __qMRMLSubjectHierarchyTreeView_h

// Qt includes
#include <QTreeView>

// CTK includes
#include <ctkVTKObject.h>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qMRMLSubjectHierarchyTreeViewPrivate;
class qMRMLSortFilterSubjectHierarchyProxyModel;
class qMRMLSubjectHierarchyModel;
class vtkMRMLSubjectHierarchyNode;
class vtkMRMLScene;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSubjectHierarchyTreeView : public QTreeView
{
  Q_OBJECT
  QVTK_OBJECT

  /// This property controls whether the root item (folder, an item for a data node, or the scene itself),
  /// if any is visible. When the root item is visible, it appears as a top-level item,
  /// if it is hidden only its children are top-level items. It doesn't have any effect if \a rootItem() is invalid. Shown by default.
  /// \sa setShowRootItem(), showRootItem(), setRootItem(), setRootIndex()
  Q_PROPERTY(bool showRootItem READ showRootItem WRITE setShowRootItem)
  /// Flag determining whether to highlight items referenced by DICOM. Storing DICOM references:
  ///   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  ///   -> SH item instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  Q_PROPERTY(bool highlightReferencedItems READ highlightReferencedItems WRITE setHighlightReferencedItems)
  /// Flag determining whether context menu is enabled
  Q_PROPERTY(bool contextMenuEnabled READ contextMenuEnabled WRITE setContextMenuEnabled)

public:
  typedef QTreeView Superclass;
  qMRMLSubjectHierarchyTreeView(QWidget *parent=0);
  virtual ~qMRMLSubjectHierarchyTreeView();

public:
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;
  Q_INVOKABLE vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;

  Q_INVOKABLE vtkIdType currentItem()const;
  Q_INVOKABLE vtkIdType rootItem()const;

  void setShowRootItem(bool show);
  bool showRootItem()const;

  bool highlightReferencedItems()const;
  void setHighlightReferencedItems(bool highlightOn);

  bool contextMenuEnabled()const;
  void setContextMenuEnabled(bool enabled);

  /// Set attribute filter that allows showing only items that have the specified attribute and their parents.
  /// \param attributeName Name of the attribute by which the items are filtered
  /// \param attributeValue Value of the specified attribute that needs to match this given value in order
  ///   for it to be shown. If empty, then existence of the attribute is enough to show. Empty by default
  Q_INVOKABLE void setAttributeFilter(const QString& attributeName, const QVariant& attributeValue=QVariant());
  /// Remove item attribute filtering \sa setAttribute
  Q_INVOKABLE void removeAttributeFilter();

  /// Set level filter that allows showing only items at a specified level and their parents. Show all items if empty
  Q_INVOKABLE void setLevelFilter(QString &levelFilter);

  Q_INVOKABLE qMRMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel()const;
  Q_INVOKABLE qMRMLSubjectHierarchyModel* model()const;

  /// Determine the number of shown items
  Q_INVOKABLE int displayedItemCount()const;

  virtual bool clickDecoration(const QModelIndex& index);

protected:
  /// Set the subject hierarchy node found in the given scene. Called only internally.
  virtual void setSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* shNode);

  /// Toggle visibility
  virtual void toggleVisibility(const QModelIndex& index);

  /// Populate context menu for given subject hierarchy item
  /// Usually one of the current ones, but if right-clicked on the empty area, then the scene.
  /// The current items are queried in the function anyway, in case of multi-selection
  virtual void populateContextMenuForItem(vtkIdType itemID);

  /// Reimplemented to increase performance
  virtual void updateGeometries();

  /// Handle mouse press event (facilitates timely update of context menu)
  virtual void mousePressEvent(QMouseEvent* event);
  /// Handle mouse release event
  virtual void mouseReleaseEvent(QMouseEvent* event);

  /// Apply highlight for subject hierarchy items referenced by argument items by DICOM
  /// \sa highlightReferencedItems
  void applyReferenceHighlightForItems(QList<vtkIdType> itemIDs);

public slots:
  /// Set MRML scene
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// Set current (=selected) subject hierarchy item
  virtual void setCurrentItem(vtkIdType itemID);

  /// Set subject hierarchy item to be the root in the shown tree
  virtual void setRootItem(vtkIdType itemID);

  /// Rename currently selected one item by popping up a dialog
  void renameCurrentItem();
  /// Delete selected subject hierarchy items and associated data nodes
  void deleteSelectedItems();
  /// Edit properties of current item
  virtual void editCurrentItem();

  /// Handle expand item requests in the subject hierarchy tree. Expands branch
  virtual void expandItem(vtkIdType itemID);
  /// Handle collapse item requests in the subject hierarchy tree. Collapses branch
  virtual void collapseItem(vtkIdType itemID);

  /// Select items in the subject hierarchy tree
  virtual void selectItems(QList<vtkIdType> itemIDs);

  /// Handle manual selection of a plugin as the new owner of a subject hierarchy node
  virtual void selectPluginForCurrentItem();

  /// Update select plugin actions. Is called when the plugin selection sub-menu is opened,
  /// and when the user manually changes the owner plugin of a node. It sets checked state
  /// and update confidence values in the select plugin actions in the node context menu
  /// for the currently selected node.
  virtual void updateSelectPluginActions();

  /// Set multi-selection
  virtual void setMultiSelection(bool multiSelectionOn);

signals:
  void currentItemChanged(vtkIdType);

protected slots:
  virtual void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  /// Updates subject hierarchy item expanded property when item is expanded
  virtual void onItemExpanded(const QModelIndex &expandedItemIndex);
  /// Updates subject hierarchy item expanded property when item is collapsed
  virtual void onItemCollapsed(const QModelIndex &collapsedItemIndex);

  /// Expand tree to depth specified by the clicked context menu action
  virtual void expandToDepthFromContextMenu();

  /// Update root item to restore view
  /// (e.g. after tree was updated in the model from the subject hierarchy)
  virtual void updateRootItem();

  /// Called when scene end is finished. Hierarchy is cleared in that case.
  void onSceneCloseEnded(vtkObject* sceneObject);

protected:
  QScopedPointer<qMRMLSubjectHierarchyTreeViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSubjectHierarchyTreeView);
  Q_DISABLE_COPY(qMRMLSubjectHierarchyTreeView);
  friend class qMRMLSubjectHierarchyComboBox;
};

#endif
