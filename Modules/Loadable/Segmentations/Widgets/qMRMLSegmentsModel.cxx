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

// Qt includes
#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QMessageBox>
#include <QMimeData>
#include <QTimer>

// qMRML includes
#include "qMRMLSegmentsModel_p.h"

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerAbstractCoreModule.h>

// MRML includes
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLSegmentationNode.h>

// Terminology includes
#include "qSlicerTerminologyItemDelegate.h"
#include "vtkSlicerTerminologiesModuleLogic.h"
#include "vtkSlicerTerminologyEntry.h"
#include "vtkSlicerTerminologyCategory.h"
#include "vtkSlicerTerminologyType.h"

// Polyseg includes
#include "vtkSegment.h"

// Segmentations includes
#include "qMRMLSegmentsTableView.h"

// Segmentations logic includes
#include "vtkSlicerSegmentationsModuleLogic.h"

//------------------------------------------------------------------------------
qMRMLSegmentsModelPrivate::qMRMLSegmentsModelPrivate(qMRMLSegmentsModel& object)
  : q_ptr(&object)
  , UpdatingItemFromSegment(false)
  , NameColumn(-1)
  , VisibilityColumn(-1)
  , ColorColumn(-1)
  , OpacityColumn(-1)
  , StatusColumn(-1)
  , LayerColumn(-1)
  , SegmentationNode(nullptr)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();

  this->HiddenIcon = QIcon(":Icons/VisibleOff.png");
  this->VisibleIcon = QIcon(":Icons/VisibleOn.png");

  this->NotStartedIcon = QIcon(":Icons/NotStarted.png");
  this->InProgressIcon = QIcon(":Icons/InProgress.png");
  this->FlaggedIcon = QIcon(":Icons/Flagged.png");
  this->CompletedIcon = QIcon(":Icons/Completed.png");

  qRegisterMetaType<QStandardItem*>("QStandardItem*");
}

//------------------------------------------------------------------------------
qMRMLSegmentsModelPrivate::~qMRMLSegmentsModelPrivate()
{
  if (this->SegmentationNode)
    {
    this->SegmentationNode->RemoveObserver(this->CallBack);
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModelPrivate::init()
{
  Q_Q(qMRMLSegmentsModel);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qMRMLSegmentsModel::onEvent);

  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)), q, SLOT(onItemChanged(QStandardItem*)));

  q->setVisibilityColumn(0);
  q->setColorColumn(1);
  q->setOpacityColumn(2);
  q->setNameColumn(3);
  q->setLayerColumn(4);
  q->setStatusColumn(5);

  QStringList columnLabels;
  for (int i = 0; i < q->columnCount(); ++i)
    {
    if (i == q->visibilityColumn())
      {
      columnLabels << "";
      }
    else if (i == q->colorColumn())
      {
      columnLabels << "";
      }
    else if (i == q->opacityColumn())
      {
      columnLabels << "Opacity";
      }
    else if (i == q->nameColumn())
      {
      columnLabels << "Name";
      }
    else if (i == q->layerColumn())
      {
      columnLabels << "Layer";
      }
    else if (i == q->statusColumn())
      {
      columnLabels << "";
      }
    }
  q->setHorizontalHeaderLabels(columnLabels);

  q->horizontalHeaderItem(q->nameColumn())->setToolTip(qMRMLSegmentsModel::tr("Segment name"));
  q->horizontalHeaderItem(q->visibilityColumn())->setToolTip(qMRMLSegmentsModel::tr("Segment visibility"));
  q->horizontalHeaderItem(q->colorColumn())->setToolTip(qMRMLSegmentsModel::tr("Segment color"));
  q->horizontalHeaderItem(q->opacityColumn())->setToolTip(qMRMLSegmentsModel::tr("Segment opacity (3D views)"));
  q->horizontalHeaderItem(q->statusColumn())->setToolTip(qMRMLSegmentsModel::tr("Segment status"));

  q->horizontalHeaderItem(q->visibilityColumn())->setIcon(QIcon(":/Icons/Small/SlicerVisibleInvisible.png"));
  q->horizontalHeaderItem(q->colorColumn())->setIcon(QIcon(":/Icons/Colors.png"));
  q->horizontalHeaderItem(q->statusColumn())->setIcon(QIcon(":/Icons/Flagged.png"));
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSegmentsModelPrivate::insertSegment(QString segmentID, int row/*=1*/)
{
  Q_Q(qMRMLSegmentsModel);
  QStandardItem* item = q->itemFromSegmentID(segmentID);
  if (item)
    {
    // It is possible that the item has been already added if it is the parent of a child item already inserted
    return item;
    }

  QList<QStandardItem*> items;
  for (int col = 0; col < q->columnCount(); ++col)
    {
    QStandardItem* newItem = new QStandardItem();
    q->updateItemFromSegment(newItem, segmentID, col);
    items.append(newItem);
    }

  if (row == -1)
    {
    row = this->SegmentationNode->GetSegmentation()->GetSegmentIndex(segmentID.toStdString());
    }
  q->insertRow(row, items);

  item = items[0];
  if (q->itemFromSegmentID(segmentID) != item)
    {
    qCritical() << Q_FUNC_INFO << ": Item mismatch when inserting segment item with ID " << segmentID;
    return nullptr;
    }
  return item;
}

//------------------------------------------------------------------------------
QString qMRMLSegmentsModelPrivate::getTerminologyUserDataForSegment(vtkSegment* segment)
{
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment given";
    return QString();
    }

  std::string tagValue;
  return (segment->GetTag(vtkSegment::GetTerminologyEntryTagName(), tagValue) ? QString(tagValue.c_str()) : QString());
}

//------------------------------------------------------------------------------
// qMRMLSegmentsModel
//------------------------------------------------------------------------------
qMRMLSegmentsModel::qMRMLSegmentsModel(QObject *_parent)
  :QStandardItemModel(_parent)
  , d_ptr(new qMRMLSegmentsModelPrivate(*this))
{
  Q_D(qMRMLSegmentsModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLSegmentsModel::qMRMLSegmentsModel(qMRMLSegmentsModelPrivate* pimpl, QObject* parent)
  : QStandardItemModel(parent)
  , d_ptr(pimpl)
{
  Q_D(qMRMLSegmentsModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLSegmentsModel::~qMRMLSegmentsModel() = default;

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::setSegmentationNode(vtkMRMLSegmentationNode* segmentationNode)
{
  Q_D(qMRMLSegmentsModel);
  if (segmentationNode == d->SegmentationNode)
    {
    return;
    }

  if (d->SegmentationNode)
    {
    d->SegmentationNode->RemoveObserver(d->CallBack);
    }
  d->SegmentationNode = segmentationNode;

  // Update all segments
  this->rebuildFromSegments();

  if (d->SegmentationNode)
    {
    d->SegmentationNode->AddObserver(vtkSegmentation::SegmentAdded, d->CallBack, 10.0);
    d->SegmentationNode->AddObserver(vtkSegmentation::SegmentRemoved, d->CallBack, 10.0);
    d->SegmentationNode->AddObserver(vtkSegmentation::SegmentModified, d->CallBack, -10.0);
    d->SegmentationNode->AddObserver(vtkSegmentation::SegmentsOrderModified, d->CallBack, -15.0);
    d->SegmentationNode->AddObserver(vtkMRMLDisplayableNode::DisplayModifiedEvent, d->CallBack, -15.0);
    d->SegmentationNode->AddObserver(vtkMRMLSegmentationNode::SegmentationChangedEvent, d->CallBack);
    }
}

//------------------------------------------------------------------------------
vtkMRMLSegmentationNode* qMRMLSegmentsModel::segmentationNode()const
{
  Q_D(const qMRMLSegmentsModel);
  return d->SegmentationNode;
}

// -----------------------------------------------------------------------------
QString qMRMLSegmentsModel::segmentIDFromIndex(const QModelIndex &index)const
{
  return this->segmentIDFromItem(this->itemFromIndex(index));
}

//------------------------------------------------------------------------------
QString qMRMLSegmentsModel::segmentIDFromItem(QStandardItem* item)const
{
  Q_D(const qMRMLSegmentsModel);
  if (!d->SegmentationNode || !item)
    {
    return "";
    }
  QVariant segmentID = item->data(qMRMLSegmentsModel::SegmentIDRole);
  if (!segmentID.isValid())
    {
    return "";
    }
  return item->data(qMRMLSegmentsModel::SegmentIDRole).toString();
}
//------------------------------------------------------------------------------
QStandardItem* qMRMLSegmentsModel::itemFromSegmentID(QString segmentID, int column/*=0*/)const
{
  QModelIndex index = this->indexFromSegmentID(segmentID, column);
  QStandardItem* item = this->itemFromIndex(index);
  return item;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLSegmentsModel::indexFromSegmentID(QString segmentID, int column/*=0*/)const
{
  Q_D(const qMRMLSegmentsModel);

  QModelIndex itemIndex;
  if (segmentID.isEmpty())
    {
    return itemIndex;
    }

  QModelIndex startIndex = this->index(0, 0);
  // QAbstractItemModel::match doesn't browse through columns, we need to do it manually
  QModelIndexList itemIndexes = this->match(
    startIndex, SegmentIDRole, segmentID, 1, Qt::MatchExactly | Qt::MatchRecursive);
  if (itemIndexes.size() == 0)
    {
    return QModelIndex();
    }
  itemIndex = itemIndexes[0];

  if (column == 0)
    {
    // QAbstractItemModel::match only search through the first column
    return itemIndex;
    }

  // Add the QModelIndexes from the other columns
  const int row = itemIndex.row();
  QModelIndex nodeParentIndex = itemIndex.parent();
  if (column >= this->columnCount(nodeParentIndex))
    {
    qCritical() << Q_FUNC_INFO << ": Invalid column " << column;
    return QModelIndex();
    }

  return this->index(row, column, itemIndex.parent());;
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLSegmentsModel::indexes(QString segmentID) const
{
  QModelIndex startIndex = this->index(0, 0);
  // QAbstractItemModel::match doesn't browse through columns, we need to do it manually
  QModelIndexList itemIndexes = this->match(
    startIndex, SegmentIDRole, segmentID, 1, Qt::MatchExactly | Qt::MatchRecursive);
  if (itemIndexes.size() != 1)
    {
    return QModelIndexList(); // If 0 it's empty, if >1 it's invalid (one item for each UID)
    }
  // Add the QModelIndexes from the other columns
  const int row = itemIndexes[0].row();
  for (int col = 1; col < this->columnCount(); ++col)
    {
    itemIndexes << this->index(row, col);
    }
  return itemIndexes;
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::rebuildFromSegments()
{
  Q_D(qMRMLSegmentsModel);

  this->beginResetModel();

  // Enabled so it can be interacted with
  this->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);

  // Remove rows before populating
  this->removeRows(0, this->rowCount());

  if (!d->SegmentationNode)
    {
    this->endResetModel();
    return;
    }

  // Populate model with the segments
  std::vector<std::string> segmentIDs;
  d->SegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
  for (std::string segmentID : segmentIDs)
    {
    d->insertSegment(QString::fromStdString(segmentID));
    }

  this->endResetModel();
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::updateFromSegments()
{
  Q_D(qMRMLSegmentsModel);

  // Update model with the segments
  std::vector<std::string> segmentIDs;
  d->SegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
  for (std::string segmentID : segmentIDs)
    {
    this->updateItemsFromSegmentID(segmentID.c_str());
    }
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLSegmentsModel::segmentFlags(QString segmentID, int column)const
{
  Q_D(const qMRMLSegmentsModel);
  Q_UNUSED(segmentID);

  Qt::ItemFlags flags;
  flags.setFlag(Qt::ItemIsEnabled);
  flags.setFlag(Qt::ItemIsSelectable);

  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation node";
    return flags;
    }

  if (column != this->visibilityColumn() && column != this->statusColumn() && column != this->layerColumn())
    {
    flags.setFlag(Qt::ItemIsEditable);
    }

  return flags;
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::updateItemFromSegment(QStandardItem* item, QString segmentID, int column)
{
  Q_D(qMRMLSegmentsModel);
  if (!item)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid item";
    return;
    }

  bool wasUpdating = d->UpdatingItemFromSegment;
  d->UpdatingItemFromSegment = true;

  Qt::ItemFlags flags = this->segmentFlags(segmentID, column);
  item->setFlags(flags);

  //// Set ID
  item->setData(segmentID, qMRMLSegmentsModel::SegmentIDRole);

  //// Update item data for the current column
  this->updateItemDataFromSegment(item, segmentID, column);

  d->UpdatingItemFromSegment = wasUpdating;
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::updateItemDataFromSegment(QStandardItem* item, QString segmentID, int column)
{
  Q_D(qMRMLSegmentsModel);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation node";
    return;
    }

  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  if (!segmentation)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation";
    return;
    }

  vtkSegment* segment = segmentation->GetSegment(segmentID.toStdString());
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment " << segmentID;
    return;
    }

  int index = segmentation->GetSegmentIndex(segmentID.toStdString());
  item->setData(index, IndexRole);

  if (column == this->nameColumn())
    {
    item->setText(segment->GetName());
    }
  else if (column == this->statusColumn())
    {
    int status = vtkSlicerSegmentationsModuleLogic::GetSegmentStatus(segment);
    QIcon statusIcon = d->NotStartedIcon;
    QString statusTooltip = "Not started";
    switch (status)
      {
      case vtkSlicerSegmentationsModuleLogic::InProgress:
        statusIcon = d->InProgressIcon;
        statusTooltip = "In progress";
        break;
      case vtkSlicerSegmentationsModuleLogic::Completed:
        statusIcon = d->CompletedIcon;
        statusTooltip = "Completed";
        break;
      case vtkSlicerSegmentationsModuleLogic::Flagged:
        statusIcon = d->FlaggedIcon;
        statusTooltip = "Flagged";
        break;
      }

    if (item->data(StatusRole).isNull() ||
        item->data(StatusRole).toInt() != status) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
      {
      item->setData(status, StatusRole);
      item->setToolTip(statusTooltip);
      item->setIcon(statusIcon);
      }
    }
  else if (column == this->layerColumn())
    {
    int layer = segmentation->GetLayerIndex(segmentID.toStdString(), vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    std::stringstream ss;
    ss << layer;
    item->setText(QString::fromStdString(ss.str()));
    item->setTextAlignment(Qt::AlignCenter);
    }
  else
    {
    // Get segment display properties
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode());
    if (!displayNode)
      {
      qCritical() << Q_FUNC_INFO << ": Invalid segmentation display node";
      return;
      }

    vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    displayNode->GetSegmentDisplayProperties(segmentID.toStdString(), properties);

    if (column == this->colorColumn())
      {
      // Set terminology information from segment to item
      item->setData(segment->GetName(), qSlicerTerminologyItemDelegate::NameRole);
      item->setData(segment->GetNameAutoGenerated(), qSlicerTerminologyItemDelegate::NameAutoGeneratedRole);
      item->setData(segment->GetColorAutoGenerated(), qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole);
      QString segmentTerminologyTagValue(d->getTerminologyUserDataForSegment(segment));
      if (segmentTerminologyTagValue != item->data(qSlicerTerminologyItemDelegate::TerminologyRole).toString())
        {
        item->setData(segmentTerminologyTagValue, qSlicerTerminologyItemDelegate::TerminologyRole);
        item->setToolTip(qMRMLSegmentsTableView::terminologyTooltipForSegment(segment));
        }
      // Set color
      double* colorArray = segment->GetColor();
      QColor color = QColor::fromRgbF(colorArray[0], colorArray[1], colorArray[2]);
      item->setData(color, Qt::DecorationRole);
      }
    else if (column == this->visibilityColumn())
      {
      // Have owner plugin give the visibility state and icon
      bool visible = properties.Visible && (properties.Visible3D || properties.Visible2DFill || properties.Visible2DOutline);
      QIcon visibilityIcon = d->HiddenIcon;
      if (visible)
        {
        visibilityIcon = d->VisibleIcon;
        }
      // It should be fine to set the icon even if it is the same, but due
      // to a bug in Qt (http://bugreports.qt.nokia.com/browse/QTBUG-20248),
      // it would fire a superfluous itemChanged() signal.
      if (item->data(VisibilityRole).isNull()
        || item->data(VisibilityRole).toBool() != visible)
        {

        if (item->data(VisibilityRole).isNull() ||
          item->data(VisibilityRole) != visible) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
          {
          item->setData(visible, VisibilityRole);
          item->setIcon(visibilityIcon);
          }
        }
      }
    else if (column == this->opacityColumn())
      {
      QString displayedOpacityStr = QString::number(properties.Opacity3D, 'f', 2);
      item->setData(displayedOpacityStr, Qt::EditRole);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::updateSegmentFromItem(QString segmentID, QStandardItem* item)
{
  Q_D(qMRMLSegmentsModel);
  //MRMLNodeModify segmentationNodeModify(d->SegmentationNode);//TODO: Add feature to item if there are performance issues
  // Calling StartModfiy/EndModify will cause the calldata to be erased, causing the whole table to be updated
  this->updateSegmentFromItemData(segmentID, item);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::updateSegmentFromItemData(QString segmentID, QStandardItem* item)
{
  Q_D(qMRMLSegmentsModel);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation node";
    return;
    }

  if (!item)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid item";
    return;
    }

  // Name column
  if (item->column() == this->nameColumn())
    {
    vtkSegment* segment = d->SegmentationNode->GetSegmentation()->GetSegment(segmentID.toStdString());
    if (!segment)
      {
      qCritical() << Q_FUNC_INFO << ": Segment with ID '" << segmentID << "' not found in segmentation node " << d->SegmentationNode->GetName();
      return;
      }
    std::string name = item->text().toStdString();
    emit segmentAboutToBeModified(segmentID);
    segment->SetName(name.c_str());
    if (!d->UpdatingItemFromSegment)
      {
      segment->SetNameAutoGenerated(false);
      }
    }
  else if (item->column() == this->statusColumn())
    {
    vtkSegment* segment = d->SegmentationNode->GetSegmentation()->GetSegment(segmentID.toStdString());
    if (!segment)
      {
      qCritical() << Q_FUNC_INFO << ": Segment with ID '" << segmentID << "' not found in segmentation node " << d->SegmentationNode->GetName();
      return;
      }
    int status = item->data(StatusRole).toInt();
    vtkSlicerSegmentationsModuleLogic::SetSegmentStatus(segment, status);
    }
  else
    {
    // For all other columns we need the display node
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
      d->SegmentationNode->GetDisplayNode());
    if (!displayNode)
      {
      qCritical() << Q_FUNC_INFO << ": No display node for segmentation!";
      return;
      }
    // Get display properties
    bool displayPropertyChanged = false;
    vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    displayNode->GetSegmentDisplayProperties(segmentID.toStdString(), properties);

    // Visibility column
    if (item->column() == this->visibilityColumn() && !item->data(VisibilityRole).isNull())
      {
      vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode());
      bool visible = item->data(VisibilityRole).toBool();
      displayNode->SetSegmentVisibility(segmentID.toStdString(), visible);
      }
    // Color column
    else if (item->column() == this->colorColumn())
      {
      vtkSegment* segment = d->SegmentationNode->GetSegmentation()->GetSegment(segmentID.toStdString());
      if (!segment)
        {
        qCritical() << Q_FUNC_INFO << ": Segment with ID '" << segmentID << "' not found in segmentation node " << d->SegmentationNode->GetName();
        return;
        }

      // Set terminology information to segment as tag
      QString terminologyString = item->data(qSlicerTerminologyItemDelegate::TerminologyRole).toString();
      segment->SetTag(vtkSegment::GetTerminologyEntryTagName(), terminologyString.toUtf8().constData());

      // Set color to segment if it changed
      QColor color = item->data(Qt::DecorationRole).value<QColor>();
      double* oldColorArray = segment->GetColor();
      QColor oldColor = QColor::fromRgbF(oldColorArray[0], oldColorArray[1], oldColorArray[2]);
      if (oldColor != color)
        {
        segment->SetColor(color.redF(), color.greenF(), color.blueF());
        }
      // Set color auto-generated flag
      segment->SetColorAutoGenerated(
        item->data(qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole).toBool());

      // Set name if it changed
      QString nameFromColorItem = item->data(qSlicerTerminologyItemDelegate::NameRole).toString();
      if (nameFromColorItem.compare(segment->GetName()))
        {
        emit segmentAboutToBeModified(segmentID);
        segment->SetName(nameFromColorItem.toUtf8().constData());
        }
      // Set name auto-generated flag
      segment->SetNameAutoGenerated(
        item->data(qSlicerTerminologyItemDelegate::NameAutoGeneratedRole).toBool());

      // Update tooltip
      item->setToolTip(qMRMLSegmentsTableView::terminologyTooltipForSegment(segment));
      }
    // Opacity changed
    else if (item->column() == this->opacityColumn())
      {
      QString opacity = item->data(Qt::EditRole).toString();
      QString currentOpacity = QString::number(properties.Opacity3D, 'f', 2);
      if (opacity != currentOpacity)
        {
        // Set to all kinds of opacities as they are combined on the UI
        properties.Opacity3D = opacity.toDouble();
        displayPropertyChanged = true;
        }
      }
    // Set changed properties to segmentation display node if a value has actually changed
    if (displayPropertyChanged)
      {
      displayNode->SetSegmentDisplayProperties(segmentID.toStdString(), properties);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::updateItemsFromColumnIndex(int column)
{
  Q_D(qMRMLSegmentsModel);
  if (column < 0)
    {
    return;
    }

  std::vector<std::string> segmentIDs;
  d->SegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
  for (std::string segmentID : segmentIDs)
    {
    QStandardItem* item = this->itemFromSegmentID(segmentID.c_str(), column);
    if (item)
      {
      this->updateItemFromSegment(item, segmentID.c_str(), column);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::updateItemsFromSegmentID(QString segmentID)
{
  Q_D(qMRMLSegmentsModel);

  QModelIndexList itemIndexes = this->indexes(segmentID);
  for (QModelIndex itemIndex : itemIndexes)
    {
    QStandardItem* item = this->itemFromIndex(itemIndex);
    this->updateItemFromSegment(item, segmentID, item->column());
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsModel::onEvent(
  vtkObject* caller, unsigned long event, void* clientData, void* callData )
{
  vtkMRMLSegmentationNode* segmentationNode = reinterpret_cast<vtkMRMLSegmentationNode*>(caller);
  qMRMLSegmentsModel* model = reinterpret_cast<qMRMLSegmentsModel*>(clientData);
  if (!model || !segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid event parameters";
    return;
    }

  // Get segment ID for segmentation node events
  QString segmentID;
  if (callData && (event == vtkSegmentation::SegmentAdded
                || event == vtkSegmentation::SegmentRemoved
                || event == vtkSegmentation::SegmentModified))
    {
    const char* segmentIDPtr = reinterpret_cast<const char*>(callData);
    if (segmentIDPtr)
      {
      segmentID = segmentIDPtr;
      }
    }

  switch (event)
    {
    case vtkSegmentation::SegmentAdded:
        model->onSegmentAdded(segmentID);
      break;
    case vtkSegmentation::SegmentRemoved:
        model->onSegmentRemoved(segmentID);
      break;
    case vtkSegmentation::SegmentModified:
      if (!segmentID.isEmpty())
        {
        model->onSegmentModified(segmentID);
        }
      else
        {
        model->updateFromSegments();
        }
      break;
    case vtkSegmentation::SegmentsOrderModified:
      model->onSegmentOrderModified();
      break;
    case vtkMRMLDisplayableNode::DisplayModifiedEvent:
      model->onDisplayNodeModified();
      break;
    case vtkMRMLSegmentationNode::SegmentationChangedEvent:
      model->rebuildFromSegments();
      break;
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::onSegmentAdded(QString segmentID)
{
  Q_D(qMRMLSegmentsModel);
  if (!segmentID.isEmpty())
    {
    d->insertSegment(segmentID);
    return;
    }

  std::vector<std::string> segmentIDs;
  d->SegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
  for (std::string currentSegmentID : segmentIDs)
    {
    QModelIndex index = this->indexFromSegmentID(currentSegmentID.c_str());
    if (index.isValid())
      {
      continue;
      }
    d->insertSegment(currentSegmentID.c_str());
    }
  this->updateItemsFromColumnIndex(this->layerColumn());
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::onSegmentRemoved(QString removedSegmentID)
{
  Q_D(qMRMLSegmentsModel);
  if (!removedSegmentID.isEmpty())
    {
    QModelIndex index = this->indexFromSegmentID(removedSegmentID);
    this->removeRow(index.row());
    return;
    }

  std::vector<std::string> segmentIDs;
  d->SegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);

  // Iterate in reverse so the index remains valid
  for (int i = this->rowCount()-1; i >= 0; --i)
    {
    QModelIndex index = this->index(i, 0);
    std::string currentSegmentID = this->segmentIDFromIndex(index).toStdString();
    std::vector<std::string>::iterator currentSegmentIt = std::find(segmentIDs.begin(), segmentIDs.end(), currentSegmentID);
    if (currentSegmentIt == segmentIDs.end())
      {
      this->removeRow(index.row());
      }
    }
  this->updateItemsFromColumnIndex(this->layerColumn());
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::onSegmentModified(QString segmentID)
{
  this->updateItemsFromSegmentID(segmentID);
  this->updateItemsFromColumnIndex(this->layerColumn());
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::onSegmentOrderModified()
{
  this->reorderItems();
  this->updateItemsFromColumnIndex(this->layerColumn());
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::onDisplayNodeModified()
{
  this->updateItemsFromColumnIndex(this->visibilityColumn());
  this->updateItemsFromColumnIndex(this->opacityColumn());
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::reorderItems()
{
  Q_D(qMRMLSegmentsModel);

  if (!d->SegmentationNode || !d->SegmentationNode->GetSegmentation())
    {
    return;
    }

  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return;
    }

  this->layoutAboutToBeChanged();
  bool wasBlocking = this->blockSignals(true);
  for (int i = 0; i < segmentation->GetNumberOfSegments(); ++i)
    {
    std::string segmentID = segmentation->GetNthSegmentID(i);
    QModelIndex index = this->indexFromSegmentID(segmentID.c_str());
    if (index.row() == i)
      {
      continue;
      }
    QList<QStandardItem*> items = this->takeRow(index.row());
    this->insertRow(i, items);
    }
  this->blockSignals(wasBlocking);
  this->layoutChanged();
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::onItemChanged(QStandardItem* item)
{
  Q_D(qMRMLSegmentsModel);
  this->updateSegmentFromItem(this->segmentIDFromItem(item), item);
}

//------------------------------------------------------------------------------
int qMRMLSegmentsModel::nameColumn()const
{
  Q_D(const qMRMLSegmentsModel);
  return d->NameColumn;
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::setNameColumn(int column)
{
  Q_D(qMRMLSegmentsModel);
  d->NameColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSegmentsModel::visibilityColumn()const
{
  Q_D(const qMRMLSegmentsModel);
  return d->VisibilityColumn;
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::setVisibilityColumn(int column)
{
  Q_D(qMRMLSegmentsModel);
  d->VisibilityColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSegmentsModel::colorColumn()const
{
  Q_D(const qMRMLSegmentsModel);
  return d->ColorColumn;
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::setColorColumn(int column)
{
  Q_D(qMRMLSegmentsModel);
  d->ColorColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSegmentsModel::opacityColumn()const
{
  Q_D(const qMRMLSegmentsModel);
  return d->OpacityColumn;
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::setOpacityColumn(int column)
{
  Q_D(qMRMLSegmentsModel);
  d->OpacityColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSegmentsModel::statusColumn()const
{
  Q_D(const qMRMLSegmentsModel);
  return d->StatusColumn;
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::setStatusColumn(int column)
{
  Q_D(qMRMLSegmentsModel);
  d->StatusColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSegmentsModel::layerColumn()const
{
  Q_D(const qMRMLSegmentsModel);
  return d->LayerColumn;
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::setLayerColumn(int column)
{
  Q_D(qMRMLSegmentsModel);
  d->LayerColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
void qMRMLSegmentsModel::updateColumnCount()
{
  Q_D(const qMRMLSegmentsModel);

  int max = this->maxColumnId();
  int oldColumnCount = this->columnCount();
  this->setColumnCount(max + 1);
  if (oldColumnCount == 0)
    {
    this->rebuildFromSegments();
    }
  else
    {
    // Update all items
    if (!d->SegmentationNode)
      {
      return;
      }
    std::vector<std::string> segmentIDs;
    d->SegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
    for (std::vector<std::string>::iterator itemIt= segmentIDs.begin(); itemIt!= segmentIDs.end(); ++itemIt)
      {
      this->updateItemsFromSegmentID(itemIt->c_str());
      }
    }
}

//------------------------------------------------------------------------------
int qMRMLSegmentsModel::maxColumnId()const
{
  Q_D(const qMRMLSegmentsModel);
  int maxId = -1;
  maxId = qMax(maxId, d->NameColumn);
  maxId = qMax(maxId, d->VisibilityColumn);
  maxId = qMax(maxId, d->ColorColumn);
  maxId = qMax(maxId, d->OpacityColumn);
  maxId = qMax(maxId, d->StatusColumn);
  return maxId;
}

// --------------------------------------------------------------------------
QString qMRMLSegmentsModel::terminologyTooltipForSegment(vtkSegment* segment)
{
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment";
    return QString();
    }

  // Get terminologies module logic
  vtkSlicerTerminologiesModuleLogic* terminologiesLogic = vtkSlicerTerminologiesModuleLogic::SafeDownCast(
    qSlicerCoreApplication::application()->moduleLogic("Terminologies"));
  if (!terminologiesLogic)
    {
    qCritical() << Q_FUNC_INFO << ": Terminologies logic is not found";
    return QString();
    }

  std::string serializedTerminology("");
  if (!segment->GetTag(vtkSegment::GetTerminologyEntryTagName(), serializedTerminology))
    {
    return QString("No terminology information");
    }
  vtkSmartPointer<vtkSlicerTerminologyEntry> terminologyEntry = vtkSmartPointer<vtkSlicerTerminologyEntry>::New();
  if (!terminologiesLogic->DeserializeTerminologyEntry(serializedTerminology, terminologyEntry))
    {
    return QString("Invalid terminology information");
    }

  return QString(terminologiesLogic->GetInfoStringFromTerminologyEntry(terminologyEntry).c_str());
}
