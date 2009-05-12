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
