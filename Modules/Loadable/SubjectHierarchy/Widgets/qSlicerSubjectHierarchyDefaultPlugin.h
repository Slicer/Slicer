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

#ifndef __qSlicerSubjectHierarchyDefaultPlugin_h
#define __qSlicerSubjectHierarchyDefaultPlugin_h

// Qt includes
#include <QObject>

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qSlicerSubjectHierarchyDefaultPluginPrivate;
class QIcon;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
/// \brief Default Subject Hierarchy plugin to exercise features of the abstract plugin.
///   This plugin must not be registered as this is the fall back plugin called when there is none found
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyDefaultPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyDefaultPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyDefaultPlugin() override;

public:
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

  /// Get help text for plugin to be added in subject hierarchy module widget help box
  const QString helpText()const override;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, nullptr if nothing to set
  QIcon icon(vtkIdType itemID) override;

  /// Get visibility icon for a visibility state
  QIcon visibilityIcon(int visible) override;

  /// Get visibility context menu item actions to add to tree view.
  /// These item visibility context menu actions can be shown in the implementations of \sa showVisibilityContextMenuActionsForItem
  QList<QAction*> visibilityContextMenuActions()const override;

  /// Show visibility context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the visibility context menu items for
  void showVisibilityContextMenuActionsForItem(vtkIdType itemID) override;

public:
  /// Set default visibility icons owned by the scene model so that the default plugin can set them
  void setDefaultVisibilityIcons(QIcon visibleIcon, QIcon hiddenIcon, QIcon partiallyVisibleIcon);

public slots:
  /// Toggle visibility for current item
  void toggleVisibility();

  /// Show all direct children of current item
  void showAllChildren();

  /// Hide all direct children of current item
  void hideAllChildren();

protected:
  QScopedPointer<qSlicerSubjectHierarchyDefaultPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyDefaultPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyDefaultPlugin);
};

#endif
