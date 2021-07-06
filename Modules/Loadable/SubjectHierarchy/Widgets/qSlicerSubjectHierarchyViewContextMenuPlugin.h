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

#ifndef __qSlicerSubjectHierarchyViewContextMenuPlugin_h
#define __qSlicerSubjectHierarchyViewContextMenuPlugin_h

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

// CTK includes
#include <ctkVTKObject.h>

class qSlicerSubjectHierarchyViewContextMenuPluginPrivate;

class vtkMRMLDisplayNode;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
/// \brief Subject hierarchy view menu plugin
///
/// Adds menu items to the view context menu (when the user right-clicks in a slice or 3D view
/// in an empty area).
///
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyViewContextMenuPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyViewContextMenuPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyViewContextMenuPlugin() override;

public:

  /// Get view context menu item actions that are available when right-clicking an object in the views.
  /// These item context menu actions can be shown in the implementations of \sa showViewContextMenuActionsForItem
  QList<QAction*> viewContextMenuActions()const override;

  /// Show context menu actions valid for a given subject hierarchy item to be shown in the view.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  /// \param eventData Supplementary data for the item that may be considered for the menu (sub-item ID, attribute, etc.)
  void showViewContextMenuActionsForItem(vtkIdType itemID, QVariantMap eventData) override;

protected slots:
  void setInteractionMode(int mode);
  void saveScreenshot();
  void configureSliceViewAnnotationsAction();
  void maximizeView();
  void fitSliceView();
  void centerThreeDView();
  void toggleTiltLock();
  void setSliceIntersectionVisible(bool);
  void setSliceIntersectionHandlesVisible(bool);

protected:
  QScopedPointer<qSlicerSubjectHierarchyViewContextMenuPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyViewContextMenuPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyViewContextMenuPlugin);
};

#endif
