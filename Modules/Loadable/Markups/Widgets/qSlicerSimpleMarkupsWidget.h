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
  Q_PROPERTY(bool enterPlaceModeOnNodeChange READ enterPlaceModeOnNodeChange WRITE setEnterPlaceModeOnNodeChange)
  Q_PROPERTY(bool jumpToSliceEnabled READ jumpToSliceEnabled WRITE setJumpToSliceEnabled)
  Q_PROPERTY(bool nodeSelectorVisible READ nodeSelectorVisible WRITE setNodeSelectorVisible)
  Q_PROPERTY(bool optionsVisible READ optionsVisible WRITE setOptionsVisible)
  Q_PROPERTY(QColor nodeColor READ nodeColor WRITE setNodeColor)
  Q_PROPERTY(QColor defaultNodeColor READ defaultNodeColor WRITE setDefaultNodeColor)
  Q_PROPERTY(int viewGroup READ viewGroup WRITE setViewGroup)

public:
  typedef qSlicerWidget Superclass;
  qSlicerSimpleMarkupsWidget(QWidget *parent=nullptr);
  ~qSlicerSimpleMarkupsWidget() override;

  /// Get the currently selected markups node.
  Q_INVOKABLE vtkMRMLNode* currentNode() const;

  /// \deprecated Use currentNode() instead.
  Q_INVOKABLE vtkMRMLNode* getCurrentNode();

  /// Get interaction node.
  /// \sa setInteractionNode()
  Q_INVOKABLE vtkMRMLInteractionNode* interactionNode()const;

  /// Get the markups table widget
  Q_INVOKABLE QTableWidget* tableWidget() const;

  /// Get the markups placement widget (containing place and delete button)
  Q_INVOKABLE qSlicerMarkupsPlaceWidget* markupsPlaceWidget() const;

  /// Get the markup node selector combo box
  Q_INVOKABLE qMRMLNodeComboBox* markupsSelectorComboBox() const;

  /// Accessors to control place mode behavior
  bool enterPlaceModeOnNodeChange() const;

  /// If enabled then the control point will be shown in all slice views when a caontrol point is selected
  /// /sa setViewGroup
  bool jumpToSliceEnabled() const;

  /// Show/hide the markup node selector widget.
  bool nodeSelectorVisible() const;

  /// Show/hide options (place, activate, color, etc buttons).
  bool optionsVisible() const;

  /// Get the selected color of the currently selected markups node.
  QColor nodeColor() const;

  /// Get the default node color that is applied to newly created nodes.
  QColor defaultNodeColor() const;

  /// Set views where slice positions will be updated on jump to slice.
  /// If it is set to -1 (by default it is) then all slices will be jumped.
  void setViewGroup(int newViewGroup);

  /// Get view group where slice positions will be updated.
  int viewGroup()const;

public slots:

  void setMRMLScene(vtkMRMLScene* scene) override;

  /// Set the currently selected markups node.
  void setCurrentNode(vtkMRMLNode* currentNode);

  /// Set the default name of the markups node created in the combo box.
  void setNodeBaseName(QString newNodeBaseName);

  /// Set interaction node used to update the widget.
  /// \sa interactionNode()
  void setInteractionNode(vtkMRMLInteractionNode* interactionNode);

  /// Accessors to control place mode behavior
  void setEnterPlaceModeOnNodeChange(bool);

  /// If enabled then the control point will be shown in all slice views when a control point is selected
  void setJumpToSliceEnabled(bool);

  /// Show/hide the markup node selector widget.
  void setNodeSelectorVisible(bool);

  /// Show/hide options (place, activate, color, etc buttons).
  void setOptionsVisible(bool);

  /// Set the selected color of the currently selected markups node.
  void setNodeColor(QColor color);

  /// Set the default color that is assigned to newly created markups nodes in the combo box.
  void setDefaultNodeColor(QColor color);

  /// Scrolls to and selects the Nth control point in the table of control points.
  void highlightNthControlPoint(int n);

  /// \deprecated Use highlightNthControlPoint instead
  void highlightNthFiducial(int n);

  /// Set the currently selected markups node to be the active markups node in the Slicer scene.
  void activate();

  /// Set the currently selected markups node to be the active markups node in the Slicer scene. The argument \a place is true then also interaction mode is set to place mode.
  void placeActive(bool place);

protected slots:

  /// Update the widget when a different markups node is selected by the combo box.
  void onMarkupsNodeChanged();
  /// Setup a newly created markups node - add display node, set color.
  void onMarkupsNodeAdded(vtkMRMLNode*);
  /// Create context menu for the table displaying the currently selected markups node.
  void onMarkupsControlPointsTableContextMenu(const QPoint& position);

  /// Edit the name or position of the currently selected markups node.
  void onMarkupsControlPointEdited(int row, int column);

  /// Clicked on a control point or used keyboard to move between control points in the table.
  void onMarkupsControlPointSelected(int row, int column);

  /// Update the GUI to reflect the currently selected markups node.
  void updateWidget();

  /// Update the GUI to reflect the currently selected markups node.
  void onPointAdded();

signals:

  /// The signal is emitted when a different markup node is selected.
  void markupsNodeChanged();

  /// \deprecated Use markupsNodeChanged instead.
  void markupsFiducialNodeChanged();

  /// This signal is emitted when a different markup point is selected in the table.
  /// The argument \a markupIndex is the index of the selected markup.
  void currentMarkupsControlPointSelectionChanged(int markupIndex);

  /// \deprecated Use currentMarkupsControlPointSelectionChanged instead
  void currentMarkupsFiducialSelectionChanged(int markupIndex);

  /// This signal is emitted when place mode for the active markup is changed to enabled or disabled.
  /// The argument \a enabled is true if the currently selected markups node is active and in place mode.
  /// The argument \a enabled is false if the currently selected markups node is not active or not in place mode.
  void activeMarkupsPlaceModeChanged(bool enabled);

  /// \deprecated Use activeMarkupsPlaceModeChanged instead
  void activeMarkupsFiducialPlaceModeChanged(bool enabled);

  /// This signal is emitted if updates to the widget have finished.
  /// It is called after control points are changed (added, position modified, etc).
  void updateFinished();

protected:
  QScopedPointer<qSlicerSimpleMarkupsWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerSimpleMarkupsWidget);
  Q_DISABLE_COPY(qSlicerSimpleMarkupsWidget);

};

#endif
