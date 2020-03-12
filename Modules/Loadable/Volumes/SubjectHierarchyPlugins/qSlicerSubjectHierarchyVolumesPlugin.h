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

#ifndef __qSlicerSubjectHierarchyVolumesPlugin_h
#define __qSlicerSubjectHierarchyVolumesPlugin_h

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerVolumesSubjectHierarchyPluginsExport.h"

// Slicer includes
#include "vtkMRMLApplicationLogic.h"

// CTK includes
#include <ctkVTKObject.h>

class qSlicerSubjectHierarchyVolumesPluginPrivate;
class vtkMRMLScalarVolumeNode;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class Q_SLICER_VOLUMES_SUBJECT_HIERARCHY_PLUGINS_EXPORT qSlicerSubjectHierarchyVolumesPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyVolumesPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyVolumesPlugin() override;

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

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, empty icon if nothing to set
  QIcon icon(vtkIdType itemID) override;

  /// Get visibility icon for a visibility state
  QIcon visibilityIcon(int visible) override;

  /// Generate tooltip for a owned subject hierarchy item
  QString tooltip(vtkIdType itemID)const override;

  /// Set display visibility of a owned subject hierarchy item
  void setDisplayVisibility(vtkIdType itemID, int visible) override;

  /// Get display visibility of a owned subject hierarchy item
  /// \return Display visibility (0: hidden, 1: shown, 2: partially shown)
  int getDisplayVisibility(vtkIdType itemID)const override;

  /// Get visibility context menu item actions to add to tree view.
  /// These item visibility context menu actions can be shown in the implementations of \sa showVisibilityContextMenuActionsForItem
  QList<QAction*> visibilityContextMenuActions() const override;

  /// Show visibility context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the visibility context menu items for
  void showVisibilityContextMenuActionsForItem(vtkIdType itemID) override;

public:
  /// Show volume in all slice views. The argument node replaces any volume shown on the specified layer
  /// \param node Volume node to show
  /// \param layer Layer to show volume on. Only one layer can be specified. By default it's the background layer
  void showVolumeInAllViews(vtkMRMLScalarVolumeNode* node, int layer=vtkMRMLApplicationLogic::BackgroundLayer);

  /// Hide given volume from all layers of all slice views
  void hideVolumeFromAllViews(vtkMRMLScalarVolumeNode* node);

  /// Collect subject hierarchy item IDs of all volumes that are shown in any slice view
  /// \param shownVolumeItemIDs Output argument for subject hierarchy item IDs of shown volumes
  /// \param layer Layer(s) from which the shown volumes are collected. By default it's all layers
  void collectShownVolumes( QSet<vtkIdType>& shownVolumeItemIDs,
    int layer=vtkMRMLApplicationLogic::BackgroundLayer | vtkMRMLApplicationLogic::ForegroundLayer | vtkMRMLApplicationLogic::LabelLayer )const;

protected slots:
  /// Show volumes in study. The first two scalar volumes are shown if there are more.
  /// Hides other volumes if there are less in the current study.
  void showVolumesInBranch();

  /// Show volume in the slice views as foreground
  void showVolumeInForeground();

  /// Re-connect slice composite node events so that visibility icons are updated when volumes
  /// are shown/hidden from outside subject hierarchy
  void onLayoutChanged();
  /// Variant of \sa onLayoutChanged so that it can be connected to the event which has the layout as parameter
  void onLayoutChanged(int layout);

  /// Trigger updating all volume visibility icons when composite node changes
  /// Note: Update all and not just the volumes in the composite node, because it is impossible
  ///       to know after a Modified event if a volume was hidden in the process
  void onSliceCompositeNodeModified();

  /// Toggle flag determining whether field of view in slice views is reset when showing a volume
  /// in subject hierarchy. By default it is off. State is stored in the application settings.
  void toggleResetFieldOfViewOnShowAction(bool);

protected:
  QScopedPointer<qSlicerSubjectHierarchyVolumesPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyVolumesPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyVolumesPlugin);
};

#endif
