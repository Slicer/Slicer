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
  qSlicerSubjectHierarchySegmentationsPlugin(QObject* parent = NULL);
  virtual ~qSlicerSubjectHierarchySegmentationsPlugin();

public:
  /// Determines if a data node can be placed in the hierarchy using the actual plugin,
  /// and gets a confidence value for a certain MRML node (usually the type and possibly attributes are checked).
  /// \param node Node to be added to the hierarchy
  /// \param parentItemID Prospective parent of the node to add.
  ///   Default value is invalid. In that case the parent will be ignored, the confidence numbers are got based on the to-be child node alone.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier attribute)
  virtual double canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )const;

  /// Creates subject hierarchy item using default method and updates all segments
  virtual bool addNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID, std::string level="");

  /// Determines if the actual plugin can handle a subject hierarchy item. The plugin with
  /// the highest confidence number will "own" the item in the subject hierarchy (set icon, tooltip,
  /// set context menu etc.)
  /// \param item Item to handle in the subject hierarchy tree
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item (by node type or identifier attribute)
  virtual double canOwnSubjectHierarchyItem(vtkIdType itemID)const;

  /// Get role that the plugin assigns to the subject hierarchy node.
  ///   Each plugin should provide only one role.
  Q_INVOKABLE virtual const QString roleForPlugin()const;

  /// Generate tooltip for a owned subject hierarchy item
  virtual QString tooltip(vtkIdType itemID)const;

  /// Get help text for plugin to be added in subject hierarchy module widget help box
  virtual const QString helpText()const;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, empty icon if nothing to set
  virtual QIcon icon(vtkIdType itemID);

  /// Get visibility icon for a visibility state
  virtual QIcon visibilityIcon(int visible);

  /// Set display visibility of a owned subject hierarchy item
  virtual void setDisplayVisibility(vtkIdType itemID, int visible);

  /// Get display visibility of a owned subject hierarchy item
  /// \return Display visibility (0: hidden, 1: shown, 2: partially shown)
  virtual int getDisplayVisibility(vtkIdType itemID)const;

  /// Get item context menu item actions to add to tree view
  virtual QList<QAction*> itemContextMenuActions()const;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  virtual void showContextMenuActionsForItem(vtkIdType itemID);

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
  /// Create binary labelmap representation
  void createBinaryLabelmapRepresentation();

  /// Create closed surface representation
  void createClosedSurfaceRepresentation();

protected:
  QScopedPointer<qSlicerSubjectHierarchySegmentationsPluginPrivate> d_ptr;

  void updateAllSegmentsFromMRML(vtkMRMLSegmentationNode* segmentationNode);

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchySegmentationsPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchySegmentationsPlugin);
};

#endif
