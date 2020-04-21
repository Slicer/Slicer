/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QAction>
#include <QDebug>
#include <QFormLayout>
#include <QMenu>

// Slicer includes
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleWidget_p.h"
#include "vtkSlicerCLIModuleLogic.h"
#include "qSlicerCLIModuleUIHelper.h"

// CLIMRML includes
#include "vtkMRMLCommandLineModuleNode.h"

//-----------------------------------------------------------------------------
// qSlicerCLIModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerCLIModuleWidgetPrivate::qSlicerCLIModuleWidgetPrivate(qSlicerCLIModuleWidget& object)
  :q_ptr(&object)
{
  this->CLIModuleUIHelper = nullptr;
  this->CommandLineModuleNode = nullptr;
  this->AutoRunWhenParameterChanged = nullptr;
  this->AutoRunWhenInputModified = nullptr;
  this->AutoRunOnOtherInputEvents = nullptr;
  this->AutoRunCancelsRunningProcess = nullptr;
}

//-----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic* qSlicerCLIModuleWidgetPrivate::logic()const
{
  Q_Q(const qSlicerCLIModuleWidget);
  return vtkSlicerCLIModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode* qSlicerCLIModuleWidgetPrivate::commandLineModuleNode()const
{
  return vtkMRMLCommandLineModuleNode::SafeDownCast(
    this->MRMLCommandLineModuleNodeSelector->currentNode());
}

//-----------------------------------------------------------------------------
qSlicerCLIModule * qSlicerCLIModuleWidgetPrivate::module()const
{
  Q_Q(const qSlicerCLIModuleWidget);
  qSlicerAbstractCoreModule* coreModule = const_cast<qSlicerAbstractCoreModule*>(q->module());
  return qobject_cast<qSlicerCLIModule*>(coreModule);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerCLIModuleWidget);

  this->Ui_qSlicerCLIModuleWidget::setupUi(widget);

  QString title = QString::fromStdString(
    this->logic()->GetDefaultModuleDescription().GetTitle());
  this->ModuleCollapsibleButton->setText(title);

  this->MRMLCommandLineModuleNodeSelector->setBaseName(title);
  /// Use the title of the CLI to filter all the command line module node
  /// It is not very robust but there shouldn't be twice the same title.
  this->MRMLCommandLineModuleNodeSelector->addAttribute(
    "vtkMRMLCommandLineModuleNode", "CommandLineModule", title);

  this->MRMLCommandLineModuleNodeSelector->setNodeTypeLabel("Parameter set", "vtkMRMLCommandLineModuleNode");

  this->addParameterGroups();

  // Setup AutoRun menu
  QMenu* autoRunMenu = new QMenu(qSlicerCLIModuleWidget::tr("AutoRun"), this->AutoRunPushButton);

  this->AutoRunWhenParameterChanged =
    new QAction(qSlicerCLIModuleWidget::tr("AutoRun on changed parameter"), autoRunMenu);
  this->AutoRunWhenParameterChanged->setToolTip(
    qSlicerCLIModuleWidget::tr("As long as the AutoRun button is down, the module "
          "is run anytime a parameter value is changed."));
  this->AutoRunWhenParameterChanged->setCheckable(true);
  this->connect(this->AutoRunWhenParameterChanged, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRunWhenParameterChanged(bool)));

  this->AutoRunWhenInputModified =
    new QAction(qSlicerCLIModuleWidget::tr("AutoRun on modified input"), autoRunMenu);
  this->AutoRunWhenInputModified->setToolTip(
    qSlicerCLIModuleWidget::tr("As long as the AutoRun button is down, the module is run anytime an "
          "input node is modified."));
  this->AutoRunWhenInputModified->setCheckable(true);
  this->connect(this->AutoRunWhenInputModified, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRunWhenInputModified(bool)));

  this->AutoRunOnOtherInputEvents =
    new QAction(qSlicerCLIModuleWidget::tr("AutoRun on other input events"), autoRunMenu);
  this->AutoRunOnOtherInputEvents->setToolTip(
    qSlicerCLIModuleWidget::tr("As long as the AutoRun button is down, the module is run anytime an "
          "input node fires an event other than a modified event."));
  this->AutoRunOnOtherInputEvents->setCheckable(true);
  this->connect(this->AutoRunOnOtherInputEvents, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRunOnOtherInputEvents(bool)));

  this->AutoRunCancelsRunningProcess =
    new QAction(qSlicerCLIModuleWidget::tr("AutoRun cancels running process"),autoRunMenu);
  this->AutoRunCancelsRunningProcess->setToolTip(
    qSlicerCLIModuleWidget::tr("When checked, on apply, the module cancels/stops the existing "
          "running instance if any, otherwise it waits the completion to start "
          "a new run."));
  this->AutoRunCancelsRunningProcess->setCheckable(true);
  this->connect(this->AutoRunCancelsRunningProcess, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRunCancelsRunningProcess(bool)));

  autoRunMenu->addAction(this->AutoRunWhenParameterChanged);
  autoRunMenu->addAction(this->AutoRunWhenInputModified);
  autoRunMenu->addAction(this->AutoRunOnOtherInputEvents);
  autoRunMenu->addAction(this->AutoRunCancelsRunningProcess);
  this->AutoRunPushButton->setMenu(autoRunMenu);

  // Connect buttons
  this->connect(this->ApplyPushButton, SIGNAL(clicked()),
                q, SLOT(apply()));

  this->connect(this->CancelPushButton, SIGNAL(clicked()),
                q, SLOT(cancel()));

  this->connect(this->DefaultPushButton, SIGNAL(clicked()),
                q, SLOT(reset()));

  this->connect(this->AutoRunPushButton, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRun(bool)));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                q, SLOT(setCurrentCommandLineModuleNode(vtkMRMLNode*)));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
                SLOT(setDefaultNodeValue(vtkMRMLNode*)));

  // Scene must be set in node selector widgets before the MRMLCommandLineModuleNodeSelector widget
  // because when the scene is set in MRMLCommandLineModuleNodeSelector the first available module node
  // is automatically selected and all widgets are updated.
  // Node selector widgets can only be updated if the scene is already set, therefore
  // we set the scene here for all widgets, before MRMLCommandLineModuleNodeSelector has a chance to trigger
  // an update. Scene in MRMLCommandLineModuleNodeSelector will be set later by qSlicerAbstractCoreModule.
  emit q->mrmlSceneChanged(this->module()->mrmlScene());
  this->connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
              this->MRMLCommandLineModuleNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::updateUiFromCommandLineModuleNode(
  vtkObject* commandLineModuleNode)
{
  this->AutoRunPushButton->setEnabled(commandLineModuleNode != nullptr);
  if (!commandLineModuleNode)
    {
    this->ApplyPushButton->setEnabled(false);
    this->CancelPushButton->setEnabled(false);
    this->DefaultPushButton->setEnabled(false);
    this->CLIModuleUIHelper->updateUi(nullptr); // disable widgets
    return;
    }

  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);

  // Update parameters except if the module is running, it would prevent the
  // the user to keep the focus into the widgets each time a progress
  // is reported. (try to select text in the label list line edit of the
  // ModelMaker while running)
  if (!(node->GetStatus() & vtkMRMLCommandLineModuleNode::Running))
    {
    this->CLIModuleUIHelper->updateUi(node);
    }

  this->ApplyPushButton->setEnabled(!node->IsBusy());
  this->DefaultPushButton->setEnabled(!node->IsBusy());
  this->CancelPushButton->setEnabled(node->IsBusy());

  this->AutoRunWhenParameterChanged->setChecked(
    node->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunOnChangedParameter);
  this->AutoRunWhenInputModified->setChecked(
    node->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunOnModifiedInputEvent);
  this->AutoRunOnOtherInputEvents->setChecked(
    node->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunOnOtherInputEvents);
  this->AutoRunCancelsRunningProcess->setChecked(
    node->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunCancelsRunningProcess);
  if (this->AutoRunPushButton->isChecked() != node->GetAutoRun())
    {
    this->AutoRunPushButton->setChecked(node->GetAutoRun());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::updateCommandLineModuleNodeFromUi(
  vtkObject* commandLineModuleNode)
{
  if (!commandLineModuleNode)
    {
    return;
    }
  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);
  this->CLIModuleUIHelper->updateMRMLCommandLineModuleNode(node);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::setDefaultNodeValue(vtkMRMLNode* commandLineModuleNode)
{
  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);
  // Note that node will fire a ModifyEvent.
  node->SetModuleDescription(this->logic()->GetDefaultModuleDescription());
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::addParameterGroups()
{
  // iterate over each parameter group
  const ModuleDescription& moduleDescription =
    this->logic()->GetDefaultModuleDescription();
  for (ParameterGroupConstIterator pgIt = moduleDescription.GetParameterGroups().begin();
       pgIt != moduleDescription.GetParameterGroups().end(); ++pgIt)
    {
    this->addParameterGroup(this->VerticalLayout, *pgIt);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::addParameterGroup(QBoxLayout* _layout,
                                                     const ModuleParameterGroup& parameterGroup)
{
  Q_ASSERT(_layout);

  ctkCollapsibleButton * collapsibleWidget = new ctkCollapsibleButton();
  collapsibleWidget->setText(QString::fromStdString(parameterGroup.GetLabel()));
  collapsibleWidget->setCollapsed(parameterGroup.GetAdvanced() == "true");

  // Create a vertical layout and add parameter to it
  QFormLayout *vbox = new QFormLayout;
  this->addParameters(vbox, parameterGroup);
  //vbox->addStretch(1);
  vbox->setVerticalSpacing(1);
  collapsibleWidget->setLayout(vbox);

  _layout->addWidget(collapsibleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::addParameters(QFormLayout* _layout,
                                                const ModuleParameterGroup& parameterGroup)
{
  Q_ASSERT(_layout);
  // iterate over each parameter in this group
  ParameterConstIterator pBeginIt = parameterGroup.GetParameters().begin();
  ParameterConstIterator pEndIt = parameterGroup.GetParameters().end();

  for (ParameterConstIterator pIt = pBeginIt; pIt != pEndIt; ++pIt)
    {
    this->addParameter(_layout, *pIt);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::addParameter(QFormLayout* _layout,
                                               const ModuleParameter& moduleParameter)
{
  Q_ASSERT(_layout);

  if (moduleParameter.GetHidden() == "true")
    {
    return;
    }

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString description = QString::fromStdString(moduleParameter.GetDescription());

  // TODO Parameters with flags can support the None node because they are optional
  //int noneEnabled = 0;
  //if (moduleParameter.GetLongFlag() != "" || moduleParameter.GetFlag() != "")
  //  {
  //  noneEnabled = 1;
  //  }

  QLabel* widgetLabel = new QLabel(_label);
  widgetLabel->setToolTip(description);

  QWidget * widget = this->CLIModuleUIHelper->createTagWidget(moduleParameter);

  _layout->addRow(widgetLabel, widget);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::onValueChanged(const QString& name, const QVariant& value)
{
  Q_Q(qSlicerCLIModuleWidget);
  // but if the scene is closing, then nevermind, values are changing
  // because nodes are getting removed
  if (!q->mrmlScene() ||
      q->mrmlScene()->IsClosing())
    {
    return;
    }
  // Make sure a command line module node is created
  if (this->CommandLineModuleNode == nullptr)
    {
    // if not, then create a default node
    this->MRMLCommandLineModuleNodeSelector->addNode();
    Q_ASSERT(this->CommandLineModuleNode);
    }
  this->CLIModuleUIHelper->setCommandLineModuleParameter(
    this->CommandLineModuleNode, name, value);
}

//-----------------------------------------------------------------------------
// qSlicerCLIModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerCLIModuleWidget::qSlicerCLIModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCLIModuleWidgetPrivate(*this))
{
  Q_D(qSlicerCLIModuleWidget);

  d->CLIModuleUIHelper = new qSlicerCLIModuleUIHelper(this);
  this->connect(d->CLIModuleUIHelper,
                SIGNAL(valueChanged(QString,QVariant)),
                d,
                SLOT(onValueChanged(QString,QVariant)));
}

//-----------------------------------------------------------------------------
qSlicerCLIModuleWidget::~qSlicerCLIModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setup()
{
  Q_D(qSlicerCLIModuleWidget);

  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::enter()
{
  Q_D(qSlicerCLIModuleWidget);

  this->Superclass::enter();

  // Make sure a command line module node is available when the module widget
  // is activated. If no CLI node is available then create a new one.
  if (d->MRMLCommandLineModuleNodeSelector->currentNode() == nullptr)
    {
    bool wasBlocked = d->MRMLCommandLineModuleNodeSelector->blockSignals(true);
    vtkMRMLCommandLineModuleNode* node = vtkMRMLCommandLineModuleNode::SafeDownCast(d->MRMLCommandLineModuleNodeSelector->addNode());
    Q_ASSERT(node);
    // Initialize module description (just to avoid warnings
    // when the node is set as current node and GUI is attempted to be updated from the node)
    d->setDefaultNodeValue(node);
    d->MRMLCommandLineModuleNodeSelector->blockSignals(wasBlocked);
    this->setCurrentCommandLineModuleNode(node);
    Q_ASSERT(d->CommandLineModuleNode);
    }
}

//-----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode * qSlicerCLIModuleWidget::currentCommandLineModuleNode()const
{
  Q_D(const qSlicerCLIModuleWidget);
  return d->CommandLineModuleNode;
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setCurrentCommandLineModuleNode(
  vtkMRMLNode* commandLineModuleNode)
{
  Q_D(qSlicerCLIModuleWidget);
  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  if (node == d->CommandLineModuleNode)
    {
    return;
    }

  // Update the selector if this slot was called programmatically
  Q_ASSERT(d->MRMLCommandLineModuleNodeSelector);
  if (d->MRMLCommandLineModuleNodeSelector->currentNode()
      != commandLineModuleNode)
    {
    d->MRMLCommandLineModuleNodeSelector->setCurrentNode(commandLineModuleNode);
    return;
    }

  // Connect node modified event to updateUi that synchronize the values of the
  // nodes with the Ui
  this->qvtkReconnect(d->CommandLineModuleNode, node,
    vtkCommand::ModifiedEvent,
    d, SLOT(updateUiFromCommandLineModuleNode(vtkObject*)));

  // After we disconnected the Modified event from the old CommandLineModuleNode
  // we can save the parameters of the command line module node so they could be
  // retrieved later on when it becomes current again
  //d->updateCommandLineModuleNodeFromUi(d->CommandLineModuleNode);

  d->CommandLineModuleNode = node;
  d->CLIProgressBar->setCommandLineModuleNode(d->CommandLineModuleNode);
  d->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::apply(bool wait)
{
  Q_D(qSlicerCLIModuleWidget);
  vtkMRMLCommandLineModuleNode* node = d->commandLineModuleNode();
  Q_ASSERT(node);
  d->CLIModuleUIHelper->updateMRMLCommandLineModuleNode(node);
  this->run(node, /* waitForCompletion= */ wait);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::cancel()
{
  Q_D(qSlicerCLIModuleWidget);
  vtkMRMLCommandLineModuleNode* node = d->commandLineModuleNode();
  Q_ASSERT(node);
  this->cancel(node);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::reset()
{
  Q_D(qSlicerCLIModuleWidget);
  vtkMRMLCommandLineModuleNode* node = d->commandLineModuleNode();
  Q_ASSERT(node);
  d->setDefaultNodeValue(node);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::run(vtkMRMLCommandLineModuleNode* parameterNode, bool waitForCompletion)
{
  Q_D(qSlicerCLIModuleWidget);
  Q_ASSERT(d->logic());

  if (waitForCompletion)
    {
    d->logic()->ApplyAndWait(parameterNode);
    }
  else
    {
    d->logic()->Apply(parameterNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::cancel(vtkMRMLCommandLineModuleNode* node)
{
  if (!node)
    {
    return;
    }
  node->Cancel();
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setAutoRun(bool enable)
{
  Q_D(qSlicerCLIModuleWidget);
  d->commandLineModuleNode()->SetAutoRun(enable);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setAutoRunWhenParameterChanged(bool autoRun)
{
  Q_D(qSlicerCLIModuleWidget);
  int newAutoRunMode = d->commandLineModuleNode()->GetAutoRunMode();
  if (autoRun)
    {
    newAutoRunMode |= vtkMRMLCommandLineModuleNode::AutoRunOnChangedParameter;
    }
  else
    {
    newAutoRunMode &= ~vtkMRMLCommandLineModuleNode::AutoRunOnChangedParameter;
    }
  d->commandLineModuleNode()->SetAutoRunMode(newAutoRunMode);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setAutoRunWhenInputModified(bool autoRun)
{
  Q_D(qSlicerCLIModuleWidget);
  int newAutoRunMode = d->commandLineModuleNode()->GetAutoRunMode();
  if (autoRun)
    {
    newAutoRunMode |= vtkMRMLCommandLineModuleNode::AutoRunOnModifiedInputEvent;
    }
  else
    {
    newAutoRunMode &= ~vtkMRMLCommandLineModuleNode::AutoRunOnModifiedInputEvent;
    }
  d->commandLineModuleNode()->SetAutoRunMode(newAutoRunMode);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setAutoRunOnOtherInputEvents(bool autoRun)
{
  Q_D(qSlicerCLIModuleWidget);
  int newAutoRunMode = d->commandLineModuleNode()->GetAutoRunMode();
  if (autoRun)
    {
    newAutoRunMode |= vtkMRMLCommandLineModuleNode::AutoRunOnOtherInputEvents;
    }
  else
    {
    newAutoRunMode &= ~vtkMRMLCommandLineModuleNode::AutoRunOnOtherInputEvents;
    }
  d->commandLineModuleNode()->SetAutoRunMode(newAutoRunMode);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setAutoRunCancelsRunningProcess(bool autoRun)
{
  Q_D(qSlicerCLIModuleWidget);
  int newAutoRunMode = d->commandLineModuleNode()->GetAutoRunMode();
  if (autoRun)
    {
    newAutoRunMode |= vtkMRMLCommandLineModuleNode::AutoRunCancelsRunningProcess;
    }
  else
    {
    newAutoRunMode &= ~vtkMRMLCommandLineModuleNode::AutoRunCancelsRunningProcess;
    }
  d->commandLineModuleNode()->SetAutoRunMode(newAutoRunMode);
}

//-----------------------------------------------------------
bool qSlicerCLIModuleWidget::setEditedNode(vtkMRMLNode* node,
                                           QString role /* = QString()*/,
                                           QString context /* = QString()*/)
{
  Q_D(qSlicerCLIModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  vtkMRMLCommandLineModuleNode* cmdLineModuleNode = vtkMRMLCommandLineModuleNode::SafeDownCast(node);
  if (!cmdLineModuleNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid input node";
    return false;
    }
  const char* moduleTitle = cmdLineModuleNode->GetAttribute("CommandLineModule");
  if (!moduleTitle)
    {
    qWarning() << Q_FUNC_INFO << " failed: CommandLineModule attribute of node is not set";
    return false;
    }
  if (moduleTitle != this->module()->title())
    {
    qWarning() << Q_FUNC_INFO << " failed: mismatch of module title in CommandLineModule attribute of node";
    return false;
    }
  d->MRMLCommandLineModuleNodeSelector->setCurrentNode(node);
  return true;
}

//-----------------------------------------------------------
double qSlicerCLIModuleWidget::nodeEditable(vtkMRMLNode* node)
{
  if (vtkMRMLCommandLineModuleNode::SafeDownCast(node))
    {
    vtkMRMLCommandLineModuleNode* cmdLineModuleNode = vtkMRMLCommandLineModuleNode::SafeDownCast(node);
    const char* moduleTitle = cmdLineModuleNode->GetAttribute("CommandLineModule");
    if (!moduleTitle)
      {
      // node is not associated to any module
      return 0.0;
      }
    if (moduleTitle != this->module()->title())
      {
      return 0.0;
      }
    // Module title matches, probably this module owns this node
    return 0.5;
    }
  else
    {
    return 0.0;
    }
}
