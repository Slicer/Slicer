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

#ifndef __qSlicerSubjectHierarchyAbstractPlugin_h
#define __qSlicerSubjectHierarchyAbstractPlugin_h

// Qt includes
#include <QObject>
#include <QMap>
#include <QStringList>
#include <QIcon>
#include <QVariant>

// MRML includes
#include <vtkMRMLSubjectHierarchyNode.h>

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class QStandardItem;
class QAction;
class QColor;
class qSlicerAbstractModuleWidget;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
///    In Widgets, not Plugins because the paths and libs need to be exported to extensions
/// \brief Abstract plugin for handling Subject Hierarchy items
///
/// This class provides an interface and some default implementations for the common operations on
/// subject hierarchy items. To exercise the default implementations, a Default plugin \sa qSlicerSubjectHierarchyDefaultPlugin
/// has to be created.
///
/// Note about confidence values (\sa canAddNodeToSubjectHierarchy, \sa canReparentItemInsideSubjectHierarchy,
///   \sa canOwnSubjectHierarchyItem - in case of the latter two the node mentioned below is the data node associated to the item):
/// The confidence value is a floating point number between 0.0 and 1.0. Meaning of some typical values:
/// 0.0 = The plugin cannot handle the node in question at all
/// 0.3 = It is likely that other plugins will be able to handle the node in question better (typical value for plugins for generic types, such as Volumes)
/// 0.5 = The plugin has equal chance to handle this node as others (an example can be color table node)
/// 0.7 = The plugin is likely be the only one that can handle the node in question, but there is a chance that other plugins can do that too
/// 1.0 = The node in question can only be handled by the plugin (by node type or identifier attribute)
///
/// Plugin subclass naming convention:
/// - Plugins providing roles: qSubjectHierarchy[Role name, noun]Plugin
/// - Plugins providing functions: qSubjectHierarchy[Function name, verb]Plugin
///
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyAbstractPlugin : public QObject
{
  Q_OBJECT

  /// This property stores the name of the plugin
  /// Cannot be empty.
  /// \sa name()
  Q_PROPERTY(QString name READ name WRITE setName)

public:
  typedef QObject Superclass;
  qSlicerSubjectHierarchyAbstractPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyAbstractPlugin() override;

// Role-related virtual methods
// If the subclass plugin does not offer a role, these do not need to be overridden
public:
  /// Determines if the actual plugin can handle a subject hierarchy item. The plugin with
  /// the highest confidence number will "own" the item in the subject hierarchy (set icon, tooltip,
  /// set context menu etc.)
  /// \param item Item to handle in the subject hierarchy tree
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item (by node type or identifier attribute)
  Q_INVOKABLE virtual double canOwnSubjectHierarchyItem(vtkIdType itemID)const;

  /// Get role that the plugin assigns to the subject hierarchy item.
  ///   Each plugin should provide only one role.
  Q_INVOKABLE virtual const QString roleForPlugin()const;

  /// Get help text for plugin to be added in subject hierarchy module widget help box
  virtual const QString helpText()const;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, empty icon if nothing to set
  virtual QIcon icon(vtkIdType itemID);

  /// Get visibility icon for a visibility state
  Q_INVOKABLE virtual QIcon visibilityIcon(int visible);

  /// Open module belonging to item and set inputs in opened module
  Q_INVOKABLE virtual void editProperties(vtkIdType itemID);

  /// Generate displayed name for the owned subject hierarchy item corresponding to its role.
  /// The default implementation returns the associated data node's name if any, otherwise the item name
  virtual QString displayedItemName(vtkIdType itemID)const;

  /// Generate tooltip for a owned subject hierarchy item
  Q_INVOKABLE virtual QString tooltip(vtkIdType itemID)const;

  /// Set display visibility of an owned subject hierarchy item
  Q_INVOKABLE virtual void setDisplayVisibility(vtkIdType itemID, int visible);

  /// Get display visibility of an owned subject hierarchy item
  /// \return Display visibility (0: hidden, 1: shown, 2: partially shown)
  Q_INVOKABLE virtual int getDisplayVisibility(vtkIdType itemID)const;

  /// Set display color of an owned subject hierarchy item
  /// \param color Display color to set
  /// \param terminologyMetaData Map containing terminology meta data. Contents:
  ///   qSlicerTerminologyItemDelegate::TerminologyRole        : string
  ///   qSlicerTerminologyItemDelegate::NameRole               : string
  ///   qSlicerTerminologyItemDelegate::NameAutoGeneratedRole  : bool
  ///   qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole : bool
  Q_INVOKABLE virtual void setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData);

  /// Get display color of an owned subject hierarchy item
  /// \param terminologyMetaData Output map containing terminology meta data. Contents:
  ///   qSlicerTerminologyItemDelegate::TerminologyRole        : string
  ///   qSlicerTerminologyItemDelegate::NameRole               : string
  ///   qSlicerTerminologyItemDelegate::NameAutoGeneratedRole  : bool
  ///   qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole : bool
  Q_INVOKABLE virtual QColor getDisplayColor(vtkIdType itemID, QMap<int, QVariant> &terminologyMetaData)const;

  // Function related virtual methods
public:
  /// Get item context menu item actions to add to tree view.
  /// These item context menu actions can be shown in the implementations of \sa showContextMenuActionsForItem
  Q_INVOKABLE virtual QList<QAction*> itemContextMenuActions()const;

  /// Get scene context menu item actions to add to tree view. Also provides actions for right-click on empty area
  /// These context menu actions are shown for the scene in the implementations of \sa showContextMenuActionsForItem
  /// Separate method is needed for the scene, as its actions are set to the
  /// tree by a different method \sa itemContextMenuActions
  Q_INVOKABLE virtual QList<QAction*> sceneContextMenuActions()const;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  Q_INVOKABLE virtual void showContextMenuActionsForItem(vtkIdType itemID) { Q_UNUSED(itemID); };

  /// Get visibility context menu item actions to add to tree view.
  /// These item visibility context menu actions can be shown in the implementations of \sa showVisibilityContextMenuActionsForItem
  Q_INVOKABLE virtual QList<QAction*> visibilityContextMenuActions()const;

  /// Show visibility context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the visibility context menu items for
  Q_INVOKABLE virtual void showVisibilityContextMenuActionsForItem(vtkIdType itemID) { Q_UNUSED(itemID); };

  /// Get view context menu item actions that are available when right-clicking an object in the views.
  /// These item context menu actions can be shown in the implementations of \sa showViewContextMenuActionsForItem
  /// Note: The actions need object names set so that they can be included in the white list
  Q_INVOKABLE virtual QList<QAction*> viewContextMenuActions()const;

  /// Show context menu actions valid for a given subject hierarchy item to be shown in the view.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  /// \param eventData Supplementary data for the item that may be considered for the menu (sub-item ID, attribute, etc.)
  Q_INVOKABLE virtual void showViewContextMenuActionsForItem(vtkIdType itemID, QVariantMap eventData) { Q_UNUSED(itemID); Q_UNUSED(eventData); };

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
  virtual double canAddNodeToSubjectHierarchy(
    vtkMRMLNode* node, vtkIdType parentItemID=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )const;

  /// Add a node to subject hierarchy under a specified parent. This is basically a convenience function to
  /// call \sa vtkMRMLSubjectHierarchyNode::CreateItem
  /// \param node Node to add to subject hierarchy
  /// \param parentItemID Parent item of the added node
  /// \return True if added successfully, false otherwise
  virtual bool addNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID);

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

// Utility functions
public:
  /// Determines if the item is owned by this plugin
  Q_INVOKABLE bool isThisPluginOwnerOfItem(vtkIdType itemID)const;

  /// Switch to module with given name
  /// \return Widget representation of the module if found, nullptr otherwise
  Q_INVOKABLE static qSlicerAbstractModuleWidget* switchToModule(QString moduleName);

public:
  /// Get the name of the plugin
  virtual QString name()const;

  /// Set the name of the plugin
  /// NOTE: name must be defined in constructor in C++ plugins, this can only be used in python scripted ones
  virtual void setName(QString name);

signals:
  /// Signal requesting expanding of the subject hierarchy tree item belonging to an item
  void requestExpandItem(vtkIdType itemID);

  /// Signal requesting invalidating the filter model for the tree view
  /// (e.g. when an item is added or removed by the plugin)
  void requestInvalidateFilter()const;

protected:
  /// Hide all context menu actions offered by the plugin.
  /// This method must be called as a first step in \sa showContextMenuActionsForItem
  /// before showing the actions that apply to the current situation. Calling this method
  /// prevents programming errors made in case plugin actions change.
  void hideAllContextMenuActions()const;

protected:
  /// Name of the plugin
  QString m_Name;

private:
  qSlicerSubjectHierarchyAbstractPlugin(const qSlicerSubjectHierarchyAbstractPlugin&); // Not implemented
  void operator=(const qSlicerSubjectHierarchyAbstractPlugin&); // Not implemented
  friend class qMRMLSubjectHierarchyTreeView;
  friend class qSlicerSubjectHierarchyPluginLogic;
};

#endif
