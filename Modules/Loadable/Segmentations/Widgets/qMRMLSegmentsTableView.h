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

/// \ingroup Slicer_QtModules_Segmentations_Widgets
class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSegmentsTableView : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  Q_PROPERTY(int selectionMode READ selectionMode WRITE setSelectionMode)
  Q_PROPERTY(bool headerVisible READ headerVisible WRITE setHeaderVisible)
  Q_PROPERTY(bool visibilityColumnVisible READ visibilityColumnVisible WRITE setVisibilityColumnVisible)
  Q_PROPERTY(bool colorColumnVisible READ colorColumnVisible WRITE setColorColumnVisible)
  Q_PROPERTY(bool opacityColumnVisible READ opacityColumnVisible WRITE setOpacityColumnVisible)
  Q_PROPERTY(bool statusColumnVisible READ statusColumnVisible WRITE setStatusColumnVisible)
  Q_PROPERTY(bool layerColumnVisible READ layerColumnVisible WRITE setLayerColumnVisible)
  Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
  Q_PROPERTY(bool filterBarVisible READ filterBarVisible WRITE setFilterBarVisible)
  Q_PROPERTY(QString textFilter READ textFilter WRITE setTextFilter)
  Q_PROPERTY(bool jumpToSelectedSegmentEnabled READ jumpToSelectedSegmentEnabled WRITE setJumpToSelectedSegmentEnabled)

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
  Q_INVOKABLE QStringList hideSegments()const;

  /// Return list of visible segment IDs
  Q_INVOKABLE QStringList displayedSegmentIDs()const;

  Q_INVOKABLE qMRMLSortFilterSegmentsProxyModel* sortFilterProxyModel()const;
  Q_INVOKABLE qMRMLSegmentsModel* model()const;

  /// The text used to filter the segments in the table
  /// \sa setTextFilter
  QString textFilter();
  // If the specified status should be shown in the table
  /// \sa setStatusShown
  Q_INVOKABLE bool statusShown(int status);

  /// Get the row for the specified segment ID
  int rowForSegmentID(QString segmentID);
  /// Get the segment ID for the specified row
  QString segmentIDForRow(int row);

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

  /// Jump position of all slice views to show the segment's center.
  /// Segment's center is determined as the center of bounding box.
  void jumpSlices();

  /// Enables automatic jumping to current segment when selection is changed.
  void setJumpToSelectedSegmentEnabled(bool enable);

  /// Set the status of the selected segments
  void setSelectedSegmentsStatus(int status=-1);

  /// Erase the contents of the selected segments and set the status to "Not started"
  void clearSelectedSegments();

  /// Move selected segments up in the list
  void moveSelectedSegmentsUp();
  /// Move selected segments down in the list
  void moveSelectedSegmentsDown();

  /// Set the text used to filter the segments in the table
  /// \sa textFilter
  void setTextFilter(QString textFilter);
  /// Set if the specified status should be shown in the table
  /// \sa statusShown
  void setStatusShown(int status, bool shown);

  /// Returns true if automatic jump to current segment is enabled.
  bool jumpToSelectedSegmentEnabled()const;

signals:
  /// Emitted if selection changes
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  /// Emitted when a segment property (e.g., name) is about to be changed.
  /// Can be used for capturing the current state of the segment, before it is modified.
  void segmentAboutToBeModified(const QString &segmentID);

protected slots:
  /// Forwards selection changed events. In case of batch update of items, selected and deselected are empty.
  void onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

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

  /// Handle MRML scene event
  void endProcessing();

  void onSegmentAddedOrRemoved();

  /// Update the widget form the MRML node
  /// Called when the segmentation node is modified
  void updateWidgetFromMRML();

  /// Update the filter parameters in the vtkMRMLSegmentationNode
  void updateMRMLFromFilterParameters();

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

protected:
  QScopedPointer<qMRMLSegmentsTableViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSegmentsTableView);
  Q_DISABLE_COPY(qMRMLSegmentsTableView);
};

#endif
