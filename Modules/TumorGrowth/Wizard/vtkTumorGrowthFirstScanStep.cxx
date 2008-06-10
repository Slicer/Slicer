#include "vtkTumorGrowthFirstScanStep.h"
#include "vtkTumorGrowthGUI.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h" 
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWMessageDialog.h"
#include "vtkMRMLTumorGrowthNode.h"
#include "vtkTumorGrowthLogic.h"
#include "vtkSlicerSliceControllerWidget.h"
 
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerApplication.h" 
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkTumorGrowthFirstScanStep);
vtkCxxRevisionMacro(vtkTumorGrowthFirstScanStep, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkTumorGrowthFirstScanStep::vtkTumorGrowthFirstScanStep()
{
  this->SetName("1/4. Define Scans");
  this->SetDescription("Select first and second scan of patient.");
  this->WizardGUICallbackCommand->SetCallback(vtkTumorGrowthFirstScanStep::WizardGUICallback);
  this->SecondVolumeMenuButton = NULL;
}

//----------------------------------------------------------------------------
vtkTumorGrowthFirstScanStep::~vtkTumorGrowthFirstScanStep() { 
 if (this->SecondVolumeMenuButton)
  {
    this->SecondVolumeMenuButton->Delete();
    this->SecondVolumeMenuButton = NULL;
  }

}

void vtkTumorGrowthFirstScanStep::UpdateMRML() 
{
  vtkMRMLTumorGrowthNode* node = this->GetGUI()->GetNode();
  if (!node) { return; }

  if (this->VolumeMenuButton && this->VolumeMenuButton->GetSelected() ) 
  {
    node->SetScan1_Ref(this->VolumeMenuButton->GetSelected()->GetID());
    vtkMRMLVolumeNode *VolNode = vtkMRMLVolumeNode::SafeDownCast(this->VolumeMenuButton->GetSelected());

    if (!VolNode && !VolNode->GetStorageNode() && !VolNode->GetStorageNode()->GetFileName()) {return; }    

    char CMD[2024];
    vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
    std::string FilePath = vtksys::SystemTools::GetParentDirectory(VolNode->GetStorageNode()->GetFileName()) + "-TG";
    // Check if it is a relative path !
    sprintf(CMD,"file pathtype %s",FilePath.c_str()); 
    if (strcmp(application->Script(CMD),"absolute")) {
      FilePath = vtksys::SystemTools::GetParentDirectory(VolNode->GetScene()->GetURL()) + FilePath;
    }

    sprintf(CMD,"file isdirectory %s",FilePath.c_str()); 
    if (!atoi(application->Script(CMD))) { 
      sprintf(CMD,"file mkdir %s",FilePath.c_str()); 
      application->Script(CMD); 
    }
    // Check if it path
    sprintf(CMD,"file writable %s",FilePath.c_str());

    if (!atoi(application->Script(CMD))) {
      FilePath = application->GetTemporaryDirectory();
    } 

    if (!node->GetWorkingDir() || strcmp(FilePath.c_str(),node->GetWorkingDir())) {
        node->SetWorkingDir(FilePath.c_str());
    }
  
  }

  if (this->SecondVolumeMenuButton && this->SecondVolumeMenuButton->GetSelected() ) {
    node->SetScan2_Ref(this->SecondVolumeMenuButton->GetSelected()->GetID());
  } 

}

void vtkTumorGrowthFirstScanStep::UpdateGUI() {

  vtkMRMLTumorGrowthNode* n = this->GetGUI()->GetNode();
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
void vtkTumorGrowthFirstScanStep::AddGUIObservers() 
{
  this->vtkTumorGrowthSelectScanStep::AddGUIObservers();
 
  // Make sure you do not add the same event twice - need to do it bc of wizrd structure
  if (this->SecondVolumeMenuButton && (!this->SecondVolumeMenuButton->HasObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand))) 
    {
      this->SecondVolumeMenuButton->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand );  
    } 
}

void vtkTumorGrowthFirstScanStep::RemoveGUIObservers() 
{
  this->vtkTumorGrowthSelectScanStep::RemoveGUIObservers(); 
  if ( this->SecondVolumeMenuButton) 
    {
      this->SecondVolumeMenuButton->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand );  
    }
}

void vtkTumorGrowthFirstScanStep::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) {
  this->vtkTumorGrowthSelectScanStep::ProcessGUIEvents(caller, event, callData); 
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (this->SecondVolumeMenuButton && selector == this->SecondVolumeMenuButton && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent && this->SecondVolumeMenuButton->GetSelected() != NULL) 
  { 
    this->GetGUI()->UpdateMRML();
  }
  
}

//----------------------------------------------------------------------------
void vtkTumorGrowthFirstScanStep::ShowUserInterface()
{
  this->vtkTumorGrowthSelectScanStep::ShowUserInterface();

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
    this->SecondVolumeMenuButton->GetWidget()->SetWidth(TUMORGROWTH_MENU_BUTTON_WIDTH);
    this->SecondVolumeMenuButton->SetBalloonHelpString("Select second scan of patient.");
    // If you want to attach a function once is selected 
    // look at GrayscaleSelector vtkSlicerVolumeMathGUI::AddGUIObservers
  }
  this->Script( "pack %s %s -side top -anchor nw -padx 2 -pady 2",  this->VolumeMenuButton->GetWidgetName(), this->SecondVolumeMenuButton->GetWidgetName());

  {
    vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();  
    wizard_widget->BackButtonVisibilityOff();
    wizard_widget->GetCancelButton()->EnabledOff();
  }

  this->AddGUIObservers();
  this->UpdateGUI();

  // this->TransitionCallback(0);
}

void vtkTumorGrowthFirstScanStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
    vtkTumorGrowthFirstScanStep *self = reinterpret_cast<vtkTumorGrowthFirstScanStep *>(clientData);
    if( (event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent) && self) {
      self->ProcessGUIEvents(caller, callData);   
    }
}

void vtkTumorGrowthFirstScanStep::ProcessGUIEvents(vtkObject *caller, void *callData) {
    // This just has to be donw if you use the same Callbakc function for severall calls 
    vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
    if (!this->VolumeMenuButton || !this->SecondVolumeMenuButton ) return;
    if ((selector == this->VolumeMenuButton) || (selector == this->SecondVolumeMenuButton) ) 
    { 
      vtkMRMLTumorGrowthNode* node = this->GetGUI()->GetNode();
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
    }
}


//----------------------------------------------------------------------------
void vtkTumorGrowthFirstScanStep::TransitionCallback(int Flag) 
{
   if (!this->VolumeMenuButton || !this->SecondVolumeMenuButton) return;


   vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

   if (this->VolumeMenuButton->GetSelected() && this->SecondVolumeMenuButton->GetSelected() ) { 
     
     wizard_widget->GetCancelButton()->EnabledOn();
     wizard_widget->GetWizardWorkflow()->AttemptToGoToNextStep();
   } else {
     if (Flag) {
       vtkKWMessageDialog::PopupMessage(this->GetGUI()->GetApplication(), this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow(),"Tumor Growth", "Please define scans before proceeding", vtkKWMessageDialog::ErrorIcon);
     }
     wizard_widget->GetCancelButton()->EnabledOff();
   }
}

//----------------------------------------------------------------------------
void vtkTumorGrowthFirstScanStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
