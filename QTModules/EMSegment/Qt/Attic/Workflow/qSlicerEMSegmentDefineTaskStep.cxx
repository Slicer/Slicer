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

 This file was originally developed by
 Danielle Pace and Jean-Christophe Fillion-Robin, Kitware Inc.
 and was partially funded by NIH grant 3P41RR013218-12S1

 ==============================================================================*/

// Qt includes
#include <QDebug>
#include <QSignalMapper>

// CTK includes
#include <ctkWorkflow.h>

// EMSegment includes
#include "qSlicerEMSegmentDefineTaskStep.h"
#include "ui_qSlicerEMSegmentDefineTaskStep.h"
#include "qSlicerEMSegmentDefineTaskStep_p.h"
#include "qSlicerEMSegmentModuleWidget.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkEMSegmentLogic.h>
#include <vtkMRMLEMSTemplateNode.h>

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineTaskStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineTaskStepPrivate::qSlicerEMSegmentDefineTaskStepPrivate(qSlicerEMSegmentDefineTaskStep& object) :
  q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineTaskStepPrivate::setupUi(qSlicerEMSegmentWorkflowWidgetStep* step)
{
  this->Ui_qSlicerEMSegmentDefineTaskStep::setupUi(step);

  Q_Q( qSlicerEMSegmentDefineTaskStep);

  // Connect TaskSelector
  connect(this->TaskSelectorComboBox, SIGNAL(currentIndexChanged(int index)), this,
      SLOT(q->LoadDefaultTask(int index)));

  // Each button is mapped with the corresponding branch name
  this->Mapper.setMapping(this->SimpleModeButton,
      qSlicerEMSegmentModuleWidget::SimpleMode);
  this->Mapper.setMapping(this->AdvancedModeButton,
      qSlicerEMSegmentModuleWidget::AdvancedMode);

  // Connect buttons
  connect(this->SimpleModeButton, SIGNAL(clicked()), &this->Mapper, SLOT(map()));
  connect(this->AdvancedModeButton, SIGNAL(clicked()), &this->Mapper, SLOT(
      map()));

connect(&this->Mapper, SIGNAL(mapped(const QString)), this, SIGNAL(modeChanged(const QString&)));

}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineTaskStepPrivate::selectTask()
{
  Q_Q( qSlicerEMSegmentDefineTaskStep);
  Q_ASSERT(q->mrmlManager());
  Q_ASSERT(q->emSegmentLogic());
  Q_ASSERT(q->emSegmentLogic()->GetMRMLScene());

//q->mrmlManager()->SetLoadedParameterSetIndex(
//      vtkMRMLEMSTemplateNode::SafeDownCast(q->emSegmentLogic->GetMRMLScene()->));

  //q->mrmlManager()->SetNodeWithCheck(vtkMRMLEMSTemplateNode::SafeDownCast(mrmlNode));
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineTaskStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefineTaskStep::StepId = "DefineTask";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineTaskStep::qSlicerEMSegmentDefineTaskStep(ctkWorkflow* newWorkflow, QWidget* newWidget) :
  Superclass(newWorkflow, qSlicerEMSegmentDefineTaskStep::StepId, newWidget),
      d_ptr(new qSlicerEMSegmentDefineTaskStepPrivate(*this))
{
  Q_D( qSlicerEMSegmentDefineTaskStep);
  d->setupUi(this);

  this->setName("1/9. Define Task");
  this->setDescription("Select a (new) task.");
  this->setButtonBoxHints(ctkWorkflowWidgetStep::ButtonBoxHidden);

}

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineTaskStep::~qSlicerEMSegmentDefineTaskStep()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineTaskStep::createUserInterface()
{
  Q_D( qSlicerEMSegmentDefineTaskStep);
  connect(d, SIGNAL(modeChanged(const QString&)), this->workflow(), SLOT(goForward(const QString&)));

  this->showDefaultTasksList();

  this->createUserInterfaceComplete();
}

//-------------vtksys_stl::string ---------------------------------------------------------------
void qSlicerEMSegmentDefineTaskStep::showDefaultTasksList()
{

  Q_D( qSlicerEMSegmentDefineTaskStep);

  Q_ASSERT(this->emSegmentLogic());

  this->emSegmentLogic()->CreateDefaultTasksList(this->pssDefaultTasksName,
      this->pssDefaultTasksFile, this->DefinePreprocessingTasksName,
      this->DefinePreprocessingTasksFile);

  if (!this->pssDefaultTasksFile.size())
    {
    qWarning("No default tasks found");
    }

  this->pssDefaultTasksFile.push_back(vtksys_stl::string(""));
  this->pssDefaultTasksName.push_back("Create new task");

//  for (unsigned int i=0; i<this->pssDefaultTasksName.size();++i)
//    {
//
//    d->TaskSelectorComboBox->addItem(QString(this->pssDefaultTasksName[i].c_str()));
//
//    }

}

//-----------------------------------------------------------------------------
int qSlicerEMSegmentDefineTaskStep::loadDefaultTask(int index)
{
  // cout << "vtkEMSegmentParametersSetStep::LoadDefaultTask " << index << " " << warningFlag << endl;
  if (index < 0 || index >  int(this->pssDefaultTasksName.size() -2) )
    {
      qErrnoWarning("Index is not defined");
      return 1;
    }


  // Load Task
  vtkEMSegmentMRMLManager *mrmlManager = this->mrmlManager();
  //if (this->LoadDefaultData(pssDefaultTasksFile[index].c_str(),warningFlag))
  //  {
      // Error occurred
    //  return 1;
  //  }


  // Remove the default selection entry from the menu,
  //this->PopulateLoadedParameterSets();

  // Figure out the menu index number of the default task that was just loaded
  // and go to the next step
  int numSets = mrmlManager->GetNumberOfParameterSets();
  for(int i = 0; i < numSets; i++)
    {
    const char *name = mrmlManager->GetNthParameterSetName(i);
    if (name && !strcmp(name,pssDefaultTasksName[index].c_str()))
      {
      // Select the Node-
      //this->SelectedParameterSetChangedCallback(i,0);
      break;
      }
    }
  return 0;
}
