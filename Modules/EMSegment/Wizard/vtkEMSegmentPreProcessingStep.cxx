#include "vtkEMSegmentPreProcessingStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkKWMessageDialog.h"
#include "vtkEMSegmentLogic.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrameWithLabel.h" 
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSNode.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWEntryWithLabel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentPreProcessingStep);
vtkCxxRevisionMacro(vtkEMSegmentPreProcessingStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentPreProcessingStep::vtkEMSegmentPreProcessingStep()
{
  this->SetName("6/9. Define Preprocessing");
  this->SetDescription("Answer questions for preprocessing of input images");
  this->askQuestionsBeforeRunningPreprocessingFlag = 1;
}

//----------------------------------------------------------------------------
vtkEMSegmentPreProcessingStep::~vtkEMSegmentPreProcessingStep()
{
}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  //
  // Source TCL Files 
  //
  if (this->SourcePreprocessingTclFiles())
    {
      return;
    }
 
  //
  // Define General Framework For GUI 
  //

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  if (!mrmlManager || !wizard_widget)
    {
    return;
    }

  vtkKWWidget *parent = wizard_widget->GetClientArea();;
  int enabled = parent->GetEnabled();
  wizard_widget->GetCancelButton()->SetEnabled(enabled);

  if (!this->CheckListFrame)
    {
    this->CheckListFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->CheckListFrame->IsCreated())
    {
    this->CheckListFrame->SetParent(parent);
    this->CheckListFrame->Create();
    this->CheckListFrame->SetLabelText("Check List");
    }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->CheckListFrame->GetWidgetName());

  //
  // Define Task Specific GUI 
  //

  this->CreateEntryLists();
  this->Script("::EMSegmenterPreProcessingTcl::ShowUserInterface");

}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::Validate()
{
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWorkflow *wizard_workflow = this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  // If they are still valid do not repeat preprocessing unless otherwhise wanted 
  // Kilian - still to do - save intermediate results 
  // so do special check here 
  if (this->askQuestionsBeforeRunningPreprocessingFlag)
    {
      if (mrmlManager->GetWorkingDataNode()->GetAlignedTargetNodeIsValid() && mrmlManager->GetWorkingDataNode()->GetAlignedAtlasNodeIsValid())
    {
      // If it is ask if preprocessing should be done again
      if (!vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "Redo Preprocessing of images?",
                          "Do you want to redo preprocessing of input images ?", 
                          vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
             {
                 // If not just proceed
                 this->Superclass::Validate();
                 return;
             }
           } else {
    
             if (!vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "Start Preprocessing of images?",
                           "Preprocessing of images might take a while. Do you want to proceed ?", 
                           vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
               {
                  wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
                  wizard_workflow->ProcessInputs();
                  return;
               }
          }
    }
  this->SetTaskPreprocessingSetting();

  int flag = atoi(this->Script("::EMSegmenterPreProcessingTcl::Run"));
  if (flag)
    {
      cout << "Pre-processing did not execute correctly" << endl;
      wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
      wizard_workflow->ProcessInputs();
      return;
    } 

    // Set it to valid so next time we do not have to recompute it 
    mrmlManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(1);
    mrmlManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(1);

    cout << "=============================================" << endl;
    cout << "Pre-processing completed successfully" << endl;
    cout << "=============================================" << endl;

  // Everything went smoothly
  this->Superclass::Validate();
}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::SetTaskPreprocessingSetting()
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
      return;
    }

  if (! mrmlManager->GetNode()->GetTaskPreprocessingSetting()) 
    {
      return;
    }

  std::string oldText = mrmlManager->GetNode()->GetTaskPreprocessingSetting();

  vtksys_stl::stringstream defText;

  for (int i =0 ; i < (int)  this->checkButton.size(); i++)
    {
      defText << "|C";
      if (this->checkButton[i] && this->checkButton[i]->GetWidget())
    {
      defText << this->checkButton[i]->GetWidget()->GetSelectedState();
    } 
    }

  for (int i =0 ; i < (int) volumeMenuButtonID.size() ; i++)
    {
       defText << "|V";
       if (this->volumeMenuButtonID[i]) {
     vtkMRMLVolumeNode* volumeNode = mrmlManager->GetVolumeNode(this->volumeMenuButtonID[i]);
     if (!volumeNode) 
       {
         vtkErrorMacro("Volume Node for ID " << this->volumeMenuButtonID[i] << " does not exists" );
         defText << "NULL";
       } 
     else 
       {
         defText << volumeNode->GetID();
       }
       }
      else 
    {
      defText << "NULL";
    }
    }


  for (int i =0 ; i < (int)  this->textEntry.size(); i++)
    {
      defText << "|E";
      if (this->textEntry[i] && this->textEntry[i]->GetWidget())
    {
      defText << this->textEntry[i]->GetWidget()->GetValue();
    } 
    }

  mrmlManager->GetNode()->SetTaskPreprocessingSetting(defText.str().c_str());
}



