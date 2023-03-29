/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported by the Center for Intelligent Image-guided Interventions (CI3).

==============================================================================*/

#ifndef __qSlicerMarkupsMeasurementsWidget_h
#define __qSlicerMarkupsMeasurementsWidget_h

// Qt includes
#include "qSlicerWidget.h"

#include "qMRMLUtils.h"

// Markups Widgets includes
#include "qSlicerMarkupsModuleWidgetsExport.h"
#include "ui_qSlicerMarkupsMeasurementsWidget.h"

class qSlicerMarkupsMeasurementsWidgetPrivate;
class vtkSlicerMarkupsLogic;
class ctkCollapsibleGroupBox;
class ctkExpandableWidget;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT
qSlicerMarkupsMeasurementsWidget : public qSlicerWidget
{
  Q_OBJECT
  Q_PROPERTY(bool jumpToSliceEnabled READ jumpToSliceEnabled WRITE setJumpToSliceEnabled)
  Q_PROPERTY(int viewGroup READ viewGroup WRITE setViewGroup)
  Q_PROPERTY(int createMarkupsButtonsColumns READ createMarkupsButtonsColumns WRITE setCreateMarkupsButtonsColumns)

public:
  typedef qSlicerWidget Superclass;
  qSlicerMarkupsMeasurementsWidget(QWidget *parent=nullptr);
  ~qSlicerMarkupsMeasurementsWidget() override;

  /// Get the logic in the proper class
  Q_INVOKABLE vtkSlicerMarkupsLogic *markupsLogic();

  /// Get the group box of the create Markups
  Q_INVOKABLE ctkCollapsibleGroupBox *createMarkupsGroupBox();

  /// Get the frame widget of the measurements
  Q_INVOKABLE ctkCollapsibleGroupBox *measurementsGroupBox();

  /// Get the group box of the measurement properties options
  Q_INVOKABLE ctkCollapsibleGroupBox *measurementDisplayGroupBox();

  /// If enabled, slices defined by the view group will be moved to the center of the measurement when it gets selected
  /// /sa setViewGroup
  bool jumpToSliceEnabled() const;

  /// Get view group where slice positions will be updated.
  int viewGroup() const;

  /// Get number of columns for organization of create markups buttons.
  int createMarkupsButtonsColumns() const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene) override;
  void setMRMLScene(vtkMRMLScene* scene, const QStringList& allowedMarkups);

  /// Set the currently selected markups node.
  void setCurrentNode(vtkMRMLNode* currentNode);

  /// If enabled then the control point will be shown in all slice views when a control point is selected
  void setJumpToSliceEnabled(bool);

  /// Update set of create markups push buttons
  void onCreateMarkupsPushButtons();

  /// Set views where slice positions will be updated on jump to slice.
  /// If it is set to -1 (by default it is) then all slices will be jumped.
  void setViewGroup(int newViewGroup);

  /// Set number of columns for organization of create markups buttons.
  void setCreateMarkupsButtonsColumns(unsigned int columns);

  /// Update widget when adding a markups node.
  void onNodeAddedEvent(vtkObject*, vtkObject*);

  /// Update widget when removing a markups node.
  void onNodeRemovedEvent(vtkObject*, vtkObject*);

  /// Update the selection node from the tree view
  void onActiveMarkupItemChanged(vtkIdType);
  void onActiveMarkupMRMLNodeChanged(vtkMRMLNode *markupsNode);

  /// Update the combo box from the selection node
  void onSelectionNodeActivePlaceNodeIDChanged();

  /// Check if markups are measurements
  void onEndPlacementEvent();

  /// Update the table after a scene is imported
  void onMRMLSceneEndImportEvent();
  /// Update the table after a scene view is restored
  void onMRMLSceneEndRestoreEvent();
  /// Update the table after batch processing is done, needed to trigger
  /// an update after a markups file is read in
  void onMRMLSceneEndBatchProcessEvent();
  /// Clear out the gui when the scene is closed
  void onMRMLSceneEndCloseEvent();

  /// Update measurements observations when collection is modified
  void onMeasurementsCollectionModified();
  /// Make sure all measurements in the current markups node are observed
  void observeMeasurementsInCurrentMarkupsNode();
  /// Update measurement information on GUI when a measurement is modified
  void onMeasurementModified(vtkObject* caller);
  /// Enable/disable measurement when an enable checkbox has been toggled
  void onMeasurementEnabledCheckboxToggled(bool on);
  /// Update measurements description label
  void updateMeasurementsDescriptionLabel();
  /// Populate measurement settings frame from the available measurements in the current markup
  void populateMeasurementSettingsTable();

  /// Provide a right click menu in the list
  void onRightClickActiveMeasurement(QPoint pos);
  /// Clear selected Measurement
  void onDeleteMeasurement();
  /// Rename Markups Node
  void onRenameMeasurement();
  /// Clone Markups Node
  void onCloneMeasurement();

  /// Go To Markups Module
  bool onGoToMarkupsModule();


protected slots:
  /// Create markups node by class.
  void onCreateMarkupByClass(const QString& className);

  /// When the user clicks a push button to create a new markups node,
  /// make sure that a display node is added
  void onActiveMarkupMRMLNodeAdded(vtkMRMLNode *markupsNode);

protected:
  QScopedPointer<qSlicerMarkupsMeasurementsWidgetPrivate> d_ptr;
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsMeasurementsWidget);
  Q_DISABLE_COPY(qSlicerMarkupsMeasurementsWidget);

};

#endif
