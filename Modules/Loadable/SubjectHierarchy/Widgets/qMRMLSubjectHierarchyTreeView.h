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
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLSubjectHierarchyNode;
class vtkIdList;

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
  /// This property controls whether the Edit properties context menu action is visible. Visible by default
  Q_PROPERTY(bool editMenuActionVisible READ editMenuActionVisible WRITE setEditMenuActionVisible)
  /// This property controls whether the Select role context menu sub-menu is visible. Visible by default
  Q_PROPERTY(bool selectRoleSubMenuVisible READ selectRoleSubMenuVisible WRITE setSelectRoleSubMenuVisible)
  /// Flag determining whether multiple items can be selected
  Q_PROPERTY(bool multiSelection READ multiSelection WRITE setMultiSelection)
  Q_PROPERTY(QString attributeNameFilter READ attributeNameFilter WRITE setAttributeNameFilter)
  Q_PROPERTY(QString attributeValueFilter READ attributeValueFilter WRITE setAttributeValueFilter)
  Q_PROPERTY(QStringList levelFilter READ levelFilter WRITE setLevelFilter)
  Q_PROPERTY(QString nameFilter READ nameFilter WRITE setNameFilter)
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes)
  Q_PROPERTY(QStringList hideChildNodeTypes READ hideChildNodeTypes WRITE setHideChildNodeTypes)

public:
  typedef QTreeView Superclass;
  qMRMLSubjectHierarchyTreeView(QWidget *parent=nullptr);
  ~qMRMLSubjectHierarchyTreeView() override;

public:
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;
  Q_INVOKABLE vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;

  /// Get current (=selected) item. If there are multiple items selected, then the first one is returned
  Q_INVOKABLE vtkIdType currentItem()const;
  /// Get current (=selected) items.
  QList<vtkIdType> currentItems();
  /// Get current (=selected) items.
  /// \param vtkIdList for python compatibility
  Q_INVOKABLE void currentItems(vtkIdList* selectedItems);

  /// Convenience method to set current item by associated data node.
  virtual vtkMRMLNode* currentNode()const;

  /// Get root item of the tree
  Q_INVOKABLE vtkIdType rootItem()const;
  /// Get root item visibility
  bool showRootItem()const;

  /// Get whether multi-selection is enabled
  bool multiSelection();

  /// Set attribute filter that allows showing only items that have the specified attribute and their parents.
  /// \param attributeName Name of the attribute by which the items are filtered
  /// \param attributeValue Value of the specified attribute that needs to match this given value in order
  ///   for it to be shown. If empty, then existence of the attribute is enough to show. Empty by default
  Q_INVOKABLE void setAttributeFilter(const QString& attributeName, const QVariant& attributeValue=QVariant());
  /// Remove item attribute filtering \sa setAttribute
  Q_INVOKABLE void removeAttributeFilter();
  void setAttributeNameFilter(const QString& attributeName);
  void setAttributeValueFilter(const QString& attributeValue);
  QString attributeNameFilter()const;
  QString attributeValueFilter()const;

  /// Set level filter that allows showing only items at a specified level and their parents. Show all items if empty
  void setLevelFilter(QStringList &levelFilter);
  QStringList levelFilter()const;

  /// Set name filter that allows showing only items containing a specified string (case-insensitive). Show all items if empty
  void setNameFilter(QString &nameFilter);
  QString nameFilter()const;

  /// Set node type filter that allows showing only data nodes of a certain type. Show all data nodes if empty
  void setNodeTypes(const QStringList& types);
  QStringList nodeTypes()const;

  /// Set child node types filter that allows hiding certain data node subclasses that would otherwise be
  /// accepted by the data node type filter. Show all data nodes if empty
  void setHideChildNodeTypes(const QStringList& types);
  QStringList hideChildNodeTypes()const;

  Q_INVOKABLE qMRMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel()const;
  Q_INVOKABLE qMRMLSubjectHierarchyModel* model()const;

  /// Determine the number of shown items
  Q_INVOKABLE int displayedItemCount()const;

  bool highlightReferencedItems()const;
  bool contextMenuEnabled()const;
  bool editMenuActionVisible()const;
  bool selectRoleSubMenuVisible()const;

public slots:
  /// Set MRML scene
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// Set current (=selected) subject hierarchy item
  virtual void setCurrentItem(vtkIdType itemID);
  /// Set current (=selected) subject hierarchy items
  virtual void setCurrentItems(QList<vtkIdType> items);
  /// Python compatibility function to set current (=selected) subject hierarchy items
  virtual void setCurrentItems(vtkIdList* items);
  /// Convenience method to set current item by associated data node
  virtual void setCurrentNode(vtkMRMLNode* node);

  /// Set subject hierarchy item to be the root in the shown tree
  virtual void setRootItem(vtkIdType itemID);
  /// Set root item visibility
  void setShowRootItem(bool show);

  /// Rename currently selected one item by popping up a dialog
  void renameCurrentItem();
  /// Delete selected subject hierarchy items and associated data nodes
  void deleteSelectedItems();
  /// Toggle visibility of selected subject hierarchy items
  void toggleVisibilityOfSelectedItems();
  /// Edit properties of current item
  virtual void editCurrentItem();

  /// Handle expand item requests in the subject hierarchy tree. Expands branch
  virtual void expandItem(vtkIdType itemID);
  /// Handle collapse item requests in the subject hierarchy tree. Collapses branch
  virtual void collapseItem(vtkIdType itemID);

  /// Handle manual selection of a plugin as the new owner of a subject hierarchy node
  virtual void selectPluginForCurrentItem();

  /// Update select plugin actions. Is called when the plugin selection sub-menu is opened,
  /// and when the user manually changes the owner plugin of a node. It sets checked state
  /// and update confidence values in the select plugin actions in the node context menu
  /// for the currently selected node.
  virtual void updateSelectPluginActions();

  /// Set multi-selection
  virtual void setMultiSelection(bool multiSelectionOn);

  /// Set list of subject hierarchy plugins that are enabled.
  /// \param whitelist List of whitelisted subject hierarchy plugin names.
  ///   Empty whitelist means all plugins are enabled. That is the default.
  void setPluginWhitelist(QStringList whitelist);
  /// Set list of subject hierarchy plugins that are disabled.
  /// \param blacklist List of blacklisted subject hierarchy plugin names.
  ///   Empty blacklist means all plugins are enabled. That is the default.
  void setPluginBlacklist(QStringList blacklist);
  /// Disable subject hierarchy plugin by adding it to the blacklist \sa setPluginBlacklist
  /// \param plugin Name of the plugin to disable
  void disablePlugin(QString plugin);

  /// Show hint to user about context menus
  /// \param visibility True if visibility context menu hint is to be shown, false for general context menu. False by default
  /// \return Flag indicating whether hint could be shown (i.e. there was an item in the tree is displayable)
  bool showContextMenuHint(bool visibility=false);

  void setHighlightReferencedItems(bool highlightOn);
  void setContextMenuEnabled(bool enabled);
  void setEditMenuActionVisible(bool visible);
  void setSelectRoleSubMenuVisible(bool visible);

  /// Resets column sizes and size policies to default.
  void resetColumnSizesToDefault();

signals:
  void currentItemChanged(vtkIdType);
  void currentItemsChanged(QList<vtkIdType>);
  void currentItemModified(vtkIdType);

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

  /// Propagate item modified event
  virtual void onSubjectHierarchyItemModified(vtkObject *caller, void *callData);
  /// Propagate item transform modified event
  virtual void onSubjectHierarchyItemTransformModified(vtkObject *caller, void *callData);

  /// Called when scene end is finished. Hierarchy is cleared in that case.
  virtual void onMRMLSceneCloseEnded(vtkObject* sceneObject);
  /// Called when batch processing starts. Makes sure stored selection does not get emptied before restoring
  virtual void onMRMLSceneStartBatchProcess(vtkObject* sceneObject);
  /// Called when batch processing ends. Restores selection, which is lost when the hierarchy is rebuilt
  virtual void onMRMLSceneEndBatchProcess(vtkObject* sceneObject);

  void onCustomContextMenu(const QPoint& point);

  virtual void onTransformActionSelected();
  virtual void onTransformInteractionInViewToggled(bool show);
  virtual void onTransformEditProperties();
  virtual void onCreateNewTransform();

protected:
  /// Set the subject hierarchy node found in the given scene. Called only internally.
  virtual void setSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* shNode);

  /// Toggle visibility for given subject hierarchy item
  void toggleSubjectHierarchyItemVisibility(vtkIdType itemID);

  /// Populate general context menu for given subject hierarchy item
  /// \param itemID Subject hierarchy item ID of the item to show context menu for. It is only used
  ///   to determine whether empty area or scene was clicked. If not, then use the current items
  ///   vector so that multi-selection can also be handled.
  virtual void populateContextMenuForItem(vtkIdType itemID);
  /// Populate visibility context menu for given subject hierarchy item
  virtual void populateVisibilityContextMenuForItem(vtkIdType itemID);
  /// Populate transform context menu for given subject hierarchy item
  virtual void populateTransformContextMenuForItem(vtkIdType itemID);

  /// Handles clicks on the decoration of items (i.e. icon). In visibility column this means either toggle
  /// visibility or show visibility context menu.
  /// \return True if decoration of an enabled item was indeed clicked (and event handled). False otherwise
  virtual bool clickDecoration(QMouseEvent* e);
  /// Handle mouse press event
  void mousePressEvent(QMouseEvent* e) override;
  /// Handle key press event
  void keyPressEvent(QKeyEvent* e) override;

  /// Apply highlight for subject hierarchy items referenced by argument items by DICOM
  /// \sa highlightReferencedItems
  void applyReferenceHighlightForItems(QList<vtkIdType> itemIDs);

  /// Return the id of the first subject hierarchy item that is found to be selected
  /// within the branch that has the input item id as its root
  vtkIdType firstSelectedSubjectHierarchyItemInBranch(vtkIdType itemID);

  void changeEvent(QEvent* e) override;

protected:
  QScopedPointer<qMRMLSubjectHierarchyTreeViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSubjectHierarchyTreeView);
  Q_DISABLE_COPY(qMRMLSubjectHierarchyTreeView);
  friend class qMRMLSubjectHierarchyComboBox;
};

#endif
