/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerMarkupsModuleWidget_h
#define __qSlicerMarkupsModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// Markups widgets include
#include "qSlicerMarkupsModuleExport.h"

class QItemSelection;
class QMenu;
class QModelIndex;
class QStringList;
class QTableWidgetItem;
class QShortcut;
class qMRMLMarkupsOptionsWidget;
class qSlicerMarkupsModuleWidgetPrivate;
class vtkMRMLMarkupsNode;
class vtkMRMLNode;
class vtkSlicerMarkupsLogic;

/// \ingroup Slicer_QtModules_Markups
class Q_SLICER_QTMODULES_MARKUPS_EXPORT qSlicerMarkupsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMarkupsModuleWidget(QWidget *parent=nullptr);
  ~qSlicerMarkupsModuleWidget() override;

  /// Set up the GUI from mrml when entering
  /// \sa updateMaximumScaleFromVolumes()
  void enter() override;
  /// Disconnect from scene when exiting
  void exit() override;

  /// Get the logic in the proper class
  vtkSlicerMarkupsLogic *markupsLogic();

  /// Refresh the gui from the currently active markup node as determined by
  /// the selection node
  /// \sa updateWidgetFromDisplayNode()
  void updateWidgetFromMRML();
  /// Update the GUI elements related to the display properties from MRML, by
  /// getting the display node associated with the active markups node.
  /// \sa  updateWidgetFromMRML()
  void updateWidgetFromDisplayNode();
  /// Check Red volume slice spacing to make sure that the glyph and text scale
  /// slider maximums allow the glyphs to be scaled so that they can be shown
  /// on volumes with spacing larger than the default. Find the maximum background
  /// volume spacing and multiply it by the volume spacing scale factor
  /// \sa volumeSpacingScaleFactor
  void updateMaximumScaleFromVolumes();
  /// Refresh a row of the control points table from the m-th markup.
  void updateRow(int m);
  /// Refresh all the rows of the control points table from the markup node.
  void updateRows();

  /// Add observations to the markups node, and remove them from other markups
  /// nodes (from all nodes if markupsNode is null)
  void setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode);

  /// Get currently selected markups node
  vtkMRMLMarkupsNode* mrmlMarkupsNode();

  /// Set up the logic default display settings from the application settings
  void updateLogicFromSettings();

  /// Query the logic as to the state of the slice intersection visibility on
  /// the slice composite nodes
  bool sliceIntersectionsVisible();

  // Set number of columns for organization of create markups buttons
  void setCreateMarkupsButtonsColumns(unsigned int columns);

  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;
  double nodeEditable(vtkMRMLNode* node) override;

  // Returns the list of associated markups
  QStringList& associatedMarkups();

  // Add additional widget
  QList<qMRMLMarkupsOptionsWidget*>& additionalWidgets();

  Q_INVOKABLE void updateToolBar(vtkMRMLMarkupsNode* node);

public slots:

  /// Respond to the scene events
  /// when a markups node is added, make it the active one in the combo box
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  /// When a node is removed and it is the last one in the scene, clear out
  /// the gui - the node combo box will signal that a remaining node has been
  /// selected and the GUI will update separately in that case
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  /// Update the table after a scene is imported
  void onMRMLSceneEndImportEvent();
  /// Update the table after a scene view is restored
  void onMRMLSceneEndRestoreEvent();
  /// Update the table after batch processing is done, needed to trigger
  /// an update after a markups file is read in
  void onMRMLSceneEndBatchProcessEvent();
  /// Clear out the gui when the scene is closed
  void onMRMLSceneEndCloseEvent();
  /// Respond to the p key being pressed
  void onPKeyActivated();

  /// Display property button slots
  void onResetToDefaultDisplayPropertiesPushButtonClicked();
  void onSaveToDefaultDisplayPropertiesPushButtonClicked();

  /// List button slots
  void onVisibilityOnAllControlPointsInListPushButtonClicked();
  void onVisibilityOffAllControlPointsInListPushButtonClicked();
  void onVisibilityAllControlPointsInListToggled();
  void onLockAllControlPointsInListPushButtonClicked();
  void onUnlockAllControlPointsInListPushButtonClicked();
  void onLockAllControlPointsInListToggled();
  void onSelectAllControlPointsInListPushButtonClicked();
  void onDeselectAllControlPointsInListPushButtonClicked();
  void onSelectedAllControlPointsInListToggled();
  void onAddControlPointPushButtonClicked();
  void onMoveControlPointUpPushButtonClicked();
  void onMoveControlPointDownPushButtonClicked();
  void onRestoreControlPointPushButtonClicked();
  void onResetControlPointPushButtonClicked();
  void onUnsetControlPointPushButtonClicked();
  void onMissingControlPointPushButtonClicked();
  void onDeleteControlPointPushButtonClicked(bool confirm=true);
  void onDeleteAllControlPointsInListPushButtonClicked();

  /// Update the selection node from the tree view
  void onActiveMarkupItemChanged(vtkIdType);
  void onActiveMarkupMRMLNodeChanged(vtkMRMLNode *markupsNode);
  /// Update the combo box from the selection node
  void onSelectionNodeActivePlaceNodeIDChanged();

  /// When the user clicks in the combo box to create a new markups node,
  /// make sure that a display node is added
  void onActiveMarkupMRMLNodeAdded(vtkMRMLNode *markupsNode);

  /// Create markups node by class.
  void onCreateMarkupByClass(const QString& className);

  /// Toggle the markups node visibility flag
  void onListVisibileInvisiblePushButtonClicked();

  /// Toggle the markups node locked flag
  void onListLockedUnlockedPushButtonClicked();

  /// Toggle the markups node control point number locked flag
  void onFixedNumberOfControlPointsPushButtonClicked();

  /// Update the markup label from the line edit entry
  void onNameFormatLineEditTextEdited(const QString text);

  /// Reset the name format string and use list name to default
  void onResetNameFormatToDefaultPushButtonClicked();

  /// Iterate over the markups in the currently active list and rename them
  /// using the current name format. Attempts to preserve any numbers already
  /// present.
  void onRenameAllWithCurrentNameFormatPushButtonClicked();

  /// Update the mrml node from the table
  void onActiveMarkupTableCellChanged(int row, int column);
  /// React to clicks in the table
  void onActiveMarkupTableCellClicked(QTableWidgetItem* item);
  /// React to arrows setting the current cell
  void onActiveMarkupTableCurrentCellChanged(int currentRow, int currentColumn,
                                             int previousRow, int previousColumn);
  /// Provide a right click menu in the table
  void onRightClickActiveMarkupTableWidget(QPoint pos);
  /// Add the coordinates of the currently selected markups as strings to the given menu, then add a separator
  void addSelectedCoordinatesToMenu(QMenu *menu);
  /// Jump slices action slot
  void onJumpSlicesActionTriggered();
  /// Refocus cameras action slot
  void onRefocusCamerasActionTriggered();

  /// Build a string list of the names of other nodes with the same
  /// class name as thisMarkup. Return an empty string list if no other
  /// markups in the scene
  QStringList getOtherMarkupNames(vtkMRMLNode *thisMarkup);

  void cutSelectedToClipboard();
  void copySelectedToClipboard();
  void pasteSelectedFromClipboard();

  /// Update table when markups node is modified
  void onActiveMarkupsNodeModifiedEvent();
  /// Update the table with the modified point information if the node is active
  void onActiveMarkupsNodePointModifiedEvent(vtkObject *caller, void *callData);
  /// Update the table with the new point information if the node is active
  void onActiveMarkupsNodePointAddedEvent();
  /// Update the table for the removed point if the node is active
  void onActiveMarkupsNodePointRemovedEvent(vtkObject *caller, void *callData);
  /// Update the display properties widgets when the display node is modified
  void onActiveMarkupsNodeDisplayModifiedEvent();
  /// Update the transform related elements of the gui when the transform node is modified
  void onActiveMarkupsNodeTransformModifiedEvent();

  /// Create a new markups node and copy the display node settings from the
  /// current markups node if set, otherwise just uses the defaults.
  void onNewMarkupWithCurrentDisplayPropertiesTriggered();

  /// Update the slice intersection visibility on all the slice composite
  /// nodes in the scene
  /// \sa sliceIntersectionsVisible()
  void onSliceIntersectionsVisibilityToggled(bool checked);

  /// update visibility of the coordinate columns in the table
  void onHideCoordinateColumnsToggled(int index);

  /// update the coordinates shown in the table to be either the transformed coordinates (checked) or the untransformed coordinates (unchecked)
  void onTransformedCoordinatesToggled(bool checked);

  /// Make sure all measurements in the current markups node are observed
  void observeMeasurementsInCurrentMarkupsNode();
  /// Update measurements description label
  void updateMeasurementsDescriptionLabel();
  /// Populate measurement settings frame from the available measurements in the current markup
  void populateMeasurementSettingsTable();
  /// Update observations on measurements when their container is modified (i.e. measurement added or removed)
  void onMeasurementsCollectionModified();
  /// Update measurement information on GUI when a measurement is modified
  void onMeasurementModified(vtkObject* caller);
  /// Enable/disable measurement when an enable checkbox has been toggled
  void onMeasurementEnabledCheckboxToggled(bool on);

  /// Update set of create markups push buttons
  void onCreateMarkupsPushButtons();

  /// Update internal list of markups options widgets from qMRMLMarkupsOptionsWidgetsFactory
  void onUpdateMarkupsOptionsWidgets();

  // Enable or disable markup table buttons
  void enableMarkupTableButtons(bool enable);

  void updateImportExportWidgets();
  void onImportExportApply();

  /// Create or get first color legend if group box is expanded
  void onColorLegendCollapsibleGroupBoxToggled(bool);

  //-----------------------------------------------------------
  // All public methods below are deprecated
  //
  // These methods are deprecated because they use old terms (markup instead of control point),

  /// \deprecated Use onVisibilityOnAllControlPointsInListPushButtonClicked instead.
  void onVisibilityOnAllMarkupsInListPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onVisibilityOnAllMarkupsInListPushButtonClicked method is deprecated, please use onVisibilityOnAllControlPointsInListPushButtonClicked instead");
    this->onVisibilityOnAllControlPointsInListPushButtonClicked();
    };
  /// \deprecated Use onVisibilityOffAllControlPointsInListPushButtonClicked instead.
  void onVisibilityOffAllMarkupsInListPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onVisibilityOffAllMarkupsInListPushButtonClicked method is deprecated, please use onVisibilityOffAllControlPointsInListPushButtonClicked instead");
    this->onVisibilityOffAllControlPointsInListPushButtonClicked();
    };
  /// \deprecated Use onVisibilityAllControlPointsInListToggled instead.
  void onVisibilityAllMarkupsInListToggled()
    {
    qWarning("qSlicerMarkupsModuleWidget::onVisibilityAllMarkupsInListToggled method is deprecated, please use onVisibilityAllControlPointsInListToggled instead");
    this->onVisibilityAllControlPointsInListToggled();
    };
  /// \deprecated Use onLockAllControlPointsInListPushButtonClicked instead.
  void onLockAllMarkupsInListPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onLockAllMarkupsInListPushButtonClicked method is deprecated, please use onLockAllControlPointsInListPushButtonClicked instead");
    this->onLockAllControlPointsInListPushButtonClicked();
    };
  /// \deprecated Use onUnlockAllControlPointsInListPushButtonClicked instead.
  void onUnlockAllMarkupsInListPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onUnlockAllMarkupsInListPushButtonClicked method is deprecated, please use onUnlockAllControlPointsInListPushButtonClicked instead");
    this->onUnlockAllControlPointsInListPushButtonClicked();
    };
  /// \deprecated Use onLockAllControlPointsInListToggled instead.
  void onLockAllMarkupsInListToggled()
    {
    qWarning("qSlicerMarkupsModuleWidget::onLockAllMarkupsInListToggled method is deprecated, please use onLockAllControlPointsInListToggled instead");
    this->onLockAllControlPointsInListToggled();
    };
  /// \deprecated Use onSelectAllControlPointsInListPushButtonClicked instead.
  void onSelectAllMarkupsInListPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onSelectAllMarkupsInListPushButtonClicked method is deprecated, please use onSelectAllControlPointsInListPushButtonClicked instead");
    this->onSelectAllControlPointsInListPushButtonClicked();
    };
  /// \deprecated Use onDeselectAllControlPointsInListPushButtonClicked instead.
  void onDeselectAllMarkupsInListPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onDeselectAllMarkupsInListPushButtonClicked method is deprecated, please use onDeselectAllControlPointsInListPushButtonClicked instead");
    this->onDeselectAllControlPointsInListPushButtonClicked();
    };
  /// \deprecated Use onSelectedAllControlPointsInListToggled instead.
  void onSelectedAllMarkupsInListToggled()
    {
    qWarning("qSlicerMarkupsModuleWidget::onSelectedAllMarkupsInListToggled method is deprecated, please use onSelectedAllControlPointsInListToggled instead");
    this->onSelectedAllControlPointsInListToggled();
    };
  /// \deprecated Use onAddControlPointPushButtonClicked instead.
  void onAddMarkupPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onAddMarkupPushButtonClicked method is deprecated, please use onAddControlPointPushButtonClicked instead");
    this->onAddControlPointPushButtonClicked();
    };
  /// \deprecated Use onMoveControlPointUpPushButtonClicked instead.
  void onMoveMarkupUpPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onMoveMarkupUpPushButtonClicked method is deprecated, please use onMoveControlPointUpPushButtonClicked instead");
    this->onMoveControlPointUpPushButtonClicked();
    };
  /// \deprecated Use onMoveControlPointDownPushButtonClicked instead.
  void onMoveMarkupDownPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onMoveMarkupDownPushButtonClicked method is deprecated, please use onMoveControlPointDownPushButtonClicked instead");
    this->onMoveControlPointDownPushButtonClicked();
    };
  /// \deprecated Use onRestoreControlPointPushButtonClicked instead.
  void onRestoreMarkupPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onRestoreMarkupPushButtonClicked method is deprecated, please use onRestoreControlPointPushButtonClicked instead");
    this->onRestoreControlPointPushButtonClicked();
    };
  /// \deprecated Use onResetControlPointPushButtonClicked instead.
  void onResetMarkupPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onResetMarkupPushButtonClicked method is deprecated, please use onResetControlPointPushButtonClicked instead");
    this->onResetControlPointPushButtonClicked();
    };
  /// \deprecated Use onUnsetControlPointPushButtonClicked instead.
  void onUnsetMarkupPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onUnsetMarkupPushButtonClicked method is deprecated, please use onUnsetControlPointPushButtonClicked instead");
    this->onUnsetControlPointPushButtonClicked();
    };
  /// \deprecated Use onMissingControlPointPushButtonClicked instead.
  void onMissingMarkupPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onMissingMarkupPushButtonClicked method is deprecated, please use onMissingControlPointPushButtonClicked instead");
    this->onMissingControlPointPushButtonClicked();
    };
  /// \deprecated Use onDeleteControlPointPushButtonClicked instead.
  void onDeleteMarkupPushButtonClicked(bool confirm=true)
    {
    qWarning("qSlicerMarkupsModuleWidget::onDeleteMarkupPushButtonClicked method is deprecated, please use onDeleteControlPointPushButtonClicked instead");
    this->onDeleteControlPointPushButtonClicked(confirm);
    };
  /// \deprecated Use onDeleteAllControlPointsInListPushButtonClicked instead.
  void onDeleteAllMarkupsInListPushButtonClicked()
    {
    qWarning("qSlicerMarkupsModuleWidget::onDeleteAllMarkupsInListPushButtonClicked method is deprecated, please use onDeleteAllControlPointsInListPushButtonClicked instead");
    this->onDeleteAllControlPointsInListPushButtonClicked();
    };

protected:
  QScopedPointer<qSlicerMarkupsModuleWidgetPrivate> d_ptr;

  void setup() override;

  /// A multiplication factor to apply to the maximum volume slice spacing when determining what the maximum value for the scale sliders should be.
  /// \sa updateMaximumScaleFromVolumes
  /// Default: 10.0
  double volumeSpacingScaleFactor;

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsModuleWidget);
  Q_DISABLE_COPY(qSlicerMarkupsModuleWidget);
};

#endif
