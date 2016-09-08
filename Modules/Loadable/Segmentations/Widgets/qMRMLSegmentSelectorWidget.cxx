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

  /// Selected segment ID
  QString SelectedSegmentID;
};

//-----------------------------------------------------------------------------
qMRMLSegmentSelectorWidgetPrivate::qMRMLSegmentSelectorWidgetPrivate(qMRMLSegmentSelectorWidget& object)
  : q_ptr(&object)
{
  this->SegmentationNode = NULL;
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
  QObject::connect(this->comboBox_Segment, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onCurrentSegmentChanged(int)));
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
qMRMLSegmentSelectorWidget::~qMRMLSegmentSelectorWidget()
{
}

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
  d->setMessage(QString());

  // Block signals so that onCurrentSegmentChanged function is not called when populating
  d->comboBox_Segment->blockSignals(true);

  // If not representation, then segmentation must be selected. Check validity
  if (!d->SegmentationNode)
      {
    d->setMessage(tr("No node is selected"));
    d->comboBox_Segment->setVisible(false);
    d->label_Segment->setVisible(false);
    d->comboBox_Segment->blockSignals(false);
    return;
      }
  else if (d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() == 0)
      {
    d->setMessage(tr("No segments in segmentation"));
    d->comboBox_Segment->setVisible(false);
    d->label_Segment->setVisible(false);
    d->comboBox_Segment->blockSignals(false);
    return;
      }

  // Show segment selector combobox if there are segments in the segmentation
  d->comboBox_Segment->setVisible(true);
  d->label_Segment->setVisible(true);

  // Add 'None' item if enabled
  if (this->noneEnabled())
    {
    d->comboBox_Segment->addItem(NONE_DISPLAY);
    }

  // Add items for each segment
  vtkSegmentation::SegmentMap segmentMap = d->SegmentationNode->GetSegmentation()->GetSegments();
  for (vtkSegmentation::SegmentMap::iterator segmentIt = segmentMap.begin(); segmentIt != segmentMap.end(); ++segmentIt)
    {
    QString segmentId(segmentIt->first.c_str());

    // Segment name
    QString name(segmentIt->second->GetName());
    d->comboBox_Segment->addItem(name, QVariant(segmentId));
    }

  // Set invalid selection so that callback function is called when first segment is selected later
  d->comboBox_Segment->setCurrentIndex(-1);

  // Unblock signals
  d->comboBox_Segment->blockSignals(false);

  // Make sure fist segment is selected (we checked before that there is at least one segment)
  d->comboBox_Segment->setCurrentIndex(this->noneEnabled() ? 1 : 0);
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
  QString selectedSegmentText = d->comboBox_Segment->itemData(index).toString();
  if (selectedSegmentText.compare(NONE_DISPLAY))
    {
    d->SelectedSegmentID = selectedSegmentText;
    }
  else // None was selected
    {
    d->SelectedSegmentID = QString();
    }

  emit currentSegmentChanged(d->SelectedSegmentID);
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentSelectorWidget::currentSegmentID()
{
  Q_D(qMRMLSegmentSelectorWidget);

  return d->SelectedSegmentID;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setCurrentSegmentID(QString segmentID)
{
  Q_D(qMRMLSegmentSelectorWidget);

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

//--------------------------------------------------------------------------
void qMRMLSegmentSelectorWidget::setNoneEnabled(bool enable)
{
  Q_D(qMRMLSegmentSelectorWidget);
  d->MRMLNodeComboBox_Segmentation->setNoneEnabled(enable);
}

//--------------------------------------------------------------------------
bool qMRMLSegmentSelectorWidget::noneEnabled()const
{
  Q_D(const qMRMLSegmentSelectorWidget);
  return d->MRMLNodeComboBox_Segmentation->noneEnabled();
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
