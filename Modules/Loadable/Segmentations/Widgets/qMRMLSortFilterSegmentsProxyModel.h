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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __qMRMLSortFilterSegmentsProxyModel_h
#define __qMRMLSortFilterSegmentsProxyModel_h

// Segmentation includes
#include "qSlicerSegmentationsModuleWidgetsExport.h"

// Qt includes
#include <QSortFilterProxyModel>

// CTK includes
#include <ctkVTKObject.h>
#include <ctkPimpl.h>

class qMRMLSortFilterSegmentsProxyModelPrivate;
class QStandardItem;
class vtkMRMLSegmentationNode;
class vtkMRMLScene;

/// \ingroup Slicer_QtModules_Segmentations
class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSortFilterSegmentsProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
  QVTK_OBJECT

  /// Filter to show only items that contain the string in their names. Empty by default
  Q_PROPERTY(QString nameFilter READ nameFilter WRITE setNameFilter)
  /// Filter to show only items that contain the string in their names or within tag values. Empty by default
  Q_PROPERTY(QString textFilter READ textFilter WRITE setTextFilter)
  /// Filter to show segments with the state NotStarted
  /// If all four boolean flags are false, than no filtering is performed
  /// \sa showInProgress showCompleted showFlagged
  Q_PROPERTY(bool showNotStarted READ showNotStarted WRITE setShowNotStarted)
  /// Filter to show segments with the state InProgress
  /// If all four boolean flags are false, than no filtering is performed
  /// \sa showNotStarted showCompleted showFlagged
  Q_PROPERTY(bool showInProgress READ showInProgress WRITE setShowInProgress)
  ///Filter to show segments with the state Completed
  /// If all four boolean flags are false, than no filtering is performed
  /// \sa showNotStarted showInProgress showFlagged
  Q_PROPERTY(bool showCompleted READ showCompleted WRITE setShowCompleted)
  /// Filter to show segments with the state Flagged
  /// If all four boolean flags are false, than no filtering is performed
  /// \sa showNotStarted showInProgress showCompleted
  Q_PROPERTY(bool showFlagged READ showFlagged WRITE setShowFlagged)

public:
  typedef QSortFilterProxyModel Superclass;
  qMRMLSortFilterSegmentsProxyModel(QObject *parent=nullptr);
  ~qMRMLSortFilterSegmentsProxyModel() override;

  /// Returns the segmentation node in the source model
  Q_INVOKABLE vtkMRMLSegmentationNode* segmentationNode()const;

  /// Segments that have their ID listed in hideSegments are
  /// not shown in the table.
  Q_INVOKABLE void setHideSegments(const QStringList& segmentIDs);
  Q_INVOKABLE QStringList hideSegments()const;

  QString nameFilter()const;
  QString textFilter()const;
  bool showNotStarted()const;
  bool showInProgress()const;
  bool showCompleted()const;
  bool showFlagged()const;

  /// Retrieve the associated segment ID from a model index
  Q_INVOKABLE QString segmentIDFromIndex(const QModelIndex& index)const;

  /// Retrieve an index for a given a segment ID
  Q_INVOKABLE QModelIndex indexFromSegmentID(QString itemID, int column=0)const;

  /// Returns true if the item in the row indicated by the given sourceRow and
  /// sourceParent should be included in the model; otherwise returns false.
  /// This method tests each item via \a filterAcceptsItem
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent)const override;

  /// Filters items to decide which to display in the view
  virtual bool filterAcceptsItem(QString segmentID)const;

  /// Returns the flags for the current index
  Qt::ItemFlags flags(const QModelIndex & index)const override;

public slots:
  void setNameFilter(QString filter);
  void setTextFilter(QString filter);
  void setShowNotStarted(bool show);
  void setShowInProgress(bool show);
  void setShowCompleted(bool show);
  void setShowFlagged(bool show);

protected:
  QStandardItem* sourceItem(const QModelIndex& index)const;

protected:
  QScopedPointer<qMRMLSortFilterSegmentsProxyModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSortFilterSegmentsProxyModel);
  Q_DISABLE_COPY(qMRMLSortFilterSegmentsProxyModel);
};

#endif
