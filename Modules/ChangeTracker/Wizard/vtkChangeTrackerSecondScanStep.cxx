#include "vtkChangeTrackerSecondScanStep.h"
#include "vtkChangeTrackerGUI.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWMessageDialog.h"
#include "vtkMRMLChangeTrackerNode.h"
#include "vtkChangeTrackerLogic.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkKWScale.h"
#include "vtkSlicerApplication.h"
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChangeTrackerSecondScanStep);
vtkCxxRevisionMacro(vtkChangeTrackerSecondScanStep, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkChangeTrackerSecondScanStep::vtkChangeTrackerSecondScanStep()
{
  this->SetName("4/4. Define Second Scan");
  this->SetDescription("Select second scan of patient");
  this->WizardGUICallbackCommand->SetCallback(vtkChangeTrackerSecondScanStep::WizardGUICallback);
}

//----------------------------------------------------------------------------
vtkChangeTrackerSecondScanStep::~vtkChangeTrackerSecondScanStep() { }

void vtkChangeTrackerSecondScanStep::RemoveAnalysisOutput() {
  this->GetGUI()->GetLogic()->DeleteAnalyzeOutput(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication()));      
}

//----------------------------------------------------------------------------
void vtkChangeTrackerSecondScanStep::ShowUserInterface()
{
  // ----------------------------------------
  // Display Super Sampled Volume 
  // ---------------------------------------- 
  this->RemoveAnalysisOutput();
  vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
  if (node) { 
    vtkMRMLVolumeNode *volumeSampleNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_SuperSampleRef()));
    vtkMRMLVolumeNode *volumeSegmentNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_SegmentRef()));
    if (volumeSampleNode && volumeSegmentNode) {
      vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();
      applicationLogic->GetSelectionNode()->SetActiveVolumeID(volumeSampleNode->GetID());

      vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();
      double oldSliceSetting[3];
      oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI0()->GetSliceController()->GetOffsetScale()->GetValue());
      oldSliceSetting[1] = double(applicationGUI->GetMainSliceGUI1()->GetSliceController()->GetOffsetScale()->GetValue());
      oldSliceSetting[2] = double(applicationGUI->GetMainSliceGUI2()->GetSliceController()->GetOffsetScale()->GetValue());

      applicationGUI->GetMainSliceGUI0()->GetSliceController()->GetForegroundSelector()->SetSelected(volumeSegmentNode);
      applicationGUI->GetMainSliceGUI1()->GetSliceController()->GetForegroundSelector()->SetSelected(volumeSegmentNode);
      applicationGUI->GetMainSliceGUI2()->GetSliceController()->GetForegroundSelector()->SetSelected(volumeSegmentNode);
      applicationGUI->GetSlicesControlGUI()->GetSliceFadeScale()->SetValue(0.6);
      applicationLogic->PropagateVolumeSelection();

      applicationGUI->GetMainSliceGUI0()->GetSliceController()->GetOffsetScale()->SetValue(oldSliceSetting[0]);
      applicationGUI->GetMainSliceGUI1()->GetSliceController()->GetOffsetScale()->SetValue(oldSliceSetting[1]);
      applicationGUI->GetMainSliceGUI2()->GetSliceController()->GetOffsetScale()->SetValue(oldSliceSetting[2]);
    } 
  }

  // ----------------------------------------
  // Build GUI 
  // ----------------------------------------


  this->vtkChangeTrackerSelectScanStep::ShowUserInterface();

  this->Frame->SetLabelText("Second Scan");
  this->VolumeMenuButton->SetBalloonHelpString("Select first scan of patient.");

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->Frame->GetWidgetName());
  this->Script( "pack %s -side top -anchor nw -padx 2 -pady 2",  this->VolumeMenuButton->GetWidgetName());

  {
    vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
    wizard_widget->GetCancelButton()->SetText("Analyze");
    wizard_widget->GetCancelButton()->EnabledOff();
  }

  // For debugging 
  vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();

  // Kilian : Has to be replaced with what ever MRML file says 
  this->VolumeMenuButton->SetSelected(applicationGUI->GetMRMLScene()->GetNodeByID("vtkMRMLScalarVolumeNode2")); 
  // this->TransitionCallback(1);  
} 

void vtkChangeTrackerSecondScanStep::UpdateMRML() 
{
  vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();

  if (!node) return;
  if (this->VolumeMenuButton && this->VolumeMenuButton->GetSelected() ) 
  {
    node->SetScan2_Ref(this->VolumeMenuButton->GetSelected()->GetID());
  }
}



void vtkChangeTrackerSecondScanStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
    vtkChangeTrackerSecondScanStep *self = reinterpret_cast<vtkChangeTrackerSecondScanStep *>(clientData);
    if( (event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent) && self) {
      self->ProcessGUIEvents(caller, callData);   
    }
}

void vtkChangeTrackerSecondScanStep::ProcessGUIEvents(vtkObject *caller, void *callData) {
    // This just has to be donw if you use the same Callbakc function for severall calls 
    vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

    if (this->VolumeMenuButton && (selector == this->VolumeMenuButton)) {
      if (this->VolumeMenuButton->GetSelected() != NULL) {
    vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
    wizard_widget->GetCancelButton()->EnabledOn();

      }
      this->UpdateMRML();      
    }
}

void vtkChangeTrackerSecondScanStep::UpdateGUI() {
  vtkMRMLChangeTrackerNode* n = this->GetGUI()->GetNode();
  if (n != NULL && this->VolumeMenuButton)
  {
    this->VolumeMenuButton->SetSelected(this->VolumeMenuButton->GetMRMLScene()->GetNodeByID(n->GetScan2_Ref()));
  }
} 

// Call from Cancel button (which is now analyze) or by selecting volume 
void vtkChangeTrackerSecondScanStep::TransitionCallback(int Flag) 
{
  if (!this->VolumeMenuButton) return;

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  vtkKWWizardWorkflow *wizard_workflow = wizard_widget->GetWizardWorkflow();

  if (this->VolumeMenuButton->GetSelected()) { 
     wizard_widget->GetCancelButton()->EnabledOn();
     // ============================
     // make sure that analyze related nodes are empty 
     // Delete old attached node first 
     vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
     if (!Node) return;
     {
       vtkMRMLVolumeNode* currentNode =  vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetAnalysis_Ref()));
       if (currentNode) { this->GetGUI()->GetMRMLScene()->RemoveNode(currentNode); }
     }

     //  Process images 
     if (this->GetGUI()->GetLogic()->AnalyzeGrowth(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication())))      
       wizard_workflow->AttemptToGoToNextStep();
  } else {
     if (Flag) {
       vtkKWMessageDialog::PopupMessage(this->GetGUI()->GetApplication(), this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow(),"Change Tracker", "Please define scan before proceeding", vtkKWMessageDialog::ErrorIcon);
     }
     wizard_widget->GetCancelButton()->EnabledOff();
  }
}

//----------------------------------------------------------------------------
void vtkChangeTrackerSecondScanStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
