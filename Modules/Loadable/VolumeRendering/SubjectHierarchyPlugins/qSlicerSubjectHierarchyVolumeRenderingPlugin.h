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

#ifndef __qSlicerSubjectHierarchyVolumeRenderingPlugin_h
#define __qSlicerSubjectHierarchyVolumeRenderingPlugin_h

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerVolumeRenderingSubjectHierarchyPluginsExport.h"

class qSlicerSubjectHierarchyVolumeRenderingPluginPrivate;
class vtkSlicerVolumeRenderingLogic;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class Q_SLICER_VOLUMERENDERING_SUBJECT_HIERARCHY_PLUGINS_EXPORT qSlicerSubjectHierarchyVolumeRenderingPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyVolumeRenderingPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyVolumeRenderingPlugin() override;

public:
  /// Set volume rendering module logic. Required for accessing display nodes and setting up volume rendering related nodes.
  void setVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* volumeRenderingLogic);

  /// Get visibility context menu item actions to add to tree view.
  /// These item visibility context menu actions can be shown in the implementations of \sa showVisibilityContextMenuActionsForItem
  QList<QAction*> visibilityContextMenuActions()const override;

  /// Show visibility context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the visibility context menu items for
  void showVisibilityContextMenuActionsForItem(vtkIdType itemID) override;

protected slots:
  /// Toggle volume rendering option for current volume item
  void toggleVolumeRenderingForCurrentItem(bool);
  /// Switch to Volume Rendering module and select current volume item
  void showVolumeRenderingOptionsForCurrentItem();

protected:
  QScopedPointer<qSlicerSubjectHierarchyVolumeRenderingPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyVolumeRenderingPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyVolumeRenderingPlugin);
};

#endif
