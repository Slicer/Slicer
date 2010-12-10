/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QLabel>
#include <QFormLayout>
#include <QDebug>

// SlicerQt includes
#include "qSlicerCLIModuleWidget.h"
#include "qSlicerCLIModuleWidget_p.h"
#include "vtkSlicerCLIModuleLogic.h"
#include "qSlicerCLIModuleUIHelper.h"
#include "qSlicerWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLCommandLineModuleNode.h>

//-----------------------------------------------------------------------------
// qSlicerCLIModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerCLIModuleWidgetPrivate::qSlicerCLIModuleWidgetPrivate(qSlicerCLIModuleWidget& object)
  :q_ptr(&object)
{
  this->ProcessInformation = 0;
  this->Name = "NA";
  this->CommandLineModuleNode = 0;
  this->CLIModuleUIHelper = 0;
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
void qSlicerCLIModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerCLIModuleWidget);
  
  this->Ui_qSlicerCLIModuleWidget::setupUi(widget);

  this->ModuleCollapsibleButton->setText(this->Title);

  this->MRMLCommandLineModuleNodeSelector->setBaseName(this->Title);
  this->MRMLCommandLineModuleNodeSelector->addAttribute(
    "vtkMRMLCommandLineModuleNode", "CommandLineModule", this->Title);

  this->addParameterGroups();

  // Connect buttons
  this->connect(this->ApplyPushButton, SIGNAL(pressed()),
                q, SLOT(apply()));

  this->connect(this->CancelPushButton, SIGNAL(pressed()),
                q, SLOT(cancel()));

  this->connect(this->DefaultPushButton, SIGNAL(pressed()),
                q, SLOT(reset()));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(currentNodeChanged(bool)),
                SLOT(enableCommandButtonState(bool)));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                q, SLOT(setCurrentCommandLineModuleNode(vtkMRMLNode*)));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
                SLOT(setDefaultNodeValue(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::enableCommandButtonState(bool enable)
{
  this->ApplyPushButton->setEnabled(enable);
  this->CancelPushButton->setEnabled(enable);
  this->DefaultPushButton->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::updateUiFromCommandLineModuleNode(
  vtkObject* commandLineModuleNode)
{
  if (!commandLineModuleNode)
    {
    return;
    }
  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);

  this->CLIModuleUIHelper->updateUi(node);
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

  int disabledModify = node->StartModify();
  node->SetModuleDescription(this->ModuleDescriptionObject);
  node->EndModify(disabledModify);
  
  // Notify observer(s)
  node->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::addParameterGroups()
{
  // iterate over each parameter group
  for (ParameterGroupConstIterator pgIt = this->ParameterGroups.begin();
       pgIt != this->ParameterGroups.end(); ++pgIt)
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
  int noneEnabled = 0; 
  if (moduleParameter.GetLongFlag() != "" || moduleParameter.GetFlag() != "")
    {
    noneEnabled = 1;
    }

  QWidget * widget = this->CLIModuleUIHelper->createTagWidget(moduleParameter);

  if (widget)
    {
    widget->setToolTip(description);
    _layout->addRow(new QLabel(_label), widget);
    }
  else
    {
    QLabel * labelWidget = new QLabel(_label); 
    labelWidget->setToolTip(description);
    _layout->addWidget(labelWidget);
    }
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
      q->mrmlScene()->GetIsClosing())
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
qSlicerCLIModuleWidget::qSlicerCLIModuleWidget(
  ModuleDescription* desc, QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCLIModuleWidgetPrivate(*this))
{
  Q_ASSERT(desc);
  Q_D(qSlicerCLIModuleWidget);

  d->ModuleDescriptionObject = *desc;

  d->CLIModuleUIHelper = new qSlicerCLIModuleUIHelper(this);
  this->connect(d->CLIModuleUIHelper,
                SIGNAL(valueChanged(const QString&, const QVariant&)),
                d,
                SLOT(onValueChanged(const QString&, const QVariant&)));

  // Set properties
  d->Title = QString::fromStdString(desc->GetTitle());
  d->Contributor = QString::fromStdString(desc->GetContributor());
  d->Category = QString::fromStdString(desc->GetCategory());

  d->ProcessInformation = desc->GetProcessInformation();
  d->ParameterGroups = desc->GetParameterGroups();
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

  // After we desconnected the Modified event from the old CommandLineModuleNode
  // we can save the paramaters of the command line module node so they could be
  // retrieved later on when it becomes current again
  d->updateCommandLineModuleNodeFromUi(d->CommandLineModuleNode);

  d->CommandLineModuleNode = node;
  d->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerCLIModuleWidget, const QString&, setModuleEntryPoint, ModuleEntryPoint);

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::apply()
{
  Q_D(qSlicerCLIModuleWidget);
  vtkMRMLCommandLineModuleNode* node = d->commandLineModuleNode();
  Q_ASSERT(node);
  d->CLIModuleUIHelper->updateMRMLCommandLineModuleNode(node);
  d->logic()->Apply(node);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::cancel()
{
  qDebug() << "qSlicerCLIModuleWidgetPrivate::onCancelButtonPressed";
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::reset()
{
  Q_D(qSlicerCLIModuleWidget);
  qDebug() << "qSlicerCLIModuleWidgetPrivate::onDefaultButtonPressed";
  vtkMRMLCommandLineModuleNode* node = d->commandLineModuleNode();
  Q_ASSERT(node);
  d->setDefaultNodeValue(node);
}
