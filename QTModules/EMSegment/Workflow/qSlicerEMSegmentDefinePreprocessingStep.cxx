
// Qt includes
#include <QMessageBox>
//#include <QObject>

// EMSegment includes
#include "qSlicerEMSegmentDefinePreprocessingStep.h"
#include "qSlicerEMSegmentDefinePreprocessingPanel.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSWorkingDataNode.h>
//#include <vtkMRMLEMSNode.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefinePreprocessingStepPrivate : public ctkPrivate<qSlicerEMSegmentDefinePreprocessingStep>
{
public:
  qSlicerEMSegmentDefinePreprocessingStepPrivate();

  void setTaskPreprocessingSetting();

  qSlicerEMSegmentDefinePreprocessingPanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefinePreprocessingStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefinePreprocessingStepPrivate::qSlicerEMSegmentDefinePreprocessingStepPrivate()
{
  this->panel = 0;
}

//----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStepPrivate::setTaskPreprocessingSetting()
{
  CTK_P(qSlicerEMSegmentDefinePreprocessingStep);
  Q_ASSERT(p->mrmlManager());

  // TODO this needs to be ported to Qt/CTK, depending on how the tcl files will be dealt with

  // vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  // if (!mrmlManager)
  //   {
  //     return;
  //   }

  // if (! mrmlManager->GetNode()->GetTaskPreprocessingSetting()) 
  //   {
  //     return;
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
  //      defText << "|V";
  //      if (this->volumeMenuButtonID[i]) {
  //    vtkMRMLVolumeNode* volumeNode = mrmlManager->GetVolumeNode(this->volumeMenuButtonID[i]);
  //    if (!volumeNode) 
  //      {
  //        vtkErrorMacro("Volume Node for ID " << this->volumeMenuButtonID[i] << " does not exists" );
  //        defText << "NULL";
  //      } 
  //    else 
  //      {
  //        defText << volumeNode->GetID();
  //      }
  //      }
  //     else 
  //   {
  //     defText << "NULL";
  //   }
  //   }


  // for (int i =0 ; i < (int)  this->textEntry.size(); i++)
  //   {
  //     defText << "|E";
  //     if (this->textEntry[i] && this->textEntry[i]->GetWidget())
  //   {
  //     defText << this->textEntry[i]->GetWidget()->GetValue();
  //   } 
  //   }

  // mrmlManager->GetNode()->SetTaskPreprocessingSetting(defText.str().c_str());
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefinePreprocessingStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefinePreprocessingStep::StepId = "DefinePreprocessing";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefinePreprocessingStep::qSlicerEMSegmentDefinePreprocessingStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefinePreprocessingStep);
  this->setName("6/9. Define Preprocessing");
  this->setDescription("Answer questions for preprocessing of input images.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentDefinePreprocessingStep);

  // TODO: would setup GUI for customized preprocessing here!
  // i.e. in current implementation the preprocessing tcl script is used to place custom
  // preprocessing widgets
  // see Modules/EMSegment/vtkEMSegmentPreProcessingStep::ShowUserInterface()

  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentDefinePreprocessingPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->panel;

  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  CTK_D(qSlicerEMSegmentDefinePreprocessingStep);
  Q_ASSERT(this->mrmlManager());

  vtkEMSegmentMRMLManager* mrmlManager = this->mrmlManager();

  // First determine whether we should do preprocessing.  If the preprocessing hasn't already been
  // done, then we definitely have to do it before continuing.  If the preprocessing has been done,
  // the user has the choice to redo it.

  // If they are still valid then don't repeat preprocessing, unless the user wants to
  if (mrmlManager->GetWorkingDataNode()->GetAlignedTargetNodeIsValid()
      && mrmlManager->GetWorkingDataNode()->GetAlignedAtlasNodeIsValid())
    {
    if (QMessageBox::No == QMessageBox::question(d->panel, "EMSegmenter",
                                                 tr("Do you want to redo preprocessing of input images?"),
                                                 QMessageBox::Yes, QMessageBox::No))
      {
      emit validationComplete(true);
      return;
      }
    }
  // If they are not valid, then notify them that preprocessing may take a while.  Validation fails
  // if the user chooses not to do preprocessing, because we can't continue without it.
  else
    {
    if (QMessageBox::No == QMessageBox::question(d->panel, "EMSegmenter",
                                                 tr("Start preprocessing of images?  Preprocessing of images might take a while, but segmentation cannot continue without preprocessing"),
                                                 QMessageBox::Yes, QMessageBox::Cancel))
      {
      emit validationComplete(false);
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
  // emit validationComplete(false);
  // If the preprocessing succeeds: the code at the end of this function performs the equivalent
  // already.

  // Set it to valid so next time we do not have to recompute it 
  mrmlManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(1);
  mrmlManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(1);

  cout << "=============================================" << endl;
  cout << "Pre-processing completed successfully" << endl;
  cout << "=============================================" << endl;

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Don't forget to call the superclass's function
  this->Superclass::onEntry(comingFrom, transitionType);

  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onExitComplete();
}

