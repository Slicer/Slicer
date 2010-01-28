#include "vtkChangeTrackerFirstScanStep.h"
#include "vtkChangeTrackerGUI.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h" 
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWMessageDialog.h"
#include "vtkMRMLChangeTrackerNode.h"
#include "vtkChangeTrackerLogic.h"
#include "vtkSlicerSliceControllerWidget.h"
 
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerApplication.h" 
#include "vtkKWProgressGauge.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChangeTrackerFirstScanStep);
vtkCxxRevisionMacro(vtkChangeTrackerFirstScanStep, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkChangeTrackerFirstScanStep::vtkChangeTrackerFirstScanStep()
{
  this->SetName("1/4. Define Scans");
  this->SetDescription("Select first and second scan of patient");
  this->WizardGUICallbackCommand->SetCallback(vtkChangeTrackerFirstScanStep::WizardGUICallback);
  this->SecondVolumeMenuButton = NULL;
}

//----------------------------------------------------------------------------
vtkChangeTrackerFirstScanStep::~vtkChangeTrackerFirstScanStep() { 
 if (this->SecondVolumeMenuButton)
   {
   this->SecondVolumeMenuButton->SetParent(NULL);
   this->SecondVolumeMenuButton->Delete();
   this->SecondVolumeMenuButton = NULL;
   }
}

void vtkChangeTrackerFirstScanStep::UpdateMRML() 
{
  vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
  if (!node) 
    return; 

  if (this->VolumeMenuButton && this->VolumeMenuButton->GetSelected() ) 
  {
    node->SetScan1_Ref(this->VolumeMenuButton->GetSelected()->GetID());
    vtkMRMLVolumeNode *VolNode = vtkMRMLVolumeNode::SafeDownCast(this->VolumeMenuButton->GetSelected());

    if (!VolNode && !VolNode->GetStorageNode() && !VolNode->GetStorageNode()->GetFileName()) 
      return;

    if(!node->GetWorkingDir())
      {
      vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
      std::string FilePath = application->GetTemporaryDirectory();
      node->SetWorkingDir(FilePath.c_str());
      }
  }

  if (this->SecondVolumeMenuButton && this->SecondVolumeMenuButton->GetSelected() ) {
    node->SetScan2_Ref(this->SecondVolumeMenuButton->GetSelected()->GetID());
  } 

}

void vtkChangeTrackerFirstScanStep::UpdateGUI() {

  vtkMRMLChangeTrackerNode* n = this->GetGUI()->GetNode();
  if (!n) {
    this->GetGUI()->UpdateNode();
    n = this->GetGUI()->GetNode();
  }

  if (n != NULL &&  this->VolumeMenuButton)
  {
    
    vtkSlicerApplicationGUI *applicationGUI = this->GetGUI()->GetApplicationGUI();
    this->VolumeMenuButton->SetSelected(applicationGUI->GetMRMLScene()->GetNodeByID(n->GetScan1_Ref()));
  }
  if (n != NULL &&  this->SecondVolumeMenuButton)
  {
    vtkSlicerApplicationGUI *applicationGUI = this->GetGUI()->GetApplicationGUI();
    this->SecondVolumeMenuButton->SetSelected(applicationGUI->GetMRMLScene()->GetNodeByID(n->GetScan2_Ref()));
  }
} 

//----------------------------------------------------------------------------
void vtkChangeTrackerFirstScanStep::AddGUIObservers() 
{
  this->vtkChangeTrackerSelectScanStep::AddGUIObservers();
 
  // Make sure you do not add the same event twice - need to do it bc of wizrd structure
  if (this->SecondVolumeMenuButton && (!this->SecondVolumeMenuButton->HasObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand))) 
    {
      this->SecondVolumeMenuButton->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand );  
    } 
}

void vtkChangeTrackerFirstScanStep::RemoveGUIObservers() 
{
  this->vtkChangeTrackerSelectScanStep::RemoveGUIObservers(); 
  if ( this->SecondVolumeMenuButton) 
    {
      this->SecondVolumeMenuButton->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand );  
    }
}

void vtkChangeTrackerFirstScanStep::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) {
  this->vtkChangeTrackerSelectScanStep::ProcessGUIEvents(caller, event, callData); 
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (this->SecondVolumeMenuButton && selector == this->SecondVolumeMenuButton && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent && this->SecondVolumeMenuButton->GetSelected() != NULL) 
  { 
    this->GetGUI()->UpdateMRML();
  }
  
}

//----------------------------------------------------------------------------
void vtkChangeTrackerFirstScanStep::ShowUserInterface()
{
  this->vtkChangeTrackerSelectScanStep::ShowUserInterface();

  this->Frame->SetLabelText("Select Scan");
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->Frame->GetWidgetName());
  this->VolumeMenuButton->SetLabelText("1. Scan  ");
  this->VolumeMenuButton->SetBalloonHelpString("Select first scan of patient.");

  if (!this->SecondVolumeMenuButton) {
    this->SecondVolumeMenuButton=vtkSlicerNodeSelectorWidget::New();
    this->SecondVolumeMenuButton->SetParent(this->Frame->GetFrame());
    this->SecondVolumeMenuButton->Create();
    this->SecondVolumeMenuButton->SetLabelText("2. Scan  ");
    this->SecondVolumeMenuButton->NoneEnabledOn();
    this->SecondVolumeMenuButton->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->SecondVolumeMenuButton->SetMRMLScene(this->GetGUI()->GetLogic()->GetMRMLScene());
    this->SecondVolumeMenuButton->GetWidget()->SetWidth(CHANGETRACKER_MENU_BUTTON_WIDTH);
    this->SecondVolumeMenuButton->SetBalloonHelpString("Select second scan of patient.");
    // If you want to attach a function once is selected 
    // look at GrayscaleSelector vtkSlicerVolumeMathGUI::AddGUIObservers
  }
  this->Script( "pack %s %s -side top -anchor nw -fill x -padx 0 -pady 2",  this->VolumeMenuButton->GetWidgetName(), this->SecondVolumeMenuButton->GetWidgetName());

  this->AddGUIObservers();
  this->UpdateGUI();

  vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();
  // ShowUserInterface() is called on Slicer startup during workflow
  // initialization in GUI construction. Do not reset the layout if called
  // before Enter()
  if(this->GUI->GetModuleEntered())
    applicationGUI->GetGUILayoutNode()->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutFourUpView);

  {
    vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();  
    wizard_widget->BackButtonVisibilityOff();
    if (!this->VolumeMenuButton->GetSelected() || !this->SecondVolumeMenuButton->GetSelected()) {
      wizard_widget->GetCancelButton()->EnabledOff();
    }
  }
  this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(0);

  // this->TransitionCallback(0);
}

void vtkChangeTrackerFirstScanStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
    vtkChangeTrackerFirstScanStep *self = reinterpret_cast<vtkChangeTrackerFirstScanStep *>(clientData);
    if( (event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent) && self) {
      self->ProcessGUIEvents(caller, callData);   
    }
}

void vtkChangeTrackerFirstScanStep::ProcessGUIEvents(vtkObject *caller,
                                                     void *vtkNotUsed(callData))
{
    // This just has to be donw if you use the same Callbakc function for severall calls 
    vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
    if (!this->VolumeMenuButton || !this->SecondVolumeMenuButton ) return;
    if ((selector == this->VolumeMenuButton) || (selector == this->SecondVolumeMenuButton) ) 
    { 
      vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
      if (!node) {
         // Create Node 
         this->GetGUI()->UpdateMRML();
      } else {
         this->UpdateMRML();
      }
    }

    if (this->VolumeMenuButton->GetSelected()&& this->SecondVolumeMenuButton->GetSelected()) { 
        // this->TransitionCallback(0);
        vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
        wizard_widget->GetCancelButton()->EnabledOn();
    } else {
      this->GetGUI()->GetWizardWidget()->GetCancelButton()->EnabledOff();
    }
}


//----------------------------------------------------------------------------
void vtkChangeTrackerFirstScanStep::TransitionCallback(int Flag) 
{
   if (!this->VolumeMenuButton || !this->SecondVolumeMenuButton) return;


   vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

   vtkSlicerApplication *application = vtkSlicerApplication::SafeDownCast(this->GetApplication());
   vtkSlicerModuleGUI *volumesGUI = application->GetModuleGUIByName("Volumes");
   if(volumesGUI == NULL){
     vtkKWMessageDialog::PopupMessage(this->GetGUI()->GetApplication(), 
                                      this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow(),
                                      "Change Tracker", "\"Volumes\" module is required, but not found. Please read module documentation.", 
                                      vtkKWMessageDialog::ErrorIcon);
     return;
   }

   if (this->VolumeMenuButton->GetSelected() && this->SecondVolumeMenuButton->GetSelected() ) { 
     // Check if image data is associated with them 
     vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
     vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_Ref()));
     if (!volumeNode->GetImageData()) {
       if (Flag) {
     vtkKWMessageDialog::PopupMessage(this->GetGUI()->GetApplication(), this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow(),"Change Tracker", "No image data associated with Scan 1", vtkKWMessageDialog::ErrorIcon);
       }
       return;
     }

     volumeNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan2_Ref()));
     if (!volumeNode->GetImageData()) {
       if (Flag) {
     vtkKWMessageDialog::PopupMessage(this->GetGUI()->GetApplication(), this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow(),"Change Tracker", "No image data associated with Scan 2", vtkKWMessageDialog::ErrorIcon);
       }
       return;
     }

     // AF >>> If the input is in place, launch Linear registration in background
     /* goes into AnalyzeGrowth for a while
     if(this->GetGUI()->GetNode()->GetUseITK()){
       vtkSlicerApplication *app = 
         vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
       wizard_widget->GetCancelButton()->EnabledOff();
       this->GetGUI()->GetLogic()->DoITKRegistration(app);
     }*/
     // AF <<<

     wizard_widget->GetCancelButton()->EnabledOn();
     wizard_widget->GetWizardWorkflow()->AttemptToGoToNextStep();
   } else {
     if (Flag) {
       vtkKWMessageDialog::PopupMessage(this->GetGUI()->GetApplication(), this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow(),"Change Tracker", "Please define scans before proceeding", vtkKWMessageDialog::ErrorIcon);
     }
     wizard_widget->GetCancelButton()->EnabledOff();
   }
}

//----------------------------------------------------------------------------
void vtkChangeTrackerFirstScanStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
