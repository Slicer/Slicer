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

#ifndef __qSlicerSubjectHierarchyCloneNodePlugin_h
#define __qSlicerSubjectHierarchyCloneNodePlugin_h

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qSlicerSubjectHierarchyCloneNodePluginPrivate;

class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyCloneNodePlugin
  : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyCloneNodePlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyCloneNodePlugin() override;

public:
  Q_INVOKABLE static const QString getCloneNodeNamePostfix();

public:
  /// Get item context menu item actions to add to tree view
  QList<QAction*> itemContextMenuActions() const override;

  /// Show context menu actions valid for  given subject hierarchy node.
  /// \param node Subject Hierarchy node to show the context menu items for. If nullptr, then shows menu items for the
  /// scene
  void showContextMenuActionsForItem(vtkIdType itemID) override;

protected slots:
  /// Clone currently selected subject hierarchy item and associated data node
  void cloneCurrentItem();

protected:
  QScopedPointer<qSlicerSubjectHierarchyCloneNodePluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyCloneNodePlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyCloneNodePlugin);
};

#endif
