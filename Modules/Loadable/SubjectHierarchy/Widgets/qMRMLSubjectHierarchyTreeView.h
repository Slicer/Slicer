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

// MRML includes
#include <vtkMRMLSubjectHierarchyNode.h>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qMRMLSubjectHierarchyTreeViewPrivate;
class qMRMLSortFilterSubjectHierarchyProxyModel;
class qMRMLSubjectHierarchyModel;

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

public:
  typedef QTreeView Superclass;
  qMRMLSubjectHierarchyTreeView(QWidget *parent=0);
  virtual ~qMRMLSubjectHierarchyTreeView();

  typedef vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID SubjectHierarchyItemID;

public:
  vtkMRMLScene* mrmlScene()const;
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;

  SubjectHierarchyItemID currentItem()const;
  SubjectHierarchyItemID rootItem()const;

  void setShowRootItem(bool show);
  bool showRootItem()const;

  bool highlightReferencedItems()const;
  void setHighlightReferencedItems(bool highlightOn);

  Q_INVOKABLE qMRMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel()const;
  Q_INVOKABLE qMRMLSubjectHierarchyModel* model()const;

  virtual bool clickDecoration(const QModelIndex& index);

protected:
  /// Set the subject hierarchy node found in the given scene. Called only internally.
  virtual void setSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* shNode);

  /// Toggle visibility
  virtual void toggleVisibility(const QModelIndex& index);

  /// Populate context menu for current subject hierarchy item
  virtual void populateContextMenuForCurrentItem();

  /// Reimplemented to increase performance
  virtual void updateGeometries();

  /// Handle mouse press event (facilitates timely update of context menu)
  virtual void mousePressEvent(QMouseEvent* event);
  /// Handle mouse release event
  virtual void mouseReleaseEvent(QMouseEvent* event);

  /// Apply highlight for subject hierarchy items referenced by argument items by DICOM
  /// \sa highlightReferencedItems
  void applyReferenceHighlightForItems(QList<SubjectHierarchyItemID> itemIDs);

public slots:
  /// Set MRML scene
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// Set current (=selected) subject hierarchy item
  virtual void setCurrentItem(SubjectHierarchyItemID itemID);

  /// Set subject hierarchy item to be the root in the shown tree
  virtual void setRootItem(SubjectHierarchyItemID itemID);

  /// Rename currently selected one item by popping up a dialog
  void renameCurrentItem();
  /// Delete selected subject hierarchy items and associated data nodes
  void deleteSelectedItems();
  /// Edit properties of current item
  virtual void editCurrentItem();

  /// Handle expand item requests in the subject hierarchy tree
  virtual void expandItem(SubjectHierarchyItemID itemID);

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
  void currentItemChanged(SubjectHierarchyItemID);

protected slots:
  virtual void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  virtual void onItemExpanded(const QModelIndex &expandedItemIndex);

  /// Expand tree to depth specified by the clicked context menu action
  virtual void expandToDepthFromContextMenu();

private:
  Q_DECLARE_PRIVATE(qMRMLSubjectHierarchyTreeView);
  Q_DISABLE_COPY(qMRMLSubjectHierarchyTreeView);
};

#endif
