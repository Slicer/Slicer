/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerSimpleMarkupsWidget_h
#define __qSlicerSimpleMarkupsWidget_h

// Qt includes
#include "qSlicerWidget.h"

#include "qMRMLUtils.h"

// Markups Widgets includes
#include "qSlicerMarkupsModuleWidgetsExport.h"
#include "ui_qSlicerSimpleMarkupsWidget.h"


class qSlicerSimpleMarkupsWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT 
qSlicerSimpleMarkupsWidget : public qSlicerWidget
{
  Q_OBJECT

public:
  typedef qSlicerWidget Superclass;
  qSlicerSimpleMarkupsWidget(QWidget *parent=0);
  virtual ~qSlicerSimpleMarkupsWidget();

  /// Get the currently selected markups node.
  Q_INVOKABLE vtkMRMLNode* currentNode() const;

  /// Deprecated. Use currentNode() instead.
  Q_INVOKABLE vtkMRMLNode* getCurrentNode();
  
public slots:
  /// Set the currently selected markups node.
  void setCurrentNode(vtkMRMLNode* currentNode);
  /// Set the default name of the markups node created in the combo box.
  void setNodeBaseName(QString newNodeBaseName);

  /// Get the selected color of the currently selected markups node.
  void getNodeColor(QColor color);
  /// Set the selected color of the currently selected markups node.
  void setNodeColor(QColor color);
  /// Set the default color that is assigned to newly created markups nodes in the combo box.
  void setDefaultNodeColor(QColor color);

  /// Highlight the Nth fiducial in the table of fiducials.
  void highlightNthFiducial(int n);
  /// Set the currently selected markups node to be the active markups node in the Slicer scene.
  void activate();

protected slots:
  /// Update the currently selected markups node to have its selected color changed.
  void onColorButtonChanged(QColor);
  /// Toggle the visibility of the markups in the viewers.
  void onVisibilityButtonClicked();
  /// Toggle whether the current markups node is locked.
  void onLockButtonClicked();
  /// Delete a fiducial from the list.
  void onDeleteButtonClicked();
  /// Set the current node to be the active node in the scene and start place mode.
  void onPlaceButtonClicked();
  /// Make the currently selected markups node the active markups node in the scene.
  void onActiveButtonClicked();

  /// Update the widget when a different markups node is selected by the combo box.
  void onMarkupsFiducialNodeChanged();
  /// Setup a newly created markups node - add display node, set color.
  void onMarkupsFiducialNodeAdded(vtkMRMLNode*);
  /// Create context menu for the table displaying the currently selected markups node.
  void onMarkupsFiducialTableContextMenu(const QPoint& position);

  /// Edit the name or position of the currently selected markups node.
  void onMarkupsFiducialEdited(int row, int column);

  /// Update the GUI to reflect the currently selected markups node.
  void updateWidget();

signals:
  /// The currently selected markups node has changed.
  void markupsFiducialNodeChanged();
  /// The currently selected markups node has been activated.
  void markupsFiducialActivated();
  /// The place mode has changed. This is independent of whether the currently selected markups node is active.
  void markupsFiducialPlaceModeChanged();
  /// The updates to the GUI have are finished.
  void updateFinished();

protected:
  QScopedPointer<qSlicerSimpleMarkupsWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();

  /// Monitor for changes in the interaction and selection nodes.
  void connectInteractionAndSelectionNodes();

private:
  Q_DECLARE_PRIVATE(qSlicerSimpleMarkupsWidget);
  Q_DISABLE_COPY(qSlicerSimpleMarkupsWidget);

};

#endif
