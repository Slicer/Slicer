#include "vtkTumorGrowthGUI.h"
#include "vtkTumorGrowthLogic.h"
#include "vtkMRMLTumorGrowthNode.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkTumorGrowthSelectScanStep.h"
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkTumorGrowthSelectScanStep);
vtkCxxRevisionMacro(vtkTumorGrowthSelectScanStep, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkTumorGrowthSelectScanStep::vtkTumorGrowthSelectScanStep()
{
  this->VolumeMenuButton = NULL;
}

//----------------------------------------------------------------------------
vtkTumorGrowthSelectScanStep::~vtkTumorGrowthSelectScanStep()
{
  if (this->VolumeMenuButton)
  {
    this->VolumeMenuButton->Delete();
    this->VolumeMenuButton = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkTumorGrowthSelectScanStep::ShowUserInterface()
{
  this->vtkTumorGrowthStep::ShowUserInterface();

  if (!this->VolumeMenuButton) {
    this->VolumeMenuButton=vtkSlicerNodeSelectorWidget::New();
    this->VolumeMenuButton->SetParent(this->Frame->GetFrame());
    this->VolumeMenuButton->Create();
    this->VolumeMenuButton->SetLabelText("Select Volume");
    this->VolumeMenuButton->NoneEnabledOn();
    this->VolumeMenuButton->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->VolumeMenuButton->SetMRMLScene(this->GetGUI()->GetLogic()->GetMRMLScene());
    this->VolumeMenuButton->GetWidget()->SetWidth(TUMORGROWTH_MENU_BUTTON_WIDTH);
    this->AddGUIObservers();

    // If you want to attach a function once is selected 
    // look at GrayscaleSelector vtkSlicerVolumeMathGUI::AddGUIObservers
  }
}

//----------------------------------------------------------------------------
void vtkTumorGrowthSelectScanStep::AddGUIObservers() 
{
  // Make sure you do not add the same event twice - need to do it bc of wizrd structure
  if (this->VolumeMenuButton && (!this->VolumeMenuButton->HasObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand))) 
    {
      this->VolumeMenuButton->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand );  
    } 
}

void vtkTumorGrowthSelectScanStep::RemoveGUIObservers() 
{
  if ( this->VolumeMenuButton) 
    {
      this->VolumeMenuButton->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand );  
    }
}

void vtkTumorGrowthSelectScanStep::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) { 
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (this->VolumeMenuButton && selector == this->VolumeMenuButton && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent && this->VolumeMenuButton->GetSelected() != NULL) 
  { 
    this->GetGUI()->UpdateMRML();
  }
}

//----------------------------------------------------------------------------
void vtkTumorGrowthSelectScanStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
