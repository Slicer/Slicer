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

#ifndef __qSlicerSubjectHierarchyScriptedPlugin_h
#define __qSlicerSubjectHierarchyScriptedPlugin_h

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif
class qSlicerSubjectHierarchyScriptedPluginPrivate;

class vtkObject;
class QStandardItem;
class QAction;
class qSlicerAbstractModuleWidget;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
///    In Widgets, not Plugins because the paths and libs need to be exported to extensions
/// \brief Scripted abstract plugin for handling subject hierarchy items or providing actions for items
///
/// This class provides an interface to plugins implemented in python.
/// USAGE: Subclass AbstractScriptedSubjectHierarchyPlugin in SubjectHierarchyPlugins subfolder
///   of python scripted module, and register plugin by creating this class in module (e.g.
///   setup method of module widget) and setting python source to implemented plugin subclass.
///   Example can be found here: https://subversion.assembla.com/svn/slicerrt/trunk/VolumeClip/src
///
/// Note about confidence values (\sa canAddNodeToSubjectHierarchy \sa canReparentItemInsideSubjectHierarchy \sa canOwnSubjectHierarchyItem):
/// The confidence value is a floating point number between 0.0 and 1.0. Meaning of some typical values:
/// 0.0 = The plugin cannot handle the item in question at all
/// 0.3 = It is likely that other plugins will be able to handle the item in question better (typical value for plugins for generic types, such as Volumes)
/// 0.5 = The plugin has equal chance to handle this item as others (an example can be color table node)
/// 0.7 = The plugin is likely be the only one that can handle the item in question, but there is a chance that other plugins can do that too
/// 1.0 = The item in question can only be handled by the plugin (by node type or identifier attribute)
///
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyScriptedPlugin
  : public qSlicerSubjectHierarchyAbstractPlugin
{
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyScriptedPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyScriptedPlugin() override;

  Q_INVOKABLE QString pythonSource()const;

  /// Set python source for the implemented plugin
  /// \param newPythonSource Python file path
  Q_INVOKABLE bool setPythonSource(const QString newPythonSource);

  /// Convenience method allowing to retrieve the associated scripted instance
  Q_INVOKABLE PyObject* self() const;

  /// Set the name property value.
  /// \sa name
  void setName(QString name) override;

// Role-related virtual methods
// If the subclass plugin does not offer a role, these do not need to be overridden
public:
  /// Determines if the actual plugin can handle a subject hierarchy item. The plugin with
  /// the highest confidence number will "own" the item in the subject hierarchy (set icon, tooltip,
  /// set context menu etc.)
  /// \param item Item to handle in the subject hierarchy tree
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item (by node type or identifier attribute)
  double canOwnSubjectHierarchyItem(vtkIdType itemID)const override;

  /// Get role that the plugin assigns to the subject hierarchy item.
  ///   Each plugin should provide only one role.
  const QString roleForPlugin()const override;

  /// Get help text for plugin to be added in subject hierarchy module widget help box
  const QString helpText()const override;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, nullptr if nothing to set
  QIcon icon(vtkIdType itemID) override;

  /// Get visibility icon for a visibility state
  QIcon visibilityIcon(int visible) override;

  /// Open module belonging to item and set inputs in opened module
  void editProperties(vtkIdType itemID) override;

  /// Generate displayed name for the owned subject hierarchy item corresponding to its role.
  /// The default implementation returns the associated data node's name if any, otherwise the item name
  QString displayedItemName(vtkIdType itemID)const override;

  /// Generate tooltip for a owned subject hierarchy item
  QString tooltip(vtkIdType itemID)const override;

  /// Set display visibility of a owned subject hierarchy item
  void setDisplayVisibility(vtkIdType itemID, int visible) override;

  /// Get display visibility of a owned subject hierarchy item
  /// \return Display visibility (0: hidden, 1: shown, 2: partially shown)
  int getDisplayVisibility(vtkIdType itemID)const override;

// Function related virtual methods
public:
  /// Get item context menu item actions to add to tree view
  QList<QAction*> itemContextMenuActions()const override;

  /// Get view item context menu item actions to add to views
  QList<QAction*> viewContextMenuActions()const override;

  /// Get scene context menu item actions to add to tree view
  /// Separate method is needed for the scene, as its actions are set to the
  /// tree by a different method \sa itemContextMenuActions
  QList<QAction*> sceneContextMenuActions()const override;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  void showContextMenuActionsForItem(vtkIdType itemID) override;

  /// Show view context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  /// \param eventData Supplementary data for the item that may be considered for the menu (sub-item ID, attribute, etc.)
  void showViewContextMenuActionsForItem(vtkIdType itemID, QVariantMap eventData) override;

  // Parenting related virtual methods with default implementation
public:
  /// Determines if a data node can be placed in the hierarchy using the actual plugin,
  /// and gets a confidence value for a certain MRML node (usually the type and possibly attributes are checked).
  /// Most plugins do not perform steps additional to the default, so the default implementation returns a 0
  /// confidence value, which can be overridden in plugins that do handle special cases.
  /// \param node Node to be added to the hierarchy
  /// \param parentItemID Prospective parent of the node to add.
  ///   Default value is invalid. In that case the parent will be ignored, the confidence numbers are got based on the to-be child node alone.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier attribute)
  double canAddNodeToSubjectHierarchy(
    vtkMRMLNode* node,
    vtkIdType parentItemID=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )const override;

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

protected:
  QScopedPointer<qSlicerSubjectHierarchyScriptedPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyScriptedPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyScriptedPlugin);
};

#endif
