/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QCheckBox>
#include <QDebug>
#include <QListWidgetItem>
#include <QtPlugin>

// Slicer includes
#include "qMRMLSequenceBrowserToolBar.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerSequencesModuleWidget.h"
#include "qSlicerSequencesModule.h"
#include "ui_qSlicerSequencesModuleWidget.h"

// MRML includes
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLTransformNode.h"

// Sequence includes
#include "vtkSlicerSequencesLogic.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceBrowserNode.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkFloatArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPlot.h>
#include <vtkTable.h>
#include <vtkWeakPointer.h>

enum
{
  SYNCH_NODES_NAME_COLUMN = 0,
  SYNCH_NODES_PROXY_COLUMN,
  SYNCH_NODES_PLAYBACK_COLUMN,
  SYNCH_NODES_RECORDING_COLUMN,
  SYNCH_NODES_OVERWRITE_PROXY_NAME_COLUMN,
  SYNCH_NODES_SAVE_CHANGES_COLUMN,
  SYNCH_NODES_NUMBER_OF_COLUMNS // this must be the last line in this enum
};

//-----------------------------------------------------------------------------
class qSlicerSequencesModuleWidgetPrivate : public Ui_qSlicerSequencesModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerSequencesModuleWidget);

protected:
  qSlicerSequencesModuleWidget* const q_ptr;

public:
  qSlicerSequencesModuleWidgetPrivate(qSlicerSequencesModuleWidget& object);
  ~qSlicerSequencesModuleWidgetPrivate();

  vtkSlicerSequencesLogic* logic() const;

  void init();
  void resetInteractiveCharting();
  void updateInteractiveCharting();
  void setAndObserveCrosshairNode();

  qMRMLSequenceBrowserToolBar* toolBar();

  /// Using this flag prevents overriding the parameter set node contents when the
  ///   QMRMLCombobox selects the first instance of the specified node type when initializing
  bool ModuleWindowInitialized;

  vtkWeakPointer<vtkMRMLSequenceBrowserNode> ActiveBrowserNode;

  vtkChartXY* ChartXY;
  vtkTable* ChartTable;
  vtkFloatArray* ArrayX;
  vtkFloatArray* ArrayY1;
  vtkFloatArray* ArrayY2;
  vtkFloatArray* ArrayY3;

  vtkWeakPointer<vtkMRMLCrosshairNode> CrosshairNode;

  QStringList SupportedProxyNodeTypes;
};

//-----------------------------------------------------------------------------
// qSlicerSequencesModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSequencesModuleWidgetPrivate::qSlicerSequencesModuleWidgetPrivate(qSlicerSequencesModuleWidget& object)
  : q_ptr(&object)
  , ModuleWindowInitialized(false)
  , ChartXY(0)
  , ChartTable(0)
  , ArrayX(0)
  , ArrayY1(0)
  , ArrayY2(0)
  , ArrayY3(0)
{
}

//-----------------------------------------------------------------------------
qSlicerSequencesModuleWidgetPrivate::~qSlicerSequencesModuleWidgetPrivate()
{
  if (this->ChartTable)
  {
    this->ChartTable->Delete();
  }
  if (this->ArrayX)
  {
    this->ArrayX->Delete();
  }
  if (this->ArrayY1)
  {
    this->ArrayY1->Delete();
  }
  if (this->ArrayY2)
  {
    this->ArrayY2->Delete();
  }
  if (this->ArrayY3)
  {
    this->ArrayY3->Delete();
  }
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidgetPrivate::setAndObserveCrosshairNode()
{
  Q_Q(qSlicerSequencesModuleWidget);

  vtkMRMLCrosshairNode* crosshairNode = 0;
  if (q->mrmlScene())
  {
    crosshairNode = vtkMRMLCrosshairNode::SafeDownCast(q->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLCrosshairNode"));
  }

  q->qvtkReconnect(this->CrosshairNode.GetPointer(),
                   crosshairNode,
                   vtkMRMLCrosshairNode::CursorPositionModifiedEvent,
                   q,
                   SLOT(updateChart()));
  this->CrosshairNode = crosshairNode;
}

//-----------------------------------------------------------------------------
vtkSlicerSequencesLogic* qSlicerSequencesModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSequencesModuleWidget);
  return vtkSlicerSequencesLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qMRMLSequenceBrowserToolBar* qSlicerSequencesModuleWidgetPrivate::toolBar()
{
  Q_Q(const qSlicerSequencesModuleWidget);
  qSlicerSequencesModule* module = dynamic_cast<qSlicerSequencesModule*>(q->module());
  if (!module)
  {
    qWarning("qSlicerSequencesModuleWidget::toolBar failed: module is not set");
    return nullptr;
  }
  return module->toolBar();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidgetPrivate::init()
{
  this->ChartXY = this->ChartView_iCharting->chart();
  this->ChartTable = vtkTable::New();
  this->ArrayX = vtkFloatArray::New();
  this->ArrayY1 = vtkFloatArray::New();
  this->ArrayY2 = vtkFloatArray::New();
  this->ArrayY3 = vtkFloatArray::New();
  this->ArrayX->SetName("X axis");
  this->ArrayY1->SetName("Y1 axis");
  this->ArrayY2->SetName("Y2 axis");
  this->ArrayY3->SetName("Y3 axis");
  this->ChartTable->AddColumn(this->ArrayX);
  this->ChartTable->AddColumn(this->ArrayY1);
  this->ChartTable->AddColumn(this->ArrayY2);
  this->ChartTable->AddColumn(this->ArrayY3);

  this->resetInteractiveCharting();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidgetPrivate::resetInteractiveCharting()
{
  this->ChartXY->RemovePlot(0);
  this->ChartXY->RemovePlot(0);
  this->ChartXY->RemovePlot(0);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidgetPrivate::updateInteractiveCharting()
{
  Q_Q(qSlicerSequencesModuleWidget);

  if (this->CrosshairNode.GetPointer() == nullptr)
  {
    qWarning()
      << "qSlicerSequencesModuleWidgetPrivate::updateInteractiveCharting failed: crosshair node is not available";
    resetInteractiveCharting();
    return;
  }
  vtkMRMLSequenceNode* sequenceNode =
    this->ActiveBrowserNode ? this->ActiveBrowserNode->GetMasterSequenceNode() : nullptr;
  if (sequenceNode == nullptr)
  {
    resetInteractiveCharting();
    return;
  }
  double croshairPosition_RAS[4] = { 0, 0, 0, 1 }; // homogeneous coordinate to allow transform by matrix multiplication
  bool validPosition = this->CrosshairNode->GetCursorPositionRAS(croshairPosition_RAS);
  if (!validPosition)
  {
    resetInteractiveCharting();
    return;
  }
  vtkMRMLNode* proxyNode = this->ActiveBrowserNode->GetProxyNode(sequenceNode);
  vtkMRMLTransformableNode* transformableProxyNode = vtkMRMLTransformableNode::SafeDownCast(proxyNode);

  int numberOfDataNodes = sequenceNode->GetNumberOfDataNodes();
  this->ChartTable->SetNumberOfRows(numberOfDataNodes);

  vtkMRMLScalarVolumeNode* vNode = vtkMRMLScalarVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(0));
  if (vNode)
  {
    int numOfScalarComponents = 0;
    numOfScalarComponents = vNode->GetImageData()->GetNumberOfScalarComponents();
    if (numOfScalarComponents > 3)
    {
      return;
    }
    vtkNew<vtkGeneralTransform> worldTransform;
    worldTransform->Identity();
    vtkMRMLTransformNode* transformNode =
      transformableProxyNode ? transformableProxyNode->GetParentTransformNode() : nullptr;
    if (transformNode)
    {
      transformNode->GetTransformFromWorld(worldTransform.GetPointer());
    }

    int numberOfValidPoints = 0;
    for (int i = 0; i < numberOfDataNodes; i++)
    {
      vNode = vtkMRMLScalarVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(i));
      this->ChartTable->SetValue(i, 0, i);

      vtkNew<vtkGeneralTransform> worldToIjkTransform;
      worldToIjkTransform->PostMultiply();
      worldToIjkTransform->Identity();
      vtkNew<vtkMatrix4x4> rasToIjkMatrix;
      vNode->GetRASToIJKMatrix(rasToIjkMatrix.GetPointer());
      worldToIjkTransform->Concatenate(rasToIjkMatrix.GetPointer());
      worldToIjkTransform->Concatenate(worldTransform.GetPointer());

      double* crosshairPositionDouble_IJK = worldToIjkTransform->TransformDoublePoint(croshairPosition_RAS);
      int croshairPosition_IJK[3] = { vtkMath::Round(crosshairPositionDouble_IJK[0]),
                                      vtkMath::Round(crosshairPositionDouble_IJK[1]),
                                      vtkMath::Round(crosshairPositionDouble_IJK[2]) };
      int* imageExtent = vNode->GetImageData()->GetExtent();
      bool isCrosshairInsideImage =
        imageExtent[0] <= croshairPosition_IJK[0] && croshairPosition_IJK[0] <= imageExtent[1]
        && imageExtent[2] <= croshairPosition_IJK[1] && croshairPosition_IJK[1] <= imageExtent[3]
        && imageExtent[4] <= croshairPosition_IJK[2] && croshairPosition_IJK[2] <= imageExtent[5];
      if (isCrosshairInsideImage)
      {
        numberOfValidPoints++;
      }
      for (int c = 0; c < numOfScalarComponents; c++)
      {
        double val = isCrosshairInsideImage ? vNode->GetImageData()->GetScalarComponentAsDouble(
                       croshairPosition_IJK[0], croshairPosition_IJK[1], croshairPosition_IJK[2], c)
                                            : 0;
        this->ChartTable->SetValue(i, c + 1, val);
      }
    }
    // this->ChartTable->Update();
    this->ChartXY->RemovePlot(0);
    this->ChartXY->RemovePlot(0);
    this->ChartXY->RemovePlot(0);

    if (numberOfValidPoints > 0)
    {
      this->ChartXY->GetAxis(0)->SetTitle(qSlicerSequencesModuleWidget::tr("Signal Intensity").toStdString());
      this->ChartXY->GetAxis(1)->SetTitle(qSlicerSequencesModuleWidget::tr("Time").toStdString());
      for (int c = 0; c < numOfScalarComponents; c++)
      {
        vtkPlot* line = this->ChartXY->AddPlot(vtkChart::LINE);
        line->SetInputData(this->ChartTable, 0, c + 1);
        // line->SetColor(255,0,0,255);
      }
    }
  }

  vtkMRMLTransformNode* tNode = vtkMRMLTransformNode::SafeDownCast(sequenceNode->GetNthDataNode(0));
  if (tNode)
  {
    for (int i = 0; i < numberOfDataNodes; i++)
    {
      tNode = vtkMRMLTransformNode::SafeDownCast(sequenceNode->GetNthDataNode(i));
      vtkAbstractTransform* trans2Parent = tNode->GetTransformToParent();

      double* transformedcroshairPosition_RAS = trans2Parent->TransformDoublePoint(croshairPosition_RAS);

      this->ChartTable->SetValue(i, 0, i);
      this->ChartTable->SetValue(i, 1, transformedcroshairPosition_RAS[0] - croshairPosition_RAS[0]);
      this->ChartTable->SetValue(i, 2, transformedcroshairPosition_RAS[1] - croshairPosition_RAS[1]);
      this->ChartTable->SetValue(i, 3, transformedcroshairPosition_RAS[2] - croshairPosition_RAS[2]);
    }
    // this->ChartTable->Update();
    this->ChartXY->RemovePlot(0);
    this->ChartXY->RemovePlot(0);
    this->ChartXY->RemovePlot(0);

    this->ChartXY->GetAxis(0)->SetTitle(qSlicerSequencesModuleWidget::tr("Displacement").toStdString());
    this->ChartXY->GetAxis(1)->SetTitle(qSlicerSequencesModuleWidget::tr("Time").toStdString());
    vtkPlot* line_X = this->ChartXY->AddPlot(vtkChart::LINE);
    vtkPlot* line_Y = this->ChartXY->AddPlot(vtkChart::LINE);
    vtkPlot* line_Z = this->ChartXY->AddPlot(vtkChart::LINE);

    line_X->SetInputData(this->ChartTable, 0, 1);
    line_Y->SetInputData(this->ChartTable, 0, 2);
    line_Z->SetInputData(this->ChartTable, 0, 3);

    line_X->SetColor(255, 0, 0, 255);
    line_Y->SetColor(0, 255, 0, 255);
    line_Z->SetColor(0, 0, 255, 255);
  }
}

//-----------------------------------------------------------------------------
// qSlicerSequencesModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSequencesModuleWidget::qSlicerSequencesModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSequencesModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerSequencesModuleWidget::~qSlicerSequencesModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::setup()
{
  Q_D(qSlicerSequencesModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  d->init();

  d->pushButton_AddSequenceNode->setIcon(QIcon(":/Icons/Add.png"));
  d->pushButton_RemoveSequenceNode->setIcon(QIcon(":/Icons/Remove.png"));

  QHeaderView* tableWidget_SynchronizedSequenceNodes_HeaderView =
    d->tableWidget_SynchronizedSequenceNodes->horizontalHeader();

  tableWidget_SynchronizedSequenceNodes_HeaderView->setSectionResizeMode(SYNCH_NODES_NAME_COLUMN,
                                                                         QHeaderView::Interactive);
  tableWidget_SynchronizedSequenceNodes_HeaderView->setSectionResizeMode(SYNCH_NODES_PROXY_COLUMN,
                                                                         QHeaderView::Interactive);
  tableWidget_SynchronizedSequenceNodes_HeaderView->setSectionResizeMode(SYNCH_NODES_PLAYBACK_COLUMN,
                                                                         QHeaderView::ResizeToContents);
  tableWidget_SynchronizedSequenceNodes_HeaderView->setSectionResizeMode(SYNCH_NODES_RECORDING_COLUMN,
                                                                         QHeaderView::ResizeToContents);
  tableWidget_SynchronizedSequenceNodes_HeaderView->setSectionResizeMode(SYNCH_NODES_OVERWRITE_PROXY_NAME_COLUMN,
                                                                         QHeaderView::ResizeToContents);
  tableWidget_SynchronizedSequenceNodes_HeaderView->setSectionResizeMode(SYNCH_NODES_SAVE_CHANGES_COLUMN,
                                                                         QHeaderView::ResizeToContents);

  tableWidget_SynchronizedSequenceNodes_HeaderView->setStretchLastSection(false);

  d->tableWidget_SynchronizedSequenceNodes->setColumnWidth(SYNCH_NODES_NAME_COLUMN, 200);
  d->tableWidget_SynchronizedSequenceNodes->setColumnWidth(SYNCH_NODES_PROXY_COLUMN, 200);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::enter()
{
  Q_D(qSlicerSequencesModuleWidget);

  if (this->mrmlScene() != 0)
  {
    // set up mrml scene observations so that the GUI gets updated
    this->qvtkConnect(
      this->mrmlScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
    this->qvtkConnect(
      this->mrmlScene(), vtkMRMLScene::NodeRemovedEvent, this, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));
    this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndImportEvent, this, SLOT(onMRMLSceneEndImportEvent()));
    this->qvtkConnect(
      this->mrmlScene(), vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onMRMLSceneEndBatchProcessEvent()));
    this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent, this, SLOT(onMRMLSceneEndCloseEvent()));
    this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndRestoreEvent, this, SLOT(onMRMLSceneEndRestoreEvent()));
    this->setActiveSequenceNode(vtkMRMLSequenceNode::SafeDownCast(d->MRMLNodeComboBox_Sequence->currentNode()));

    if (!d->ModuleWindowInitialized)
    {
      // Connect events after the module is entered to make sure that initial events that are triggered when scene is
      // set are not mistaken for user clicks.
      connect(d->mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged()));
      // Edit
      connect(d->MRMLNodeComboBox_Sequence,
              SIGNAL(currentNodeChanged(vtkMRMLNode*)),
              this,
              SLOT(onSequenceNodeSelectionChanged()));
      // Browse
      connect(d->MRMLNodeComboBox_ActiveBrowser,
              SIGNAL(currentNodeChanged(vtkMRMLNode*)),
              this,
              SLOT(activeBrowserNodeChanged(vtkMRMLNode*)));
      connect(d->MRMLNodeComboBox_MasterSequence,
              SIGNAL(currentNodeChanged(vtkMRMLNode*)),
              this,
              SLOT(sequenceNodeChanged(vtkMRMLNode*)));
      connect(d->checkBox_PlaybackItemSkippingEnabled,
              SIGNAL(toggled(bool)),
              this,
              SLOT(playbackItemSkippingEnabledChanged(bool)));
      connect(d->checkBox_RecordMasterOnly, SIGNAL(toggled(bool)), this, SLOT(recordMasterOnlyChanged(bool)));
      connect(d->comboBox_RecordingSamplingMode,
              SIGNAL(currentIndexChanged(int)),
              this,
              SLOT(recordingSamplingModeChanged(int)));
      connect(d->comboBox_IndexDisplayMode, SIGNAL(currentIndexChanged(int)), this, SLOT(indexDisplayModeChanged(int)));
      connect(d->lineEdit_IndexDisplayFormat,
              SIGNAL(textEdited(const QString)),
              this,
              SLOT(indexDisplayFormatChanged(const QString)));
      connect(d->pushButton_AddSequenceNode, SIGNAL(clicked()), this, SLOT(onAddSequenceNodeButtonClicked()));
      connect(d->pushButton_RemoveSequenceNode, SIGNAL(clicked()), this, SLOT(onRemoveSequenceNodesButtonClicked()));
      // Toolbar
      qMRMLSequenceBrowserToolBar* toolBar = d->toolBar();
      if (toolBar)
      {
        connect(
          toolBar, SIGNAL(activeBrowserNodeChanged(vtkMRMLNode*)), this, SLOT(activeBrowserNodeChanged(vtkMRMLNode*)));
      }
      d->ModuleWindowInitialized = true;
    }

    // For the user's convenience, create a browser node by default, when entering to the module and no browser node
    // exists in the scene yet
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLSequenceBrowserNode");
    if (node == nullptr)
    {
      vtkSmartPointer<vtkMRMLSequenceBrowserNode> newBrowserNode = vtkSmartPointer<vtkMRMLSequenceBrowserNode>::New();
      this->mrmlScene()->AddNode(newBrowserNode);
      this->activeBrowserNodeChanged(newBrowserNode);
    }
    else
    {
      this->activeBrowserNodeChanged(d->MRMLNodeComboBox_ActiveBrowser->currentNode());
    }
  }
  else
  {
    qCritical() << "Entering the Sequences module failed, scene is invalid";
  }

  d->setAndObserveCrosshairNode();

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::exit()
{
  Q_D(qSlicerSequencesModuleWidget);
  this->Superclass::exit();

  // remove mrml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerSequencesModuleWidget);

  if (this->mrmlScene() == scene)
  {
    return;
  }

  this->Superclass::setMRMLScene(scene);

  // Only those nodes can be proxy nodes that support content copy.
  // Retrieve that information from the scene now.
  d->SupportedProxyNodeTypes.clear();
  if (scene)
  {
    int numberOfRegisteredNodeClasses = scene->GetNumberOfRegisteredNodeClasses();
    for (int nodeClassIndex = 0; nodeClassIndex < numberOfRegisteredNodeClasses; ++nodeClassIndex)
    {
      vtkMRMLNode* registeredNode = scene->GetNthRegisteredNodeClass(nodeClassIndex);
      if (registeredNode->HasCopyContent())
      {
        d->SupportedProxyNodeTypes << registeredNode->GetClassName();
      }
    }
  }

  d->SequenceEditWidget->setMRMLScene(scene);
  d->setAndObserveCrosshairNode();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onMRMLSceneEndImportEvent()
{
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onMRMLSceneEndRestoreEvent()
{
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onMRMLSceneEndBatchProcessEvent()
{
  if (!this->mrmlScene())
  {
    return;
  }
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onMRMLSceneEndCloseEvent()
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onSequenceNodeSelectionChanged()
{
  Q_D(qSlicerSequencesModuleWidget);
  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(d->MRMLNodeComboBox_Sequence->currentNode());
  this->setActiveSequenceNode(sequenceNode);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::activeBrowserNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerSequencesModuleWidget);
  vtkMRMLSequenceBrowserNode* browserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(node);
  this->setActiveBrowserNode(browserNode);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::sequenceNodeChanged(vtkMRMLNode* inputNode)
{
  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(inputNode);
  this->setMasterSequenceNode(sequenceNode);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::playbackItemSkippingEnabledChanged(bool enabled)
{
  Q_D(qSlicerSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
  {
    return; // no active node, nothing to update
  }
  d->ActiveBrowserNode->SetPlaybackItemSkippingEnabled(enabled);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::recordMasterOnlyChanged(bool enabled)
{
  Q_D(qSlicerSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
  {
    return; // no active node, nothing to update
  }
  d->ActiveBrowserNode->SetRecordMasterOnly(enabled);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::recordingSamplingModeChanged(int index)
{
  Q_D(qSlicerSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
  {
    return; // no active node, nothing to update
  }
  d->ActiveBrowserNode->SetRecordingSamplingMode(index);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::indexDisplayModeChanged(int index)
{
  Q_D(qSlicerSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
  {
    return; // no active node, nothing to update
  }
  d->ActiveBrowserNode->SetIndexDisplayMode(index);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::indexDisplayFormatChanged(const QString& format)
{
  Q_D(qSlicerSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
  {
    return; // no active node, nothing to update
  }
  d->ActiveBrowserNode->SetIndexDisplayFormat(format.toStdString());
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onActiveBrowserNodeModified(vtkObject* caller)
{
  Q_UNUSED(caller);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onMRMLInputSequenceInputNodeModified(vtkObject* inputNode)
{
  Q_UNUSED(inputNode);
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::setActiveBrowserNode(vtkMRMLSequenceBrowserNode* browserNode)
{
  Q_D(qSlicerSequencesModuleWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }
  if (d->ActiveBrowserNode == browserNode
      && browserNode
           != nullptr) // always update if browserNode is nullptr (needed for proper update during scene close)
  {
    // no change
    return;
  }

  this->qvtkReconnect(
    d->ActiveBrowserNode, browserNode, vtkCommand::ModifiedEvent, this, SLOT(onActiveBrowserNodeModified(vtkObject*)));

  d->ActiveBrowserNode = browserNode;

  d->MRMLNodeComboBox_ActiveBrowser->setCurrentNode(browserNode);
  d->sequenceBrowserPlayWidget->setMRMLSequenceBrowserNode(browserNode);
  d->sequenceBrowserSeekWidget->setMRMLSequenceBrowserNode(browserNode);
  qMRMLSequenceBrowserToolBar* toolBar = d->toolBar();
  if (toolBar)
  {
    toolBar->setActiveBrowserNode(browserNode);
  }

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::setMasterSequenceNode(vtkMRMLSequenceNode* sequenceNode)
{
  Q_D(qSlicerSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
  {
    // this happens when entering the module (the node selector already finds a suitable sequence node so it selects it,
    // but no browser node is selected yet)
    this->updateWidgetFromMRML();
    return;
  }
  if (sequenceNode != d->ActiveBrowserNode->GetMasterSequenceNode())
  {
    bool oldModify = d->ActiveBrowserNode->StartModify();

    // Reconnect the input node's Modified() event observer
    this->qvtkReconnect(d->ActiveBrowserNode->GetMasterSequenceNode(),
                        sequenceNode,
                        vtkCommand::ModifiedEvent,
                        this,
                        SLOT(onMRMLInputSequenceInputNodeModified(vtkObject*)));

    char* sequenceNodeId = sequenceNode == nullptr ? nullptr : sequenceNode->GetID();

    d->ActiveBrowserNode->SetAndObserveMasterSequenceNodeID(sequenceNodeId);

    // Update d->ActiveBrowserNode->SetAndObserveSelectedSequenceNodeID
    if (sequenceNode != nullptr && sequenceNode->GetNumberOfDataNodes() > 0)
    {
      d->ActiveBrowserNode->SetSelectedItemNumber(0);
    }
    else
    {
      d->ActiveBrowserNode->SetSelectedItemNumber(-1);
    }

    d->ActiveBrowserNode->EndModify(oldModify);
  }
}

// --------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onAddSequenceNodeButtonClicked()
{
  Q_D(qSlicerSequencesModuleWidget);
  vtkMRMLSequenceBrowserNode* browserNode =
    vtkMRMLSequenceBrowserNode::SafeDownCast(d->MRMLNodeComboBox_ActiveBrowser->currentNode());
  if (!browserNode)
  {
    qWarning() << Q_FUNC_INFO << " failed: no browser node is selected";
    return;
  }
  vtkMRMLSequenceNode* sequenceNode =
    vtkMRMLSequenceNode::SafeDownCast(d->MRMLNodeComboBox_SynchronizeSequenceNode->currentNode());
  vtkMRMLSequenceNode* addedSequenceNode = d->logic()->AddSynchronizedNode(sequenceNode, nullptr, browserNode);
  if (addedSequenceNode)
  {
    if (browserNode->GetNumberOfSynchronizedSequenceNodes() == 0)
    {
      // master node is added - if it's a new (empty) sequence node, enable recording by default
      if (sequenceNode == nullptr)
      {
        browserNode->SetRecording(addedSequenceNode, true);
      }
    }
    else
    {
      // synchronized node is added - copy the recording setting from the master sequence node
      browserNode->SetRecording(addedSequenceNode, browserNode->GetRecording(browserNode->GetMasterSequenceNode()));
    }
  }
}

// --------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onRemoveSequenceNodesButtonClicked()
{
  Q_D(qSlicerSequencesModuleWidget);
  // First, grab all of the selected rows
  QModelIndexList modelIndexList = d->tableWidget_SynchronizedSequenceNodes->selectionModel()->selectedIndexes();
  std::vector<std::string> selectedSequenceIDs;
  for (QModelIndexList::iterator index = modelIndexList.begin(); index != modelIndexList.end(); index++)
  {
    QWidget* proxyNodeComboBox =
      d->tableWidget_SynchronizedSequenceNodes->cellWidget(index->row(), SYNCH_NODES_PROXY_COLUMN);
    std::string currSelectedSequenceID = proxyNodeComboBox->property("MRMLNodeID").toString().toStdString().c_str();
    selectedSequenceIDs.push_back(currSelectedSequenceID);
    disconnect(proxyNodeComboBox,
               SIGNAL(currentNodeChanged(vtkMRMLNode*)),
               this,
               SLOT(onProxyNodeChanged(vtkMRMLNode*))); // No need to reconnect - the entire row is going to be removed
  }
  // Now, use the MRML ID stored by the proxy node combo box to determine the sequence nodes to remove from the browser
  std::vector<std::string>::iterator sequenceIDItr;
  for (sequenceIDItr = selectedSequenceIDs.begin(); sequenceIDItr != selectedSequenceIDs.end(); sequenceIDItr++)
  {
    d->ActiveBrowserNode->RemoveSynchronizedSequenceNode(sequenceIDItr->c_str());
  }
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerSequencesModuleWidget);

  d->SynchronizedBrowsingSection->setEnabled(d->ActiveBrowserNode != nullptr);
  d->PlottingSection->setEnabled(d->ActiveBrowserNode != nullptr);
  d->AdvancedSection->setEnabled(d->ActiveBrowserNode != nullptr);

  if (d->ActiveBrowserNode == nullptr)
  {
    this->refreshSynchronizedSequenceNodesTable();
    return;
  }

  vtkMRMLSequenceNode* sequenceNode = d->ActiveBrowserNode->GetMasterSequenceNode();

  bool wasBlocked = d->MRMLNodeComboBox_MasterSequence->blockSignals(true);
  d->MRMLNodeComboBox_MasterSequence->setCurrentNode(sequenceNode);
  d->MRMLNodeComboBox_MasterSequence->blockSignals(wasBlocked);

  wasBlocked = d->checkBox_PlaybackItemSkippingEnabled->blockSignals(true);
  d->checkBox_PlaybackItemSkippingEnabled->setChecked(d->ActiveBrowserNode->GetPlaybackItemSkippingEnabled());
  d->checkBox_PlaybackItemSkippingEnabled->blockSignals(wasBlocked);

  wasBlocked = d->checkBox_RecordMasterOnly->blockSignals(true);
  d->checkBox_RecordMasterOnly->setChecked(d->ActiveBrowserNode->GetRecordMasterOnly());
  d->checkBox_RecordMasterOnly->blockSignals(wasBlocked);

  wasBlocked = d->comboBox_RecordingSamplingMode->blockSignals(true);
  d->comboBox_RecordingSamplingMode->setCurrentIndex(d->ActiveBrowserNode->GetRecordingSamplingMode());
  d->comboBox_RecordingSamplingMode->blockSignals(wasBlocked);

  wasBlocked = d->comboBox_IndexDisplayMode->blockSignals(true);
  d->comboBox_IndexDisplayMode->setCurrentIndex(d->ActiveBrowserNode->GetIndexDisplayMode());
  d->comboBox_IndexDisplayMode->blockSignals(wasBlocked);

  wasBlocked = d->lineEdit_IndexDisplayFormat->blockSignals(true);
  int position = d->lineEdit_IndexDisplayFormat->cursorPosition();
  d->lineEdit_IndexDisplayFormat->setText(QString::fromStdString(d->ActiveBrowserNode->GetIndexDisplayFormat()));
  d->lineEdit_IndexDisplayFormat->setCursorPosition(position);
  d->lineEdit_IndexDisplayFormat->blockSignals(wasBlocked);

  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::refreshSynchronizedSequenceNodesTable()
{
  Q_D(qSlicerSequencesModuleWidget);

  if (d->ActiveBrowserNode != nullptr
      && (d->ActiveBrowserNode->GetRecordingActive() || d->ActiveBrowserNode->GetPlaybackActive()))
  {
    // this is an expensive operation, we cannot afford to do it while recording or replaying
    // TODO: make this update method much more efficient
    return;
  }

  // Clear the table
  for (int row = 0; row < d->tableWidget_SynchronizedSequenceNodes->rowCount(); row++)
  {
    QCheckBox* playbackCheckbox =
      dynamic_cast<QCheckBox*>(d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_PLAYBACK_COLUMN));
    disconnect(
      playbackCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodePlaybackStateChanged(int)));
    QCheckBox* recordingCheckbox =
      dynamic_cast<QCheckBox*>(d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_RECORDING_COLUMN));
    disconnect(
      recordingCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeRecordingStateChanged(int)));
    QCheckBox* overwriteProxyNameCheckbox = dynamic_cast<QCheckBox*>(
      d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_OVERWRITE_PROXY_NAME_COLUMN));
    disconnect(overwriteProxyNameCheckbox,
               SIGNAL(stateChanged(int)),
               this,
               SLOT(synchronizedSequenceNodeOverwriteProxyNameStateChanged(int)));
    QCheckBox* saveChangesCheckbox = dynamic_cast<QCheckBox*>(
      d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_SAVE_CHANGES_COLUMN));
    disconnect(
      saveChangesCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeSaveChangesStateChanged(int)));
    qMRMLNodeComboBox* proxyNodeComboBox = dynamic_cast<qMRMLNodeComboBox*>(
      d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_PROXY_COLUMN));
    disconnect(
      proxyNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onProxyNodeChanged(vtkMRMLNode*)));
  }

  if (d->ActiveBrowserNode == nullptr)
  {
    d->tableWidget_SynchronizedSequenceNodes->setRowCount(0); // clear() would not actually remove the rows
    return;
  }
  // A valid active browser node is selected
  vtkMRMLSequenceNode* sequenceNode = d->ActiveBrowserNode->GetMasterSequenceNode();
  if (sequenceNode == nullptr)
  {
    d->tableWidget_SynchronizedSequenceNodes->setRowCount(0); // clear() would not actually remove the rows
    return;
  }

  disconnect(d->tableWidget_SynchronizedSequenceNodes,
             SIGNAL(cellChanged(int, int)),
             this,
             SLOT(sequenceNodeNameEdited(int, int)));

  vtkNew<vtkCollection> syncedNodes;
  d->ActiveBrowserNode->GetSynchronizedSequenceNodes(syncedNodes.GetPointer(), true);
  d->tableWidget_SynchronizedSequenceNodes->setRowCount(
    syncedNodes->GetNumberOfItems()); // +1 because we add the master as well

  // Create line for the compatible nodes
  for (int i = 0; i < syncedNodes->GetNumberOfItems(); ++i)
  {
    vtkMRMLSequenceNode* syncedNode = vtkMRMLSequenceNode::SafeDownCast(syncedNodes->GetItemAsObject(i));
    if (!syncedNode)
    {
      continue;
    }

    QTableWidgetItem* verticalHeaderItem = new QTableWidgetItem();
    if (!strcmp(syncedNode->GetID(), d->ActiveBrowserNode->GetMasterSequenceNode()->GetID()))
    {
      verticalHeaderItem->setText(tr("M"));
      verticalHeaderItem->setToolTip(tr("Master sequence"));
    }
    else
    {
      verticalHeaderItem->setText(QString::number(i));
      verticalHeaderItem->setToolTip(tr("Synchronized sequence"));
    }
    d->tableWidget_SynchronizedSequenceNodes->setVerticalHeaderItem(i, verticalHeaderItem);

    // Create checkboxes
    QCheckBox* playbackCheckbox = new QCheckBox(d->tableWidget_SynchronizedSequenceNodes);
    playbackCheckbox->setToolTip(tr("Include this node in synchronized playback"));
    playbackCheckbox->setProperty("MRMLNodeID", QString(syncedNode->GetID()));

    QCheckBox* overwriteProxyNameCheckbox = new QCheckBox(d->tableWidget_SynchronizedSequenceNodes);
    overwriteProxyNameCheckbox->setToolTip(tr("Overwrite the associated node's name during playback"));
    overwriteProxyNameCheckbox->setProperty("MRMLNodeID", QString(syncedNode->GetID()));

    QCheckBox* saveChangesCheckbox = new QCheckBox(d->tableWidget_SynchronizedSequenceNodes);
    saveChangesCheckbox->setToolTip(tr("Save changes to the node into the sequence"));
    saveChangesCheckbox->setProperty("MRMLNodeID", QString(syncedNode->GetID()));

    QCheckBox* recordingCheckbox = new QCheckBox(d->tableWidget_SynchronizedSequenceNodes);
    recordingCheckbox->setToolTip(tr("Include this node in synchronized recording"));
    recordingCheckbox->setProperty("MRMLNodeID", QString(syncedNode->GetID()));

    // Set previous checked state of the checkbox
    bool playbackChecked = d->ActiveBrowserNode->GetPlayback(syncedNode);
    playbackCheckbox->setChecked(playbackChecked);

    bool overwriteProxyNameChecked = d->ActiveBrowserNode->GetOverwriteProxyName(syncedNode);
    overwriteProxyNameCheckbox->setChecked(overwriteProxyNameChecked);

    bool saveChangesChecked = d->ActiveBrowserNode->GetSaveChanges(syncedNode);
    saveChangesCheckbox->setChecked(saveChangesChecked);

    bool recordingChecked = d->ActiveBrowserNode->GetRecording(syncedNode);
    recordingCheckbox->setChecked(recordingChecked);

    connect(playbackCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodePlaybackStateChanged(int)));
    connect(overwriteProxyNameCheckbox,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(synchronizedSequenceNodeOverwriteProxyNameStateChanged(int)));
    connect(
      saveChangesCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeSaveChangesStateChanged(int)));
    connect(
      recordingCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeRecordingStateChanged(int)));

    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(i, SYNCH_NODES_PLAYBACK_COLUMN, playbackCheckbox);
    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(i, SYNCH_NODES_RECORDING_COLUMN, recordingCheckbox);
    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(
      i, SYNCH_NODES_OVERWRITE_PROXY_NAME_COLUMN, overwriteProxyNameCheckbox);
    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(i, SYNCH_NODES_SAVE_CHANGES_COLUMN, saveChangesCheckbox);

    QTableWidgetItem* nameItem = new QTableWidgetItem(QString(syncedNode->GetName()));
    d->tableWidget_SynchronizedSequenceNodes->setItem(i, SYNCH_NODES_NAME_COLUMN, nameItem);

    vtkMRMLNode* proxyNode = d->ActiveBrowserNode->GetProxyNode(syncedNode);
    qMRMLNodeComboBox* proxyNodeComboBox = new qMRMLNodeComboBox();
    if (!syncedNode->GetDataNodeClassName().empty())
    {
      proxyNodeComboBox->setNodeTypes(QStringList() << syncedNode->GetDataNodeClassName().c_str());
    }
    else
    {
      proxyNodeComboBox->setNodeTypes(d->SupportedProxyNodeTypes);
    }
    proxyNodeComboBox->setAddEnabled(false);
    proxyNodeComboBox->setNoneEnabled(true);
    proxyNodeComboBox->setRemoveEnabled(true);
    proxyNodeComboBox->setRenameEnabled(true);
    proxyNodeComboBox->setShowChildNodeTypes(false); // all supported node types are explicitly listed
    proxyNodeComboBox->setShowHidden(true);          // display nodes are hidden by default
    proxyNodeComboBox->setMRMLScene(this->mrmlScene());
    proxyNodeComboBox->setCurrentNode(proxyNode);
    proxyNodeComboBox->setProperty("MRMLNodeID", QString(syncedNode->GetID()));
    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(i, SYNCH_NODES_PROXY_COLUMN, proxyNodeComboBox);

    connect(proxyNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onProxyNodeChanged(vtkMRMLNode*)));
  }

  connect(d->tableWidget_SynchronizedSequenceNodes,
          SIGNAL(cellChanged(int, int)),
          this,
          SLOT(sequenceNodeNameEdited(int, int)));
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::sequenceNodeNameEdited(int row, int column)
{
  Q_D(qSlicerSequencesModuleWidget);

  if (d->ActiveBrowserNode == nullptr)
  {
    qCritical()
      << "qSlicerSequencesModuleWidget::synchronizedSequenceNodePlaybackStateChanged: Invalid activeBrowserNode";
    return;
  }
  if (column != SYNCH_NODES_NAME_COLUMN)
  {
    return;
  }

  std::string newSequenceNodeName = d->tableWidget_SynchronizedSequenceNodes->item(row, column)->text().toStdString();

  QWidget* proxyNodeComboBox = d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_PROXY_COLUMN);
  std::string synchronizedNodeID = proxyNodeComboBox->property("MRMLNodeID").toString().toStdString().c_str();
  vtkMRMLSequenceNode* synchronizedNode =
    vtkMRMLSequenceNode::SafeDownCast(this->mrmlScene()->GetNodeByID(synchronizedNodeID));

  synchronizedNode->SetName(newSequenceNodeName.c_str());
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::synchronizedSequenceNodePlaybackStateChanged(int aState)
{
  Q_D(qSlicerSequencesModuleWidget);

  if (d->ActiveBrowserNode == nullptr)
  {
    qCritical()
      << "qSlicerSequencesModuleWidget::synchronizedSequenceNodePlaybackStateChanged: Invalid activeBrowserNode";
    return;
  }

  QCheckBox* senderCheckbox = dynamic_cast<QCheckBox*>(sender());
  if (!senderCheckbox)
  {
    qCritical()
      << "qSlicerSequencesModuleWidget::synchronizedSequenceNodePlaybackStateChanged: Invalid sender checkbox";
    return;
  }

  std::string synchronizedNodeID = senderCheckbox->property("MRMLNodeID").toString().toStdString().c_str();
  vtkMRMLSequenceNode* synchronizedNode =
    vtkMRMLSequenceNode::SafeDownCast(this->mrmlScene()->GetNodeByID(synchronizedNodeID));
  d->ActiveBrowserNode->SetPlayback(synchronizedNode, aState);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::synchronizedSequenceNodeRecordingStateChanged(int aState)
{
  Q_D(qSlicerSequencesModuleWidget);

  if (d->ActiveBrowserNode == nullptr)
  {
    qCritical()
      << "qSlicerSequencesModuleWidget::synchronizedSequenceNodeRecordingStateChanged: Invalid activeBrowserNode";
    return;
  }

  QCheckBox* senderCheckbox = dynamic_cast<QCheckBox*>(sender());
  if (!senderCheckbox)
  {
    qCritical()
      << "qSlicerSequencesModuleWidget::synchronizedSequenceNodeRecordingStateChanged: Invalid sender checkbox";
    return;
  }

  std::string synchronizedNodeID = senderCheckbox->property("MRMLNodeID").toString().toStdString().c_str();
  vtkMRMLSequenceNode* synchronizedNode =
    vtkMRMLSequenceNode::SafeDownCast(this->mrmlScene()->GetNodeByID(synchronizedNodeID));
  d->ActiveBrowserNode->SetRecording(synchronizedNode, aState);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::synchronizedSequenceNodeOverwriteProxyNameStateChanged(int aState)
{
  Q_D(qSlicerSequencesModuleWidget);

  if (d->ActiveBrowserNode == nullptr)
  {
    qCritical() << "qSlicerSequencesModuleWidget::synchronizedSequenceNodeOverwriteProxyNameStateChanged: Invalid "
                   "activeBrowserNode";
    return;
  }

  QCheckBox* senderCheckbox = dynamic_cast<QCheckBox*>(sender());
  if (!senderCheckbox)
  {
    qCritical() << "qSlicerSequencesModuleWidget::synchronizedSequenceNodeOverwriteProxyNameStateChanged: Invalid "
                   "sender checkbox";
    return;
  }

  std::string synchronizedNodeID = senderCheckbox->property("MRMLNodeID").toString().toStdString().c_str();
  vtkMRMLSequenceNode* synchronizedNode =
    vtkMRMLSequenceNode::SafeDownCast(this->mrmlScene()->GetNodeByID(synchronizedNodeID));
  d->ActiveBrowserNode->SetOverwriteProxyName(synchronizedNode, aState);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::synchronizedSequenceNodeSaveChangesStateChanged(int aState)
{
  Q_D(qSlicerSequencesModuleWidget);

  if (d->ActiveBrowserNode == nullptr)
  {
    qCritical()
      << "qSlicerSequencesModuleWidget::synchronizedSequenceNodeSaveChangesStateChanged: Invalid activeBrowserNode";
    return;
  }

  QCheckBox* senderCheckbox = dynamic_cast<QCheckBox*>(sender());
  if (!senderCheckbox)
  {
    qCritical()
      << "qSlicerSequencesModuleWidget::synchronizedSequenceNodeSaveChangesStateChanged: Invalid sender checkbox";
    return;
  }

  std::string synchronizedNodeID = senderCheckbox->property("MRMLNodeID").toString().toStdString().c_str();
  vtkMRMLSequenceNode* synchronizedNode =
    vtkMRMLSequenceNode::SafeDownCast(this->mrmlScene()->GetNodeByID(synchronizedNodeID));
  d->ActiveBrowserNode->SetSaveChanges(synchronizedNode, aState);
}

//-----------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::onProxyNodeChanged(vtkMRMLNode* newProxyNode)
{
  Q_D(qSlicerSequencesModuleWidget);

  if (d->ActiveBrowserNode == nullptr)
  {
    qCritical() << "qSlicerSequencesModuleWidget::onProxyNodeChanged: Invalid activeBrowserNode";
    return;
  }

  qMRMLNodeComboBox* senderComboBox = dynamic_cast<qMRMLNodeComboBox*>(sender());
  if (!senderComboBox)
  {
    qCritical() << "qSlicerSequencesModuleWidget::onProxyNodeChanged: Invalid sender checkbox";
    return;
  }

  std::string synchronizedNodeID = senderComboBox->property("MRMLNodeID").toString().toStdString().c_str();
  vtkMRMLSequenceNode* synchronizedNode =
    vtkMRMLSequenceNode::SafeDownCast(this->mrmlScene()->GetNodeByID(synchronizedNodeID));

  // If name sync is enabled between sequence and proxy node then update the sequence node name based on the proxy node
  if (newProxyNode && newProxyNode->GetName() != nullptr && synchronizedNode
      && d->ActiveBrowserNode->GetOverwriteProxyName(synchronizedNode))
  {
    std::string baseName = "Data";
    if (newProxyNode->GetAttribute("Sequences.BaseName") != 0)
    {
      baseName = newProxyNode->GetAttribute("Sequences.BaseName");
    }
    else if (newProxyNode->GetName() != 0)
    {
      baseName = newProxyNode->GetName();
    }
    baseName += " sequence";
    std::string proxyNodeName = this->mrmlScene()->GetUniqueNameByString(baseName.c_str());
    synchronizedNode->SetName(proxyNodeName.c_str());
  }

  d->logic()->AddSynchronizedNode(synchronizedNode, newProxyNode, d->ActiveBrowserNode);
}

//------------------------------------------------------------------------------
void qSlicerSequencesModuleWidget::updateChart()
{
  Q_D(qSlicerSequencesModuleWidget);
  if (d->pushButton_iCharting->isChecked())
  {
    d->updateInteractiveCharting();
  }
}

//-----------------------------------------------------------
bool qSlicerSequencesModuleWidget::setEditedNode(vtkMRMLNode* node,
                                                 QString role /* = QString()*/,
                                                 QString context /* = QString() */)
{
  Q_UNUSED(context);
  Q_D(qSlicerSequencesModuleWidget);
  if (vtkMRMLSequenceBrowserNode::SafeDownCast(node))
  {
    if (role == "toolbar")
    {
      qMRMLSequenceBrowserToolBar* toolBar = d->toolBar();
      if (toolBar)
      {
        toolBar->setActiveBrowserNode(vtkMRMLSequenceBrowserNode::SafeDownCast(node));
      }
    }
    else
    {
      d->mainTabWidget->setCurrentIndex(0); // browse tab
      d->MRMLNodeComboBox_ActiveBrowser->setCurrentNode(node);
    }
    return true;
  }

  if (vtkMRMLSequenceNode::SafeDownCast(node))
  {
    d->mainTabWidget->setCurrentIndex(1); // edit tab
    d->MRMLNodeComboBox_Sequence->setCurrentNode(node);
    return true;
  }

  return false;
}

//-----------------------------------------------------------
void qSlicerSequencesModuleWidget::setActiveSequenceNode(vtkMRMLSequenceNode* newActiveSequenceNode)
{
  Q_D(qSlicerSequencesModuleWidget);
  d->SequenceEditWidget->setMRMLSequenceNode(newActiveSequenceNode);

  if (d->mainTabWidget->currentWidget() == d->editSequenceTab && newActiveSequenceNode
      && newActiveSequenceNode->GetNumberOfDataNodes() == 0)
  {
    // If an empty sequence is selected then show the candidate nodes section
    // because it is likely that the user wants to add data nodes to the sequence now.
    d->SequenceEditWidget->setCandidateNodesSectionVisible(true);
  }
}

//-----------------------------------------------------------
void qSlicerSequencesModuleWidget::onCurrentTabChanged()
{
  Q_D(qSlicerSequencesModuleWidget);
  if (d->mainTabWidget->currentWidget() == d->editSequenceTab)
  {
    vtkMRMLSequenceNode* sequenceNode = d->SequenceEditWidget->mrmlSequenceNode();
    if (sequenceNode && sequenceNode->GetNumberOfDataNodes() == 0)
    {
      // If an empty sequence is selected then show the candidate nodes section
      // because it is likely that the user wants to add data nodes to the sequence now.
      d->SequenceEditWidget->setCandidateNodesSectionVisible(true);
    }
  }
  else
  {
    // Candidate nodes table updates are expensive, collapse it if the tab is not visible
    d->SequenceEditWidget->setCandidateNodesSectionVisible(false);
  }
}
