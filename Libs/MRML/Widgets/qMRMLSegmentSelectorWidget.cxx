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
#include "qMRMLSegmentSelectorWidget.h"

#include "ui_qMRMLSegmentSelectorWidget.h"

#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkSegmentation.h"
#include "vtkSegment.h"

// VYK includes
#include <vtkWeakPointer.h>

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
static const char* NONE_DISPLAY = "None";

//-----------------------------------------------------------------------------
class qMRMLSegmentSelectorWidgetPrivate: public Ui_qMRMLSegmentSelectorWidget
{
  Q_DECLARE_PUBLIC(qMRMLSegmentSelectorWidget);

protected:
  qMRMLSegmentSelectorWidget* const q_ptr;
public:
  qMRMLSegmentSelectorWidgetPrivate(qMRMLSegmentSelectorWidget& object);
  void init();

  /// Set message to display. If message is empty, then the label widget is hidden
  void setMessage(QString message);

public:
  /// Segmentation MRML node containing shown segments
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;

  /// Current segment ID in case of single selection
  QString CurrentSegmentID;
  /// Selected segment IDs in case of multi-selection
  QStringList SelectedSegmentIDs;

  /// Flag determining whether multi selection is enabled
  /// Note: Cannot tell from combobox visibility as in case of no segments or none segmentation
  ///       node selection, both are hidden
  bool MultiSelection;
};

//-----------------------------------------------------------------------------
qMRMLSegmentSelectorWidgetPrivate::qMRMLSegmentSelectorWidgetPrivate(qMRMLSegmentSelectorWidget& object)
  : q_ptr(&object)
  , SegmentationNode(nullptr)
  , MultiSelection(false)
{
}

//-----------------------------------------------------------------------------
void qMRMLSegmentSelectorWidgetPrivate::init()
{
  Q_Q(qMRMLSegmentSelectorWidget);
  this->setupUi(q);

  // Make connections
  QObject::connect( this->MRMLNodeComboBox_Segmentation, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(onCurrentNodeChanged(vtkMRMLNode*)) );
  QObject::connect( this->MRMLNodeComboBox_Segmentation, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SIGNAL(currentNodeChanged(vtkMRMLNode*)) );
  QObject::connect( this->MRMLNodeComboBox_Segmentation, SIGNAL(currentNodeChanged(bool)),
    q, SIGNAL(currentNodeChanged(bool)) );
  QObject::connect( this->MRMLNodeComboBox_Segmentation, SIGNAL(nodeAboutToBeEdited(vtkMRMLNode*)),
                q, SIGNAL(nodeAboutToBeEdited(vtkMRMLNode*)));

  QObject::connect( this->comboBox_Segment, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onCurrentSegmentChanged(int)) );
  QObject::connect( this->CheckableComboBox_Segment, SIGNAL(checkedIndexesChanged()),
    q, SLOT(onSegmentMultiSelectionChanged()) );

  // Hide multi-select combobox by default
  this->CheckableComboBox_Segment->setVisible(false);
}

//--------------------------------------------------------------------------
void qMRMLSegmentSelectorWidgetPrivate::setMessage(QString message)
{
  this->label_Message->setVisible(!message.isEmpty());
  this->label_Message->setText(message);
}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// qMRMLSegmentSelectorWidget methods

//-----------------------------------------------------------------------------
qMRMLSegmentSelectorWidget::qMRMLSegmentSelectorWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLSegmentSelectorWidgetPrivate(*this))
{
  Q_D(qMRMLSegmentSelectorWidget);
  d->init();
  this->populateSegmentCombobox();
}

//-----------------------------------------------------------------------------
qMRMLSegmentSelectorWidget::~qMRMLSegmentSelectorWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::onCurrentNodeChanged(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentSelectorWidget);

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  if (d->SegmentationNode != segmentationNode)
    {
    // Connect segment added/removed and display modified events to population of the table
    qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentAdded,
                   this, SLOT( populateSegmentCombobox() ) );
    qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentRemoved,
                   this, SLOT( populateSegmentCombobox() ) );
    qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentModified,
                   this, SLOT( populateSegmentCombobox() ) );

    d->SegmentationNode = segmentationNode;
    this->populateSegmentCombobox();
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setCurrentNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentSelectorWidget);
  d->MRMLNodeComboBox_Segmentation->setCurrentNode(node);
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSegmentSelectorWidget::currentNode()
{
  Q_D(qMRMLSegmentSelectorWidget);
  return d->MRMLNodeComboBox_Segmentation->currentNode();
}

//------------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setCurrentNodeID(const QString& nodeID)
{
  Q_D(qMRMLSegmentSelectorWidget);
  d->MRMLNodeComboBox_Segmentation->setCurrentNodeID(nodeID);
}

//------------------------------------------------------------------------------
QString qMRMLSegmentSelectorWidget::currentNodeID()
{
  Q_D(qMRMLSegmentSelectorWidget);
  return d->MRMLNodeComboBox_Segmentation->currentNodeID();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::populateSegmentCombobox()
{
  Q_D(qMRMLSegmentSelectorWidget);

  d->comboBox_Segment->clear();
  d->CheckableComboBox_Segment->clear();

  // Check if segmentation is valid and non-empty
  if (!d->SegmentationNode || !d->SegmentationNode->GetSegmentation() || d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() == 0)
    {
    d->setMessage(tr( d->SegmentationNode ? "No segments in segmentation" : "No node is selected" )+" ");

    d->label_Segment->setVisible(false);
    d->comboBox_Segment->setVisible(false);
    d->CheckableComboBox_Segment->setVisible(false);
    return;
    }

  d->label_Segment->setVisible(true);
  d->setMessage(QString());

  // Populate the proper combobox
  if (d->MultiSelection)
    {
    d->CheckableComboBox_Segment->setVisible(true);

    // Block signals so that onSegmentMultiSelectionChanged function is not called when populating
    bool wasBlocked = d->CheckableComboBox_Segment->blockSignals(true);

    // Add items for each segment
    vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
    std::vector< std::string > segmentIDs;
    segmentation->GetSegmentIDs(segmentIDs);
    for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
      QString segmentId(segmentIdIt->c_str());
      vtkSegment* segment = segmentation->GetSegment(*segmentIdIt);

      // Segment name
      QString name(segment->GetName());
      d->CheckableComboBox_Segment->addItem(name, QVariant(segmentId));
      }

    // Restore stored selection
    this->setSelectedSegmentIDs(d->SelectedSegmentIDs);

    // Unblock signals
    d->CheckableComboBox_Segment->blockSignals(wasBlocked);
    }
  else
    {
    d->comboBox_Segment->setVisible(true);

    // Block signals so that onCurrentSegmentChanged function is not called when populating
    bool wasBlocked = d->comboBox_Segment->blockSignals(true);

    // Add 'None' item if enabled
    if (this->noneEnabled())
      {
      d->comboBox_Segment->addItem(NONE_DISPLAY, QVariant(NONE_DISPLAY));
      }

    // Add items for each segment
    vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
    std::vector< std::string > segmentIDs;
    segmentation->GetSegmentIDs(segmentIDs);
    for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
      QString segmentId(segmentIdIt->c_str());
      // Segment name
      QString name(segmentation->GetSegment(*segmentIdIt)->GetName());
      d->comboBox_Segment->addItem(name, QVariant(segmentId));
      }

    // Set invalid selection so that callback function is called when first segment is selected later
    d->comboBox_Segment->setCurrentIndex(-1);

    // Unblock signals
    d->comboBox_Segment->blockSignals(wasBlocked);

    // Make sure fist segment is selected (we checked before that there is at least one segment)
    d->comboBox_Segment->setCurrentIndex(this->noneEnabled() ? 1 : 0);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::onCurrentSegmentChanged(int index)
{
  Q_D(qMRMLSegmentSelectorWidget);

  d->setMessage(QString());

  if (!d->SegmentationNode)
    {
    return;
    }

  // All items contain the segment ID, get that
  QString currentSegmentID = d->comboBox_Segment->itemData(index).toString();
  if (currentSegmentID.compare(NONE_DISPLAY))
    {
    d->CurrentSegmentID = currentSegmentID;
    }
  else // None was selected
    {
    d->CurrentSegmentID = QString();
    }

  emit currentSegmentChanged(d->CurrentSegmentID);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::onSegmentMultiSelectionChanged()
{
  Q_D(qMRMLSegmentSelectorWidget);

  d->setMessage(QString());

  if (!d->SegmentationNode)
    {
    return;
    }

  d->SelectedSegmentIDs.clear();
  foreach (QModelIndex index, d->CheckableComboBox_Segment->checkedIndexes())
    {
    d->SelectedSegmentIDs << d->CheckableComboBox_Segment->itemData(index.row()).toString();
    }

  emit segmentSelectionChanged(d->SelectedSegmentIDs);
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentSelectorWidget::currentSegmentID()
{
  Q_D(qMRMLSegmentSelectorWidget);

  if (d->MultiSelection)
    {
    qCritical() << Q_FUNC_INFO << ": Multi-selection is on, use selectedSegmentIDs instead";
    return QString();
    }

  return d->CurrentSegmentID;
}

//-----------------------------------------------------------------------------
QStringList qMRMLSegmentSelectorWidget::selectedSegmentIDs()
{
  Q_D(qMRMLSegmentSelectorWidget);

  if (!d->MultiSelection)
    {
    qCritical() << Q_FUNC_INFO << ": Multi-selection is off, use currentSegmentID instead";
    return QStringList();
    }

  return d->SelectedSegmentIDs;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setCurrentSegmentID(QString segmentID)
{
  Q_D(qMRMLSegmentSelectorWidget);

  if (d->MultiSelection)
    {
    qCritical() << Q_FUNC_INFO << ": Multi-selection is on, use setSelectedSegmentIDs instead";
    return;
    }

  if (segmentID.isEmpty() && this->noneEnabled())
    {
    d->comboBox_Segment->setCurrentIndex(0);
    return;
    }

  int index = d->comboBox_Segment->findData(QVariant(segmentID));
  if (index != -1 && d->comboBox_Segment->currentIndex() != index)
    {
    d->comboBox_Segment->setCurrentIndex(index);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setSelectedSegmentIDs(QStringList segmentIDList)
{
  Q_D(qMRMLSegmentSelectorWidget);

  if (!d->MultiSelection)
    {
    qCritical() << Q_FUNC_INFO << ": Multi-selection is off, use setCurrentSegmentID instead";
    return;
    }
  if (!d->SegmentationNode)
    {
    return;
    }

  // Make sure all the IDs in the list are valid
  QStringList invalidSegmentIDs;
  foreach (QString segmentID, segmentIDList)
    {
    if (!d->SegmentationNode->GetSegmentation()->GetSegment(segmentID.toUtf8().constData()))
      {
      // If no segment present with the current ID, then mark it for removal
      invalidSegmentIDs << segmentID;
      }
    }
  foreach (QString invalidID, invalidSegmentIDs)
    {
    segmentIDList.removeAll(invalidID);
    }

  d->SelectedSegmentIDs = segmentIDList;

  // Update checkbox states in checkable combobox
  bool wasBlocked = d->CheckableComboBox_Segment->blockSignals(true);
  for (int row=0; row<d->CheckableComboBox_Segment->model()->rowCount(); ++row)
    {
    QModelIndex index = d->CheckableComboBox_Segment->model()->index(row,0);
    QString segmentID = d->CheckableComboBox_Segment->itemData(row).toString();
    if (segmentIDList.contains(segmentID))
      {
      d->CheckableComboBox_Segment->setCheckState(index, Qt::Checked);
      }
    else
      {
      d->CheckableComboBox_Segment->setCheckState(index, Qt::Unchecked);
      }
    }
  d->CheckableComboBox_Segment->blockSignals(wasBlocked);
  d->CheckableComboBox_Segment->repaint();
}

//--------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setNoneEnabled(bool enable)
{
  Q_D(qMRMLSegmentSelectorWidget);
  d->MRMLNodeComboBox_Segmentation->setNoneEnabled(enable);

  // Re-populate combobox in case of single selection so that none selection is reflected for the segments too
  this->populateSegmentCombobox();
}

//--------------------------------------------------------------------------
bool qMRMLSegmentSelectorWidget::noneEnabled()const
{
  Q_D(const qMRMLSegmentSelectorWidget);
  return d->MRMLNodeComboBox_Segmentation->noneEnabled();
}

//--------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setEditEnabled(bool enable)
{
  Q_D(qMRMLSegmentSelectorWidget);
  d->MRMLNodeComboBox_Segmentation->setEditEnabled(enable);
}

//--------------------------------------------------------------------------
bool qMRMLSegmentSelectorWidget::editEnabled()const
{
  Q_D(const qMRMLSegmentSelectorWidget);
  return d->MRMLNodeComboBox_Segmentation->editEnabled();
}

//--------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setSegmentationNodeSelectorVisible(bool visible)
{
  Q_D(qMRMLSegmentSelectorWidget);
  d->MRMLNodeComboBox_Segmentation->setVisible(visible);
}

//--------------------------------------------------------------------------
bool qMRMLSegmentSelectorWidget::segmentationNodeSelectorVisible()const
{
  Q_D(const qMRMLSegmentSelectorWidget);
  return d->MRMLNodeComboBox_Segmentation->isVisible();
}

//--------------------------------------------------------------------------
bool qMRMLSegmentSelectorWidget::multiSelection()const
{
  Q_D(const qMRMLSegmentSelectorWidget);
  return d->MultiSelection;
}

//--------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setMultiSelection(bool multi)
{
  Q_D(qMRMLSegmentSelectorWidget);

  d->MultiSelection = multi;
  d->CheckableComboBox_Segment->setVisible(multi);
  d->comboBox_Segment->setVisible(!multi);

  // Invalidate the segment ID container that is not used
  if (multi)
    {
    d->CurrentSegmentID = QString();
    }
  else
    {
    d->SelectedSegmentIDs.clear();
    }

  // Re-populate comboboxes
  this->populateSegmentCombobox();
}

//--------------------------------------------------------------------------
bool qMRMLSegmentSelectorWidget::horizontalLayout()const
{
  Q_D(const qMRMLSegmentSelectorWidget);
  return ( d->gridLayout->rowCount() == 1 );
}

//--------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setHorizontalLayout(bool horizontal)
{
  Q_D(qMRMLSegmentSelectorWidget);

  d->gridLayout->takeAt(d->gridLayout->indexOf(d->frame_Segment));
  if (horizontal)
    {
    d->gridLayout->addWidget(d->frame_Segment, 0,1);
    d->label_Segment->setText("");
    d->MRMLNodeComboBox_Segmentation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->comboBox_Segment->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->CheckableComboBox_Segment->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }
  else
    {
    d->gridLayout->addWidget(d->frame_Segment, 1,0);
    d->label_Segment->setText("Segment: ");
    d->MRMLNodeComboBox_Segmentation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->comboBox_Segment->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->CheckableComboBox_Segment->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
}

//--------------------------------------------------------------------------
bool qMRMLSegmentSelectorWidget::selectNodeUponCreation()const
{
  Q_D(const qMRMLSegmentSelectorWidget);
  return d->MRMLNodeComboBox_Segmentation->selectNodeUponCreation();
}

//--------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setSelectNodeUponCreation(bool value)
{
  Q_D(qMRMLSegmentSelectorWidget);
  d->MRMLNodeComboBox_Segmentation->setSelectNodeUponCreation(value);
}
