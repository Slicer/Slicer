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
#include <QDebug>
#include <QPushButton>

// EMSegment QTModule includes
#include "qSlicerEMSegmentModuleWidget.h"
#include "ui_qSlicerEMSegmentModule.h"
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

// CTK includes
#include <ctkWorkflowStackedWidget.h>
#include <ctkWorkflowButtonBoxWidget.h>
#include <ctkWorkflowWidgetStep.h>
#include <ctkWorkflow.h>

// EMSegment QTModule includes
#include "qSlicerEMSegmentDefineTaskStep.h"
#include "qSlicerEMSegmentDefineInputChannelsStep.h"
#include "qSlicerEMSegmentDefineAnatomicalTreeStep.h"
#include "qSlicerEMSegmentDefineAtlasStep.h"
#include "qSlicerEMSegmentEditRegistrationParametersStep.h"
#include "qSlicerEMSegmentDefinePreprocessingStep.h"
#include "qSlicerEMSegmentSpecifyIntensityDistributionStep.h"
#include "qSlicerEMSegmentEditNodeBasedParametersStep.h"
#include "qSlicerEMSegmentRunSegmentationStep.h"
#include "qSlicerEMSegmentSegmentationLogicStep.h"

// STD includes
#include <cstdlib>

// EMSegment/Logic includes
#include <vtkSlicerEMSegmentLogic.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentModuleWidgetPrivate: public Ui_qSlicerEMSegmentModule
{
  Q_DECLARE_PUBLIC(qSlicerEMSegmentModuleWidget);
protected:
  qSlicerEMSegmentModuleWidget* const q_ptr;
public:
  qSlicerEMSegmentModuleWidgetPrivate(qSlicerEMSegmentModuleWidget& object);
  ~qSlicerEMSegmentModuleWidgetPrivate();

  vtkSlicerEMSegmentLogic* logic() const;

  ctkWorkflow*                    Workflow;
  ctkWorkflowStackedWidget *      WorkflowWidget;
   
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentModuleWidgetPrivate::qSlicerEMSegmentModuleWidgetPrivate(qSlicerEMSegmentModuleWidget& object)
  : q_ptr(&object)
{
  this->Workflow = 0;
  this->WorkflowWidget = 0;
}

//-------------------------------------------------------------------------------
qSlicerEMSegmentModuleWidgetPrivate::~qSlicerEMSegmentModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerEmSegmentModuleWidget methods

//-----------------------------------------------------------------------------
// qSlicerEMSegmentModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
vtkSlicerEMSegmentLogic* qSlicerEMSegmentModuleWidgetPrivate::logic()const
{
  Q_Q(const qSlicerEMSegmentModuleWidget);
  return vtkSlicerEMSegmentLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentModuleWidget methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentModuleWidget::SimpleMode = "simple";
const QString qSlicerEMSegmentModuleWidget::AdvancedMode = "advanced";

//-----------------------------------------------------------------------------
qSlicerEMSegmentModuleWidget::qSlicerEMSegmentModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerEMSegmentModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentModuleWidget::~qSlicerEMSegmentModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentModuleWidget::setup()
{
  Q_D(qSlicerEMSegmentModuleWidget);
  d->setupUi(this);

  // create the workflow and workflow widget if necessary
  d->Workflow = new ctkWorkflow;

  d->WorkflowWidget = new ctkWorkflowStackedWidget(this);
  d->WorkflowWidget->setWorkflow(d->Workflow);
  d->gridLayout->addWidget(d->WorkflowWidget);

  QList<qSlicerEMSegmentWorkflowWidgetStep*> allSteps;

  // DefineTask Step is common to "simple" and "advanced" modes
  qSlicerEMSegmentWorkflowWidgetStep * defineTaskStep =
      new qSlicerEMSegmentDefineTaskStep(d->Workflow);
  allSteps << defineTaskStep;

  // Step specific to "advanced" mode
  qSlicerEMSegmentWorkflowWidgetStep * defineInputChannelsAdvancedStep =
    new qSlicerEMSegmentDefineInputChannelsStep(d->Workflow,
                                                qSlicerEMSegmentDefineInputChannelsStep::Advanced);
  allSteps << defineInputChannelsAdvancedStep;

  // Step specific to "simple" mode
  qSlicerEMSegmentWorkflowWidgetStep * defineInputChannelsSimpleStep =
    new qSlicerEMSegmentDefineInputChannelsStep(d->Workflow,
        qSlicerEMSegmentDefineInputChannelsStep::Simple);
  allSteps << defineInputChannelsSimpleStep;

  // The following steps are common to "simple" and "advanced" modes
  QList<qSlicerEMSegmentWorkflowWidgetStep*> commonSteps;
  commonSteps << new qSlicerEMSegmentDefineAnatomicalTreeStep(d->Workflow)
              << new qSlicerEMSegmentDefineAtlasStep(d->Workflow)
              << new qSlicerEMSegmentEditRegistrationParametersStep(d->Workflow)
              << new qSlicerEMSegmentDefinePreprocessingStep(d->Workflow)
              << new qSlicerEMSegmentSpecifyIntensityDistributionStep(d->Workflow)
              << new qSlicerEMSegmentEditNodeBasedParametersStep(d->Workflow)
              << new qSlicerEMSegmentRunSegmentationStep(d->Workflow)
              << new qSlicerEMSegmentSegmentationLogicStep(d->Workflow);
  allSteps << commonSteps;

  // Initial step
  d->Workflow->setInitialStep(defineTaskStep);

  // Add transition from DefineTask step to DefineInputChannelsAdvanced step
  d->Workflow->addTransition(defineTaskStep, defineInputChannelsAdvancedStep,
                             qSlicerEMSegmentModuleWidget::AdvancedMode);

  // ... and from DefineInputChannelsAdvanced step to DefineAnatomicalTree step
  d->Workflow->addTransition(defineInputChannelsAdvancedStep, commonSteps.first());

  // Add transition from DefineTask step to DefineInputChannelsSimple step
  d->Workflow->addTransition(defineTaskStep, defineInputChannelsSimpleStep,
                             qSlicerEMSegmentModuleWidget::SimpleMode);

  // ... and from DefineInputChannelsAdvanced to DefineAnatomicalTree step
  d->Workflow->addTransition(defineInputChannelsSimpleStep, commonSteps.first());

  // Add transition associated to common steps
  for(int i = 0; i < commonSteps.size() - 1; ++i)
    {
    d->Workflow->addTransition(commonSteps.value(i),
                               commonSteps.value(i + 1));
    }

  // Assign MRML manager and EMSegment logic to all steps
  foreach(qSlicerEMSegmentWorkflowWidgetStep * step, allSteps)
    {
    step->setMRMLManager(d->logic()->GetMRMLManager());
    step->setEMSegmentLogic(d->logic());
    }

  d->WorkflowWidget->buttonBoxWidget()->setBackButtonDefaultText("");
  d->WorkflowWidget->buttonBoxWidget()->setNextButtonDefaultText("");

  d->Workflow->start();
}

