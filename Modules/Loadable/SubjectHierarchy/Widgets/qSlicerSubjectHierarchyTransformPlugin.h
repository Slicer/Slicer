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

==============================================================================*/

#ifndef __qSlicerSubjectHierarchyTransformPlugin_h
#define __qSlicerSubjectHierarchyTransformPlugin_h

// SubjectHierarchy Plugins includes
#include "qMRMLSubjectHierarchyTreeView.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qSlicerSubjectHierarchyTransformPluginPrivate;
class qMRMLSubjectHierarchyTreeView;

class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyTransformPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyTransformPlugin(QObject *parent = nullptr);
  ~qSlicerSubjectHierarchyTransformPlugin() override;

public:
  /// Get transform context menu item actions to add to tree view.
  /// These item transform context menu actions can be shown in the
  /// implementations of \sa showTransformContextMenuActionsForItem
  QList<QAction *> transformContextMenuActions() const override;

  /// Show context menu actions valid for given subject hierarchy item
  void showTransformContextMenuActionsForItem(vtkIdType itemID) override;

  /// Menu section where transform nodes are listed.
  /// It can be useful for positioning menu items above or below in other plugins
  /// using qSlicerSubjectHierarchyAbstractPlugin::setActionPosition.
  int transformListSection() const;

protected slots:

  virtual void onTransformActionSelected();
  virtual void onTransformInteractionInViewToggled(bool show);
  virtual void onTransformEditProperties();
  virtual void onHardenTransformOnBranchOfCurrentItem();
  virtual void onRemoveTransformsFromBranchOfCurrentItem();
  virtual void onCreateNewTransform();

protected:
  QScopedPointer<qSlicerSubjectHierarchyTransformPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyTransformPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyTransformPlugin);
};

#endif
