/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

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
qSlicerCLIModuleWidgetPrivate::qSlicerCLIModuleWidgetPrivate()
{
  this->ProcessInformation = 0;
  this->Name = "NA";
  this->CommandLineModuleNode = 0;
  this->CLIModuleUIHelper = 0;
}
    
//-----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic* qSlicerCLIModuleWidgetPrivate::logic()const
{
  CTK_P(const qSlicerCLIModuleWidget);
  return vtkSlicerCLIModuleLogic::SafeDownCast(p->logic());
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
  CTK_P(qSlicerCLIModuleWidget);
  
  this->Ui_qSlicerCLIModule::setupUi(widget);

  this->MainCollapsibleWidget->setText(this->Title);

  this->addParameterGroups();

  // Connect buttons
  this->connect(this->ApplyPushButton,
                SIGNAL(pressed()),
                SLOT(onApplyButtonPressed()));
                
  this->connect(this->CancelPushButton,
                SIGNAL(pressed()),
                SLOT(onCancelButtonPressed()));
                
  this->connect(this->DefaultPushButton,
                SIGNAL(pressed()),
                SLOT(onDefaultButtonPressed()));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(currentNodeChanged(bool)),
                SLOT(enableCommandButtonState(bool)));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                p, 
                SLOT(setCurrentCommandLineModuleNode(vtkMRMLNode*)));
                
  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
                SLOT(setDefaultNodeValue(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::onApplyButtonPressed()
{
  vtkMRMLCommandLineModuleNode* node = this->commandLineModuleNode();
  Q_ASSERT(node);
  this->CLIModuleUIHelper->updateMRMLCommandLineModuleNode(node);
  this->logic()->Apply(node);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::onCancelButtonPressed()
{
  qDebug() << "qSlicerCLIModuleWidgetPrivate::onCancelButtonPressed";
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::onDefaultButtonPressed()
{
  qDebug() << "qSlicerCLIModuleWidgetPrivate::onDefaultButtonPressed";
  vtkMRMLCommandLineModuleNode* node = this->commandLineModuleNode();
  Q_ASSERT(node);
  this->setDefaultNodeValue(node);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::enableCommandButtonState(bool enable)
{
  this->ApplyPushButton->setEnabled(enable);
  this->CancelPushButton->setEnabled(enable);
  this->DefaultPushButton->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::updateUi(vtkObject* commandLineModuleNode)
{
  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);

  this->CLIModuleUIHelper->updateUi(node);
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
// qSlicerCLIModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerCLIModuleWidget::qSlicerCLIModuleWidget(
  ModuleDescription* desc, QWidget* _parent):Superclass(_parent)
{
  Q_ASSERT(desc);
  CTK_INIT_PRIVATE(qSlicerCLIModuleWidget);
  CTK_D(qSlicerCLIModuleWidget);

  d->ModuleDescriptionObject = *desc;

  d->CLIModuleUIHelper = new qSlicerCLIModuleUIHelper(this);

  // Set properties
  d->Title = QString::fromStdString(desc->GetTitle());
  d->Contributor = QString::fromStdString(desc->GetContributor());
  d->Category = QString::fromStdString(desc->GetCategory());

  d->ProcessInformation = desc->GetProcessInformation();
  d->ParameterGroups = desc->GetParameterGroups();
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setup()
{
  CTK_D(qSlicerCLIModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setCurrentCommandLineModuleNode(
  vtkMRMLNode* commandLineModuleNode)
{
  CTK_D(qSlicerCLIModuleWidget);
  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  if (node == d->CommandLineModuleNode)
    {
    return;
    }
    
  // Connect node modified event
  this->qvtkReconnect(d->CommandLineModuleNode, node,
    vtkCommand::ModifiedEvent,
    d, SLOT(updateUi(vtkObject*)));
    
  d->CLIModuleUIHelper->updateUi(node);
  d->CommandLineModuleNode = node;
}

//-----------------------------------------------------------------------------
CTK_SET_CXX(qSlicerCLIModuleWidget, const QString&, setModuleEntryPoint, ModuleEntryPoint);
