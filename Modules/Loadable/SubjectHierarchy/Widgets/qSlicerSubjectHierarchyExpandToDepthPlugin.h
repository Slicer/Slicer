/*==============================================================================

  Program: 3D Slicer

  Copyright (c) EBATINCA, S.L., Las Palmas de Gran Canaria, Spain.
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.

==============================================================================*/

#ifndef __qSlicerSubjectHierarchyExpandToDepthPlugin_h
#define __qSlicerSubjectHierarchyExpandToDepthPlugin_h

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qSlicerSubjectHierarchyExpandToDepthPluginPrivate;

class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyExpandToDepthPlugin
  : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyExpandToDepthPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyExpandToDepthPlugin() override;

public:
  /// Get scene context menu item actions to add to tree view
  /// Separate method is needed for the scene, as its actions are set to the
  /// tree by a different method \sa itemContextMenuActions
  QList<QAction*> sceneContextMenuActions() const override;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  void showContextMenuActionsForItem(vtkIdType itemID) override;

public slots:
  /// Expand tree to depth specified by the clicked context menu action
  virtual void expandToDepthFromContextMenu();

protected:
  QScopedPointer<qSlicerSubjectHierarchyExpandToDepthPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyExpandToDepthPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyExpandToDepthPlugin);
};

#endif
