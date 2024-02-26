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

#ifndef __qMRMLSegmentEditorWidget_h
#define __qMRMLSegmentEditorWidget_h

// Segmentations includes
#include "qSlicerSegmentationsModuleWidgetsExport.h"

// MRMLWidgets includes
#include "qMRMLWidget.h"

// Qt includes
#include <QVariant>

// CTK includes
#include <ctkVTKObject.h>

// STD includes
#include <cstdlib>

class vtkMRMLNode;
class vtkMRMLInteractionNode;
class vtkMRMLSegmentationNode;
class vtkMRMLSegmentEditorNode;
class vtkMRMLVolumeNode;
class vtkObject;
class QItemSelection;
class QAbstractButton;
class qMRMLSegmentEditorWidgetPrivate;
class qSlicerSegmentEditorAbstractEffect;
class qSlicerAbstractModuleWidget;

/// \brief Qt widget for editing a segment from a segmentation using Editor effects.
///
/// Widget for editing segmentations that can be re-used in any module.
///
/// IMPORTANT: The embedding module is responsible for setting the MRML scene and the
///   management of the \sa vtkMRMLSegmentEditorNode parameter set node.
///   The empty parameter set node should be set before the MRML scene, so that the
///   default selections can be stored in the parameter set node. Also, re-creation
///   of the parameter set node needs to be handled after scene close, and usage of
///   occasional existing parameter set nodes after scene import.
///
class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSegmentEditorWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(
    bool segmentationNodeSelectorVisible READ segmentationNodeSelectorVisible WRITE setSegmentationNodeSelectorVisible)
  Q_PROPERTY(
    bool sourceVolumeNodeSelectorVisible READ sourceVolumeNodeSelectorVisible WRITE setSourceVolumeNodeSelectorVisible)
  Q_PROPERTY(bool autoShowSourceVolumeNode READ autoShowSourceVolumeNode WRITE setAutoShowSourceVolumeNode)
  Q_PROPERTY(bool masterVolumeNodeSelectorVisible READ masterVolumeNodeSelectorVisible WRITE
               setMasterVolumeNodeSelectorVisible) // deprecated
  Q_PROPERTY(
    bool autoShowMasterVolumeNode READ autoShowMasterVolumeNode WRITE setAutoShowMasterVolumeNode) // deprecated
  Q_PROPERTY(bool switchToSegmentationsButtonVisible READ switchToSegmentationsButtonVisible WRITE
               setSwitchToSegmentationsButtonVisible)
  Q_PROPERTY(bool undoEnabled READ undoEnabled WRITE setUndoEnabled)
  Q_PROPERTY(int maximumNumberOfUndoStates READ maximumNumberOfUndoStates WRITE setMaximumNumberOfUndoStates)
  Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
  Q_PROPERTY(Qt::ToolButtonStyle effectButtonStyle READ effectButtonStyle WRITE setEffectButtonStyle)
  Q_PROPERTY(int effectColumnCount READ effectColumnCount WRITE setEffectColumnCount)
  Q_PROPERTY(bool unorderedEffectsVisible READ unorderedEffectsVisible WRITE setUnorderedEffectsVisible)
  Q_PROPERTY(QString defaultTerminologyEntrySettingsKey READ defaultTerminologyEntrySettingsKey WRITE
               setDefaultTerminologyEntrySettingsKey)
  Q_PROPERTY(QString defaultTerminologyEntry READ defaultTerminologyEntry WRITE setDefaultTerminologyEntry)
  Q_PROPERTY(bool jumpToSelectedSegmentEnabled READ jumpToSelectedSegmentEnabled WRITE setJumpToSelectedSegmentEnabled)

public:
  typedef qMRMLWidget Superclass;
  /// Constructor
  explicit qMRMLSegmentEditorWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qMRMLSegmentEditorWidget() override;

  /// Get the segment editor parameter set node
  Q_INVOKABLE vtkMRMLSegmentEditorNode* mrmlSegmentEditorNode() const;

  /// Get currently selected segmentation MRML node
  Q_INVOKABLE vtkMRMLNode* segmentationNode() const;
  /// Get ID of currently selected segmentation node
  Q_INVOKABLE QString segmentationNodeID() const;
  /// Get currently selected source volume MRML node
  Q_INVOKABLE vtkMRMLNode* sourceVolumeNode() const;
  /// Get ID of currently selected source volume node
  Q_INVOKABLE QString sourceVolumeNodeID() const;

  /// Deprecated. Use sourceVolumeNode() method instead.
  Q_INVOKABLE vtkMRMLNode* masterVolumeNode() const
  {
    qWarning("qMRMLSegmentEditorWidget::masterVolumeNode is deprecated, use sourceVolumeNode() method instead.");
    return this->sourceVolumeNode();
  };
  /// Deprecated. Use sourceVolumeNodeID() method instead.
  Q_INVOKABLE QString masterVolumeNodeID() const
  {
    qWarning("qMRMLSegmentEditorWidget::masterVolumeNodeID is deprecated, use sourceVolumeNodeID() method instead.");
    return this->sourceVolumeNodeID();
  };
  /// Get segment ID of selected segment
  Q_INVOKABLE QString currentSegmentID() const;

  /// Return active effect if selected, nullptr otherwise
  /// \sa m_ActiveEffect, setActiveEffect()
  Q_INVOKABLE qSlicerSegmentEditorAbstractEffect* activeEffect() const;
  /// Set active effect
  /// \sa m_ActiveEffect, activeEffect()
  Q_INVOKABLE void setActiveEffect(qSlicerSegmentEditorAbstractEffect* effect);

  /// Get an effect object by name
  /// \return The effect instance if exists, nullptr otherwise
  Q_INVOKABLE qSlicerSegmentEditorAbstractEffect* effectByName(QString name);

  /// Get list of all registered effect names that can be displayed in the widget.
  Q_INVOKABLE QStringList availableEffectNames();

  /// Request displaying effects in the specified order.
  /// Effects that are not listed will be hidden if \sa unorderedEffectsVisible is false.
  Q_INVOKABLE void setEffectNameOrder(const QStringList& effectNames);

  /// Get requested order of effects.
  /// Actually displayed effects can be retrieved by using \sa effectCount and \sa effectByIndex.
  /// \return List of effect names to be shown in the widget.
  Q_INVOKABLE QStringList effectNameOrder() const;

  /// Show/hide effect names that are not listed in \sa effectNameOrder().
  /// True by default to make effects registered by extensions show up by default.
  /// This can be used to simplify the editor widget to show only a limited number of effects.
  void setUnorderedEffectsVisible(bool visible);

  /// Get visibility status of effect names that are not listed in effectNameOrder().
  /// \sa setEffectNameOrder
  bool unorderedEffectsVisible() const;

  /// Get number of displayed effects
  /// \return Number of effects shown in the widget
  Q_INVOKABLE int effectCount();

  /// Get n-th effect shown in the widget. n>=0 and n<effectCount().
  /// \return The effect instance if exists, nullptr otherwise
  Q_INVOKABLE qSlicerSegmentEditorAbstractEffect* effectByIndex(int index);

  /// Create observations between slice view interactor and the widget.
  /// The captured events are propagated to the active effect if any.
  /// NOTE: This method should be called from the enter function of the
  ///   embedding module widget so that the events are correctly processed.
  Q_INVOKABLE void setupViewObservations();

  /// Remove observations
  /// NOTE: This method should be called from the exit function of the
  ///   embedding module widget so that events are not processed unnecessarily.
  Q_INVOKABLE void removeViewObservations();

  /// Show/hide the segmentation node selector widget.
  bool segmentationNodeSelectorVisible() const;
  /// Show/hide the source volume node selector widget.
  bool sourceVolumeNodeSelectorVisible() const;
  /// If autoShowSourceVolumeNode is enabled then source volume is automatically
  /// displayed in slice views when a new source volume is selected or layout is changed.
  /// Enabled by default.
  bool autoShowSourceVolumeNode() const;

  /// Deprecated. Use sourceVolumeNodeSelectorVisible() method instead.
  bool masterVolumeNodeSelectorVisible() const
  {
    qWarning("qMRMLSegmentEditorWidget::masterVolumeNodeSelectorVisible is deprecated, use "
             "sourceVolumeNodeSelectorVisible() method instead.");
    return this->sourceVolumeNodeSelectorVisible();
  };
  /// Deprecated. Use autoShowSourceVolumeNode() method instead.
  bool autoShowMasterVolumeNode() const
  {
    qWarning("qMRMLSegmentEditorWidget::autoShowMasterVolumeNode is deprecated, use autoShowSourceVolumeNode() method "
             "instead.");
    return this->autoShowSourceVolumeNode();
  };

  /// Show/hide the switch to Segmentations module button
  bool switchToSegmentationsButtonVisible() const;
  /// Undo/redo enabled.
  bool undoEnabled() const;
  /// Get maximum number of saved undo/redo states.
  int maximumNumberOfUndoStates() const;
  /// Get whether widget is read-only
  bool readOnly() const;

  /// Get appearance of effect buttons. Showing text may make it easier
  /// to find an effect but uses more space.
  Qt::ToolButtonStyle effectButtonStyle() const;

  /// Get number of columns being used by the effects.
  /// \return Number of columns being used for effects.
  int effectColumnCount() const;

  /// Add node type attribute that filter the segmentation nodes to display.
  /// \sa qMRMLNodeComboBox::addAttribute
  Q_INVOKABLE void segmentationNodeSelectorAddAttribute(const QString& nodeType,
                                                        const QString& attributeName,
                                                        const QVariant& attributeValue = QVariant());
  /// Remove node type attribute filtering the displayed segmentation nodes.
  /// \sa qMRMLNodeComboBox::addAttribute
  Q_INVOKABLE void segmentationNodeSelectorRemoveAttribute(const QString& nodeType, const QString& attributeName);

  /// Add node type attribute that filter the source volume nodes to display.
  /// \sa qMRMLNodeComboBox::addAttribute
  Q_INVOKABLE void sourceVolumeNodeSelectorAddAttribute(const QString& nodeType,
                                                        const QString& attributeName,
                                                        const QVariant& attributeValue = QVariant());
  /// Remove node type attribute filtering the displayed reference volume nodes.
  /// \sa qMRMLNodeComboBox::addAttribute
  Q_INVOKABLE void sourceVolumeNodeSelectorRemoveAttribute(const QString& nodeType, const QString& attributeName);

  /// Deprecated use sourceVolumeNodeSelectorAddAttribute method instead.
  Q_INVOKABLE void masterVolumeNodeSelectorAddAttribute(const QString& nodeType,
                                                        const QString& attributeName,
                                                        const QVariant& attributeValue = QVariant())
  {
    qWarning("qMRMLSegmentEditorWidget::masterVolumeNodeSelectorAddAttribute is deprecated, use "
             "sourceVolumeNodeSelectorAddAttribute method instead.");
    this->sourceVolumeNodeSelectorAddAttribute(nodeType, attributeName, attributeValue);
  }
  /// Deprecated use sourceVolumeNodeSelectorRemoveAttribute method instead.
  Q_INVOKABLE void masterVolumeNodeSelectorRemoveAttribute(const QString& nodeType, const QString& attributeName)
  {
    qWarning("qMRMLSegmentEditorWidget::masterVolumeNodeSelectorRemoveAttribute is deprecated,"
             " use sourceVolumeNodeSelectorRemoveAttribute instead.");
    this->sourceVolumeNodeSelectorRemoveAttribute(nodeType, attributeName);
  }

  /// Get current interaction node.
  /// \sa SetInteractionNode()
  Q_INVOKABLE vtkMRMLInteractionNode* interactionNode() const;

  /// Set settings key that stores defaultTerminologyEntry. If set to empty string then
  /// the defaultTerminologyEntry is not saved to/loaded from application settings.
  /// By default it is set to "Segmentations/DefaultTerminologyEntry".
  /// If settings key is changed then the current default terminology entry is not written
  /// into application settings (as it would overwrite its current value in the settings,
  /// which is usually not the expected behavior).
  void setDefaultTerminologyEntrySettingsKey(const QString& terminologyEntrySettingsKey);
  /// Get settings key that stores defaultTerminologyEntry.
  /// \sa setDefaultTerminologyEntrySettingsKey
  QString defaultTerminologyEntrySettingsKey() const;

  /// Set terminology entry string that is used for the first segment by default.
  /// The value is also written to application settings, if defaultTerminologyEntrySettingsKey is not empty.
  void setDefaultTerminologyEntry(const QString& terminologyEntry);
  /// Get terminology entry string that is used for the first segment by default.
  /// The value is read from application settings, if defaultTerminologyEntrySettingsKey is not empty.
  QString defaultTerminologyEntry();

  /// Returns true if automatic jump to current segment is enabled.
  bool jumpToSelectedSegmentEnabled() const;

public slots:
  /// Set the MRML \a scene associated with the widget
  void setMRMLScene(vtkMRMLScene* newScene) override;

  /// Set the segment editor parameter set node
  void setMRMLSegmentEditorNode(vtkMRMLSegmentEditorNode* newSegmentEditorNode);

  /// Update widget state from the MRML scene
  virtual void updateWidgetFromMRML();

  /// Set segmentation MRML node
  void setSegmentationNode(vtkMRMLNode* node);
  /// Set segmentation MRML node by its ID
  void setSegmentationNodeID(const QString& nodeID);
  /// Set source volume MRML node.
  /// If source volume has multiple scalar components
  /// then only the first scalar component is used.
  void setSourceVolumeNode(vtkMRMLNode* node);
  /// Set source volume MRML node by its ID
  void setSourceVolumeNodeID(const QString& nodeID);

  /// Deprecated. Use setSourceVolumeNode method instead.
  void setMasterVolumeNode(vtkMRMLNode* node)
  {
    qWarning("qMRMLSegmentEditorWidget::setMasterVolumeNode is deprecated, use setSourceVolumeNode method instead.");
    this->setSourceVolumeNode(node);
  }
  /// Deprecated. Use setSourceVolumeNodeID method instead.
  void setMasterVolumeNodeID(const QString& nodeID)
  {
    qWarning(
      "qMRMLSegmentEditorWidget::setMasterVolumeNodeID is deprecated, use setSourceVolumeNodeID method instead.");
    this->setSourceVolumeNodeID(nodeID);
  }

  /// Set selected segment by its ID
  void setCurrentSegmentID(const QString segmentID);

  /// Set active effect by name
  void setActiveEffectByName(QString effectName);

  /// Save current segmentation before performing an edit operation
  /// to allow reverting to the current state by using undo
  void saveStateForUndo();

  /// Update modifierLabelmap, maskLabelmap, or alignedSourceVolumeNode
  void updateVolume(void* volumePtr, bool& success);

  /// Show/hide the segmentation node selector widget.
  void setSegmentationNodeSelectorVisible(bool);
  /// Show/hide the source volume node selector widget.
  void setSourceVolumeNodeSelectorVisible(bool);
  /// If autoShowSourceVolumeNode is enabled then source volume is automatically
  /// displayed in slice views when a new source volume is selected or layout is changed.
  /// Enabled by default.
  void setAutoShowSourceVolumeNode(bool);
  /// Show/hide the switch to Segmentations module button
  void setSwitchToSegmentationsButtonVisible(bool);
  /// Undo/redo enabled.
  void setUndoEnabled(bool);
  /// Set maximum number of saved undo/redo states.
  void setMaximumNumberOfUndoStates(int);
  /// Set whether the widget is read-only
  void setReadOnly(bool aReadOnly);
  /// Enable/disable masking using source volume intensity
  void toggleSourceVolumeIntensityMaskEnabled();

  /// Deprecated. Use setSourceVolumeNodeSelectorVisible method instead.
  void setMasterVolumeNodeSelectorVisible(bool visible)
  {
    qWarning("qMRMLSegmentEditorWidget::setMasterVolumeNodeSelectorVisible is deprecated, use "
             "setSourceVolumeNodeSelectorVisible method instead.");
    this->setSourceVolumeNodeSelectorVisible(visible);
  }
  /// Deprecated. Use setAutoShowSourceVolumeNode method instead.
  void setAutoShowMasterVolumeNode(bool autoShow)
  {
    qWarning("qMRMLSegmentEditorWidget::setAutoShowMasterVolumeNode is deprecated, use setAutoShowSourceVolumeNode "
             "method instead.");
    this->setAutoShowSourceVolumeNode(autoShow);
  }
  /// Deprecated. Use toggleSourceVolumeIntensityMaskEnabled method instead.
  void toggleMasterVolumeIntensityMaskEnabled()
  {
    qWarning("qMRMLSegmentEditorWidget::toggleMasterVolumeIntensityMaskEnabled is deprecated, use "
             "toggleSourceVolumeIntensityMaskEnabled method instead.");
    this->toggleSourceVolumeIntensityMaskEnabled();
  }

  /// Restores previous saved state of the segmentation
  void undo();

  /// Restores next saved state of the segmentation
  void redo();

  /// Install keyboard shortcuts to allow quick selection of effects and segments.
  /// If parent is not specified then the main window will be used as parent.
  /// Previous keyboard shortcuts will be uninstalled.
  void installKeyboardShortcuts(QWidget* parent = nullptr);

  /// Uninstall previously installed keyboard shortcuts.
  void uninstallKeyboardShortcuts();

  /// Convenience method to turn off lightbox view in all slice viewers.
  /// Segment editor is not compatible with lightbox view layouts.
  /// Returns true if there were lightbox views.
  bool turnOffLightboxes();

  /// Unselect labelmap layer in all slice views in the active layout
  void hideLabelLayer();

  /// Set appearance of effect buttons. Showing text may make it easier
  /// to find an effect but uses more space.
  void setEffectButtonStyle(Qt::ToolButtonStyle toolButtonStyle);

  /// Request displaying effects in the grid layout with the specified column count
  void setEffectColumnCount(int columnCount);

  /// Update list of effect buttons.
  /// It adds all effects registered with the effect factory
  /// (and not filtered by unorderedEffectsVisible).
  void updateEffectList();

  /// Show source volume in slice views by hiding foreground and label volumes.
  /// \param forceShowInBackground If set to false then views will only change
  ///   if source volume is not selected as either foreground or background volume.
  /// \param fitSlice Reset field of view to include all volumes.
  void showSourceVolumeInSliceViewers(bool forceShowInBackground = false, bool fitSlice = false);

  // Deprecated. Use showSourceVolumeInSliceViewers method instead.
  void showMasterVolumeInSliceViewers(bool forceShowInBackground = false, bool fitSlice = false)
  {
    qWarning("qMRMLSegmentEditorWidget::showMasterVolumeInSliceViewers is deprecated, use "
             "showSourceVolumeInSliceViewers method instead.");
    this->showSourceVolumeInSliceViewers(forceShowInBackground, fitSlice);
  }

  /// Rotate slice views to be aligned with segmentation node's internal
  /// labelmap representation axes.
  void rotateSliceViewsToSegmentation();

  /// Set node used to notify active effect about interaction node changes.
  /// \sa interactionNode()
  void setInteractionNode(vtkMRMLInteractionNode* interactionNode);

  /// Select the segment above the currently selected one in the table (skipping segments that are not visible)
  void selectPreviousSegment();

  /// Select the segment below the currently selected one in the table (skipping segments that are not visible)
  void selectNextSegment();

  /// Select the segment offset from the currently selected one in the table (skipping segments that are not visible)
  /// Positive offset will move down the table
  /// Negative offset will move up the table
  void selectSegmentAtOffset(int offset);

  /// Jump position of all slice views to show the segment's center.
  /// Segment's center is determined as the center of bounding box.
  void jumpSlices();

  /// Enables automatic jumping to current segment when selection is changed.
  void setJumpToSelectedSegmentEnabled(bool enable);

signals:
  /// Emitted if different segment is selected in the segment list.
  void currentSegmentIDChanged(const QString&);

  /// Emitted when the user selects a different source volume
  /// (or any time source volume selection is changed in the segment editor parameter node).
  void sourceVolumeNodeChanged(vtkMRMLVolumeNode*);

  /// Deprecated. Use sourceVolumeNodeChanged signal instead.
  void masterVolumeNodeChanged(vtkMRMLVolumeNode*);

  /// Emitted when the user selects a different segmentation node
  /// (or any time segmentation node selection is changed in the segment editor parameter node).
  void segmentationNodeChanged(vtkMRMLSegmentationNode*);

protected slots:
  /// Handles changing of current segmentation MRML node
  void onSegmentationNodeChanged(vtkMRMLNode* node);
  /// Handles changing of the current source volume MRML node
  void onSourceVolumeNodeChanged(vtkMRMLNode* node);
  /// Handles segment selection changes
  void onSegmentSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  /// Handles mouse mode changes (view / place markups)
  void onInteractionNodeModified();

  /// Activate effect on clicking its button
  void onEffectButtonClicked(QAbstractButton* button);

  /// Effect selection shortcut is activated.
  /// 0 means deselect active effect.
  /// -1 toggles between no effect/last active effect.
  void onSelectEffectShortcut();

  /// Segment selection shortcut is activated
  void onSelectSegmentShortcut();

  /// Add empty segment
  void onAddSegment();
  /// Remove selected segment
  void onRemoveSegment();
  /// Edit segmentation properties in Segmentations module
  void onSwitchToSegmentations();
  /// Create/remove closed surface model for the segmentation that is automatically updated when editing
  void onCreateSurfaceToggled(bool on);
  /// Called if a segment or representation is added or removed
  void onSegmentAddedRemoved();
  /// Called if source volume image data is changed
  void onSourceVolumeImageDataModified();
  /// Handle layout changes
  void onLayoutChanged(int layoutIndex);
  /// Handle display node view ID changes
  void onSegmentationDisplayModified();

  /// Changed selected editable segment area
  void onMaskModeChanged(int);

  /// Enable/disable threshold when checkbox is toggled
  void onSourceVolumeIntensityMaskChecked(bool checked);
  /// Handles threshold values changed event
  void onSourceVolumeIntensityMaskRangeChanged(double low, double high);

  /// Changed selected overwritable segments
  void onOverwriteModeChanged(int);

  /// Clean up when scene is closed
  void onMRMLSceneEndCloseEvent();

  /// Updates needed after batch processing is done
  void onMRMLSceneEndBatchProcessEvent();

  /// Set default parameters in parameter set node (after setting or closing scene)
  void initializeParameterSetNode();

  /// Update undo/redo button states
  void updateUndoRedoButtonsState();

  /// Update GUI if segmentation history is changed (e.g., undo/redo button states)
  void onSegmentationHistoryChanged();

  /// Switch to Segmentations module and jump to Import/Export section
  void onImportExportActionClicked();
  /// Open Export to files dialog
  void onExportToFilesActionClicked();

  /// Update masking section on the UI
  void updateMaskingSection();

  /// Show slice rotation warning button if slice views are rotated, hide otherwise
  void updateSliceRotateWarningButtonVisibility();

  /// Show segmentation geometry dialog to specify labelmap geometry
  void showSegmentationGeometryDialog();

protected:
  /// Callback function invoked when interaction happens
  static void processEvents(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  void updateWidgetFromSegmentationNode();
  void updateWidgetFromSourceVolumeNode();
  void updateEffectsSectionFromMRML();

  /// Switches the source representation to binary labelmap. If the source representation
  /// cannot be set to binary labelmap (e.g., the user does not allow it) then false is returned.
  bool setSourceRepresentationToBinaryLabelmap();

  /// Switches to Segmentations module and returns the module widget
  qSlicerAbstractModuleWidget* switchToSegmentationsModule();

protected:
  QScopedPointer<qMRMLSegmentEditorWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSegmentEditorWidget);
  Q_DISABLE_COPY(qMRMLSegmentEditorWidget);
};

#endif
