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

#ifndef __qSlicerSubjectHierarchyParseLocalDataPlugin_h
#define __qSlicerSubjectHierarchyParseLocalDataPlugin_h

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qSlicerSubjectHierarchyParseLocalDataPluginPrivate;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyParseLocalDataPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyParseLocalDataPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyParseLocalDataPlugin() override;

public:
  /// Get scene context menu item actions to add to tree view
  /// Separate method is needed for the scene, as its actions are set to the
  /// tree by a different method \sa itemContextMenuActions
  QList<QAction*> sceneContextMenuActions()const override;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  void showContextMenuActionsForItem(vtkIdType itemID) override;

protected slots:
  /// Create subject hierarchy from loaded local directories.
  /// Organizes all items in subject hierarchy that have storable data nodes and has a valid storage node with a file
  /// name (meaning it has been loaded from local disk). Creates patient/study/series hierarchies according to the
  /// paths of the loaded files, ignoring the part that is identical (if everything has been loaded from the same directory,
  /// then only creates subject hierarchy nodes for the directories within that directory).
  void createHierarchyFromLoadedDirectoryStructure();

protected:
  QScopedPointer<qSlicerSubjectHierarchyParseLocalDataPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyParseLocalDataPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyParseLocalDataPlugin);
};

#endif
