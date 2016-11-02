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

// Segmentations includes
#include "qMRMLSegmentsTableView.h"
#include "ui_qMRMLSegmentsTableView.h"

#include "qMRMLDoubleSpinBoxDelegate.h"

#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkSegmentation.h"
#include "vtkSegment.h"

// Terminologies includes
#include "qSlicerTerminologyItemDelegate.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLModelNode.h>

// VTK includes
#include <vtkWeakPointer.h>

// Qt includes
#include <QAction>
#include <QDebug>
#include <QKeyEvent>
#include <QStringList>
#include <QToolButton>
#include <QContextMenuEvent>
#include <QMenu>

// qMRML includes
#include "qMRMLItemDelegate.h"

#define ID_PROPERTY "ID"
#define VISIBILITY_PROPERTY "Visible"

//-----------------------------------------------------------------------------
class qMRMLSegmentsTableViewPrivate: public Ui_qMRMLSegmentsTableView
{
  Q_DECLARE_PUBLIC(qMRMLSegmentsTableView);

protected:
  qMRMLSegmentsTableView* const q_ptr;
public:
  qMRMLSegmentsTableViewPrivate(qMRMLSegmentsTableView& object);
  void init();

  /// Sets table message and takes care of the visibility of the label
  void setMessage(const QString& message);

  /// Return the column index for a given string, -1 if not a valid header
  int columnIndex(QString label);

  /// Find name item of row corresponding to a segment ID
  QTableWidgetItem* findItemBySegmentID(QString segmentID);

public:
  /// Segmentation MRML node containing shown segments
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;

  /// Flag determining whether the long-press per-view segment visibility options are available
  bool AdvancedSegmentVisibility;

  QIcon VisibleIcon;
  QIcon InvisibleIcon;

  // Currently, if we are requesting segment display information from the
  // segmentation display node,  the display node may emit modification events.
  // We make sure these events do not interrupt the update process by setting
  // IsUpdatingWidgetFromMRML to true when an update is already in progress.
  // TODO: When terminology infrastructure is in place then segmentation display
  // node should not invoke modification events on Get...() method calls and then
  // this flag can probably be removed.
  bool IsUpdatingWidgetFromMRML;

private:
  QStringList ColumnLabels;
};

//-----------------------------------------------------------------------------
qMRMLSegmentsTableViewPrivate::qMRMLSegmentsTableViewPrivate(qMRMLSegmentsTableView& object)
  : q_ptr(&object)
  , SegmentationNode(NULL)
  , AdvancedSegmentVisibility(false)
  , IsUpdatingWidgetFromMRML(false)
{
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableViewPrivate::init()
{
  Q_Q(qMRMLSegmentsTableView);
  this->setupUi(q);

  this->VisibleIcon = QIcon(":/Icons/Small/SlicerVisible.png");
  this->InvisibleIcon = QIcon(":/Icons/Small/SlicerInvisible.png");

  this->setMessage(QString());

  // Set table header properties
  this->ColumnLabels << "Visible" << "Color" << "Opacity" << "Name";
  this->SegmentsTable->setHorizontalHeaderLabels(
    QStringList() << "" << "Color" << "Opacity" << "Name" );
  this->SegmentsTable->setColumnCount(this->ColumnLabels.size());

  this->SegmentsTable->horizontalHeaderItem(
    this->columnIndex("Visible"))->setIcon(QIcon(":/Icons/Small/SlicerVisibleInvisible.png") );

  this->SegmentsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  this->SegmentsTable->horizontalHeader()->setStretchLastSection(1);

  // Select rows
  this->SegmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

  // Change edit triggers so that starting typing does not rename the segment
  this->SegmentsTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  // Make connections
  QObject::connect(this->SegmentsTable, SIGNAL(itemChanged(QTableWidgetItem*)),
                   q, SLOT(onSegmentTableItemChanged(QTableWidgetItem*)));
  QObject::connect(this->SegmentsTable->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                   q, SLOT(onSegmentSelectionChanged(QItemSelection,QItemSelection)));

  // Set item delegate to handle color and opacity changes
  qMRMLItemDelegate* itemDelegate = new qMRMLItemDelegate(this->SegmentsTable);
  //this->SegmentsTable->setItemDelegateForColumn(this->columnIndex("Color"), itemDelegate);
  this->SegmentsTable->setItemDelegateForColumn(this->columnIndex("Color"), new qSlicerTerminologyItemDelegate(this->SegmentsTable));
  this->SegmentsTable->setItemDelegateForColumn(this->columnIndex("Opacity"), itemDelegate);
  //this->SegmentsTable->setItemDelegateForColumn(this->columnIndex("Opacity"), new qMRMLDoubleSpinBoxDelegate(this->SegmentsTable));

  this->SegmentsTable->installEventFilter(q);

}

//-----------------------------------------------------------------------------
int qMRMLSegmentsTableViewPrivate::columnIndex(QString label)
{
  if (!this->ColumnLabels.contains(label))
    {
    qCritical() << Q_FUNC_INFO << ": Invalid column label!";
    return -1;
    }
  return this->ColumnLabels.indexOf(label);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableViewPrivate::setMessage(const QString& message)
{
  this->SegmentsTableMessageLabel->setVisible(!message.isEmpty());
  this->SegmentsTableMessageLabel->setText(message);
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qMRMLSegmentsTableViewPrivate::findItemBySegmentID(QString segmentID)
{
  Q_Q(qMRMLSegmentsTableView);
  for (int row=0; row<this->SegmentsTable->rowCount(); ++row)
    {
    QTableWidgetItem* item = this->SegmentsTable->item(row, this->columnIndex("Name"));
    if (!item)
      {
      continue;
      }
    if (!item->data(q->IDRole).toString().compare(segmentID))
      {
      return item;
      }
    }

  return NULL;
}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// qMRMLSegmentsTableView methods

//-----------------------------------------------------------------------------
qMRMLSegmentsTableView::qMRMLSegmentsTableView(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLSegmentsTableViewPrivate(*this))
{
  Q_D(qMRMLSegmentsTableView);
  d->init();
  this->populateSegmentTable();
}

//-----------------------------------------------------------------------------
qMRMLSegmentsTableView::~qMRMLSegmentsTableView()
{
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSegmentationNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentsTableView);

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);

  // Connect display modified event to population of the table
  qvtkReconnect( d->SegmentationNode, segmentationNode, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                 this, SLOT( updateWidgetFromMRML() ) );

  // Connect segment added/removed and display modified events to population of the table
  qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentAdded,
                 this, SLOT( populateSegmentTable() ) );
  qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentRemoved,
                 this, SLOT( populateSegmentTable() ) );
  qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentModified,
                 this, SLOT( updateWidgetFromMRML() ) );
  qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentsOrderModified,
                 this, SLOT(populateSegmentTable()));

  d->SegmentationNode = segmentationNode;
  this->populateSegmentTable();
}

//---------------------------------------------------------------------------
void qMRMLSegmentsTableView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSegmentsTableView);
  if (newScene == this->mrmlScene())
    {
    return;
    }

  this->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::EndBatchProcessEvent, this, SLOT(endProcessing()));

  if (d->SegmentationNode && newScene != d->SegmentationNode->GetScene())
    {
    this->setSegmentationNode(NULL);
    }

  Superclass::setMRMLScene(newScene);
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSegmentsTableView::segmentationNode()
{
  Q_D(qMRMLSegmentsTableView);

  return d->SegmentationNode;
}

//-----------------------------------------------------------------------------
QTableWidget* qMRMLSegmentsTableView::tableWidget()
{
  Q_D(qMRMLSegmentsTableView);
  return d->SegmentsTable;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::populateSegmentTable()
{
  Q_D(qMRMLSegmentsTableView);

  if (d->IsUpdatingWidgetFromMRML)
    {
    return;
    }
  d->IsUpdatingWidgetFromMRML = true;

  d->setMessage(QString());

  QStringList selectedSegmentIDs = this->selectedSegmentIDs();

  // Block signals so that onSegmentTableItemChanged function is not called when populating
  bool wasBlocked = d->SegmentsTable->blockSignals(true);

  // Clear table so that it can be populated
  d->SegmentsTable->clearContents();

  if (!d->SegmentationNode)
    {
    d->setMessage(tr("No node is selected"));
    d->SegmentsTable->setRowCount(0);
    d->SegmentsTable->blockSignals(wasBlocked);
    d->IsUpdatingWidgetFromMRML = false;
    emit selectionChanged(QItemSelection(), QItemSelection());
    return;
    }
  else if (d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() == 0)
    {
    d->setMessage(tr("Empty segmentation"));
    d->SegmentsTable->setRowCount(0);
    d->SegmentsTable->blockSignals(wasBlocked);
    d->IsUpdatingWidgetFromMRML = false;
    emit selectionChanged(QItemSelection(), QItemSelection());
    return;
    }

  // Get segmentation display node
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    d->SegmentationNode->GetDisplayNode() );

  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  std::vector< std::string > segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  d->SegmentsTable->setRowCount(segmentIDs.size());
  int row = 0;
  for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt, ++row)
  {
    QString segmentId(segmentIdIt->c_str());
    vtkSegment* segment = segmentation->GetSegment(*segmentIdIt);

    // Row height is smaller than default (which is 30)
    d->SegmentsTable->setRowHeight(row, 20);

    // Segment name
    QString name(segment->GetName());
    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
    nameItem->setData(IDRole, segmentId);
    d->SegmentsTable->setItem(row, d->columnIndex("Name"), nameItem);

    // Get segment display properties
    vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    if (displayNode)
      {
      displayNode->GetSegmentDisplayProperties(*segmentIdIt, properties);
      }

    // Visibility (show only 3D visibility; if the user changes it then it applies to all types of visibility)
    QToolButton* visibilityButton = new QToolButton();
    visibilityButton->setEnabled(displayNode != NULL);
    visibilityButton->setAutoRaise(true);
    visibilityButton->setToolTip("Set visibility for segment. Keep the button pressed for the advanced visibility options to show");
    visibilityButton->setProperty(ID_PROPERTY, segmentId);
    if (displayNode != NULL && properties.Visible && (properties.Visible3D || properties.Visible2DFill || properties.Visible2DOutline))
      {
      visibilityButton->setProperty(VISIBILITY_PROPERTY, true);
      visibilityButton->setIcon(d->VisibleIcon);
      }
    else
      {
      visibilityButton->setProperty(VISIBILITY_PROPERTY, false);
      visibilityButton->setIcon(d->InvisibleIcon);
      }
    d->SegmentsTable->setCellWidget(row, d->columnIndex("Visible"), visibilityButton);
    connect(visibilityButton, SIGNAL(clicked()), this, SLOT(onVisibilityButtonClicked()));

    // Set up actions for the visibility button if required
    if (d->AdvancedSegmentVisibility)
      {
      QAction* visibility3DAction = new QAction("Show in 3D", visibilityButton);
      visibility3DAction->setCheckable(true);
      visibility3DAction->setChecked(properties.Visible3D);
      visibility3DAction->setProperty(ID_PROPERTY, segmentId);
      connect(visibility3DAction, SIGNAL(triggered(bool)), this, SLOT(onVisibility3DActionToggled(bool)));
      visibilityButton->addAction(visibility3DAction);

      QAction* visibility2DFillAction = new QAction("Show in 2D as fill", visibilityButton);
      visibility2DFillAction->setCheckable(true);
      visibility2DFillAction->setChecked(properties.Visible2DFill);
      visibility2DFillAction->setProperty(ID_PROPERTY, segmentId);
      connect(visibility2DFillAction, SIGNAL(triggered(bool)), this, SLOT(onVisibility2DFillActionToggled(bool)));
      visibilityButton->addAction(visibility2DFillAction);

      QAction* visibility2DOutlineAction = new QAction("Show in 2D as outline", visibilityButton);
      visibility2DOutlineAction->setCheckable(true);
      visibility2DOutlineAction->setChecked(properties.Visible2DOutline);
      visibility2DOutlineAction->setProperty(ID_PROPERTY, segmentId);
      connect(visibility2DOutlineAction, SIGNAL(triggered(bool)), this, SLOT(onVisibility2DOutlineActionToggled(bool)));
      visibilityButton->addAction(visibility2DOutlineAction);
      }

    // Terminology / color
    QTableWidgetItem* colorItem = new QTableWidgetItem();
    QColor color = QColor::fromRgbF(properties.Color[0], properties.Color[1], properties.Color[2]);
    colorItem->setData(Qt::DecorationRole, color);
    colorItem->setData(Qt::EditRole, qMRMLSegmentsTableView::getCodeMeaningsFromTerminologySegmentTags(segment));
    colorItem->setData(IDRole, segmentId);
    colorItem->setToolTip(qMRMLSegmentsTableView::terminologyInfoStringForSegment(segment));
    d->SegmentsTable->setItem(row, d->columnIndex("Color"), colorItem);

    // Opacity (only 3D opacity - 2D outline and fill can be set in the display widget)
    QTableWidgetItem* opacityItem = new QTableWidgetItem();
    QString displayedOpacity = QString::number(properties.Opacity3D, 'f', 2);
    opacityItem->setData(Qt::EditRole, displayedOpacity); // for qMRMLItemDelegate
    //opacityItem->setData(Qt::EditRole, properties.Opacity3D); // for qMRMLDoubleSpinBoxDelegate
    opacityItem->setData(IDRole, segmentId);
    opacityItem->setToolTip("Opacity");
    d->SegmentsTable->setItem(row, d->columnIndex("Opacity"), opacityItem);
    }

  // Unblock signals
  this->setSelectedSegmentIDs(selectedSegmentIDs);
  d->SegmentsTable->blockSignals(wasBlocked);
  d->IsUpdatingWidgetFromMRML = false;
  emit selectionChanged(QItemSelection(), QItemSelection());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::updateWidgetFromMRML()
{
  Q_D(qMRMLSegmentsTableView);

  if (d->IsUpdatingWidgetFromMRML)
    {
    return;
    }

  if ( !d->SegmentationNode
    || d->SegmentsTable->rowCount() != d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() )
    {
    this->populateSegmentTable();
    return;
    }
  // Get segmentation display node
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    d->SegmentationNode->GetDisplayNode() );

  // Find items for each segment and update each field
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  std::vector< std::string > segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    QTableWidgetItem* nameItem = d->findItemBySegmentID(segmentIdIt->c_str());
    if (!nameItem)
      {
      qCritical() << Q_FUNC_INFO << ": Cannot find table item corresponding to segment ID '"
        << segmentIdIt->c_str() << "' in segmentation node " << d->SegmentationNode->GetName();
      continue;
      }
    int row = nameItem->row();

    // Name
    vtkSegment* segment = segmentation->GetSegment(*segmentIdIt);
    nameItem->setText(segment->GetName());

    // Get segment display properties
    vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    if (displayNode)
      {
      displayNode->GetSegmentDisplayProperties(*segmentIdIt, properties);
      }

    // Visibility
    QToolButton* visibilityButton = qobject_cast<QToolButton*>(
      d->SegmentsTable->cellWidget(row, d->columnIndex("Visible")) );
    if (visibilityButton)
      {
      visibilityButton->setEnabled(displayNode != NULL);
      if (displayNode != NULL && properties.Visible && (properties.Visible3D || properties.Visible2DFill || properties.Visible2DOutline))
        {
        visibilityButton->setProperty(VISIBILITY_PROPERTY, true);
        visibilityButton->setIcon(d->VisibleIcon);
        }
      else
        {
        visibilityButton->setProperty(VISIBILITY_PROPERTY, false);
        visibilityButton->setIcon(d->InvisibleIcon);
        }

      // Update actions if enabled
      if (d->AdvancedSegmentVisibility)
        {
        QList<QAction*> visibilityActions = visibilityButton->actions();
        visibilityActions[0]->setChecked(properties.Visible3D);
        visibilityActions[1]->setChecked(properties.Visible2DFill);
        visibilityActions[2]->setChecked(properties.Visible2DOutline);
        }
      }

    // Color
    QTableWidgetItem* colorItem = d->SegmentsTable->item(row, d->columnIndex("Color"));
    if (colorItem)
      {
      // Set terminology information from segment to item
      colorItem->setData(Qt::EditRole, qMRMLSegmentsTableView::getCodeMeaningsFromTerminologySegmentTags(segment));
      colorItem->setToolTip(qMRMLSegmentsTableView::terminologyInfoStringForSegment(segment));
      // Set color
      QColor color = QColor::fromRgbF(properties.Color[0], properties.Color[1], properties.Color[2]);
      colorItem->setData(Qt::DecorationRole, color);
      }

    // Opacity (show only 3D opacity; if the user changes it then it applies to all types of opacity)
    QTableWidgetItem* opacityItem =  d->SegmentsTable->item(row, d->columnIndex("Opacity"));
    if (opacityItem)
      {
      QString displayedOpacityStr = QString::number(properties.Opacity3D, 'f', 2);
      opacityItem->setData(Qt::EditRole, displayedOpacityStr); // for qMRMLItemDelegate
      //opacityItem->setData(Qt::EditRole, properties.Opacity3D); // for qMRMLDoubleSpinBoxDelegate
      }
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_D(qMRMLSegmentsTableView);
  if (d->SegmentsTable->signalsBlocked())
    {
    return;
    }
  emit selectionChanged(selected, deselected);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onSegmentTableItemChanged(QTableWidgetItem* changedItem)
{
  Q_D(qMRMLSegmentsTableView);

  d->setMessage(QString());

  if (!changedItem || !d->SegmentationNode)
    {
    return;
    }

  // All items contain the segment ID, get that
  QString segmentId = changedItem->data(IDRole).toString();

  // If segment name has been changed
  if (changedItem->column() == d->columnIndex("Name"))
    {
    QString nameText(changedItem->text());
    vtkSegment* segment = d->SegmentationNode->GetSegmentation()->GetSegment(segmentId.toLatin1().constData());
    if (!segment)
      {
      qCritical() << Q_FUNC_INFO << ": Segment with ID '" << segmentId << "' not found in segmentation node " << d->SegmentationNode->GetName();
      return;
      }
    emit segmentAboutToBeModified(segmentId);
    segment->SetName(nameText.toLatin1().constData());
    }
  // If visualization has been changed
  else
    {
    // For all other columns we need the display node
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
      d->SegmentationNode->GetDisplayNode() );
    if (!displayNode)
      {
      qCritical() << Q_FUNC_INFO << ": No display node for segmentation!";
      return;
      }
    // Get display properties
    vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    displayNode->GetSegmentDisplayProperties(segmentId.toLatin1().constData(), properties);

    bool displayPropertyChanged = false;

    // Color / terminology changed
    if (changedItem->column() == d->columnIndex("Color"))
      {
      QColor color = changedItem->data(Qt::DecorationRole).value<QColor>();
      QColor oldColor = QColor::fromRgbF(properties.Color[0], properties.Color[1], properties.Color[2]);
      if (oldColor != color)
        {
        properties.Color[0] = color.redF();
        properties.Color[1] = color.greenF();
        properties.Color[2] = color.blueF();
        displayPropertyChanged = true;
        }

      vtkSegment* segment = d->SegmentationNode->GetSegmentation()->GetSegment(segmentId.toLatin1().constData());
      if (!segment)
        {
        qCritical() << Q_FUNC_INFO << ": Segment with ID '" << segmentId << "' not found in segmentation node " << d->SegmentationNode->GetName();
        return;
        }
      QStringList terminologyCodeMeanings = changedItem->data(Qt::EditRole).toStringList();
      qMRMLSegmentsTableView::setTerminologySegmentTagsFromCodeMeanings(terminologyCodeMeanings, segment);
      }
    // Opacity changed
    else if (changedItem->column() == d->columnIndex("Opacity"))
      {
      QString opacity = changedItem->data(Qt::EditRole).toString();
      QString currentOpacity = QString::number( properties.Opacity3D, 'f', 2);
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
      displayNode->SetSegmentDisplayProperties(segmentId.toLatin1().constData(), properties);
      }
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onVisibilityButtonClicked()
{
  Q_D(qMRMLSegmentsTableView);

  QToolButton* senderButton = qobject_cast<QToolButton*>(sender());
  if (!senderButton)
    {
    return;
    }

  bool visible = !senderButton->property(VISIBILITY_PROPERTY).toBool();

  // Set all visibility types to segment referenced by button toggled
  this->setSegmentVisibility(senderButton, visible, -1, -1, -1);

  // Change button icon
  senderButton->setProperty(VISIBILITY_PROPERTY, visible);
  if (visible)
    {
    senderButton->setIcon(d->VisibleIcon);
    }
  else
    {
    senderButton->setIcon(d->InvisibleIcon);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onVisibility3DActionToggled(bool visible)
{
  QAction* senderAction = qobject_cast<QAction*>(sender());
  if (!senderAction)
    {
    return;
    }

  // Set 3D visibility to segment referenced by action toggled
  this->setSegmentVisibility(senderAction, -1, visible, -1, -1);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onVisibility2DFillActionToggled(bool visible)
{
  QAction* senderAction = qobject_cast<QAction*>(sender());
  if (!senderAction)
    {
    return;
    }

  // Set 2D fill visibility to segment referenced by action toggled
  this->setSegmentVisibility(senderAction, -1, -1, visible, -1);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onVisibility2DOutlineActionToggled(bool visible)
{
  QAction* senderAction = qobject_cast<QAction*>(sender());
  if (!senderAction)
    {
    return;
    }

  // Set 2D outline visibility to segment referenced by action toggled
  this->setSegmentVisibility(senderAction, -1, -1, -1, visible);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSegmentVisibility(QObject* senderObject, int visible, int visible3D, int visible2DFill, int visible2DOutline)
{
  Q_D(qMRMLSegmentsTableView);

  if (!d->SegmentationNode)
  {
    qCritical() << Q_FUNC_INFO << " failed: segmentation node is not set";
    return;
  }

  QString segmentId = senderObject->property(ID_PROPERTY).toString();

  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    d->SegmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation!";
    return;
    }
  vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
  displayNode->GetSegmentDisplayProperties(segmentId.toLatin1().constData(), properties);

  // Change visibility to all modes
  bool valueChanged = false;
  if (visible == 0 || visible == 1)
    {
    properties.Visible = (bool)visible;

    // If overall visibility is explicitly set to true then enable all visibility options
    // to make sure that something is actually visible.
    if (properties.Visible && !properties.Visible3D && !properties.Visible2DFill && !properties.Visible2DOutline)
      {
      properties.Visible3D = true;
      properties.Visible2DFill = true;
      properties.Visible2DOutline = true;
      }

    valueChanged = true;
    }
  if (visible3D == 0 || visible3D == 1)
    {
    properties.Visible3D = (bool)visible3D;
    valueChanged = true;
    }
  if (visible2DFill == 0 || visible2DFill == 1)
    {
    properties.Visible2DFill = (bool)visible2DFill;
    valueChanged = true;
    }
  if (visible2DOutline == 0 || visible2DOutline == 1)
    {
    properties.Visible2DOutline = (bool)visible2DOutline;
    valueChanged = true;
    }

  // Set visibility to display node
  if (valueChanged)
    {
    displayNode->SetSegmentDisplayProperties(segmentId.toLatin1().constData(), properties);
    }
}

//-----------------------------------------------------------------------------
int qMRMLSegmentsTableView::segmentCount() const
{
  Q_D(const qMRMLSegmentsTableView);

  return d->SegmentsTable->rowCount();
}

//-----------------------------------------------------------------------------
QStringList qMRMLSegmentsTableView::selectedSegmentIDs()
{
  Q_D(qMRMLSegmentsTableView);

  QList<QTableWidgetItem*> selectedItems = d->SegmentsTable->selectedItems();
  QStringList selectedSegmentIds;
  QSet<int> rows;
  foreach (QTableWidgetItem* item, selectedItems)
    {
    int row = item->row();
    if (!rows.contains(row))
      {
      rows.insert(row);
      selectedSegmentIds << item->data(IDRole).toString();
      }
    }

  return selectedSegmentIds;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSelectedSegmentIDs(QStringList segmentIDs)
{
  Q_D(qMRMLSegmentsTableView);

  if (!d->SegmentationNode && !segmentIDs.empty())
    {
    qCritical() << Q_FUNC_INFO << " failed: segmentation node is not set";
    return;
    }

  // Select items that have to be selected
  // (need to select first to avoid a transient state when nothing is selected)
  int columnCount = d->SegmentsTable->columnCount();
  for (int row = 0; row<d->SegmentsTable->rowCount(); ++row)
    {
    QTableWidgetItem* nameItem = d->SegmentsTable->item(row, d->columnIndex("Name"));
    if (!nameItem)
      {
      // invalid item, canot determine selection state
      continue;
      }
    if (!segmentIDs.contains(nameItem->data(IDRole).toString()))
      {
      // not selected
      continue;
      }
    // select all items in this row
    for (int column = 0; column < columnCount; column++)
      {
      QModelIndex index = d->SegmentsTable->model()->index(row, column);
      d->SegmentsTable->selectionModel()->select(index, QItemSelectionModel::Select);
      }
    }

  // Deselect items that don't have to be selected anymore
  for (int row = 0; row<d->SegmentsTable->rowCount(); ++row)
    {
    QTableWidgetItem* nameItem = d->SegmentsTable->item(row, d->columnIndex("Name"));
    if (!nameItem)
      {
      // invalid item, canot determine selection state
      continue;
      }
    if (segmentIDs.contains(nameItem->data(IDRole).toString()))
      {
      // selected
      continue;
      }
    // unselect all items in this row
    for (int column = 0; column < columnCount; column++)
      {
      QModelIndex index = d->SegmentsTable->model()->index(row, column);
      d->SegmentsTable->selectionModel()->select(index, QItemSelectionModel::Deselect);
      }
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::clearSelection()
{
  Q_D(qMRMLSegmentsTableView);

  // Deselect selected items first
  QList<QTableWidgetItem*> selectedItems = d->SegmentsTable->selectedItems();
  foreach (QTableWidgetItem* item, selectedItems)
    {
    d->SegmentsTable->setItemSelected(item, false);
    }
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::eventFilter(QObject* target, QEvent* event)
{
  Q_D(qMRMLSegmentsTableView);
  if (target == d->SegmentsTable)
  {
    // Prevent giving the focus to the previous/next widget if arrow keys are used
    // at the edge of the table (without this: if the current cell is in the top
    // row and user press the Up key, the focus goes from the table to the previous
    // widget in the tab order)
    if (event->type() == QEvent::KeyPress)
    {
      QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);
      QAbstractItemModel* model = d->SegmentsTable->model();
      QModelIndex currentIndex = d->SegmentsTable->currentIndex();

      if (model && (
        (keyEvent->key() == Qt::Key_Left && currentIndex.column() == 0)
        || (keyEvent->key() == Qt::Key_Up && currentIndex.row() == 0)
        || (keyEvent->key() == Qt::Key_Right && currentIndex.column() == model->columnCount() - 1)
        || (keyEvent->key() == Qt::Key_Down && currentIndex.row() == model->rowCount() - 1)))
      {
        return true;
      }
    }
  }
  return this->QWidget::eventFilter(target, event);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::endProcessing()
{
  this->populateSegmentTable();
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSelectionMode(int mode)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->setSelectionMode(static_cast<QAbstractItemView::SelectionMode>(mode));
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setHeaderVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->horizontalHeader()->setVisible(visible);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setVisibilityColumnVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->columnIndex("Visible"), !visible);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setColorColumnVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->columnIndex("Color"), !visible);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setOpacityColumnVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->columnIndex("Opacity"), !visible);
}

//------------------------------------------------------------------------------
int qMRMLSegmentsTableView::selectionMode()
{
  Q_D(qMRMLSegmentsTableView);
  return d->SegmentsTable->selectionMode();
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::headerVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return d->SegmentsTable->horizontalHeader()->isVisible();
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::visibilityColumnVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->columnIndex("Visible"));
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::colorColumnVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->columnIndex("Color"));
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::opacityColumnVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->columnIndex("Opacity"));
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::contextMenuEvent(QContextMenuEvent* event)
{
  QMenu* contextMenu = new QMenu(this);

  QAction* showOnlySelectedAction = new QAction("Show only selected segments", this);
  QObject::connect(showOnlySelectedAction, SIGNAL(triggered()), this, SLOT(showOnlySelectedSegments()));
  contextMenu->addAction(showOnlySelectedAction);

  contextMenu->popup(event->globalPos());
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::showOnlySelectedSegments()
{
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": No segment selected";
    return;
    }

  Q_D(qMRMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": No current segmentation node";
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    d->SegmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation " << d->SegmentationNode->GetName();
    return;
    }

  // Hide all segments except the selected ones
  int disabledModify = displayNode->StartModify();
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  std::vector< std::string > segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
  {
    bool visible = false;
    if (selectedSegmentIDs.contains(segmentIdIt->c_str()))
      {
      visible = true;
      }

    displayNode->SetSegmentVisibility(*segmentIdIt, visible);
    }
  displayNode->EndModify(disabledModify);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setTerminologySegmentTagsFromCodeMeanings(QStringList codeMeanings, vtkSegment* segment)
{
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment given";
    return;
    }

  int index=0;
  foreach (QString string, codeMeanings) // Do it in a loop so that it exits at last element
    {
    switch (index)
      {
      case 0:
        segment->SetTag(vtkSegment::GetTerminologyContextTagName(), string.toLatin1().constData());
        break;
      case 1:
        segment->SetTag(vtkSegment::GetTerminologyCategoryTagName(), string.toLatin1().constData());
        break;
      case 2:
        segment->SetTag(vtkSegment::GetTerminologyTypeTagName(), string.toLatin1().constData());
        break;
      case 3:
        segment->SetTag(vtkSegment::GetTerminologyTypeModifierTagName(), string.toLatin1().constData());
        break;
      case 4:
        segment->SetTag(vtkSegment::GetAnatomicContextTagName(), string.toLatin1().constData());
        break;
      case 5:
        segment->SetTag(vtkSegment::GetAnatomicRegionTagName(), string.toLatin1().constData());
        break;
      case 6:
        segment->SetTag(vtkSegment::GetAnatomicRegionModifierTagName(), string.toLatin1().constData());
        break;
      }
    ++index;
    }
}

//------------------------------------------------------------------------------
QStringList qMRMLSegmentsTableView::getCodeMeaningsFromTerminologySegmentTags(vtkSegment* segment)
{
  QStringList codeMeanings;
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment given";
    return codeMeanings;
    }

  codeMeanings.clear();
  std::string tagValue("");

  codeMeanings << ( segment->GetTag(vtkSegment::GetTerminologyContextTagName(), tagValue) ? tagValue.c_str() : "" );
  codeMeanings << ( segment->GetTag(vtkSegment::GetTerminologyCategoryTagName(), tagValue) ? tagValue.c_str() : "" );
  codeMeanings << ( segment->GetTag(vtkSegment::GetTerminologyTypeTagName(), tagValue) ? tagValue.c_str() : "" );
  codeMeanings << ( segment->GetTag(vtkSegment::GetTerminologyTypeModifierTagName(), tagValue) ? tagValue.c_str() : "" );
  codeMeanings << ( segment->GetTag(vtkSegment::GetAnatomicContextTagName(), tagValue) ? tagValue.c_str() : "" );
  codeMeanings << ( segment->GetTag(vtkSegment::GetAnatomicRegionTagName(), tagValue) ? tagValue.c_str() : "" );
  codeMeanings << ( segment->GetTag(vtkSegment::GetAnatomicRegionModifierTagName(), tagValue) ? tagValue.c_str() : "" );
  return codeMeanings;
}

// --------------------------------------------------------------------------
QString qMRMLSegmentsTableView::terminologyInfoStringForSegment(vtkSegment* segment)
{
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment given";
    return QString();
    }

  std::string tagValue("");
  QString terminology = QString("Terminology:");
  if (segment->GetTag(vtkSegment::GetTerminologyContextTagName(), tagValue))
    {
    terminology = terminology + QString("\n  Context: %1").arg(tagValue.c_str());
    }
  else
    {
    // Definitely invalid terminology
    return QString("No terminology information");
    }

  if (segment->GetTag(vtkSegment::GetTerminologyCategoryTagName(), tagValue))
    {
    terminology = terminology + QString("\n  Category: %1").arg(tagValue.c_str());
    }
  else
    {
    terminology = terminology + QString("\n  Category: NONE");
    }
  if (segment->GetTag(vtkSegment::GetTerminologyTypeTagName(), tagValue))
    {
    terminology = terminology + QString("\n  Type: %1").arg(tagValue.c_str());
    }
  else
    {
    terminology = terminology + QString("\n  Type: NONE");
    }
  if (segment->GetTag(vtkSegment::GetTerminologyTypeModifierTagName(), tagValue) && !tagValue.empty())
    {
    terminology = terminology + QString("\n    Modifier: %1").arg(tagValue.c_str());
    }

  // If anatomic region is not selected, then do not show anatomic context name either
  std::string anatomicRegion("");
  bool anatomicRegionTagPresent = segment->GetTag(vtkSegment::GetAnatomicRegionTagName(), anatomicRegion);
  if (!anatomicRegion.empty() && segment->GetTag(vtkSegment::GetAnatomicContextTagName(), tagValue))
    {
    terminology = terminology + QString("\n  Anatomic context: %1").arg(tagValue.c_str());
    }
  if (anatomicRegionTagPresent && !anatomicRegion.empty())
    {
    terminology = terminology + QString("\n  Anatomic region: %1").arg(anatomicRegion.c_str());
    }
  if (segment->GetTag(vtkSegment::GetAnatomicRegionModifierTagName(), tagValue) && !tagValue.empty())
    {
    terminology = terminology + QString("\n    Modifier: %1").arg(tagValue.c_str());
    }

  return terminology;
}
