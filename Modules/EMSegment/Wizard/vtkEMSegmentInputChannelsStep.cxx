#include "vtkEMSegmentInputChannelsStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWListBoxWithScrollbarsWithLabel.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkMRMLEMSTargetNode.h"
#include "vtkMRMLEMSNode.h"
#include "vtkEMSegmentPreProcessingStep.h"
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentInputChannelsStep);
vtkCxxRevisionMacro(vtkEMSegmentInputChannelsStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkEMSegmentInputChannelsStep::vtkEMSegmentInputChannelsStep()
{
  this->InputChannelDefineFrame   = NULL;

  this->InputChannelDefineLineFrame.clear();
  this->InputChannelDefineLineVolume.clear();
  this->InputChannelDefineLineName.clear();

  this->InputAddRemoveChannelFrame   = NULL;
  this->AddInputChannelButton = NULL;
  this->RemoveInputChannelButton = NULL;

  this->TargetToTargetRegistrationFrame             = NULL;
  this->IntensityImagesAlignTargetImagesCheckButton = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentInputChannelsStep::~vtkEMSegmentInputChannelsStep()
{
  if (this->InputChannelDefineFrame)
    {
    this->InputChannelDefineFrame->Delete();
    this->InputChannelDefineFrame = NULL;
    }

  if (this->InputAddRemoveChannelFrame)
    {
      this->InputAddRemoveChannelFrame->Delete();
      this->InputAddRemoveChannelFrame = NULL;
    }


  if (this->AddInputChannelButton)
    {
      this->AddInputChannelButton->Delete();
      this->AddInputChannelButton = NULL;
    }

  if (this->RemoveInputChannelButton)
    {
      this->RemoveInputChannelButton->Delete();
      this->RemoveInputChannelButton = NULL;
    }

  this->DeleteAllInputChannelFrames();

  if (this->TargetToTargetRegistrationFrame)
    {
    this->TargetToTargetRegistrationFrame->Delete();
    this->TargetToTargetRegistrationFrame = NULL;
    }

  if (this->IntensityImagesAlignTargetImagesCheckButton)
    {
    this->IntensityImagesAlignTargetImagesCheckButton->Delete();
    this->IntensityImagesAlignTargetImagesCheckButton = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();
  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  if (this->GetGUI()->IsSegmentationModeAdvanced())
    {
       wizardWidget->SetTitle("2/9. Define Input Channels");
       wizardWidget->SetSubTitle("Name the input channels and choose the set of scans for segmentation.");
       wizardWidget->SetNextButtonVisibility(1);
    } 
  else 
    {
       wizardWidget->SetTitle("2/2. Define Input Channels");
       wizardWidget->SetSubTitle("Choose the set of scans for segmentation.");
       wizardWidget->SetNextButtonVisibility(0);
    }

  wizardWidget->SetBackButtonVisibility(1);
  wizardWidget->SetFinishButtonVisibility(1);

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  if (!mrmlManager || !wizard_widget)
    {
    return;
    }
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizard_widget->GetClientArea();
  int enabled = parent->GetEnabled();

  // Create the frame

  if (!this->InputChannelDefineFrame)
    {
    this->InputChannelDefineFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->InputChannelDefineFrame->IsCreated())
    {
    this->InputChannelDefineFrame->SetParent(parent);
    this->InputChannelDefineFrame->Create();
    this->InputChannelDefineFrame->SetLabelText("Define Input Channels");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
    this->InputChannelDefineFrame->GetWidgetName());

  this->UpdateInputChannelsfromMRML();

  if (this->GetGUI()->IsSegmentationModeAdvanced()) 
    {
    if (!this->InputAddRemoveChannelFrame)
      {
    this->InputAddRemoveChannelFrame = vtkKWFrame::New();
      }
    if (!this->InputAddRemoveChannelFrame->IsCreated())
      {
    this->InputAddRemoveChannelFrame->SetParent(this->InputChannelDefineFrame->GetFrame());
    this->InputAddRemoveChannelFrame->Create();
      }

    if (this->GetGUI()->IsSegmentationModeAdvanced())
      {
    this->Script("pack %s -side bottom -anchor nw -fill x -padx 0 -pady 2", this->InputAddRemoveChannelFrame->GetWidgetName());
      }

    if (!this->AddInputChannelButton) 
      {
    this->AddInputChannelButton = vtkKWPushButton::New ();
      }
    if (!this->AddInputChannelButton->IsCreated())  
      {
    this->AddInputChannelButton->SetParent(this->InputAddRemoveChannelFrame);
    this->AddInputChannelButton->Create ( );
    this->AddInputChannelButton->SetText ("Add Channel");
    this->AddInputChannelButton->SetCommand (this, "AddInputChannel");
      }

    this->Script("pack %s -side left -padx 4 -anchor c",  this->AddInputChannelButton->GetWidgetName());
  
    if (!this->RemoveInputChannelButton)
      {
    this->RemoveInputChannelButton = vtkKWPushButton::New();
      }
    if (!this->RemoveInputChannelButton->IsCreated())
      {
    this->RemoveInputChannelButton->SetParent(this->InputAddRemoveChannelFrame);
    this->RemoveInputChannelButton->Create ( );
    this->RemoveInputChannelButton->SetText ("Remove Channel");
    this->RemoveInputChannelButton->SetCommand (this, "RemoveInputChannel");
      }
    this->Script("pack %s -side left -padx 4 -anchor c",  this->RemoveInputChannelButton->GetWidgetName());
    } else {
      // To remove it from GUI when switching between advanced and simple mode 
      if (this->InputAddRemoveChannelFrame )
    {
      this->InputAddRemoveChannelFrame->Unpack();
      this->InputAddRemoveChannelFrame->Delete();
      this->InputAddRemoveChannelFrame = NULL;
    }
      if (this->AddInputChannelButton )
    {
      this->AddInputChannelButton->Delete(); 
      this->AddInputChannelButton= NULL;
    } 
      if (this->RemoveInputChannelButton )
    {
      this->RemoveInputChannelButton->Delete(); 
      this->RemoveInputChannelButton= NULL;
    } 
    }



  // Update the UI with the proper value
  //this->PopulateIntensityImagesTargetVolumeSelector();

 if (!this->TargetToTargetRegistrationFrame)
    {
    this->TargetToTargetRegistrationFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->TargetToTargetRegistrationFrame->IsCreated())
    {
    this->TargetToTargetRegistrationFrame->SetParent(parent);
    this->TargetToTargetRegistrationFrame->Create();
    this->TargetToTargetRegistrationFrame->SetLabelText("Input-to-Input Channel Registration");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
    this->TargetToTargetRegistrationFrame->GetWidgetName());

  if (!this->IntensityImagesAlignTargetImagesCheckButton)
    {
    this->IntensityImagesAlignTargetImagesCheckButton = 
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->IntensityImagesAlignTargetImagesCheckButton->IsCreated())
    {
    this->IntensityImagesAlignTargetImagesCheckButton->SetParent(
      this->TargetToTargetRegistrationFrame->GetFrame());
    this->IntensityImagesAlignTargetImagesCheckButton->Create();
    this->IntensityImagesAlignTargetImagesCheckButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityImagesAlignTargetImagesCheckButton->
      SetLabelText("Align Input Scans:");
    this->IntensityImagesAlignTargetImagesCheckButton->
      GetWidget()->SetCommand(this, "AlignTargetImagesCallback");
    }
  this->IntensityImagesAlignTargetImagesCheckButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->IntensityImagesAlignTargetImagesCheckButton->GetWidgetName());

  this->IntensityImagesAlignTargetImagesCheckButton->
    GetWidget()->SetSelectedState(
      mrmlManager->GetEnableTargetToTargetRegistration());

  // wizard_widget->SetErrorText("Please note that the order of the images is important.");

  // -----------------------------------------
  //

  if (!this->GetGUI()->IsSegmentationModeAdvanced()) 
    {
      this->SourceTaskFiles();
      
      int showCheckList = atoi(this->Script("::EMSegmenterSimpleTcl::CreateCheckList"));
      // cout << "showCheckList: " <<  showCheckList << endl;
      if (showCheckList) 
      {
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

          this->CreateEntryLists();
          this->Script("::EMSegmenterSimpleTcl::ShowCheckList");
      }
   }
}

 

//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::AlignTargetImagesCallback(int state)
{
  // The align target images checkbutton has changed because of user
  // interaction
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetEnableTargetToTargetRegistration(state);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::Validate()
{
  vtkKWWizardWorkflow *wizard_workflow = this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  //-----------------------------------------------
  // Check if at least one input channel is defined 
  if (!this->GetNumberOfInputChannels())
     {
       vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Input Channel Error", "Please define at least one input channel",
                        vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
       wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
       wizard_workflow->ProcessInputs();
       return;
     }
  //-----------------------------------------------
  // General 
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkMRMLEMSTargetNode *inputNodes = mrmlManager->GetTargetInputNode();
  if (!inputNodes) 
    {
      vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Input Channel Error", "Internal Error",
                        vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
      wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
      wizard_workflow->ProcessInputs();
      return;
    }
     
   //-----------------------------------------------
   // decide if the number of target volumes changed
   if (this->GetNumberOfInputChannels() != inputNodes->GetNumberOfVolumes() && 
       !vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "Change the number of input channels?",
                       "Are you sure you want to change the number of input images?", 
                       vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
     {
       // don't change number of volumes; stay on this step
       wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
       wizard_workflow->ProcessInputs();
       return;
     }

   //-----------------------------------------------
   // Check if currently defined input volumes are non-negative
   std::vector<const char*> failedTestNodes;

   for (int i=0;  i < this->GetNumberOfInputChannels(); i++)
     {
       vtkMRMLVolumeNode* cNode = vtkMRMLVolumeNode::SafeDownCast(this->InputChannelDefineLineVolume[i]->GetSelected());
       if (!cNode)
       {
           vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Input Channel Error", "Please assign an volume to each input channel",
                        vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
           wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
           wizard_workflow->ProcessInputs();
           return;
       }
       if (!cNode->GetImageData()) 
     {
       std::stringstream errorMessage;
       errorMessage <<  "Volume of " << i + 1 << "th Input channel is empty !";
           vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Input Channel Error", errorMessage.str().c_str(), vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
           wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
           wizard_workflow->ProcessInputs();
           return;
     }

       if (cNode->GetImageData()->GetScalarRange()[0] < 0 )
       {
         failedTestNodes.push_back(cNode->GetName());
       }
     }
    
   if (!failedTestNodes.empty())
     {
       std::stringstream errorMessage;
       errorMessage <<  "EMSegmenter can currently only process non-negative input images. The following images have negative values:\n";
       int size = failedTestNodes.size();
       for (signed int i = 0 ; i < size; i++ ) errorMessage  << failedTestNodes[i] <<  " ";       
       vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Input Channel Error", errorMessage.str().c_str(),
                       vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
       wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
       wizard_workflow->ProcessInputs();
       return;
     } 
   //-----------------------------------------------
   // Check if names are defined for input channel
    for (int i=0;  i < this->GetNumberOfInputChannels(); i++)
     {
       if (!this->InputChannelDefineLineName[i]->GetWidget()->GetValue() || !strcmp(this->InputChannelDefineLineName[i]->GetWidget()->GetValue(),""))
     {
       vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Input Channel Error", "Please specify a name for each input channel",
                        vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
       wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
       wizard_workflow->ProcessInputs();
       return;
     }
     }

   //-----------------------------------------------
   // Check if they are all different and update MRML Nodes 

   // Removes all the input channels
   // std::vector<vtkIdType> resetNull;
   // mrmlManager->ResetTargetSelectedVolumes(resetNull);

   for (int i=0;  i < this->GetNumberOfInputChannels(); i++)
     {
       vtkMRMLVolumeNode* cNode = vtkMRMLVolumeNode::SafeDownCast(this->InputChannelDefineLineVolume[i]->GetSelected());
       for (int j=i+1;  j < this->GetNumberOfInputChannels(); j++)
       {
         if (cNode == vtkMRMLVolumeNode::SafeDownCast(this->InputChannelDefineLineVolume[j]->GetSelected()))
         {
           vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Input Channel Error", "No two Input volumes can be the same", 
                        vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
           wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
           wizard_workflow->ProcessInputs();
           return;
         }
       }

       // i cannot be larger then umver of target volumes bc i starts with 0 !
       if (i ==  mrmlManager->GetTargetNumberOfSelectedVolumes()) 
     {
       mrmlManager->AddTargetSelectedVolumeByMRMLID(cNode->GetID());
     }
       else 
     {
       mrmlManager->SetTargetSelectedVolumeNthMRMLID(i,cNode->GetID());
     }
     }

   inputNodes->SetNumberOfInputChannelName(this->GetNumberOfInputChannels());
   for (int i=0;  i < this->GetNumberOfInputChannels(); i++)
     {
       inputNodes->SetNthInputChannelName(i,this->InputChannelDefineLineName[i]->GetWidget()->GetValue());
     }
     

   //
   if (!this->GetGUI()->IsSegmentationModeAdvanced()) 
     {
       if (atoi(this->Script("::EMSegmenterSimpleTcl::ValidateCheckList")) )
       {
         return;
       }
       
       this->UpdateTaskPreprocessingSetting();
       mrmlManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);
       mrmlManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(0);
       if (this->GetGUI()->GetPreProcessingStep()) 
     {
       this->GetGUI()->GetPreProcessingStep()->askQuestionsBeforeRunningPreprocessingFlagOff();
     }
     } 
   else 
     {
       if (this->GetGUI()->GetPreProcessingStep()) 
     {
       this->GetGUI()->GetPreProcessingStep()->askQuestionsBeforeRunningPreprocessingFlagOn();
     }
     }

   // Check Values for 
   this->Superclass::Validate();
}


//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::DeleteInputChannelFrame(vtkIdType lineNum)
{
  if (lineNum >= this->GetNumberOfInputChannels())
    {
      return;
    }
  this->InputChannelDefineLineFrame[lineNum]->Unpack();
  this->InputChannelDefineLineFrame[lineNum]->Delete();
  this->InputChannelDefineLineFrame[lineNum] = NULL;
  this->InputChannelDefineLineName[lineNum]->Delete();
  this->InputChannelDefineLineName[lineNum] = NULL;
  this->InputChannelDefineLineVolume[lineNum]->Delete();
  this->InputChannelDefineLineVolume[lineNum] = NULL;

  if (this->GetNumberOfInputChannels() == lineNum + 1)
    {
      this->InputChannelDefineLineFrame.resize(lineNum);
      this->InputChannelDefineLineVolume.resize(lineNum);
      this->InputChannelDefineLineName.resize(lineNum);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::DeleteAllInputChannelFrames()
{
  for (vtkIdType i=0 ; i < this->GetNumberOfInputChannels(); i++)
    {
      this->DeleteInputChannelFrame(i);
    }

  this->InputChannelDefineLineFrame.clear();
  this->InputChannelDefineLineName.clear();
  this->InputChannelDefineLineVolume.clear();
}

//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::CreateInputChannelFrame(int i, const char* newName, vtkMRMLVolumeNode* newVolumeNode) 
{
  if (!this->InputChannelDefineFrame)
    {
      return;
    }

  if (this->GetNumberOfInputChannels() <= i) 
    {
      this->InputChannelDefineLineFrame.resize(i+1);
      this->InputChannelDefineLineName.resize(i+1);
      this->InputChannelDefineLineVolume.resize(i+1);
    }
  
  if (!this->InputChannelDefineLineFrame[i])
    {
      this->InputChannelDefineLineFrame[i] = vtkKWFrameWithLabel::New();
     }
  
  if (!this->InputChannelDefineLineFrame[i]->IsCreated())
    {
      char title[30];
      sprintf(title, "%2d. Input Channel", i+1);
      this->InputChannelDefineLineFrame[i]->SetParent(this->InputChannelDefineFrame->GetFrame());
      this->InputChannelDefineLineFrame[i]->Create();
      this->InputChannelDefineLineFrame[i]->SetLabelText(title);
    }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->InputChannelDefineLineFrame[i]->GetWidgetName());
  
  if (!this->InputChannelDefineLineName[i])
    {
      this->InputChannelDefineLineName[i] = vtkKWEntryWithLabel::New();
    }

  if (!this->InputChannelDefineLineName[i]->IsCreated())
    {
      this->InputChannelDefineLineName[i]->SetParent(this->InputChannelDefineLineFrame[i]->GetFrame());
      this->InputChannelDefineLineName[i]->Create();
      this->InputChannelDefineLineName[i]->SetLabelWidth(6);
      this->InputChannelDefineLineName[i]->SetLabelText("Name:");
    }
  
  vtkKWEntry *entry =  this->InputChannelDefineLineName[i]->GetWidget();
  entry->SetWidth(10);
  entry->SetValue (newName);
  if (this->GetGUI()->IsSegmentationModeAdvanced())
    {
      entry->ReadOnlyOff();
    } 
  else
    {
      entry->ReadOnlyOn();
    } 

  this->Script("pack %s -side left -anchor nw -fill x -padx 0 -pady 2",this->InputChannelDefineLineName[i]->GetWidgetName());
    
  if (!this->InputChannelDefineLineVolume[i])
    {
      this->InputChannelDefineLineVolume[i] =vtkSlicerNodeSelectorWidget::New();
    }
  if (!this->InputChannelDefineLineVolume[i]->IsCreated())
    {
      this->InputChannelDefineLineVolume[i]->SetParent(this->InputChannelDefineLineFrame[i]->GetFrame());
      this->InputChannelDefineLineVolume[i]->Create();
      this->InputChannelDefineLineVolume[i]->SetLabelWidth(9);
      this->InputChannelDefineLineVolume[i]->SetLabelText(" Volume:");
      this->InputChannelDefineLineVolume[i]->NoneEnabledOn();
      this->InputChannelDefineLineVolume[i]->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
      this->InputChannelDefineLineVolume[i]->SetMRMLScene(this->GetGUI()->GetLogic()->GetMRMLScene());
      this->InputChannelDefineLineVolume[i]->GetWidget()->SetWidth(20);
      this->InputChannelDefineLineVolume[i]->SetBalloonHelpString("Select volume that defines input channel.");
    }

  this->InputChannelDefineLineVolume[i]->SetSelected(newVolumeNode);
 
  this->Script( "pack %s -side top -anchor nw -fill x -padx 0 -pady 2",  this->InputChannelDefineLineVolume[i]->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::ChangeNumberOfInputChannel(int newNumLines) 
{
  int oldNumLines = this->GetNumberOfInputChannels();
  if ( oldNumLines > newNumLines)
    {
      for (int i=oldNumLines-1;  i >= newNumLines; i--)
    {
       this->DeleteInputChannelFrame(i);
    }
    } 

  if ( oldNumLines < newNumLines)
    {
      for (int i=oldNumLines;  i < newNumLines; i++)
    {
       this->CreateInputChannelFrame(i,"",NULL);
    }
    } 
}

//----------------------------------------------------------------------------
int vtkEMSegmentInputChannelsStep::AddInputChannel() 
{
  this->ChangeNumberOfInputChannel(this->GetNumberOfInputChannels()+1);
  return this->GetNumberOfInputChannels();
}

//----------------------------------------------------------------------------
int vtkEMSegmentInputChannelsStep::RemoveInputChannel() 
{
  if (!this->GetNumberOfInputChannels())
    {
      return 0;
    }
  this->ChangeNumberOfInputChannel(this->GetNumberOfInputChannels()-1);
  return this->GetNumberOfInputChannels();
}


//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::UpdateInputChannelsfromMRML()
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  if (!mrmlManager || !wizard_widget)
    {
    return;
    }

  vtkMRMLEMSTargetNode *inputNodes = mrmlManager->GetTargetInputNode();

  if (!inputNodes) 
    {
      cout << "UpdateInputChannelsfromMRML: no input node" << endl;
      return;
    }

  this->DeleteAllInputChannelFrames();  

  if (!inputNodes->GetNumberOfVolumes()) 
    {
      return;
    } 
  
  for (int i=0; i < inputNodes->GetNumberOfVolumes(); i++)
    {
      //if (this->GetGUI()->IsSegmentationModeAdvanced()) {
    this->CreateInputChannelFrame(i,inputNodes->GetNthInputChannelName(i),inputNodes->GetNthVolumeNode(i));
    //} else {
    //    this->CreateInputChannelFrame(i,inputNodes->GetNthInputChannelName(i),NULL);
    //}
    }
}

int vtkEMSegmentInputChannelsStep::GetNumberOfInputChannels()
{
  return InputChannelDefineLineFrame.size();
}


//----------------------------------------------------------------------------
void vtkEMSegmentInputChannelsStep::UpdateTaskPreprocessingSetting()
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
      return;
    }


  std::string oldText = mrmlManager->GetNode()->GetTaskPreprocessingSetting();

  vtksys_stl::stringstream defText;
  size_t  startPos =0;
  size_t  endPos   =0;

  if (oldText.size())
    {
      endPos =oldText.find("|",1);
    }
  else {
    // Nothing to update
    return;
  } 

  for (int i =0 ; i < (int)  this->checkButton.size(); i++)
    {
      // assumes that the entry with index 0 is used during processin otherwise have problems
      if (this->checkButton[i]) {
    defText << "|C";
    defText << this->checkButton[i]->GetWidget()->GetSelectedState();
      } else {
    defText << oldText.substr(startPos,endPos-startPos).c_str();
      }

      if (endPos == std::string::npos)
    {
      startPos = std::string::npos;
      break;
    } 
      else {
    startPos = endPos;
    endPos ++;
    endPos =oldText.find("|",startPos); 
      }
    }

  for (int i =0 ; i < (int) this->volumeMenuButton.size(); i++)
    {
      // assumes that the entry with index 0 is used during processin otherwise have problems
      if (this->volumeMenuButton.size()) 
    {
      defText << "|V";
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
      defText << oldText.substr(startPos,endPos-startPos).c_str();
    }
      if (endPos == std::string::npos)
    {
      startPos = std::string::npos;
      break;
    } 
      else {
    startPos = endPos;
    endPos ++;
    endPos =oldText.find("|",startPos); 
      }
    }


  for (int i =0 ; i < (int) this->textEntry.size(); i++)
    {
      // assumes that the entry with index 0 is used during processin otherwise have problems
      if (this->textEntry[i] && this->textEntry[i]->GetWidget()) {
    defText << "|E";
    defText <<  this->textEntry[i]->GetWidget()->GetValue();
      } else {
    defText << oldText.substr(startPos,endPos-startPos).c_str();
      }

      if (endPos == std::string::npos)
    {
      startPos = std::string::npos;
      break;
    } 
      else {
    startPos = endPos;
    endPos ++;
    endPos =oldText.find("|",startPos); 
      }
    }
  mrmlManager->GetNode()->SetTaskPreprocessingSetting(defText.str().c_str());
}
