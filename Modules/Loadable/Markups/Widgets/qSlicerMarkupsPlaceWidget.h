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

#ifndef __qSlicerMarkupsPlaceWidget_h
#define __qSlicerMarkupsPlaceWidget_h

// Qt includes
#include "qSlicerWidget.h"

#include "qMRMLUtils.h"

// Markups Widgets includes
#include "qSlicerMarkupsModuleWidgetsExport.h"
#include "ui_qSlicerMarkupsPlaceWidget.h"


class qSlicerMarkupsPlaceWidgetPrivate;
class vtkMRMLInteractionNode;
class vtkMRMLSelectionNode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLMarkupsNode;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT
qSlicerMarkupsPlaceWidget : public qSlicerWidget
{
  Q_OBJECT
  Q_ENUMS(PlaceMultipleMarkupsType)
  Q_PROPERTY(bool buttonsVisible READ buttonsVisible WRITE setButtonsVisible)
  Q_PROPERTY(bool deleteAllControlPointsOptionVisible READ deleteAllControlPointsOptionVisible WRITE setDeleteAllControlPointsOptionVisible)
  Q_PROPERTY(bool unsetLastControlPointOptionVisible READ unsetLastControlPointOptionVisible WRITE setUnsetLastControlPointOptionVisible)
  Q_PROPERTY(bool unsetAllControlPointsOptionVisible READ unsetAllControlPointsOptionVisible WRITE setUnsetAllControlPointsOptionVisible)
  Q_PROPERTY(PlaceMultipleMarkupsType placeMultipleMarkups READ placeMultipleMarkups WRITE setPlaceMultipleMarkups)
  Q_PROPERTY(QColor nodeColor READ nodeColor WRITE setNodeColor)
  Q_PROPERTY(QColor defaultNodeColor READ defaultNodeColor WRITE setDefaultNodeColor)
  Q_PROPERTY(bool currentNodeActive READ currentNodeActive WRITE setCurrentNodeActive)
  Q_PROPERTY(bool placeModeEnabled READ placeModeEnabled WRITE setPlaceModeEnabled)
  Q_PROPERTY(bool placeModePersistency READ placeModePersistency WRITE setPlaceModePersistency)

  /// \deprecated Use deleteAllControlPointsOptionVisible instead.
  Q_PROPERTY(bool deleteAllMarkupsOptionVisible READ deleteAllMarkupsOptionVisible WRITE setDeleteAllMarkupsOptionVisible)

public:
  typedef qSlicerWidget Superclass;
  qSlicerMarkupsPlaceWidget(QWidget *parent=nullptr);
  ~qSlicerMarkupsPlaceWidget() override;

  enum PlaceMultipleMarkupsType
  {
    ShowPlaceMultipleMarkupsOption, // show a menu on the place button to place multiple markup points
    HidePlaceMultipleMarkupsOption, // don't allow to change persistency of place mode, just use current
    ForcePlaceSingleMarkup, // always disable persistency when enabling place mode
    ForcePlaceMultipleMarkups // always enable persistency when enabling place mode
  };

  /// Get the currently selected markups node.
  Q_INVOKABLE vtkMRMLNode* currentNode() const;
  Q_INVOKABLE vtkMRMLMarkupsFiducialNode* currentMarkupsFiducialNode() const;
  Q_INVOKABLE vtkMRMLMarkupsNode* currentMarkupsNode() const;

  /// Get interaction node.
  /// \sa setInteractionNode()
  Q_INVOKABLE vtkMRMLInteractionNode* interactionNode()const;
  Q_INVOKABLE vtkMRMLSelectionNode* selectionNode()const;

  /// Returns true if the current markups node is the active markups node in the scene.
  bool currentNodeActive() const;

  /// Returns true if place mode enabled and current node is active.
  bool placeModeEnabled() const;

  /// Returns true if placement mode is persistent, regardless if place mode is enabled or not.
  bool placeModePersistency() const;

  /// Returns markup placement multiplicity option.
  PlaceMultipleMarkupsType placeMultipleMarkups() const;

  /// Returns true if all buttons are visible.
  bool buttonsVisible() const;

  /// Returns true if the Delete all option on the Delete button is visible.
  bool deleteAllControlPointsOptionVisible() const;

  /// Returns true if the Unset last control point option on the Delete button is visible.
  bool unsetLastControlPointOptionVisible() const;

  /// Returns true if the Unset all control points option on the Delete button is visible.
  bool unsetAllControlPointsOptionVisible() const;

  /// Get the selected color of the currently selected markups node.
  QColor nodeColor() const;

  /// Get the default node color that is shown when no node is selected.
  QColor defaultNodeColor() const;

  // Returns true if additional points are allowed in the current markup node, false when point number is fixed
  bool currentMarkupPointPlacementValid();

  Q_INVOKABLE QToolButton* placeButton() const;

  // Button to delete control point(s) or unset their position
  Q_INVOKABLE QToolButton* deleteButton() const;

  //-----------------------------------------------------------
  // All public methods below are deprecated
  //
  // These methods are deprecated because they use old terms (markup instead of control point),

  /// \deprecated Use deleteAllControlPointsOptionVisible instead.
  bool deleteAllMarkupsOptionVisible() const
  {
    qWarning("qSlicerMarkupsPlaceWidget::deleteAllMarkupsOptionVisible method is deprecated, please use deleteAllControlPointsOptionVisible instead");
    return this->deleteAllControlPointsOptionVisible();
  };


public slots:

  void setMRMLScene(vtkMRMLScene* scene) override;

  /// Set the currently selected markups node.
  void setCurrentNode(vtkMRMLNode* currentNode);

  /// Set the currently selected markups node to be the active markups node in the Slicer scene. Does not change place mode.
  void setCurrentNodeActive(bool active);

  /// Set interaction node used to update the widget.
  /// \sa interactionNode()
  void setInteractionNode(vtkMRMLInteractionNode* interactionNode);
  void setSelectionNode(vtkMRMLSelectionNode* selectionNode);

  void setDefaultNodeColor(QColor color);

  void setNodeColor(QColor color);

  /// Enable/disable place mode. If place mode is enabled then current markups node is set active, too.
  void setPlaceModeEnabled(bool enable);

  /// Configures multiplicity of markups placement.
  void setPlaceMultipleMarkups(PlaceMultipleMarkupsType option);

  /// Show/hide all buttons. Useful for showing/hiding all buttons except specific ones.
  void setButtonsVisible(bool visible);

  /// Set visibility of Delete all markups option.
  void setDeleteAllControlPointsOptionVisible(bool visible);

  /// Set visibility of Unset last control point option.
  void setUnsetLastControlPointOptionVisible(bool visible);

  /// Set visibility of Unset all control point option
  void setUnsetAllControlPointsOptionVisible(bool visible);

  /// Set place mode to persistent (remains active until deactivated). Does not enable or disable placement mode.
  void setPlaceModePersistency(bool);

  /// Delete or unset position of the last placed markup point.
  /// If the number of points is locked, the last point will be unset, otherwise
  /// it will be deleted
  void modifyLastPoint();

  /// Delete last placed markup point.
  void deleteLastPoint();

  /// Delete all points from the markups node.
  void deleteAllPoints();

  /// Unset the position status of the last placed markup point.
  void unsetLastDefinedPoint();

  /// Unset the position of all points from the markups node.
  void unsetAllPoints();

  //-----------------------------------------------------------
  // All public methods below are deprecated
  //
  // These methods are deprecated because they use old terms (markup instead of control point),

  /// \deprecated Use deleteLastPoint instead.
  void deleteLastMarkup()
  {
  qWarning("qSlicerMarkupsPlaceWidget::deleteLastMarkup method is deprecated, please use deleteLastPoint instead");
  this->deleteLastPoint();
  };
  /// \deprecated Use deleteAllPoints instead.
  void deleteAllMarkups()
  {
  qWarning("qSlicerMarkupsPlaceWidget::deleteAllMarkups method is deprecated, please use deleteAllPoints instead");
  this->deleteAllPoints();
  };
  /// \deprecated Use setDeleteAllControlPointsOptionVisible instead.
  void setDeleteAllMarkupsOptionVisible(bool visible)
  {
  qWarning("qSlicerMarkupsPlaceWidget::setDeleteAllMarkupsOptionVisible method is deprecated, please use setDeleteAllControlPointsOptionVisible instead");
  this->setDeleteAllControlPointsOptionVisible(visible);
  };

protected slots:

  /// Update the GUI to reflect the currently selected markups node.
  void updateWidget();

  /// Update the Delete Button to reflect the currently visible delete button options.
  void updateDeleteButton();

  /// Update the currently selected markups node to have its selected color changed.
  void onColorButtonChanged(QColor);
  /// Toggle the visibility of the markups in the viewers.
  void onVisibilityButtonClicked();
  /// Toggle whether the current markups node is locked.
  void onLockedButtonClicked();
  /// Toggle whether the number of points in the current markups node is locked.
  void onFixedNumberOfControlPointsButtonClicked();

  void onPlacePersistentPoint(bool enable);

signals:

  /// This signal is emitted when place mode for the active markup is changed to enabled or disabled.
  /// The argument \a enabled is true if the currently selected markups node is active and in place mode.
  /// The argument \a enabled is false if the currently selected markups node is not active or not in place mode.
  void activeMarkupsPlaceModeChanged(bool enabled);

  /// \deprecated Use activeMarkupsPlaceModeChanged instead.
  void activeMarkupsFiducialPlaceModeChanged(bool enabled);

protected:
  QScopedPointer<qSlicerMarkupsPlaceWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsPlaceWidget);
  Q_DISABLE_COPY(qSlicerMarkupsPlaceWidget);

};

#endif
