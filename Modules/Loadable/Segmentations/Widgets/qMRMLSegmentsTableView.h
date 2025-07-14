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

#ifndef __qMRMLSegmentsTableView_h
#define __qMRMLSegmentsTableView_h

// Segmentations includes
#include "qSlicerSegmentationsModuleWidgetsExport.h"

// MRMLWidgets includes
#include "qMRMLWidget.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class qMRMLSegmentsModel;
class qMRMLSegmentsTableViewPrivate;
class qMRMLSortFilterSegmentsProxyModel;
class QContextMenuEvent;
class QItemSelection;
class QStandardItem;
class QStringList;
class QTableWidgetItem;
class QTableView;
class vtkMRMLNode;
class vtkSegment;

class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSegmentsTableView : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  Q_PROPERTY(int selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged);
  Q_PROPERTY(bool headerVisible READ headerVisible WRITE setHeaderVisible NOTIFY headerVisibleChanged);
  Q_PROPERTY(bool visibilityColumnVisible READ visibilityColumnVisible WRITE setVisibilityColumnVisible NOTIFY visibilityColumnVisibleChanged);
  Q_PROPERTY(bool colorColumnVisible READ colorColumnVisible WRITE setColorColumnVisible NOTIFY colorColumnVisibleChanged);
  Q_PROPERTY(bool opacityColumnVisible READ opacityColumnVisible WRITE setOpacityColumnVisible NOTIFY opacityColumnVisibleChanged);
  Q_PROPERTY(bool statusColumnVisible READ statusColumnVisible WRITE setStatusColumnVisible NOTIFY statusColumnVisibleChanged);
  Q_PROPERTY(bool layerColumnVisible READ layerColumnVisible WRITE setLayerColumnVisible NOTIFY layerColumnVisibleChanged);
  Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly NOTIFY readOnlyChanged);
  Q_PROPERTY(bool filterBarVisible READ filterBarVisible WRITE setFilterBarVisible NOTIFY filterBarVisibleChanged);
  Q_PROPERTY(QString textFilter READ textFilter WRITE setTextFilter NOTIFY textFilterChanged);
  Q_PROPERTY(bool jumpToSelectedSegmentEnabled READ jumpToSelectedSegmentEnabled WRITE setJumpToSelectedSegmentEnabled NOTIFY jumpToSelectedSegmentEnabledChanged);
  Q_PROPERTY(int segmentCount READ segmentCount)
  Q_PROPERTY(
    QString useTerminologySelectorSettingsKey READ useTerminologySelectorSettingsKey WRITE setUseTerminologySelectorSettingsKey NOTIFY useTerminologySelectorSettingsKeyChanged)
  Q_PROPERTY(bool useTerminologySelector READ useTerminologySelector WRITE setUseTerminologySelector NOTIFY useTerminologySelectorChanged);
  Q_PROPERTY(bool terminologySelectorOptionVisible READ terminologySelectorOptionVisible WRITE setTerminologySelectorOptionVisible NOTIFY terminologySelectorOptionVisibleChanged)
  Q_PROPERTY(bool terminologySelectorAutoDisable READ terminologySelectorAutoDisable WRITE setTerminologySelectorAutoDisable NOTIFY terminologySelectorAutoDisableChanged);

  typedef qMRMLWidget Superclass;
  /// Constructor
  explicit qMRMLSegmentsTableView(QWidget* parent = nullptr);
  /// Destructor
  ~qMRMLSegmentsTableView() override;

  /// Get segmentation MRML node
  Q_INVOKABLE vtkMRMLNode* segmentationNode();

  /// Get access to the table widget to allow low-level customization
  Q_INVOKABLE QTableView* tableWidget();

  /// Return number of segments (rows) in the table
  int segmentCount() const;

  /// Get the segment IDs of selected segments
  Q_INVOKABLE QStringList selectedSegmentIDs();
  /// Select segments with specified IDs
  Q_INVOKABLE void setSelectedSegmentIDs(QStringList segmentIDs);
  /// Clear segment selection
  Q_INVOKABLE void clearSelection();

  /// Assemble terminology info string (for tooltips) from a segment's terminology tags
  Q_INVOKABLE static QString terminologyTooltipForSegment(vtkSegment* segment);

  int selectionMode();
  bool headerVisible();
  bool visibilityColumnVisible();
  bool colorColumnVisible();
  bool opacityColumnVisible();
  bool statusColumnVisible();
  bool layerColumnVisible();
  bool readOnly();
  bool filterBarVisible();

  /// Segments that have their ID listed in hideSegments are
  /// not shown in the table.
  Q_INVOKABLE void setHideSegments(const QStringList& segmentIDs);
  Q_INVOKABLE QStringList hideSegments() const;

  /// Return list of visible segment IDs
  Q_INVOKABLE QStringList displayedSegmentIDs() const;

  Q_INVOKABLE qMRMLSortFilterSegmentsProxyModel* sortFilterProxyModel() const;
  Q_INVOKABLE qMRMLSegmentsModel* model() const;

  /// The text used to filter the segments in the table
  /// \sa setTextFilter
  QString textFilter();

  /// The settings key used to specify whether standard terminologies or simple selectors are used for choosing segment name and color.
  /// Default value is "Segmentations/UseTerminologySelector".
  /// If set to empty then the option will not be saved to and loaded from application settings.
  /// \sa setUseTerminologySelectorSettingsKey
  QString useTerminologySelectorSettingsKey() const;

  /// Returns true if standard terminologies are used for choosing segment name and color.
  /// If false then simple selectors are used.
  bool useTerminologySelector() const;

  /// Returns true if the user can choose between the standard terminologies selector or the simple selectors for segment name and color.
  bool terminologySelectorOptionVisible() const;

  /// Offer automatic disable of using standard terminologies selector if custom segment names or colors are used frequently.
  bool terminologySelectorAutoDisable() const;

  // If the specified status should be shown in the table
  /// \sa setStatusShown
  Q_INVOKABLE bool statusShown(int status);

  /// Get the row for the specified segment ID
  Q_INVOKABLE int rowForSegmentID(QString segmentID);
  /// Get the segment ID for the specified row
  Q_INVOKABLE QString segmentIDForRow(int row);

public slots:
  /// Set segmentation MRML node
  void setSegmentationNode(vtkMRMLNode* node);
  /// Set MRML scene
  void setMRMLScene(vtkMRMLScene* newScene) override;

  /// Set selection mode in the table. Input value is int for Python compatibility. Actual values are
  /// defined in QAbstractItemView::SelectionMode. For example, QAbstractItemView::NoSelection,
  /// QAbstractItemView::SingleSelection, QAbstractItemView::ExtendedSelection.
  void setSelectionMode(int mode);

  void setHeaderVisible(bool visible);
  void setVisibilityColumnVisible(bool visible);
  void setColorColumnVisible(bool visible);
  void setOpacityColumnVisible(bool visible);
  void setStatusColumnVisible(bool visible);
  void setLayerColumnVisible(bool visible);
  void setReadOnly(bool aReadOnly);
  void setFilterBarVisible(bool visible);

  /// Show only selected segments
  void showOnlySelectedSegments();

  /// Toggle visibility for the selected segments
  void toggleSelectedSegmentsVisibility();

  /// Jump position of all slice views to show the segment's center.
  /// Segment's center is determined as the center of bounding box.
  void jumpSlices();

  /// Enables automatic jumping to current segment when selection is changed.
  void setJumpToSelectedSegmentEnabled(bool enable);

  /// Set the status of the selected segments
  void setSelectedSegmentsStatus(int status = -1);

  /// Erase the contents of the selected segments and set the status to "Not started"
  void clearSelectedSegments();

  /// Move selected segments up in the list
  void moveSelectedSegmentsUp();
  /// Move selected segments down in the list
  void moveSelectedSegmentsDown();

  /// Set the text used to filter the segments in the table
  /// \sa textFilter
  void setTextFilter(QString textFilter);
  /// Set the settings key used to specify whether standard terminologies are used for name and color.
  /// \sa useTerminologySelectorSettingsKey
  void setUseTerminologySelectorSettingsKey(QString settingsKey);
  /// Set if standard terminologies are used for choosing segment name and color.
  void setUseTerminologySelector(bool useTerminologySelector);
  /// Set if the user can choose between the standard terminologies selector or the simple selectors for segment name and color.
  void setTerminologySelectorOptionVisible(bool visible);
  /// Offer automatic disable of using standard terminologies selector if custom segment names or colors are used frequently.
  void setTerminologySelectorAutoDisable(bool autoDisable);

  /// Set if the specified status should be shown in the table
  /// \sa statusShown
  void setStatusShown(int status, bool shown);

  /// Returns true if automatic jump to current segment is enabled.
  bool jumpToSelectedSegmentEnabled() const;

signals:
  /// Emitted if selection changes
  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  /// Emitted when a segment property (e.g., name) is about to be changed.
  /// Can be used for capturing the current state of the segment, before it is modified.
  void segmentAboutToBeModified(const QString& segmentID);
Q_SIGNALS:
  void selectionModeChanged(int);
  void headerVisibleChanged(bool);
  void visibilityColumnVisibleChanged(bool);
  void colorColumnVisibleChanged(bool);
  void opacityColumnVisibleChanged(bool);
  void statusColumnVisibleChanged(bool);
  void layerColumnVisibleChanged(bool);
  void readOnlyChanged(bool);
  void filterBarVisibleChanged(bool);
  void textFilterChanged(QString);
  void jumpToSelectedSegmentEnabledChanged(bool);
  void useTerminologySelectorChanged(bool);
  void terminologySelectorAutoDisableChanged(bool);
  void useTerminologySelectorSettingsKeyChanged(QString);
  void terminologySelectorOptionVisibleChanged(bool);

protected slots:
  /// Forwards selection changed events. In case of batch update of items, selected and deselected are empty.
  void onSegmentSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  /// Handles actions on table cell (visibility)
  void onVisibility3DActionToggled(bool visible);
  void onVisibility2DFillActionToggled(bool visible);
  void onVisibility2DOutlineActionToggled(bool visible);

  /// Handles when the filters on underlying sort model are modified
  void onSegmentsFilterModified();
  /// Handles clicks on the show status buttons
  void onShowStatusButtonClicked();

  /// Handles clicks on a table cell (visibility + state)
  void onSegmentsTableClicked(const QModelIndex& modelIndex);
  /// Handles clicks on a table cell (name + color change / terminology change)
  void onSegmentsTableDoubleClicked(const QModelIndex& modelIndex);

  /// Handle MRML scene event
  void endProcessing();

  void onSegmentAddedOrRemoved();

  /// Update the widget form the MRML node
  /// Called when the segmentation node is modified
  void updateWidgetFromMRML();

  /// Update the filter parameters in the vtkMRMLSegmentationNode
  void updateMRMLFromFilterParameters();

  /// Signals to save/restore segment ID selection when the model is reset
  void modelAboutToBeReset();
  void modelReset();

  /// Toggle using standard terminology for this segments table
  void onUseTerminologyActionToggled(bool useTerminology);

protected:
  /// Convenience function to set segment visibility options from event handlers
  /// \sa onVisibilityButtonToggled \sa onVisibility3DActionToggled \sa onVisibility2DFillActionToggled \sa onVisibility2DOutlineActionToggled
  /// \param visible Visibility of the segment referenced from senderObject. If 0, then hide, if 1 then show, otherwise don't change
  /// \param visible3D Visibility of the segment referenced from senderObject in 3D. If 0, then hide, if 1 then show, otherwise don't change
  /// \param visible2DFill Visibility of the segment referenced from senderObject for 2D fill. If 0, then hide, if 1 then show, otherwise don't change
  /// \param visible2DOutline Visibility of the segment referenced from senderObject for 2D outline. If 0, then hide, if 1 then show, otherwise don't change
  void setSegmentVisibility(QObject* senderObject, int visible, int visible3D, int visible2DFill, int visible2DOutline);
  void setSegmentVisibility(QString segmentId, int visible, int visible3D, int visible2DFill, int visible2DOutline);

  /// To prevent accidentally moving out of the widget when pressing up/down arrows
  bool eventFilter(QObject* target, QEvent* event) override;

  /// Handle context menu events
  void contextMenuEvent(QContextMenuEvent* event) override;

  bool userSetCustomNameOrColor();

protected:
  QScopedPointer<qMRMLSegmentsTableViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSegmentsTableView);
  Q_DISABLE_COPY(qMRMLSegmentsTableView);
};

#endif
