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
#include <QMessageBox>
#include <QDebug>

// EMSegment includes
#include "qSlicerEMSegmentDefinePreprocessingStep.h"
#include "ui_qSlicerEMSegmentDefinePreprocessingStep.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSWorkingDataNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentDefinePreprocessingStepPrivate :
                                                       public Ui_qSlicerEMSegmentDefinePreprocessingStep
{
  Q_DECLARE_PUBLIC(qSlicerEMSegmentDefinePreprocessingStep);
protected:
  qSlicerEMSegmentDefinePreprocessingStep* const q_ptr;
public:
  qSlicerEMSegmentDefinePreprocessingStepPrivate(qSlicerEMSegmentDefinePreprocessingStep& object);

  void setTaskPreprocessingSetting();
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefinePreprocessingStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefinePreprocessingStepPrivate::qSlicerEMSegmentDefinePreprocessingStepPrivate(qSlicerEMSegmentDefinePreprocessingStep& object)
  : q_ptr(&object)
{
}

//----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStepPrivate::setTaskPreprocessingSetting()
{
  #ifndef QT_NO_DEBUG
  Q_Q(qSlicerEMSegmentDefinePreprocessingStep);
  #endif
  Q_ASSERT( q->mrmlManager());

  // TODO this needs to be ported to Qt/CTK, depending on how the tcl files will be dealt with

  // vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  // if (!mrmlManager)
  //   {
  //   return;
  //   }

  // if (! mrmlManager->GetNode()->GetTaskPreprocessingSetting()) 
  //   {
  //   return;
  //   }

  // std::string oldText = mrmlManager->GetNode()->GetTaskPreprocessingSetting();

  // vtksys_stl::stringstream defText;

  // for (int i = 0; i < (int)this->checkButton.size(); i++)
  //   {
  //   defText << "|C";
  //   if (this->checkButton[i] && this->checkButton[i]->GetWidget())
  //     {
  //     defText << this->checkButton[i]->GetWidget()->GetSelectedState();
  //     } 
  //   }

  // for (int i =0 ; i < (int) volumeMenuButtonID.size() ; i++)
  //   {
  //   defText << "|V";
  //   if (this->volumeMenuButtonID[i])
  //     {
  //     vtkMRMLVolumeNode* volumeNode = mrmlManager->GetVolumeNode(this->volumeMenuButtonID[i]);
  //     if (!volumeNode) 
  //       {
  //       vtkErrorMacro("Volume Node for ID " << this->volumeMenuButtonID[i] << " does not exists" );
  //       defText << "NULL";
  //       } 
  //     else 
  //       {
  //       defText << volumeNode->GetID();
  //       }
  //     }
  //     else 
  //       {
  //       defText << "NULL";
  //       }
  //     }
  //   }


  // for (int i =0 ; i < (int)  this->textEntry.size(); i++)
  //   {
  //   defText << "|E";
  //   if (this->textEntry[i] && this->textEntry[i]->GetWidget())
  //     {
  //     defText << this->textEntry[i]->GetWidget()->GetValue();
  //     } 
  //   }

  // mrmlManager->GetNode()->SetTaskPreprocessingSetting(defText.str().c_str());
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefinePreprocessingStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefinePreprocessingStep::StepId = "DefinePreprocessing";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefinePreprocessingStep::qSlicerEMSegmentDefinePreprocessingStep(
  ctkWorkflow* newWorkflow, QWidget* newWidget)
  : Superclass(newWorkflow, qSlicerEMSegmentDefinePreprocessingStep::StepId, newWidget)
  , d_ptr(new qSlicerEMSegmentDefinePreprocessingStepPrivate(*this))
{
  Q_D(qSlicerEMSegmentDefinePreprocessingStep);
  d->setupUi(this);

  this->setName("6/9. Define Preprocessing");
  this->setDescription("Answer questions for preprocessing of input images.");
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefinePreprocessingStep::~qSlicerEMSegmentDefinePreprocessingStep()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::createUserInterface()
{
  // TODO: would setup GUI for customized preprocessing here!
  // i.e. in current implementation the preprocessing tcl script is used to place custom
  // preprocessing widgets
  // see Modules/EMSegment/vtkEMSegmentPreProcessingStep::ShowUserInterface()

  this->createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);
  Q_ASSERT(this->mrmlManager());

  vtkEMSegmentMRMLManager* mrmlManager = this->mrmlManager();

  // First determine whether we should do preprocessing.  If the preprocessing hasn't already been
  // done, then we definitely have to do it before continuing.  If the preprocessing has been done,
  // the user has the choice to redo it.

  // If they are still valid then don't repeat preprocessing, unless the user wants to
  if (mrmlManager->GetWorkingDataNode()->GetAlignedTargetNodeIsValid()
      && mrmlManager->GetWorkingDataNode()->GetAlignedAtlasNodeIsValid())
    {
    if (QMessageBox::No == QMessageBox::question(
        this, "EMSegmenter",
        tr("Do you want to redo preprocessing of input images?"),
        QMessageBox::Yes, QMessageBox::No))
      {
      this->validationComplete(true);
      return;
      }
    }
  // If they are not valid, then notify them that preprocessing may take a while.  Validation fails
  // if the user chooses not to do preprocessing, because we can't continue without it.
  else
    {
    if (QMessageBox::Cancel == QMessageBox::question(
        this, "EMSegmenter",
        tr("Start preprocessing of images?\nPreprocessing of images might take a while, "
           "but segmentation cannot continue without preprocessing"),
        QMessageBox::Yes, QMessageBox::Cancel))
      {
      this->validationComplete(false);
      return;
      }
    }

  // TODO setTaskPreprocessingSetting is not implemented, may be necessary in future depending on
  // how the tcl scripts are dealt with
  // d->setTaskPreprocessingSetting();

  // TODO: would do actual preprocessing here!
  // i.e. in current implementation would run tcl script to do the preprocessing
  // see Modules/EMSegment/vtkEMSegmentPreProcessingStep::Validate()

  // To whoever is implementing this:
  // If the preprocessing fails: you probably need the following three lines of code:
  // mrmlManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);
  // mrmlManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(0);
  // this->validationComplete(false);
  // If the preprocessing succeeds: the code at the end of this function performs the equivalent
  // already.

  // Set it to valid so next time we do not have to recompute it 
  mrmlManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(1);
  mrmlManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(1);

  qDebug() << "=============================================";
  qDebug() << "Pre-processing completed successfully";
  qDebug() << "=============================================";

  this->validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);

  // TODO create updateWidgetFromMRML, as in other steps, and implement alongside customized preprocessing

  // Indicates that we are finished
  this->onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);
  Q_UNUSED(transitionType);

  // TODO create updateMRMLFromWidget, as in other steps, and implement alongside customized preprocessing

  // Indicates that we are finished
  this->onExitComplete();
}

