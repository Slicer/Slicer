#include "vtkEMSegmentPreProcessingStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWProgressDialog.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"
#include "vtkMRMLEMSGlobalParametersNode.h"
#include "vtkEMSegmentKWDynamicFrame.h"
#include "vtkKWTkUtilities.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentPreProcessingStep);
vtkCxxRevisionMacro(vtkEMSegmentPreProcessingStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentPreProcessingStep::vtkEMSegmentPreProcessingStep()
{
  this->SetName("6/9. Define Preprocessing");
  this->SetDescription("Answer questions for preprocessing of input images");
  this->askQuestionsBeforeRunningPreprocessingFlag = 1;
  // Have to do it right here bc of Generic.tcl - InitializeVariables ! 
  this->CheckListFrame = vtkEMSegmentKWDynamicFrame::New();
}

//----------------------------------------------------------------------------
vtkEMSegmentPreProcessingStep::~vtkEMSegmentPreProcessingStep()
{
  if (this->CheckListFrame )
    {
      this->CheckListFrame->Delete();
      this->CheckListFrame = NULL;
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentPreProcessingStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();
  
  //
  // Source TCL Files 
  //
  //----------------------------------------------------------------------------
   if (this->GUI->GetLogic()->SourcePreprocessingTclFiles())
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
      this->CheckListFrame = vtkEMSegmentKWDynamicFrame::New();
      }
    if (!this->CheckListFrame->IsCreated())
      {
         this->CheckListFrame->SetParent(parent);
         this->CheckListFrame->Create();
         this->CheckListFrame->SetLabelText("Check List");
         this->CheckListFrame->SetMRMLManager(mrmlManager);
      }

  if (this->GetGUI()->IsSegmentationModeAdvanced()) {
          this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->CheckListFrame->GetWidgetName());
     }

  // Sets up Task Specific GUI
  this->CheckListFrame->CreateEntryLists();
  
  const char* logicTcl = vtkKWTkUtilities::GetTclNameFromPointer(vtkSlicerApplication::GetInstance()->GetMainInterp(),this->GUI->GetLogic());

  std::string showCheckListCommand = "::EMSegmenterPreProcessingTcl::ShowUserInterface ";
  showCheckListCommand += logicTcl;

 this->Script(showCheckListCommand.c_str());

}

//----------------------------------------------------------------------------
void
vtkEMSegmentPreProcessingStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentPreProcessingStep::Validate()
{
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWorkflow *wizard_workflow = this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  // If they are still valid do not repeat preprocessing unless otherwise wanted 
  // Kilian - still to do - save intermediate results 
  // so do special check here 

  if ( mrmlManager->GetRegistrationPackageType() == mrmlManager->GetPackageTypeFromString("CMTK") ) {
    const char* path = this->Script("::EMSegmenterPreProcessingTcl::Get_CMTK_Installation_Path");
    if ( *path == '\0' ) {
      if (!vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "CMTK is not installed",
                                          "\nDo you want to proceed with BRAINSTools instead?",
                                          vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
        {
          wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
          wizard_workflow->ProcessInputs();
          return;
        }
    }
  }

  if ( mrmlManager->GetRegistrationPackageType() == mrmlManager->GetPackageTypeFromString("PLASTIMATCH") ) {
    const char* path = this->Script("::EMSegmenterPreProcessingTcl::Get_PLASTIMATCH_Installation_Path");
    if ( *path == '\0' ) {
      if (!vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "PLASTIMATCH is not installed",
                                          "\nDo you want to proceed with BRAINSTools instead?",
                                          vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
        {
          wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
          wizard_workflow->ProcessInputs();
          return;
        }
    }
  }

  if ( mrmlManager->GetRegistrationPackageType() == mrmlManager->GetPackageTypeFromString("DEMONS") ) {
    const char* path = this->Script("::EMSegmenterPreProcessingTcl::Get_DEMONS_Installation_Path");
    if ( *path == '\0' ) {
      if (!vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "DEMONS is not installed",
                                          "\nDo you want to proceed with BRAINSTools instead?",
                                          vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
        {
          wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
          wizard_workflow->ProcessInputs();
          return;
        }
    }
  }

  if ( mrmlManager->GetRegistrationPackageType() == mrmlManager->GetPackageTypeFromString("DRAMMS") ) {
    const char* path = this->Script("::EMSegmenterPreProcessingTcl::Get_DRAMMS_Installation_Path");
    if ( *path == '\0' ) {
      if (!vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "DRAMMS is not installed",
                                          "\nDo you want to proceed with BRAINSTools instead?",
                                          vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
        {
          wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
          wizard_workflow->ProcessInputs();
          return;
        }
    }
  }

  if ( mrmlManager->GetRegistrationPackageType() == mrmlManager->GetPackageTypeFromString("ANTS") ) {
    const char* path = this->Script("::EMSegmenterPreProcessingTcl::Get_ANTS_Installation_Path");
    if ( *path == '\0' ) {
      if (!vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "ANTS is not installed",
                                          "\nDo you want to proceed with BRAINSTools instead?",
                                          vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
        {
          wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
          wizard_workflow->ProcessInputs();
          return;
        }
    }
  }

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
    this->CheckListFrame->SaveSettingToMRML();

    vtkKWProgressDialog* progress = vtkKWProgressDialog::New();
    progress->SetParent(this->GetGUI ()->GetApplicationGUI ()->GetMainSlicerWindow ());
    progress->SetMasterWindow (this->GetGUI ()->GetApplicationGUI ()->GetMainSlicerWindow());
    progress->Create();
    progress->SetMessageText("Please wait until pre-processing has finished.");
    progress->Display();
    int flag = atoi(this->Script("::EMSegmenterPreProcessingTcl::Run"));
    progress->SetParent(NULL);
    progress->Delete();
    
    if (flag)
    {
      vtkKWMessageDialog::PopupMessage(this->GetApplication(), NULL,
        "Error", "Pre-processing did not execute correctly",
        vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer
      );
      cerr << "Pre-processing did not execute correctly" << endl;
      wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
      wizard_workflow->ProcessInputs();
      return;
    } 

    // Set it to valid so next time we do not have to recompute it 
    mrmlManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(1);
    mrmlManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(1);

    vtkMRMLEMSVolumeCollectionNode* targetNode = this->GetGUI()->GetMRMLManager()->GetWorkingDataNode()->GetInputTargetNode();
    if (targetNode) 
      {
        vtkMRMLVolumeNode* output =  targetNode->GetNthVolumeNode(0);
        vtkSlicerApplicationGUI *applicationGUI = this->GetGUI ()->GetApplicationGUI ();

        applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetBackgroundSelector()->SetSelected(output);
        applicationGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetBackgroundSelector()->SetSelected(output);
        applicationGUI->GetMainSliceGUI("Green")->GetSliceController()->GetBackgroundSelector()->SetSelected(output);
      }
  
    cout << "=============================================" << endl;
    cout << "Pre-processing completed successfully" << endl;
    cout << "=============================================" << endl;

    // Everything went smoothly
    this->Superclass::Validate();
}

