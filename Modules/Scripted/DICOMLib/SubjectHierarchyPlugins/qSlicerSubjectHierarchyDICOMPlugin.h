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

#ifndef __qSlicerSubjectHierarchyDICOMPlugin_h
#define __qSlicerSubjectHierarchyDICOMPlugin_h

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyFolderPlugin.h"

#include "qSlicerDICOMLibSubjectHierarchyPluginsExport.h"

class qSlicerSubjectHierarchyDICOMPluginPrivate;

/// \ingroup Slicer_QtModules_DICOMLob_SubjectHierarchyPlugins
///
/// The DICOM subject hierarchy plugin provides subject hierarchy items for
/// Subject and Study, in addition to functions to convert folders to these
/// types. It also supplies the context menu for DICOM export.
/// This plugin is a subclass of the Folder plugin, in order to be able to
/// override display properties of their branches.
class Q_SLICER_DICOMLIB_SUBJECT_HIERARCHY_PLUGINS_EXPORT qSlicerSubjectHierarchyDICOMPlugin : public qSlicerSubjectHierarchyFolderPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyFolderPlugin Superclass;
  qSlicerSubjectHierarchyDICOMPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyDICOMPlugin() override;

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

  /// Get help text for plugin to be added in subject hierarchy module widget help box
  const QString helpText()const override;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, empty icon if nothing to set
  QIcon icon(vtkIdType itemID) override;

  /// Get visibility icon for a visibility state
  QIcon visibilityIcon(int visible) override;

  /// Open module belonging to item and set inputs in opened module
  void editProperties(vtkIdType itemID) override;

  /// Get item context menu item actions to add to tree view
  QList<QAction*> itemContextMenuActions()const override;

  /// Get scene context menu item actions to add to tree view. Also provides actions for right-click on empty area
  /// Separate method is needed for the scene, as its actions are set to the
  /// tree by a different method \sa itemContextMenuActions
  QList<QAction*> sceneContextMenuActions()const override;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  void showContextMenuActionsForItem(vtkIdType itemID) override;

  /// Create a default patient item in the subject hierarchy
  /// (with anonymous patient name and random patient ID).
  Q_INVOKABLE vtkIdType createDefaultPatientItem();

  /// Create a default study item in the subject hierarchy
  /// (with no study description and random study instance UID).
  Q_INVOKABLE vtkIdType createDefaultStudyItem();

protected slots:
  /// Create patient item
  void createSubjectItem();

  /// Create study item under current item (must be patient)
  void createChildStudyUnderCurrentItem();

  /// Convert current item (must be folder) to patient
  void convertCurrentItemToPatient();

  /// Convert current item (must be folder) to study
  void convertCurrentItemToStudy();

  /// Open DICOM export dialog to export the selected series
  void openDICOMExportDialog();

protected:
  QScopedPointer<qSlicerSubjectHierarchyDICOMPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyDICOMPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyDICOMPlugin);
};

#endif
