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

#ifndef __qSlicerSubjectHierarchyFolderPlugin_h
#define __qSlicerSubjectHierarchyFolderPlugin_h

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

// CTK includes
#include <ctkVTKObject.h>

class qSlicerSubjectHierarchyFolderPluginPrivate;

class vtkMRMLDisplayNode;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
/// \brief Subject hierarchy folder plugin
///
/// Support folder items in subject hierarchy
///
/// 1. Folders define hierarchies in subject hierarchy.
///
/// 2. Folder items have a directly associated display node after any interaction with
///    display settings of the folder. A special display node type vtkMRMLFolderDisplayNode
///    is used.
///
/// 3. Folders have a feature for overriding display properties of the displayable nodes in
///    its branch. If the user activates it, then all the child displayable nodes use the
///    display settings from the display node associated to the folder.
///
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyFolderPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyFolderPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyFolderPlugin() override;

public:
  /// Determines if a data node can be placed in the hierarchy using the actual plugin,
  /// and gets a confidence value for a certain MRML node (usually the type and possibly attributes are checked).
  /// \param node Node to be added to the hierarchy
  /// \param parentItemID Prospective parent of the node to add.
  ///   Default value is invalid. In that case the parent will be ignored, the confidence numbers are got based on the to-be child node alone.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier attribute)
  double canAddNodeToSubjectHierarchy(
    vtkMRMLNode* node,
    vtkIdType parentItemID=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )const override;

  /// Make sure a folder display node that is added externally shows up in the hierarchy
  bool addNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID) override;

  /// Determines if the actual plugin can handle a subject hierarchy item. The plugin with
  /// the highest confidence number will "own" the item in the subject hierarchy (set icon, tooltip,
  /// set context menu etc.)
  /// \param item Item to handle in the subject hierarchy tree
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item (by node type or identifier attribute)
  double canOwnSubjectHierarchyItem(vtkIdType itemID)const override;

  /// Get role that the plugin assigns to the subject hierarchy item.
  ///   Each plugin should provide only one role.
  Q_INVOKABLE const QString roleForPlugin()const override;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, nullptr if nothing to set
  QIcon icon(vtkIdType itemID) override;

  /// Get visibility icon for a visibility state
  QIcon visibilityIcon(int visible) override;

  /// Open module belonging to item and set inputs in opened module
  void editProperties(vtkIdType itemID) override;

  /// Set display visibility of an owned subject hierarchy item
  void setDisplayVisibility(vtkIdType itemID, int visible) override;

  /// Get display visibility of an owned subject hierarchy item
  /// \return Display visibility (0: hidden, 1: shown, 2: partially shown)
  int getDisplayVisibility(vtkIdType itemID)const override;

  /// Set display color of an owned subject hierarchy item
  /// In case of folders only color is set but no terminology. The properties are not used directly,
  /// but only if applied to the branch (similarly to how it worked in model hierarchies).
  /// \param color Display color to set
  /// \param terminologyMetaData Map containing terminology meta data. Not used in this plugin
  void setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData) override;

  /// Get display color of an owned subject hierarchy item
  /// In case of folders only color is set but no terminology. The properties are not used directly,
  /// but only if applied to the branch (similarly to how it worked in model hierarchies).
  QColor getDisplayColor(vtkIdType itemID, QMap<int, QVariant> &terminologyMetaData)const override;

  /// Get item context menu item actions to add to tree view
  QList<QAction*> itemContextMenuActions()const override;

  /// Get scene context menu item actions to add to tree view
  /// Separate method is needed for the scene, as its actions are set to the
  /// tree by a different method \sa itemContextMenuActions
  QList<QAction*> sceneContextMenuActions()const override;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  void showContextMenuActionsForItem(vtkIdType itemID) override;

  /// Get visibility context menu item actions to add to tree view.
  /// These item visibility context menu actions can be shown in the implementations of \sa showVisibilityContextMenuActionsForItem
  QList<QAction*> visibilityContextMenuActions()const override;

  /// Show visibility context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the visibility context menu items for
  void showVisibilityContextMenuActionsForItem(vtkIdType itemID) override;

public:
  /// Create folder under specified item
  /// \param parentNode Parent item for folder to create
  Q_INVOKABLE vtkIdType createFolderUnderItem(vtkIdType parentItemID);

  /// Name of color attribute in folder subject hierarchy items
  Q_INVOKABLE QString colorItemAttributeName()const { return "Color"; };

  /// Create model display node for given item. If the folder item has an associated model hierarchy
  /// node, then create a display node associated to that. Otherwise create display node for folder item
  vtkMRMLDisplayNode* createDisplayNodeForItem(vtkIdType itemID);

protected slots:
  /// Create folder node under the scene
  void createFolderUnderScene();

  /// Create folder node under current node
  void createFolderUnderCurrentNode();

  /// Toggle apply color to branch
  void onApplyColorToBranchToggled(bool);

protected:
  /// Retrieve model display node for given item. If the folder item has an associated model display
  /// node (created by the plugin), then return that. Otherwise see if it has a model hierarchy node
  /// with a display node.
  vtkMRMLDisplayNode* displayNodeForItem(vtkIdType itemID)const;

  /// Determine if apply color to branch option is enabled to a given item or not
  bool isApplyColorToBranchEnabledForItem(vtkIdType itemID)const;
  /// Determine if apply color to branch option is enabled to a given item or not
  void setApplyColorToBranchEnabledForItem(vtkIdType itemID, bool enabled);

protected:
  QScopedPointer<qSlicerSubjectHierarchyFolderPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyFolderPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyFolderPlugin);
};

#endif
