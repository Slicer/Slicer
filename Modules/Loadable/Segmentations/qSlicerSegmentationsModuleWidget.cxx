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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations includes
#include "qSlicerSegmentationsModuleWidget.h"
#include "ui_qSlicerSegmentationsModule.h"

#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkSlicerSegmentationsModuleLogic.h"

#include "qMRMLSegmentsTableView.h"
#include "qMRMLSegmentationRepresentationsListView.h"

// SlicerQt includes
#include <qSlicerApplication.h>
#include <qSlicerAbstractModuleWidget.h>
#include <qSlicerSubjectHierarchyAbstractPlugin.h>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLLabelMapVolumeNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// VTK includes
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QMessageBox>

//-----------------------------------------------------------------------------
/// \ingroup SlicerRt_QtModules_Segmentations
class qSlicerSegmentationsModuleWidgetPrivate: public Ui_qSlicerSegmentationsModule
{
  Q_DECLARE_PUBLIC(qSlicerSegmentationsModuleWidget);
protected:
  qSlicerSegmentationsModuleWidget* const q_ptr;
public:
  qSlicerSegmentationsModuleWidgetPrivate(qSlicerSegmentationsModuleWidget& object);
  ~qSlicerSegmentationsModuleWidgetPrivate();
  vtkSlicerSegmentationsModuleLogic* logic() const;
public:
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;
  /// Using this flag prevents overriding the parameter set node contents when the
  ///   QMRMLCombobox selects the first instance of the specified node type when initializing
  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerSegmentationsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSegmentationsModuleWidgetPrivate::qSlicerSegmentationsModuleWidgetPrivate(qSlicerSegmentationsModuleWidget& object)
  : q_ptr(&object)
  , ModuleWindowInitialized(false)
{
}

//-----------------------------------------------------------------------------
qSlicerSegmentationsModuleWidgetPrivate::~qSlicerSegmentationsModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerSegmentationsModuleLogic*
qSlicerSegmentationsModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSegmentationsModuleWidget);
  return vtkSlicerSegmentationsModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerSegmentationsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSegmentationsModuleWidget::qSlicerSegmentationsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerSegmentationsModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerSegmentationsModuleWidget::~qSlicerSegmentationsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::enter()
{
  this->onEnter();
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::exit()
{
  this->Superclass::exit();

  // remove mrml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onEnter()
{
  if (!this->mrmlScene())
    {
    qCritical() << Q_FUNC_INFO << ": Invalid scene!";
    return;
    }

  Q_D(qSlicerSegmentationsModuleWidget);

  d->ModuleWindowInitialized = true;

  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndImportEvent,
                    this, SLOT(onMRMLSceneEndImportEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndBatchProcessEvent,
                    this, SLOT(onMRMLSceneEndBatchProcessEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent,
                    this, SLOT(onMRMLSceneEndCloseEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndRestoreEvent,
                    this, SLOT(onMRMLSceneEndRestoreEvent()));

  this->onSegmentationNodeChanged( d->MRMLNodeComboBox_Segmentation->currentNode() );
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::setup()
{
  this->init();
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode* qSlicerSegmentationsModuleWidget::segmentationDisplayNode(bool create/*=false*/)
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLSegmentationNode* segmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode() );
  if (!segmentationNode)
    {
    return NULL;
    }

  vtkMRMLSegmentationDisplayNode* displayNode =
    vtkMRMLSegmentationDisplayNode::SafeDownCast( segmentationNode->GetDisplayNode() );
  if (!displayNode && create)
    {
    segmentationNode->CreateDefaultDisplayNodes();
    displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast( segmentationNode->GetDisplayNode() );
    }
  return displayNode;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  // Don't update widget while there are pending operations.
  // (for example, we may create a new display node while a display node already exists, just the node
  // references have not been finalized yet)
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  // Hide the current node in the other segmentation combo box
  QStringList hiddenNodeIDs;
  if (d->SegmentationNode)
    {
    hiddenNodeIDs << QString(d->SegmentationNode->GetID());
    }
  d->MRMLNodeComboBox_OtherSegmentationOrRepresentationNode->sortFilterProxyModel()->setHiddenNodeIDs(hiddenNodeIDs);

  // Update display group from segmentation display node
  d->SegmentationDisplayNodeWidget->setSegmentationNode(d->SegmentationNode);
  d->SegmentationDisplayNodeWidget->updateWidgetFromMRML();

  // Update copy/move/import/export buttons from selection
  this->updateCopyMoveButtonStates();

  // Update segment handler button states based on segment selection
  this->onSegmentSelectionChanged(QItemSelection(),QItemSelection());
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::updateCopyMoveButtonStates()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  // Disable copy/move buttons then enable later based on selection
  d->toolButton_MoveFromCurrentSegmentation->setEnabled(false);
  d->toolButton_CopyFromCurrentSegmentation->setEnabled(false);
  d->toolButton_CopyToCurrentSegmentation->setEnabled(false);
  d->toolButton_MoveToCurrentSegmentation->setEnabled(false);

  // Set button states that copy/move to current segmentation
  vtkMRMLSegmentationNode* otherSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    d->SegmentsTableView_Other->segmentationNode() );
  vtkMRMLNode* otherRepresentationNode = d->SegmentsTableView_Other->representationNode();
  if (otherSegmentationNode)
    {
    // All options are possible if other node is segmentation
    d->toolButton_CopyToCurrentSegmentation->setEnabled(true);
    d->toolButton_MoveToCurrentSegmentation->setEnabled(true);
    }
  else if (otherRepresentationNode)
    {
    // Move from other node is disabled if other node is representation
    d->toolButton_CopyToCurrentSegmentation->setEnabled(true);
    }

  // Set button states that copy/move from current segmentation
  vtkMRMLSegmentationNode* currentSegmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode() );
  if (currentSegmentationNode && currentSegmentationNode->GetSegmentation()->GetNumberOfSegments() > 0)
    {
    d->toolButton_MoveFromCurrentSegmentation->setEnabled(true);
    d->toolButton_CopyFromCurrentSegmentation->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::init()
{
  Q_D(qSlicerSegmentationsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Ensure that four representations fit in the table by default
  d->RepresentationsListView->setMinimumHeight(108);

  // Set icons to tool buttons
  d->toolButton_AddLabelmap->setIcon(QIcon(":/Icons/AddLabelmap.png"));
  d->toolButton_AddModel->setIcon(QIcon(":/Icons/Small/SlicerAddModel.png"));

  // Make connections
  connect(d->MRMLNodeComboBox_Segmentation, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onSegmentationNodeChanged(vtkMRMLNode*)) );
  connect(d->MRMLNodeComboBox_Segmentation, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    d->SegmentsTableView, SLOT(setSegmentationNode(vtkMRMLNode*)) );
  connect(d->MRMLNodeComboBox_Segmentation, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    d->SegmentsTableView_Current, SLOT(setSegmentationNode(vtkMRMLNode*)) );
  connect(d->MRMLNodeComboBox_Segmentation, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    d->RepresentationsListView, SLOT(setSegmentationNode(vtkMRMLNode*)) );

  connect(d->SegmentsTableView, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
    this, SLOT(onSegmentSelectionChanged(QItemSelection,QItemSelection)));
  connect(d->pushButton_AddSegment, SIGNAL(clicked()),
    this, SLOT(onAddSegment()) );
  connect(d->pushButton_EditSelected, SIGNAL(clicked()),
    this, SLOT(onEditSelectedSegment()) );
  connect(d->pushButton_RemoveSelected, SIGNAL(clicked()),
    this, SLOT(onRemoveSelectedSegments()) );

  connect(d->MRMLNodeComboBox_OtherSegmentationOrRepresentationNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(setOtherSegmentationOrRepresentationNode(vtkMRMLNode*)) );

  connect(d->toolButton_AddLabelmap, SIGNAL(clicked()),
    this, SLOT(onAddLabelmap()) );
  connect(d->toolButton_AddModel, SIGNAL(clicked()),
    this, SLOT(onAddModel()) );

  connect(d->toolButton_MoveFromCurrentSegmentation, SIGNAL(clicked()),
    this, SLOT(onMoveFromCurrentSegmentation()) );
  connect(d->toolButton_CopyFromCurrentSegmentation, SIGNAL(clicked()),
    this, SLOT(onCopyFromCurrentSegmentation()) );
  connect(d->toolButton_CopyToCurrentSegmentation, SIGNAL(clicked()),
    this, SLOT(onCopyToCurrentSegmentation()) );
  connect(d->toolButton_MoveToCurrentSegmentation, SIGNAL(clicked()),
    this, SLOT(onMoveToCurrentSegmentation()) );

  // Show only segment names in copy/view segment list and make it non-editable
  d->SegmentsTableView_Current->setSelectionMode(QAbstractItemView::ExtendedSelection);
  d->SegmentsTableView_Current->setHeaderVisible(false);
  d->SegmentsTableView_Current->setVisibilityColumnVisible(false);
  d->SegmentsTableView_Current->setColorColumnVisible(false);
  d->SegmentsTableView_Current->setOpacityColumnVisible(false);

  d->SegmentsTableView_Other->setSelectionMode(QAbstractItemView::ExtendedSelection);
  d->SegmentsTableView_Other->setHeaderVisible(false);
  d->SegmentsTableView_Other->setVisibilityColumnVisible(false);
  d->SegmentsTableView_Other->setColorColumnVisible(false);
  d->SegmentsTableView_Other->setOpacityColumnVisible(false);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onSegmentationNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerSegmentationsModuleWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  if (!d->ModuleWindowInitialized)
    {
    return;
    }

  vtkMRMLSegmentationNode* segmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(node);

  qvtkReconnect(d->SegmentationNode, segmentationNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->SegmentationNode, segmentationNode, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::MasterRepresentationModified, this, SLOT(updateWidgetFromMRML()));

  d->SegmentationNode = segmentationNode;
  d->SegmentationDisplayNodeWidget->setSegmentationNode(segmentationNode);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::selectSegmentationNode(vtkMRMLSegmentationNode* segmentationNode)
{
  Q_D(qSlicerSegmentationsModuleWidget);

  d->MRMLNodeComboBox_Segmentation->setCurrentNode(segmentationNode);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);
  Q_D(qSlicerSegmentationsModuleWidget);

  d->pushButton_AddSegment->setEnabled(d->SegmentationNode != NULL);

  QStringList selectedSegmentIds = d->SegmentsTableView->selectedSegmentIDs();
  d->pushButton_EditSelected->setEnabled(selectedSegmentIds.count() == 1);
  d->pushButton_RemoveSelected->setEnabled(selectedSegmentIds.count() > 0);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onAddSegment()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLSegmentationNode* currentSegmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode() );
  if (!currentSegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": No segmentation selected!";
    return;
    }

  // Create empty segment in current segmentation
  std::string addedSegmentID = currentSegmentationNode->GetSegmentation()->AddEmptySegment();

  // Select the new segment
  if (!addedSegmentID.empty())
    {
    QStringList segmentIDList;
    segmentIDList << QString(addedSegmentID.c_str());
    d->SegmentsTableView->setSelectedSegmentIDs(segmentIDList);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onEditSelectedSegment()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  if ( !d->MRMLNodeComboBox_Segmentation->currentNode()
    || d->SegmentsTableView->selectedSegmentIDs().count() != 1 )
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment selection!";
    return;
    }
  QStringList segmentID;
  segmentID << d->SegmentsTableView->selectedSegmentIDs()[0];

  // Switch to Segment Editor module, select segmentation node and segment ID
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("SegmentEditor");
  if (!moduleWidget)
    {
    qCritical() << Q_FUNC_INFO << ": segment editor widget is not available";
    return;
    }
  // Get segmentation selector combobox and set segmentation
  qMRMLNodeComboBox* nodeSelector = moduleWidget->findChild<qMRMLNodeComboBox*>("MRMLNodeComboBox_Segmentation");
  if (!nodeSelector)
    {
    qCritical() << Q_FUNC_INFO << ": MRMLNodeComboBox_Segmentation is not found in segment editor widget";
    return;
    }
  nodeSelector->setCurrentNode(d->MRMLNodeComboBox_Segmentation->currentNode());

  // Get segments table and select segment
  qMRMLSegmentsTableView* segmentsTable = moduleWidget->findChild<qMRMLSegmentsTableView*>("SegmentsTableView");
  if (!segmentsTable)
    {
    qCritical() << Q_FUNC_INFO << ": SegmentsTableView is not found in segment editor widget";
    return;
    }
  segmentsTable->setSelectedSegmentIDs(segmentID);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onRemoveSelectedSegments()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLSegmentationNode* currentSegmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode() );
  if (!currentSegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": No segmentation selected!";
    return;
    }

  QStringList selectedSegmentIds = d->SegmentsTableView->selectedSegmentIDs();
  foreach (QString segmentId, selectedSegmentIds)
    {
    currentSegmentationNode->GetSegmentation()->RemoveSegment(segmentId.toLatin1().constData());
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::setOtherSegmentationOrRepresentationNode(vtkMRMLNode* node)
{
  Q_D(qSlicerSegmentationsModuleWidget);

  if (!this->mrmlScene())
    {
    qCritical() << Q_FUNC_INFO << ": Invalid scene!";
    return;
    }
  if (!d->ModuleWindowInitialized)
    {
    return;
    }

  // Decide if segmentation or representation node
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  vtkMRMLLabelMapVolumeNode* labelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(node);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);

  if (segmentationNode)
    {
    d->SegmentsTableView_Other->setSegmentationNode(node);
    }
  else if (labelmapNode || modelNode)
    {
    d->SegmentsTableView_Other->setRepresentationNode(node);
    }
  else
    {
    d->SegmentsTableView_Other->setSegmentationNode(NULL);
    d->SegmentsTableView_Other->setRepresentationNode(NULL);
    }

  // Update widgets based on selection
  this->updateCopyMoveButtonStates();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onAddLabelmap()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
    {
    return;
    }

  vtkSmartPointer<vtkMRMLLabelMapVolumeNode> labelmapNode = vtkSmartPointer<vtkMRMLLabelMapVolumeNode>::New();
  scene->AddNode(labelmapNode);

  // Select new labelmap in the other representation combobox
  d->MRMLNodeComboBox_OtherSegmentationOrRepresentationNode->setCurrentNode(labelmapNode);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onAddModel()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
    {
    return;
    }

  vtkSmartPointer<vtkMRMLModelNode> modelNode = vtkSmartPointer<vtkMRMLModelNode>::New();
  scene->AddNode(modelNode);

  // Select new model in the other representation table
  d->MRMLNodeComboBox_OtherSegmentationOrRepresentationNode->setCurrentNode(modelNode);
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentationsModuleWidget::copySegmentBetweenSegmentations(
  vtkSegmentation* fromSegmentation, vtkSegmentation* toSegmentation,
  QString segmentId, bool removeFromSource/*=false*/ )
{
  if (!fromSegmentation || !toSegmentation || segmentId.isEmpty())
    {
    return false;
    }

  std::string segmentIdStd(segmentId.toLatin1().constData());

  // Get segment
  vtkSegment* segment = fromSegmentation->GetSegment(segmentIdStd);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get segment!";
    return false;
    }

  // If target segmentation is empty, make it match the source
  if (toSegmentation->GetNumberOfSegments()==0)
    {
    toSegmentation->SetMasterRepresentationName(fromSegmentation->GetMasterRepresentationName());
    }

  // Check whether target is suitable to accept the segment.
  if (!toSegmentation->CanAcceptSegment(segment))
    {
    qCritical() << Q_FUNC_INFO << ": Segmentation cannot accept segment " << segment->GetName() << "!";

    // Pop up error message to the user explaining the problem
    vtkMRMLSegmentationNode* fromNode = vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentation(this->mrmlScene(), fromSegmentation);
    vtkMRMLSegmentationNode* toNode = vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentation(this->mrmlScene(), toSegmentation);
    if (!fromNode || !toNode) // Sanity check, should never happen
      {
      qCritical() << Q_FUNC_INFO << ": Unable to get parent nodes for segmentaiton objects!";
      return false;
      }

    QString message = QString("Cannot convert source master representation '%1' into target master '%2', "
      "thus unable to copy segment '%3' from segmentation '%4' to '%5'.\n\nWould you like to change the master representation of '%5' to '%1'?\n\n"
      "Note: This may result in unwanted data loss in %5.")
      .arg(fromSegmentation->GetMasterRepresentationName().c_str())
      .arg(toSegmentation->GetMasterRepresentationName().c_str()).arg(segmentId).arg(fromNode->GetName()).arg(toNode->GetName());
    QMessageBox::StandardButton answer =
      QMessageBox::question(NULL, tr("Failed to copy segment"), message,
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::Yes)
      {
      // Convert target segmentation to master representation of source segmentation
      bool successfulConversion = toSegmentation->CreateRepresentation(fromSegmentation->GetMasterRepresentationName());
      if (!successfulConversion)
        {
        QString message = QString("Failed to convert %1 to %2!").arg(toNode->GetName()).arg(fromSegmentation->GetMasterRepresentationName().c_str());
        QMessageBox::warning(NULL, tr("Conversion failed"), message);
        return false;
        }

      // Change master representation of target to that of source
      toSegmentation->SetMasterRepresentationName(fromSegmentation->GetMasterRepresentationName());

      // Retry copy of segment
      return this->copySegmentBetweenSegmentations(fromSegmentation, toSegmentation, segmentId, removeFromSource);
      }

    return false;
    }

  // Perform the actual copy/move operation
  return toSegmentation->CopySegmentFromSegmentation(fromSegmentation, segmentIdStd, removeFromSource);
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentationsModuleWidget::copyFromCurrentSegmentation(bool removeFromSource/*=false*/)
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLSegmentationNode* currentSegmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode() );
  if (!currentSegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": No segmentation selected!";
    return false;
    }

  vtkMRMLSegmentationNode* otherSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    d->SegmentsTableView_Other->segmentationNode() );
  vtkMRMLNode* otherRepresentationNode = d->SegmentsTableView_Other->representationNode();

  // Get selected segment IDs
  QStringList selectedSegmentIds = d->SegmentsTableView_Current->selectedSegmentIDs();
  if (selectedSegmentIds.empty())
    {
    return false;
    }

  // If other node is segmentation
  if (otherSegmentationNode)
    {
    // Copy/move selected segments into other segmentation
    foreach (QString segmentId, selectedSegmentIds)
      {
      this->copySegmentBetweenSegmentations(currentSegmentationNode->GetSegmentation(),
        otherSegmentationNode->GetSegmentation(), segmentId, removeFromSource);
      }
    }
  // If other node is representation
  else if (otherRepresentationNode)
    {
    if (selectedSegmentIds.count() == 1)
      {
      // Export segment into representation node
      QString firstSegmentId = selectedSegmentIds.first();
      vtkSegment* segment = currentSegmentationNode->GetSegmentation()->GetSegment(firstSegmentId.toLatin1().constData());
      if (!vtkSlicerSegmentationsModuleLogic::ExportSegmentToRepresentationNode(segment, otherRepresentationNode))
        {
        QString message = QString("Failed to export segment %1 from segmentation %2 to representation node %3!\n\nMost probably the segment cannot be converted into representation corresponding to the selected representation node.").
          arg(firstSegmentId).arg(currentSegmentationNode->GetName()).arg(otherRepresentationNode->GetName());
        qCritical() << Q_FUNC_INFO << ": " << message;
        QMessageBox::warning(NULL, tr("Failed to export segment"), message);
        return false;
        }
      else if (removeFromSource)
        {
        // Remove segment from current segmentation if export was successful
        currentSegmentationNode->GetSegmentation()->RemoveSegment(firstSegmentId.toLatin1().constData());
        }
      // Add representation node into the same subject hierarchy branch as the segmentation
      vtkMRMLSubjectHierarchyNode* currentSegmentationShNode = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(currentSegmentationNode);
      vtkMRMLSubjectHierarchyNode* otherRepresentationShNode = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(otherRepresentationNode);
      if (otherRepresentationShNode && currentSegmentationShNode->GetParentNodeID())
        {
        otherRepresentationShNode->SetParentNodeID(currentSegmentationShNode->GetParentNodeID());
        }
      }
    // Multiple selection is only allowed for exporting to labelmap volume
    else
      {
      if (!otherRepresentationNode->IsA("vtkMRMLLabelMapVolumeNode"))
        {
        QMessageBox::warning(NULL, tr("Failed to export multiple segments"), tr("More than one segment can only be exported into a labelmap volume node"));
        return false;
        }

      // Export selected segments into a multi-label labelmap volume
      std::vector<std::string> segmentIDs;
      foreach (QString segmentId, selectedSegmentIds)
        {
        segmentIDs.push_back(segmentId.toLatin1().constData());
        }
      vtkMRMLLabelMapVolumeNode* otherLabelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(otherRepresentationNode);
      if (!vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(currentSegmentationNode, segmentIDs, otherLabelmapNode))
        {
        QString message = QString("Failed to export segments from segmentation %1 to representation node %2!\n\nMost probably the segment cannot be converted into representation corresponding to the selected representation node.").
          arg(currentSegmentationNode->GetName()).arg(otherLabelmapNode->GetName());
        qCritical() << Q_FUNC_INFO << ": " << message;
        QMessageBox::warning(NULL, tr("Failed to export segment"), message);
        return false;
        }
      else if (removeFromSource)
        {
        // Remove segments from current segmentation if export was successful
        foreach (QString segmentId, selectedSegmentIds)
          {
          currentSegmentationNode->GetSegmentation()->RemoveSegment(segmentId.toLatin1().constData());
          }
        }
      }

    // Refresh other representation table
    d->SegmentsTableView_Other->setRepresentationNode(otherRepresentationNode);
    }

  return true;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onMoveFromCurrentSegmentation()
{
  this->copyFromCurrentSegmentation(true);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onCopyFromCurrentSegmentation()
{
  this->copyFromCurrentSegmentation();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onCopyToCurrentSegmentation()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLSegmentationNode* currentSegmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode() );
  if (!currentSegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": No segmentation selected!";
    return;
    }

  vtkMRMLSegmentationNode* otherSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    d->SegmentsTableView_Other->segmentationNode() );
  vtkMRMLNode* otherRepresentationNode = d->SegmentsTableView_Other->representationNode();

  // If other node is segmentation
  if (otherSegmentationNode)
    {
    // Copy other segments into current segmentation
    foreach (QString segmentId, d->SegmentsTableView_Other->selectedSegmentIDs())
      {
      this->copySegmentBetweenSegmentations(otherSegmentationNode->GetSegmentation(),
        currentSegmentationNode->GetSegmentation(), segmentId, false);
      }
    }
  // If other node is representation
  else if (otherRepresentationNode)
    {
    vtkMRMLLabelMapVolumeNode* labelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(otherRepresentationNode);
    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(otherRepresentationNode);

    if (labelmapNode)
      {
      if (this->updateMasterRepresentationInSegmentation(currentSegmentationNode->GetSegmentation(), vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
        {
        if (!vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(labelmapNode, currentSegmentationNode))
          {
          QString message = QString("Failed to copy labels from labelmap volume node %1!").arg(labelmapNode->GetName());
          qCritical() << Q_FUNC_INFO << ": " << message;
          QMessageBox::warning(NULL, tr("Failed to import from labelmap volume"), message);
          }
        }
      }
    else if (modelNode)
      {
      if (this->updateMasterRepresentationInSegmentation(currentSegmentationNode->GetSegmentation(),
        vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()))
        {
        if (!vtkSlicerSegmentationsModuleLogic::ImportModelToSegmentationNode(modelNode, currentSegmentationNode))
          {
          QString message = QString("Failed to copy polydata from model node %1!").arg(modelNode->GetName());
          qCritical() << Q_FUNC_INFO << ": " << message;
          QMessageBox::warning(NULL, tr("Failed to import model node"), message);
          }
        }
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Reprsentation node needs to be either model or labelmap, but " << otherRepresentationNode->GetName() << " is " << otherRepresentationNode->GetNodeTagName();
      return;
      }
    }
  currentSegmentationNode->CreateDefaultDisplayNodes();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onMoveToCurrentSegmentation()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLSegmentationNode* currentSegmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode() );
  if (!currentSegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": No segmentation selected!";
    return;
    }

  vtkMRMLSegmentationNode* otherSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    d->SegmentsTableView_Other->segmentationNode() );
  // If other node is segmentation
  if (otherSegmentationNode)
    {
    // Move other segment into current segmentation
    foreach (QString segmentId, d->SegmentsTableView_Other->selectedSegmentIDs())
      {
      this->copySegmentBetweenSegmentations(otherSegmentationNode->GetSegmentation(),
        currentSegmentationNode->GetSegmentation(), segmentId, true);
      }
    }
  // Only segment in segmentation can be moved into current segmentation
  else
    {
    qCritical() << Q_FUNC_INFO << ": Invalid operation!";
    }
  currentSegmentationNode->CreateDefaultDisplayNodes();
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentationsModuleWidget::updateMasterRepresentationInSegmentation(vtkSegmentation* segmentation, QString representation)
{
  if (!segmentation || representation.isEmpty())
    {
    return false;
    }
  std::string newMasterRepresentation(representation.toLatin1().constData());

  // Set master representation to the added one if segmentation is empty or master representation is undefined
  if (segmentation->GetNumberOfSegments() == 0)
    {
    segmentation->SetMasterRepresentationName(newMasterRepresentation);
    return true;
    }

  // No action is necessary if segmentation is non-empty and the master representation matches the contained one in segment
  if (segmentation->GetMasterRepresentationName() == newMasterRepresentation)
    {
    return true;
    }

  // Get segmentation node for segmentation
  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
    {
    return false;
    }
  vtkMRMLSegmentationNode* segmentationNode = vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentation(scene, segmentation);
  if (!segmentationNode)
    {
    return false;
    }

  // Ask the user if master was different but not empty
  QString message = QString("Segment is to be added in segmentation '%1' that contains a representation (%2) different than the master representation in the segmentation (%3). "
    "The master representation need to be changed so that the segment can be added. This might result in unwanted data loss.\n\n"
    "Do you wish to change the master representation to %2?")
    .arg(segmentationNode->GetName()).arg(newMasterRepresentation.c_str())
    .arg(segmentation->GetMasterRepresentationName().c_str());
  QMessageBox::StandardButton answer =
    QMessageBox::question(NULL, tr("Master representation is needed to be changed to add segment"), message,
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (answer == QMessageBox::No)
    {
    return false;
    }

  // Make sure the new master representation exists before setting it
  if (!segmentation->CreateRepresentation(newMasterRepresentation.c_str()))
    {
    std::vector<std::string> containedRepresentationNamesInSegmentation;
    segmentation->GetContainedRepresentationNames(containedRepresentationNamesInSegmentation);
    if (containedRepresentationNamesInSegmentation.empty())
      {
      qCritical() << Q_FUNC_INFO << ": Master representation cannot be created in segmentation as it does not contain any representations!";
      return false;
      }

    std::string firstContainedRepresentation = (*containedRepresentationNamesInSegmentation.begin());
    qCritical() << Q_FUNC_INFO << ": Master representation cannot be created in segmentation! Setting master to the first representation found: " << firstContainedRepresentation.c_str();
    segmentation->SetMasterRepresentationName(newMasterRepresentation.c_str());
    return false;
    }

  // Set master representation to the added one if user agreed
  segmentation->SetMasterRepresentationName(newMasterRepresentation.c_str());
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onMRMLSceneEndImportEvent()
{
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onMRMLSceneEndRestoreEvent()
{
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onMRMLSceneEndBatchProcessEvent()
{
  if (!this->mrmlScene())
      {
    return;
      }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onMRMLSceneEndCloseEvent()
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
      {
    return;
      }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentationsModuleWidget::setEditedNode(
    vtkMRMLNode* node,
    QString role/*=QString()*/,
    QString context/*=QString()*/)
{
  Q_D(qSlicerSegmentationsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkMRMLSegmentationNode::SafeDownCast(node))
      {
    d->MRMLNodeComboBox_Segmentation->setCurrentNode(node);
    return true;
      }
  return false;
}
