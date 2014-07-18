/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class vtkObject;
class vtkMRMLNode;
class vtkMRMLSubjectHierarchyNode;
class QStandardItem;
class QAction;
class qSlicerAbstractModuleWidget;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
///    In Widgets, not Plugins because the paths and libs need to be exported to extensions
/// \brief Abstract plugin for handling Subject Hierarchy nodes
///
/// This class provides an interface and some default implementations for the common operations on
/// subject hierarchy nodes. To exercise the default implementations, a Default plugin \sa qSlicerSubjectHierarchyDefaultPlugin
/// has to be created.
///
/// Note about confidence values (\sa canAddNodeToSubjectHierarchy \sa canReparentNodeInsideSubjectHierarchy \sa canOwnSubjectHierarchyNode):
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
  Q_PROPERTY(QString name READ name)

public:
  typedef QObject Superclass;
  qSlicerSubjectHierarchyAbstractPlugin(QObject* parent = NULL);
  virtual ~qSlicerSubjectHierarchyAbstractPlugin();

// Role-related virtual methods
// If the subclass plugin does not offer a role, these do not need to be overridden
public:
  /// Determines if the actual plugin can handle a subject hierarchy node. The plugin with
  /// the highest confidence number will "own" the node in the subject hierarchy (set icon, tooltip,
  /// set context menu etc.)
  /// \param node Note to handle in the subject hierarchy tree
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier attribute)
  virtual double canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const;

  /// Get role that the plugin assigns to the subject hierarchy node.
  ///   Each plugin should provide only one role.
  Q_INVOKABLE virtual const QString roleForPlugin()const;

  /// Get help text for plugin to be added in subject hierarchy module widget help box
  virtual const QString helpText()const;

  /// Set icon of a owned subject hierarchy node
  /// \return Flag indicating whether setting an icon was successful
  virtual bool setIcon(vtkMRMLSubjectHierarchyNode* node, QStandardItem* item);

  /// Set visibility icon of a owned subject hierarchy node
  virtual void setVisibilityIcon(vtkMRMLSubjectHierarchyNode* node, QStandardItem* item);

  /// Open module belonging to node and set inputs in opened module
  virtual void editProperties(vtkMRMLSubjectHierarchyNode* node);

  /// Generate displayed name for the owned subject hierarchy node corresponding to its role.
  /// The default implementation removes the '_SubjectHierarchy' ending from the node's name.
  virtual QString displayedName(vtkMRMLSubjectHierarchyNode* node)const;

  /// Generate tooltip for a owned subject hierarchy node
  virtual QString tooltip(vtkMRMLSubjectHierarchyNode* node)const;

  /// Set display visibility of a owned subject hierarchy node
  virtual void setDisplayVisibility(vtkMRMLSubjectHierarchyNode* node, int visible);

  /// Get display visibility of a owned subject hierarchy node
  /// \return Display visibility (0: hidden, 1: shown, 2: partially shown)
  virtual int getDisplayVisibility(vtkMRMLSubjectHierarchyNode* node)const;

// Function related virtual methods
public:
  /// Get node context menu item actions to add to tree view
  Q_INVOKABLE virtual QList<QAction*> nodeContextMenuActions()const;

  /// Get scene context menu item actions to add to tree view
  /// Separate method is needed for the scene, as its actions are set to the
  /// tree by a different method \sa nodeContextMenuActions
  virtual QList<QAction*> sceneContextMenuActions()const;

  /// Show context menu actions valid for  given subject hierarchy node.
  /// \param node Subject Hierarchy node to show the context menu items for. If NULL, then shows menu items for the scene
  virtual void showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node) { Q_UNUSED(node); };

// Parenting related virtual methods with default implementation
public:
  /// Determines if a non subject hierarchy node can be placed in the hierarchy using the actual plugin,
  /// and gets a confidence value for a certain MRML node (usually the type and possibly attributes are checked).
  /// Most plugins do not perform steps additional to the default, so the default implementation returns a 0
  /// confidence value, which can be overridden in plugins that do handle special cases.
  /// \param node Node to be added to the hierarchy
  /// \param parent Prospective parent of the node to add.
  ///   Default value is NULL. In that case the parent will be ignored, the confidence numbers are got based on the to-be child node alone.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier attribute)
  virtual double canAddNodeToSubjectHierarchy(vtkMRMLNode* node , vtkMRMLSubjectHierarchyNode* parent=NULL)const;

  /// Add a node to subject hierarchy under a specified parent node. If added non subject hierarchy nodes
  ///   have certain steps to perform when adding them in Subject Hierarchy, those steps take place here.
  /// \return True if added successfully, false otherwise
  virtual bool addNodeToSubjectHierarchy(vtkMRMLNode* node, vtkMRMLSubjectHierarchyNode* parent);

  /// Determines if a subject hierarchy node can be reparented in the hierarchy using the actual plugin,
  /// and gets a confidence value for a certain MRML node (usually the type and possibly attributes are checked).
  /// Most plugins do not perform steps additional to the default, so the default implementation returns a 0
  /// confidence value, which can be overridden in plugins that do handle special cases.
  /// \param node Node to be reparented in the hierarchy
  /// \param parent Prospective parent of the node to reparent.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier attribute)
  virtual double canReparentNodeInsideSubjectHierarchy(vtkMRMLSubjectHierarchyNode* node, vtkMRMLSubjectHierarchyNode* parent)const;

  /// Reparent a node that was already in the subject hierarchy under a new parent.
  /// \return True if reparented successfully, false otherwise
  virtual bool reparentNodeInsideSubjectHierarchy(vtkMRMLSubjectHierarchyNode* node, vtkMRMLSubjectHierarchyNode* parent);

// Utility functions
public:
  /// Create child node for a given parent node based on the child level map
  virtual vtkMRMLSubjectHierarchyNode* createChildNode(vtkMRMLSubjectHierarchyNode* parentNode, QString nodeName, vtkMRMLNode* associatedNode=NULL);

  /// Determines if the node is owned by this plugin
  bool isThisPluginOwnerOfNode(vtkMRMLSubjectHierarchyNode* node)const;

  /// Emit owner plugin changed signal
  void emitOwnerPluginChanged(vtkObject* node, void* callData);

  /// Switch to module with given name
  /// \return Widget representation of the module if found, NULL otherwise
  static qSlicerAbstractModuleWidget* switchToModule(QString moduleName);

public:
  /// Get the name of the plugin
  virtual QString name()const;

  /// Get child level map
  QMap<QString, QString> childLevelMap()const { return m_ChildLevelMap; };

signals:
  /// Signal requesting expanding of the subject hierarchy tree item belonging to a node
  void requestExpandNode(vtkMRMLSubjectHierarchyNode* node);

  /// Signal requesting invalidating the models for the tree view and the potential nodes list
  void requestInvalidateModels()const;

  /// Signal that is emitted when a node changes owner plugin
  /// \param node Subject hierarchy node changing owner plugin
  /// \param callData Name of the old plugin (the name of the new plugin can be get from the node)
  void ownerPluginChanged(vtkObject* node, void* callData);

protected:
  /// Get child level according to child level map of the current plugin
  virtual QString childLevel(QString parentLevel);

  /// Hide all context menu actions offered by the plugin.
  /// This method must be called as a first step in \sa showContextMenuActionsForNode
  /// before showing the actions that apply to the current situation. Calling this method
  /// prevents programming errors made in case plugin actions change.
  void hideAllContextMenuActions()const;

protected slots:
  /// Create supported child for the current node (which is selected in the tree)
  void createChildForCurrentNode();

protected:
  /// Name of the plugin
  QString m_Name;

  /// Map assigning a child level to a parent level for the plugin
  QMap<QString, QString> m_ChildLevelMap;

private:
  qSlicerSubjectHierarchyAbstractPlugin(const qSlicerSubjectHierarchyAbstractPlugin&); // Not implemented
  void operator=(const qSlicerSubjectHierarchyAbstractPlugin&); // Not implemented
};

#endif
