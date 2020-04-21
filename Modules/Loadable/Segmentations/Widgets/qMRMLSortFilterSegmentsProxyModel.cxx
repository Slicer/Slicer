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

// Segmentations includes
#include "qMRMLSegmentsModel.h"
#include "qMRMLSortFilterSegmentsProxyModel.h"

// Segmentations logic includes
#include "vtkSlicerSegmentationsModuleLogic.h"

// MRML include
#include "vtkMRMLSegmentationNode.h"

// Qt includes
#include <QDebug>
#include <QStandardItem>

// -----------------------------------------------------------------------------
// qMRMLSortFilterSegmentsProxyModelPrivate

// -----------------------------------------------------------------------------
/// \ingroup Slicer_MRMLWidgets
class qMRMLSortFilterSegmentsProxyModelPrivate
{
public:
  qMRMLSortFilterSegmentsProxyModelPrivate();

  bool FilterEnabled{false};
  QString NameFilter;
  QString TextFilter;
  bool ShowStatus[vtkSlicerSegmentationsModuleLogic::LastStatus];
  QStringList HideSegments;
};

// -----------------------------------------------------------------------------
qMRMLSortFilterSegmentsProxyModelPrivate::qMRMLSortFilterSegmentsProxyModelPrivate()
  : NameFilter(QString())
  , TextFilter(QString())
{
  for (int i = 0; i < vtkSlicerSegmentationsModuleLogic::LastStatus; ++i)
    {
    this->ShowStatus[i] = false;
    }
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterSegmentsProxyModel

// -----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSortFilterSegmentsProxyModel, bool, filterEnabled, FilterEnabled);
CTK_GET_CPP(qMRMLSortFilterSegmentsProxyModel, QString, nameFilter, NameFilter);
CTK_GET_CPP(qMRMLSortFilterSegmentsProxyModel, QString, textFilter, TextFilter);

//------------------------------------------------------------------------------
qMRMLSortFilterSegmentsProxyModel::qMRMLSortFilterSegmentsProxyModel(QObject *vparent)
 : QSortFilterProxyModel(vparent)
 , d_ptr(new qMRMLSortFilterSegmentsProxyModelPrivate)
{
  this->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------------
qMRMLSortFilterSegmentsProxyModel::~qMRMLSortFilterSegmentsProxyModel() = default;

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* qMRMLSortFilterSegmentsProxyModel::segmentationNode()const
{
  qMRMLSegmentsModel* model = qobject_cast<qMRMLSegmentsModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return model->segmentationNode();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSegmentsProxyModel::setFilterEnabled(bool filterEnabled)
{
  Q_D(qMRMLSortFilterSegmentsProxyModel);
  if (d->FilterEnabled == filterEnabled)
  {
    return;
  }
  d->FilterEnabled = filterEnabled;
  this->invalidateFilter();
  emit filterModified();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSegmentsProxyModel::setNameFilter(QString filter)
{
  Q_D(qMRMLSortFilterSegmentsProxyModel);
  if (d->NameFilter == filter)
    {
    return;
    }
  d->NameFilter = filter;
  this->invalidateFilter();
  emit filterModified();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSegmentsProxyModel::setTextFilter(QString filter)
{
  Q_D(qMRMLSortFilterSegmentsProxyModel);
  if (d->TextFilter == filter)
    {
    return;
    }
  d->TextFilter = filter;
  this->invalidateFilter();
  emit filterModified();
}

//-----------------------------------------------------------------------------
bool qMRMLSortFilterSegmentsProxyModel::showStatus(int status) const
{
  Q_D(const qMRMLSortFilterSegmentsProxyModel);
  if (status < 0 || status >= vtkSlicerSegmentationsModuleLogic::LastStatus)
    {
    return false;
    }
  return d->ShowStatus[status];
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSegmentsProxyModel::setShowStatus(int status, bool shown)
{
  Q_D(qMRMLSortFilterSegmentsProxyModel);
  if (status < 0 || status >= vtkSlicerSegmentationsModuleLogic::LastStatus)
    {
    return;
    }
  if (d->ShowStatus[status] == shown)
    {
    return;
    }

  d->ShowStatus[status] = shown;
  this->invalidateFilter();
  emit filterModified();
}

//-----------------------------------------------------------------------------
QString qMRMLSortFilterSegmentsProxyModel::segmentIDFromIndex(const QModelIndex& index)const
{
  qMRMLSegmentsModel* segmentsModel = qobject_cast<qMRMLSegmentsModel*>(this->sourceModel());
  return segmentsModel->segmentIDFromIndex( this->mapToSource(index) );
}

//-----------------------------------------------------------------------------
QModelIndex qMRMLSortFilterSegmentsProxyModel::indexFromSegmentID(QString segmentID, int column)const
{
  qMRMLSegmentsModel* segmentsModel = qobject_cast<qMRMLSegmentsModel*>(this->sourceModel());
  return this->mapFromSource(segmentsModel->indexFromSegmentID(segmentID, column));
}

//-----------------------------------------------------------------------------
QStandardItem* qMRMLSortFilterSegmentsProxyModel::sourceItem(const QModelIndex& sourceIndex)const
{
  qMRMLSegmentsModel* model = qobject_cast<qMRMLSegmentsModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return sourceIndex.isValid() ? model->itemFromIndex(sourceIndex) : model->invisibleRootItem();
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterSegmentsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent)const
{
  QStandardItem* parentItem = this->sourceItem(sourceParent);
  if (!parentItem)
    {
    return false;
    }
  QStandardItem* item = nullptr;

  // Sometimes the row is not complete (DnD), search for a non null item
  for (int childIndex=0; childIndex < parentItem->columnCount(); ++childIndex)
    {
    item = parentItem->child(sourceRow, childIndex);
    if (item)
      {
      break;
      }
    }
  if (item == nullptr)
    {
    return false;
    }

  qMRMLSegmentsModel* model = qobject_cast<qMRMLSegmentsModel*>(this->sourceModel());
  QString segmentID = model->segmentIDFromItem(item);
  return this->filterAcceptsItem(segmentID);
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterSegmentsProxyModel::filterAcceptsItem(QString segmentID)const
{
  Q_D(const qMRMLSortFilterSegmentsProxyModel);

  // Filter if segment is hidden
  if (d->HideSegments.contains(segmentID))
    {
    return false;
    }

  if (!d->FilterEnabled)
    {
    return true;
    }

  qMRMLSegmentsModel* model = qobject_cast<qMRMLSegmentsModel*>(this->sourceModel());
  if (!model)
    {
    return false;
    }
  vtkMRMLSegmentationNode* segmentationNode = model->segmentationNode();
  if (!segmentationNode)
    {
    return false;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return false;
    }
  vtkSegment* segment = segmentation->GetSegment(segmentID.toStdString());
  if (!segment)
    {
    return false;
    }

  // Filter by segment name
  if (!d->NameFilter.isEmpty())
    {
    QString segmentName(segment->GetName());
    if (!segmentName.contains(d->NameFilter, Qt::CaseInsensitive))
      {
        return false;
      }
    }

  // Filter by segment text (name and tag value)
  if (!d->TextFilter.isEmpty())
    {
    bool matchFound = false;
    QString segmentName = segment->GetName();
    if (segmentName.contains(d->TextFilter, Qt::CaseInsensitive))
      {
      matchFound = true;
      }
    if (!matchFound)
      {
      std::map<std::string, std::string> tags;
      segment->GetTags(tags);
      for (const auto& keyValue : tags)
        {
        QString value = keyValue.second.c_str();
        if (value.contains(d->TextFilter))
          {
          matchFound = true;
          break;
          }
        }
      }
    if (!matchFound)
      {
      return false;
      }
    }

  // Filter if segment state does not match one of the shown states
  bool statusFilterEnabled = false;
  for (int i = 0; i < vtkSlicerSegmentationsModuleLogic::LastStatus; ++i)
    {
    statusFilterEnabled = d->ShowStatus[i];
    if (statusFilterEnabled)
      {
      break;
      }
    }

  if (statusFilterEnabled)
    {
    int status = vtkSlicerSegmentationsModuleLogic::GetSegmentStatus(segment);
    if (status >= 0 && status < vtkSlicerSegmentationsModuleLogic::LastStatus && !d->ShowStatus[status])
      {
      return false;
      }
    }

  // All criteria were met
  return true;
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLSortFilterSegmentsProxyModel::flags(const QModelIndex & index)const
{
  QString segmentID = this->segmentIDFromIndex(index);
  bool isSelectable = this->filterAcceptsItem(segmentID);
  qMRMLSegmentsModel* segmentsModel = qobject_cast<qMRMLSegmentsModel*>(this->sourceModel());
  QStandardItem* item = segmentsModel->itemFromSegmentID(segmentID, index.column());
  if (!item)
    {
    return Qt::ItemFlags();
    }

  QFlags<Qt::ItemFlag> flags = item->flags();
  if (isSelectable)
    {
    return flags | Qt::ItemIsSelectable;
    }
  else
    {
    return flags & ~Qt::ItemIsSelectable;
    }
}

// --------------------------------------------------------------------------
void qMRMLSortFilterSegmentsProxyModel::setHideSegments(const QStringList& segmentIDs)
{
  Q_D(qMRMLSortFilterSegmentsProxyModel);
  d->HideSegments = segmentIDs;
  this->invalidateFilter();
  emit filterModified();
}

// --------------------------------------------------------------------------
QStringList qMRMLSortFilterSegmentsProxyModel::hideSegments()const
{
  Q_D(const qMRMLSortFilterSegmentsProxyModel);
  return d->HideSegments;
}
