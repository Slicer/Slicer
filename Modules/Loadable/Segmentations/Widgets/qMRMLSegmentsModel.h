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

#ifndef __qMRMLSegmentsModel_h
#define __qMRMLSegmentsModel_h

// Qt includes
#include <QStandardItemModel>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Segments includes
#include "qSlicerSegmentationsModuleWidgetsExport.h"

class qMRMLSegmentsModelPrivate;
class vtkMRMLSegmentationNode;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkSegment;

/// \brief Item model for segments
///
/// Associated with a vtkMRMLSegmentation node. This model creates one model item
/// for each segment in the vtkSegmentation.
/// Individual segment items are updated only if the associated segment is updated
/// (vtkSegmentation::SegmentModified)
class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSegmentsModel : public QStandardItemModel
{
  Q_OBJECT
  QVTK_OBJECT

  /// Control in which column the segment name is displayed
  Q_PROPERTY (int nameColumn READ nameColumn WRITE setNameColumn)
  /// Control in which column the segment visibility is displayed (Qt::DecorationRole).
  /// A value of -1 (default) hides the column
  Q_PROPERTY (int visibilityColumn READ visibilityColumn WRITE setVisibilityColumn)
  /// Control in which column the segment color is displayed (qMRMLSegmentsModel::VisibilityRole).
  /// A value of -1 (default) hides the column
  Q_PROPERTY(int colorColumn READ colorColumn WRITE setColorColumn)
  /// Control in which column the segment opacity is displayed (Qt::EditRole).
  Q_PROPERTY (int opacityColumn READ opacityColumn WRITE setOpacityColumn)
  /// Control in which column the segment status is displayed (qMRMLSegmentsModel::StatusRole).
  Q_PROPERTY(int statusColumn READ statusColumn WRITE setStatusColumn)
  /// Control in which column the segment layer is displayed (qMRMLSegmentsModel::LayerRole).
  Q_PROPERTY(int layerColumn READ layerColumn WRITE setLayerColumn)

public:

  enum SegmentTableItemDataRole
  {
    SegmentIDRole = Qt::UserRole + 1,
    IndexRole,
    VisibilityRole,
    StatusRole,
  };

  typedef QStandardItemModel Superclass;
  qMRMLSegmentsModel(QObject *parent=nullptr);
  ~qMRMLSegmentsModel() override;

  int nameColumn()const;
  void setNameColumn(int column);
  int visibilityColumn()const;
  void setVisibilityColumn(int column);
  int colorColumn()const;
  void setColorColumn(int column);
  int opacityColumn()const;
  void setOpacityColumn(int column);
  int statusColumn()const;
  void setStatusColumn(int column);
  int layerColumn()const;
  void setLayerColumn(int layer);

  /// Returns the segment ID for the given index
  QString segmentIDFromIndex(const QModelIndex &index)const;
  // Returns the segment ID for the given item
  QString segmentIDFromItem(QStandardItem* item)const;
  // Returns the index for the given segment ID
  QModelIndex indexFromSegmentID(QString segmentID, int column=0)const;
  // Returns the item for the given segment ID
  QStandardItem* itemFromSegmentID(QString segmentID, int column=0)const;

  /// Return all the QModelIndexes (all the columns) for a given segment ID
  QModelIndexList indexes(QString segmentID)const;

  /// The segmentation node that is used to populate the model
  vtkMRMLSegmentationNode* segmentationNode()const;
  virtual void setSegmentationNode(vtkMRMLSegmentationNode* segmentation);

  /// Assemble terminology info string (for tooltips) from a segment's terminology tags
  Q_INVOKABLE static QString terminologyTooltipForSegment(vtkSegment* segment);

signals:
  /// Emitted when a segment property (e.g., name) is about to be changed.
  /// Can be used for capturing the current state of the segment, before it is modified.
  void segmentAboutToBeModified(const QString& segmentID);

  /// Signal requesting selecting items in the tree
  void requestSelectItems(QList<vtkIdType> itemIDs);

protected slots:
  /// Invoked when an item in the model is changed
  virtual void onItemChanged(QStandardItem* item);

  /// Recompute the number of columns in the model. Called when a [some]Column property is set.
  /// Needs maxColumnId() to be reimplemented in subclasses
  void updateColumnCount();

protected:
  qMRMLSegmentsModel(qMRMLSegmentsModelPrivate* pimpl, QObject *parent=nullptr);

  /// Removes all items and regenerates the model from the segments in the segmentation node
  virtual void rebuildFromSegments();
  /// Updates all items from the segments in the segmentation model
  virtual void updateFromSegments();

  virtual Qt::ItemFlags segmentFlags(QString segmentID, int column)const;

  /// Update QStandardItem associated using segmentID and column
  virtual void updateItemFromSegment(QStandardItem* item, QString segmentID, int column );
  /// Update QStandardItem data associated using segmentID and column
  virtual void updateItemDataFromSegment(QStandardItem* item, QString segmentID, int column );
  /// Update a segment in the MRML node using the associated QStandardItem
  virtual void updateSegmentFromItem(QString segmentID, QStandardItem* item );
  /// Update a segment in the MRML node using the associated QStandardItem data
  virtual void updateSegmentFromItemData(QString segmentID, QStandardItem* item );
  /// Update all of the the QStandardItem associated with a column
  void updateItemsFromColumnIndex(int column);
  /// Update all of the the QStandardItem associated with a segment ID
  void updateItemsFromSegmentID(QString segmentID);

  /// Rearrange the order of the rows to match the indices of the segments in the MRML node
  void reorderItems();

  /// Invoked when the segmentation node is modified with one of these events:
  /// vtkSegmentation::SegmentAdded,
  /// vtkSegmentation::SegmentRemoved,
  /// vtkSegmentation::SegmentModified,
  /// vtkSegmentation::SegmentsOrderModified
  static void onEvent(vtkObject* caller, unsigned long event, void* clientData, void* callData);

  /// Must be reimplemented in subclasses that add new column types
  virtual int maxColumnId()const;

  /// Called when a segment is added to the segmentation node
  virtual void onSegmentAdded(QString segmentID);
  /// Called when a segment is removed from the segmentation node
  virtual void onSegmentRemoved(QString segmentID);
  /// Called when a segment is modified in the segmentation node
  virtual void onSegmentModified(QString segmentID);
  /// Called when a the order of segment in the segmentation node is modified
  virtual void onSegmentOrderModified();
  /// Called when the vtkMRMLSegmentationDisplayNode is modified
  virtual void onDisplayNodeModified();

protected:
  QScopedPointer<qMRMLSegmentsModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSegmentsModel);
  Q_DISABLE_COPY(qMRMLSegmentsModel);
};

#endif
