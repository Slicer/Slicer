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

#ifndef __qSlicerSubjectHierarchyPluginLogic_h
#define __qSlicerSubjectHierarchyPluginLogic_h

// Slicer includes
#include "qSlicerObject.h"

// CTK includes
#include <ctkVTKObject.h>

// Qt includes
#include <QObject>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class QAction;
class QMenu;
class qSlicerSubjectHierarchyPluginLogicPrivate;
class qSlicerSubjectHierarchyAbstractPlugin;

/// \ingroup Slicer_QtModules_SubjectHierarchy
/// \brief Qt-based logic class to exercise Qt-related logic functions. The Subject
///   hierarchy plugin mechanism and those are Qt classes
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyPluginLogic :
  public QObject, public virtual qSlicerObject
{
  Q_OBJECT
  QVTK_OBJECT

  /// Allow-list for view context menu actions. If empty (by default) then all registered view context menu action names will be displayable.
  /// Allow-list can be further restricted for a specific node by using setAllowedViewContextMenuActionNamesForItem method.
  Q_PROPERTY(QStringList allowedViewContextMenuActionNames READ allowedViewContextMenuActionNames WRITE setAllowedViewContextMenuActionNames)
  /// List of all registered view context menu actions.
  Q_PROPERTY(QStringList registeredViewContextMenuActionNames READ registeredViewContextMenuActionNames)

public:
  typedef QObject Superclass;
  qSlicerSubjectHierarchyPluginLogic(QObject *parent=nullptr);
  ~qSlicerSubjectHierarchyPluginLogic() override;

public:
  /// Set the current MRML scene to the logic
  void setMRMLScene(vtkMRMLScene* scene) override;

  /// Assessor function for getting subject hierarchy plugin by name (for python)
  Q_INVOKABLE qSlicerSubjectHierarchyAbstractPlugin* subjectHierarchyPluginByName(QString name)const;

  /// Assessor function for getting currently selected subject hierarchy item (for python)
  Q_INVOKABLE vtkIdType currentSubjectHierarchyItem()const;

  /// Utility test function to be able to set currently selected subject hierarchy node from python
  /// Only used in python tests!
  Q_INVOKABLE void setCurrentSubjectHierarchyItem(vtkIdType itemID);

  /// Register subject hierarchy core plugins
  /// Note: Registering plugins provided by other modules is the responsibility
  ///       of the module!
  void registerCorePlugins();

  /// Get all view context menu actions available
  /// \return List of object names of all registered view menu actions
  QStringList registeredViewContextMenuActionNames();

  /// Set list of view context menu action names that are allowed to be displayed.
  /// Allow-list can be further restricted for a specific node by using setAllowedViewContextMenuActionNamesForItem method.
  /// \param actionObjectNames List of view context menu actions to consider for displaying.
  ///        Only actions that are chosen to be visible by the owner plugin and listed in
  ///        actionObjectNames will be displayed to the user.
  void setAllowedViewContextMenuActionNames(QStringList actionObjectNames);

  /// Get desired set of view menu actions.
  QStringList allowedViewContextMenuActionNames() const;

  /// Set allowed view context menu items for a specific item. Only those actions are offered for the item
  /// that are allowed overall (using allowedViewContextMenuActionNames) and using this method, too.
  /// List of allowed action names are stored in "allowedViewContextMenuActions" subject hierarchy
  /// item attribute (separated by semicolons).
  /// \sa setAllowedViewContextMenuActionNames
  Q_INVOKABLE void setAllowedViewContextMenuActionNamesForItem(vtkIdType itemID, const QStringList& actionObjectNames);

  /// Get allowed view context menu items for a specific item.
  /// \sa setAllowedViewContextMenuActionNamesForItem
  Q_INVOKABLE QStringList allowedViewContextMenuActionNamesForItem(vtkIdType itemID);

  /// Create menu from list of actions.
  /// Uses "section" property to determine position of the action in the menu:
  /// each integer section value corresponds to a section and fractional part is used for ordering actions within the section.
  /// \param menu will be set by inserting the actions. If it is set to nullptr then a string will be returned that contains
  /// name and "section" value of each action.
  /// \param allowedActions specifies object name of actions may be added to the menu. If the list is empty then it is ignored.
  static Q_INVOKABLE QString buildMenuFromActions(QMenu* menu, QList< QAction* > actions, const QStringList& allowedActions=QStringList());

protected:
  /// Add observations for node that was added to subject hierarchy
  void observeNode(vtkMRMLNode* node);

  /// Add supported nodes to subject hierarchy.
  /// This method is called if auto-creation is enabled and a supported node is added to the
  /// scene, or if the user answers yes to the question that pops up upon entering subject
  /// hierarchy module if supported nodes are found that are not in the hierarchy.
  void addSupportedDataNodesToSubjectHierarchy();

  /// Add view menu action. Called by plugin handler when registering a plugin
  void registerViewContextMenuAction(QAction* action);

protected slots:
  /// Called when a node is added to the scene so that a plugin can create an item for it
  void onNodeAdded(vtkObject* scene, vtkObject* nodeObject);
  /// Called when a node is removed from the scene so that the associated
  /// subject hierarchy item can be deleted too
  void onNodeAboutToBeRemoved(vtkObject* scene, vtkObject* nodeObject);
  /// Called when a node is removed from the scene so if the subject hierarchy node is
  /// removed, it is re-created and the hierarchy rebuilt
  void onNodeRemoved(vtkObject* scene, vtkObject* nodeObject);
  /// Called when scene import is finished.
  /// Subject hierarchy items are created for supported data nodes if they have not
  /// been imported with the scene (backwards compatibility for older scenes)
  void onSceneImportEnded(vtkObject* sceneObject);
  /// Called when scene import is finished. Hierarchy is cleared in that case.
  void onSceneCloseEnded(vtkObject* sceneObject);
  /// Called when scene restore is finished. As the restored node contains only unresolved
  /// items, they need to be resolved when restoring ended
  void onSceneRestoreEnded(vtkObject* sceneObject);
  /// Called when batch processing is ended. Subject hierarchy is updated after batch processing,
  /// when nodes may be added/removed without individual events
  void onSceneBatchProcessEnded(vtkObject* sceneObject);

  /// Called when display node modified event is invoked on an owned displayable node
  void onDisplayNodeModified(vtkObject*, vtkObject*);
  /// Called when menu event is invoked on a display node of an owned displayable node
  void onDisplayMenuEvent(vtkObject*, vtkObject*);
  /// Called when the user clicks the Edit properties View menu action
  void editProperties();

protected:
  QScopedPointer<qSlicerSubjectHierarchyPluginLogicPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyPluginLogic);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyPluginLogic);
  friend class qSlicerSubjectHierarchyPluginHandler;
};

#endif
