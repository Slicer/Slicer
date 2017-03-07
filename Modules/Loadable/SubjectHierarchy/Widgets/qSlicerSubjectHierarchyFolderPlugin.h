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

// MRML includes
#include <vtkMRMLHierarchyNode.h>

class qSlicerSubjectHierarchyFolderPluginPrivate;

// Due to some reason the Python wrapping of this class fails, therefore
// put everything between BTX/ETX to exclude from wrapping.
// TODO investigate why the wrapping fails:
//   https://www.assembla.com/spaces/slicerrt/tickets/210-python-wrapping-error-when-starting-up-slicer-with-slicerrt
//BTX

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
/// \brief Subject hierarchy folder plugin
///
/// 1. Supports folder items in subject hierarchy
/// 2. Supports node hierarchies in subject hierarchy by adding folder items for the intermediate nodes.
///    Handled cases
///
///    Scene changes
///    -------------
///
///    1. Hierarchy node added
///      -> Add folder item
///         ( folderPlugin::addNodeToSubjectHierarchy )
///
///    2. New parent is set to a hierarchy node
///      (Modified event for both old and new parents)
///      newParent.vtkMRMLHierarchyNode::ChildNodeAddedEvent(hierarchyNode)
///      -> Set parent of item for hierarchy node to item for new parent
///         ( folderPlugin::onMRMLHierarchyNodeChildNodeAdded )
///
///    3. Scene import ends
///      -> Add each hierarchy node to subject hierarchy and resolve hierarchy on each added item
///        ( addSupportedDataNodesToSubjectHierarchy,
///          folderPlugin::addNodeToSubjectHierarchy,
///          resolveHierarchyForItem )
///
///    4. Data node is associated to a hierarchy node
///      dataNode.vtkMRMLNode::HierarchyModifiedEvent
///      (Should only happen if building hierarchy programmaticaly from scratch)
///         Remove hierarchy node's item from subject hierarchy
///         ( folderPlugin::onDataNodeAssociatedToHierarchyNode )
///
///    Subject hierarchy changes
///    -------------------------
///
///    1. Item is reparented under an item with node hierarchy
///      1a. New parent has associated hierarchy node
///        -> Set parent item's hierarchy node as parent of reparented item's hierarchy node
///           ( folderPlugin::reparentItemInsideSubjectHierarchy )
///      1b. Otherwise
///        -> Set scene as parent of reparented item's hierarchy node
///           ( folderPlugin::reparentItemInsideSubjectHierarchy )
///
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyFolderPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyFolderPlugin(QObject* parent = NULL);
  virtual ~qSlicerSubjectHierarchyFolderPlugin();

public:
  /// Determines if the actual plugin can handle a subject hierarchy item. The plugin with
  /// the highest confidence number will "own" the item in the subject hierarchy (set icon, tooltip,
  /// set context menu etc.)
  /// \param item Item to handle in the subject hierarchy tree
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item (by node type or identifier attribute)
  virtual double canOwnSubjectHierarchyItem(vtkIdType itemID)const;

  /// Get role that the plugin assigns to the subject hierarchy item.
  ///   Each plugin should provide only one role.
  Q_INVOKABLE virtual const QString roleForPlugin()const;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, NULL if nothing to set
  virtual QIcon icon(vtkIdType itemID);

  /// Get visibility icon for a visibility state
  virtual QIcon visibilityIcon(int visible);

  /// Open module belonging to item and set inputs in opened module
  virtual void editProperties(vtkIdType itemID);

  /// Get item context menu item actions to add to tree view
  virtual QList<QAction*> itemContextMenuActions()const;

  /// Get scene context menu item actions to add to tree view
  /// Separate method is needed for the scene, as its actions are set to the
  /// tree by a different method \sa itemContextMenuActions
  virtual QList<QAction*> sceneContextMenuActions()const;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  virtual void showContextMenuActionsForItem(vtkIdType itemID);

public:
  /// Determines if a data node can be placed in the hierarchy using the actual plugin,
  /// and gets a confidence value for a certain MRML node (usually the type and possibly attributes are checked).
  /// \param node Node to be added to the hierarchy
  /// \param parentItemID Prospective parent of the node to add.
  ///   Default value is invalid. In that case the parent will be ignored, the confidence numbers are got based on the to-be child node alone.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier attribute)
  virtual double canAddNodeToSubjectHierarchy(
    vtkMRMLNode* node,
    vtkIdType parentItemID=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )const;

  /// Add a node to subject hierarchy under a specified parent. This is basically a convenience function to
  /// call \sa vtkMRMLSubjectHierarchyNode::CreateItem
  /// \param node Node to add to subject hierarchy
  /// \param parentItemID Parent item of the added node
  /// \param level Level of the added node in subject hierarchy, none be default
  /// \return True if added successfully, false otherwise
  virtual bool addNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID, std::string level="");

  /// Determines if a subject hierarchy item can be reparented in the hierarchy using the current plugin,
  /// and gets a confidence value for the reparented item.
  /// Most plugins do not perform steps additional to the default, so the default implementation returns a 0
  /// confidence value, which can be overridden in plugins that do handle special cases.
  /// \param itemID Item to be reparented in the hierarchy
  /// \param parentItemID Prospective parent of the item to reparent.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item
  virtual double canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)const;

  /// Reparent an item that was already in the subject hierarchy under a new parent.
  /// \return True if reparented successfully, false otherwise
  virtual bool reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID);

public:
  /// Create folder under specified item
  /// \param parentNode Parent item for folder to create
  Q_INVOKABLE vtkIdType createFolderUnderItem(vtkIdType parentItemID);

  /// Get hierarchy node for subject hierarchy item.
  /// The item can be associated directly to a hierarchy node (if it's an intermediate hierarchy node),
  /// or it can be associated to a data node that is associated to a hierarchy node.
  /// \return The hierarchy node associated to the item either directly or indirectly. NULL otherwise
  Q_INVOKABLE vtkMRMLHierarchyNode* hierarchyNodeForItem(vtkIdType itemID);

  /// Get subject hierarchy item for a hierarchy node.
  /// The item can be associated directly to a hierarchy node (if it's an intermediate hierarchy node),
  /// or it can be associated to a data node that is associated to a hierarchy node.
  /// \return ID of the item associated to the hierarchy node directly or indirectly. Invalid ID otherwise.
  Q_INVOKABLE vtkIdType itemForHierarchyNode(vtkMRMLHierarchyNode* hierarchyNode);

  /// Add the item for the hierarchy node to the proper place in subject hierarchy.
  /// If the parent node hierarchy item does not exist yet, add that too, all the way to the scene
  Q_INVOKABLE bool resolveHierarchyForItem(vtkIdType itemID);

  /// Resolve all node hierarchy items in the scene.
  /// Traverses scene for hierarchy nodes, and makes sure the same parents are set in subject hierarchy
  Q_INVOKABLE bool resolveHierarchies();

public slots:
  /// Called when hierarchy modified event is invoked for a data node
  /// Ensures that if a hierarchy node gets associated to a data node, then the item for the hierarchy
  /// node is removed from subject hierarchy (the logic implemented in the folder plugin handles data
  /// nodes with hierarchy nodes)
  void onDataNodeAssociatedToHierarchyNode(vtkObject* dataNodeObject);

protected slots:
  /// Create folder node under the scene
  void createFolderUnderScene();

  /// Create folder node under current node
  void createFolderUnderCurrentNode();

  /// Called when child node was added to hierarchy node
  /// Ensures that the hierarchy specified by node hierarchies are mirrored in subject hierarchy
  void onHierarchyNodeChildNodeAdded(vtkObject* parentNodeObject, vtkObject* childNodeObject);

protected:
  QScopedPointer<qSlicerSubjectHierarchyFolderPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyFolderPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyFolderPlugin);
};

//ETX

#endif
