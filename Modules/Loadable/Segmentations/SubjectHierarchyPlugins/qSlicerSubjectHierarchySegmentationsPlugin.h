/*==============================================================================

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

#ifndef __qSlicerSubjectHierarchySegmentationsPlugin_h
#define __qSlicerSubjectHierarchySegmentationsPlugin_h

// Subject Hierarchy includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSegmentationsSubjectHierarchyPluginsExport.h"

class qSlicerSubjectHierarchySegmentationsPluginPrivate;
class vtkMRMLSegmentationNode;

/// \ingroup SlicerRt_QtModules_Segmentations
class Q_SLICER_SEGMENTATIONS_PLUGINS_EXPORT qSlicerSubjectHierarchySegmentationsPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchySegmentationsPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchySegmentationsPlugin() override;

public:
  /// Determines if a data node can be placed in the hierarchy using the actual plugin,
  /// and gets a confidence value for a certain MRML node (usually the type and possibly attributes are checked).
  /// \param node Node to be added to the hierarchy
  /// \param parentItemID Prospective parent of the node to add.
  ///   Default value is invalid. In that case the parent will be ignored, the confidence numbers are got based on the to-be child node alone.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier attribute)
  double canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )const override;

  /// Creates subject hierarchy item using default method and updates all segments
  bool addNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID) override;

  /// Determines if a subject hierarchy item can be reparented in the hierarchy using the current plugin,
  /// and gets a confidence value for the reparented item.
  /// Most plugins do not perform steps additional to the default, so the default implementation returns a 0
  /// confidence value, which can be overridden in plugins that do handle special cases.
  /// \param itemID Item to be reparented in the hierarchy
  /// \param parentItemID Prospective parent of the item to reparent.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item
  double canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)const override;

  /// Reparent an item that was already in the subject hierarchy under a new parent.
  /// \return True if reparented successfully, false otherwise
  bool reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID) override;

  /// Determines if the actual plugin can handle a subject hierarchy item. The plugin with
  /// the highest confidence number will "own" the item in the subject hierarchy (set icon, tooltip,
  /// set context menu etc.)
  /// \param item Item to handle in the subject hierarchy tree
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item (by node type or identifier attribute)
  double canOwnSubjectHierarchyItem(vtkIdType itemID)const override;

  /// Get role that the plugin assigns to the subject hierarchy node.
  ///   Each plugin should provide only one role.
  Q_INVOKABLE const QString roleForPlugin()const override;

  /// Generate tooltip for a owned subject hierarchy item
  QString tooltip(vtkIdType itemID)const override;

  /// Get help text for plugin to be added in subject hierarchy module widget help box
  const QString helpText()const override;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, empty icon if nothing to set
  QIcon icon(vtkIdType itemID) override;

  /// Get visibility icon for a visibility state
  QIcon visibilityIcon(int visible) override;

  /// Set display visibility of a owned subject hierarchy item
  void setDisplayVisibility(vtkIdType itemID, int visible) override;

  /// Get display visibility of a owned subject hierarchy item
  /// \return Display visibility (0: hidden, 1: shown, 2: partially shown)
  int getDisplayVisibility(vtkIdType itemID)const override;

  /// Get item context menu item actions to add to tree view
  QList<QAction*> itemContextMenuActions()const override;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  void showContextMenuActionsForItem(vtkIdType itemID) override;

  /// Get visibility context menu item actions to add to tree view.
  /// These item visibility context menu actions can be shown in the implementations of \sa showVisibilityContextMenuActionsForItem
  QList<QAction*> visibilityContextMenuActions()const override;

  /// Show visibility context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the visibility context menu items for
  void showVisibilityContextMenuActionsForItem(vtkIdType itemID) override;

public slots:
  /// Called when segment is added in an observed segmentation node
  /// Adds per-segment subject hierarchy node
  void onSegmentAdded(vtkObject* caller, void* callData);

  /// Called when segment is removed in an observed segmentation node.
  /// Removes per-segment subject hierarchy node
  void onSegmentRemoved(vtkObject* caller, void* callData);

  /// Called when segment is modified in an observed segmentation node.
  /// Renames per-segment subject hierarchy node if necessary
  void onSegmentModified(vtkObject* caller, void* callData);

  /// Called when a subject hierarchy item is modified.
  /// Renames segment if the modified item belongs to a segment
  void onSubjectHierarchyItemModified(vtkObject* caller, void* callData);

  /// Called when a subject hierarchy item is about to be removed.
  /// Removes segment from parent segmentation if the removed item belongs to a segment
  void onSubjectHierarchyItemAboutToBeRemoved(vtkObject* caller, void* callData);

protected slots:
  /// Export to binary labelmap
  void exportToBinaryLabelmap();

  /// Export to closed surface
  void exportToClosedSurface();

  /// Convert labelmap to segmentation node
  void convertLabelmapToSegmentation();

  /// Convert model to segmentation node
  void convertModelToSegmentation();

  /// Convert model hierarchy to segmentation node
  void convertModelsToSegmentation();

  /// Toggle 2D fill visibility for the current segmentation
  void toggle2DFillVisibility(bool);

  /// Toggle 2D outline visibility for the current segmentation
  void toggle2DOutlineVisibility(bool);

protected:
  QScopedPointer<qSlicerSubjectHierarchySegmentationsPluginPrivate> d_ptr;

  void updateAllSegmentsFromMRML(vtkMRMLSegmentationNode* segmentationNode);

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchySegmentationsPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchySegmentationsPlugin);
};

#endif
