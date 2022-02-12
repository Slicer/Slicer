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

#ifndef __qSlicerSubjectHierarchyExportPlugin_h
#define __qSlicerSubjectHierarchyExportPlugin_h

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qSlicerSubjectHierarchyExportPluginPrivate;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyExportPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyExportPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyExportPlugin() override;


public:
  /// Get node item context menu actions to add to tree view
  QList<QAction*> itemContextMenuActions() const override;

  /// Get scene item context menu actions to add to tree view
  QList<QAction*> sceneContextMenuActions() const override;

  /// Show context menu actions valid for given subject hierarchy item
  void showContextMenuActionsForItem(vtkIdType itemID) override;

protected slots:
  /// Export currently selected subject hierarchy item and/or its children
  void exportItems();

protected:
  QScopedPointer<qSlicerSubjectHierarchyExportPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyExportPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyExportPlugin);
};

#endif
