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
#include <QDebug>
#include <QFormLayout>

// SlicerQt includes
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
  this->CLIModuleUIHelper = 0;
  this->CommandLineModuleNode = 0;
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

  this->addParameterGroups();

  // Connect buttons
  this->connect(this->ApplyPushButton, SIGNAL(pressed()),
                q, SLOT(apply()));

  this->connect(this->CancelPushButton, SIGNAL(pressed()),
                q, SLOT(cancel()));

  this->connect(this->DefaultPushButton, SIGNAL(pressed()),
                q, SLOT(reset()));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                q, SLOT(setCurrentCommandLineModuleNode(vtkMRMLNode*)));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
                SLOT(setDefaultNodeValue(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::updateUiFromCommandLineModuleNode(
  vtkObject* commandLineModuleNode)
{
  if (!commandLineModuleNode)
    {
    this->ApplyPushButton->setEnabled(false);
    this->CancelPushButton->setEnabled(false);
    this->DefaultPushButton->setEnabled(false);
    return;
    }

  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);

  // Update parameters
  this->CLIModuleUIHelper->updateUi(node);

  switch (node->GetStatus())
    {
    case vtkMRMLCommandLineModuleNode::Scheduled:
      this->ApplyPushButton->setEnabled(false);
      this->CancelPushButton->setEnabled(true);
      break;
    case vtkMRMLCommandLineModuleNode::Running:
    case vtkMRMLCommandLineModuleNode::Cancelling:
      this->DefaultPushButton->setEnabled(false);
      this->ApplyPushButton->setEnabled(false);
      this->CancelPushButton->setEnabled(true);
      break;
    case vtkMRMLCommandLineModuleNode::Cancelled:
    case vtkMRMLCommandLineModuleNode::Completed:
    case vtkMRMLCommandLineModuleNode::CompletedWithErrors:
      this->DefaultPushButton->setEnabled(true);
      this->ApplyPushButton->setEnabled(true);
      this->CancelPushButton->setEnabled(false);
      break;
    default:
    case vtkMRMLCommandLineModuleNode::Idle:
      this->DefaultPushButton->setEnabled(true);
      this->ApplyPushButton->setEnabled(true);
      this->CancelPushButton->setEnabled(false);
      break;
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
  this->CLIProgressBar->setCommandLineModuleNode(vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode));
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
  // make sure a command line module node is created
  if (this->CommandLineModuleNode != 0 ||
      // but if the scene is closing, then nevermind, values are changing
      // because nodes are getting removed
      !q->mrmlScene() ||
      q->mrmlScene()->IsClosing())
    {
    return;
    }
  // if not, then create a default node
  this->MRMLCommandLineModuleNodeSelector->addNode();
  Q_ASSERT(this->CommandLineModuleNode);
  // and reset the value as it may have changed because of the new node
  this->CLIModuleUIHelper->setValue(name, value);
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
qSlicerCLIModuleWidget::~qSlicerCLIModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setup()
{
  Q_D(qSlicerCLIModuleWidget);

  d->setupUi(this);
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
  // we can save the paramaters of the command line module node so they could be
  // retrieved later on when it becomes current again
  d->updateCommandLineModuleNodeFromUi(d->CommandLineModuleNode);

  d->CommandLineModuleNode = node;
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
  node->SetStatus(vtkMRMLCommandLineModuleNode::Cancelling);
}

}
