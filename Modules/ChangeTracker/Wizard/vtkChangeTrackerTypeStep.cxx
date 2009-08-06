#include "vtkChangeTrackerTypeStep.h"
#include "vtkChangeTrackerGUI.h"

#include "vtkMRMLChangeTrackerNode.h"

#include "vtkChangeTrackerLogic.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerModelsLogic.h"
#include "vtkKWCheckButton.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkSlicerApplication.h"
#include "vtkKWScale.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWProgressGauge.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSetWithLabel.h"
#include "vtkMRMLTransformNode.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChangeTrackerTypeStep);
vtkCxxRevisionMacro(vtkChangeTrackerTypeStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkChangeTrackerTypeStep::vtkChangeTrackerTypeStep()
{
  this->SetName("4/4. Define Metric"); 
  this->SetDescription("We provide several metrics to measure growth"); 
  this->WizardGUICallbackCommand->SetCallback(vtkChangeTrackerTypeStep::WizardGUICallback);

  this->FrameTypeIntensity = NULL;
  this->FrameTypeJacobian = NULL;

  this->TypeIntensityCheckButton  = NULL;
  this->TypeJacobianCheckButton  = NULL;
  
  this->RegistrationChoice = NULL;
  this->ROIRegistrationChoice = NULL;
}

//----------------------------------------------------------------------------
vtkChangeTrackerTypeStep::~vtkChangeTrackerTypeStep()
{

  if (this->FrameTypeIntensity) 
    {
    this->FrameTypeIntensity->Delete();
    this->FrameTypeIntensity = NULL;
    }


  if (this->FrameTypeJacobian) 
    {
    this->FrameTypeJacobian->Delete();
    this->FrameTypeJacobian = NULL;
    }

  if (this->TypeIntensityCheckButton)
    {
    this->TypeIntensityCheckButton->Delete();
    this->TypeIntensityCheckButton = NULL;
    }

  if (this->TypeJacobianCheckButton)
    {
    this->TypeJacobianCheckButton->Delete();
    this->TypeJacobianCheckButton = NULL;
    }

  if (this->RegistrationChoice)
    {
    this->RegistrationChoice->Delete();
    this->RegistrationChoice = NULL;
    }
  
  if (this->ROIRegistrationChoice)
    {
    this->ROIRegistrationChoice->Delete();
    this->ROIRegistrationChoice = NULL;
    }

}


//----------------------------------------------------------------------------
void vtkChangeTrackerTypeStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
    vtkChangeTrackerTypeStep *self = reinterpret_cast<vtkChangeTrackerTypeStep *>(clientData);
    if (self) { self->ProcessGUIEvents(caller, event, callData); }


}

//----------------------------------------------------------------------------
void vtkChangeTrackerTypeStep::ShowUserInterface()
{
  // cout << "vtkChangeTrackerTypeStep::ShowUserInterface Start" << endl;
  // ----------------------------------------
  // Display Super Sampled Volume 
  // ---------------------------------------- 
  this->GetGUI()->GetLogic()->DeleteAnalyzeOutput(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication()));      
  vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
  if (node) { 
    vtkMRMLVolumeNode *volumeSampleNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_SuperSampleRef()));
    vtkMRMLVolumeNode *volumeSegmentNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_SegmentRef()));
    if (volumeSampleNode && volumeSegmentNode) {
      vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();
      applicationLogic->GetSelectionNode()->SetActiveVolumeID(volumeSampleNode->GetID());

      /*
      vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();
      double oldSliceSetting[3];
      oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceOffset());
      oldSliceSetting[1] = double(applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceOffset());
      oldSliceSetting[2] = double(applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceOffset());

      applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetForegroundSelector()->SetSelected(volumeSegmentNode);
      applicationGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetForegroundSelector()->SetSelected(volumeSegmentNode);
      applicationGUI->GetMainSliceGUI("Green")->GetSliceController()->GetForegroundSelector()->SetSelected(volumeSegmentNode);
      applicationGUI->GetSlicesControlGUI()->GetSliceFadeScale()->SetValue(0.6);
      applicationLogic->PropagateVolumeSelection();

      applicationGUI->GetMainSliceGUI("Red")->GetLogic()->SetSliceOffset(oldSliceSetting[0]);
      applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->SetSliceOffset(oldSliceSetting[0]);
      applicationGUI->GetMainSliceGUI("Green")->GetLogic()->SetSliceOffset(oldSliceSetting[0]);
      
      applicationGUI->GetGUILayoutNode()->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutFourUpView);
      */

      float color[3] = {0.8, 0.8, 0.0};
      this->CreateRender(volumeSegmentNode, 0);
      this->SetRender_HighPassFilter(1,color,color);
    } 
  }

  // ----------------------------------------
  // Build GUI 
  // ----------------------------------------

  this->vtkChangeTrackerStep::ShowUserInterface();

  // Create the frame
  // Needs to be check bc otherwise with wizrd can be created over again

  this->Frame->SetLabelText("Select Growth Metric");
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->Frame->GetWidgetName());

  if (!this->FrameTypeIntensity)
    {
    this->FrameTypeIntensity = vtkKWFrame::New();
    }
  if (!this->FrameTypeIntensity->IsCreated())
    {
      this->FrameTypeIntensity->SetParent(this->Frame->GetFrame());
      this->FrameTypeIntensity->Create();
  }

  if (!this->FrameTypeJacobian)
    {
    this->FrameTypeJacobian = vtkKWFrame::New();
    }
  if (!this->FrameTypeJacobian->IsCreated())
    {
      this->FrameTypeJacobian->SetParent(this->Frame->GetFrame());
      this->FrameTypeJacobian->Create();
  }

  this->Script("pack %s %s -side top -anchor nw -fill x -padx 0 -pady 0", 
                this->FrameTypeIntensity->GetWidgetName(),
                this->FrameTypeJacobian->GetWidgetName());


  if (!this->TypeIntensityCheckButton) {
    this->TypeIntensityCheckButton = vtkKWCheckButton::New();
  }

  if (!this->TypeIntensityCheckButton->IsCreated()) {
    this->TypeIntensityCheckButton->SetParent(this->FrameTypeIntensity);
    this->TypeIntensityCheckButton->Create();
    if (node) { 
      this->TypeIntensityCheckButton->SetSelectedState(node->GetAnalysis_Intensity_Flag());
    } else {
      this->TypeIntensityCheckButton->SelectedStateOn();
    }
    this->TypeIntensityCheckButton->SetText("Analyze Intensity Patterns (fast)");
    this->TypeIntensityCheckButton->SetBalloonHelpString("The tool fuses the second scan to the first. It then detects regions of growth as areas with unusual intensity patterns between the two scans.");
  }

  if (!this->TypeJacobianCheckButton) {
    this->TypeJacobianCheckButton = vtkKWCheckButton::New();
  }

  if (!this->TypeJacobianCheckButton->IsCreated()) {
    this->TypeJacobianCheckButton->SetParent(this->FrameTypeJacobian);
    this->TypeJacobianCheckButton->Create();
    if (node) { 
      this->TypeJacobianCheckButton->SetSelectedState(node->GetAnalysis_Deformable_Flag()); 
    } else {
      this->TypeJacobianCheckButton->SelectedStateOff();
    }
    this->TypeJacobianCheckButton->SetText("Analyze Deformation Map (slow)");
    this->TypeJacobianCheckButton->SetBalloonHelpString("The fusion of the second to the first scan results in a deformation map. Unusual patterns in the map are flagged as region of growth.");
    // Currently not yet implemented
    this->TypeJacobianCheckButton->EnabledOn();
  }
  this->Script("pack %s %s -side left -anchor nw -fill x -padx 2 -pady 2", 
                this->TypeIntensityCheckButton->GetWidgetName(),
                this->TypeJacobianCheckButton->GetWidgetName());

  this->AdvancedFrame->SetLabelText("Advanced settings");
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2",
    this->AdvancedFrame->GetWidgetName());

  if(!this->RegistrationChoice)
    {
    this->RegistrationChoice = vtkKWRadioButtonSetWithLabel::New();
    }

  if(!this->RegistrationChoice->IsCreated())
    {
    this->RegistrationChoice->SetParent(this->AdvancedFrame->GetFrame());
    this->RegistrationChoice->Create();
    this->RegistrationChoice->SetLabelText("Input data alignment:");
    this->RegistrationChoice->SetBalloonHelpString("Describe if and how the iput data should be aligned");
//    this->RegistrationChoice->GetWidget()->PackVerticallyOn();

    // now add a widget for each of the options
    vtkKWRadioButton *rc0 = this->RegistrationChoice->GetWidget()->AddWidget(0);
    rc0->SetValue("0");
    rc0->SetText("Do not align my data");
    rc0->SetBalloonHelpString("If selected, ChangeTracker will not attempt to apply any registration to your data. Choose this if the input images are already aligned in space.");
    rc0->SetAnchorToWest();
    rc0->SetSelectedState(0);
//    rc0->SetEnabled(0);

    vtkKWRadioButton *rc1 = this->RegistrationChoice->GetWidget()->AddWidget(1);
    rc1->SetValue("1");
    rc1->SetText("I have a transform that aligns my data");
    rc1->SetBalloonHelpString("If selected, the images requiring alignment must be under a linear transform in the MRML tree hierarchy. The image corresponding to the second time point will be resampled according to the specified transform.");
    rc1->SetSelectedState(0);
//    rc1->SetEnabled(0);

    vtkKWRadioButton *rc2 = this->RegistrationChoice->GetWidget()->AddWidget(2);
    rc2->SetValue("2");
    rc2->SetText("Align my data automatically");
    rc2->SetBalloonHelpString("If selected, ChangeTracker will attempt to register your data using the method and parameters optimized for brain post-contrast MRI T1 sequence of head with meningioma pathology.");
    rc2->SetAnchorToWest();
    rc2->SetSelectedState(0);
//    rc2->SetEnabled(0);

    if (node) 
      {
      switch(node->GetRegistrationChoice())
        {
        case REGCHOICE_ALIGNED: rc0->SetSelectedState(1);break;
        case REGCHOICE_RESAMPLE: rc1->SetSelectedState(1);break;
        case REGCHOICE_REGISTER: rc2->SetSelectedState(1);break;
        default: std::cerr << "MRML node contains invalid data!";
        }
      }
    }

  if(!this->ROIRegistrationChoice)
    {
    this->ROIRegistrationChoice = vtkKWCheckButton::New();
    }

  if(!this->ROIRegistrationChoice->IsCreated())
    {
    this->ROIRegistrationChoice->SetParent(this->AdvancedFrame->GetFrame());
    this->ROIRegistrationChoice->Create();
    this->ROIRegistrationChoice->SetText("Rigid alignment of ROI");
    this->ROIRegistrationChoice->SetBalloonHelpString("The ROIs in the two scans will be rigidly registered prior to the analysis.");
    this->ROIRegistrationChoice->SelectedStateOn();
//    this->ROIRegistrationChoice->SetEnabled(0);
    
    this->Script("pack %s %s -side top -anchor nw -fill x -padx 2 -pady 2", 
                 this->RegistrationChoice->GetWidgetName(),
                 this->ROIRegistrationChoice->GetWidgetName());
    
    if(node)
      {
      this->ROIRegistrationChoice->SetSelectedState(node->GetROIRegistration());
      }
    }

    // hide the gory details from the user
    this->AdvancedFrame->CollapseFrame();
  {
    vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
    wizard_widget->GetCancelButton()->SetText("Analyze");
    // wizard_widget->GetCancelButton()->EnabledOff();
    if(!node){
      // No node? Should never get here. Don't know what to do.
      cerr << "ChangeTracker: This is a bug. Wrong state -- should never be here. Abort." << endl;
      abort();
    } else {
      // Linear registration has not yet finished, the button will be enabled
      // upon completion
      /*
      if(!node->GetScan2_RegisteredReady() && node->GetUseITK()){
        wizard_widget->GetCancelButton()->EnabledOff();
        wizard_widget->GetCancelButton()->SetBalloonHelpString("Please wait until initial alignment of scans is complete...");
      }
      */
    }

  }
  
  this->CreateGridButton();
  this->CreateSliceButton();

  this->AddGUIObservers(); 

  // Does not change anything 
  // this->GetGUI()->GetSliceLogic()->GetSliceNode()->SetFieldOfView(250,250,1);

  // this->SliceLogicDefine(); 
  this->GetGUI()->PropagateVolumeSelection();
  this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(0); 
}

//----------------------------------------------------------------------------
void vtkChangeTrackerTypeStep::TransitionCallback( ) 
{
  // Do not proceed if buttons are not defined
  if (!this->TypeIntensityCheckButton || !this->TypeJacobianCheckButton) return; 

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  vtkKWWizardWorkflow *wizard_workflow = wizard_widget->GetWizardWorkflow();

  wizard_widget->GetCancelButton()->EnabledOn();
  // ============================
  // make sure that analyze related nodes are empty 
  // Delete old attached node first 
  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  if (!Node) return;
  {
    vtkMRMLVolumeNode* currentNode =  vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetAnalysis_Intensity_Ref()));
    if (currentNode) { 
      this->GetGUI()->GetMRMLScene()->RemoveNode(currentNode); 
      Node->SetAnalysis_Intensity_Ref("");
    }
  }
  
  //  Process images 
  Node->SetAnalysis_Intensity_Flag(this->TypeIntensityCheckButton->GetSelectedState());
  Node->SetAnalysis_Deformable_Flag(this->TypeJacobianCheckButton->GetSelectedState());

  vtkKWRadioButton *rc0 = this->RegistrationChoice->GetWidget()->GetWidget(0);
  vtkKWRadioButton *rc1 = this->RegistrationChoice->GetWidget()->GetWidget(1);
  vtkKWRadioButton *rc2 = this->RegistrationChoice->GetWidget()->GetWidget(2);

  if(rc0->GetSelectedState())
    Node->SetRegistrationChoice(REGCHOICE_ALIGNED);
  if(rc1->GetSelectedState())
    {
    std::cerr << "User says transform is available" << std::endl;
    Node->SetRegistrationChoice(REGCHOICE_RESAMPLE);
    vtkMRMLVolumeNode *volumeNode = NULL;
    vtkMRMLTransformNode *transformNode = NULL;
    volumeNode = vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan2_Ref()));
    if(volumeNode)
      {
      transformNode = volumeNode->GetParentTransformNode();
      }
    if(!transformNode || !volumeNode)
      {
      vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(),
                                     this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),
                                     "ChangeTracker",
                                     "With the current selection, second image must be defined, and under a transform.",
                                     vtkKWMessageDialog::ErrorIcon);
      return;
      }
    Node->SetScan2_TransformRef(transformNode->GetID());
    }

  if(rc2->GetSelectedState())
    Node->SetRegistrationChoice(REGCHOICE_REGISTER);

  Node->SetROIRegistration(this->ROIRegistrationChoice->GetSelectedState());

  vtkChangeTrackerLogic* Logic = this->GetGUI()->GetLogic();

  int analysisReturnStatus =
    Logic->AnalyzeGrowth(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication()));

  if(analysisReturnStatus){
    std::string errorMessage;
    switch(analysisReturnStatus){
    case ERR_GLOBAL_REG:
      errorMessage = "ERROR: Failed to align input scans!";
      break;
    case ERR_LOCAL_REG:
      errorMessage = "ERROR: Failed to align ROI!";
      break;
    default:
      errorMessage = "ERROR: Other error during analysis";
    }
    vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(),
                                     this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),
                                     "ChangeTracker",
                                     errorMessage.c_str(),
                                     vtkKWMessageDialog::ErrorIcon);
    return;
  }

  this->RemoveResults();  
  wizard_workflow->AttemptToGoToNextStep();
}

//----------------------------------------------------------------------------
void vtkChangeTrackerTypeStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


void vtkChangeTrackerTypeStep::RemoveResults()  { 
  this->RenderRemove();
}

// AF: enable "Analyze" button in response to the completion of global
// registration
void vtkChangeTrackerTypeStep::UpdateGUI(){
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  vtkMRMLChangeTrackerNode *node = this->GetGUI()->GetNode();
  if(node->GetScan2_RegisteredReady() && node->GetUseITK()){
    wizard_widget->GetCancelButton()->EnabledOn();
    wizard_widget->GetCancelButton()->SetBalloonHelpString("Proceed with the analysis.");
  }
}
