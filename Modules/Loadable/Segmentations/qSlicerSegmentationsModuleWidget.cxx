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
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
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

  QButtonGroup* ImportExportOperationButtonGroup; // import/export
  QButtonGroup* ImportExportTypeButtonGroup; // model/labelmap
};

//-----------------------------------------------------------------------------
// qSlicerSegmentationsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSegmentationsModuleWidgetPrivate::qSlicerSegmentationsModuleWidgetPrivate(qSlicerSegmentationsModuleWidget& object)
  : q_ptr(&object)
  , ModuleWindowInitialized(false)
  , ImportExportOperationButtonGroup(NULL)
  , ImportExportTypeButtonGroup(NULL)
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
  if (otherSegmentationNode)
    {
    // All options are possible if other node is segmentation
    d->toolButton_CopyToCurrentSegmentation->setEnabled(true);
    d->toolButton_MoveToCurrentSegmentation->setEnabled(true);
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

  d->ImportExportOperationButtonGroup = new QButtonGroup(d->CollapsibleButton_ImportExportSegment);
  d->ImportExportOperationButtonGroup->addButton(d->radioButton_Export);
  d->ImportExportOperationButtonGroup->addButton(d->radioButton_Import);

  d->ImportExportTypeButtonGroup = new QButtonGroup(d->CollapsibleButton_ImportExportSegment);
  d->ImportExportTypeButtonGroup->addButton(d->radioButton_Labelmap);
  d->ImportExportTypeButtonGroup->addButton(d->radioButton_Model);

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

  this->connect(d->ImportExportOperationButtonGroup,
    SIGNAL(buttonClicked(QAbstractButton*)),
    SLOT(onImportExportOptionsButtonClicked()));

  this->connect(d->ImportExportTypeButtonGroup,
    SIGNAL(buttonClicked(QAbstractButton*)),
    SLOT(onImportExportOptionsButtonClicked()));

  connect(d->PushButton_ImportExport, SIGNAL(clicked()),
    this, SLOT(onImportExportApply()));

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

  d->radioButton_Export->setChecked(true);
  d->radioButton_Labelmap->setChecked(true);
  this->onImportExportOptionsButtonClicked();
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
  if (segmentationNode)
    {
    // Make sure display node exists
    segmentationNode->CreateDefaultDisplayNodes();
    }

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
  d->SegmentsTableView_Other->setSegmentationNode(segmentationNode);

  // Update widgets based on selection
  this->updateCopyMoveButtonStates();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onImportExportOptionsButtonClicked()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  // Operation: export/import
  if (d->radioButton_Export->isChecked())
    {
    d->label_ImportExportType->setText("Output type:");
    d->label_ImportExportNode->setText("Output node:");
    d->PushButton_ImportExport->setText("Export");
    }
  else
    {
    d->label_ImportExportType->setText("Input type:");
    d->label_ImportExportNode->setText("Input node:");
    d->PushButton_ImportExport->setText("Import");
    }
  d->MRMLNodeComboBox_ImportExportNode->setNoneEnabled(d->radioButton_Export->isChecked());
  d->ComboBox_ExportedSegments->setEnabled(d->radioButton_Export->isChecked());
  d->MRMLNodeComboBox_ExportLabelmapReferenceVolume->setEnabled(
    d->radioButton_Labelmap->isChecked() && d->radioButton_Export->isChecked());

  // Type: labelmap/model
  QStringList nodeTypes;
  if (d->radioButton_Labelmap->isChecked())
    {
    nodeTypes << "vtkMRMLLabelMapVolumeNode";
    if (d->radioButton_Export->isChecked())
      {
      d->MRMLNodeComboBox_ImportExportNode->setNoneDisplay("Export to new labelmap");
      }
    }
  else
    {
    nodeTypes << "vtkMRMLModelHierarchyNode";
    if (d->radioButton_Export->isChecked())
      {
      d->MRMLNodeComboBox_ImportExportNode->setNoneDisplay("Export to new model hierarchy");
      }
    else
      {
      // Import is supported from an individual model node as well
      nodeTypes << "vtkMRMLModelNode";
      }
    }
  d->MRMLNodeComboBox_ImportExportNode->setNodeTypes(nodeTypes);
  d->MRMLNodeComboBox_ExportLabelmapReferenceVolume->setEnabled(
    d->radioButton_Labelmap->isChecked() && d->radioButton_Export->isChecked());
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onImportExportApply()
{
  Q_D(qSlicerSegmentationsModuleWidget);
  if (d->radioButton_Export->isChecked())
    {
    this->exportFromCurrentSegmentation();
    }
  else
    {
    this->importToCurrentSegmentation();
    }
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
bool qSlicerSegmentationsModuleWidget::copySegmentsBetweenSegmentations(bool copyFromCurrentSegmentation, bool removeFromSource/*=false*/)
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLSegmentationNode* currentSegmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode() );
  if (!currentSegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": No current segmentation is selected";
    return false;
    }

  vtkMRMLSegmentationNode* otherSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    d->SegmentsTableView_Other->segmentationNode() );
  if (!otherSegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": No other segmentation is selected";
    return false;
    }

  // Get source and target segmentation
  QStringList selectedSegmentIds;
  vtkSegmentation* sourceSegmentation = NULL;
  vtkSegmentation* targetSegmentation = NULL;
  if (copyFromCurrentSegmentation)
    {
    sourceSegmentation = currentSegmentationNode->GetSegmentation();
    targetSegmentation = otherSegmentationNode->GetSegmentation();
    otherSegmentationNode->CreateDefaultDisplayNodes();
    selectedSegmentIds = d->SegmentsTableView_Current->selectedSegmentIDs();
    }
  else
    {
    sourceSegmentation = otherSegmentationNode->GetSegmentation();
    targetSegmentation = currentSegmentationNode->GetSegmentation();
    currentSegmentationNode->CreateDefaultDisplayNodes();
    selectedSegmentIds = d->SegmentsTableView_Other->selectedSegmentIDs();
    }

  if (selectedSegmentIds.empty())
    {
    qWarning() << Q_FUNC_INFO << ": No segments are selected";
    return false;
    }

  // Copy/move segments
  bool success = true;
  foreach(QString segmentId, selectedSegmentIds)
    {
    success = success && this->copySegmentBetweenSegmentations(sourceSegmentation,
      targetSegmentation, segmentId, removeFromSource);
    }

  return success;
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentationsModuleWidget::exportFromCurrentSegmentation()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLSegmentationNode* currentSegmentationNode =  vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode() );
  if (!currentSegmentationNode || !currentSegmentationNode->GetSegmentation())
    {
    qWarning() << Q_FUNC_INFO << ": No segmentation selected!";
    return false;
    }

  // If existing node was not selected then create a new one that we will export into
  vtkMRMLNode* otherRepresentationNode = d->MRMLNodeComboBox_ImportExportNode->currentNode();
  if (!otherRepresentationNode)
    {
    std::string namePostfix;
    if (d->radioButton_Labelmap->isChecked())
      {
      vtkSmartPointer<vtkMRMLNode> newNode = vtkSmartPointer<vtkMRMLNode>::Take(
        currentSegmentationNode->GetScene()->CreateNodeByClass("vtkMRMLLabelMapVolumeNode"));
      vtkMRMLLabelMapVolumeNode* newLabelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(
        currentSegmentationNode->GetScene()->AddNode(newNode));
      newLabelmapNode->CreateDefaultDisplayNodes();
      otherRepresentationNode = newLabelmapNode;
      namePostfix = "-label";
      }
    else
      {
      vtkSmartPointer<vtkMRMLNode> newNode = vtkSmartPointer<vtkMRMLNode>::Take(
        currentSegmentationNode->GetScene()->CreateNodeByClass("vtkMRMLModelHierarchyNode"));
      vtkMRMLModelHierarchyNode* newModelHierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(
        currentSegmentationNode->GetScene()->AddNode(newNode));
      otherRepresentationNode = newModelHierarchyNode;
      namePostfix = "-models";
      }
    std::string exportedNodeName = std::string(currentSegmentationNode->GetName() ? currentSegmentationNode->GetName() : "Unknown") + namePostfix;
    exportedNodeName = currentSegmentationNode->GetScene()->GetUniqueNameByString(exportedNodeName.c_str());
    otherRepresentationNode->SetName(exportedNodeName.c_str());
    d->MRMLNodeComboBox_ImportExportNode->setCurrentNode(otherRepresentationNode);
    }

  // Get IDs of segments to be exported
  std::vector<std::string> segmentIDs;
  if (d->ComboBox_ExportedSegments->currentIndex() == 0)
    {
    // all segments
    currentSegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
    }
  else
    {
    // visible segments
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(currentSegmentationNode->GetDisplayNode());
    displayNode->GetVisibleSegmentIDs(segmentIDs);
    }

  vtkMRMLLabelMapVolumeNode* labelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(otherRepresentationNode);
  vtkMRMLModelHierarchyNode* modelHierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(otherRepresentationNode);
  if (labelmapNode)
    {
    // Export selected segments into a multi-label labelmap volume
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    bool success = vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(currentSegmentationNode, segmentIDs, labelmapNode,
      vtkMRMLVolumeNode::SafeDownCast(d->MRMLNodeComboBox_ExportLabelmapReferenceVolume->currentNode()));
    QApplication::restoreOverrideCursor();
    if (!success)
      {
      QString message = QString("Failed to export segments from segmentation %1 to representation node %2!\n\n"
        "Most probably the segment cannot be converted into representation corresponding to the selected representation node.").
        arg(currentSegmentationNode->GetName()).arg(labelmapNode->GetName());
      qCritical() << Q_FUNC_INFO << ": " << message;
      QMessageBox::warning(NULL, tr("Failed to export segment"), message);
      return false;
      }
    }
  else if (modelHierarchyNode)
    {
    // Create a map that can be used for quickly looking up existing models in a hierarchy
    vtkNew<vtkCollection> existingModels;
    modelHierarchyNode->GetChildrenModelNodes(existingModels.GetPointer());
    std::map< std::string, vtkMRMLModelNode* > existingModelNamesToModels;
    vtkObject* object = NULL;
    vtkCollectionSimpleIterator it;
    for (existingModels->InitTraversal(it); (object = existingModels->GetNextItemAsObject(it));)
      {
      vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(object);
      if (!modelNode)
        {
        continue;
        }
      existingModelNamesToModels[modelNode->GetName()] = modelNode;
      }

    // Get subject hierarchy item for segmentation node
    vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(currentSegmentationNode->GetScene());
    if (!shNode)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return false;
      }
    vtkIdType currentSegmentationShItemID = shNode->GetItemByDataNode(currentSegmentationNode);
    if (currentSegmentationShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find subject hierarchy item for segmentation node " << currentSegmentationNode->GetName();
      return false;
      }

    // Export each segment into a model
    QString errorMessage;
    for (std::vector<std::string>::iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
      {
      // Export segment into model node
      vtkSegment* segment = currentSegmentationNode->GetSegmentation()->GetSegment(*segmentIdIt);
      vtkMRMLModelNode* modelNode = NULL;
      if (existingModelNamesToModels.find(segment->GetName()) != existingModelNamesToModels.end())
        {
        // Model by the same name exists in the selected hierarchy, overwrite that model
        modelNode = existingModelNamesToModels[segment->GetName()];
        }
      else
        {
        // Create new model node
        vtkNew<vtkMRMLModelNode> newModelNode;
        modelHierarchyNode->GetScene()->AddNode(newModelNode.GetPointer());
        newModelNode->CreateDefaultDisplayNodes();
        modelNode = newModelNode.GetPointer();
        // Add to model hierarchy
        vtkNew<vtkMRMLModelHierarchyNode> newModelHierarchyNode;
        newModelHierarchyNode->SetHideFromEditors(true);
        newModelHierarchyNode->SetAssociatedNodeID(modelNode->GetID());
        newModelHierarchyNode->SetParentNodeID(modelHierarchyNode->GetID());
        modelHierarchyNode->GetScene()->AddNode(newModelHierarchyNode.GetPointer());
        }
      QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
      if (!vtkSlicerSegmentationsModuleLogic::ExportSegmentToRepresentationNode(segment, modelNode))
        {
        if (errorMessage.isEmpty())
          {
          errorMessage = QString("Failed to export segment(s) to model:");
          }
        errorMessage.append(" ");
        errorMessage.append(modelNode->GetName());
        }
      QApplication::restoreOverrideCursor();

      // Add representation node into the same subject hierarchy branch as the segmentation
      vtkIdType otherRepresentationShItemID = shNode->GetItemByDataNode(modelNode);
      if (otherRepresentationShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
        {
        qCritical() << Q_FUNC_INFO << ": Failed to find subject hierarchy item for node " << otherRepresentationNode->GetName();
        continue;
        }
      shNode->SetItemParent(otherRepresentationShItemID, shNode->GetItemParent(currentSegmentationShItemID));
      }
    if (!errorMessage.isEmpty())
      {
      qCritical() << Q_FUNC_INFO << ": " << errorMessage;
      QMessageBox::warning(NULL, tr("Failed to export segment"), errorMessage);
      return false;
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentationsModuleWidget::importToCurrentSegmentation()
{
  Q_D(qSlicerSegmentationsModuleWidget);

  vtkMRMLSegmentationNode* currentSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    d->MRMLNodeComboBox_Segmentation->currentNode());
  if (!currentSegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": No segmentation selected";
    return false;
    }
  currentSegmentationNode->CreateDefaultDisplayNodes();

  vtkMRMLNode* otherRepresentationNode = d->MRMLNodeComboBox_ImportExportNode->currentNode();
  if (!otherRepresentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": No other node is selected";
    return false;
    }
  vtkMRMLLabelMapVolumeNode* labelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(otherRepresentationNode);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(otherRepresentationNode);
  vtkMRMLModelHierarchyNode* modelHierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(otherRepresentationNode);

  if (labelmapNode)
    {
    if (this->updateMasterRepresentationInSegmentation(currentSegmentationNode->GetSegmentation(),
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
      {
      QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
      bool success = vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(labelmapNode, currentSegmentationNode);
      QApplication::restoreOverrideCursor();
      if (!success)
        {
        QString message = QString("Failed to copy labels from labelmap volume node %1!").arg(labelmapNode->GetName());
        qCritical() << Q_FUNC_INFO << ": " << message;
        QMessageBox::warning(NULL, tr("Failed to import from labelmap volume"), message);
        return false;
        }
      }
    }
  else if (modelNode || modelHierarchyNode)
    {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    if (this->updateMasterRepresentationInSegmentation(currentSegmentationNode->GetSegmentation(),
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()))
      {
      vtkNew<vtkCollection> modelNodes;
      if (modelNode)
        {
        modelNodes->AddItem(modelNode);
        }
      else if (modelHierarchyNode)
        {
        modelHierarchyNode->GetChildrenModelNodes(modelNodes.GetPointer());
        }
      QString errorMessage;
      vtkObject* object = NULL;
      vtkCollectionSimpleIterator it;
      for (modelNodes->InitTraversal(it); (object = modelNodes->GetNextItemAsObject(it));)
        {
        modelNode = vtkMRMLModelNode::SafeDownCast(object);
        if (!modelNode)
          {
          continue;
          }
        // TODO: look up segment with matching name and overwrite that
        if (!vtkSlicerSegmentationsModuleLogic::ImportModelToSegmentationNode(modelNode, currentSegmentationNode))
          {
          if (errorMessage.isEmpty())
            {
            errorMessage = tr("Failed to copy polydata from model node:");
            }
          errorMessage.append(" ");
          errorMessage.append(QString(modelNode->GetName() ? modelNode->GetName() : "(unknown)"));
          }
        }
      if (!errorMessage.isEmpty())
        {
        qCritical() << Q_FUNC_INFO << ": " << errorMessage;
        QMessageBox::warning(NULL, tr("Failed to import model node"), errorMessage);
        }
      }
    QApplication::restoreOverrideCursor();
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": Reprsentation node needs to be either model, model hierarchy, or labelmap, but "
      << otherRepresentationNode->GetName() << " is " << otherRepresentationNode->GetNodeTagName();
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onMoveFromCurrentSegmentation()
{
  this->copySegmentsBetweenSegmentations(true, true);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onCopyFromCurrentSegmentation()
{
  this->copySegmentsBetweenSegmentations(true, false);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onCopyToCurrentSegmentation()
{
  this->copySegmentsBetweenSegmentations(false, false);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsModuleWidget::onMoveToCurrentSegmentation()
{
  this->copySegmentsBetweenSegmentations(false, true);
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
