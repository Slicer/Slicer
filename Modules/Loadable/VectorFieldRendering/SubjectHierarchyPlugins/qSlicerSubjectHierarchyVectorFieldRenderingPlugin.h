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

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __qSlicerSubjectHierarchyVectorFieldRenderingPlugin_h
#define __qSlicerSubjectHierarchyVectorFieldRenderingPlugin_h

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerVectorFieldRenderingSubjectHierarchyPluginsExport.h"

class qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
/// \brief Adds "Show as vector field" to the visibility menu of models, vector volumes and transforms.
///
/// Like volume rendering, a vector field that is shown this way is shown and hidden with its node by the eye icon;
/// once the option is turned off, the eye icon does not show it again. The plugin does not own any items.
class Q_SLICER_VECTORFIELDRENDERING_SUBJECT_HIERARCHY_PLUGINS_EXPORT qSlicerSubjectHierarchyVectorFieldRenderingPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyVectorFieldRenderingPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyVectorFieldRenderingPlugin() override;

public:
  /// Visibility context menu item actions, shown for items whose node has a vector field
  QList<QAction*> visibilityContextMenuActions() const override;

  /// Show the visibility context menu actions for an item
  void showVisibilityContextMenuActionsForItem(vtkIdType itemID) override;

  /// Show or hide the vector fields of the node of an item. Showing adds a vector field display node if the node has
  /// none. Returns false if the node has no vector field.
  Q_INVOKABLE bool showVectorField(bool show, vtkIdType itemID);

  /// True if a vector field of the node of an item is shown with the node (see showVectorField())
  Q_INVOKABLE bool isVectorFieldShown(vtkIdType itemID);

protected slots:
  void toggleVectorFieldForCurrentItem(bool show);
  void showVectorFieldOptionsForCurrentItem();

protected:
  QScopedPointer<qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyVectorFieldRenderingPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyVectorFieldRenderingPlugin);
};

#endif
